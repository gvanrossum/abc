/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1990. */

/* Generic version, as basis for porting.
 * See Portability Guide (./PORTING.DOC) for details.
 */

/* Implements vterm virtual terminal package for vt100's.
 * To do: tidy up the hidemouse() showmouse() stuff;
 */

#define hidemouse() {}
#define showmouse() {}

#include "b.h"
#include "trm.h"

/*****************************************************************************/

/* If you can't lookahead in the system's input queue, and so can't implement
 * trmavail(), you have to enable keyboard interrupts.
 * Otherwise, computations are uninterruptable.
 */

#ifndef CANLOOKAHEAD

#ifdef SIGNAL
#ifndef KEYS          /* only for abc, not for the abckeys program */
#define CATCHINTR
#include <signal.h>
#endif
#endif

#endif /* !CANLOOKAHEAD */

/*****************************************************************************/

#define Min(a,b) ((a) <= (b) ? (a) : (b))
#define Undefined (-1)

/* terminal status */
Hidden bool started = No;
Hidden int lines;
Hidden int cols;
Hidden int flags;

/* current cursor position */
Hidden intlet cur_y = Undefined, cur_x = Undefined;
Hidden bool auto_margins; /* Whether the term has automatic margins */

/* current standout mode */
#define Off	PLAIN
#define On	STANDOUT
Hidden int so_mode = Off;

/* "linedata[y][x]" holds the char on the terminal,
 * "linemode[y][x]" the STANDOUT bit.
 * The STANDOUT bit tells whether the character is standing out.
 * "lenline[y]" holds the length of the line.
 * (Partially) empty lines are distinguished by "lenline[y] < cols".
 * Unknown chars will be ' ', so the scrolling routines
 * can use "unwritten" chars (with indent > 0 in trmputdata).
 * To make the optimising compare in putline fail, lenline[y] is initially 0.
 * The latter implies that if a line is first addressed with trmputdata,
 * any rubbish that is on the screen beyond the data that gets put, will
 * remain there.
 */

Hidden char 	**linedata = 0;
Hidden char 	**linemode = 0;

Hidden intlet	*lenline = 0;

/* To compare the mode part of the line when the
 * mode parameter of trmputdata == NULL, we use the following:
 */
Hidden char plain[1]= {PLAIN};

Hidden FILE *scrfp = NULL; /* file pointer to write to the screen */
#define low_put_char(c) fputc(c, scrfp)
#define low_putstr(s) fputs(s, scrfp)

/*
 * Starting, Ending and (fatal) Error.
 */

/*
 * Initialization call.
 * Determine terminal mode.
 * Start up terminal and internal administration.
 * Return TE_OK if succeeded, error code (see trm.h) if trouble
 */

Visible int trmstart(plines, pcols, pflags)
     int *plines;
     int *pcols;
     int *pflags;
{
	int err;

	if (started) return TE_TWICE;

	err = start_trm();
	if (err != TE_OK) {
		trmend();
		return err;
	}

	*plines = lines;
	*pcols = cols;
	*pflags = flags;

	started = Yes;

	trmsync(lines-1, 0);		/* Position to end of screen */

	return TE_OK;
}

Hidden int start_trm()
{
	static char setup = No;
	int err;

	if (!setup) {
		err = low_trmstart();
		if (err != TE_OK) return err;
		setup = Yes;
	}

	err = init_trm();		/* internal administration */
	if (err != TE_OK) return err;

	return TE_OK;
}

/* initialise internal administration */

Hidden int init_trm()
{
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
		if ((linemode = (char**) malloc(MALLOC_ARG(MALLOC_ARG(lines * sizeof(char*))))) == NULL)
			return TE_NOMEM;
		for (y = 0; y < lines; y++) {
			if ((linemode[y] = (char*) malloc(MALLOC_ARG(cols * sizeof(char)))) == NULL)
				return TE_NOMEM;
		}
	}
	if (lenline == 0) {
		if ((lenline = (intlet *)
				malloc(MALLOC_ARG(lines * sizeof(intlet)))) == NULL)
			return TE_NOMEM;
	}

	trmundefined();

	return TE_OK;
}

/*
 * Termination call.
 * Beware that it might be called by a caught interrupt even in the middle
 * of trmstart()!
 */

Visible Procedure trmend()
{
	if (started && so_mode != Off)
		standend();
	low_trmend();
	started = No;
}

/*
 * Set all internal statuses to undefined, especially the contents of
 * the screen, so a hard redraw will not be optimised to heaven.
 */

Visible Procedure trmundefined()
{
	register int y, x;

	cur_y = cur_x = Undefined;
	so_mode = Undefined;

	for (y = 0; y < lines; y++) {
		for (x = 0; x < cols; x++) {
			linedata[y][x] = ' ';
			linemode[y][x] = PLAIN;
			/* they may get printed in scrolling */
		}
		lenline[y] = 0;
	}
}

/*
 * Sensing the cursor.
 *
 * Sense the current (y, x) cursor position.
 * On terminals with local cursor motion, the first argument must be the
 * string that must be sent to the terminal to ask for the current cursor
 * position after a possible manual change by the user;
 * the format describes the answer as a parameterized string
 * a la termcap(5).
 * In keys.c, you can bound the sensestring to GSENSE and the formatstring
 * to GFORMAT.
 * If the terminal cannot be asked for the current cursor position,
 * or if the string returned by the terminal is garbled,
 * the position is made Undefined.
 *
 * This scheme can also be used for mouse clicks, if these can be made to
 * send the cursor position; the sense string can normally be empty then.
 * In keys.c, the proper xterm/DECSET sequences to initialise the mouse
 * are bound to TERMINIT and TERMDONE; these strings get send by initgetc.
 * Mouse clicks are then sent as ESC [ M SPACE followed by the x,y position
 * in a form described by the formatstring;
 * SPACE indicates button 1, ! button 2 and " button 3.
 * Therefore, in keys.c we also bound ESC [ M SPACE etc to MOUSE
 * (and a format example to MFORMAT).
 *
 */

Visible Procedure trmsense(sense, format, py, px)
     string sense, format;
     int *py, *px;
{
	bool dotrmsense();

	*py = *px = Undefined;
	if (dotrmsense(sense, format, py, px)) {
		if (*py < 0 || lines <= *py || *px < 0 || cols <= *px)
			*py = *px = Undefined;
	}
	cur_y = Undefined;
	cur_x = Undefined;
}

Hidden bool dotrmsense(sense, format, py, px)
     string sense, format;
     int *py, *px;
{
	bool get_pos();

	if (sense != NULL) {
		low_putstr(sense);
		low_trmsync();
	}
	return get_pos(format, py, px);
}     

Hidden bool get_pos(format, py, px)
     string format;
     int *py, *px;
{
	int fc; 		/* current format character */
	int ic; 		/* current input character */
	int num;
	int on_y = 1;
	bool incr_orig = No;
	int i, ni;

	if (format == NULL)
		return No;
	while (fc = *format++) {
		if (fc != '%') {
			if (trminput() != fc)
				return No;
		}
		else {
			switch (fc = *format++) {
			case '%':
				if (trminput() != '%')
					return No;
				continue;
			case '.':
				VOID trminput(); /* skip one char */
				continue;
			case 'r':
				on_y = 1 - on_y;
				continue;
			case 'i':
				incr_orig = Yes;
				continue;
			case 'd':
				ic = trminput();
				if (!isdigit(ic))
					return No;
				num = ic - '0';
				while (isdigit(ic=trminput()))
					num = 10*num + ic - '0';
				trmpushback(ic);
				break;
			case '2':
			case '3':
				ni = fc - '0';
		    		num = 0;
				for (i=0; i<ni; i++) {
					ic = trminput();
					if (isdigit(ic))
						num = 10*num + ic - '0';
					else
						return No;
				}
				break;
			case '+':
				num = trminput() - *format++;
				break;
			case '-':
				num = trminput() + *format++;
				break;
			default:
				return No;
			}
			/* assign num to parameter */
			if (incr_orig)
				num--;
			if (on_y)
				*py = num;
			else
				*px = num;
			on_y = 1 - on_y;
		}
	}

	return Yes;
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
	int yfirst, ylast;
	register int indent;
	register string data;
	register string mode;
{
	register int y;
	int x, len, lendata, space;

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
		while (/*len > 0 &&*/ y <= ylast) {
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

Hidden int put_line(y, xskip, data, mode, len)
	int y, xskip;
	string data;
	string mode;
	int len;
{
	register char *op, *oq, *mp;
	register char *np, *nq, *mo;
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
	if (cur_x == cols && auto_margins) {
		cur_y++;
		cur_x = 0;
	}
}

/*
 * Scrolling (part of) the screen up (or down, dy<0).
 */

Visible Procedure trmscrollup(yfirst, ylast, by)
     register int yfirst;
     register int ylast;
     register int by;
{
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
	} else if (flags & CAN_SCROLL) {
		/* It can do hardware scrolling */
	   	low_trmscrollup(yfirst, ylast, by);
	} else {
		/* We'll do it by brute force */
		hidemouse();

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
		showmouse();
	}
}

/*
 * Synchronization, move cursor to given position (or previous if < 0).
 */

Visible Procedure trmsync(y, x)
     int y;
     int x;
{
	if (0 <= y && y < lines && 0 <= x && x < cols) {
		move(y, x);
	}
	low_trmsync();
}

/*
 * Send a bell, visible if possible.
 */

Visible Procedure trmbell()
{
	low_trmbell();
}

/*
 * The following routine is the time bottleneck, I believe!
 */

Hidden Procedure put_str(data, mode, n)
	char *data, *mode;
	int n;
{
	register char c, mo, so;

	hidemouse();

	so = so_mode;
	while (--n >= 0) {
		c = *data++;
		mo= (mode != NULL ? *mode++ : PLAIN);
		if (mo != so) {
			so= mo;
			so ? standout() : standend();
		}
		low_put_char(c);
		linedata[cur_y][cur_x] = c;
		linemode[cur_y][cur_x] = mo;
		cur_x++;
	}
	showmouse();
}

Hidden Procedure clear_lines(yfirst, ylast)
     int yfirst;
     int ylast;
{
	register int y;

	hidemouse();
	if (yfirst == 0 && ylast == lines-1) {
		if (so_mode == On)
			standend();
		low_home_and_clear();
		cur_y = cur_x = 0;
		for (y = yfirst; y < ylast; y++)
			lenline[y] = 0;
	} else {
		for (y = yfirst; y <= ylast; y++) {
			if (lenline[y] > 0) {
				move(y, 0);
				clr_to_eol();
			}
		}
	}
	showmouse();
}

Hidden Procedure clr_to_eol()
{
	if (so_mode == On)
		standend();
	low_clr_to_eol();
	lenline[cur_y] = cur_x;
}

/* Reset internal administration accordingly */

Hidden Procedure scr_lines(yfrom, yto, n, dy)
     int yfrom;
     int yto;
     int n;
     int dy;
{
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

Hidden Procedure lf_scroll(yto, by)
     int yto;
     int by;
{
	register int n = by;

	move(lines-1, 0);
	while (n-- > 0) {
		low_put_char('\n');
	}
	scr_lines(0, lines-1, by, 1);
	move_lines(lines-1-by, lines-1, lines-1-yto, -1);
	clear_lines(yto-by+1, yto);
}

/* for dumb scrolling, uses and updates internal administration */

Hidden Procedure move_lines(yfrom, yto, n, dy)
     int yfrom;
     int yto;
     int n;
     int dy;
{
	while (n-- > 0) {
		put_line(yto, 0, linedata[yfrom], linemode[yfrom], lenline[yfrom]);
		yfrom += dy;
		yto += dy;
	}
}

/*
 * Move to position y,x on the screen
 */

Hidden Procedure move(y, x)
     int y;
     int x;
{
	if (cur_y == y && cur_x == x) return;
	low_move(y, x);

	cur_y = y;
	cur_x = x;
}

Hidden Procedure standout()
{
	so_mode = On;
	low_standout();
}

Hidden Procedure standend()
{
	so_mode = Off;
	low_standend();
}

#ifdef CATCHINTR

Hidden char intrchar;
Hidden bool trmintrptd= No;
Hidden bool readintrcontext = No;
#ifdef SETJMP
#include <setjmp.h>
Hidden jmp_buf readinterrupt;
#endif

Hidden SIGTYPE trmintrhandler(sig)
     int sig;
{
	VOID signal(SIGINT, trmintrhandler);
	trmintrptd= Yes;
#ifdef SETJMP
	if (readintrcontext) longjmp(readinterrupt, 1);
#endif
}

#endif /* CATCHINTR */

Hidden int pushback= -1;

Hidden Procedure trmpushback(c)
     int c;
{
	pushback= c;
}

/*
 * Terminal input without echo.
 */

Visible int trminput()
{
	int c;

#ifdef CATCHINTR
	if (trmintrptd) {
		trmintrptd = No;
		return intrchar & 0377;
	}
#ifdef SETJMP
	if (setjmp(readinterrupt) != 0) {
		readintrcontext = No;
		trmintrptd = No;
		return intrchar & 0377;
	}
#endif
#endif /* CATCHINTR */

	if (pushback >= 0) {
		c = pushback;
		pushback = -1;
		return c;
	}

#ifdef CATCHINTR
	readintrcontext = Yes;
#endif

	c = low_trminput();

#ifdef CATCHINTR
	readintrcontext = No;
#endif

	return c;
}

/*
 * Check for character available.
 */

Visible int trmavail()
{
#ifdef CATCHINTR
	if (trmintrptd)
		return 1;
#endif
	if (pushback >= 0)
		return 1;

#ifdef CANLOOKAHEAD

	return low_trmavail();
  
#else /* !CANLOOKAHEAD */

	return -1;

#endif /* !CANLOOKAHEAD */
}

Visible int trmsuspend()
{
	return low_trmsuspend();
}

/* Low-level stuff #################################################### */

/*
 * ANSI escape sequences
 */

#define A_CUP	"\033[%d;%dH"   /* cursor position */
#define A_CPR	"\033[6n"	/* enquire cursor position */
#define A_SGR0	"\033[0m"       /* set graphics rendition to normal */
#define A_SGR7	"\033[7m"       /* set graphics rendition to standout */
#define A_ED	"\033[2J"       /* erase display (and cursor home) */
#define A_EL	"\033[K"        /* erase (to end of) line */

Hidden Procedure low_printf(fmt, y, x)
     char *fmt;
     int y;
     int x;
{
	char buf[100];
	sprintf(buf, fmt, y, x);
	low_putstr(buf);
}

/* Initialise the terminal, and set up initial parameters */

Hidden int low_setraw()
{
	/* Open the output, and set it in raw mode.
	   This may also be the place to set intrchar and suspchar,
	   if they are terminal/session dependent
	*/

	if (scrfp == NULL) {
		scrfp = fopen("/dev/tty", "w");
		if (scrfp == NULL)
			return TE_NOTTY;
	}
#ifndef CATCHINTR
	system("stty raw -echo");
	/* Don't look at me like that, it's portable isn't it? */
#else
	intrchar = '\003';        /* or get it from your tty settings */
	system("stty raw -echo");
	system("stty intr ^C");          /* do not disable interrupt */
	signal(SIGINT, trmintrhandler);  /* but catch it */
#endif
	return TE_OK;
}

Hidden int low_startmouse()
{
	/* If you've got a mouse, there may be initialisations to do */
	return TE_OK;
}

Hidden int low_trmstart()
{
	int low_get_screen_env();
	int err;

	/* Set the flags to the properties of the terminal:
	      HAS_STANDOUT
	      CAN_SCROLL
	      (CAN_OPTIMISE) - not used
	   Find the size of the screen, etc, etc, etc.
	*/

	flags = HAS_STANDOUT;

	if ((err= low_setraw()) != TE_OK) return err;
	if ((err= low_startmouse()) != TE_OK) return err;
	/* If, after writing a character to position (y, max)
	   the next character will be written to (y+1, 1),
	   set auto_margins to Yes, otherwise to No
	*/
	auto_margins= Yes; 
	return low_get_screen_env(&lines, &cols);
}

Hidden int low_get_screen_env(pheight, pwidth)
     int *pheight;
     int *pwidth;
{
	int maxx, maxy, upx, upy;

	/* Find out the size of the screen.
	   We use the cursor sense sequence to discover the maximum size.
	 */

	upy= 100; upx= 100;
	maxy= upy; maxx= upx;
	while (maxy == upy && maxx == upx) {
		if (maxy == upy) upy*=2;
		if (maxx == upx) upx*=2;
		low_move(upy, upx);
		if (!cursor_sense(&maxy, &maxx)) return TE_OTHER;
	}

	*pheight= maxy + 1;
	*pwidth=  maxx + 1;

	return TE_OK;
}

Hidden bool cursor_sense(py, px)
     int *py;
     int *px;
{
	/* Enquire where the cursor is.
	   Points to consider are for instance
	    - whether you should switch standout off
	    - whether you have to return to 'normal' mode
	*/
	low_putstr(A_CPR);
	low_trmsync();
	return get_yx(py, px);
}

Hidden bool get_yx(py, px)
     int *py;
     int *px;
{
	/* Get the string ESC [ y ; x R */
	int n, y, x;
	char c;

	y= 0; x= 0;

	if (low_trminput() != '\033') return No;
	if (low_trminput() != '[') return No;
	c = low_trminput();
	if (!isdigit(c)) return No;
	y = c - '0';
	while (isdigit(c= low_trminput()))
		y = 10*y + c - '0';
	if (c != ';') return No;
	c = low_trminput();
	if (!isdigit(c)) return No;
	x = c - '0';
	while (isdigit(c= low_trminput()))
		x = 10*x + c - '0';
	if (c != 'R') return No;
	*py= y-1; *px= x-1;

	return Yes;
}

Hidden Procedure low_endmouse()
{
}

Hidden Procedure low_endraw()
{
	system("stty -raw echo");
	fclose(scrfp);
	scrfp = NULL;
}

Hidden Procedure low_trmend()
{
	low_trmsync();
	low_endmouse();
	low_endraw();
}

Hidden Procedure low_trmscrollup(yfirst, ylast, by)
     int yfirst;
     int ylast;
     int by;
{
	/* Here you can implement hardware scrolling.
	   This is only called if the CAN_SCROLL bit is set in the flags,
	   otherwise it is done by moving and clearing lines in trmscrollup
	*/
}

Hidden Procedure low_trmbell()
{
	low_put_char('\007');
}

Hidden Procedure low_trmsync()
{
	/* make sure everything that has been written has been sent to the
	 * screen.
	 */
	VOID fflush(scrfp);
}

Hidden Procedure low_clr_to_eol()
{
	low_putstr(A_EL);
}

Hidden Procedure low_move(y, x)
     int y;
     int x;
{
	hidemouse();
	low_printf(A_CUP, y+1, x+1);
	showmouse();
}

Hidden Procedure low_standout()
{
	low_putstr(A_SGR7);
}

Hidden Procedure low_standend()
{
	low_putstr(A_SGR0);
}

Hidden Procedure low_home_and_clear()
{
	move(0, 0); /* for safety */
	low_putstr(A_ED);
}

Hidden int low_trminput()
{
	int n;
	char c;

	n= read(0, &c, 1);
	if (n <= 0) return -1;
	return c & 0377;
}

#ifdef CANLOOKAHEAD

Hidden int low_trmavail()
{
      /* code to look ahead in the input queue;
       * return 1 if there is a character available;
       * 0 if there isn't
       */
}

#endif /* CANLOOKAHEAD */

Hidden int low_trmsuspend()
{
	int r;

	r = system("exec ${SHELL-/bin/sh}");
	if (r == -1 || r == 127)
		return 0;
	else
		return 1;
}
