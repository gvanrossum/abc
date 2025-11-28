/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1986. */

/* B texts */

#include "b.h"
#include "bmem.h"
#include "bobj.h"
#include "i1btr.h"
#include "i1tlt.h"

Forward Hidden Procedure convbtext();

#define CURTAIL_TEX	MESS(200, "in t|n, t is not a text")
#define CURTAIL_NUM	MESS(201, "in t|n, n is not a number")
#define CURTAIL_INT	MESS(202, "in t|n, n is not an integer")
#define CURTAIL_BND	MESS(203, "in t|n, n is < 0")

#define BEHEAD_TEX	MESS(204, "in t@n, t is not a text")
#define BEHEAD_NUM	MESS(205, "in t@n, n is not a number")
#define BEHEAD_INT	MESS(206, "in t@n, n is not an integer")
#define BEHEAD_BND	MESS(207, "in t@n, n is > #t + 1")

#define CONCAT_TEX	MESS(208, "in t^u, t or u is not a text")
#define CONCAT_LONG	MESS(209, "in t^u, the result is too long")

#define REPEAT_TEX	MESS(210, "in t^^n, t is not a text")
#define REPEAT_NUM	MESS(211, "in t^^n, n is not a number")
#define REPEAT_INT	MESS(212, "in t^^n, n is not an integer")
#define REPEAT_NEG	MESS(213, "in t^^n, n is negative")
#define REPEAT_LONG	MESS(214, "in t^^n, the result is too long")

/*
 * Operations on texts represented as B-trees.
 *
 * Comments:
 * - The functions with 'i' prepended (ibehead, etc.) do no argument
 *   checking at all.  They actually implement the planned behaviour
 *   of | and @, where out-of-bounds numerical values are truncated
 *   rather than causing errors {"abc"|100 = "abc"@-100 = "abc"}.
 * - The 'size' field of all texts must fit in a C int.  If the result of
 *   ^ or ^^ would exceed Maxint in size, a user error is signalled.  If
 *   the size of the *input* value(s) of any operation is Bigsize, a syserr
 *   is signalled.
 * - Argument checking: trims, concat and repeat must check their arguments
 *   for user errors.
 * - t^^n is implemented with an algorithm similar to the 'square and
 *   multiply' algorithm for x**n, using the binary representation of n,
 *   but it uses straightforward 'concat' operations.  A more efficient
 *   scheme is possible [see IW219], but small code seems more important.
 * - Degenerated cases (e.g. t@1, t|0, t^'' or t^^n) are not optimized,
 *   but produce the desired result by virtue of the algorithms used.
 *   The extra checking does not seem worth the overhead for the
 *   non-degenerate cases.
 * - The code for PUT v IN t@h|l is still there, but it is not compiled,
 *   as the interpreter implements the same strategy directly.
 * - Code for outputting texts has been added.	This is called from wri()
 *   to output a text, and has running time O(n), compared to O(n log n)
 *   for the old code in wri().
 *
 * *** WARNING ***
 * - The 'zip' routine and its subroutine 'copynptrs' assume that items and
 *   pointers are stored contiguously, so that &Ptr(p, i+1) == &Ptr(p, i)+1
 *   and &[IB]char(p, i+1) == &[IB]char(p, i)+1.  For pointers, the order
 *   might be reversed in the future; then change the macro Incr(pp, n) below
 *   to *decrement* the pointer!
 * - Mkbtext and bstrval make the same assumption about items (using strncpy
 *   to move charaters to/from a bottom node).
 */

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#define IsInner(p) (Flag(p) == Inner)
#define IsBottom(p) (Flag(p) == Bottom)

#define Incr(pp, n) ((pp) += (n))

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

Visible char charval(v) value v; {
	if (!Character(v))
		syserr(MESS(215, "charval on non-char"));
	return Bchar(Root(v), 0);
}

Visible char ncharval(n, v) int n; value v; {
	value c= thof(n, v);
	char ch= charval(c);
	release(c);
	return ch;
}

Visible bool character(v) value v; {
	return Character(v);
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

Hidden btreeptr mkbtext(s, len) string s; int len; {
	btreeptr p; int chunk, i, n, nbig;

	/*
	 * Determine level of tree.
	 * This is done for each inner node anew, to avoid having
	 * to keep an explicit stack.
	 * Problem is: make sure that for each node at the same
	 * level, the computation indeed finds the same level!
	 * (Don't care about efficiency here; in practice the trees
	 * built by mk_text rarely need more than two levels.)
	 */
	chunk = 0;
	i = Maxbottom; /* Next larger chunk size */
	while (len > i) {
		chunk = i;
		i = (i+1) * Maxinner + Maxinner;
	}
	n = len / (chunk+1); /* Number of items at this level; n+1 subtrees */
	chunk = len / (n+1); /* Use minimal chunk size for subtrees */
	p = grabbtreenode(chunk ? Inner : Bottom, Ct);
	Size(p) = len;
	Lim(p) = n;
	if (!chunk)
		strncpy(&Bchar(p, 0), s, len);
	else {
		nbig = len+1 - (n+1)*chunk;
			/* There will be 'nbig' nodes of size 'chunk'. */
			/* The remaining 'n-nbig' will have size 'chunk-1'. */
		for (i = 0; i < n; ++i) {
			Ptr(p, i) = mkbtext(s, chunk);
			s += chunk;
			Ichar(p, i) = *s++;
			len -= chunk+1;
			if (--nbig == 0)
				--chunk; /* This was the last 'big' node */
		}
		Ptr(p, i) = mkbtext(s, len);
	}
	return p;
}

Visible value mk_text(s) string s; {
	value v; int len = strlen(s);

	v = grab(Tex, Ct);
	if (len == 0)
		Root(v) = Bnil;
	else
		Root(v) = mkbtext(s, len);
	return v;
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

Hidden string bstrval(buf, p) string buf; btreeptr p; {
	/* Returns *next* available position in buffer */
	int i, n = Lim(p);
	if (IsInner(p)) {
		for (i = 0; i < n; ++i) {
			buf = bstrval(buf, Ptr(p, i));
			*buf++ = Ichar(p, i);
		}
		return bstrval(buf, Ptr(p, i));
	}
	strncpy(buf, &Bchar(p, 0), n);
	return buf+n;
}

Hidden char *buffer= NULL;
Visible string strval(v) value v; {
	int len = Tltsize(v);
	if (len == Bigsize) syserr(MESS(216, "strval on big text"));
	if (len == 0) return "";
	if (buffer != NULL)
		regetmem(&buffer, (unsigned) len+1);
	else
		buffer = getmem((unsigned) len+1);
	*bstrval(buffer, Root(v)) = '\0';
	return buffer;
}

Visible Procedure endstrval() { 	/* hack to free static store */
#ifdef MEMTRACE
	if (buffer != NULL)
		freemem(buffer);
#endif
}

Visible string sstrval(v) value v; {
	return (string) savestr(strval(v));
}

Visible Procedure fstrval(s) string s; {
	freestr(s);
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

typedef struct stackelem {
	btreeptr s_ptr;
	int s_lim;
} stackelem;

typedef stackelem stack[Maxheight];
typedef stackelem *stackptr;

#define Snil ((stackptr)0)

#define Push(s, p, l) ((s)->s_ptr = (p), ((s)->s_lim = (l)), (s)++)
#define Pop(s, p, l) (--(s), (p) = (s)->s_ptr, (l) = (s)->s_lim)

extern stackptr unzip();
extern int movnptrs();

Hidden btreeptr zip(s1, sp1, s2, sp2) stackptr s1, sp1, s2, sp2; {
	btreeptr p1, p2, newptr[2]; int l1, l2, i, n, n2;
#define q1 newptr[0]
#define q2 newptr[1]
	char newitem; bool overflow, underflow, inner;
	char *cp; btreeptr *pp;
	char cbuf[2*Maxbottom]; btreeptr pbuf[2*Maxinner+2];

	while (s1 < sp1 && s1->s_lim == 0)
		++s1;
	while (s2 < sp2 && s2->s_lim == Lim(s2->s_ptr))
		++s2;
	inner = overflow = underflow = No;
	q1 = Bnil;
	while (s1 < sp1 || s2 < sp2) {
		if (s1 < sp1)
			Pop(sp1, p1, l1);
		else
			p1 = Bnil;
		if (s2 < sp2)
			Pop(sp2, p2, l2);
		else
			p2 = Bnil;
		cp = cbuf;
		if (p1 != Bnil) {
			strncpy(cp, (inner ? &Ichar(p1, 0) : &Bchar(p1, 0)), l1);
			cp += l1;
		}
		if (overflow)
			*cp++ = newitem;
		n = cp - cbuf;
		if (p2 != Bnil) {
			strncpy(cp, (inner ? &Ichar(p2, l2) : &Bchar(p2, l2)), Lim(p2)-l2);
			n += Lim(p2)-l2;
		}
		if (inner) {
			pp = pbuf; /***** Change if reverse direction! *****/
			if (p1 != Bnil) {
				cpynptrs(pp, &Ptr(p1, 0), l1);
				Incr(pp, l1);
			}
			movnptrs(pp, newptr, 1+overflow);
			Incr(pp, 1+overflow);
			if (p2 != Bnil) {
				cpynptrs(pp, &Ptr(p2, l2+1), Lim(p2)-l2);
				Incr(pp, Lim(p2)-l2);
			}
			if (underflow) {
				underflow= No;
				n= uflow(n, p1 ? l1 : 0, cbuf, pbuf, Ct);
			}
		}
		overflow = No;
		if (n > (inner ? Maxinner : Maxbottom)) {
			overflow = Yes;
			n2 = (n-1)/2;
			n -= n2+1;
		}
		else if (n < (inner ? Mininner : Minbottom))
			underflow = Yes;
		q1 = grabbtreenode(inner ? Inner : Bottom, Ct);
		Lim(q1) = n;
		cp = cbuf;
		strncpy((inner ? &Ichar(q1, 0) : &Bchar(q1, 0)), cp, n);
		cp += n;
		if (inner) {
			pp = pbuf;
			i = movnptrs(&Ptr(q1, 0), pp, n+1);
			Incr(pp, n+1);
			n += i;
		}
		Size(q1) = n;
		if (overflow) {
			newitem = *cp++;
			q2 = grabbtreenode(inner ? Inner : Bottom, Ct);
			Lim(q2) = n2;
			strncpy((inner ? &Ichar(q2, 0) : &Bchar(q2, 0)), cp, n2);
			if (inner)
				n2 += movnptrs(&Ptr(q2, 0), pp, n2+1);
			Size(q2) = n2;
		}
		inner = Yes;
	}
	if (overflow)
		q1 = mknewroot(q1, (itemptr)&newitem, q2, Ct);
	return q1;
#undef q1
#undef q2
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

Hidden value ibehead(v, h) value v; int h; { /* v@h */
	stack s; stackptr sp;
	sp = (stackptr) unzip(Root(v), h-1, s);
	v = grab(Tex, Ct);
	Root(v) = zip(Snil, Snil, s, sp);
	return v;
}

Hidden value icurtail(v, t) value v; int t; { /* v|t */
	stack s; stackptr sp;
	sp = (stackptr) unzip(Root(v), t, s);
	v = grab(Tex, Ct);
	Root(v) = zip(s, sp, Snil, Snil);
	return v;
}

Hidden value iconcat(v, w) value v, w; { /* v^w */
	stack s1, s2;
	stackptr sp1 = (stackptr) unzip(Root(v), Tltsize(v), s1);
	stackptr sp2 = (stackptr) unzip(Root(w), 0, s2);
	v = grab(Tex, Ct);
	Root(v) = zip(s1, sp1, s2, sp2);
	return v;
}

#define Odd(n) (((n)&1) != 0)

Hidden value irepeat(v, n) value v; int n; { /* v^^n */
	value x, w = grab(Tex, Ct);
	Root(w) = Bnil;
	v = copy(v);
	while (n > 0) {
		if (Odd(n)) {
			w = iconcat(x = w, v);
			release(x);
		}
		n /= 2;
		if (n == 0)
			break;
		v = iconcat(x = v, v);
		release(x);
	}
	release(v);
	return w;
}

#ifdef UNUSED_CODE
Hidden value jrepeat(v, n) value v; int n; { /* v^^n, recursive solution */
	value w, x;
	if (n <= 1) {
		if (n == 1)
			return copy(v);
		w = grab(Tex, Ct);
		Root(w) = Bnil;
		return w;
	}
	w = jrepeat(v, n/2);
	w = iconcat(x = w, w);
	release(x);
	if (Odd(n)) {
		w = iconcat(x = w, v);
		release(x);
	}
	return w;
}
#endif /* UNUSED_CODE */

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

Visible value curtail(t, after) value t, after; {
	int syzcurv, syztext;

	if (!Is_text(t)) {
		reqerr(CURTAIL_TEX);
		return Vnil;
	}
	if (!Is_number(after)) {
		reqerr(CURTAIL_NUM);
		return Vnil;
	}
	syztext = Tltsize(t);
	if (syztext == Bigsize)
		syserr(MESS(217, "curtail on very big text"));
 	if (large(after) || (syzcurv = intval(after)) < 0) {
		reqerr(CURTAIL_BND);
		return Vnil;
	}
	return icurtail(t, syzcurv);
}

Visible value behead(t, before) value t, before; {
	int syzbehv, syztext;

	if (!Is_text(t)) {
		reqerr(BEHEAD_TEX);
		return Vnil;
	}
	if (!Is_number(before)) {
		reqerr(BEHEAD_NUM);
		return Vnil;
	}
	syztext = Tltsize(t);
	if (syztext == Bigsize) syserr(MESS(218, "behead on very big text"));
	if (large(before) || (syzbehv = intval(before)) > syztext + 1) {
		reqerr(BEHEAD_BND);
		return Vnil;
	}
	return ibehead(t, syzbehv);
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

Visible value concat(tleft, tright) value tleft, tright; {
	int syzleft, syzright;
	if (!Is_text(tleft) || !Is_text(tright)) {
		reqerr(CONCAT_TEX);
		return Vnil;
	}
	syzleft = Tltsize(tleft);
	syzright =  Tltsize(tright);
	if (syzleft == Bigsize || syzright == Bigsize)
		syserr(MESS(219, "concat on very big text"));
	if (syzleft > Maxint-syzright
		|| syzright > Maxint-syzleft) {
		reqerr(CONCAT_LONG);
		return Vnil;
	}
	return iconcat(tleft, tright);
}

Visible Procedure concato(v, t) value* v; value t; {
	value v1= *v;
	*v= concat(*v, t);
	release(v1);
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

Visible value repeat(t, n) value t, n; {
	int tsize, k;

	if (!Is_text(t)) {
		reqerr(REPEAT_TEX);
		return Vnil;
	}
	if (!Is_number(n)) {
		reqerr(REPEAT_NUM);
		return Vnil;
	}
	if (numcomp(n, zero) < 0) {
		reqerr(REPEAT_NEG);
		return Vnil;
	}
	tsize = Tltsize(t);
	if (tsize == 0) return copy(t);

	if (large(n) || Maxint/tsize < (k = intval(n))) {
		reqerr(REPEAT_LONG);
		return Vnil;
	}
	return irepeat(t, k);
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

Visible Procedure convtext(outproc, v, quote)
     Procedure (*outproc)();
     value v;
     char quote;
{
	if (!Valid(v) || !Is_text(v)) {
		(*outproc)('?');
		return;
	}
	if (quote) (*outproc)(quote);
	if (Root(v) != Bnil) convbtext(outproc, Root(v), quote);
	if (quote) (*outproc)(quote);
}

Hidden Procedure convbtext(outproc, p, quote)
     Procedure (*outproc)();
     btreeptr p;
     char quote;
{
	int i, n = Lim(p); char c;

	if (!still_ok) return;
	if (IsInner(p)) {
		for (i = 0; i < n && !Interrupted();  ++i) {
			convbtext(outproc, Ptr(p, i), quote);
			c = Ichar(p, i);
			(*outproc)(c);
			if (quote && (c == quote || c == '`')) (*outproc)(c);
		}
		convbtext(outproc, Ptr(p, i), quote);
	}
	else if (quote) {
		for (i = 0; i < n && !Interrupted(); ++i) {
			c = Bchar(p, i);
			(*outproc)(c);
			if (c == quote || c == '`') (*outproc)(c);
		}
	}
	else {
		for (i = 0; i < n && !Interrupted(); ++i) {
			(*outproc)(Bchar(p, i));
		}
	}
}

