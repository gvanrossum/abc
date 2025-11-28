/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1986. */

/* Generic routines for all values */

#include "b.h"
#include "bmem.h"
#include "bobj.h"
#include "i1btr.h"
#include "i1tlt.h"
#include "i3typ.h"

/*ARGSUSED*/
Visible unsigned tltsyze(type, len, nptrs)
	literal type; intlet len; int *nptrs;
{
	*nptrs= 1;
	return (unsigned) (sizeof(value));
}

Visible Procedure rel_subvalues(v) value v; {
	if (Is_tlt(v)) {
		relbtree(Root(v), Itemtype(v));
		v->type= '\0';
		freemem((ptr) v);
	}
	else rrelease(v);
}

#define INCOMP	MESS(500, "incompatible types %s and %s")

Hidden Procedure incompatible(v, w) value v, w; {
	value m1, m2, m3, m;
	string s1, s2;
	
	m1= convert(m3= (value) valtype(v), No, No); release(m3);
	m2= convert(m3= (value) valtype(w), No, No); release(m3);
	s1= sstrval(m1);
	s2= sstrval(m2);
	sprintf(messbuf, getmess(INCOMP), s1, s2);
	m= mk_text(messbuf);
	interrV(-1, m);

	fstrval(s1); fstrval(s2);
	release(m1); release(m2);
	release(m);
}

Visible bool comp_ok = Yes; 		/* Temporary, to catch type errors */

relation comp_tlt(), comp_text();	/* From b1lta.c */

Visible relation compare(v, w) value v, w; {
	literal vt, wt;
	int i;
	relation rel;
	
	comp_ok = Yes;

	if (v EQ w) return(0);
	if (IsSmallInt(v) && IsSmallInt(w))
		return SmallIntVal(v) - SmallIntVal(w);
	vt = Type(v);
	wt = Type(w);
	switch (vt) {
	case Num:
		if (wt != Num) {
 incomp:
			/*Temporary until static checks are implemented*/
 			incompatible(v, w);
			comp_ok= No;
			return -1;
 		}
		return(numcomp(v, w));
	case Com:
		if (wt != Com || Nfields(v) != Nfields(w)) goto incomp;
		for (i = 0; i < Nfields(v); i++) {
			rel = compare(*Field(v, i), *Field(w, i));
			if (rel NE 0) return(rel);
		}
		return(0);
	case Tex:
		if (wt != Tex) goto incomp;
		return(comp_text(v, w));
	case Lis:
		if (wt != Lis && wt != ELT) goto incomp;
		return(comp_tlt(v, w));
	case Tab:
		if (wt != Tab && wt != ELT) goto incomp;
		return(comp_tlt(v, w));
	case ELT:
		if (wt != Tab && wt != Lis && wt != ELT) goto incomp;
		return(Root(w) EQ Bnil ? 0 : -1);
	default: 
		syserr(MESS(501, "comparison of unknown types"));
		/*NOTREACHED*/
	}
}

/* Used for set'random. Needs to be rewritten so that for small changes in v */
/* you get large changes in hash(v) */

Visible double hash(v) value v; {
	if (Is_number(v)) return numhash(v);
	else if (Is_compound(v)) {
		int len= Nfields(v), k; double d= .404*len;
		k_Overfields {
			d= .874*d+.310*hash(*Field(v, k));
		}
		return d;
	} else {
		int len= length(v), k; double d= .404*len;
		if (len == 0) return .909;
		else if (Is_text(v)) {
			value ch;
			for (k= 0; k<len; ++k) {
				ch= thof(k+1, v);
				d= .987*d+.277*charval(ch);
				release(ch);
			}
			return d;
		} else if (Is_list(v)) {
			value el;
			for (k= 0; k<len; ++k) {
				d= .874*d+.310*hash(el= thof(k+1, v));
				release(el);
			}
			return d;
		} else if (Is_table(v)) {
			for (k= 0; k<len; ++k) {
				d= .874*d+.310*hash(*key(v, k))
					 +.123*hash(*assoc(v, k));
			}
			return d;
		} else {
			syserr(MESS(502, "hash called with unknown type"));
			return (double) 0; /* (double)NULL crashes atari MWC */
		}
	}
}

Visible value convert(v, coll, outer) value v; bool coll, outer; {
	value t, quote, c, cv, sep, th, openbr, closebr; int k, len; char ch;
	switch (Type(v)) {
	case Num:
		return mk_text(convnum(v));
	case Tex:
		if (outer) return copy(v);
		quote= mk_text("\"");
		len= length(v);
		t= copy(quote);
		for (k=1; k<=len; k++) {
			c= thof(k, v);
			ch= charval(c);
			concato(&t, c);
			if (ch == '"' || ch == '`') concato(&t, c);
			release(c);
		}
		concato(&t, quote);
		release(quote);
		break;
	case Com:
		len= Nfields(v);
		outer&= coll;
		sep= mk_text(outer ? " " : ", ");
		t= mk_text(coll ? "" : "(");
		for (k= 0; k<len; ++k) {
			concato(&t, cv= convert(*Field(v, k), No, outer));
			release(cv);
			if (k < len - 1) concato(&t, sep);
		}
		release(sep);
		if (!coll) {
			concato(&t, cv= mk_text(")"));
			release(cv);
		}
		break;
	case Lis:
	case ELT:
		len= length(v);
		t= mk_text("{");
		sep= mk_text("; ");
		for (k=1; k<=len; k++) {
			concato(&t, cv= convert(th= thof(k, v), No, No));
			release(cv); release(th);
			if (k != len) concato(&t, sep);
		}
		release(sep);
		concato(&t, cv= mk_text("}"));
		release(cv);
		break;
	case Tab:
		len= length(v);
		openbr= mk_text("[");
		closebr= mk_text("]: ");
		sep= mk_text("; ");
		t= mk_text("{");
		for (k= 0; k<len; ++k) {
			concato(&t, openbr);
			concato(&t, cv= convert(*key(v, k), Yes, No));
			release(cv);
			concato(&t, closebr);
			concato(&t, cv= convert(*assoc(v, k), No, No));
			release(cv);
			if (k < len - 1) concato(&t, sep);
		}
		concato(&t, cv= mk_text("}")); release(cv);
		release(openbr); release(closebr); release(sep);
		break;
	default:
		if (testing) {
			t= mk_text("?");
			concato(&t, cv= mkchar(Type(v))); release(cv);
			concato(&t, cv= mkchar('$')); release(cv);
			break;
		}
		syserr(MESS(503, "unknown type in convert"));
	}
	return t;
}

Hidden value adj(v, w, side) value v, w; char side; {
	value t, c, sp, r, i;
	int len, wid, delta, left, right;
	c= convert(v, Yes, Yes);
	len= length(c);
	wid= intval(w);
	if (wid<=len) return c;
	else {
		delta= wid-len;
		if (side == 'L') { left= 0; right= delta; }
		else if (side == 'R') { left= delta; right= 0; }
		else {left= delta/2; right= (delta+1)/2; }
		sp= mk_text(" ");
		if (left == 0) t= c;
		else {
			t= repeat(sp, i= mk_integer(left)); release(i);
			concato(&t, c);
			release(c);
		}
		if (right != 0) {
			r= repeat(sp, i= mk_integer(right)); release(i);
			concato(&t, r);
			release(r);
		}
		release(sp);
		return t;
	}
}

Visible value adjleft(v, w) value v, w; {
	return adj(v, w, 'L');
}

Visible value adjright(v, w) value v, w; {
	return adj(v, w, 'R');
}

Visible value centre(v, w) value v, w; {
	return adj(v, w, 'C');
}

