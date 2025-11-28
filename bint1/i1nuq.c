/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1986. */

#include "b.h"
#include "bobj.h"
#include "i1num.h"


/* Product of integer and single "digit" */

Visible integer int1mul(v, n1) integer v; digit n1; {
	integer a;
	digit save, bigcarry, carry = 0;
	twodigit z, zz, n = n1;
	register int i;
	struct integer vv;

	FreezeSmallInt(v, vv);

	a = (integer) grab_num(Length(v)+2);

	for (i = 0; i < Length(v); ++i) {
		z = Digit(v,i) * n;
		bigcarry = zz = (z/ABCBASE);
		carry += z - zz*ABCBASE;
		Digit(a,i) = save = Modulo(carry, ABCBASE);
		carry = (carry-save)/ABCBASE + bigcarry;
	}

	Digit(a,i) = save = Modulo(carry, ABCBASE);
	Digit(a,i+1) = (carry-save)/ABCBASE;

	return int_canon(a);
}


/* Quotient of positive integer and single "digit" > 0 */

Hidden integer int1div(v, n1, prem) integer v; digit n1, *prem; {
	integer q;
	twodigit r_over_n, r = 0, n = n1;
	register int i;
	struct integer vv;

	FreezeSmallInt(v, vv);

	q = (integer) grab_num(Length(v));
	for (i = Length(v)-1; i >= 0; --i) {
		r = r*ABCBASE + Digit(v,i);
		Digit(q,i) = r_over_n = (r/n);
		r -= r_over_n * n;
	}
	if (prem)
		*prem = r;
	return int_canon(q);
}


/* Long division routine, gives access to division algorithm. */

Visible digit int_ldiv(v1, w1, pquot, prem) integer v1, w1, *pquot, *prem; {
	integer a;
	int sign = 1, rel_v = 0, rel_w = 0;
	digit div, rem;
	struct integer vv1, ww1;

	if (w1 == int_0) syserr(MESS(1100, "zero division (int_ldiv)"));

	/* Make v, w positive */
	if (Msd(v1) < 0) {
		sign = -1;
		++rel_v;
		v1 = int_neg(v1);
	}

	if (Msd(w1) < 0) {
		sign *= -1;
		++rel_w;
		w1 = int_neg(w1);
	}
	
	FreezeSmallInt(v1, vv1);
	FreezeSmallInt(w1, ww1);

	div = sign;

	/* Check v << w or single-digit w */
	if (Length(v1) < Length(w1)
		|| Length(v1) == Length(w1)
			&& Digit(v1, Length(v1)-1) < Digit(w1, Length(w1)-1)) {
		a = int_0;
		if (prem) {
			if (v1 == &vv1) *prem= (integer) MkSmallInt(Digit(v1,0));
			else *prem = (integer) Copy(v1);
		}
	}
	else if (Length(w1) == 1) {
		/* Single-precision division */
		a = int1div(v1, Digit(w1,0), &rem);
		if (prem) *prem = mk_int((double)rem);
	}
	else {
		/* Multi-precision division */
		/* Cf. Knuth II Sec. 4.3.1. Algorithm D */
		/* Note that we count in the reverse direction (not easier!) */

		twodigit z, zz;
		digit carry, save, bigcarry;
		twodigit q, d = ABCBASE/(Digit(w1, Length(w1)-1)+1);
		register int i, j, k;
		integer v, w;
		digit vj;

		/* Normalize: make Msd(w) >= ABCBASE/2 by multiplying
		   both v and w by d */

		v = int1mul(v1, (digit)d);
			/* v is used as accumulator, must make a copy */
			/* v cannot be int_1 */
			/* (then it would be one of the cases above) */

		if (d == 1) w = (integer) Copy(w1);
		else w = int1mul(w1, (digit)d);

		a = (integer) grab_num(Length(v1)-Length(w)+1);

		/* Division loop */

		for (j = Length(v1), k = Length(a)-1; k >= 0; --j, --k) {
			vj = j >= Length(v) ? 0 : Digit(v,j);

			/* Find trial digit */

			if (vj == Digit(w, Length(w)-1)) q = ABCBASE-1;
			else q =  ((twodigit)vj*ABCBASE + Digit(v,j-1)) /
					Digit(w, Length(w)-1);

			/* Correct trial digit */

			while (Digit(w,Length(w)-2) * q >
				((twodigit)vj*ABCBASE + Digit(v,j-1)
					- q*Digit(w, Length(w)-1)) *ABCBASE + Digit(v,j-2))
				--q;

			/* Subtract q*w from v */

			carry = 0;
			for (i = 0; i < Length(w) && i+k < Length(v); ++i) {
				z = Digit(w,i) * q;
				bigcarry = zz = (z/ABCBASE);
				carry += Digit(v,i+k) - z + zz*ABCBASE;
				Digit(v,i+k) =
					save = Modulo(carry, ABCBASE);
				carry = (carry-save)/ABCBASE - bigcarry;
			}

			if (i+k < Length(v))
				carry += Digit(v, i+k), Digit(v, i+k) = 0;

			/* Add back necessary? */

				/* It is very difficult to find test cases
				   where add back is necessary if ABCBASE is large.
				   Thanks to Arjen Lenstra, we have v=n*n-1, w=n,
				   where n = 8109636009903000000 (the last six
				   digits are not important). */

			if (carry == 0)		/* No */
				Digit(a,k) = q;
			else {		/* Yes, add back */
				if (carry != -1) syserr(MESS(1101, "int_ldiv internal failure"));
				Digit(a,k) = q-1;
				carry = 0;
				for (i = 0; i < Length(w) && i+k < Length(v); ++i) {
					carry += Digit(v, i+k) + Digit(w,i);
					Digit(v,i+k) =
						save = Modulo(carry, ABCBASE);
					carry = (carry-save)/ABCBASE;
				}
			}
		}	/* End for(j) */

		if (prem) *prem = int_canon(v);	/* Store remainder */
		else Release(v);
		div = sign*d;	/* Store normalization factor */
		Release(w);
		a = int_canon(a);
	}

	if (rel_v) Release(v1);
	if (rel_w) Release(w1);

	if (sign < 0) {
		integer temp = a;
		a = int_neg(a);
		Release(temp);
	}

	if (pquot) *pquot = a;
	else Release(a);
	return div;
}


Visible integer int_quot(v, w) integer v, w; {
	integer quo;
	VOID int_ldiv(v, w, &quo, (integer*)0);
	return quo;
}

Visible integer int_mod(v, w) integer v, w; {
	integer rem;
	digit div;
	bool flag;
	div = int_ldiv(v, w, (integer*)0, &rem); /* Rem. is always positive */
	if (rem == int_0)
		return rem; /* v mod w = 0 */
	flag = (div < 0);
	if (flag || Msd(w) < 0) div = -div;
	if (div != 1) {	/* Divide by div to get proper remainder back */
		v = int1div(rem, div, (digit*)0);
		Release(rem);
		rem = v;
	}
	if (flag) { /* Make same sign as w */
		if (Msd(w) < 0) v = int_sum(w, rem);
		else v = int_diff(w, rem);
		Release(rem);
		rem = v;
	}
	return rem;
}
