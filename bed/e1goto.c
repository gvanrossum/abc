/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1986. */

/*
 * B editor -- Random access focus positioning.
 */

#include "b.h"
#include "bedi.h"
#include "etex.h"
#include "bobj.h"
#include "erro.h"
#include "node.h"
#include "gram.h"
#include "supr.h"

extern int winheight;
extern int winstart;


#define BEFORE (-1)
#define INSIDE 0
#define BEYOND 1


#ifdef GOTOCURSOR

/*
 * Random cursor positioning:
 * gotocursor() for terminals with local cursor motion;
 * gotomouse() for mouse-clicks.
 */

Visible bool cansense= No;
Visible string gotosense;
Visible string gotoformat;

Visible string mousesense;
Visible string mouseformat;

Forward Hidden bool dosense();
Forward Hidden bool gotoscrollbar();
Forward Hidden int poscomp();
Forward Hidden Procedure fixsublist();

Visible bool gotocursor(ep) environ *ep; {
	int y;
	int x;
	
	if (!cansense) {
		while (narrow(ep))
			;
		if (ep->mode == ATEND)
			leftvhole(ep);
		y = lineno(ep);
		x = colno(ep);
		if (!gotoyx(ep, y, x))
			return No;
		gotofix(ep, y, x);
		return Yes;
	}
	/* else: */
	return dosense(ep, gotosense, gotoformat);
}

Visible bool gotomouse(ep) environ *ep; {
	return dosense(ep, mousesense, mouseformat);
}

Hidden bool dosense(ep, sense, format) environ *ep; string sense, format; {
	int y;
	int x;
	
	trmsense(sense, format, &y, &x);
	if (y < 0 || x < 0) {
		ederr(GOTO_BAD);
		return No;
	}
#ifdef SCROLLBAR
	if (y == winheight)
		return gotoscrollbar(ep, x);
#endif /* SCROLLBAR */
	if (!backtranslate(&y, &x))
		return No;
	if (!gotoyx(ep, y, x))
		return No;
	gotofix(ep, y, x);
	return Yes;
}

#ifdef SCROLLBAR

/*
 * Special case for goto: user pointed at some point in the scroll bar.
 * Go directly to the corresponding line.
 * (The scroll bar is only present when winstart == 0; it extends from
 * col 0 to winheight-1 inclusive.)
 */

Hidden bool
gotoscrollbar(ep, x)
	environ *ep;
	int x;
{
	int w;

	if (winstart != 0 || x >= winheight) { /* Not within scroll bar */
		ederr(GOTO_OUT);
		return No;
	}
	top(&ep->focus);
	ep->mode = WHOLE;
	higher(ep);
	w = nodewidth(tree(ep->focus));
	if (w >= 0)
		w = 1;
	else
		w = 1-w;
	if (!gotoyx(ep, x * w / winheight, 0))
		return No;
	oneline(ep);
	return Yes;
}

#endif /* SCROLLBAR */

#endif /* GOTOCURSOR */

/*
 * Set the focus to the smallest node or subset surrounding
 * the position (y, x).
 */

Visible bool
gotoyx(ep, y, x)
	register environ *ep;
	register int y;
	register int x;
{
	register node n;
	register string *rp;
	register int i;
	register int pc;

	ep->mode = WHOLE;
	while ((pc = poscomp(ep->focus, y, x)) != INSIDE) {
		if (!up(&ep->focus)) {
			if (pc == BEFORE)
				ep->mode = ATBEGIN;
			else
				ep->mode = ATEND;
			higher(ep);
			return No;
		}
	}
	higher(ep);
	for (;;) {
		switch (poscomp(ep->focus, y, x)) {

		case BEFORE:
			i = ichild(ep->focus);
			n = tree(parent(ep->focus)); /* Parent's !!! */
			rp = noderepr(n);
			if (Fw_positive(rp[i-1])) {
				s_up(ep);
				ep->s1 = ep->s2 = 2*i - 1;
				ep->mode = SUBSET;
			}
			else if (left(&ep->focus))
				ep->mode = ATEND;
			else
				ep->mode = ATBEGIN;
			return Yes;

		case INSIDE:
			n = tree(ep->focus);
			if (nchildren(n) >= 1 && !Is_etext(firstchild(n))) {
				s_down(ep);
				continue;
			}
			ep->mode = WHOLE;
			return Yes;

		case BEYOND:
			if (rite(&ep->focus))
				continue;
			n = tree(parent(ep->focus)); /* Parent's !!! */
			rp = noderepr(n);
			i = ichild(ep->focus);
			if (Fw_positive(rp[i])) {
				s_up(ep);
				ep->s1 = ep->s2 = 2*i + 1;
				ep->mode = SUBSET;
			}
			else
				ep->mode = ATEND;
			return Yes;

		default:
			Abort();
			/* NOTREACHED */

		}
	}
}


/*
 * Deliver relative position of (y, x) with respect to focus p:
 * BEFORE: (y, x) precedes focus;
 * INSIDE: (y, x) contained in focus;
 * EAFTER:  (y, x) follows focus.

 */

Hidden int
poscomp(p, y, x)
	register path p;
	register int y;
	register int x;
{
	register int ly;
	register int lx;
	register int w;
	register string *rp;
	register node n;

	ly = Ycoord(p);
	lx = Xcoord(p);
	if (y < ly || y == ly && (lx < 0 || x < lx))
		return BEFORE;
	n = tree(p);
	w = nodewidth(n);
	if (w < 0) {
		if (y == ly) { /* Hack for position beyond end of previous line */
			rp = noderepr(n);
			if (Fw_negative(rp[0]))
				return BEFORE;
		}
		ly += -w;
		lx = -1;
	}
	else {
		if (lx >= 0)
			lx += w;
	}
	if (y < ly || y == ly && (lx < 0 || x < lx))
		return INSIDE;
	return BEYOND;
}


/*
 * Position focus exactly at character indicated by (y, x) if possible.
 * If this is the start of something larger, position focus at largest
 * object starting here.
 */

Visible Procedure
gotofix(ep, y, x)
	environ *ep;
	int y;
	int x;
{
	int fx;
	int fy;
	int len;
	string repr;

	switch (ep->mode) {

	case ATBEGIN:
	case ATEND:
		return; /* No change; the mouse pointed in the margin. */

	case SUBSET:
		if (ep->s1 > 1) {
			fx = Xcoord(ep->focus);
			fy = Ycoord(ep->focus);
			len = focoffset(ep);
			if (len < 0 || fy != y)
				return;
			if ((ep->s1&1) && fx + len >= x-1) {
				string *nr= noderepr(tree(ep->focus));
				repr = nr[ep->s1/2];
				if ((repr && repr[0] == ' ') != (fx + len == x))
					return;
			}
			else if (fx + len == x)
				return;
		}
		ep->mode = WHOLE;
		/* Fall through */
	case WHOLE:
		fx = Xcoord(ep->focus);
		fy = Ycoord(ep->focus);
		if (y != fy)
			return;
		if (x <= fx ) {
			for (;;) {
				if (ichild(ep->focus) > 1)
					break;
				if (!up(&ep->focus))
					break;
				repr = noderepr(tree(ep->focus))[0];
				if (!Fw_zero(repr)) {
					s_down(ep);
					break;
				}
				higher(ep);
			}
			if (issublist(symbol(tree(ep->focus))))
				fixsublist(ep);
			return;
		}
		fixfocus(ep, x - fx);
		ritevhole(ep);
		switch(ep->mode) {
		case VHOLE:
			len = nodewidth(tree(ep->focus));
			break;
		case FHOLE:
			{
			string *nr= noderepr(tree(ep->focus));
			len = fwidth(nr[ep->s1/2]);
			}
			break;
		default:
			return;
		}
		if (ep->s2 < len) {
			ep->mode = SUBRANGE;
			ep->s3 = ep->s2;
		}
		return;

	default:
		Abort();
	}
}


/*
 * Refinement for gotofix -- don't show right sublist of something.
 */

Hidden Procedure
fixsublist(ep)
	environ *ep;
{
	path pa = parent(ep->focus);
	node n;

	if (!pa)
		return;
	n = tree(pa);
	if (nchildren(n) > ichild(ep->focus))
		return;
	if (samelevel(symbol(n), symbol(tree(ep->focus)))) {
		ep->mode = SUBLIST;
		ep->s3 = 1;
	}
}
