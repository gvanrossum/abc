/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1986. */

#include "b.h"
#include "bmem.h"
#include "bobj.h"
#include "i1num.h"

#define MAXDIGITS (MAXNUMDIG)
	/* Max precision for non-integral, non-rounded numbers */
#define MAXNUMSIZE (MAXDIGITS+MAXNUMDIG+10)
	/* Maximum width of non-rounded number in convnum;
	 * occurs for e.g. -0.xxxxxxe-yyy:
	 * MAXDIGITS x's and MAXNUMDIG (with EXT_RANGE on) y's 
	 * 10 is a few extra, not a holy number, but guard against evil:-) */


/* Convert an integer to a C character string.
   The character string is overwritten on each next call.
   It assumes BASE is a power of 10. */

Hidden char *convint(v) register integer v; {
	static char *buffer, shortbuffer[tenlogBASE+3];
	static char fmt[10];
	register char *cp;
	register int i;
	bool neg = No;

	if (IsSmallInt(v)) {
		sprintf(shortbuffer, "%d", SmallIntVal(v));
		return shortbuffer;
	}

	if (Digit(v, Length(v)-1) < 0) {
		neg = Yes;
		v = int_neg(v);
	}
	if (buffer) freemem((ptr)buffer);
	buffer = getmem((unsigned)(Length(v)*tenlogBASE + 1 + neg));
	cp = buffer;
	if (neg) *cp++ = '-';
	sprintf(cp, "%d", Msd(v));
	if (!IsSmallInt(v)) {
		if (!*fmt) sprintf(fmt, "%%0%dd", tenlogBASE);
		while (*cp) ++cp;
		for (i = Length(v)-2; i >= 0; --i, cp += tenlogBASE)
			sprintf(cp, fmt, Digit(v, i));
		if (neg) Release(v);
	}
	return buffer;
}

Hidden value tento_d(x) double x; {
	if (x > Maxint || x < -Maxint) {
		value n= (value) mk_int(x);
		value v= power((value) int_10, n);
		release(n);
		return v;
	}
	else return tento((int) x);
}

/* return number of digits before decimal point,
 * or minus the number of zero's after the decimal point
 */

Hidden int digits_in(v) value v; {
	integer p, q;
	struct integer pp, qq;
	double x;
	value t1= Vnil, t2= Vnil;

	if (numcomp(v, zero) == 0)
		return 0;

	v= absval(v);
	if (Integral(v)) {
		p= (integer) v;
		q= (integer) one;
	}
	else {
		p= Numerator((rational) v);
		q= Denominator((rational) v);
	}
	FreezeSmallInt(p, pp); FreezeSmallInt(q, qq);

	x = log10((double) Msd(p));
	x-= log10((double) Msd(q));
	x+= (double) ((Length(p) - Length(q)) * tenlogBASE);
	x= floor(x) + 1;

	/* it can be +1 or -1 off!!! */
	if (numcomp(v, t1 = tento_d(x)) >= 0) /* one too low */
		++x;
	else if (numcomp(v, t2 = tento_d(x-1)) < 0) /* one too high */
		--x;

	release(t1); release(t2);
	release(v);

	if (x > Maxint)
		return Maxint;
	else if (x < -Maxint)
		return -Maxint;
	else
		return (int) x;
}

/* Convert a numeric value to a C character string.
 * The character string is released on each next call.
 *
 * prod10n() is a routine with does a fast multiplication with a ten power
 * and does not normalize a rational result sometimes.
 */

Visible string convnum(v) register value v; {
	value r, re, rre;
	int rndsize= 0;
	int num;
	int ndigits;
	int precision= MAXDIGITS;
	register string txt;
	int txtlen;
	static char *numbuf;
	register char *str;
	bool rem; /* remainder */
	bool rndflag;
	int buflen= MAXNUMSIZE;

	if (Integral(v)) return convint((integer)v);

	/* Aproximates and rationale are treated alike,
	 * using MAXDIGITS precision, and e-notation when
	 * necessary.
	 * However, rationals resulting from 'n round x' are
	 * transformed to f-format, printing n=Roundsize digits
	 * after the decimal point. */

	if (Rational(v) && Roundsize(v) > 0)
		rndsize= Roundsize(v);
	
	r= Approximate(v) ? exactly(v) : copy(v);

	if ((num=numcomp(r, zero)) == 0 && rndsize == 0) {
		release(r);
		return "0";
	}
	else if (num < 0) {
		r= negated(v= r);
		release(v);
	}

	ndigits= digits_in(r);
	rndflag= rndsize > 0 && (rndsize > precision - ndigits || num == 0);

	re= prod10n(r, rndflag ? rndsize : precision - ndigits, No);
	rre= round1(re);
	txt= convint((integer) rre);
	txtlen= strlen(txt);

	if (rndflag) {
		ndigits= txtlen - rndsize;
		precision= (ndigits > 0 ? txtlen : rndsize);
		rem= No;
	}
	else {
		if (txtlen > precision) {
			/* rounding caused extra digit, e.g. 999.9 ->1000 */
			txtlen--;
			txt[txtlen]= '\0';
			ndigits++;
		}
		rem= (numcomp(re, rre) != 0);
		if (!rem) {
			/* delete trailing zero's after decimal point */
			int headlen= ndigits + rndsize;
			int minlen= headlen;

			if (headlen <= 0 || headlen > precision)
				minlen= 1;
			while (txtlen > minlen && txt[txtlen-1] == '0') {
				txtlen--;
			}
			txt[txtlen]= '\0';
			if (rndsize > 0 && txtlen == headlen)
				rndflag= Yes;
		}
	}
	
	release(r); release(re); release(rre);

	/* now copy to buffer */
	if (numbuf) freemem(numbuf);
	if (rndflag)
		buflen= txtlen + (ndigits < 0 ? -ndigits : ndigits) + 10;
	
	numbuf= getmem((unsigned) buflen);
	
	str= numbuf;
	if (num<0) *str++= '-';
	
	if (ndigits > precision || (ndigits == precision && rem)) {
		*str++= *txt++;
		if (txtlen > 1) {
			*str++= '.';
			while (*txt) *str++ = *txt++;
		}
		sprintf(str, "e+%d", ndigits-1);
	}
	else if (ndigits == precision && !rem) {
		while (*txt) *str++ = *txt++;
		*str= '\0';
	}
	else if (ndigits > 0) {
		/* we end up here too for rndflag == Yes, r > 1 */
		while (ndigits-- > 0) *str++ = *txt++;
		if (*txt) *str++= '.';
		while (*txt) *str++ = *txt++;
		*str= '\0';
	}
	else if (ndigits >= -3 || rndflag) {
		/* 3 is about size of exponent,
		 * therefore allow upto 3 0's after decimal point
		 * giving 0.000ddddd instead
		 * of     0.ddddde-3 notation below;
		 *
		 * also handle rndflag == Yes, 1>r>0 here
		 */

		*str++= '0'; *str++= '.';
		while (ndigits++ < 0) *str++= '0';
		while (*txt) *str++ = *txt++;
		*str= '\0';
	}
	else {
		*str++= '0'; *str++= '.';
		while (*txt) *str++ = *txt++;
		sprintf(str, "e%d", ndigits);	/* ndigits < 0, %d gives -nnn */
	}
		
	return numbuf;
}

#define E_EXACT ABC

/* Convert a text to a number (assume it's syntactically correct!).
   Again, BASE must be a power of 10.
   ********** NEW **********
   If E_EXACT is undefined, numbers in e-notation are made
   approximate.
*/

Visible value numconst(v) register value v; {
	string txt, txt0;
	register string tp;
	register int numdigs, fraclen;
	integer a;
	register digit accu;
	value c;

	txt= sstrval(v);
	if (*txt == 'e') a = int_1;
	else {
		txt0= txt;
		while (*txt0 && *txt0=='0') ++txt0; /* Skip leading zeros */

		for (tp = txt0; isdigit(*tp); ++tp)
			; /* Count integral digits */
		numdigs = tp-txt0;
		fraclen = 0;
		if (*tp=='.') {
			++tp;
			for (; isdigit(*tp); ++tp)
				++fraclen; /* Count fractional digits */
			numdigs += fraclen;
		}
		a = (integer) grab_num((numdigs+tenlogBASE-1) / tenlogBASE);
		if (!a) goto recover;
		accu = 0;
		/* Integer part: */
		for (tp = txt0; isdigit(*tp); ++tp) {
			accu = accu*10 + *tp - '0';
			--numdigs;
			if (numdigs%tenlogBASE == 0) {
				Digit(a, numdigs/tenlogBASE) = accu;
				accu = 0;
			}
		}
		/* Fraction: */
		if (*tp == '.') {
			++tp;
			for (; isdigit(*tp); ++tp) {
				accu = accu*10 + *tp - '0';
				--numdigs;
				if (numdigs%tenlogBASE == 0) {
					Digit(a, numdigs/tenlogBASE) = accu;
					accu = 0;
				}
			}
		}
		if (numdigs != 0) syserr(MESS(800, "numconst: can't happen"));
		a = int_canon(a);
	}

	/* Exponent: */
	if (*tp != 'e') {
		integer b = int_tento(fraclen);
		if (!b) {
			/* Can't happen now; for robustness */
			Release(a);
			goto recover;
		}
		c = mk_exact(a, b, fraclen);
		Release(b);
	}
	else {
		double expo = 0;
		int sign = 1;
		value b;
		++tp;
		if (*tp == '+') ++tp;
		else if (*tp == '-') {
			++tp;
			sign = -1;
		}
		for (; isdigit(*tp); ++tp) {
			expo = expo*10 + *tp - '0';
			if (expo > Maxint) {
				interr(MESS(801, "excessive exponent in e-notation"));
				expo = 0;
				break;
			}
		}
		b = tento((int)expo * sign - fraclen);
		if (!b) {
			Release(a);
			goto recover;
		}
#ifndef E_EXACT
		/* Make approximate number if e-notation used */
		c = approximate(b);
		Release(b);
		b = c;
#endif
		if (a == int_1) c = b;
		else c = prod((value)a, b), Release(b);
	}
	Release(a);
	fstrval(txt);
	return c;

recover:
    /* from failure of grab_num, also indirect (int_tento); 
	   an error has already been reported */
	fstrval(txt);
	return Vnil;
}


/*
 * printnum(f, v) writes a number v on file f in such a way that it
 * can be read back identically.
 */

Visible Procedure printnum(fp, v) FILE *fp; value v; {
	if (Approximate(v)) {
		app_print(fp, (real) v);
		return;
	}
	if (Rational(v) && Denominator((rational)v) != int_1) {
		int i = Roundsize(v);
		fputs(convnum((value)Numerator((rational)v)), fp);
		if (i > 0) {
			/* The assumption here is that in u/v, the Roundsize
			   of the result is the sum of that of the operands. */
			putc('.', fp);
			do putc('0', fp); while (--i > 0);
		}
		putc('/', fp);
		v = (value) Denominator((rational)v);
	}
	fputs(convnum(v), fp);
}
