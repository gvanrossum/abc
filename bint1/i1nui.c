/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1986. */

/* Multi-precision integer arithmetic */

#include "b.h"
#include "bobj.h"
#include "i1num.h"

/*
 * Number representation:
 * ======================
 *
 * (Think of ABCBASE = 10 for ordinary decimal notation.)
 * A number is a sequence of N "digits" b1, b2, ..., bN
 * where each bi is in {0..ABCBASE-1}, except for negative numbers,
 * where bN = -1.
 * The number represented by b1, ..., bN is
 *      b1*ABCBASE**(N-1) + b2*ABCBASE**(N-2) + ... + bN .
 * The base ABCBASE is chosen so that multiplication of two positive
 * integers up to ABCBASE-1 can be multiplied exactly using long arithmetic.
 * Also it must be possible to add two long integers between
 * -ABCBASE and +ABCBASE (exclusive), giving a result between -2*ABCBASE and
 * +2*ABCBASE.
 * ABCBASE must be even (so we can easily decide whether the whole
 * number is even), and positive (to avoid all kinds of other trouble).
 * Presently, it is restricted to a power of 10 by the I/O-conversion
 * routines (file "i1nuc.c").
 *
 * Canonical representation:
 * bN is never zero (for the number zero itself, N is zero).
 * If bN is -1, b[N-1] is never ABCABCBASE-1 .
 * All operands are assumed to be in canonical representation.
 * Routine "int_canon" brings a number in canonical representation.
 *
 * Mapping to C objects:
 * A "digit" is an integer of type "digit", probably an "int".
 * A number is represented as a "B-integer", i.e. something
 * of type "integer" (which is actually a pointer to some struct).
 * The number of digits N is extracted through the macro Length(v).
 * The i-th digit is extracted through the macro Digit(v,N-i).
 * (So in C, we count in a backwards direction from 0 ... n-1 !)
 * A number is created through a call to grab_num(N), which sets
 * N zero digits (thus not in canonical form!).
 */


/*
 * Bring an integer into canonical form.
 * Make a SmallInt if at all possible.
 */

Visible integer int_canon(v) integer v; {
	register int i;

	if (IsSmallInt(v)) return v;

	for (i = Length(v) - 1; i >= 0 && Digit(v,i) == 0; --i)
		;

	if (i < 0) {
		Release(v);
		return int_0;
	}

	if (i == 0) {
		digit dig = Digit(v,0);
		Release(v);
		return (integer) MkSmallInt(dig);
	}

	/* i > 0 */
	if (Digit(v,i) == -1) {
		while (i > 0 && Digit(v, i-1) == ABCBASE-1) --i;
		if (i == 0) {
			Release(v);
			return int_min1;
		}
		if (i == 1) {
			digit dig = Digit(v,0) - ABCBASE;
			Release(v);
			return (integer) MkSmallInt(dig);
		}
		Digit(v,i) = -1;
	}
	else if (Digit(v, i) < -1) {
		/* e.g. after -100 * 10**7, with ABCBASE == 10**4 */
		++i;
		if (i+1 != Length(v))
			v = (integer) regrab_num((value) v, i+1);
		Digit(v, i) = -1;
		Digit(v, i-1) += ABCBASE;
		/* note: i>=2 && Digit(v, i-1) != ABCBASE-1 */
	}

	if (i+1 < Length(v)) return (integer) regrab_num((value) v, i+1);

	return v;
}


/* General add/subtract subroutine */

Hidden twodigit fmodulo(x) twodigit x; {
	/* RETURN x - (ABCBASE * floor(x/ABCBASE)) */
	twodigit d= x/ABCBASE;
	/* next one remedies if negative x/ABCBASE rounds towards 0 */
	if (x < 0 && d*ABCBASE > x) --d;
	return x - ABCBASE*d;
}

Hidden Procedure dig_gadd(to, nto, from, nfrom, ffactor)
	digit *to, *from; intlet nto, nfrom; digit ffactor; {
	twodigit carry= 0;
	twodigit factor= ffactor;
	digit save;

	nto -= nfrom;
	if (nto < 0)
		syserr(MESS(1000, "dig_gadd: nto < nfrom"));
	for (; nfrom > 0; ++to, ++from, --nfrom) {
		carry += *to + *from * factor;
		*to= save= fmodulo(carry);
		carry= (carry-save) / ABCBASE;
	}
	for (; nto > 0; ++to, --nto) {
		if (carry == 0)
			return;
		carry += *to;
		*to= save= fmodulo(carry);
		carry= (carry-save) / ABCBASE;
	}
	if (carry != 0)
		to[-1] += carry*ABCBASE;
		/* Mostly -1, but it can be <-1,
		 * e.g. after -100*10**7 with ABCBASE == 10**4
		 */
}


/* Sum or difference of two integers */
/* Should have its own version of dig-gadd without double precision */

Visible integer int_gadd(v, w, factor) integer v, w; intlet factor; {
	struct integer vv, ww;
	integer s;
	int len, lenv, i;

	FreezeSmallInt(v, vv);
	FreezeSmallInt(w, ww);
	lenv= len= Length(v);
	if (Length(w) > len)
		len= Length(w);
	++len;
	s= (integer) grab_num(len);
	for (i= 0; i < lenv; ++i)
		Digit(s, i)= Digit(v, i);
	for (; i < len; ++i)
		Digit(s, i)= 0;
	dig_gadd(&Digit(s, 0), len, &Digit(w, 0), Length(w), (digit)factor);
	return int_canon(s);
}

/* Sum of two integers */

Visible integer int_sum(v, w) integer v, w; {
	if (IsSmallInt(v) && IsSmallInt(w))
		return mk_int((double)SmallIntVal(v) + (double)SmallIntVal(w));
	return int_gadd(v, w, 1);
}

/* Difference of two integers */

Visible integer int_diff(v, w) integer v, w; {
	if (IsSmallInt(v) && IsSmallInt(w))
		return mk_int((double)SmallIntVal(v) - (double)SmallIntVal(w));
	return int_gadd(v, w, -1);
}

/* Product of two integers */

Visible integer int_prod(v, w) integer v, w; {
	int i;
	integer a;
	struct integer vv, ww;

	if (v == int_0 || w == int_0) return int_0;
	if (v == int_1) return (integer) Copy(w);
	if (w == int_1) return (integer) Copy(v);

	if (IsSmallInt(v) && IsSmallInt(w))
		return mk_int((double)SmallIntVal(v) * (double)SmallIntVal(w));
	FreezeSmallInt(v, vv);
	FreezeSmallInt(w, ww);

	a = (integer) grab_num(Length(v) + Length(w));

	for (i= Length(a)-1; i >= 0; --i)
		Digit(a, i)= 0;
	for (i = 0; i < Length(v) && !Interrupted(); ++i)
		dig_gadd(&Digit(a, i), Length(w)+1, &Digit(w, 0), Length(w), 
			Digit(v, i));
	return int_canon(a);
}

Visible integer int_neg(u) integer u; {
	if (IsSmallInt(u))
		return mk_int((double) (-SmallIntVal(u)));
	return int_gadd(int_0, u, -1);
}

/* Compare two integers */

Visible relation int_comp(v, w) integer v, w; {
	int sv, sw;
	register int i;
	struct integer vv, ww;

	/* 1. Compare pointers and equal SmallInts */
	if (v == w) return 0;

	/* 1a. Handle SmallInts */
	if (IsSmallInt(v) && IsSmallInt(w))
		return SmallIntVal(v) - SmallIntVal(w);
	FreezeSmallInt(v, vv);
	FreezeSmallInt(w, ww);

	/* 2. Extract signs */
	sv = Length(v)==0 ? 0 : Digit(v,Length(v)-1)<0 ? -1 : 1;
	sw = Length(w)==0 ? 0 : Digit(w,Length(w)-1)<0 ? -1 : 1;

	/* 3. Compare signs */
	if (sv != sw) return (sv>sw) - (sv<sw);

	/* 4. Compare sizes */
	if (Length(v) != Length(w))
		return sv * ( (Length(v)>Length(w)) - (Length(v)<Length(w)) );

	/* 5. Compare individual digits */
	for (i = Length(v)-1; i >= 0 && Digit(v,i) == Digit(w,i); --i)
		;

	/* 6. All digits equal? */
	if (i < 0) return 0;  /* Yes */

	/* 7. Compare leftmost different digits */
	if (Digit(v,i) < Digit(w,i)) return -1;

	return 1;
}


/* Construct an integer out of a floating point number */

#define GRAN 8	/* Granularity used when requesting more storage */
		/* MOVE TO MEM! */
Visible integer mk_int(x) double x; {
	register integer a;
	integer b;
	register int i, j;
	int negate;

	if (MinSmallInt <= x && x <= MaxSmallInt)
		return (integer) MkSmallInt((int)x);

	a = (integer) grab_num(1);
	negate = x < 0 ? 1 : 0;
	if (negate) x = -x;

	for (i = 0; x != 0; ++i) {
		double z = floor(x/ABCBASE);
		double y = z*ABCBASE;
		digit save = Modulo((int)(x-y), ABCBASE);
		if (i >= Length(a)) {
			a = (integer) regrab_num((value) a, Length(a)+GRAN);
			for (j = Length(a)-1; j > i; --j)
				Digit(a,j) = 0;	/* clear higher digits */
		}
		Digit(a,i) = save;
		x = floor((x-save)/ABCBASE);
	}

	if (negate) {
		b = int_neg(a);
		Release(a);
		return b;
	}

	return int_canon(a);
}

/* Construct an integer out of a C int.  Like mk_int, but optimized. */

Visible value mk_integer(x) int x; {
	if (MinSmallInt <= x && x <= MaxSmallInt) return MkSmallInt(x);
	return (value) mk_int((double)x);
}


/* Efficiently compute 10**n as a B integer, where n is a C int >= 0 */

Visible integer int_tento(n) int n; {
	integer i;
	digit msd = 1;
	if (n < 0) syserr(MESS(1001, "int_tento(-n)"));
	if (n < tenlogBASE) {
		while (n != 0) msd *= 10, --n;
		return (integer) MkSmallInt(msd);
	}
	i = (integer) grab_num(1 + (int)(n/tenlogBASE));
	if (i) {
		n %= tenlogBASE;
		while (n != 0) msd *= 10, --n;
		Digit(i, Length(i)-1) = msd;
	}
	/* else caveat invocator */
	return i;
}
