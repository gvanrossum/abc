/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1988. */

#include "b.h"
#include "bobj.h" /* for relation */
#include "i1num.h"

/*
 * This file contains routines to speed up some time consuming operations
 * when dealing with big numbers, such as:
 * . exactly() via prod2n()
 * . convnum() and round2() via prod10n()
 */

/* To shut off lint and other warnings: */
#undef Copy
#define Copy(x) ((integer)copy((value)(x)))

/*
 * prod2n() returns (v= p/q) * (2**n),
 * simplification rationals is done by sieving the 2 factors out of
 * q if n>0 or p if n<0
 */

Visible value prod2n(v, n, simplify) value v, n; bool simplify; {
	relation n1 = numcomp(n, zero);
	integer p, q;
	integer t1, t2;
	value k;

	if (n1 >= 0) {
		p = Integral(v) ? Copy(v) : Copy(Numerator((rational) v));
		q = Integral(v) ? int_1   : Copy(Denominator((rational) v));
		n = copy(n);
	}
	else {
		p = Integral(v) ? int_1   : Copy(Denominator((rational) v));
		q = Integral(v) ? Copy(v) : Copy(Numerator((rational) v));
		n = negated(n);
	}
	
	if (simplify) {
		while (n != zero && Even(Lsd(q)) && !Interrupted()) {
			q = int_half(q);
			n = diff(k = n, one);
			release(k);
		}
	}
	
	if (n != zero) {
		t1 = (integer) power((value) int_2, n);
		p = int_prod(t2 = p, t1);
		Release(t1); Release(t2);
	}
	release(n);
	
	if (n1 >= 0 && q == int_1)
		return (value) p;
	else if (n1 < 0 && p == int_1)
		return (value) q;
	else {
		rational r = mk_rat(n1>=0 ? p : q, n1>=0 ? q : p, 0, No);
		Release(p); Release(q);
		return (value) r;
	}
}

/* v is shifted n "digits" to the left */

Hidden integer int10shift(v, n) integer v; intlet n; {
	struct integer vv;
	integer w;
	int i;
	
	if (n == 0)
		return Copy(v);
	FreezeSmallInt(v, vv);
	w = (integer) grab_num(Length(v) + n);
	for (i = 0; i<Length(v); ++i)
		Digit(w, i+n) = Digit(v, i);
	return int_canon(w);
}

/* returns u * 10**|n| */

Hidden integer int10mul(u, n) integer u; int n; {
	integer v, w;

	if (n<0) n = -n;
	v = int10shift(u, n / tenlogBASE);
	w = (integer) tento(n % tenlogBASE);
	u = int_prod(v, w);
	Release(v); Release(w);
	return u;
}

/* prod10n(v,n) returns (v= p/q) * 10**n;
 * to prevent a time consuming multiplication of two possible big
 * numbers, the relevant operand (p if n>0 and q else) is first shifted
 * |n|/tenlogBASE "digits"
 */

Visible value prod10n(v, n, simplify) value v; int n; bool simplify; {
	integer p, q, t;

	v = Approximate(v) ? exactly(v) : copy(v);

	if (Integral(v)) {
		p = Copy(v);
		q = int_1;
	}
	else {
		p = Copy(Numerator((rational) v));
		q = Copy(Denominator((rational) v));
	}
	if (n > 0) {
		p = int10mul(t = p, n);
		Release(t);
	}
	else if (n < 0) {
		q = int10mul(t = q, -n);
		Release(t);
	}
	release(v);

	if (q == int_1)
		return (value) p;
	else {
		rational r = mk_rat(p, q, 0, simplify);
		Release(p); Release(q);
		return (value) r;
	}
}

/* returns u+0.5 not simplified */

Visible rational ratsumhalf(u) rational u; {
	integer p, q;
	rational s;

	p = int_prod(Numerator(u), int_2);
	p = int_sum(q = p, Denominator(u)); Release(q);
	q = int_prod(Denominator(u), int_2);

	s = mk_rat(p, q, 0, No);
		/* roundsize not used, so 0 is sufficient */
	Release(p); Release(q);
	return s;
}
