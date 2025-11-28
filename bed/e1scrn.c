/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1986. */

/*
 * B editor -- Screen management package, higher level routines.
 */

#include "b.h"
#include "bedi.h"
#include "etex.h"
#include "bobj.h"
#include "erro.h"
#include "node.h"
#include "supr.h"
#include "gram.h"
#include "cell.h"
#include "trm.h"
#include "port.h"

#ifndef NDEBUG
extern bool dflag;
#endif

Forward cell *gettop();
Forward Hidden Procedure growwin();
Forward Hidden int makeroom();
Forward Hidden int make2room();
Forward Hidden bool atlinestart();
Forward Hidden int fixlevels();
Forward Hidden Procedure initvtrm();


extern int focy;
extern int focx;

Visible int winstart;

Visible int winheight;
Visible int indent;
Visible int llength;

Visible bool noscroll;

Hidden cell *tops;


/*
 * Actual screen update.
 */

Visible Procedure
actupdate(copybuffer, recording, lasttime)
	value copybuffer;
	bool recording;
	bool lasttime; /* Yes if called from final screen update */
{
	register cell *p;
	cell *ptop = tops;
	register int delta;
	register int curlno;
	register int delcnt = 0; /* Lines deleted during the process. */
		/* Used as offset for lines that are on the screen. */
	int totlines = 0;
	int topline = 0;
	int scrlines = 0;

	if (winstart > 0)
		growwin();
	if (winstart <= 0) {
		ptop = gettop(tops);
		for (p = tops; p && p != ptop; p = p->c_link)
			++topline;
		totlines = topline;
	}
	startactupdate(lasttime);
	focy = Nowhere;
	for (p = ptop, curlno = winstart; p && curlno < winheight;
		curlno += Space(p), p = p->c_link) {
		++scrlines;
		if (lasttime) {
			p->c_newfocus = No;
			p->c_newvhole = 0;
		}
		if (p->c_onscreen != Nowhere && Space(p) == Oldspace(p)) {
			/* Old comrade */
			delta = p->c_onscreen - (curlno+delcnt);
			/* delta can't be negative due to 'makeroom' below! */
			if (delta > 0) { /* Get him here */
				trmscrollup(curlno, winheight, delta);
				delcnt += delta;
			}
			if (p->c_oldfocus || p->c_newfocus
				|| p->c_oldindent != p->c_newindent
				|| p->c_onscreen + Space(p) >= winheight) {
				delcnt = make2room(p, curlno, delcnt);
				outline(p, curlno);
			}
		}
		else { /* New guy, make him toe the line */
			delcnt = makeroom(p, curlno, delcnt);
			delcnt = make2room(p, curlno, delcnt);
			outline(p, curlno);
		}
		p->c_onscreen = curlno;
		p->c_oldindent = p->c_newindent;
		p->c_oldvhole = p->c_newvhole;
		p->c_oldfocus = p->c_newfocus;
	}
	totlines += scrlines;
	for (; p; p = p->c_link) { /* Count rest and remove old memories */
		++totlines;
		/* This code should never find any garbage?! */
#ifndef NDEBUG
		if (p->c_onscreen != Nowhere)
			debug("[Garbage removed from screen list]");
#endif /* NDEBUG */
		p->c_onscreen = Nowhere;
	}
	trmscrollup(curlno, winheight, -delcnt);
	curlno += delcnt;
	if (curlno < winheight) { /* Clear lines beyond end of unit */
		trmputdata(curlno, winheight-1, 0, "", (string)0);
		scrlines += winheight-curlno;
	}
	if (!lasttime) {
		stsline(totlines, topline, scrlines, copybuffer, recording);
		if (focy != Nowhere)
			trmsync(focy, focx);
		else
			trmsync(winheight, 0);
	}
	endactupdate();
}


/*
 * Grow the window if not maximum size.
 */

Hidden Procedure
growwin()
{
	register int winsize;
	register int growth;
	register cell *p;

	winsize = 0;
	for (p = tops; p; p = p->c_link)
		winsize += Space(p);
	if (winsize <= winheight - winstart)
		return; /* No need to grow */
	if (winsize > winheight)
		winsize = winheight; /* Limit size to maximum available */

	growth = winsize - (winheight - winstart);
	trmscrollup(0, winheight - (winstart!=winheight), growth);
	winstart -= growth;
	for (p = tops; p; p = p->c_link) {
		if (p->c_onscreen != Nowhere)
			p->c_onscreen -= growth;
	}
}


/*
 * Make room for possible insertions.
 * (If a line is inserted, it may be necessary to delete lines
 * further on the screen.)
 */

Hidden int
makeroom(p, curlno, delcnt)
	register cell *p;
	register int curlno;
	register int delcnt;
{
	register int here = 0;
	register int needed = Space(p);
	register int amiss;
	int avail;
	int delta;

	Assert(p);
	do {
		p = p->c_link;
		if (!p)
			return delcnt;
	} while (p->c_onscreen == Nowhere);
	here = p->c_onscreen - delcnt;
	avail = here - curlno;
	amiss = needed - avail;
#ifndef NDEBUG
	if (dflag)
		debug("[makeroom: curlno=%d, delcnt=%d, here=%d, avail=%d, amiss=%d]",
			curlno, delcnt, here, avail, amiss);
#endif /* NDEBUG */
	if (amiss <= 0)
		return delcnt;
	if (amiss > delcnt) {
		for (; p; p = p->c_link) {
			if (p->c_onscreen != Nowhere) {
				delta = amiss-delcnt;
				if (p->c_onscreen - delcnt - here < delta)
					delta = p->c_onscreen - delcnt - here;
				if (delta > 0) {
					trmscrollup(here, winheight, delta);
					delcnt += delta;
				}
				p->c_onscreen += -delcnt + amiss;
				here = p->c_onscreen - amiss;
				if (p->c_onscreen >= winheight)
					p->c_onscreen = Nowhere;
			}
			here += Space(p);
		}
		/* Now for all p encountered whose p->c_onscreen != Nowhere,
		 * p->c_onscreen - amiss is its actual position.
		 */
		if (amiss > delcnt) {
			trmscrollup(winheight - amiss, winheight, amiss-delcnt);
			delcnt = amiss;
		}
	}
	/* Now amiss <= delcnt */
	trmscrollup(curlno + avail, winheight, -amiss);
	return delcnt - amiss;
}


/*
 * Addition to makeroom - make sure the status line is not overwritten.
 * Returns new delcnt, like makeroom does.
 */

Hidden int
make2room(p, curlno, delcnt)
	cell *p;
	int curlno;
	int delcnt;
{
	int nextline = curlno + Space(p);
	int sline = winheight - delcnt;
	int delta;

	if (sline < curlno) {
#ifndef NDEBUG
		debug("[Status line overwritten]");
#endif /* NDEBUG */
		return delcnt;
	}
	if (nextline > winheight)
		nextline = winheight;
	delta = nextline - sline;
	if (delta > 0) {
		trmscrollup(sline, winheight, -delta);
		delcnt -= delta;
	}
	return delcnt;
		
}


/*
 * Routine called for every change in the screen.
 */

Visible Procedure
virtupdate(oldep, newep, highest)
	environ *oldep;
	environ *newep;
	int highest;
{
	environ old;
	environ new;
	register int oldlno;
	register int newlno;
	register int oldlcnt;
	register int newlcnt;
	register int i;

	if (!oldep) {
		highest = 1;
		trmputdata(winstart, winheight, indent, "", (string)0);
		discard(tops);
		tops = Cnil;
		Ecopy(*newep, old);
	}
	else {
		Ecopy(*oldep, old);
	}
	Ecopy(*newep, new);

	savefocus(&new);

	oldlcnt = fixlevels(&old, &new, highest);
	newlcnt = -nodewidth(tree(new.focus));
	if (newlcnt < 0)
		newlcnt = 0;
	i = -nodewidth(tree(old.focus));
	if (i < 0)
		i = 0;
	newlcnt -= i - oldlcnt;
		/* Offset newlcnt as much as oldcnt is offset */
	
	oldlno = Ycoord(old.focus);
	newlno = Ycoord(new.focus);
	if (!atlinestart(&old))
		++oldlcnt;
	else
		++oldlno;
	if (!atlinestart(&new))
		++newlcnt;
	else
		++newlno;
	Assert(oldlno == newlno);

	tops = replist(tops, build(new.focus, newlcnt), oldlno, oldlcnt);

	setfocus(tops); /* Incorporate the information saved by savefocus */

	Erelease(old);
	Erelease(new);
}


Hidden bool
atlinestart(ep)
	environ *ep;
{
	register string repr = noderepr(tree(ep->focus))[0];

	return Fw_negative(repr);
}


/*
 * Make the two levels the same, and make sure they both are line starters
 * if at all possible.  Return the OLD number of lines to be replaced.
 * (0 if the whole unit has no linefeeds.)
 */

Hidden int
fixlevels(oldep, newep, highest)
	register environ *oldep;
	register environ *newep;
	register int highest;
{
	register int oldpl = pathlength(oldep->focus);
	register int newpl = pathlength(newep->focus);
	register bool intraline = No;
	register int w;

	if (oldpl < highest)
		highest = oldpl;
	if (newpl < highest)
		highest = newpl;
	while (oldpl > highest) {
		if (!up(&oldep->focus)) Abort();
		--oldpl;
	}
	while (newpl > highest) {
		if (!up(&newep->focus)) Abort();
		--newpl;
	}
	if (Ycoord(newep->focus) != Ycoord(oldep->focus) ||
		Level(newep->focus) != Level(oldep->focus)) {
		/* Inconsistency found.  */
		Assert(highest > 1); /* Inconsistency at top level. Stop. */
		return fixlevels(oldep, newep, 1); /* Try to recover. */
	}
	intraline = nodewidth(tree(oldep->focus)) >= 0
		&& nodewidth(tree(newep->focus)) >= 0;
	while (!atlinestart(oldep) || !atlinestart(newep)) {
		/* Find beginning of lines for both */
		if (!up(&newep->focus)) {
			Assert(!up(&newep->focus));
			break;
		}
		--oldpl;
		if (!up(&oldep->focus)) Abort();
		--newpl;
	}
	if (intraline)
		return atlinestart(oldep);
	w = nodewidth(tree(oldep->focus));
	return w < 0 ? -w : 0;
}


/*
 * Initialization code.
 */
 
Visible Procedure
initterm()
{
	initvtrm(); /* init virtual terminal package */
	initgetc(); /* term-init string */
}


Visible bool vtrmactive= No;
extern bool in_init;

Hidden Procedure
initvtrm() 
{
	int flags = 0;
	int err;
	
	err= trmstart(&winheight, &llength, &flags);
	if (err != TE_OK) {
		if (err <= TE_DUMB)
	putmess(MESS(6600, "*** Bad $TERM or termcap, or dumb terminal\n"));
		else if (err == TE_BADSCREEN)
	putmess(MESS(6601, "*** Bad SCREEN environment\n"));
		else
	putmess(MESS(6602, "*** Cannot reach keyboard or screen\n"));

		if (in_init)
			immexit(2);
		else
			bye(2);
	}
	noscroll = (flags&CAN_SCROLL) == 0;

	winstart = --winheight;

	init_interpreter_output(winheight, llength);

	vtrmactive= Yes;
}

Visible Procedure
endterm()
{
	trmsync(winheight, 0);	/* needed for buggy vt100's, that
				 * may leave cusor at top of screen
				 * if only trmstart was called
				 * (which did send cs_str)
				 */
	endgetc(); /* term-end string */
	trmend();
	vtrmactive= No;
}

/*
 * Routine to move the cursor to the first line after the just edited
 * document.  (Called after each editing action.)
 */

Visible Procedure
endshow()
{
	register cell *p;
	register int last = winheight;

	for (p = tops; p; p = p->c_link) {
		if (p->c_onscreen != Nowhere)
			last = p->c_onscreen + Oldspace(p);
	}
	if (last > winheight)
		last = winheight;
	discard(tops);
	tops = Cnil;
	trmputdata(last, winheight, 0, "", (string)0);
	trmsync(winheight, 0);

	re_interpreter_output();
}

#ifdef GOTOCURSOR

/*
 * Translate a cursor position in tree coordinates.
 *
 * ***** DOESN'T WORK IF SCREEN INDENT DIFFERS FROM TREE INDENT! *****
 * (I.e. for lines with >= 80 spaces indentation)
 */

Visible bool
backtranslate(py, px)
	int *py;
	int *px;
{
	cell *p;
	int y = *py;
	int x = *px;
	int i;

	for (i = 0, p = tops; p; ++i, p = p->c_link) {
		if (p->c_onscreen != Nowhere
			&& y >= p->c_onscreen && y < p->c_onscreen + Space(p)) {
			*px += (y - p->c_onscreen) * llength - indent;
			if (*px < 0)
				*px = 0;
			*py = i;
			if (p->c_oldvhole && (y > focy || y == focy && x > focx))
				--*px; /* Correction if beyond Vhole on same logical line */
			return Yes;
		}
	}
	ederr(GOTO_OUT);
	return No;
}

#endif /*GOTOCURSOR*/
/*
 * Set the indent level and window start line.
 */

Visible Procedure
setindent(x)
	int x;
{
	winstart= winheight-1;
	/* the following is a hack; should change when
	 * interpreter also writes through trm-interface.
	 * Then it must be clear what's on the screen already
	 * Handled in this file?
	 */
	if (llength==0)
		indent= x;
	else {
		indent= x % llength;
		if (indent == 0 && x != 0)
		  trmscrollup(0, winstart, 1);
	}
}


/*
 * Show the command prompt.
 */

Visible Procedure cmdprompt(prompt)
	string prompt;
{
	setindent(strlen(prompt));
	trmputdata(winstart, winstart, 0, prompt, (string)0);
}
