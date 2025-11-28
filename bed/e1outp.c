/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1986. */

/*
 * B editor -- Screen management package, lower level output part.
 */

#include "b.h"
#include "bedi.h"
#include "etex.h"
#include "bobj.h"
#include "bmem.h"
#include "node.h"
#include "supr.h"
#include "gram.h"
#include "cell.h"
#include "tabl.h"
#include "trm.h"

/*
 * Variables used for communication with outfocus.
 */

Hidden node thefocus;
Hidden environ wherebuf;
Hidden environ *where = &wherebuf;
Hidden bool realvhole;
Hidden int multiline; /* Height of focus */
Hidden int yfocus;

Visible int focy; /* Where the cursor must go */
Visible int focx;

Forward Hidden Procedure focsmash();
Forward Hidden Procedure smash();
Forward Hidden Procedure strsmash();
Forward Hidden Procedure subsmash();
Forward Hidden bool chismash();

/*
 * Save position of the focus for use by outnode/outfocus.
 */

Visible Procedure
savefocus(ep)
	register environ *ep;
{
	register int sym;
	register int w;

	realvhole = No;
	thefocus = Nnil;
	multiline = 0;
	yfocus = Ycoord(ep->focus);
	w = focoffset(ep);
	if (w < 0)
		yfocus += -w;
	w = focwidth(ep);
	if (w < 0) {
		multiline = -w;
		if (focchar(ep) == '\n')
			++yfocus;
		else
			++multiline;
		return;
	}
	if (ep->mode == WHOLE) {
		sym = symbol(tree(ep->focus));
		if (sym == Optional)
			ep->mode = ATBEGIN;
	}
	switch(ep->mode) {
	case VHOLE:
		if (ep->s1&1)
			ep->mode = FHOLE;
	case ATBEGIN:
	case ATEND:
	case FHOLE:
		ritevhole(ep);
		switch (ep->mode) {
		case ATBEGIN:
		case FHOLE:
			sym = symbol(tree(ep->focus));
			if (sym == Hole && (ep->mode == ATBEGIN || ep->s2 == 0)) {
				ep->mode = WHOLE;
				break;
			}
			/* Fall through */
		case VHOLE:
		case ATEND:
			leftvhole(ep);
			realvhole = 1 + ep->spflag;
		}
	}
	touchpath(&ep->focus); /* Make sure it is a unique pointer */
	thefocus = tree(ep->focus); /* No copy; used for comparison only! */
	where->mode = ep->mode;
	where->s1 = ep->s1;
	where->s2 = ep->s2;
	where->s3 = ep->s3;
	where->spflag = ep->spflag;
}


/*
 * Incorporate the information saved about the focus.
 */

Visible Procedure
setfocus(tops)
	register cell *tops;
{
	register cell *p;
	register int i;

	for (p = tops, i = 0; i < yfocus; ++i, p = p->c_link) {
		if (!p) {
#ifndef NDEBUG
			debug("[Focus lost (setfocus)]");
#endif /* NDEBUG */
			return;
		}
	}
	p->c_newvhole = realvhole;
	i = multiline;
	do {
		p->c_newfocus = Yes;
		p = p->c_link;
	} while (--i > 0);
}


/*
 * Signal that actual updata is started.
 */

Visible Procedure
startactupdate(nofocus)
	bool nofocus;
{
	if (nofocus) {
		multiline = 0;
		thefocus = Nnil;
	}
}


/*
 * Signal the end of the actual update.
 */

Visible Procedure
endactupdate()
{
}


/*
 * Output a line of text.
 */

Visible Procedure
outline(p, lineno)
	register cell *p;
	register int lineno;
{
	register node n = p->c_data;
	register int w = nodewidth(n);
	register int len=  p->c_newindent + 4 + (w < 0 ? linelen(n) : w);
			/* some 4 extra for spflag and vhole */
	register string buf;
	auto string bp;
	register string mode;
	auto string mp;
	register int i;
	register int endarea = lineno+Space(p)-1;

	buf= (string) getmem((unsigned) len);
	bp= buf;
	mode= (string) getmem((unsigned) len);
	mp= mode;
	if (endarea >= winheight)
		endarea = winheight-1;
	for (i = p->c_newindent; i-- > 0; ) {
		*bp++ = ' ';
		*mp++ = PLAIN;
	}
	if (!p->c_newfocus) {
		smash(&bp, &mp, n, 0);
		*bp = 0;
		Assert(bp-buf < len);
	}
	else {
		if (multiline)
			smash(&bp, &mp, n, STANDOUT);
		else if (n == thefocus)
			focsmash(&bp, &mp, n);
		else
			smash(&bp, &mp, n, 0);
		*bp = 0; *mp= 0;
		Assert(bp-buf < len);
		len= mp-mode;
		mp= mode;
		for (i = 0; i < len; i++) {
			if (*mp == STANDOUT)
				break;
			mp++;
		}
		if (*mp == STANDOUT) {
			if (focy == Nowhere) {
				focx = indent + (mp-mode);
				focy = lineno + focx/llength;
				focx %= llength;
			}
			if (multiline <= 1 && mp[1] != STANDOUT)
				*mp = PLAIN; /* Clear mask if just one char in focus */
		}
	}
	trmputdata(lineno, endarea, indent, buf, mode);
	freemem((ptr) buf);
	freemem((ptr) mode);
}


/*
 * Smash -- produce a linear version of a node in a buffer (which had
 * better be long enough!).  The buffer pointer is moved to the end of
 * the resulting string.
 * Care is taken to represent the focus.
 * Characters in the focus have their upper bit set.
 */

#define Outvhole() \
	if (where->spflag) \
		strsmash(pbuf, pmod, " ", 0); \
	strsmash(pbuf, pmod, "?", STANDOUT)

Hidden Procedure
focsmash(pbuf, pmod, n)
	string *pbuf;
	string *pmod;
	node n;
{
	value v;
	string str;
	register string *rp;
	register int maxs2;
	register int i;
	register bool ok;
	register int j;
	register int mask;

	switch (where->mode) {

	case WHOLE:
		smash(pbuf, pmod, n, STANDOUT);
		break;

	case ATBEGIN:
		Outvhole();
		smash(pbuf, pmod, n, 0);
		break;

	case ATEND:
		smash(pbuf, pmod, n, 0);
		Outvhole();
		break;

	case VHOLE:
		if (!(where->s1&1)) {
			v = (value) child(n, where->s1/2);
			Assert(Is_etext(v));
			str= e_sstrval(v);
			subsmash(pbuf, pmod, str, where->s2, 0);
			Outvhole();
			j= symbol(n);
			i= str[where->s2] == '?' &&
			 (j == Suggestion || j == Sugghowname);
			strsmash(pbuf, pmod, str + where->s2 + i, 0);
			e_fstrval(str);
			break;
		}
		/* Else, fall through */
	case FHOLE:
		rp = noderepr(n);
		maxs2 = 2*nchildren(n) + 1;
		for (ok = Yes, i = 1; ok && i <= maxs2; ++i) {
			if (i&1) {
				if (i == where->s1) {
					subsmash(pbuf, pmod, rp[i/2], where->s2, 0);
					Outvhole();
					if (rp[i/2])
						strsmash(pbuf, pmod, rp[i/2] + where->s2, 0);
				}
				else
					strsmash(pbuf, pmod, rp[i/2], 0);
			}
			else
				ok = chismash(pbuf, pmod, n, i/2, 0);
		}
		break;

	case SUBRANGE:
		rp = noderepr(n);
		maxs2 = 2*nchildren(n) + 1;
		for (ok = Yes, i = 1; ok && i <= maxs2; ++i) {
			if (i&1) {
				if (i == where->s1) {
					subsmash(pbuf, pmod, rp[i/2], where->s2,0);
					if (rp[i/2])
						subsmash(pbuf, pmod, rp[i/2] + where->s2,
							where->s3 - where->s2 + 1, STANDOUT);
					if (rp[i/2])
						strsmash(pbuf, pmod, rp[i/2] + where->s3 + 1, 0);
				}
				else
					strsmash(pbuf, pmod, rp[i/2], 0);
			}
			else if (i == where->s1) {
				v = (value)child(n, i/2);
				Assert(Is_etext(v));
				str = e_sstrval(v);
				subsmash(pbuf, pmod, str, where->s2, 0);
				subsmash(pbuf, pmod, str + where->s2, where->s3 - where->s2 + 1,
					STANDOUT);
				strsmash(pbuf, pmod, str + where->s3 + 1, 0);
				e_fstrval(str);
			}
			else
				ok = chismash(pbuf, pmod, n, i/2, 0);
		}
		break;

	case SUBLIST:
		for (ok = Yes, j = where->s3; j > 0; --j) {
			rp = noderepr(n);
			maxs2 = 2*nchildren(n) - 1;
			for (i = 1; ok && i <= maxs2; ++i) {
				if (i&1)
					strsmash(pbuf, pmod, rp[i/2], STANDOUT);
				else
					ok = chismash(pbuf, pmod, n, i/2, STANDOUT);
			}
			if (ok)
				n = lastchild(n);
		}
		if (ok)
			smash(pbuf, pmod, n, 0);
		break;

	case SUBSET:
		rp = noderepr(n);
		maxs2 = 2*nchildren(n) + 1;
		mask = 0;
		for (ok = Yes, i = 1; ok && i <= maxs2; ++i) {
			if (i == where->s1)
				mask = STANDOUT;
			if (i&1)
				strsmash(pbuf, pmod, rp[i/2], mask);
			else
				ok = chismash(pbuf, pmod, n, i/2, mask);
			if (i == where->s2)
				mask = 0;
		}
		break;

	default:
		Abort();
	}
}

Hidden Procedure
smash(pbuf, pmod, n, mask)
	register string *pbuf;
	register string *pmod;
	register node n;
	register int mask;
{
	register string *rp;
	register int i;
	register int nch;

	rp = noderepr(n);
	strsmash(pbuf, pmod, rp[0], mask);
	nch = nchildren(n);
	for (i = 1; i <= nch; ++i) {
		if (!chismash(pbuf, pmod, n, i, mask))
			break;
		strsmash(pbuf, pmod, rp[i], mask);
	}
}

Hidden Procedure
strsmash(pbuf, pmod, str, mask)
	register string *pbuf;
	register string *pmod;
	register string str;
	register int mask;
{
	if (!str)
		return;
	for (; *str; ++str) {
		if (isprint(*str) || *str == ' ') {
			**pbuf = *str, ++*pbuf;
			**pmod = mask, ++*pmod;
		}
	}
}

Hidden Procedure
subsmash(pbuf, pmod, str, len, mask)
	register string *pbuf;
	register string *pmod;
	register string str;
	register int len;
	register int mask;
{
	if (!str)
		return;
	for (; len > 0 && *str; --len, ++str) {
		if (isprint(*str) || *str == ' ') {
			**pbuf = *str, ++*pbuf;
			**pmod = mask, ++*pmod;
		}
	}
}


/*
 * Smash a node's child.
 * Return No if it contained a newline (to stop the parent).
 */

Hidden bool
chismash(pbuf, pmod, n, i, mask)
	register string *pbuf;
	register string *pmod;
	register node n;
	register int i;
{
	register node nn = child(n, i);
	register int w;

	if (Is_etext(nn)) {
		strsmash(pbuf, pmod, e_strval((value)nn), mask);
		return Yes;
	}
	w = nodewidth(nn);
	if (w < 0 && Fw_negative(noderepr(nn)[0]))
		return No;
	if (nn == thefocus)
		focsmash(pbuf, pmod, nn);
	else
		smash(pbuf, pmod, nn, mask);
	return w >= 0;
}
