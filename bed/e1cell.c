/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1986. */

/*
 * B editor -- Screen management package, cell list manipulation routines.
 */

#include "b.h"
#include "b0lan.h"
#include "bedi.h"
#include "bmem.h"
#include "bobj.h"
#include "node.h"
#include "cell.h"
#include "port.h"

extern bool noscroll;
#ifndef NDEBUG
extern bool dflag;
#endif

Forward Hidden Procedure feedfreelist();
Forward Hidden Procedure dupmatch();

/*
 * Definitions for internals of cell manipulations.
 */

Hidden cell *freelist;

#define CELLSIZE (sizeof(cell))

#ifndef PAGESIZE /* 4.2 BSD freaks compile with -DPAGESIZE='getpagesize()' */
#define PAGESIZE 1024
#endif

#ifndef MALLOCLOSS
#define MALLOCLOSS (sizeof(char*))
	/* number of bytes taken by malloc administration per block */
#endif


/*
 * Replace `oldlcnt' cells from `tops', starting at the one numbered `oldlno',
 * by the list `rep'.
 * Returns a pointer to the deleted chain (with a Nil end pointer).
 */

Visible cell *
replist(tops, rep, oldlno, oldlcnt)
	cell *tops;
	cell *rep;
	int oldlno;
	register int oldlcnt;
{
	cell head;
	register cell *p;
	register cell *q;
	register cell *old;
	register cell *end;
	register int i;
	int replcnt;

	if (!tops) /* Start with empty list */
		return rep;
	head.c_link = tops;
	p = &head;
	for (i = oldlno; i > 0; --i) {
		p = p->c_link;
		Assert(p);
	}
	q = p;
	for (i = oldlcnt; i > 0 && p; --i)
		p = p->c_link;
	if (i > 0) {
#ifndef NDEBUG
	if (dflag)
		debug("[replist jackpot]");
#endif /* NDEBUG */
		oldlcnt -= i;
	}
	old = q->c_link;
	q->c_link = rep;
	if (p) {
		end = p->c_link;
		p->c_link = Cnil;
	}
	for (replcnt = 0; q->c_link; ++replcnt, q = q->c_link)
		;
	dupmatch(old, rep, oldlcnt, replcnt);
	discard(old);
	if (p)
		q->c_link = end;
	return head.c_link;
}


/*
 * Allocate a new cell.
 */

Hidden cell *
newcell()
{
	register cell *p;

	if (!freelist)
		feedfreelist();
	p = freelist;
	freelist = p->c_link;
	p->c_link = Cnil;
	return p;
}


/*
 * Feed the free list with a block of new entries.
 * We try to keep them together on a page
 * to keep consecutive accesses fast.
 */

Hidden Procedure
feedfreelist()
{
	register int n = (PAGESIZE-MALLOCLOSS) / CELLSIZE;
	register cell *p = (cell*) getmem((unsigned)(n*CELLSIZE));
#ifdef MEMTRACE
	fixmem((ptr) p);
#endif
	Assert(n > 0);
	freelist = p;
	for (; n > 1; --n, ++p)
		p->c_link = p+1;
	p->c_link = Cnil;
}


/*
 * Discard all entries of a list of cells.
 */

Visible Procedure
discard(p)
	register cell *p;
{
	register cell *savefreelist;

	if (!p)
		return;
	savefreelist = p;
	for (;;) {
		noderelease(p->c_data);
		p->c_data = Nnil;
		if (!p->c_link)
			break;
		p = p->c_link;
	}
	p->c_link = freelist;
	freelist = savefreelist;
}


/*
 * Replace the `onscreen' fields in the replacement chain by those
 * in the old chain, if they match.
 */

Hidden Procedure
dupmatch(old, rep, oldcnt, repcnt)
	register cell *old;
	register cell *rep;
	int oldcnt;
	int repcnt;
{
	register int delta = repcnt - oldcnt;

#ifndef NDEBUG
	if (dflag)
		debug("[dupmatch(oldcnt=%d, newcnt=%d)]", oldcnt, repcnt);
#endif /* NDEBUG */
	while (rep && old) {
		if (old->c_length == rep->c_length
			&& eqlines(old->c_data, rep->c_data)) {
			if (old->c_onscreen != Nowhere) {
				rep->c_onscreen = old->c_onscreen;
				rep->c_oldindent = old->c_oldindent;
				rep->c_oldvhole = old->c_oldvhole;
				rep->c_oldfocus = old->c_oldfocus;
			}
			rep = rep->c_link;
			old = old->c_link;
		}
		else {
			if (delta >= 0) {
				--delta;
				rep = rep->c_link;
			}
			if (delta < 0) {
				++delta;
				old = old->c_link;
			}
		}
	}
}


/*
 * Build a list of cells consisting of the first `lcnt' lines of the tree.
 */

Visible cell *
build(p, lcnt)
	/*auto*/ path p;
	register int lcnt;
{
	cell head;
	register cell *q = &head;

	p = pathcopy(p);
	for (;;) {
		q = q->c_link = newcell();
		q->c_onscreen = Nowhere;
		q->c_data = nodecopy(tree(p));
		q->c_length = linelen(q->c_data);
		q->c_newindent = Level(p) * INDENTSIZE;
		q->c_oldindent = 0;
		q->c_oldvhole = q->c_newvhole = q->c_oldfocus = q->c_newfocus = No;
		--lcnt;
		if (lcnt <= 0)
			break;
		if (!nextline(&p)) Abort();
	}
	q->c_link = Cnil;
	pathrelease(p);
	return head.c_link;
}


/*
 * Decide which line is to be on top of the screen.
 * We slide a window through the list of lines, recognizing
 * lines of the focus and lines already on the screen,
 * and stop as soon as we find a reasonable focus position.
 *
 * - The focus must always be on the screen completely;
 *   if it is larger than the screen, its first line must be
 *   on top of the screen.
 * - When old lines can be retained, at least one line above
 *   and below the focus must be shown; the retained lines
 *   should be moved as little as possible.
 * - As little as possible blank space should be shown at the
 *   bottom, even if the focus is at the end of the unit.
 * - If no rule applies, try to center the focus on the screen.
 * - If noscroll is Yes (the terminal can't scroll), and the top
 *   line can't be retained, also try to center the focus on the
 *   screen.
 */

Visible cell *
gettop(tops)
	cell *tops;
{
	register cell *pfwa = tops; /* First line of sliding window */
	register cell *plwa = tops; /* Last+1 line of sliding window */
	register cell *pffocus = Cnil; /* First line of focus */
	cell *pscreen = Cnil; /* First line still on screen */
	register int nfwa = 0; /* Corresponding line numbers in parse tree */
	register int nlwa = 0;
	register int nffocus;
	int nlfocus;
	int nscreen;
	int syz;

	for (;;) { /* plwa is the current candidate for top line. */
		if (!pfwa) {
#ifndef NDEBUG
			debug("[Lost the focus!]");
#endif /* NDEBUG */
			return tops; /* To show *something*... */
		}
		while (plwa && nlwa < nfwa+winheight) {
			/* Find first line *not* in window */
			syz = Space(plwa);
			if (plwa->c_newfocus) { /* Hit a focus line */
				if (!pffocus) { /* Note first focus line */
					pffocus = plwa;
					nffocus = nlwa;
				}
				nlfocus = nlwa + syz;
			}
			if (plwa->c_onscreen != Nowhere) { /* Hello old chap */
				if (!pscreen) { /* Note first line on screen */
					pscreen = plwa;
					nscreen = nlwa;
				}
			}
			nlwa += syz;
			plwa = plwa->c_link;
		}
		if (pffocus) {
			/* Focus in sight; stop at first reasonable opportunity */
			if (pffocus == pfwa)
				break; /* Grab last chance! */
			if (!noscroll && nlwa - nfwa <= winheight - winheight/3)
				break; /* Don't show too much white space at bottom */
			if (pffocus == pfwa->c_link && nlfocus < nfwa+winheight)
				break; /* Near top line */
			if (pscreen && (!noscroll || nffocus > nscreen)) {
				/* Conservatism may succeed */
				if (pscreen->c_onscreen >= nscreen - nfwa
					&& (nlfocus < nfwa+winheight
						|| !plwa && nlfocus == nfwa+winheight))
					break; /* focus entirely on screen */
			}
			else { /* No comrades seen */
				if (nffocus - nfwa <= nfwa+winheight - nlfocus
					|| !plwa && nlwa <= nfwa+winheight)
					break; /* Nicely centered focus or end of unit */
			}
		}
		if (pfwa == pscreen) { /* Say farewell to oldest comrade */
			pscreen->c_onscreen = Nowhere;
			do { /* Find next in age */
				nscreen += Space(pscreen);
				pscreen = pscreen->c_link;
				if (pscreen == plwa) {
					pscreen = Cnil;
					break;
				}
			} while (pscreen->c_onscreen == Nowhere);
		}
		nfwa += Space(pfwa);
		pfwa = pfwa->c_link; /* Pass the buck */
	}
	return pfwa; /* This is what all those breaks aim at */
}
