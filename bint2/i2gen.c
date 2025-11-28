/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1986. */

/* Code generation */

#include "b.h"
#include "bint.h"
#include "bobj.h"
#include "i0err.h"
#include "i2nod.h"
#include "i2gen.h" /* Must be after i2nod.h */
#include "i2par.h"
#include "i3cen.h"
#include "i3env.h"
#include "i3int.h"
#include "i3sou.h"

Forward Hidden Procedure no_mysteries();
Forward Hidden Procedure inithreads();
Forward Hidden Procedure endthreads();
Forward Hidden Procedure here();
Forward Hidden Procedure sk_tsuite_comment();
Forward Hidden bool trim_opr();
Forward Hidden Procedure act_expr_gen();
Forward Hidden Procedure f_ucommand();
Forward Hidden Procedure f_fpr_formals();
Forward Hidden Procedure f_etag();
Forward Hidden Procedure f_ttag();
Forward Hidden Procedure f_ctag();

Visible Procedure fix_nodes(pt, code) parsetree *pt; parsetree *code; {
	context c; value *setup(), *su;
	sv_context(&c);
	curline= *pt; curlino= one;
	su= setup(*pt);
	if (su != Pnil) analyze(*pt, su);
	if (still_ok) no_mysteries();
	curline= *pt; curlino= one;
	inithreads();
	fix(pt, su ? 'x' : 'v');
	endthreads(code);
	cleanup();
#ifdef TYPE_CHECK
	if (cntxt != In_wsgroup && cntxt != In_prmnv)
		type_check(*pt);
#endif
	set_context(&c);
}

Hidden Procedure no_mysteries() {
	value names= keys(mysteries);
	int i, n= length(names);
	for (i= 1; i <= n; ++i) {
		value name= thof(i, names);

		if (!is_zerfun(name, Pnil)) {
			value *aa= envassoc(mysteries, name);
			if (locals != Vnil)
				e_replace(*aa, &locals, name);
			else
				e_replace(zero, &globals, name);
		}
	}
	release(names);
}

/* ******************************************************************** */

/* Utilities used by threading. */

/* A 'threaded tree' is, in our case, a fixed(*) parse tree with extra links
   that are used by the interpreter to determine the execution order.
   __________
   (*) 'Fixed' means: processed by 'fix_nodes', which removes UNPARSED
       nodes and distinguishes TAG nodes into local, global tags etc.
       fix_nodes also creates the threads, but this is accidental, not
       essential.  For UNPARSED nodes, the threads are actually laid
       in a second pass through the subtree that was UNPARSED.
   __________

   A small example: the parse tree for the expression  'a+b*c'  looks like

	(DYOP,
		(TAGlocal, "a"),
		"+",
		(DYOP,
			(TAGlocal, "b"),
			"*",
			(TAGlocal, "c"))).

   The required execution order is here:

	1) (TAGlocal, "a")
	2) (TAGlocal, "b")
	3) (TAGlocal, "c")
	4) (DYOP, ..., "*", ...)
	5) (DYOP, ..., "+", ...)

   Of course, the result of each operation (if it has a result) is pushed
   on a stack, and the operands are popped from this same stack.  Think of
   reversed polish notation (well-known by owners of HP pocket calculators).

   The 'threads' are explicit links from each node to its successor in this
   execution order.  Conditional operations like IF and AND have two threads,
   one for success and one for failure.  Loops can be made by having the
   thread from the last node of the loop body point to the head of the loop.

   Threading expressions, locations and simple-commands is easy: recursively
   thread each of the subtrees, then lay a thread from the last threaded
   to the current node.  Nodes occurring in a 'location' context are
   marked, so that the interpreter knows when to push a 'location' on
   the stack.

   Tests and looping commands cause most of the complexity of the threading
   utilities.  The basic technique is 'backpatching'.
   Nodes that need a conditional forward jump are chained together in a
   linked list, and when their destination is reached, all nodes in the
   chain get its 'address' patched into their secondary thread.  There is
   one such chain, called 'bpchain', which at all times contains those nodes
   whose secondary destination would be the next generated instruction.
   This is used by IF, WHILE, test-suites, AND and OR.

   To generate a loop, both this chain and the last normal instruction
   (if any) are diverted to the node where the loop continues.

   For test-suites, we also need to be capable of jumping unconditionally
   forward (over the remainder of the SELECT-command).  This is done by
   saving both the backpatch chain and the last node visited, and restoring
   them after the remainder has been processed.
*/

/* Implementation tricks: in order not to show circular lists to 'release',
   parse tree nodes are generated as compounds where there is room for two
   more fields than their length indicates.
*/

#define Flag (MkSmallInt(1))
	/* Flag used to indicate Location or TestRefinement node */

Hidden parsetree start; /* First instruction.  Picked up by endthreads() */

Hidden parsetree last; /* Last visited node */

Hidden parsetree bpchain; /* Backpatch chain for conditional goto's */
Hidden parsetree *wanthere; /* Chain of requests to return next tree */

#ifdef MSDOS
#ifdef M_I86LM

/* Patch for MSC 3.0 large model bugs... */
Visible parsetree *_thread(p) parsetree p; {
	return &_Thread(p);
}

Visible parsetree *_thread2(p) parsetree p; {
	return &_Thread2(p);
}

#endif /* M_I86LM */
#endif /* MSDOS */

/* Start threading */

Hidden Procedure inithreads() {
	bpchain= NilTree;
	wanthere= 0;
	last= NilTree;
	here(&start);
}

/* Finish threading */

Hidden Procedure endthreads(code) parsetree *code; {
	jumpto(Stop);
	if (!still_ok) start= NilTree;
	*code= start;
}


/* Fill 't' as secondary thread for all nodes in the backpatch chain,
   leaving the chain empty. */

Hidden Procedure backpatch(t) parsetree t; {
	parsetree u;
	while (bpchain != NilTree) {
		u= Thread2(bpchain);
		Thread2(bpchain)= t;
		bpchain= u;
	}
}

Visible Procedure jumpto(t) parsetree t; {
	parsetree u;
	if (!still_ok) return;
	while (wanthere != 0) {
		u= *wanthere;
		*wanthere= t;
		wanthere= (parsetree*)u;
	}
	while (last != NilTree) {
		u= Thread(last);
		Thread(last)= t;
		last= u;
	}
	backpatch(t);
}

Hidden parsetree seterr(n) int n; {
	return (parsetree)MkSmallInt(n);
}

/* Visit node 't', and set its secondary thread to 't2'. */

Hidden Procedure visit2(t, t2) parsetree t, t2; {
	if (!still_ok) return;
	jumpto(t);
	Thread2(t)= t2;
	Thread(t)= NilTree;
	last= t;
}

/* Visit node 't' */

Hidden Procedure visit(t) parsetree t; {
	visit2(t, NilTree);
}

/* Visit node 't' and flag it as a location (or test-refinement). */

Hidden Procedure lvisit(t) parsetree t; {
	visit2(t, Flag);
}

#ifdef NOT_USED
Hidden Procedure jumphere(t) parsetree t; {
	Thread(t)= last;
	last= t;
}
#endif

/* Add node 't' to the backpatch chain. */

Hidden Procedure jump2here(t) parsetree t; {
	if (!still_ok) return;
	Thread2(t)= bpchain;
	bpchain= t;
}

Hidden Procedure here(pl) parsetree *pl; {
	if (!still_ok) return;
	*pl= (parsetree) wanthere;
	wanthere= pl;
}

Visible Procedure hold(pl) struct state *pl; {
	if (!still_ok) return;
	pl->h_last= last; pl->h_bpchain= bpchain; pl->h_wanthere= wanthere;
	last= bpchain= NilTree; wanthere= 0;
}

Visible Procedure let_go(pl) struct state *pl; {
	parsetree p, *w;
	if (!still_ok) return;
	if (last != NilTree) {
		for (p= last; Thread(p) != NilTree; p= Thread(p))
			;
		Thread(p)= pl->h_last;
	}
	else last= pl->h_last;
	if (bpchain != NilTree) {
		for (p= bpchain; Thread2(p) != NilTree; p= Thread2(p))
			;
		Thread2(p)= pl->h_bpchain;
	}
	else bpchain= pl->h_bpchain;
	if (wanthere) {
		for (w= wanthere; *w != 0; w= (parsetree*) *w)
			;
		*w= (parsetree) pl->h_wanthere;
	}
	else wanthere= pl->h_wanthere;
}

Hidden bool reachable() {
	return last != NilTree || bpchain != NilTree || wanthere != 0;
}


/* ******************************************************************** */
/* *********************** code generation **************************** */
/* ******************************************************************** */

Forward Hidden bool is_variable();
Forward Hidden bool is_cmd_ref();

Visible Procedure fix(pt, flag) parsetree *pt; char flag; {
	struct state st; value v, function;
	parsetree t, l1= NilTree, w;
	typenode nt, nt1; string s; char c; int n, k, len;

	t= *pt;
	if (!Is_node(t) || !still_ok) return;
	nt= Nodetype(t);
	if (nt < 0 || nt >= NTYPES) syserr(MESS(2200, "fix bad tree"));
	s= gentab[nt];
	if (s == NULL) return;
	n= First_fieldnr;
	if (flag == 'x') curline= t;
	while ((c= *s++) != '\0' && still_ok) {
		switch (c) {
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			n= (c - '0') + First_fieldnr;
			break;
		case 'c':
			v= *Branch(t, n);
			if (v != Vnil) {
				len= Nfields(v);
				for (k= 0; k < len; ++k)
					fix(Field(v, k), flag);
			}
			++n;
			break;
		case '#':
			curlino= *Branch(t, n);
			++n;
			break;
		case 'g':
		case 'h':
			++n;
			break;
		case 'a':
		case 'l':
			if (flag == 'v' || flag == 't')
				c= flag;
			/* Fall through */
		case 'b':
		case 't':
		case 'u':	
		case 'v':
		case 'x':
			fix(Branch(t, n), c);
			++n;
			break;
		case 'f':
			f_fpr_formals(*Branch(t, n));
			++n;
			break;

		case ':':	/* code for WHILE loop */
			curlino= *Branch(t, WHL_LINO);
			here(&l1);
			visit(t);
			fix(Branch(t, WHL_TEST), 't');
			v= *Branch(t, WHL_SUITE);
			if (nodetype((parsetree) v) != COLON_NODE)
				syserr(BAD_WHILE);
			visit(v);
			fix(Branch(v, COLON_SUITE), 'x');
			jumpto(l1);
			jump2here(v);
			break;
			
		case ';':	/* code for TEST_SUITE */
			if (*Branch(t, TSUI_TEST) == NilTree) {
				sk_tsuite_comment(t, &w);
				if (w != NilTree)
					fix(&w, 'x');
				break;
			}
			curlino= *Branch(t, TSUI_LINO);
			visit(t);
			curline= *Branch(t, TSUI_TEST);
			fix(Branch(t, TSUI_TEST), 't');
			v= *Branch(t, TSUI_SUITE);
			if (nodetype((parsetree) v) != COLON_NODE)
				syserr(BAD_TESTSUITE);
			visit2(v, seterr(1));
			fix(Branch(v, COLON_SUITE), 'x');
			hold(&st);
			sk_tsuite_comment(t, &w);
			if (w != NilTree) {
				jump2here(v);
				fix(&w, 'x');
			}
			let_go(&st);
			break;
			
		case '?':
			if (flag == 'v')
				f_eunparsed(pt);
			else if (flag == 't')
				f_cunparsed(pt);
			else
			  syserr(MESS(2201, "fix unparsed with bad flag"));
			fix(pt, flag);
			break;
		case '@':
			f_trim_target(t, '@');
			break;
		case '|':
			f_trim_target(t, '|');
			break;
		case 'C':
			v= *Branch(t, REL_LEFT);
			nt1= nodetype((parsetree) v);
			if (Comparison(nt1))
				jump2here(v);
			break;
		case 'D':
			v= (value)*Branch(t, DYA_NAME);
			if (!is_dyafun(v, &function))
				fixerrV(NO_DEFINITION, v);
			else {
				release((value)*Branch(t, DYA_FCT));
				*Branch(t, DYA_FCT)= copystddef(function);
			}
			break;
		case 'E':
			v= (value)*Branch(t, DYA_NAME);
			if (!is_dyaprd(v, &function))
				fixerrV(NO_DEFINITION, v);
			else {
				release((value)*Branch(t, DYA_FCT));
				*Branch(t, DYA_FCT)= copystddef(function);
			}
			break;
		case 'F':
			if (*Branch(t, NUM_VALUE) == Vnil) {
				*Branch(t, NUM_VALUE)=
				numconst(*Branch(t, NUM_TEXT));
			}
			break;
		case 'G':
			jumpto(l1);
			break;
		case 'H':
			here(&l1);
			break;
		case 'I':
			if (*Branch(t, n) == NilTree)
				break;
			/* Else fall through */
		case 'J':
			jump2here(t);
			break;
		case 'K':
			hold(&st);
			break;
		case 'L':
			let_go(&st);
			break;
		case 'M':
			v= (value)*Branch(t, MON_NAME);
			if (is_variable(v) || !is_monfun(v, &function))
				fixerrV(NO_DEFINITION, v);
			else {
				release((value)*Branch(t, MON_FCT));
				*Branch(t, MON_FCT)= copystddef(function);
			}
			break;
		case 'N':
			v= (value)*Branch(t, MON_NAME);
			if (is_variable(v) || !is_monprd(v, &function))
				fixerrV(NO_DEFINITION, v);
			else {
				release((value)*Branch(t, MON_FCT));
				*Branch(t, MON_FCT)= copystddef(function);
			}
			break;
		case 'Q':	/* don't visit comment SUITE nodes */
			if (*Branch(t, n) != NilTree)
				visit(t);
			break;
#ifdef REACH
		case 'R':
			if (*Branch(t, n) != NilTree && !reachable())
			    fixerr(MESS(2202, "command cannot be reached"));
			break;
#endif
		case 'S':
			jumpto(Stop);
			break;
		case 'T':
			if (flag == 't')
				f_ctag(pt);
			else if (flag == 'v' || flag == 'x')
				f_etag(pt);
			else
				f_ttag(pt);
			break;
		case 'U':
			f_ucommand(pt);
			break;
		case 'V':
			visit(t);
			break;
		case 'X':
			if (flag == 'a' || flag == 'l' || flag == 'b')
				lvisit(t);
			else
				visit(t);
			break;
		case 'W':
/*!*/			visit2(t, seterr(1));
			break;
		case 'Y':
			if (still_ok && reachable()) {
			  if (nt == YIELD)
			    fixerr(YIELD_NO_RETURN);
			  else
			    fixerr(TEST_NO_REPORT);
			}
			break;
		case 'Z':
			if (!is_cmd_ref(t) && still_ok && reachable())
  fixerr(MESS(2203, "refinement returns no value or reports no outcome"));
  			*Branch(t, REF_START)= copy(l1);
			break;
		}
	}
}

/* skip test-suite comment nodes */

Hidden Procedure sk_tsuite_comment(v, w) parsetree v, *w; {
	while ((*w= *Branch(v, TSUI_NEXT)) != NilTree &&
	                Nodetype(*w) == TEST_SUITE &&
			*Branch(*w, TSUI_TEST) == NilTree)
		v= *w;
}

/* ******************************************************************** */

Hidden bool is_cmd_ref(t) parsetree t; { /* HACK */
	value name= *Branch(t, REF_NAME);
	string s;
	
	if (!Valid(name))
		return No;
	s= strval(name);
	/* return isupper(*s); */
	return *s <= 'Z' && *s >= 'A';
}

Visible bool is_name(v) value v; {
	if (!Valid(v) || !Is_text(v))
		return No;
	else {
		string s= strval(v);
		/* return islower(*s); */
		return *s <= 'z' && *s >= 'a';
	}
}

Visible value copystddef(f) value f; {
	if (f == Vnil || Funprd(f)->pre == Use) return Vnil;
	return copy(f);
}

Hidden bool is_basic_target(v) value v; {
	if (!Valid(v))
		return No;
	return	locals != Vnil && envassoc(locals, v) != Pnil ||
		envassoc(globals, v) != Pnil;
}

Hidden bool is_variable(v) value v; {
	if (!Valid(v))
		return No;
	return is_basic_target(v) ||
		envassoc(refinements, v) != Pnil ||
		is_zerfun(v, Pnil);
}

Hidden bool is_target(p) parsetree *p; {
	value v;
	int k, len;
	parsetree w, *left, *right;
	typenode trimtype;
	typenode nt= nodetype(*p);

	switch (nt) {

	case TAG:
		v= *Branch(*p, First_fieldnr);
		return is_basic_target(v);

	case SELECTION:
	case BEHEAD:
	case CURTAIL:
	case COMPOUND:
		return is_target(Branch(*p, First_fieldnr));

	case COLLATERAL:
		v= *Branch(*p, First_fieldnr);
		len= Nfields(v);
		k_Overfields {
			if (!is_target(Field(v, k))) return No;
		}
		return Yes;
	case DYAF:
		if (trim_opr(*Branch(*p, DYA_NAME), &trimtype)) {
			left= Branch(*p, DYA_LEFT);
			if (is_target(left)) {
				right= Branch(*p, DYA_RIGHT);
				w= node3(trimtype, copy(*left), copy(*right));
				release(*p);
				*p= w;
				return Yes;
			}
		}
		return No;

	default:
		return No;

	}
}

Hidden bool trim_opr(name, type) value name; typenode *type; {
	value v;

	if (!Valid(name))
		return No;
	if (compare(name, v= mk_text(S_BEHEAD)) == 0) {
		release(v);
		*type= BEHEAD;
		return Yes;
	}
	release(v);
	if (compare(name, v= mk_text(S_CURTAIL)) == 0) {
		release(v);
		*type= CURTAIL;
		return Yes;
	}
	release(v);
	return No;
}
	
/* ******************************************************************** */

#define WRONG_KEYWORD	MESS(2204, "wrong keyword %s")
#define NO_ACTUAL	MESS(2205, "missing actual parameter after %s")
#define EXP_KEYWORD	MESS(2206, "can't find expected %s")
#define ILL_ACTUAL	MESS(2207, "unexpected actual parameter after %s")
#define ILL_KEYWORD	MESS(2208, "unexpected keyword %s")

Hidden Procedure f_actuals(formals, actuals) parsetree formals, actuals; {
	/* name, actual, next */
	parsetree act, form, next_a, next_f, kw, *pact;
	
	do {
		kw= *Branch(actuals, ACT_KEYW);
		pact= Branch(actuals, ACT_EXPR); act= *pact;
		form= *Branch(formals, FML_TAG);
		next_a= *Branch(actuals, ACT_NEXT);
		next_f= *Branch(formals, FML_NEXT);
	
		if (compare(*Branch(formals, FML_KEYW), kw) != 0)
			fixerrV(WRONG_KEYWORD, kw);
		else if (act == NilTree && form != NilTree)
			fixerrV(NO_ACTUAL, kw);
		else if (next_a == NilTree && next_f != NilTree)
			fixerrV(EXP_KEYWORD, *Branch(next_f, FML_KEYW));
		else if (act != NilTree && form == NilTree)
			fixerrV(ILL_ACTUAL, kw);
		else if (next_a != NilTree && next_f == NilTree)
			fixerrV(ILL_KEYWORD, *Branch(next_a, ACT_KEYW));
		else if (act != NilTree)
			act_expr_gen(pact, form);
		actuals= next_a;
		formals= next_f;
	}
	while (still_ok && actuals != NilTree);
}

/* Fix and generate code for an actual parameter.
   This generates 'locate' code if it looks like a target,
   or 'evaluate' code if the parameter looks like an expression.
   The formal parameter's form is also taken into account:
   if it is a compound, and the actual is also a compound,
   the number of fields must match and the decision between 'locate'
   and 'evaluate' code is made recursively for each field.
   (If the formal is a compound but the actual isn't,
   that's OK, since it might be an expression or simple location
   of type compound.
   The reverse is also acceptable: then the formal parameter has
   a compound type.) */

Hidden Procedure act_expr_gen(pact, form) parsetree *pact; parsetree form; {
	while (Nodetype(form) == COMPOUND)
		form= *Branch(form, COMP_FIELD);
	while (Nodetype(*pact) == COMPOUND)
		pact= Branch(*pact, COMP_FIELD);
	if (Nodetype(form) == COLLATERAL && Nodetype(*pact) == COLLATERAL) {
		value vact= *Branch(*pact, COLL_SEQ);
		value vform= *Branch(form, COLL_SEQ);
		int n= Nfields(vact);
		if (n != Nfields(vform))
			fixerr(MESS(2209, "compound parameter has wrong length"));
		else {
			int k;
			for (k= 0; k < n; ++k)
				act_expr_gen(Field(vact, k), *Field(vform, k));
			visit(*pact);
		}
	}
	else {
		if (is_target(pact))
			f_targ(pact);
		else
			f_expr(pact);
	}
}

Hidden Procedure f_ucommand(pt) parsetree *pt; {
	value t= *pt, *aa;
	parsetree u, f1= *Branch(t, UCMD_NAME), f2= *Branch(t, UCMD_ACTUALS);
	release(*Branch(t, UCMD_DEF));
	*Branch(t, UCMD_DEF)= Vnil;
	if ((aa= envassoc(refinements, f1)) != Pnil) {
		if (*Branch(f2, ACT_EXPR) != Vnil
				|| *Branch(f2, ACT_NEXT) != Vnil)
			fixerr(MESS(2210, "refinement with parameters"));
		else *Branch(t, UCMD_DEF)= copy(*aa);
	}
	else if (is_unit(f1, Cmd, &aa, (wsenvptr *)0)) {
		u= How_to(*aa)->unit;
		f_actuals(*Branch(u, HOW_FORMALS), f2);
	}
	else fixerrV(MESS(2211, "you haven't told me HOW TO %s"), f1);
}

Hidden Procedure f_fpr_formals(t) parsetree t; {
	typenode nt= nodetype(t);

	switch (nt) {
	case TAG:
		break;
	case MONF: case MONPRD:
		f_targ(Branch(t, MON_RIGHT));
		break;
	case DYAF: case DYAPRD:
		f_targ(Branch(t, DYA_LEFT));
		f_targ(Branch(t, DYA_RIGHT));
		break;
	default:
		syserr(MESS(2212, "f_fpr_formals"));
	}
}

Visible bool modify_tag(name, tag) parsetree *tag; value name; {
	value *aa, function;
	*tag= NilTree;
	if (!Valid(name))
		return No;
	else if (locals != Vnil && (aa= envassoc(locals, name)) != Pnil)
		*tag= node3(TAGlocal, name, copy(*aa));
	else if ((aa= envassoc(globals, name)) != Pnil)
		*tag= node2(TAGglobal, name);
	else if ((aa= envassoc(refinements, name)) != Pnil)
		*tag= node3(TAGrefinement, name, copy(*aa));
	else if (is_zerfun(name, &function))
		*tag= node3(TAGzerfun, name, copystddef(function));
	else if (is_zerprd(name, &function))
		*tag= node3(TAGzerprd, name, copystddef(function));
	else return No;
	return Yes;
}

Hidden Procedure f_etag(pt) parsetree *pt; {
	parsetree t= *pt; value name= copy(*Branch(t, TAG_NAME));
	if (modify_tag(name, &t)) {
		release(*pt);
		*pt= t;
		if (Nodetype(t) == TAGzerprd)
			fixerrV(MESS(2213, "%s cannot be used in an expression"), name);
		else
			visit(t);
	} else {
		fixerrV(NO_INIT_OR_DEF, name);
		release(name);
	}
}

Hidden Procedure f_ttag(pt) parsetree *pt; {
	parsetree t= *pt; value name= copy(*Branch(t, TAG_NAME));
	if (modify_tag(name, &t)) {
		release(*pt);
		*pt= t;
		switch (Nodetype(t)) {
		case TAGrefinement:
			fixerr(REF_NO_TARGET);
			break;
		case TAGzerfun:
		case TAGzerprd:
			fixerrV(NO_INIT_OR_DEF, name);
			break;
		default:
			lvisit(t);
			break;
		}
	} else {
		fixerrV(NO_INIT_OR_DEF, name);
		release(name);
	}
}

#define NO_REF_OR_ZER	MESS(2214, "%s is neither a refined test nor a zeroadic predicate")

Hidden Procedure f_ctag(pt) parsetree *pt; {
	parsetree t= *pt; value name= copy(*Branch(t, TAG_NAME));
	if (modify_tag(name, &t)) {
		release(*pt);
		*pt= t;
		switch (Nodetype(t)) {
		case TAGrefinement:
			lvisit(t); /* 'Loc' flag here means 'Test' */
			break;
		case TAGzerprd:
			visit(t);
			break;
		default:
			fixerrV(NO_REF_OR_ZER, name);
			break;
		}
	} else {
		fixerrV(NO_REF_OR_ZER, name);
		release(name);
	}
}
