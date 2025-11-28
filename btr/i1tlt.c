/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1986. */

/* generic routines for B texts, lists and tables */

#include "b.h"
#include "bobj.h"
#include "i1btr.h"
#include "i1tlt.h"

#define SIZE_TLT	MESS(300, "in #t, t is not a text list or table")

#define SIZE2_TLT	MESS(301, "in e#t, t is not a text list or table")
#define SIZE2_CHAR	MESS(302, "in e#t, t is a text, but e is not a character")

#define MIN_TLT		MESS(303, "in min t, t is not a text list or table")
#define MIN_EMPTY	MESS(304, "in min t, t is empty")

#define MAX_TLT		MESS(305, "in max t, t is not a text list or table")
#define MAX_EMPTY	MESS(306, "in max t, t is empty")

#define MIN2_TLT	MESS(307, "in e min t, t is not a text list or table")
#define MIN2_EMPTY	MESS(308, "in e min t, t is empty")
#define MIN2_CHAR	MESS(309, "in e min t, t is a text, but e is not a character")
#define MIN2_ELEM	MESS(310, "in e min t, no element of t exceeds e")

#define MAX2_TLT	MESS(311, "in e max t, t is not a text list or table")
#define MAX2_EMPTY	MESS(312, "in e max t, t is empty")
#define MAX2_CHAR	MESS(313, "in e max t, t is a text, but e is not a character")
#define MAX2_ELEM	MESS(314, "in e max t, no element of t is less than e")

#define ITEM_TLT	MESS(315, "in t item n, t is not a text list or table")
#define ITEM_EMPTY	MESS(316, "in t item n, t is empty")
#define ITEM_NUM	MESS(317, "in t item n, n is not a number")
#define ITEM_INT	MESS(318, "in t item n, n is not an integer")
#define ITEM_L_BND	MESS(319, "in t item n, n is < 1")
#define ITEM_U_BND	MESS(320, "in t item n, n exceeds #t")

#ifdef B_COMPAT

#define THOF_TLT	MESS(321, "in n th'of t, t is not a text list or table")
#define THOF_EMPTY	MESS(322, "in n th'of t, t is empty")
#define THOF_NUM	MESS(323, "in n th'of t, n is not a number")
#define THOF_INT	MESS(324, "in n th'of t, n is not an integer")
#define THOF_L_BND	MESS(325, "in n th'of t, n is < 1")
#define THOF_U_BND	MESS(326, "in n th'of t, n exceeds #t")

#endif /* B_COMPAT */

/* From b1lta.c */
int l2size();
value l2min(), l2max();

Visible value mk_elt() { /* {}, internal only */
	value e = grab(ELT, Lt);
	Root(e) = Bnil;
	return e;
}

Visible bool empty(v) value v; { /* #v=0, internal only */
	switch (Type(v)) {
	case ELT:
	case Lis:
	case Tex:
	case Tab:
		return Root(v) EQ Bnil;
	default:
		return No;
		/* Some routines must test empty(t) end return an error
		   message if it fails, before testing Type(t).
		   In this way, they won't give the wrong error message. */
	}
}

/* return size of (number of items in) dependent tree */

Hidden value treesize(pnode) btreeptr pnode; {
    int psize;
    value vsize, childsize, u;
    intlet l;
    psize = Size(pnode);
    if (psize EQ Bigsize) {
	switch (Flag(pnode)) {        
	case Inner:
	    vsize = mk_integer((int) Lim(pnode));
	    for (l = 0; l <= Lim(pnode); l++) {
		childsize = treesize(Ptr(pnode, l));
		u = vsize;
		vsize = sum(vsize, childsize);
		release(u);
		release(childsize);
	    }
	    break;
	case Irange: 
	    u = diff(Upbval(pnode), Lwbval(pnode));
	    vsize = sum(u, one);
	    release(u);
	    break;
	case Bottom: 
	case Crange: 
	    syserr(MESS(327, "Bigsize in Bottom or Crange"));
	}
	return(vsize);
    }
    return mk_integer(psize);
}

Visible value size(t) value t; { /* #t */
	int tsize;
	switch (Type(t)) {
	case ELT:
	case Lis:
	case Tex:
	case Tab:
		tsize = Tltsize(t);
		if (tsize EQ Bigsize) return treesize(Root(t));
		return mk_integer(tsize);
	default:
		reqerr(SIZE_TLT);
		return zero;
	}
}

Visible value item(v, num) value v, num; { /* v item num */
	value m= Vnil;
	if (!Is_tlt(v))
		interr(ITEM_TLT);
	else if (!Is_number(num))
		interr(ITEM_NUM);
	else if (empty(v))
		interr(ITEM_EMPTY);
	else if (numcomp(num, one) < 0)
		interr(ITEM_L_BND);
	else if (Tltsize(v) == Bigsize) {
		/* only happens for big Iranges;
		 * the following code is only valid for flat ranges
		 */
		value r;
		r= treesize(Root(v));
		if (compare(r, num) < 0)
			interr(ITEM_U_BND);
		else {
			release(r);
			r= sum(num, Lwbval(Root(v)));
			m= diff(r, one);
		}
		release(r);
	}		
	else {
		m= thof(intval(num), v);
		if (m == Vnil && still_ok)
			interr(ITEM_U_BND);
	}
	return m;
}

#ifdef B_COMPAT

Visible value th_of(num, v) value num, v; { /* num th'of v */
	value m= Vnil;
	if (!Is_tlt(v))
		interr(THOF_TLT);
	else if (!Is_number(num))
		interr(THOF_NUM);
	else if (empty(v))
		interr(THOF_EMPTY);
	else if (numcomp(num, one) < 0)
		interr(THOF_L_BND);
	else if (Tltsize(v) == Bigsize) {
		/* only happens for big Iranges;
		 * the following code is only valid for flat ranges
		 */
		value r;
		r= treesize(Root(v));
		if (compare(r, num) < 0)
			interr(ITEM_U_BND);
		else {
			release(r);
			r= sum(num, Lwbval(Root(v)));
			m= diff(r, one);
		}
		release(r);
	}		
	else {
		m= thof(intval(num), v);
		if (m == Vnil && still_ok)
			interr(THOF_U_BND);
	}
	return m;
}

#endif /* B_COMPAT */

/*
 * 'Walktree' handles functions on texts and associates of tables.
 * The actual function performed is determined by the 'visit' function.
 * The tree is walked (possibly recursively) and all items are visited.
 * The return value of walktree() and visit() is used to determine whether
 * the walk should continue (Yes == continue, No == stop now).
 * Global variables are used to communicate the result, and the parameters
 * of the function. The naming convention is according to "e func t".
 */

Hidden intlet tt;		/* type of walked value t */
Hidden intlet wt;		/* width of items in walked value t */
Hidden value ve; 		/* value of e, if func is dyadic */
Hidden char ce; 		/* C char in e, if t is a text */

Hidden int count; 		/* result of size2 */
Hidden bool found; 		/* result for in */
Hidden intlet m_char; 		/* result for min/max on texts */
Hidden value m_val;		/* result for min/max on tables */

#define Lowchar (-Maxintlet)	/* -infinity for characters */
#define Highchar (Maxintlet)	/* +infinity */

Hidden bool walktree(p, visit) btreeptr p; bool (*visit)(); {
	intlet l;
	
	if (p EQ Bnil) return Yes; /* i.e., not found (used by in() !) */
	for (l=0; l < Lim(p); l++) {
		switch (Flag(p)) {
		case Inner:
			if (!walktree(Ptr(p, l), visit) || !still_ok)
				return No;
			if (!(*visit)(Piitm(p, l, wt)) || !still_ok)
				return No;
			break;
		case Bottom:
			if (!(*visit)(Pbitm(p, l, wt)) || !still_ok)
				return No;
		}
	}
	return Flag(p) EQ Bottom || walktree(Ptr(p, l), visit);
}

/* Common code for min/max-1/2, size2, in. */

Hidden int tlterr;
#define T_TLT 1
#define T_EMPTY 2
#define T_CHAR 3

Hidden int tlt_func(e, t, li_func, te_visit, ta_visit)
	value e, t; 			/* [e] func t */
	value (*li_func)(); 		/* func for lists */
	bool (*te_visit)(), (*ta_visit)(); /* 'visit' for walktree */
{
	m_val = Vnil;
	if (empty(t)) {
		tlterr= T_EMPTY;
		return -1;
	}
	tt = Type(t);
	switch (tt) {
	case Lis:
		m_val = (*li_func)(e, t);
		break;
	case Tex:
		if (e NE Vnil) {
			if (!Character(e)) {
				tlterr= T_CHAR;
				return -1;
			}
			ce = Bchar(Root(e), 0);
		}
		wt = Itemwidth(Itemtype(t));
		found = !walktree(Root(t), te_visit);
		if (m_char NE Lowchar && m_char NE Highchar)
			m_val = mkchar(m_char);
		break;
	case Tab:
		ve = e;
		wt = Itemwidth(Itemtype(t));
		found = !walktree(Root(t), ta_visit);
		break;
	default:
		tlterr= T_TLT;
		return -1;
	}
	return 0;
}

Hidden value li2size(e, t) value e, t; {
	count = l2size(e, t);
	return Vnil;
}

Hidden bool te2size(pitm) itemptr pitm; {
	if (ce EQ Charval(pitm))
		count++;
	return Yes;
}

Hidden bool ta2size(pitm) itemptr pitm; {
	if (compare(ve, Ascval(pitm)) EQ 0)
		count++;
	return Yes;
}

Visible value size2(e, t) value e, t; { /* e#t */
	m_char = Lowchar;
	count = 0;
	if (tlt_func(e, t, li2size, te2size, ta2size) == -1) {
		switch (tlterr) {
		case T_TLT: interr(SIZE2_TLT);
		case T_EMPTY: return copy(zero);
		case T_CHAR: interr(SIZE2_CHAR);
		}
	}
	return mk_integer(count);
}

Hidden value li_in(e, t) value e, t; {
	found = in_keys(e, t);
	return Vnil;
}
	
Hidden bool te_in(pitm) itemptr pitm; {
	return Charval(pitm) NE ce;
}

Hidden bool ta_in(pitm) itemptr pitm; {
	return compare(ve, Ascval(pitm)) NE 0;
}

Visible bool in(e, t) value e, t; {
	m_char = Lowchar;
	found = No;
	if (tlt_func(e, t, li_in, te_in, ta_in) == -1) {
		switch (tlterr) {
		case T_EMPTY: return No;
		}
	}
	return found;
}

/*ARGSUSED*/
Hidden value li_min(e, t) value e, t; {
	return item(t, one);
}

Hidden bool te_min(pitm) itemptr pitm; {
	if (m_char > Charval(pitm))
		m_char = Charval(pitm);
	return Yes;
}

Hidden bool ta_min(pitm) itemptr pitm; {
	if (m_val EQ Vnil || compare(m_val, Ascval(pitm)) > 0) {
		release(m_val);
		m_val = copy(Ascval(pitm));
	}
	return Yes;
}

Visible value min1(t) value t; {
	m_char = Highchar;
	if (tlt_func(Vnil, t, li_min, te_min, ta_min) == -1) {
		switch (tlterr) {
		case T_TLT: interr(MIN_TLT);
		case T_EMPTY: interr(MIN_EMPTY);
		}
	}
	return m_val;
}

/*ARGSUSED*/
Hidden value li_max(e, t) value e, t; {
	value v= size(t);
	m_val = item(t, v);
	release(v);
	return m_val;
}

Hidden bool te_max(pitm) itemptr pitm; {
	if (m_char < Charval(pitm))
		m_char = Charval(pitm);
	return Yes;
}

Hidden bool ta_max(pitm) itemptr pitm; {
	if (m_val EQ Vnil || compare(Ascval(pitm), m_val) > 0) {
		release(m_val);
		m_val = copy(Ascval(pitm));
	}
	return Yes;
}

Visible value max1(t) value t; {
	m_char = Lowchar;
	if (tlt_func(Vnil, t, li_max, te_max, ta_max) == -1) {
		switch (tlterr) {
		case T_TLT: interr(MAX_TLT);
		case T_EMPTY: interr(MAX_EMPTY);
		}
	}
	return m_val;
}

Hidden bool te2min(pitm) itemptr pitm; {
	if (m_char > Charval(pitm) && Charval(pitm) > ce) {
		m_char = Charval(pitm);
	}
	return Yes;
}

Hidden bool ta2min(pitm) itemptr pitm; {
	if (compare(Ascval(pitm), ve) > 0
	    &&
	    (m_val EQ Vnil || compare(m_val, Ascval(pitm)) > 0)) {
		release(m_val);
		m_val = copy(Ascval(pitm));
	}
	return Yes;
}

Visible value min2(e, t) value e, t; {
	m_char = Highchar;
	if (tlt_func(e, t, l2min, te2min, ta2min) == -1) {
		switch (tlterr) {
		case T_TLT: interr(MIN2_TLT);
		case T_EMPTY: interr(MIN2_EMPTY);
		case T_CHAR: interr(MIN2_CHAR);
		}
		return Vnil;
	}
	if (m_val EQ Vnil && still_ok)
		reqerr(MIN2_ELEM);
	return m_val;
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

Hidden bool te2max(pitm) itemptr pitm; {
	if (ce > Charval(pitm) && Charval(pitm) > m_char) {
		m_char = Charval(pitm);
	}
	return Yes;
}

Hidden bool ta2max(pitm) itemptr pitm; {
	if (compare(ve, Ascval(pitm)) > 0
	    &&
	    (m_val EQ Vnil || compare(Ascval(pitm), m_val) > 0)) {
		release(m_val);
		m_val = copy(Ascval(pitm));
	}
	return Yes;
}

Visible value max2(e, t) value e, t; {
	m_char = Lowchar;
	if (tlt_func(e, t, l2max, te2max, ta2max) == -1) {
		switch (tlterr) {
		case T_TLT: interr(MAX2_TLT);
		case T_EMPTY: interr(MAX2_EMPTY);
		case T_CHAR: interr(MAX2_CHAR);
		}
		return Vnil;
	}
	if (m_val EQ Vnil && still_ok)
		reqerr(MAX2_ELEM);
	return m_val;
}

