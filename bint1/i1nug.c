/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1988. */

#include "b.h"
#include "bobj.h"
#include "i1num.h"


/*
 * Routines for greatest common divisor calculation
 * "Binary gcd algorithm"
 *
 * Assumptions about built-in arithmetic:
 * x>>1 == x/2  (if x >= 0)
 * 1<<k == 2**k (if it fits in a word)
 */

/* Single-precision gcd for integers > 0 */

Hidden digit dig_gcd(u, v) register digit u, v; {
	register digit temp;
	register int k = 0;

	if (u <= 0 || v <= 0) syserr(MESS(900, "dig_gcd of number(s) <= 0"));

	while (Even(u) && Even(v)) ++k, u >>= 1, v >>= 1;

	/* u or v is odd */
	
	while (Even(u)) u >>= 1;

	while (v) {
		/* u is odd */
		
		while (Even(v)) v >>= 1;
		
		/* u and v odd */
		
		if (u > v) { temp = v; v = u - v; u = temp; }
		else v = v - u;
		
		/* u is odd and v even */
	}

	return u * (1<<k);
}

Visible integer int_half(v) integer v; {
	register int i;
	register long carry;

	if (IsSmallInt(v))
		return (integer) MkSmallInt(SmallIntVal(v) / 2);

	if (Msd(v) < 0) {
		i = Length(v)-2;
		if (i < 0) {
			Release(v);
			return int_0;
		}
		carry = ABCBASE;
	}
	else {
		carry = 0;
		i = Length(v)-1;
	}

	if (Refcnt(v) > 1) uniql((value *) &v);

	for (; i >= 0; --i) {
		carry += Digit(v,i);
		Digit(v,i) = carry/2;
		carry = carry&1 ? ABCBASE : 0;
	}

	return int_canon(v);
}

/*
 * u or v is a smallint
 * call int_mod() to make the other smallint too
 * call dig_gcd()
 * multiply with twopow
 */
 
Hidden integer gcd_small(u, v, twopow) integer u, v, twopow; {
	integer g;

	if (!IsSmallInt(u) && !IsSmallInt(v))
		syserr(MESS(901, "gcd_small of numbers > smallint"));

	if (!IsSmallInt(v))
		{ g = u; u = v; v = g; }	
	if (v == int_0)
		g = (integer) Copy(u);
	else if (v == int_1)
		g = int_1;
	else {
		u= IsSmallInt(u) ? (integer) Copy(u) : int_mod(u, v);
		if (u == int_0)
			g = (integer) Copy(v);
		else if (u == int_1)
			g = int_1;
		else  g= (integer) MkSmallInt(
			dig_gcd(SmallIntVal(u), SmallIntVal(v)));
		Release(u);
	}

	g = int_prod(u= g, twopow);
	Release(u);

	if (interrupted && g == int_0)
		{ Release(g); g = int_1; }
	return g;
}

Hidden int lwb_lendiff = (3 / tenlogBASE) + 1;

#define Modgcd(u, v) (Length(u) - Length(v) > lwb_lendiff)

/* Multi-precision gcd of integers > 0 */

Visible integer int_gcd(u1, v1) integer u1, v1; {
	integer t, u, v;
	integer twopow= int_1;
	long k = 0;

	if (Msd(u1) <= 0 || Msd(v1) <= 0)
		syserr(MESS(902, "gcd of number(s) <= 0"));
	
	if (IsSmallInt(u1) || IsSmallInt(v1))
		return gcd_small(u1, v1, int_1);

	u = (integer) Copy(u1);
	v = (integer) Copy(v1);

	if (int_comp(u, v) < 0)
		{ t = u; u = v; v = t; }

	while (Modgcd(u, v)) {
		t = int_mod(u, v); /* u > v > t >= 0 */
		Release(u);
		u = v;
		v = t;
		if (IsSmallInt(v))
			goto smallint;
	}
	

	while (Even(Lsd(u)) && Even(Lsd(v))) {
		u = int_half(u);
		v = int_half(v);
		if (++k < 0) {
			/*It's a number we can't cope with,
			  with too many common factors 2.
			  Though the user can't help it,
			  the least we can do is to allow
			  continuation of the session.
			*/
			interr(MESS(903, "exceptionally large rational number"));
			k = 0;
		}
	}
	
	t= mk_int((double) k);
	twopow= (integer) power((value) int_2, (value) t);
	Release(t);
	
	if (IsSmallInt(v))
		goto smallint;
	
	while (Even(Lsd(u)))
		u = int_half(u);
		
	if (IsSmallInt(u))
		goto smallint;

	/* u is odd */
	
	while (v != int_0) {
		
		while (Even(Lsd(v)))
			v = int_half(v);
			
		if (IsSmallInt(v))
			goto smallint;

		/* u and v are odd */
		
		if (int_comp(u, v) > 0) {
			if (Modgcd(u, v))
				t = int_mod(u, v); /* u>v>t>=0 */
				/* t can be odd */
			else
				t = int_diff(u, v);
				/* t is even */
			Release(u);
			u = v;
			v = t;
		}
		else {
			if (Modgcd(v, u))
				t = int_mod(v, u); /* v>u>t>=0 */
				/* t can be odd */
			else
				t = int_diff(v, u);
				/* t is even */
			Release(v);
			v = t;
		}
		/* u is odd
		 * v can be odd too, but in that case is the new value
		 * smaller than the old one
		 */
	}
			
	Release(v);

	u = int_prod(v = u, twopow);
	Release(v); Release(twopow);

	if (interrupted && u == int_0)
		{ Release(u); u = int_1; }
	return u;

smallint:
	t = gcd_small(u, v, twopow);
	Release(u); Release(v); Release(twopow);
	
	return t;
}
