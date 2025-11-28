/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1986. */

/* Approximate arithmetic */

#include "b.h"
#include "bobj.h"
#include "i0err.h"
#include "i1num.h"

/*
 * For various reasons, on some machines (notably the VAX), the range
 * of the exponent is too small (ca. 1.7E38), and we cope with this by
 * adding a second word which holds the exponent.
 * However, on other machines (notably the IBM PC), the range is sufficient
 * (ca. 1E300), and here we try to save as much code as possible by not
 * doing our own exponent handling.  (To be fair, we also don't check
 * certain error conditions, to save more code.)
 * The difference is made by #defining EXT_RANGE (in i1num.h), meaning we
 * have to EXTend the RANGE of the exponent.
 */

#ifdef EXT_RANGE

Hidden struct real app_0_buf = {Num, 1, -1, FILLER  0.0, -BIG};
	/* Exponent must be less than any realistic exponent! */

#else /* !EXT_RANGE */

Hidden struct real app_0_buf = {Num, 1, -1, FILLER  0.0};

#endif /* !EXT_RANGE */

Visible real app_0 = &app_0_buf;

Hidden double logtwo;
Hidden double twologBASE;

/* we have come across several frexp() functions, for which 
 * frexp(0, &e) == 0 does not apply.
 * So here is our own version.
 */

Hidden double b_frexp(frac, ep)
     double frac;
     expint *ep;
{
	if (frac == 0.0) {
		*ep = 0;
		return 0.0;
	}
	else return frexp(frac, ep);
}

/*
 * Build an approximate number.
 */

#define TOO_LARGE MESS(700, "approximate number too large")

Visible real mk_approx(frac, expo) double frac, expo; {
	real u;
#ifdef EXT_RANGE
	expint v;
	if (frac != 0) frac = b_frexp(frac, &v), expo += v;
	if (frac == 0 || expo < -BIG) return (real) Copy(app_0);
	if (expo > BIG) {
		interr(TOO_LARGE);
		expo = BIG;
	}
#else /* !EXT_RANGE */
	if (frac == 0.0) return (real) Copy(app_0);
	if (frac > 0 && log(frac)+expo*logtwo > log(Maxreal)) {
		interr(TOO_LARGE);
		frac= Maxreal;
	}
	else
		frac= ldexp(frac, (int)expo);
#endif /* EXT_RANGE */
	u = (real) grab_num(-1);
	Frac(u) = frac;
#ifdef EXT_RANGE
	Expo(u) = expo;
#endif /* EXT_RANGE */
	return u;
}

Hidden value twotodblbits; /* 2**DBLBITS */
Hidden value twoto_dblbitsmin1; /* 2**(DBLBITS-1) */
	/* stored as an unnormalized rational */
	
Hidden double getexponent(v) value v; {
	integer p, q;
	struct integer pp, qq;
	double x;

	v = absval(v);
	if (Integral(v)) {
		p = (integer) v;
		q = (integer) one;
	}
	else {
		p = Numerator((rational) v);
		q = Denominator((rational) v);
	}
	FreezeSmallInt(p, pp); FreezeSmallInt(q, qq);

	x = log((double) Msd(p)) / logtwo;
	x-= log((double) Msd(q)) / logtwo;
	x+= (double) ((Length(p)-Length(q)) * twologBASE);

	release(v);
	return floor(x) + 1;
}

Visible value app_frexp(v) value v; {
	integer w;
	struct integer ww;
	value s, t;
	double frac, expo;
	relation neg;
	int i;

	if ((neg = numcomp(v, zero)) == 0)
		return Copy(app_0);
	else if (neg < 0)
		v = negated(v);

	expo = getexponent(v); /* it can be +1 or -1 off !!! */

	s = (value) mk_int((double)DBLBITS - expo);
	s = prod2n(v, t = s, No);
	release(t);
	/* do the correction */
	if (numcomp(s, twotodblbits) >= 0) {
		s = prod2n(t = s, (value) int_min1, No); /* s / 2 */
		++expo;
		release(t);
	}
	else if (numcomp(s, twoto_dblbitsmin1) < 0) {
		s = prod2n(t = s, (value) int_1, No); /* s * 2 */
		--expo;
		release(t);
	}
	w = (integer) round1(s);
	release(s);
	FreezeSmallInt(w, ww);

	frac = 0.0;
	for (i = Length(w) - 1; i >= 0; --i) {
		frac = frac * ABCBASE + Digit(w, i);
	}
	frac = ldexp(frac, -DBLBITS);

	release((value) w);
	if (neg < 0) {
		frac = -frac;
		release(v);
	}
	return (value) mk_approx(frac, expo);
}

/*
 * Approximate arithmetic.
 */

Visible real app_sum(u, v) real u, v; {
#ifdef EXT_RANGE
	real w;
	if (Expo(u) < Expo(v)) w = u, u = v, v = w;
	if (Expo(v) - Expo(u) < Minexpo) return (real) Copy(u);
	return mk_approx(Frac(u) + ldexp(Frac(v), (int)(Expo(v) - Expo(u))),
		Expo(u));
#else /* !EXT_RANGE */
	return mk_approx(Frac(u) + Frac(v), 0.0);
#endif /* !EXT_RANGE */
}

Visible real app_diff(u, v) real u, v; {
#ifdef EXT_RANGE
	real w;
	int sign = 1;
	if (Expo(u) < Expo(v)) w = u, u = v, v = w, sign = -1;
	if (Expo(v) - Expo(u) < Minexpo)
		return sign < 0 ? app_neg(u) : (real) Copy(u);
	return mk_approx(
		sign * (Frac(u) - ldexp(Frac(v), (int)(Expo(v) - Expo(u)))),
		Expo(u));
#else /* !EXT_RANGE */
	return mk_approx(Frac(u) - Frac(v), 0.0);
#endif /* !EXT_RANGE */
}

Visible real app_neg(u) real u; {
	return mk_approx(-Frac(u), Expo(u));
}

Visible real app_prod(u, v) real u, v; {
	return mk_approx(Frac(u) * Frac(v), Expo(u) + Expo(v));
}

Visible real app_quot(u, v) real u, v; {
	if (Frac(v) == 0.0) {
		interr(ZERO_DIVIDE);
		return (real) Copy(u);
	}
	return mk_approx(Frac(u) / Frac(v), Expo(u) - Expo(v));
}

/*
	YIELD log"(frac, expo):
		CHECK frac > 0
		RETURN normalize"(expo*logtwo + log(frac), 0)
*/

Visible real app_log(v) real v; {
	double frac, expo;

	if (!still_ok) return (real) Copy(app_0);
	frac = Frac(v), expo = Expo(v);
 	return mk_approx(expo*logtwo + log(frac), 0.0);
}

/*
	YIELD exp"(frac, expo):
		IF expo < minexpo: RETURN zero"
		WHILE expo < 0: PUT frac/2, expo+1 IN frac, expo
		PUT exp frac IN f
		PUT normalize"(f, 0) IN f, e
		WHILE expo > 0:
			PUT (f, e) prod" (f, e) IN f, e
			PUT expo-1 IN expo
		RETURN f, e
*/

Visible real app_exp(v) real v; {
#ifdef EXT_RANGE
	expint ei;
	double frac = Frac(v), vexpo = Expo(v), new_expo;
	static double canexp;
	if (!canexp)
		canexp = floor(log(log(Maxreal/2.718281828459045235360)+1.0)/logtwo);
	if (vexpo <= canexp) {
		if (vexpo < Minexpo) return mk_approx(1.0, 0.0);
		frac = ldexp(frac, (int)vexpo);
		vexpo = 0;
	}
	else if (vexpo >= Maxexpo) {
		/* Definitely too big (the real boundary is much smaller
		   but here we are in danger of overflowing new_expo
		   in the loop below) */
		if (frac < 0)
			return (real) Copy(app_0);
		return mk_approx(1.0, Maxreal); /* Force an error! */
	}
	else {
		frac = ldexp(frac, (int)canexp);
		vexpo -= canexp;
	}
	frac = exp(frac);
	new_expo = 0;
	while (vexpo > 0 && frac != 0) {
		frac = b_frexp(frac, &ei);
		new_expo += ei;
		frac *= frac;
		new_expo += new_expo;
		--vexpo;
	}
	return mk_approx(frac, new_expo);
#else /* !EXT_RANGE */
	if (Frac(v) > (Maxexpo)*logtwo)
		return mk_approx(1.0, Maxreal); 
		/* Force error! 
		 * (since BSD exp generates illegal instr) 
		 * [still ~2**126 ain't save against their failing exp] */
	return mk_approx(exp(Frac(v)), 0.0);
#endif /* !EXT_RANGE */
}

Visible real app_power(u, v) real u, v; {
	double ufrac = Frac(u);
	if (ufrac <= 0) {
		if (ufrac < 0) interr(NEG_EXACT);
		if (v == app_0) return mk_approx(1.0, 0.0); /* 0**0 = 1 */
		return (real) Copy(app_0); /* 0**x = 0 */
	}
	else {
		/* u ** v = exp(v * log (u)) */
		real logu= app_log(u);
		real vlogu= app_prod(v, logu);
		real expvlogu= app_exp(vlogu);
		Release(logu);
		Release(vlogu);
		return expvlogu;
	}
}

/* about2_to_integral(ru, v, rv) returns, via rv, exactly (0.5, v+1)
 * if ru == ~2 and v is an integral. Why?, well,
 * to speed up reading the value of an approximate from a file,
 * the exponent part is stored as ~2**expo and
 * to prevent loss of precision, we cannot use the normal procedure
 * app_power().
 */

Visible bool about2_to_integral(ru, v, rv) value v; real ru, *rv; {
	double expo;
	integer w;
	struct integer ww;
	int i;
	bool neg = No;

#ifdef EXT_RANGE
	if (!(Frac(ru) == 0.5 && Expo(ru) == 2.0 && Integral(v)))
		return No;
#else
	if (!(Frac(ru) == 2.0 && Integral(v)))
		return No;
#endif
	w = (integer) v;
	if (numcomp((value) w, zero) < 0) {
		w = int_neg(w);
		neg = Yes;
	}
	FreezeSmallInt(w, ww);
	
	expo = 0.0;
	for (i = Length(w) - 1; i >= 0; --i) {
		expo = expo * ABCBASE + Digit(w, i);
	}
	if (neg) {
		expo = -expo;
		Release(w);
	}
	*rv = mk_approx(0.5, expo+1);
	return Yes;
}

Visible int app_comp(u, v) real u, v; {
	double xu, xv;
#ifdef EXT_RANGE
	double eu, ev;
#endif /* EXT_RANGE */
	if (u == v) return 0;
	xu = Frac(u), xv = Frac(v);
#ifdef EXT_RANGE
	if (xu*xv > 0) {
		eu = Expo(u), ev = Expo(v);
		if (eu < ev) return xu < 0 ? 1 : -1;
		if (eu > ev) return xu < 0 ? -1 : 1;
	}
#endif /* EXT_RANGE */
	if (xu < xv) return -1;
	if (xu > xv) return 1;
	return 0;
}

Visible integer app_floor(u) real u; {
	double frac, expo;
	expint ei;
	integer v, w;
	value twotow, result;
	
	frac= Frac(u);
	expo= Expo(u);
	frac= b_frexp(frac, &ei);
	expo+= ei;

	if (expo <= DBLBITS) {
		return 	mk_int(floor(ldexp(frac,
				(int)(expo < 0 ? -1 : expo))));
	}
	v = mk_int(ldexp(frac, DBLBITS));
	w = mk_int(expo - DBLBITS);
	twotow = power((value)int_2, (value)w);
	result = prod((value)v, twotow);
	Release(v), Release(w), Release(twotow);
	if (!Integral(result)) 
		syserr(MESS(701, "app_floor: result not integral"));
	return (integer) result;
}

Hidden value twotolongbits;

Visible value app_exactly(u) real u; {
	value w;
	integer v, n, t1, t2;
	double frac, expo, rest, p;
	unsigned long l;
	expint e, re, dummy;
	int z, digits;
	bool neg;
	
	if (Frac(u) == 0.0)
		return zero;
	frac= Frac(u);
	expo= Expo(u);
	if (frac < 0.0) { frac= -frac; neg= Yes; }
	else neg= No;
	frac= b_frexp(frac, &e);
	expo+= e;
	p= floor(ldexp(frac, LONGBITS));	/* shift the digits */
	l= (unsigned long) p;
	v= mk_int((double) l);
	rest= b_frexp(frac - b_frexp(p, &dummy), &re);
	z= -re - LONGBITS;		/* number of leading zeros */
	digits= LONGBITS;		/* count the number of digits */

	while (rest != 0.0) {
		p= floor(ldexp(rest, LONGBITS - z));
		l= (unsigned long) p;
		v= int_prod(t1= v, (integer) twotolongbits);
		Release(t1);
		v= int_sum(t1= v, t2= mk_int((double) l));
		Release(t1); Release(t2);
		rest= b_frexp(rest - b_frexp(p, &dummy), &re);
		z= z - re - LONGBITS;
		digits+= LONGBITS;
	}
	if (neg) {
		v= int_neg(t1= v);
		Release(t1);
	}
	n= mk_int(expo - (double) digits);
	w= prod2n((value) v, (value) n, Yes);
	Release(v); Release(n);

	return w;
}

/*
 * app_print(f, v) writes an approximate v on file f in such a way that it
 * can be read back identically, assuming integral powers of ~2 can be
 * computed exactly. To ensure this we have incorporated a test in the
 * routine power().
 */

Visible Procedure app_print(fp, v) FILE *fp; real v; {
	double frac= Frac(v);
	double expo= Expo(v);
	expint ei;
	integer w;
	string str;
	
	frac = b_frexp(frac, &ei);
	expo += ei;

	if (frac == 0.0) {
		fputs("~0", fp);
		return;
	}
	if (frac < 0) {
		frac = -frac;
		putc('-', fp);
	}
	if (frac == 0.5)
		fprintf(fp, "~2**%.0lf", expo-1);
	else {
		w = mk_int(ldexp(frac, DBLBITS));
		expo -= DBLBITS;
		str = convnum((value) w);
		fprintf(fp, "%s*~2**%.0lf", str, expo);
		Release(w);
	}
}

Hidden Procedure initlog() {
	double logBASE, invlogtwo;

	logtwo= log(2.0);

	logBASE= log(10.0) * tenlogBASE;
	invlogtwo= 1.0 / logtwo;
	twologBASE= logBASE * invlogtwo;
}

Visible Procedure initapp() {
	value v;
	rational r;

	initlog();

	twotolongbits= (value) mk_int((double) TWOTO_LONGBITS);

	v = (value) mk_int((double) TWOTO_DBLBITSMIN1);
	twotodblbits= prod(v, (value) int_2);
	release(v);

	/* to save space, twoto_dblbitsmin1 is stored as 
	 * an unnormalized rational.
	 */
	r = (rational) grab_rat(0);
	Numerator(r) = (integer) copy(twotodblbits);
	Denominator(r) = int_2;
	twoto_dblbitsmin1= (value) r;
}

Visible Procedure endapp() {
#ifdef MEMTRACE
	release(twoto_dblbitsmin1);
	release(twotodblbits);
	release(twotolongbits);
#endif
}
