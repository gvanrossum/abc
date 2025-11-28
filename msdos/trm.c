/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1986. */

/*
 * ibm Pc virtual TeRMinal package.
 *
 * (Under reconstruction by Guido!)
 *
 * An implementation of the VTRM interface for MS-DOS machines.
 *
 * This code supports two modes of accessing the screen.
 * The first one (BIOS) will be used, unless the user overwrites this
 * by setting the SCREEN environment variable.
 * This variable can also be used to convey a screen size that differs
 * from the default 25 lines and 80 columns. See below.
 *
 * The two modes are:
 *
 * 1) IBM BIOS interrupt 10 hex, video io.
 *    (See IBM PC XT Technical Reference 6936833, May 1983,
 *     Appendix A, pages A46-A47).
 *    This is what you really want to use, since it's the only one that
 *    can decently scroll. It cannot insert or delete characters, so
 *    most optimisations from vtrm.c are useless and taken out.
 *    Unfortunately, not every PC-compatible machine supports this BIOS
 *    interrupt, so for these unlucky souls there is the following escape:
 *
 * 2) The ANSI.SYS driver.
 *    (See IBM MS-DOS 6936839, Jan 1983, Version 2.00, Chapter 13.)
 *    (Some compatibles don't have a separate ANSI.SYS driver but do the
 *    same escape interpretation by default.)
 *    This works reasonably, apart from scrolling downward, or part of
 *    the screen, which is clumsy.
 *    (The ANSI standard provides an escape sequence for scrolling
 *    but ANSI.SYS does not support it, nor any other way of scrolling.)
 *
 * The rest of the interface is the same as described in vtrm.c,
 * with the following exceptions:
 *    - to ease coding for ansi scrolls, the terminal is supposed to
 *	contain blanks at positions that were not written yet;
 *	the unknown rubbish that is initially on the screen can
 *	only be cleared by the caller by scrolling the whole screen up
 *	by one or more lines;
 *    - the number of lines on the terminal is assumed to be 25;
 *	the number of columns is (1) determined by a BIOS function, or
 *	(2) assumed to be 80 for ANSI;
 *	the user can overwrite this by setting the environment variable:
 *
 *		SET SCREEN=BIOS y x
 *	or
 *		SET SCREEN=ANSI y x
 *
 *	where x and y are the number of lines and columns respectively.
 *
 * The lines and columns of our virtual terminal are numbered
 *	y = {0...lines-1} from top to bottom, and
 *	x = {0...cols-1} from left to right,
 * respectively.
 *
 * The Visible Procedures in this package are as described in vtrm.c.
 *
 */

/*
 * Includes and data definitions.
 */

#include "b.h"
#include "trm.h"

#include <dos.h>
#include <fcntl.h>
#include <io.h>
#include <process.h>  /* for P_WAIT */

#ifdef GFX
#include "bgfx.h"
#endif

Forward Hidden Procedure put_line();
Forward Hidden Procedure put_str();
Forward Hidden Procedure move();
Forward Hidden Procedure standout();
Forward Hidden Procedure standend();
Forward Hidden Procedure clear_lines();
Forward Hidden Procedure clr_to_eol();
Forward Hidden Procedure biosscrollup();
Forward Hidden Procedure scr_lines();
Forward Hidden Procedure lf_scroll();
Forward Hidden Procedure move_lines();
Forward Hidden Procedure ring_bell();
Forward Hidden Procedure setraw();
Forward Hidden Procedure setbreak();

#ifdef VTRMTRACE
extern FILE *vtrmfp;
#endif

#define STDIN_HANDLE 0

#define Undefined (-1)

#define Min(a,b) ((a) <= (b) ? (a) : (b))

/* terminal status */

Hidden int started = No;

Hidden int scr_mode = 0;
#define ANSI 'A'
#define BIOS 'B'

#define Nlines	25
#define Ncols	80
Hidden int lines = Nlines;
Hidden int cols = Ncols;
Hidden int flags = 0;

/* current standout mode */
#define Off	PLAIN
#define On	STANDOUT
Hidden int so_mode = Off;

/* current cursor position */
Hidden intlet cur_y = Undefined, cur_x = Undefined;

/* "linedata[y][x]" holds the char on the terminal,
 * "linemode[y][x]" the STANDOUT bit.
 * The STANDOUT bit tells whether the character is standing out.
 * "lenline[y]" holds the length of the line.
 * (Partially) empty lines are distinghuished by "lenline[y] < cols".
 * Unknown chars will be ' ', so the scrolling routines for ANSI
 * can use "unwritten" chars (with indent > 0 in trmputdata).
 * To make the optimising compare in putline fail, lenline[y] is initially 0.
 * The latter implies that if a line is first addressed with trmputdata,
 * any rubbish that is on the screen beyond the data that gets put, will
 * remain there.
 */

Hidden char 	**linedata = 0;
Hidden char 	**linemode = 0;

Hidden intlet *lenline = 0;

/* To compare the mode part of the line when the
 * mode parameter of trmputdata == NULL, we use the following:
 */
Hidden char plain[1]= {PLAIN};

/* Make the cursor invisible when trmsync() tries to move outside the screen */
Hidden bool no_cursor = No;

/*
 * Starting, Ending and (fatal) Error.
 */

Hidden bool wasbreak;

/*
 * Initialization call.
 * Determine terminal mode.
 * Start up terminal and internal administration.
 * Return Yes if succeeded, No if trouble (which doesn't apply here).
 */

Visible int trmstart(plines, pcols, pflags) int *plines, *pcols, *pflags; {
	static char setup = No;
	int err;

#ifdef VTRMTRACE
	if (vtrmfp) fprintf(vtrmfp, "\ttrmstart(&li, &co, &fl);\n");
#endif

	if (started)
		return TE_TWICE;

#ifdef GFX
	if (gfx_mode != TEXT_MODE)
		gfx_mode= SPLIT_MODE;
#endif

	if (!setup) {
		err= set_screen_up();
		if (err != TE_OK)
			return err;
		setup = Yes;
	}

	err= start_trm();		/* internal administration */
	if (err != TE_OK)
		return err;

	*plines = lines;
	*pcols = cols;
	*pflags = flags;

	setmode(STDIN_HANDLE, O_BINARY); /* Don't translate CRLF to LF */
	setraw(STDIN_HANDLE, Yes);
	wasbreak= getbreak(); /* Save BREAK status; restore when done */
	setbreak(No);

#ifdef NOT_USED
	set_handler();
#endif
	started = Yes;
	return TE_OK;
}

Hidden int set_screen_up() {
	int height;
	int width;
	int get_screen_env();
	int get_cols();

	height = width = 0;
	scr_mode = get_screen_env(&height, &width);

	switch (scr_mode) {
	case BIOS:
	case TE_OK:
		cols = get_cols();
		flags = HAS_STANDOUT|CAN_SCROLL;
		break;
	case ANSI:
		flags = HAS_STANDOUT;
		break;
	default:
		return scr_mode; /* Error flag */
	}

	/* allow x and y in environment variable SCREEN to override */
	if (height > 0)
		lines = height;
	if (width > 0)
		cols = width;
	return TE_OK;
}

Hidden int get_screen_env(pheight, pwidth) int *pheight, *pwidth; {
	string s;
	int mode;
	char screrr;
	string getenv();
	string strip();
	string skip();

	screrr = No;
	s = getenv("SCREEN");
	if (s == NULL)
		return BIOS;

	s = strip(s);
	switch (*s) {
	case '\0':
		return BIOS;
	case 'a':
		mode = ANSI;
		s = skip(s, "ansi");
		break;
	case 'A':
		mode = ANSI;
		s = skip(s, "ANSI");
		break;
	case 'b':
		mode = BIOS;
		s = skip(s, "bios");
		break;
	case 'B':
		mode = BIOS;
		s = skip(s, "BIOS");
		break;
	default:
		mode = BIOS;
		screrr = Yes;
	}

	/* *pheight and *pwidth were set to 0 above */
	s = strip(s);
	while (isdigit(*s)) {
		*pheight = *pheight * 10 + (*s++ - '0');
	}
	s = strip(s);
	while (isdigit(*s)) {
		*pwidth = *pwidth * 10 + (*s++ -'0');
	}
	s = strip(s);
	if (screrr || *s != '\0')
		return TE_BADSCREEN;

	return mode;
}

Hidden string strip(s) string s; {
	while (*s == ' ' || *s == '\t')
		++s;
	return s;
}

Hidden string skip(s, pat) string s, pat; {
	while (*s && *s == *pat)
		++s, ++pat;
	return s;
}

/* initialise internal administration */

Hidden int start_trm() {
	register int y;

	if (linedata == NULL) {
		if ((linedata = (char**) malloc(MALLOC_ARG(lines * sizeof(char*)))) == NULL)
			return TE_NOMEM;
		for (y = 0; y < lines; y++) {
			if ((linedata[y] = (char*) malloc(MALLOC_ARG(cols * sizeof(char)))) == NULL)
				return TE_NOMEM;
		}
	}
	if (linemode == NULL) {
		if ((linemode = (char**) malloc(MALLOC_ARG(lines * sizeof(char*)))) == NULL)
			return TE_NOMEM;
		for (y = 0; y < lines; y++) {
			if ((linemode[y] = (char*) malloc(MALLOC_ARG(cols * sizeof(char)))) == NULL)
				return TE_NOMEM;
		}
	}
	if (lenline == 0) {
		if ((lenline = (intlet *) malloc(MALLOC_ARG(lines * sizeof(intlet)))) == NULL)
			return TE_NOMEM;
	}
		
	trmundefined();
	return TE_OK;
}

/*
 * Termination call.
 * Beware that it might be called by a catched interrupt even in the middle
 * of trmstart()!
 */

Visible Procedure trmend() {
#ifdef VTRMTRACE
	if (vtrmfp) fprintf(vtrmfp, "\ttrmend();\n");
#endif
	if (started && so_mode != Off)
		standend();
	if (scr_mode == ANSI) {
		VOID fflush(stdout);
	}
	/* Always turn off RAW mode -- it is unlikely that anybody
	   would want to interface to COMMAND.COM in raw mode.
	   This way, if you were accidentally left in RAW mode
	   because of a crash, it will go away if you re-enter. */
	setraw(STDIN_HANDLE, No);
	setbreak(wasbreak);

	started = No;
}

/*
 * Set all internal statuses to undefined, especially the contents of
 * the screen, so a hard redraw will not be optimised to heaven.
 */

Visible Procedure trmundefined() {
	register int y, x;
#ifdef VTRMTRACE
	if (vtrmfp) fprintf(vtrmfp, "\ttrmundefined();\n");
#endif

	cur_y = cur_x = Undefined;
	so_mode = Undefined;

	for (y = 0; y < lines; y++) {
		for (x = 0; x < cols; x++) {
			linedata[y][x] = ' ';
			linemode[y][x] = PLAIN;
		}
			/* they may get printed in scrolling */
		lenline[y] = 0;
	}
}

#ifdef DEBUG
Hidden Procedure check_started(m) char *m; {
	if (!started) {
		putsSerr("Not started: %s\n", m);
		immexit(TE_TWICE);
	}
}
#else
#define check_started(m) /*empty*/
#endif

/*
 * Sensing the cursor.
 * (NOT IMPLEMENTED, since there is no way to locally move the cursor.)
 */

/*
 * Sense the current (y, x) cursor position, after a possible manual
 * change by the user with local cursor motions.
 * If the terminal cannot be asked for the current cursor position,
 * or if the string returned by the terminal is garbled,
 * the position is made Undefined.
 */
Visible Procedure trmsense(py, px) int *py, *px; {
/*	bool getpos(); */
#ifdef VTRMTRACE
	if (vtrmfp) fprintf(vtrmfp, "\ttrmsense(&yy, &xx);\n");
#endif
	check_started(MESS(7900, "trmsense called outside trmstart/trmend"));

/*	*py = *px = Undefined;
 *
 *
 *	if (flags&CAN_SENSE && get_pos(py, px)) {
 *		if (*py < 0 || lines <= *py || *px < 0 || cols <= *px)
 *			*py = *px = Undefined;
 *	}
 */
	*py = *px= Undefined;
}

/*
 * Putting data on the screen.
 */

/*
 * Fill screen area with given "data".
 * Characters for which the corresponding chars in "mode" have the value
 * STANDOUT must be put in inverse video.
 */
Visible Procedure trmputdata(yfirst, ylast, indent, data, mode)
	int yfirst, ylast; register int indent;
	register string data; string mode; {
	register int y;
	int x, len, lendata, space;

#ifdef VTRMTRACE
	if (vtrmfp) fprintf(vtrmfp, "\ttrmputdata(%d, %d, %d, \"%s\");\n",
				yfirst, ylast, indent, data);
#endif
	check_started(MESS(7901, "trmputdata called outside trmstart/trmend"));

	if (yfirst < 0)
		yfirst = 0;
	if (ylast >= lines)
		ylast = lines-1;
	space = cols*(ylast-yfirst+1) - indent;
	if (space <= 0)
		return;
	yfirst += indent/cols;
	indent %= cols;
	y = yfirst;
	if (data) {
		x = indent;
		lendata = strlen(data);
		if (ylast == lines-1 && lendata >= space)
			lendata = space - 1;
		len = Min(lendata, cols-x);
		while (/*** len > 0 && ***/ y <= ylast) {
			put_line(y, x, data, mode, len);
			y++;
			lendata -= len;
			if (lendata > 0) {
				x = 0;
				data += len;
				if (mode != NULL)
					mode += len;
				len = Min(lendata, cols);
			}
			else
				break;
		}
	}
	if (y <= ylast)
		clear_lines(y, ylast);
}

/*
 * We will try to get the picture:
 *
 *		    op>>>>>>>>>>>op				       oq
 *		    ^		 ^				       ^
 *	     <xskip><-----m1----><---------------od-------------------->
 *   OLD:   "You're in a maze of twisty little pieces of code, all alike"
 *   NEW:	   "in a maze of little twisting pieces of code, all alike"
 *		    <-----m1----><----------------nd--------------------->
 *		    ^		 ^					 ^
 *		    np>>>>>>>>>>>np					 nq
 * where
 *	op, oq, np, nq are pointers to start and end of Old and New data,
 * and
 *	xskip = length of indent to be skipped,
 *	m1 = length of Matching part at start,
 *	od = length of Differing end on screen,
 *	nd = length of Differing end in data to be put.
 */
Hidden Procedure put_line(y, xskip, data, mode, len)
     int y, xskip;
     string data;
     string mode;
     int len;
{
	register char *op, *oq, *mp;
	char *np, *nq, *mo;
	int m1, od, nd, delta;

	/* Bugfix GvR 19-June-87: */
	while (lenline[y] < xskip) {
		linedata[y][lenline[y]] = ' ';
		linemode[y][lenline[y]] = PLAIN;
		lenline[y]++;
	}
	
	/* calculate the magic parameters */
	op = &linedata[y][xskip];
	oq = &linedata[y][lenline[y]-1];
	mp = &linemode[y][xskip];
	np = data;
	nq = data + len - 1;
	mo = (mode != NULL ? mode : plain);
	m1 = 0;
	while (*op == *np && *mp == *mo && op <= oq && np <= nq) {
		op++, np++, mp++, m1++;
		if (mode != NULL) mo++;
	}
	od = oq - op + 1;
	nd = nq - np + 1;
	/* now we have the picture above */

	if (od==0 && nd==0)
		return;

	delta = nd - od;
	move(y, xskip + m1);
	if (nd > 0) {
		mo= (mode != NULL ? mode+(np-data) : NULL);
		put_str(np, mo, nd);
	}
	if (delta < 0) {
		clr_to_eol();
		return;
	}
	lenline[y] = xskip + len;
	if (cur_x == cols) {
		cur_y++;
		cur_x = 0;
	}
}

/*
 * Scrolling (part of) the screen up (or down, dy<0).
 */

Visible Procedure trmscrollup(yfirst, ylast, by) register int yfirst;
		register int ylast; register int by; {
#ifdef VTRMTRACE
	if (vtrmfp) fprintf(vtrmfp, "\ttrmscrollup(%d, %d, %d);\n", yfirst, ylast, by);
#endif
	check_started(MESS(7902, "trmscrollup called outside trmstart/trmend"));

	if (by == 0)
		return;

	if (yfirst < 0)
		yfirst = 0;
	if (ylast >= lines)
		ylast = lines-1;

	if (yfirst > ylast)
		return;

	if (so_mode != Off)
		standend();

	if (by > 0 && yfirst + by > ylast
	    ||
	    by < 0 && yfirst - by > ylast)
	{
		clear_lines(yfirst, ylast);
		return;
	}

	switch (scr_mode) {
	case BIOS:
		biosscrollup(yfirst, ylast, by);
		break;
	case ANSI:
		if (by > 0 && yfirst == 0) {
			lf_scroll(ylast, by);
		}
		else if (by > 0) {
			move_lines(yfirst+by, yfirst, ylast-yfirst+1-by, 1);
			clear_lines(ylast-by+1, ylast);
		}
		else {
			move_lines(ylast+by, ylast, ylast-yfirst+1+by, -1);
			clear_lines(yfirst, yfirst-by-1);
		}
		break;
	}
}

/*
 * Synchronization, move cursor to given position (or previous if < 0).
 */

Visible Procedure trmsync(y, x) int y, x; {
#ifdef VTRMTRACE
	if (vtrmfp) fprintf(vtrmfp, "\ttrmsync(%d, %d);\n", y, x);
#endif
	check_started(MESS(7903, "trmsync called outside trmstart/trmend"));

	if (0 <= y && y < lines && 0 <= x && x < cols) {
		move(y, x);
	}
	VOID fflush(stdout);
}

/*
 * Send a bell, visible if possible.
 */

Visible Procedure trmbell() {
#ifdef VTRMTRACE
	if (vtrmfp) fprintf(vtrmfp, "\ttrmbell();\n");
#endif
	check_started(MESS(7904, "trmbell called outside trmstart/trmend"));
	ring_bell();
}

/*
 * Now for the real work: here are all low level routines that really
 * differ for BIOS or ANSI mode.
 */

/*
 * BIOS video io is called by generating an 8086 software interrupt,
 * using lattice's int86() function.
 * To ease coding, all routines fill in the apropriate parameters in regs,
 * and then call bios10(code), where code is to be placed in ah.
 */

Hidden union REGS regs, outregs;

/* A macro for speed  */
#define bios10(code) (regs.h.ah = (code), int86(0x10, &regs, &regs))
#define nbios10(code) (regs.h.ah = (code), int86(0x10, &regs, &outregs))

/* Video attributes: (see the BASIC manual) (used for standout mode) */

Hidden int video_attr;
#ifndef GFX
#define V_NORMAL 7
#else
#define V_NORMAL (gfx_mode == TEXT_MODE ? 7 : 0)
#endif
#define V_STANDOUT (7<<4)

/* Some BIOS only routines */

Hidden get_cols() {
	bios10(15);
	return regs.h.ah;
}

/*
 * ANSI escape sequences
 */
#define A_CUP	"\033[%d;%dH"   /* cursor position */
#define A_SGR0	"\033[0m"       /* set graphics rendition to normal */
#define A_SGR7	"\033[7m"       /* set graphics rendition to standout */
#define A_ED	"\033[2J"       /* erase display (and cursor home) */
#define A_EL	"\033[K"        /* erase (to end of) line */

/*
 * The following routine is the time bottleneck, I believe!
 */

Hidden Procedure put_str(data, mode, n) char *data, *mode; int n; {
	register char c, mo, so;

	so = so_mode;
	if (scr_mode == BIOS) {
		regs.x.cx = 1;	/* repition count */
		regs.h.bh = 0;	/* page number */
		regs.h.bl = video_attr;
		while (--n >= 0) {
			c = *data++;
			mo= (mode != NULL ? *mode++ : PLAIN);
			if (mo != so) {
				so= mo;
				so ? standout() : standend();
				regs.h.bl = video_attr;
			}
			regs.h.al = c /* &CHAR */;
			nbios10(9);
			if (cur_x >= cols-1) {
				linedata[cur_y][cols-1] = c;
				linemode[cur_y][cols-1] = mo;
				continue;
			}
			regs.h.dh = cur_y;
			regs.h.dl = cur_x + 1;
			nbios10(2);
			linedata[cur_y][cur_x] = c;
			linemode[cur_y][cur_x]= mo;
			cur_x++;
		}
	}
	else {
		while (--n >= 0) {
			c = *data++;
			mo= (mode != NULL ? *mode++ : PLAIN);
			if (mo != so) {
				so= mo;
				so ? standout() : standend();
			}
			putch(c);
			linedata[cur_y][cur_x] = c;
			linemode[cur_y][cur_x]= mo;
			cur_x++;
		}
	}
}

/*
 * Move to position y,x on the screen
 */

Hidden Procedure move(y, x) int y, x; {
	if (scr_mode != BIOS && cur_y == y && cur_x == x)
		return;
	switch (scr_mode) {
	case BIOS:
		regs.h.dh = y;
		regs.h.dl = x;
		regs.h.bh = 0; /* Page; must be 0 for graphics */
		bios10(2);
		break;
	case ANSI:
		cprintf(A_CUP, y+1, x+1);
		break;
	}
	cur_y = y;
	cur_x = x;
}

Hidden Procedure standout() {
	so_mode = On;
	switch (scr_mode) {
	case BIOS:
		video_attr = V_STANDOUT;
		break;
	case ANSI:
		cputs(A_SGR7);
		break;
	}
}

Hidden Procedure standend() {
	so_mode = Off;
	switch (scr_mode) {
	case BIOS:
		video_attr = V_NORMAL;
		break;
	case ANSI:
		cputs(A_SGR0);
		break;
	}
}

Hidden Procedure clear_lines(yfirst, ylast) int yfirst, ylast; {
	register int y;

	if (scr_mode == BIOS) {
		regs.h.al = 0;	/* scroll with al = 0 means blank window */
		regs.h.ch = yfirst;
		regs.h.cl = 0;
		regs.h.dh = ylast;
		regs.h.dl = cols-1;
		regs.h.bh = V_NORMAL;
		bios10(6);
		for (y = yfirst; y <= ylast; y++)
			lenline[y] = 0;
		return;
	}
	/* scr_mode == ANSI */
	if (yfirst == 0 && ylast == lines-1) {
		if (so_mode == On)
			standend();
		move(0, 0);		/* since some ANSI'd don't move */
		cputs(A_ED);
		cur_y = cur_x = 0;
		for (y = yfirst; y < ylast; y++)
			lenline[y] = 0;
		return;
	}
	for (y = yfirst; y <= ylast; y++) {
		if (lenline[y] > 0) {
			move(y, 0);
			clr_to_eol();
		}
	}
}

Hidden Procedure clr_to_eol() {
	if (so_mode == On)
		standend();
	switch (scr_mode) {
	case BIOS:
		regs.h.bh = 0;	/* page */
		regs.x.cx = lenline[cur_y] - cur_x;
		regs.h.al = ' ';
		regs.h.bl = V_NORMAL;
		bios10(9);
		break;
	case ANSI:
		cputs(A_EL);
		break;
	}
	lenline[cur_y] = cur_x;
}

/* scrolling for BIOS */

Hidden Procedure biosscrollup(yfirst, ylast, by) int yfirst, ylast, by; {
	regs.h.al = (by < 0 ? -by : by);
	regs.h.ch = yfirst;
	regs.h.cl = 0;
	regs.h.dh = ylast;
	regs.h.dl = cols-1;
	regs.h.bh= V_NORMAL;
	bios10(by < 0 ? 7 : 6);
	cur_y = cur_x = Undefined;
	if (by > 0)
		scr_lines(yfirst, ylast, by, 1);
	else
		scr_lines(ylast, yfirst, -by, -1);
}

/* Reset internal administration accordingly */

Hidden Procedure scr_lines(yfrom, yto, n, dy) int yfrom, yto, n, dy; {
	register int y, x;
	char *savedata;
	char *savemode;

	while (n-- > 0) {
		savedata = linedata[yfrom];
		savemode= linemode[yfrom];
		for (y = yfrom; y != yto; y += dy) {
			linedata[y] = linedata[y+dy];
			linemode[y] = linemode[y+dy];
			lenline[y] = lenline[y+dy];
		}
		linedata[yto] = savedata;
		linemode[yto] = savemode;
		for (x = 0; x < cols; x++ ) {
			linedata[yto][x] = ' ';
			linemode[yto][x] = PLAIN;
		}
		lenline[yto] = 0;
	}
}

Hidden Procedure lf_scroll(yto, by) int yto, by; {
	register int n = by;

	move(lines-1, 0);
	while (n-- > 0) {
		putch('\n');
	}
	scr_lines(0, lines-1, by, 1);
	move_lines(lines-1-by, lines-1, lines-1-yto, -1);
	clear_lines(yto-by+1, yto);
}

/* for dumb scrolling, uses and updates internal administration */

Hidden Procedure move_lines(yfrom, yto, n, dy) int yfrom, yto, n, dy; {
	while (n-- > 0) {
		put_line(yto, 0, linedata[yfrom], linemode[yfrom], lenline[yfrom]);
		yfrom += dy;
		yto += dy;
	}
}

Hidden Procedure ring_bell() {
	switch (scr_mode) {
	case BIOS:
		regs.h.al = '\007';
		regs.h.bl = V_NORMAL;
		bios10(14);
		break;
	case ANSI:
		putch('\007');
		break;
	}
}

/*
 * Show the current internal statuses of the screen on vtrmfp.
 * For debugging only.
 */

#ifdef SHOW
Visible Procedure trmshow(s) char *s; {
	int y, x;

	if (!vtrmfp)
		return;
	fprintf(vtrmfp, "<<< %s >>>\n", s);
	for (y = 0; y < lines; y++) {
		for (x = 0; x <= lenline[y] /*** && x < cols ***/ ; x++) {
			fputc(linedata[y][x], vtrmfp);
		}
		fputc('\n', vtrmfp);
		for (x = 0; x <= lenline[y] && x < cols-1; x++) {
			if (linemode[y][x] == STANDOUT)
				fputc('-', vtrmfp);
			else
				fputc(' ', vtrmfp);
		}
		fputc('\n', vtrmfp);
	}
	fprintf(vtrmfp, "CUR_Y = %d, CUR_X = %d.\n", cur_y, cur_x);
	VOID fflush(vtrmfp);
}
#endif

/*
 * Interrupt handling.
 *
 * (This has not properly been tested, nor is it clear that
 * this interface is what we want.  Anyway, it's here for you
 * to experiment with.  What does it do, you may ask?
 * Assume an interactive program which reads its characters
 * through trminput.  Assume ^C is the interrupt character.
 * Normally, ^C is treated just like any other character: when
 * typed, it turns up in the input.  The program may understand
 * input ^C as "quit from the current mode".
 * Occasionally, the program goes into a long period of computation.
 * Now it would be uninterruptible, except if it calls pollinterrupt()
 * [in bed/e1getc.c] at times in its computational loop;
 * pollinterrupt() magically looks ahead in the input queue, 
 * via trmavail() and trminput(), and if it sees a ^C, discards all input
 * before that point and returns Yes.  It also sets a flag, so that
 * the interupt "sticks around" until either trminput or trmavail
 * is called.  It is undefined whether typing ^C several times in
 * a row is seen as one interrupt, or an interrupt followed by input
 * of ^C's.  A program should be prepared for either.)
 */

#ifdef NOT_USED

extern bool intrflag;

bool trminterrupt() {
	/* Force a check for ^C which will call handler. */
	/* (This does a ^C check even if stdin is in RAW mode. */
	(void) kbhit();
	return intrflag;
}

#endif /* NOT_USED */


/* Definitions for DOS function calls. */

#define IOCTL		0x44
#define IOCTL_GETDATA	0x4400
#define IOCTL_SETDATA	0x4401
#define DEVICEBIT	0x80
#define RAWBIT		0x20

#define BREAKCK		0x33
#define GET		0x00
#define SET		0x01

#define IOCTL_GETSTS	0x4406

/* #define STDIN_HANDLE	0 */

/*
 * Terminal input without echo.
 */

Visible int trminput() {
	char c;

#ifdef NOT_USED
	intrflag= No;
#endif
	/* Assume stdin is in RAW mode; this turns echo and ^C checks off. */
	if (read(STDIN_HANDLE, &c, 1) < 1)
		return -1;
	else
		return c;
}

/*
 * Check for character available.
 *
 */

Visible bool trmavail() {
#ifdef NOT_USED
	intrflag= No;
#endif
	regs.x.ax= IOCTL_GETSTS;
	regs.x.bx= STDIN_HANDLE;
	intdos(&regs, &regs);
	if (regs.x.cflag)
		return -1; /* Error */
	return regs.h.al != 0;
}

/* Issue an IOCTL to turn RAW for a device on or off. */

Hidden Procedure setraw(handle, raw) int handle; bool raw; {
	regs.x.ax= IOCTL_GETDATA;
	regs.x.bx= handle;
	intdos(&regs, &regs);
	if (regs.x.cflag || !(regs.h.dl & DEVICEBIT))
		return; /* Error or not a device -- ignore it */
	regs.h.dh= 0;
	if (raw)
		regs.h.dl |= RAWBIT;
	else
		regs.h.dl &= ~RAWBIT;
	regs.x.ax= IOCTL_SETDATA;
	intdos(&regs, &regs);
	/* Ignore errors */
}

/* Get the raw bit of a device. */

Hidden int getraw(handle) int handle; {
	regs.x.ax= IOCTL_GETDATA;
	regs.x.bx= handle;
	intdos(&regs, &regs);
	return !regs.x.cflag &&
		(regs.h.dh & (DEVICEBIT|RAWBIT)) == (DEVICEBIT|RAWBIT);
}

/* Set the break status. */

Hidden Procedure setbreak(on) bool on; {
	bdos(BREAKCK, on, SET);
}

/* Get the break status. */

Hidden int getbreak() {
	regs.x.ax= (BREAKCK << 8) | GET;
	intdos(&regs, &regs);
	return regs.h.dl;
}

Visible int trmsuspend() {
	return spawnlp(P_WAIT, "COMMAND.COM", NULL) == 0;
}
