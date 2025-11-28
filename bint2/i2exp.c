/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1986. */

#include "b.h"
#include "bint.h"
#include "bmem.h"
#include "bobj.h"
#include "i0err.h"
#include "i2par.h"
#include "i2nod.h"
#include "i2gen.h" /* Must be after i2nod.h */
#include "i2exp.h"
#include "i3sou.h"

Forward Hidden Procedure st_extend();
Forward Hidden Procedure push_dya();
Forward Hidden parsetree par_expr();
Forward Hidden bool element();
Forward Hidden bool sel_tag();
Forward Hidden bool closed_expr();
Forward Hidden bool constant();
Forward Hidden bool digits();
Forward Hidden bool text_dis();
Forward Hidden bool is_conversion();
Forward Hidden bool tlr_dis();

/************************************************************************/

Hidden struct prio priorities[] = {
	{"", '\0', 1, 13},		/* tag functions */
	
	{S_ABOUT,	P_mon, 13, 13},
	{S_PLUS,	P_mon, 13, 13},
	{S_NUMBER,	P_mon, 12, 12},
	{S_MINUS,	P_mon, 9, 9},
	{S_NUMERATOR,	P_mon, 2, 13},
	{S_DENOMINATOR,	P_mon, 2, 13},
	{S_NUMBER,	P_dya, 12, 12},
	{S_POWER,	P_dya, 10, 11},
	{S_TIMES,	P_dya, 8, 8},
	{S_OVER,	P_dya, 7, 8},
	{S_PLUS,	P_dya, 6, 6},
	{S_MINUS,	P_dya, 6, 6},
	{S_BEHEAD,	P_dya, 5, 5},
	{S_CURTAIL,	P_dya, 5, 5},
	{S_REPEAT,	P_dya, 4, 4},
	{S_JOIN,	P_dya, 3, 3},
	{S_LEFT_ADJUST,	P_dya, 1, 1},
	{S_CENTER,	P_dya, 1, 1},
	{S_RIGHT_ADJUST, P_dya, 1, 1},
	{Bottom,	P_dya, 0, 0}
};

#define NPRIORITIES (sizeof priorities / sizeof priorities[0])

Visible struct prio *pprio(f, adic) value f; char adic; {
	struct prio *pp;
	string s= strval(f);
	
	for (pp= priorities+1; pp < &priorities[NPRIORITIES]; ++pp) {
		if (strcmp(pp->fun, s) == 0 && pp->adic == adic)
			return pp;
	}
	/* tag */
	return priorities;
}

/************************************************************************/

/*
 * Stack(adm) <= "allocated space" < Top(adm)
 * Sp(adm) points to the first free entry
 */
 
Hidden Procedure initstack(adm, n) expadm *adm; int n; {
	Stack(adm)= Sp(adm)=
	    (parsetree *) getmem((unsigned) (n * sizeof(parsetree *)));
	Top(adm)= Stack(adm) + n;
	Nextend(adm)= n;
}

Visible Procedure initexp(adm, n, level) expadm *adm; int n; char level; {
	initstack(adm, n);
	push_dya(adm, mk_text(Bottom));
	Level(adm)= level;
	Prop(adm)= No;
	Nfld(adm)= 0;
}

Visible Procedure endstack(adm) expadm *adm; {
	while (Sp(adm) > Stack(adm)) release(Pop(adm));
	freemem((ptr) Stack(adm));
}

Visible Procedure push_item(adm, v) expadm *adm; parsetree v; {
	if (Sp(adm) >= Top(adm)) st_extend(adm);
	*Sp(adm)++= v;
}

Hidden Procedure st_extend(adm) expadm *adm; {
	int syze= (Top(adm) - Stack(adm)) + Nextend(adm);
	int n= Sp(adm) - Stack(adm);
	
	regetmem((ptr *) &Stack(adm),
			(unsigned) syze * sizeof(parsetree *));
	Sp(adm)= Stack(adm) + n;
	Top(adm)= Stack(adm) + syze;
}

/* to recognize them on the stack, dyadic operators are pushed as compounds */

Hidden Procedure push_dya(adm, v) expadm *adm; value v; {
	value w= mk_compound(1);
	*Field(w, 0)= v;
	push_item(adm, (parsetree) w);
}

/* ******************************************************************** */
/*		expression						*/
/* ******************************************************************** */

Visible parsetree expr(q) txptr q; {
	return collateral(q, singexpr);
}

Visible parsetree singexpr(q) txptr q; {
	expadm adm;
	parsetree v;
		
	if (nothing(q, MESS(2100, "nothing instead of expected expression")))
		return NilTree;
	initexp(&adm, N_EXP_STACK, PARSER);
	v= par_expr(q, &adm);
	endstack(&adm);
	return v;
}

Hidden bool unparsed= No;

Visible parsetree unp_test(q) txptr q; {
	unparsed= Yes;
	return singexpr(q);
}
	
#define S_tag1	0	/* zer or mon tag */
#define S_tag2	1	/* zer, mon or dya tag */
#define S_elmt	2	/* element */
#define S_dm	3	/* dyamon sign */
#define S_mon	4	/* mon sign */
#define S_dya	5	/* dya sign */
#define S_unp	6	/* unparsed */
#define S_err	7	/* error */

#define K_tag		0
#define K_element	1
#define K_dyamon	2
#define K_mon		3
#define K_dya		4

Hidden int trans[5][8]= {
	{S_tag2, S_unp, S_dya,  S_unp, S_tag1, S_tag1, S_unp, S_err},
	{S_elmt, S_unp, S_err,  S_unp, S_elmt, S_elmt, S_unp, S_err},
	{S_dm,   S_unp, S_dya,  S_unp, S_mon,  S_mon,  S_unp, S_err},
	{S_mon,  S_unp, S_err,  S_unp, S_mon,  S_mon,  S_unp, S_err},
	{S_dya,  S_dya, S_dya,  S_err, S_err,  S_err,  S_unp, S_err}
};

Hidden parsetree par_expr(q, adm) txptr q; expadm *adm; {
	parsetree v= NilTree;
	value w, c;
	int state= S_dya;
	int kind, n;
	txptr tx0, tx1;

	if (unparsed) {
		state= S_unp;
		unparsed= No;
	}
	skipsp(&tx);
	tx0= tx;
	while (Text(q)) {
		tx1= tx;
		if (tag_operator(q, &w)) kind= K_tag;
		else if (element(q, &w)) kind= K_element;
		else if (dyamon_sign(&w)) kind= K_dyamon;
		else if (mon_sign(&w)) kind= K_mon;
		else if (dya_sign(&w)) kind= K_dya;
		else break;
		
		state= trans[kind][state];
		if (state == S_err) {
			release(w);
			tx= tx1;
			break;
		}
		else if (state == S_dya) 
			do_dya(adm, w);
		else
			push_item(adm, (parsetree) w);
		skipsp(&tx);
	}
	if (Text(q)) {
		if (tx == tx0) parerr(NO_EXPR);
		else parerr(UPTO_EXPR);
		tx= q;
		return NilTree;
	}
	switch (state) {
		case S_mon:
		case S_dya:
		case S_dm:
		case S_err:
			parerr(NO_EXPR);
			return NilTree;
		case S_unp:
			n= (Sp(adm) - Stack(adm)) - 1;
			c= mk_compound(n);
			while (n > 0) *Field(c, --n)= Pop(adm);
			for (tx1= tx; Space(Char(tx1-1)); --tx1);
			return node3(UNPARSED, c, cr_text(tx0, tx1));
		default:
			while (Sp(adm) - Stack(adm) > 2)
				reduce(adm);
			v= Pop(adm);
			if (ValidTree(v) && Is_text(v))
				v= node2(TAG, v);
			return v;
	}
}

#define SHIFT	'1'
#define START	'2'

#define Prio_err(adm) \
		(Level(adm) == PARSER ? pprerr(PRIO) : fixerr(PRIO))

Visible Procedure do_dya(adm, v) expadm *adm; value v; {
	parsetree *p= Sp(adm) - 2;	/* skip operand */
	struct prio *pdya, *popr;
	char action= START;

	pdya= dprio(v);
	for (;;) {
		popr= Dya_opr(*p) ? dprio(*Field(*p, 0)) : mprio(*p);
		if (popr->L >= pdya->H) {	/* reduce */
			if (action == SHIFT)
				Prio_err(adm);
			reduce(adm);
			p= Sp(adm) - 2;
			action= START;
		}
		else if (pdya->L > popr->H) {
			if (Dya_opr(*p))
				break;
			--p;
			action= SHIFT;
		}
		else {
			if (action == START)
				Prio_err(adm);
			break;
		}
	}
	push_dya(adm, v);	/* shift */
}

Visible Procedure reduce(adm) expadm *adm; {
	parsetree x, y;
	value opr, f= Vnil, v;
	
	/* right operand */
	y= Pop(adm);
	if (Level(adm) == PARSER && ValidTree(y) && Is_text(y))
		y= node2(TAG, y);
	
	/* operator */
	opr= (value) Pop(adm);
	if (!Dya_opr(opr)) {
		if (Level(adm) == FIXER) {
			VOID is_monfun(opr, &f);
			f= copystddef(f);
		}
		push_item(adm, node4(MONF, opr, y, f));
		return;
	}
	opr= copy(*Field(v= opr, 0));
	release(v);

	/* left operand */
	x= Pop(adm);
	if (Level(adm) == PARSER && ValidTree(x) && Is_text(x))
		x= node2(TAG, x);
	if (Level(adm) == FIXER) {
		VOID is_dyafun(opr, &f);
		f= copystddef(f);
	}
	push_item(adm, node5(DYAF, x, opr, y, f));
}

/* ******************************************************************** */
/*		element							*/
/* ******************************************************************** */

Hidden bool element(q, v) txptr q; value *v; {
	parsetree w;
	
	if (sel_tag(q, &w) || closed_expr(q, &w) || constant(q, &w) ||
			text_dis(q, &w) || tlr_dis(q, &w)) {
		selection(q, &w);
		*v= (value) w;
		return Yes;
	}
	return No;
}

/* ******************************************************************** */
/*		(sel_tag)						*/
/* ******************************************************************** */

Hidden bool sel_tag(q, v) txptr q; parsetree *v; {
	value name; txptr tx0= tx;
	if (Text(q) && is_tag(&name)) {
		txptr tx1= tx;
		skipsp(&tx);
		if (Text(q) && sub_sign) {
			tx= tx1;
			*v= node2(TAG, name);
			return Yes;
		}
		else {
			release(name);
			tx= tx0;
		}
	}
	return No;
}

/* ******************************************************************** */
/*		(expression)						*/
/* ******************************************************************** */

Hidden bool closed_expr(q, v) txptr q; parsetree *v; {
	return open_sign ? (*v= compound(q, expr), Yes) : No;
}

/* ******************************************************************** */
/*		constant						*/
/*									*/
/* note: stand_alone e<number> not allowed				*/
/* ******************************************************************** */

Hidden bool constant(q, v) txptr q; parsetree *v; {
	if (Dig(Char(tx)) || Char(tx) == C_POINT) {
		txptr tx0= tx;
		bool d= digits(q);
		value text;
		if (Text(q) && point_sign && !digits(q) && !d)
			pprerr(MESS(2101, "point without digits"));
		if (Text(q) && Char(tx) == 'e' &&
		    (Dig(Char(tx+1)) || !Tagmark(tx+1) )
		   ) {
			tx++;
			if (Text(q) && (plus_sign || minus_sign));
			if (!digits(q)) pprerr(MESS(2102, "e not followed by exponent"));
		}
		text= cr_text(tx0, tx);
		*v= node3(NUMBER, Vnil, text);
		return Yes;
	}
	return No;
}

Hidden bool digits(q) txptr q; {
	txptr tx0= tx;
	while (Text(q) && Dig(Char(tx))) tx++;
	return tx > tx0;
}

/* ******************************************************************** */
/*		textual_display						*/
/* ******************************************************************** */

Forward Hidden parsetree text_body();

Hidden bool text_dis(q, v) txptr q; parsetree *v; {
	value aq;
	if (texdis_sign(&aq)) {
		parsetree w;
		w= text_body(q, aq);
		if (w == NilTree) w= node3(TEXT_LIT, mk_text(""), NilTree);
		*v= node3(TEXT_DIS, aq, w);
		return Yes;
	}
	return No;
}

Hidden parsetree text_body(q, aq) txptr q; value aq; {
	value head; parsetree tail;
	char quote= strval(aq)[0];
	txptr tx0= tx;
	while (Text(q)) {
		if (Char(tx) == quote || Char(tx) == C_CONVERT) {
			head= tx0 < tx ? cr_text(tx0, tx) : Vnil;
			if (Char(tx) == Char(tx+1)) {
				value spec= cr_text(tx, tx+1);
				tx+= 2;
				tail= text_body(q, aq);
				tail= node3(TEXT_LIT, spec, tail);
			}
			else {
				parsetree v;
				if (is_conversion(q, &v)) {
					tail= text_body(q, aq);
					tail= node3(TEXT_CONV, v, tail);
				}
				else {
					tx++;
					tail= NilTree;
				}
			}
			if (head == Vnil) return tail;
			else return node3(TEXT_LIT, head, tail);
		}
		else tx++;
	}
	parerrV(MESS(2103, "cannot find matching %s"), aq);
	return NilTree;
}

Hidden bool is_conversion(q, v) txptr q; parsetree *v; {
	if (conv_sign) {
		txptr ftx, ttx;
		req(S_CONVERT, q, &ftx, &ttx);
		*v= expr(ftx); tx= ttx; 
		return Yes;
	}
	return No;
}

/* ******************************************************************** */
/*		table_display; list_display; range_display;		*/
/* ******************************************************************** */

Hidden bool elt_dis(v) parsetree *v; {
	if (curlyclose_sign) {
		*v= node1(ELT_DIS);
		return Yes;
	}
	return No;
}

Hidden parsetree par_lta(q, adm, lta_item) txptr q; expadm *adm;
		Procedure (*lta_item)(); {
	txptr ftx, ttx;
	int n;
	parsetree v;

	while (still_ok && find(S_SEMICOLON, q, &ftx, &ttx)) {
		(*lta_item)(ftx, adm);
		tx= ttx;
	}
	(*lta_item)(q, adm);
	n= Sp(adm) - Stack(adm);
	v= mk_compound(n);
	while (n>0) *Field(v, --n)= Pop(adm);
	return v;
}

Hidden Procedure tab_item(q, adm) txptr q; expadm *adm; {
	txptr ftx, ttx;
	
	need(S_SUB);
	req(S_BUS, q, &ftx, &ttx);
	push_item(adm, expr(ftx));
	tx= ttx;
	need(S_COLON);
	push_item(adm, singexpr(q));
}

Hidden bool tab_dis(q, v) txptr q; parsetree *v; {
	if (Char(tx) == C_SUB) {
		expadm adm;
		parsetree w;
		
		initstack(&adm, N_LTA_STACK);
		w= par_lta(q, &adm, tab_item);
		endstack(&adm);
		*v= node2(TAB_DIS, w);
		return Yes;
	}
	return No;
}

Hidden bool range_elem(q, v) txptr q; parsetree *v; {
	txptr ftx, ttx;
	if (find(S_RANGE, q, &ftx, &ttx)) {
		parsetree w;
		if (Char(ttx) == '.') { ftx++; ttx++; }
		w= singexpr(ftx); tx= ttx;
		*v= node3(RANGE_BNDS, w, singexpr(q));
		return Yes;
	}
	return No;
}

Hidden Procedure list_item(q, adm) txptr q; expadm *adm; {
	parsetree r;
	if (range_elem(q, &r))
		push_item(adm, r);
	else
		push_item(adm, singexpr(q));
}

Hidden Procedure list_dis(q, v) txptr q; parsetree *v; {
	expadm adm;
	parsetree w;
	
	initstack(&adm, N_LTA_STACK);
	w= par_lta(q, &adm, list_item);
	endstack(&adm);
	*v= node2(LIST_DIS, w);
}

Hidden bool tlr_dis(q, v) txptr q; parsetree *v; {
	if (curlyopen_sign) {
		skipsp(&tx);
		if (!elt_dis(v)) {
			txptr ftx, ttx;
			req(S_CURCLOSE, q, &ftx, &ttx);
			skipsp(&tx);
			if (!tab_dis(ftx, v)) list_dis(ftx, v);
			tx= ttx;
		}
		return Yes;
	}
	return No;
}

/* ******************************************************************** */
/*		selection						*/
/* ******************************************************************** */

Visible Procedure selection(q, v) txptr q; parsetree *v; {
	txptr ftx, ttx;
	skipsp(&tx);
	while (Text(q) && sub_sign) {
		req(S_BUS, q, &ftx, &ttx);
		*v= node3(SELECTION, *v, expr(ftx)); tx= ttx;
		skipsp(&tx);
	}
}

/* ******************************************************************** */
/*		trim_target						*/
/* ******************************************************************** */

Visible Procedure trim_target(q, v) txptr q; parsetree *v; {
	parsetree w;
	value name;
	bool beh;
	struct prio *ptrim, *pdya;
	txptr ftx;

	skipsp(&tx);
	while (Text(q) && ((beh= behead_sign) || curtl_sign)) {
		skipsp(&tx);
		if (!findtrim(q, &ftx)) ftx= q;
		w= singexpr(ftx); tx= ftx;
		if (nodetype(w) == DYAF) {
			pdya= dprio(*Branch(w, DYA_NAME));
			name= mk_text(beh ? S_BEHEAD : S_CURTAIL);
			ptrim= dprio(name);
			if (!(pdya->L > ptrim->H))
				pprerr(NO_TRIM_TARG);
			release(name);
		}
		*v= node3(beh ? BEHEAD : CURTAIL, *v, w);
	}
}

/* ******************************************************************** */
/*		tag_operator	 					*/
/* ******************************************************************** */

Visible bool tag_operator(q, v) txptr q; value *v; {
	value w;
	txptr tx0= tx;
	if (Text(q) && is_tag(&w)) {
		skipsp(&tx);
		if (Text(q) && sub_sign) {
			release(w);
			tx= tx0;
			return No;
		}
		*v= w;
		return Yes;
	}
	return No;
}
