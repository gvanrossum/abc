/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1986. */

/* Stacks used by the interpreter */

#include "b.h"
#include "bint.h"
#include "bmem.h"
#include "bobj.h"
#include "i0err.h"
#include "i1num.h"
#include "i2nod.h"
#include "i3cen.h"
#include "i3env.h"
#include "i3int.h"
#include "i3in2.h"
#include "i3sou.h"

Forward Hidden Procedure sub_epibreer();
Forward Hidden Procedure putbackargs();
Forward Hidden Procedure sub_putback();
Forward Hidden bool collect_value();
Forward Hidden Procedure put_it_back();

/* Fundamental registers: (shared only between this file and b3int.c) */

Visible parsetree pc; /* 'Program counter', current parsetree node */
Visible parsetree next; /* Next parsetree node (changed by jumps) */
Visible bool report; /* 'Condition code register', outcome of last test */

Hidden env boundtags; /* Holds bound tags chain */

/* Value stack: */

/* The run-time value stack grows upward, sp points to the next free entry.
   Allocated stack space lies between st_base and st_top.
   In the current invocation, the stack pointer (sp) must lie between
   st_bottom and st_top.
   Stack overflow is corrected by growing st_top, underflow is a fatal
   error (generated code is wrong).
*/

Hidden value *st_base, *st_bottom, *st_top, *sp;
Visible int call_level; /* While run() can be called recursively */

#define EmptyStack() (sp == st_bottom)
#define BotOffset() (st_bottom - st_base)
#define SetBotOffset(n) (st_bottom= st_base + (n))

#define INCREMENT 100

Hidden Procedure st_grow(incr) int incr; {
	if (st_base == Pnil) { /* First time ever */
		st_bottom= sp= st_base=
			(value*) getmem((unsigned) incr * sizeof(value *));
		st_top= st_base + incr;
	}
	else {
		int syze= (st_top - st_base) + incr;
		int n_bottom= BotOffset();
		int n_sp= sp - st_base;
		regetmem((ptr*) &st_base, (unsigned) syze * sizeof(value *));
		sp = st_base + n_sp;
		SetBotOffset(n_bottom);
		st_top= st_base + syze;
	}
}

Visible value pop() {
	if (sp <= st_bottom) {
		syserr(MESS(4100, "stack underflow"));
		return Vnil;
	}
	return *--sp;
}

Visible Procedure push(v) value v; {
	if (sp >= st_top) st_grow(INCREMENT);
	*sp++ = (v);
}

/* - - - */

/* Various call types, used as index in array: */

#define C_howto 0
#define C_yield 1
#define C_test 2

#define C_refcmd 3
#define C_refexp 4
#define C_reftest 5


/* What can happen to a thing: */

#define Old 'o'
#define Cpy 'c'
#define New 'n'
#define Non '-'

typedef struct {
	literal do_cur;
	literal do_prm;
	literal do_bnd;
	literal do_for;
	literal do_resexp;
} dorecord;


/* Table encoding what to save/restore for various call/return types: */
/* (Special cases are handled elsewhere.) */

Hidden dorecord doo[] = {
	/*		 cur  prm  bnd  for  resexp */

	/* HOW-TO */	{New, Old, Non, New, Voi},
	/* YIELD */	{New, Cpy, Non, Non, Ret},
	/* TEST */	{New, Cpy, Non, Non, Rep},

	/* REF-CMD */	{Old, Old, Old, Old, Voi},
	/* ref-expr */	{Cpy, Cpy, Non, Old, Ret},
	/* ref-test */	{Cpy, Cpy, New, Old, Rep}
};

#define MAXTYPE ((sizeof doo) / (sizeof doo[0]))

#define Checksum(type) (12345 - (type)) /* Reversible */


#define Ipush(n) push(MkSmallInt(n))
#define Ipop() SmallIntVal(pop())

/* Howto environment */
#define H_using   0  /* using workspace */
#define H_central 1  /* central workspace */

Hidden env newenv(tab, inv_env) envtab tab; env inv_env; {
	env ev= (env) getmem(sizeof(envchain));
	ev->tab= tab; /* Eats a reference to tab! */
	ev->inv_env= inv_env;
	return ev;
}

Hidden Procedure pushenv(pe) env *pe; {
	env ev= (env) getmem(sizeof(envchain));
	ev->tab= copy((*pe)->tab);
	ev->inv_env= *pe;
	*pe= ev;
}	

Hidden Procedure popenv(pe) env *pe; {
	env ev= *pe;
	*pe= ev->inv_env;
	release(ev->tab);
	freemem((ptr) ev);
}


Hidden Procedure call(type, new_pc)
     intlet type;
     parsetree new_pc;
{
	if (type < 0 || type >= MAXTYPE) syserr(MESS(4101, "bad call type"));

	/* Push other stacks */

	if (doo[type].do_bnd != Old) {
		boundtags= newenv(
			(doo[type].do_bnd == New) ? mk_elt() : Vnil,
			boundtags);
		bndtgs= &boundtags->tab;
	}
	switch (doo[type].do_cur) {

	case New:
		curnv= newenv(Vnil, curnv);
		break;

	case Cpy:
		pushenv(&curnv);
		break;

	}
	switch (doo[type].do_prm) {

	case Old:
		break;

	case Cpy:
		pushenv(&prmnv);
		break;
	}

	/* Push those things that depend on the call type: */

	if (doo[type].do_for != Old) {
		push(copy(howtoname));
	}

	/* Push howto environment: */
	if (InUsingEnv()) Ipush(H_using);
	else Ipush(H_central);

	/* Push miscellaneous context info: */
	push(curline);
	push(curlino);
	Ipush(resexp); resexp= doo[type].do_resexp;
	Ipush(cntxt);
	resval= Vnil;

	/* Push vital data: */
	push(next);
	Ipush(BotOffset()); ++call_level;
	Ipush(Checksum(type)); /* Kind of checksum */

	/* Set st_bottom and jump: */
	st_bottom= sp;
	next= new_pc;
}


Visible Procedure ret() {
	int type; value rv= resval; literal re= resexp;
	value oldcurnvtab= Vnil, oldbtl= Vnil;

	/* Clear stack: */
	while (!EmptyStack()) release(pop());

	/* Pop type and hope it's good: */
	st_bottom= st_base; /* Trick to allow popping the return info */
	type= Checksum(Ipop());
	if (type < 0 || type >= MAXTYPE) syserr(MESS(4102, "stack clobbered"));

	/* Pop vital data: */
	SetBotOffset(Ipop()); --call_level;
	next= pop();

	/* Pop context info: */
	cntxt= Ipop();
	resexp= Ipop();
	curlino= pop();
	curline= pop();

	/* Pop howto environment: */
	if (Ipop() == H_using) resetcurenv(use_env);
	else resetcurenv(cen_env);

	/* Variable part: */
	if (doo[type].do_for != Old) {
		sethowtoname(pop());
		/* FP removed */
	}
	if (doo[type].do_prm != Old)
		popenv(&prmnv);
	switch (doo[type].do_cur) {

	case Cpy:	
	case New:
		oldcurnvtab= copy(curnv->tab);
		popenv(&curnv);
		break;

	}
	if (doo[type].do_bnd != Old) {
		oldbtl= copy(*bndtgs);
		popenv(&boundtags);
		bndtgs= &boundtags->tab;
	}

	/* Fiddle bound tags */
	if (Valid(oldbtl)) {
		extbnd_tags(oldbtl, oldcurnvtab);
		release(oldbtl);
	}
	
	/* Put back arguments for commands: */
	if (type == C_howto && still_ok) putbackargs(oldcurnvtab);

	if (Valid(oldcurnvtab)) release(oldcurnvtab);
	if (call_level == 0) re_env(); /* Resets bndtgs */

	/* Push return value (if any): */
	if (re == Ret && still_ok) push(rv);
}

/* - - - */

/*ARGSUSED*/
Visible Procedure call_refinement(name, def, test)
		value name; parsetree def; bool test; {
	call(test ? C_reftest : C_refexp,
		*Branch(Refinement(def)->rp, REF_START));
}

#define YOU_TEST MESS(4103, "You haven't told me HOW TO REPORT %s")
#define YOU_YIELD MESS(4104, "You haven't told me HOW TO RETURN %s")

Hidden Procedure udfpr(nd1, name, nd2, isfunc)
		value nd1, name, nd2; bool isfunc; {
	value *aa;
	bool bad = No;
	parsetree u; int k, nlocals; funprd *fpr;
	int adicity;
	wsenvptr wse;
	wsenvptr oldwse;

	if (isfunc) adicity= nd1 ? Dfd : nd2 ? Mfd : Zfd;
	else adicity= nd1 ? Dpd : nd2 ? Mpd : Zpd;

	if (!is_unit(name, adicity, &aa, &wse)) bad = Yes;
	else if (isfunc) bad = !Is_function(*aa);
	else bad= !Is_predicate(*aa);
	if (bad) {
		interrV(isfunc ? YOU_YIELD : YOU_TEST, name);
		return;
	}
	fpr= Funprd(*aa);

	if (fpr->adic==Zfd || fpr->adic==Zpd) {
		if (Valid(nd2)) bad = Yes;
	}
	else if (fpr->adic==Mfd || fpr->adic==Mpd) {
		if (Valid(nd1)) bad = Yes;
	}

	if (bad) syserr(MESS(4105, "invoked how-to has other adicity than invoker"));
	if (fpr->pre != Use) syserr(MESS(4106, "udfpr with predefined how-to"));

	u= fpr->unit;
	oldwse = setcurenv(wse);
	if (fpr->unparsed) fix_nodes(&u, &fpr->code);
	resetcurenv(oldwse);
	if (!still_ok) {
		rem_unit(u, wse);
		return;
	}
	fpr->unparsed= No;
	nlocals= intval(*Branch(u, FPR_NLOCALS));
	call(isfunc ? C_yield : C_test, fpr->code);
	VOID setcurenv(wse); /* reset in ret(); brrr */
	curnv->tab= mk_compound(nlocals);
	for (k= 0; k < nlocals; ++k) *Field(curnv->tab, k)= Vnil;
	if (Valid(nd1)) push(copy(nd1));
	if (Valid(nd2)) push(copy(nd2));
}

Visible Procedure formula(nd1, name, nd2, tor) value nd1, name, nd2, tor; {
	if (!Valid(tor)) udfpr(nd1, name, nd2, Yes);
	else {
		if (!Is_function(tor))
			syserr(MESS(4107, "formula called with non-function"));
		push(pre_fun(nd1, Funprd(tor)->pre, nd2));
	}
}

Visible Procedure proposition(nd1, name, nd2, pred) value nd1, name, nd2, pred; {
	if (!Valid(pred)) udfpr(nd1, name, nd2, No);
	else {
		if (!Is_predicate(pred))
			syserr(MESS(4108, "proposition called with non-predicate"));
		report= pre_prop(nd1, Funprd(pred)->pre, nd2);
	}
}

/* Temporary code to hack copy/restore parameters.
   Note -- this needs extension to the case where an actuals can be
   a compound mixture of expressions and locations. */

Hidden bool is_location(v) value v; {
	while (Valid(v) && Is_compound(v))
		v= *Field(v, 0);
	return Valid(v) && (Is_simploc(v) || Is_tbseloc(v) || Is_trimloc(v));
}

Hidden value n_trim(v, B, C) value v; value B, C; {
	/* Return v|(#v-C)@(B+1) */
	value B_plus_1= sum(B, one);
	value res1= behead(v, B_plus_1);
	value sz= size(res1);
	value tail= diff(sz, C);
	value res= curtail(res1, tail);
	release(B_plus_1), release(res1), release(sz), release(tail);
	return res;
}

/* Extract a value from something that may be a location or a value.
   If it's a value, return No.
   If it's a non-empty location,
   	return Yes and put a copy of its content in *pv;
   if it's an empty location, return Yes and put Vnil in *pv. */

Hidden bool extract(l, pv) loc l; value *pv; {
	value *ll, lv;
	*pv= Vnil;
	if (l == Lnil)
		return No;
	else if (Is_simploc(l)) {
		lv= locvalue(l, &ll, No);
		if (Valid(lv))
			*pv= copy(lv);
		return Yes;
	}
	else if (Is_tbseloc(l)) {
		tbseloc *tl= Tbseloc(l);
		lv= locvalue(tl->R, &ll, Yes);
		if (still_ok) {
			if (!Is_table(lv))
				interr(SEL_NO_TABLE);
			else {
				ll= adrassoc(lv, tl->K);
				if (ll != Pnil)
					*pv= copy(*ll);
			}
		}
		return Yes;
	}
	else if (Is_trimloc(l)) {
		trimloc *rr= Trimloc(l);
		lv= locvalue(rr->R, &ll, Yes);
		if (still_ok)
			*pv= n_trim(lv, rr->B, rr->C);
		return Yes;
	}
	else if (Is_compound(l)) {
		/* Assume that if one field is a location, they all are.
		   That's not really valid, but for now it works
		   (until someone fixes the code generation...) */
		value v;
		if (!extract(*Field(l, 0), &v))
			return No;
		if (Valid(v)) {
			bool ok= Yes;
			int i;
			*pv= mk_compound(Nfields(l));
			*Field(*pv, 0)= v;
			for (i= 1; i < Nfields(l) && still_ok; ++i) {
				if (!extract(*Field(l, i), Field(*pv, i))
						&& still_ok)
					syserr(MESS(4109, "extract"));
				if (!Valid(*Field(*pv, i)))
					ok= No;
			}
			if (!ok) {
				release(*pv);
				*pv= Vnil;
			}
		}
		return Yes;
	}
	return No;
}

/* Return a copy of the value of something that may be a location or a
   value.  If it's a location, return a copy of its content
   (or Vnil if it's empty); if it's a value, return a copy of it. */

Hidden value n_content(l) loc l; {
	value v;
	if (extract(l, &v))
		return v;
	else
		return copy(l);
}

/* Put the actuals in the locals representing formals;
   save the locations of the actuals, and save their values.
   Also (actually, first of all), save the parse tree for the formals.
   Return a compound for the initialized locals.
   
   Input: the actuals are found on the stack;
   they have been pushed from left to right so have to be popped off
   in reverse order.  Each actual corresponds to one 'slot' for a
   formal parameter, which may be a multiple identifier.  It has to be
   unraveled and put in the individual locals.  There are a zillion
   reasons why this might fail.
   
   This routine is called 'epibreer' after a famous Dutch nonsense word,
   the verb 'epibreren', coined by the Amsterdam writer S. Carmiggelt (?),
   which has taken on the meaning or any complicated processing job
   (at least in the ABC group). */

Hidden value epibreer(formals, argcnt, nlocals)
	parsetree formals;			/* Parse tree for formals */
	int argcnt;				/* Nr. of argument slots */
	int nlocals;				/* Nr. of local variables */
{
	value locals= mk_compound(nlocals);	/* Local variables */
	value actuals= mk_compound(argcnt);	/* Actuals (locs/values) */
	int nextlocal= 0;			/* Next formal tag's number */
	int slot;				/* Formal slot number */
	
	/* Pop actuals from stack, in reverse order. */
	for (slot= argcnt; --slot >= 0; )
		*Field(actuals, slot)= pop();	/* Hope the count's ok... */
	
	/* Save parse tree and actuals on stack.
	   Must push a *copy* of formals because when we stop after an
	   error, everything on the stack will be popped and released.
	   Normally the copy is cancelled by a release in putbackargs. */
	push(copy((value)formals));
	push(actuals);
	slot= 0;
	while (still_ok && Valid(formals)) {
		parsetree argtree= *Branch(formals, FML_TAG);
		if (Valid(argtree)) { /* Process one parameter slot: */
			sub_epibreer(
				argtree,
				*Field(actuals, slot),
				&locals,
				&nextlocal);
			++slot;
		}
		formals= *Branch(formals, FML_NEXT);
	}
	for (; nextlocal < nlocals; ++nextlocal)
		*Field(locals, nextlocal)= Vnil;
	push(copy(locals));
	return locals;
}

#define NON_COMPOUND	MESS(4110, "putting non-compound in compound parameter")
#define WRONG_LENGTH	MESS(4111, "parameter has wrong length")

/* Unravel one actual parameter slot into possibly a collection of locals.
   The parse tree has to be traversed in the same order as when
   the numbers were assigned to local variables much earlier;
   this is a simple left-to right tree traversal. */

Hidden Procedure sub_epibreer(argtree, vl, plocals, pnextlocal)
	parsetree argtree;
	value vl;		/* Value or location */
	value *plocals;
	int *pnextlocal;
{
	value v;
	int k;
	
	switch (Nodetype(argtree)) {
	
	case TAG:
		vl= n_content(vl);
		*Field(*plocals, *pnextlocal)= mk_indirect(vl);
		release(vl);
		++*pnextlocal;
		break;
	
	case COLLATERAL:
		v= *Branch(argtree, COLL_SEQ);
		if (!Valid(v) || !Is_compound(v))
			syserr(MESS(4112, "not a compound in sub_epibreer"));
		if (Valid(vl) && !Is_compound(vl))
			vl= n_content(vl);
			/* If that isn't a simple or table-selection
			   location whose content is either Vnil or
			   a compound of the right size, we'll get an
			   error below. */
		if (Valid(vl)) {
			if (!Is_compound(vl))
				interr(NON_COMPOUND);
			else if (Nfields(vl) != Nfields(v))
				interr(WRONG_LENGTH);
		}
		for (k= 0; still_ok && k < Nfields(v); ++k)
			sub_epibreer(
				*Field(v, k),
				Valid(vl) ? *Field(vl, k) : Vnil,
				plocals,
				pnextlocal);
		break;
	
	case COMPOUND:
		sub_epibreer(
			*Branch(argtree, COMP_FIELD),
			vl,
			plocals,
			pnextlocal);
		break;
	
	default:
		syserr(MESS(4113, "bad nodetype in sub_epibreer"));
		break;
	
	}
}

/* Put a value in a location, but empty it if the value is Vnil. */

Hidden Procedure n_put(v, l) value v; loc l; {
	if (!Valid(v))
		l_del(l);
	else
		put(v, l);
}

/* Put changed formal parameters back in the corresponding locations.
   It is an error to put a changed value back in an expression. */

Hidden Procedure putbackargs(locenv) value locenv; {
	value oldlocenv= pop();	/* Original contents of locenv */
	value locs= pop();	/* Corresponding locations */
	parsetree formals= (parsetree) pop();	/* Parse tree of formals */
	
	/* Cancel extra ref to formals caused by push(copy(formals))
	   in epibreer; this leaves enough refs so we can still use it. */
	release(formals);
	
	if (locenv != oldlocenv) {
		int slot= 0;
		int nextlocal= 0;
		
		while (still_ok && Valid(formals)) {
			parsetree argtree= *Branch(formals, FML_TAG);
			if (Valid(argtree)) {
				/* Process one parameter slot: */
				sub_putback(
					argtree,
					*Field(locs, slot),
					locenv,
					&nextlocal);
				++slot;
			}
			formals= *Branch(formals, FML_NEXT);
		}
	}
	
	release(locs);
	release(oldlocenv);
}

Hidden Procedure sub_putback(argtree, lv, locenv, pnextlocal)
	parsetree argtree;
	/*loc-or*/value lv;
	value locenv;
	int *pnextlocal;
{
	value v;
	int k;
	
	while (Nodetype(argtree) == COMPOUND)
		argtree= *Branch(argtree, COMP_FIELD);
	switch (Nodetype(argtree)) {
	
	case TAG:
		if (*pnextlocal >= Nfields(locenv))
			syserr(MESS(4114, "too many tags in sub_putback"));
		v= *Field(locenv, *pnextlocal);
		if (Changed_formal(v))
			put_it_back(v, lv);
		++*pnextlocal;
		break;
	
	case COLLATERAL:
		v= *Branch(argtree, COLL_SEQ);
		if (!Valid(v) || !Is_compound(v))
			syserr(MESS(4115, "not a compound in sub_putback"));
		if (Valid(lv) && Is_compound(lv)) {
			if (Nfields(v) != Nfields(lv))
				interr(WRONG_LENGTH);
			for (k= 0; still_ok && k < Nfields(v); ++k)
				sub_putback(
					*Field(v, k),
					*Field(lv, k),
					locenv,
					pnextlocal);
		}
		else {
			if (collect_value(
					&v,
					v,
					locenv,
					pnextlocal))
				put_it_back(v, lv);
			release(v);
		}
		break;
	
	default:
		syserr(MESS(4116, "bad node type in sub_putback"));
	}
}

/* Construct the compound value corresponding to the compound of formal
   parameters held in 'seq'.
   Return Yes if any subvalue has changed.
   It is possible that the value is to be deleted; in this case all
   components must be Vnil.  A mixture of values and Vnil causes an
   error. */

Hidden bool collect_value(pv, seq, locenv, pnextlocal)
	value *pv;
	value seq;
	value locenv;
	int *pnextlocal;
{
	bool changed= No;
	int k;
	int len= Nfields(seq);
	int n_value= 0;
	
	if (!Valid(seq) || !Is_compound(seq))
		syserr(MESS(4117, "not a compound in collect_value"));
	*pv= mk_compound(len);
	for (k= 0; k < len; ++k) {
		parsetree tree= *Field(seq, k);
		value v;
		
		while (Nodetype(tree) == COMPOUND)
			tree= *Branch(tree, COMP_FIELD);
		
		switch (Nodetype(tree)) {
		
		case TAG:
			v= copy(*Field(locenv, *pnextlocal));
			if (Changed_formal(v))
				changed= Yes;
			if (Valid(v) && Is_indirect(v)) {
				release(v);
				v= copy(Indirect(v)->val);
			}
			++*pnextlocal;
			break;
		
		case COLLATERAL:
			if (collect_value(
					&v,
					*Branch(tree, COLL_SEQ),
					locenv,
					pnextlocal))
				changed= Yes;
			break;
		
		default:
			syserr(MESS(4118, "bad node type in collect_value"));
		
		}
		*Field(*pv, k)= v;
	}
	
	for (k= 0; k < len; ++k) {
		if (Valid(*Field(*pv, k)))
			n_value++;
	}
	
	if (n_value < len && n_value > 0)
	      interr(MESS(4119, "on return, part of compound holds no value"));
	if (n_value < len) {
		release(*pv);
		*pv= Vnil;
	}
	
	return changed;
}

/* Put a value in something that may be a location or a value.
   If it's a value, an error message is issued. */

Hidden Procedure put_it_back(v, l) value v; loc l; {
	if (!is_location(l))
		interr(MESS(4120, "value of expression parameter changed"));
	if (still_ok)
		n_put(v, l);
}

Visible Procedure x_user_command(name, actuals, def)
 value name; parsetree actuals; value def;
{
	how *h; parsetree u, formals; value *aa;
	value v; int len, argcnt;
	wsenvptr wse;
	wsenvptr oldwse;

	if (Valid(def)) {
		if (!Is_refinement(def)) syserr(MESS(4121, "bad def in x_user_command"));
		call(C_refcmd, *Branch(Refinement(def)->rp, REF_START));
		return;
	}
	if (!is_unit(name, Cmd, &aa, &wse)) {
		interrV(MESS(4122, "You haven't told me HOW TO %s"), name);
		return;
	}
	u= (h= How_to(*aa))->unit;
	oldwse = setcurenv(wse);
	if (h->unparsed) fix_nodes(&u, &h->code);
	resetcurenv(oldwse);
	if (!still_ok) {
		rem_unit(u, wse);
		return;
	}
	h->unparsed= No;
	formals= *Branch(u, HOW_FORMALS);
	len= intval(*Branch(u, HOW_NLOCALS));
	argcnt= 0;
	while (Valid(actuals)) { /* Count actuals */
		if (Valid(*Branch(actuals, ACT_EXPR)))
			++argcnt;
		actuals= *Branch(actuals, ACT_NEXT);
	} /* Could just as well count formals... */
	
	v= epibreer(formals, argcnt, len);
	
	call(C_howto, h->code);
	VOID setcurenv(wse); /* reset in ret(); brrr */
	
	curnv->tab= v; 
	sethowtoname(permkey(name, Cmd));
	cntxt= In_unit;
}

Visible Procedure endsta() {
#ifdef MEMTRACE
	if (st_base != Pnil) {
		freemem((ptr) st_base);
		st_base= Pnil;		
	}
#endif
}
