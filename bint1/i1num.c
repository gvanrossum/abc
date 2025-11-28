/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1986. */

/* B numbers, basic external interface */

#include "b.h"
#include "bobj.h"
#include "i1num.h"

/*
 * This file contains operations on numbers that are not predefined
 * B functions (the latter are defined in `i1fun.c').
 * This includes conversion of numeric B values to C `int's and
 * `double's, (numval() and intval()),
 * but also utilities for comparing numeric values and hashing a numeric
 * value to something usable as initialization for the random generator
 * without chance of overflow (so numval(v) is unusable).
 * It is also possible to build numbers of all types using mk_integer,
 * mk_exact (or mk_rat) and mk_approx.  Note the rather irregular
 * (historical!) argument structure for these: mk_approx has a
 * `double' argument, where mk_exact and mk_rat have two values
 * which must be `integer' (not `int's!) as arguments.
 * The random number generator used by the random and choice functions
 * is also in this file.
 */

/*
 * ival is used internally by intval() and large().
 * It converts an integer to double precision, yielding
 * a huge value when overflow occurs (but giving no error).
 */

Hidden double ival(u) integer u; {
	double x = 0;
	register int i;

	if (IsSmallInt(u)) return SmallIntVal(u);
	for (i = Length(u)-1; i >= 0; --i) {
		if (x >= Maxreal/ABCBASE)
			return Msd(u) < 0 ? -Maxreal : Maxreal;
		x = x*ABCBASE + Digit(u, i);
	}

	return x;
}


/* Determine if a value may be converted to an int */

#define NO_INTEGER	MESS(1300, "number not an integer")

Visible bool large(v) value v; {
	double r;
	if (!Is_number(v) || !integral(v)) {
		interr(NO_INTEGER);
		return No;
	}
	if (Rational(v)) v= (value) Numerator((rational)v);
	r= ival((integer)v);
	if (r > Maxint) return Yes;
	return No;
}

/* return the C `int' value of a B numeric value, if it exists. */

#define LARGE_INTEGER	MESS(1301, "exceedingly large integer")

Visible int intval(v) value v; {
	/* v must be an Integral number or a Rational with Denominator==1
	    which may result from n round x [via mk_exact]!. */
	double i;
	if (IsSmallInt(v)) i= SmallIntVal(v);
	else {
		if (!Is_number(v)) syserr(MESS(1302, "intval on non-number"));
		if (!integral(v)) {
			interr(NO_INTEGER);
			return 0;
		}
		if (Rational(v)) v= (value) Numerator((rational)v);
		i= ival((integer)v);
	}
	if (i > Maxint || i < -Maxint) {
		interr(LARGE_INTEGER);
		return 0;
	}
	return (int) i;
}


/* convert an int to an intlet */

Visible int propintlet(i) int i; {
	if (i > Maxintlet || i < -Maxintlet) {
		interr(LARGE_INTEGER);
		return 0;
	}
	return i;
}


/*
 * determine if a number is integer
 */

Visible bool integral(v) value v; {
	if (Integral(v) || (Rational(v) && Denominator((rational)v) == int_1))
		return Yes;
	else return No;
}

/*
 * mk_exact makes an exact number out of two integers.
 * The third argument is the desired number of digits after the decimal
 * point when the number is printed (see comments in `i1fun.c' for
 * `round' and code in `i1nuc.c').
 * This printing size (if positive) is hidden in the otherwise nearly
 * unused * 'Size' field of the value struct
 * (cf. definition of Rational(v) etc.).
 */

Visible value mk_exact(p, q, len) integer p, q; int len; {
	rational r = mk_rat(p, q, len, Yes);

	if (Denominator(r) == int_1 && len <= 0) {
		integer i = (integer) Copy(Numerator(r));
		Release(r);
		return (value) i;
	}

	return (value) r;
}

#define uSMALL 1
#define uINT 2
#define uRAT 4
#define uAPP 8
#define vSMALL 16
#define vINT 32
#define vRAT 64
#define vAPP 128

Visible relation numcomp(u, v) value u, v; {
	int tu, tv; relation s;

	if (IsSmallInt(u)) tu = uSMALL;
	else if (Length(u) >= 0) tu = uINT;
	else if (Length(u) <= -2) tu = uRAT;
	else tu = uAPP;
	if (IsSmallInt(v)) tv = vSMALL;
	else if (Length(v) >= 0) tv = vINT;
	else if (Length(v) <= -2) tv = vRAT;
	else tv = vAPP;

	switch (tu|tv) {

	case uSMALL|vSMALL: return SmallIntVal(u) - SmallIntVal(v);

	case uSMALL|vINT:
	case uINT|vSMALL:
	case uINT|vINT: return int_comp((integer)u, (integer)v);

	case uSMALL|vRAT:
	case uINT|vRAT:
		u = (value) mk_rat((integer)u, int_1, 0, Yes);
		s = rat_comp((rational)u, (rational)v);
		release(u);
		return s;

	case uRAT|vRAT:
		return rat_comp((rational)u, (rational)v);

	case uRAT|vSMALL:
	case uRAT|vINT:
		v = (value) mk_rat((integer)v, int_1, 0, Yes);
		s = rat_comp((rational)u, (rational)v);
		release(v);
		return s;

	case uSMALL|vAPP:
		u = approximate(u);
		s = app_comp((real)u, (real)v);
		release(u);
		return s;

	case uINT|vAPP:
	case uRAT|vAPP:
		v = exactly(v);
		s = numcomp(u, v);
		release(v);
		return s;

	case uAPP|vAPP:
		return app_comp((real)u, (real)v);

	case uAPP|vSMALL:
		v = approximate(v);
		s = app_comp((real)u, (real)v);
		release(v);
		return s;

	case uAPP|vINT:
	case uAPP|vRAT:
		u = exactly(u);
		s = numcomp(u, v);
		release(u);
		return s;

	default: syserr(MESS(1303, "num_comp")); /* NOTREACHED */

	}
}

#ifdef RANGEPRINT
/* if ranges are written as {1..10} instead of {1; 2; etc},
 * the following function is used in convert() and wri().
 */

Visible bool is_increment(a, b) value a, b; {
	value v;
	relation c;
	
	if (!(Is_number(a) && integral(a))) {
		return No;
	}
	c= compare(a, v= sum(b, one));
	release(v);
	return c==0;
}
#endif /* RANGEPRINT */


/*
 * Deliver 10**n, where n is a (maybe negative!) C integer.
 * The result is a value (integer or rational, actually).
 */

Visible value tento(n) int n; {
	/* If int_tento fails, so will tento; caveat invocator */
	if (n < 0) {
		integer i= int_tento(-n); value v;
		if (!i) return Vnil;
		v= (value) mk_exact(int_1, i, 0);
		Release(i);
		return v;
	}
	return (value) int_tento(n);
}


/*
 * numval returns the numerical value of any numeric B value
 * as a C `double'.
 */

Visible double numval(u) value u; {
	double expo, frac;

	if (!Is_number(u)) {
		interr(MESS(1304, "value not a number"));
		return 0.0;
	}
	u = approximate(u);
	expo = Expo((real)u), frac = Frac((real)u);
	release(u);
	if (expo > Maxexpo) {
		interr(MESS(1305, "approximate number too large to be handled"));
		return 0.0;
	}
	if(expo < Minexpo) return 0.0;
	return ldexp(frac, (int)expo);
}


/*
 * Random numbers
 */


/*
 * numhash produces a `double' number that depends on the value of
 * v, useful for initializing the random generator.
 * Needs rewriting, so that for instance numhash(n) never equals n.
 * The magic numbers here are chosen at random.
 */

/* The following is an auxiliary function for scrambling integers. */

Hidden double inthash(v) double v; {
	long i= ((long) v)^0x96696996;
	v= 987.6543210987654321*v;
	return .666*(((long) (v*.543))^((long) v)^i)+.747+v;
}

Visible double numhash(v) value v; {
	if (Integral(v)) {
		double d = 0;
		register int i;

		if (IsSmallInt(v)) return inthash((double)SmallIntVal(v));
		
		for (i = Length(v) - 1; i >= 0; --i) {
			d *= 2;
			d += Digit((integer)v, i);
		}

		return d;
	}

	if (Rational(v))
		return .777 * numhash((value) Numerator((rational)v)) +
		       .123 * numhash((value) Denominator((rational)v));

	return numval(v); /* Which fails for HUGE reals.  Never mind. */
}


/* Initialize the random generator */

Hidden double lastran;

Hidden Procedure setran (seed) double seed; {
	double x;

	/* Here is a change to make SETRANDOM -x differ from SETRANDOM x: */
	x = seed >= 0 ? seed : .775533-seed;
	/* This gives a small speed-up: */
	while (x >= 10000) x /= 10000;
	/* as before: */
	while (x >= 1) x /= 10;
	lastran = floor(67108864.0*x);
}

Visible Procedure set_random(v) value v; {
	setran((double) hash(v));
}


/* Return a random number in [0, 1). */

#define AA 29247341.0
#define CC 66664423.0
#define MM 67108864.0	/* 2**26 */

#define T21 2097152.0	/* 2**21 */

Visible value random() {
	double p, r;

	/* Get three packets of 21 bits.
	 * We get the full width of a double as random bits.
	 * no group of bits has an obvious cyclic pattern,
	 * because we chop off the last 5 bits of each lastran
	 * (the last n bits (of the 26) go thru a 2**n cycle).
	 * Disadvantage: it is slow -- but someone heavy into using
	 * random might prefer a slower good random() over a fast one
	 * giving meaningless data.
	 */
	
	p = AA * lastran + CC;
	lastran = p - MM*floor(p/MM);
	r= floor(lastran/32.0)/T21;

	p = AA * lastran + CC;
	lastran = p - MM*floor(p/MM);
	r= (floor(lastran/32.0)+r)/T21;

	p = AA * lastran + CC;
	lastran = p - MM*floor(p/MM);
	r= (floor(lastran/32.0)+r)/T21;

	if (r >= 1.0) return random();
	return (value) mk_approx(r, 0.0);
}

Visible value rndm_limit;

Visible Procedure initnum() {
	/* cast added to circumvent compiler bug on Alliant: */
	rndm_limit= (value) mk_int((double) RNDM_LIMIT);
	rat_init();
	setran((double) getseed());
	initapp();
}

Visible Procedure endnum() {
	endapp();
	endrat();
}


Visible value grab_num(len) register int len; {
	integer v;
	register int i;

	if (len > Maxintlet) {
		interr(MESS(1306, "exceptionally large number"));
		return Vnil;
	}
	if (len < -Maxintlet) len = -2;
	v = (integer) grab(Num, len);
	for (i = Length(v)-1; i >= 0; --i) Digit(v, i) = 0;
	return (value) v;
}

Visible value grab_rat(len) register int len; {
	if (len > 0 && len+2 <= Maxintlet);
	else len= 0;
		
	return grab(Num, -2 - len);
}

Visible value regrab_num(v, len) value v; register int len; {
	uniql(&v);
	regrab(&v, len);
	return v;
}

Visible unsigned numsyze(len, nptrs)
	intlet len;
	int *nptrs;
{
	register unsigned syze= 0;
	*nptrs= 0;
	if (len >= 0) syze= len*sizeof(digit);	   /* Integral */
	else if (len == -1) {
#ifdef EXT_RANGE
		syze= 2*sizeof(double); 	   /* Approximate */
#else
		syze= sizeof(double);		   /* Approximate */
#endif
	}
	else { 					   /* Rational */
		syze= 2*sizeof(value);
		*nptrs= 2;
	}
	return syze;
}

