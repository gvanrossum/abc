/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1986. */

#define lenline len_line /* Avoid name conflict with lenline in tex.c */

/*
 * Virtual TeRMinal package.
 * (For a description see at the end of this file.)
 *
 * Big change:
 *	output goes to /dev/tty, in case stdout is redirected to a file.
 *
 * TO DO:
 *	- add interrupt handling (trminterrupt)
 *	- adapt to changed window size when suspended or at SIGWINCH
 *	  (unfortunately, the calling module must be changed first --
 *	  it is not prepared for the changed window size...)
 */

/*
 * Includes and data definitions.
 */

#include "b.h"

#ifndef TERMIO
#include <sgtty.h>
#else
#include <termio.h>
#endif

#include "trm.h"

#ifdef SIGNAL
#include <signal.h>
#endif

char *getenv();
int tgetent();
int tgetnum();
int tgetflag();
char *tgetstr();

#define Min(a,b) ((a) <= (b) ? (a) : (b))

/* tty modes */
#ifndef TERMIO

/* v7/BSD tty control */
Hidden struct sgttyb oldtty, newtty;
#ifdef TIOCSETN
/* Redefine stty to use TIOCSETN, so type-ahead is not flushed */
#define stty(fd, bp) VOID ioctl(fd, TIOCSETN, (char *) bp)
#endif

#ifdef TIOCSLTC /* BSD -- local special chars, must all be turned off */
static struct ltchars oldltchars;
static struct ltchars newltchars= {-1, -1, -1, -1, -1, -1};
#endif /* TIOCSLTC */

#ifdef TIOCSETC /* V7 -- standard special chars, some must be turned off too */
static struct tchars oldtchars;
static struct tchars newtchars;
#endif /* TIOCSETC */

#else /* TERMIO */

/* AT&T tty control */
Hidden struct termio oldtty, newtty, polltty;
#define gtty(fd,bp) ioctl(fd, TCGETA, (char *) bp)
#define stty(fd,bp) VOID ioctl(fd, TCSETAW, (char *) bp)

#endif /* TERMIO */

/* If you can't lookahead in the system's input queue, and so can't implement
 * trmavail(), you have to enable keyboard interrupts.
 * Otherwise, computations are uninterruptable.
 * Furthermore, you cannot use the interrupt character in any binding,
 * except for the [cancel] binding of course.
 */

#ifndef CANLOOKAHEAD

#ifdef SIGNAL
#ifndef KEYS          /* only for abc proper, not for the abckeys program */
#define CATCHINTR
#endif
#endif

#endif /* !CANLOOKAHEAD */

#ifdef VTRMTRACE
extern FILE *vtrmfp;
#endif

Hidden bool know_ttys = No;

/* visible data for termcap */
char PC;
char *BC;
char *UP;
short ospeed;

Hidden FILE *fp= NULL;
Forward Hidden int outchar(); 		/* procedure for termcap's tputs */
#define Putstr(str)	tputs((str), 1, outchar)
extern char *tgoto();

/* termcap terminal capabilities */

Hidden int lines;
Hidden int cols;

/*
 * String-valued capabilities are saved in one big array.
 * Extend this only at the end (even though it disturbs the sorting)
 * because otherwise you have to change all macros...
 */

#define par_al_str strcaps[0]	/* parametrized al (AL) */
#define cap_cm_str strcaps[1]	/* screen-relative cursor motion (CM) */
#define par_dl_str strcaps[2]	/* parametrized dl (DL) */
#define al_str strcaps[3] 	/* add new blank line */
#define cd_str strcaps[4] 	/* clear to end of display */
#define ce_str strcaps[5] 	/* clear to end of line */
#define cl_str strcaps[6] 	/* cursor home and clear screen */
#define cm_str strcaps[7] 	/* cursor motion */
#define cp_str strcaps[8]	/* cursor position sense reply; obsolete */
#define cr_str strcaps[9] 	/* carriage return */
#define cs_str strcaps[10] 	/* change scrolling region */
#define dc_str strcaps[11] 	/* delete character */
#define dl_str strcaps[12] 	/* delete line */
#define dm_str strcaps[13] 	/* enter delete mode */
#define do_str strcaps[14] 	/* cursor down one line */
#define ed_str strcaps[15] 	/* end delete mode */
#define ei_str strcaps[16] 	/* end insert mode */
#define ho_str strcaps[17]	/* cursor home */
#define ic_str strcaps[18] 	/* insert character (if necessary; may pad) */
#define im_str strcaps[19] 	/* enter insert mode */
#define nd_str strcaps[20] 	/* cursor right (non-destructive space) */
#define nl_str strcaps[21]	/* newline */
#define se_str strcaps[22] 	/* end standout mode */
#define sf_str strcaps[23] 	/* scroll text up (from bottom of region) */
#define so_str strcaps[24] 	/* begin standout mode */
#define sp_str strcaps[25]	/* sense cursor position; obsolete */
#define sr_str strcaps[26] 	/* scroll text down (from top of region) */
#define te_str strcaps[27] 	/* end termcap */
#define ti_str strcaps[28] 	/* start termcap */
#define vb_str strcaps[29] 	/* visible bell */
#define ve_str strcaps[30] 	/* make cursor visible again */
#define vi_str strcaps[31] 	/* make cursor invisible */
#define le_str strcaps[32] 	/* cursor left */
#define bc_str strcaps[33]	/* backspace character */
#define up_str strcaps[34] 	/* cursor up */
#define pc_str strcaps[35]	/* pad character */
#define ks_str strcaps[36]	/* keypad mode start */
#define ke_str strcaps[37]	/* keypad mode end */
#define us_str strcaps[38]	/* start underscore mode */
#define ue_str strcaps[39]	/* end underscore mode */
/* Insert new entries here only! Don't forget to change the next line! */
#define NSTRCAPS 40 /* One more than the last entry's index */

Hidden char *strcaps[NSTRCAPS];
Hidden char strcapnames[] =
"ALCMDLalcdceclcmcpcrcsdcdldmdoedeihoicimndnlsesfsospsrtetivbvevilebcuppckskeusue";

/* Same for Boolean-valued capabilities */

#define has_am flagcaps[0]	/* has automatic margins */
#define has_da flagcaps[1]	/* display may be retained above screen */
#define has_db flagcaps[2]	/* display may be retained below screen */
#define has_in flagcaps[3]	/* not safe to have null chars on the screen */
#define has_mi flagcaps[4]	/* move safely in insert (and delete?) mode */
#define has_ms flagcaps[5]	/* move safely in standout mode */
#define has_xs flagcaps[6]	/* standout not erased by overwriting */
#define has_bs flagcaps[7]	/* terminal can backspace */
#define hardcopy flagcaps[8]	/* hardcopy terminal */
#define has_xn flagcaps[9]	/* Vt100 / Concept glitch */
#define NFLAGS 10

Hidden char flagcaps[NFLAGS];
Hidden char flagnames[]= "amdadbinmimsxsbshcxn";

Hidden Procedure getcaps(parea) register char **parea; {
	register char *capname;
	register char **capvar;
	register char *flagvar;

	for (capname= flagnames, flagvar= flagcaps;
			*capname != '\0'; capname += 2, ++flagvar)
		*flagvar= tgetflag(capname);

	for (capname= strcapnames, capvar= strcaps;
			*capname != '\0'; capname += 2, ++capvar)
		*capvar= tgetstr(capname, parea);
}

/* terminal status */

/* calling order of Visible Procs */
Hidden bool started = No;

/* to exports the capabilities mentioned in vtrm.h: */
Hidden int flags = 0;

/* cost for impossible operations */
#define Infinity 9999
	/* Allow for adding Infinity+Infinity within range */
	/* (Range is assumed at least 2**15 - 1) */

/* The following for all sorts of undefined things (except for UNKNOWN char) */
#define Undefined (-1)

/* current mode of putting char's */
#define Normal	0
#define Insert	1
#define	Delete	2
Hidden short mode = Normal;

/* masks for char's */
#define NULCHAR	'\000'
#define UNKNOWN	1
#define SOBIT	1
/* if (has_xs) record cookies placed on screen in extra bit */
/* type of cookie is determined by the SO bit */
#define XSBIT	2
#define SOCOOK	3
#define SECOOK	2
#define COOKBITS SOCOOK
#define NOCOOK	0

/* current standout mode */
#define Off	0
#define On	SOBIT
Hidden short so_mode = Off;

/* current cursor position */
Hidden short cur_y = Undefined, cur_x = Undefined;

/* "linedata[y][x]" holds the char on the terminal;
 * "linemode[y][x]" holds the SOBIT and XSBIT.
 * the SOBIT tells whether the character is standing out, the XSBIT whether
 * there is a cookie on the screen at this position.
 * In particular a standend-cookie may be recorded AFTER the line
 * (just in case some trmputdata will write after that position).
 * "lenline[y]" holds the length of the line.
 * Unknown chars will be 1, so the optimising compare in putline will fail.
 * (Partially) empty lines are distinghuished by "lenline[y] < cols".
 */
Hidden char **linedata = NULL, **linemode = NULL;
Hidden intlet *lenline = NULL;

/* To compare the mode part of the line when the
 * mode parameter of trmputdata == NULL, we use the following:
 */
Hidden char plain[1]= {PLAIN};

/* Clear the screen initially iff only memory cursor addressing available */
Hidden bool mustclear = No;

/* Make the cursor invisible when trmsync() tries to move outside the screen */
Hidden bool no_cursor = No;

/* Optimise cursor motion */
Hidden int abs_cost; 		/* cost of absolute cursor motion */
Hidden int cr_cost; 		/* cost of carriage return */
Hidden int do_cost; 		/* cost of down */
Hidden int le_cost; 		/* cost of left */
Hidden int nd_cost; 		/* cost of right */
Hidden int up_cost; 		/* cost of up */

/* Optimise trailing match in put_line, iff the terminal can insert and delete
 * characters; the cost per n characters will be:
 * 	n * MultiplyFactor + OverHead
 */
Hidden int ins_mf, ins_oh, del_mf, del_oh;
Hidden int ed_cost, ei_cost; 		/* used in move() */

Forward Hidden int getttyfp();
Forward Hidden int str0cost();
Forward Hidden int gettermcaps();
Forward Hidden int setttymode();
Forward Hidden Procedure resetttymode();
Forward Hidden int start_trm();
Forward Hidden bool get_pos();
Forward Hidden int put_line();
Forward Hidden Procedure set_mode();
Forward Hidden Procedure get_so_mode();
Forward Hidden Procedure standout();
Forward Hidden Procedure standend();
Forward Hidden Procedure put_str();
Forward Hidden Procedure ins_str();
Forward Hidden Procedure del_str();
Forward Hidden Procedure put_c();
Forward Hidden Procedure clear_lines();
Forward Hidden Procedure clr_to_eol();
Forward Hidden Procedure set_blanks();
Forward Hidden Procedure scr_lines();
Forward Hidden Procedure addlines();
Forward Hidden Procedure dellines();
Forward Hidden Procedure lf_scroll();
Forward Hidden Procedure move_lines();
Forward Hidden Procedure trmpushback();
Forward Hidden int subshell();

/* The type of scrolling possible determines which routines get used;
 * these may be:
 * (1) with addline and deleteline (termcap: al_str & dl_str);
 * (2) with a settable scrolling region, like VT100 (cs_str, sr_str, sf_str);
 * (3) no hardware scrolling available.
 */
Forward Hidden Procedure (*scr_up)();
Forward Hidden Procedure (*scr_down)();
Forward Hidden Procedure scr1up();
Forward Hidden Procedure scr1down();
Forward Hidden Procedure scr2up();
Forward Hidden Procedure scr2down();
Forward Hidden Procedure scr3up();
Hidden bool canscroll= Yes;

/*
 * Starting, Ending and (fatal) Error.
 */

/* 
 * Initialization call.
 * Determine terminal capabilities from termcap.
 * Set up tty modes.
 * Start up terminal and internal administration.
 * Return 0 if all well, error code if in trouble.
 */
Visible int trmstart(plines, pcols, pflags) int *plines, *pcols, *pflags; {
	register int err;

#ifdef VTRMTRACE
	if (vtrmfp) fprintf(vtrmfp, "\ttrmstart(&li, &co, &fl);\n");
#endif
	if (started)
		return TE_TWICE;
	err= getttyfp();
	if (err != TE_OK)
		return err;
	err= gettermcaps();
	if (err != TE_OK)
		return err;
	err= setttymode();
	if (err != TE_OK)
		return err;
	err= start_trm();
	if (err != TE_OK) {
		trmend();
		return err;
	}

	*plines = lines;
	*pcols = cols;
	*pflags = flags;

	started = Yes;
	
	trmsync(lines-1, 0);
		/* position for >ws message from initbws()
		 * on vt100's, and for alternate screen buffers
		 */
	
	return TE_OK;
}

/*
 * Termination call.
 * Reset tty modes, etc.
 * Beware that it might be called by a caught interrupt even in the middle
 * of trmstart()!
 */
Visible Procedure trmend() {
#ifdef VTRMTRACE
	if (vtrmfp) fprintf(vtrmfp, "\ttrmend();\n");
#endif
	set_mode(Normal);
	if (so_mode != Off)
		standend();
	Putstr(ke_str);
	Putstr(te_str);
	VOID fflush(fp);
	resetttymode();

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
	mode = so_mode = Undefined;
	
	for (y = 0; y < lines; y++) {
		for (x = 0; x <= cols; x++) {
			linedata[y][x] = UNKNOWN; /* impossible char */
			linemode[y][x] = NOCOOK;  /* no so bits */
		}
		lenline[y] = 0;
	}
}

#ifdef VTRMTRACE

/*ARGSUSED*/
Hidden Procedure check_started(m) char *m; {
	if (!started) {
		trmend();
		if (vtrmfp) fprintf(vtrmfp, "bad VTRM call\n");
		abort();
	}
}
#else

#define check_started(m) /*empty*/

#endif /* NDEBUG */

Hidden int getttyfp() {
	if (fp != NULL)	/* already initialised */
		return TE_OK;
	fp= fopen("/dev/tty", "w");
	if (fp == NULL)
		return TE_NOTTY;
	return TE_OK;
}

Hidden int ccc;

/*ARGSUSED*/
Hidden Procedure countchar(ch) char ch; {
	ccc++;
}

Hidden int strcost(str) char *str; {
	if (str == NULL)
		return Infinity;
	return str0cost(str);
}

Hidden int str0cost(str) char *str; {
	ccc = 0;
	tputs(str, 1, countchar);
	return ccc;
}

/*
 * Get terminal capabilities from termcap and compute related static
 * properties.  Return TE_OK if all well, error code otherwise.
 */

Hidden int gettermcaps() {
	string trmname;
	char tc_buf[1024];
	static char strbuf[1024];
	char *area = strbuf;
	int sg;
	static bool tc_initialized = No;

	if (tc_initialized)
		return TE_OK;
	
	trmname=getenv("TERM");
	if (trmname == NULL || trmname[0] == '\0')
		return TE_NOTERM;
	if (tgetent(tc_buf, trmname) != 1)
		return TE_BADTERM;

	getcaps(&area); /* Read all flag and string type capabilities */
	if (hardcopy)
		return TE_DUMB;
	BC = le_str;
	if (BC == NULL) {
		BC = bc_str;
		if (BC == NULL) {
			if (has_bs)
				BC = "\b";
			else
				return TE_DUMB;
		}
	}
	UP = up_str;
	if (UP == NULL)
		return TE_DUMB;
	PC = (pc_str != NULL? pc_str[0] : NULCHAR);

	if (cm_str == NULL) {
		cm_str = cap_cm_str;
		if (cm_str == NULL) {
			if (ho_str == NULL || do_str == NULL || nd_str == NULL)
				return TE_DUMB;
		}
		else
			mustclear = Yes;
	}
	if (al_str && dl_str) {
		scr_up = scr1up;
		scr_down = scr1down;
		flags |= CAN_SCROLL;
	}
	else {
		if (sf_str == NULL)
			sf_str = "\n";
		if (cs_str && sr_str) {
			scr_up = scr2up;
			scr_down = scr2down;
			flags |= CAN_SCROLL;
		}
		else {
			canscroll= No;
		}
	}
		
	lines = tgetnum("li");
	cols = tgetnum("co");
	if (lines <= 0) lines = 24;
	if (cols <= 0) cols = 80;
	
	if (!ce_str)
		return TE_DUMB;
	if (cr_str == NULL) cr_str = "\r";
	if (do_str == NULL) {
		do_str = nl_str;
		if (do_str == NULL) do_str = "\n";
	}
	le_str = BC;
	up_str = UP;
	if (vb_str == NULL) 	/* then we will do with the audible bell */
		vb_str = "\007";
	
	if (us_str != NULL && ue_str != NULL) {
		so_str = us_str; se_str = ue_str;
		flags |= HAS_STANDOUT;
	}
	else if (so_str != NULL && se_str != NULL && (sg=tgetnum("sg")) <= 0) {
		if (sg == 0)
			has_xs = Yes;
		flags |= HAS_STANDOUT;
	}
	else
		return TE_DUMB;

	/* calculate costs of local and absolute cursor motions */
	if (cm_str == NULL)
		abs_cost = Infinity;
	else
		abs_cost = strcost(tgoto(cm_str, 0, 0));
	cr_cost = strcost(cr_str);
	do_cost = strcost(do_str);
	le_cost = strcost(le_str);
	nd_cost = strcost(nd_str);
	up_cost = strcost(up_str);

	/* cost of leaving insert or delete mode, used in move() */
	ei_cost = str0cost(ei_str);
	ed_cost = str0cost(ed_str);
	
	/* calculate insert and delete cost multiply_factor and overhead */
	if (((im_str && ei_str) || ic_str) && dc_str) {
		flags |= CAN_OPTIMISE;
		ins_mf = 1 + str0cost(ic_str);
		ins_oh = str0cost(im_str) + ei_cost;
		del_mf = str0cost(dc_str);
		del_oh = str0cost(dm_str) + ed_cost;
	}
		
	tc_initialized = Yes;
	return TE_OK;
}

#ifdef CATCHINTR

Hidden char intrchar;
Hidden bool trmintrptd= No;
Hidden bool readintrcontext = No;
#ifdef SETJMP
#include <setjmp.h>
Hidden jmp_buf readinterrupt;
#endif

Hidden SIGTYPE trmintrhandler(sig) int sig; {
	VOID signal(SIGINT, trmintrhandler);
	trmintrptd= Yes;
#ifdef SETJMP
	if (readintrcontext) longjmp(readinterrupt, 1);
#endif
}

#endif /* CATCHINTR */

Hidden int setttymode() {
	if (!know_ttys) {
		if (gtty(0, &oldtty) != 0
		    || gtty(0, &newtty) != 0
#ifdef TERMIO
		    || gtty(0, &polltty) != 0
#endif
		    )
			return TE_NOTTY;
#ifndef TERMIO
		ospeed = oldtty.sg_ospeed;
		newtty.sg_flags = (newtty.sg_flags & ~ECHO & ~CRMOD & ~XTABS)
				  | CBREAK;
#ifdef TIOCSLTC
		VOID ioctl(0, TIOCGLTC, (char *) &oldltchars);
#endif
#ifdef TIOCSETC
		VOID ioctl(0, TIOCGETC, (char *) &oldtchars);
#endif

#else /* TERMIO */
		ospeed= oldtty.c_lflag & CBAUD;
		newtty.c_iflag &= ~ICRNL; /* No CR->NL mapping on input */
		newtty.c_oflag &= ~ONLCR; /* NL doesn't output CR */
		newtty.c_lflag &= ~(ICANON|ECHO|ISIG);
				/* No line editing, no echo, 
				 * no quit, intr or susp signals */
		newtty.c_cc[VMIN]= 3; /* wait for 3 characters */
		newtty.c_cc[VTIME]= 1; /* or 0.1 sec. */

		polltty.c_iflag= newtty.c_iflag;
		polltty.c_oflag= newtty.c_oflag;
		polltty.c_lflag= newtty.c_lflag;
		polltty.c_cc[VQUIT]= newtty.c_cc[VQUIT];
#ifdef VSUSP
		polltty.c_cc[VSUSP]= newtty.c_cc[VSUSP];
#endif
		polltty.c_cc[VMIN]= 0;  /* don't block read() */
		polltty.c_cc[VTIME]= 0; /* just give 'em chars iff available */
#endif /* TERMIO */
		know_ttys = Yes;
	}
	stty(0, &newtty);
#ifndef TERMIO
#ifdef TIOCSLTC
	VOID ioctl(0, TIOCSLTC, (char *) &newltchars);
#endif
#ifdef TIOCSETC
	VOID ioctl(0, TIOCGETC, (char *) &newtchars);
#ifndef CATCHINTR
	newtchars.t_intrc= -1;
#else
	intrchar= oldtchars.t_intrc;
	newtchars.t_intrc= intrchar;   /* do not disable interupt */
	signal(SIGINT, trmintrhandler);/* but catch it */
#endif /* CATCHINTR */
	newtchars.t_quitc= -1;
	newtchars.t_eofc= -1;
	newtchars.t_brkc= -1;
	VOID ioctl(0, TIOCSETC, (char *) &newtchars);
#endif /* TIOCSETC */
#endif /* TERMIO */
	return TE_OK;
}

Hidden Procedure resetttymode() {
	if (know_ttys) {
		stty(0, &oldtty);
#ifndef TERMIO
#ifdef TIOCSLTC
		VOID ioctl(0, TIOCSLTC, (char *) &oldltchars);
#endif
#ifdef TIOCSETC
		VOID ioctl(0, TIOCSETC, (char *) &oldtchars);
#endif
#endif /* TERMIO */
		know_ttys= No;
	}
}

Hidden int start_trm() {
	register int y;
#ifdef TIOCGWINSZ
	struct winsize win;

	if (ioctl(0, TIOCGWINSZ, (char*)&win) == 0) {
		if (win.ws_col > 0 && ((int) win.ws_col) != cols
		    ||
		    win.ws_row > 0 && ((int) win.ws_row) != lines) {
			/* Window size has changed.
			   Release previously allocated buffers. */
			if (linedata != NULL) {
				for (y= 0; y < lines; ++y) {
					free((char *) linedata[y]);
				}
				free((char *) linedata);
				linedata= NULL;
			}
			if (linemode != NULL) {
				for (y= 0; y < lines; ++y) {
					free((char *) linemode[y]);
				}
				free((char *) linemode);
				linemode= NULL;
			}
			if (lenline != NULL) {
				free((char *) lenline);
				lenline= NULL;
			}
		}
		if (((int)win.ws_col) > 0)
			cols = win.ws_col;
		if (((int)win.ws_row) > 0)
			lines = win.ws_row;
	}
#endif
	if (linedata == NULL) {
		if ((linedata = (char**) malloc(MALLOC_ARG(lines * sizeof(char*)))) == NULL)
			return TE_NOMEM;
		for (y = 0; y < lines; y++) {
			if ((linedata[y] = (char*) malloc(MALLOC_ARG((cols+1) * sizeof(char)))) == NULL)
				return TE_NOMEM;
		}
	}
	if (linemode == NULL) {
		if ((linemode = (char**) malloc(MALLOC_ARG(lines * sizeof(char*)))) == NULL)
			return TE_NOMEM;
		for (y = 0; y < lines; y++) {
			if ((linemode[y] = (char*) malloc(MALLOC_ARG((cols+1) * sizeof(char)))) == NULL)
				return TE_NOMEM;
		}
	}
	if (lenline == NULL) {
		if ((lenline = (intlet*) malloc(MALLOC_ARG(lines * sizeof(intlet)))) == NULL)
			return TE_NOMEM;
	}

	trmundefined();

	Putstr(ti_str);
	Putstr(ks_str);
	if (cs_str)
		Putstr(tgoto(cs_str, lines-1, 0));
	if (mustclear)
		clear_lines(0, lines-1);
	VOID fflush(fp);
	
	return TE_OK;
}


/*
 * Sensing and moving the cursor.
 */

/*
 * Sense the current (y, x) cursor position.
 * On terminals with local cursor motion, the first argument must be the
 * string that must be sent to the terminal to ask for the current cursor
 * position after a possible manual change by the user;
 * the format describes the answer as a parameterized string
 * a la termcap(5).
 * If the terminal cannot be asked for the current cursor position,
 * or if the string returned by the terminal is garbled,
 * the position is made Undefined.
 * This scheme can also be used for mouse clicks, if these can be made to
 * send the cursor position; the sense string can normally be empty.
 */

Visible Procedure trmsense(sense, format, py, px)
	string sense, format; int *py, *px;
{
	bool get_pos();

#ifdef VTRMTRACE
	if (vtrmfp) fprintf(vtrmfp, "\ttrmsense(&yy, &xx);\n");
#endif
	check_started("trmsense");

	if (sense != NULL)
		Putstr(sense);
	VOID fflush(fp);

	*py = *px = Undefined;
	set_mode(Normal);
	if (so_mode != Off)
		standend();
	
	if (format != NULL && get_pos(format, py, px)) {
		if (*py < 0 || lines <= *py || *px < 0 || cols <= *px)
			*py = *px = Undefined;
	}
	cur_y = Undefined;
	cur_x = Undefined;
}

Hidden bool get_pos(format, py, px) string format; int *py, *px; {
	int fc; 		/* current format character */
	int ic; 		/* current input character */
	int num;
	int on_y = 1;
	bool incr_orig = No;
	int i, ni;

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
 * To move over characters by rewriting them, we have to check:
 * (1) that the screen has been initialised on these positions;
 * (2) we do not screw up characters or so_mode
 * when rewriting linedata[y] from x_from upto x_to
 */
Hidden bool rewrite_ok(y, xfrom, xto) int y, xfrom, xto; {
	register char *plnyx, *pmdyx, *plnyto;
	
	if (xto > lenline[y])
		return No;

	plnyto = &linedata[y][xto];
	plnyx = &linedata[y][xfrom];
	pmdyx = &linemode[y][xfrom];
	while (plnyx <= plnyto) {
		if (*plnyx == UNKNOWN
		    ||
		    (!has_xs && (*pmdyx != so_mode))
		   )
			return No;
		plnyx++, pmdyx++;
	}
	return Yes;
}
		
/*
 * Move to position y,x on the screen
 */
/* possible move types for y and x respectively: */
#define None	0
#define Down	1
#define Up	2
#define Right	1
#define ReWrite	2
#define Left	3
#define CrWrite	4

Hidden Procedure move(y, x) int y, x; {
	int dy, dx;
	int y_cost, x_cost, y_move, x_move;
	int mode_cost;
	int xi;
	
	if (cur_y == y && cur_x == x)
		return;
	
	if (!has_mi || mode == Undefined)
		set_mode(Normal);
	if (!has_xs && ((!has_ms && so_mode != Off) || so_mode == Undefined))
		standend();
	
	if (cur_y == Undefined || cur_x == Undefined)
		goto absmove;
	
	dy = y - cur_y;
	dx = x - cur_x;

	if (dy > 0) {
		y_move = Down;
		y_cost = dy * do_cost;
	}
	else if (dy < 0) {
		y_move = Up;
		y_cost = -dy * up_cost;
	}
	else {
		y_move = None;
		y_cost = 0;
	}
	if (y_cost < abs_cost) {
		switch (mode) {
		case Normal:
			mode_cost = 0;
			break;
		case Insert:
			mode_cost = ei_cost;
			break;
		case Delete:
			mode_cost = ed_cost;
			break;
		}
		if (dx > 0) {
			x_cost = dx + mode_cost;
			if (dx*nd_cost < x_cost || !rewrite_ok(y, cur_x, x)) {
				x_cost = dx * nd_cost;
				x_move = Right;
			}
			else
				x_move = ReWrite;
		}
		else if (dx < 0) {
			x_cost = -dx * le_cost;
			x_move = Left;
		}
		else {
			x_cost = 0;
			x_move = None;
		}
		if (cr_cost + x + mode_cost < x_cost && rewrite_ok(y, 0, x)) {
			x_move = CrWrite;
			x_cost = cr_cost + x + mode_cost;
		}
	}
	else
		x_cost = abs_cost;

	if (y_cost + x_cost < abs_cost) {
		switch (y_move) {
		case Down:
			while (dy-- > 0) Putstr(do_str);
			break;
		case Up:
			while (dy++ < 0) Putstr(up_str);
			break;
		}
		switch (x_move) {
		case Right:
			while (dx-- > 0) Putstr(nd_str);
			break;
		case Left:
			while (dx++ < 0) Putstr(le_str);
			break;
		case CrWrite:
			Putstr(cr_str);
			cur_x = 0;
			/* FALL THROUGH */
		case ReWrite:
			set_mode(Normal);
			for (xi = cur_x; xi < x; xi++)
				fputc(linedata[y][xi], fp);
			break;
		}
	}
	else
	{
    absmove:
		if (cm_str == NULL) {
			Putstr(ho_str);
			for (cur_y = 0; cur_y < y; ++cur_y)
				Putstr(do_str);
			/* Should try to use tabs here: */
			for (cur_x = 0; cur_x < x; ++cur_x)
				Putstr(nd_str);
		}
		else
			Putstr(tgoto(cm_str, x, y));
	}
	
	cur_y = y;
	cur_x = x;
}


/*
 * Putting data on the screen.
 */

/*
 * Fill screen area with given "data".
 * Characters for which the corresponding char in "mode" have the value
 * STANDOUT must be put in inverse video.
 */
Visible Procedure trmputdata(yfirst, ylast, indent, data, mode)
	int yfirst, ylast; register int indent;
	register string data; register string mode;
{
	register int y;
	int x, len, lendata, space;

#ifdef VTRMTRACE
	if (vtrmfp) fprintf(vtrmfp, "\ttrmputdata(%d, %d, %d, \"%s\");\n", yfirst, ylast, indent, data);
#endif
	check_started("trmputdata");
	
	if (yfirst < 0)
		yfirst = 0;
	if (ylast >= lines)
		ylast = lines-1;
	space = cols*(ylast-yfirst+1) - indent;
	if (space <= 0)
		return;
	yfirst += indent/cols;
	indent %= cols;
	y= yfirst;
	if (!data)
		data= ""; /* Safety net */
	x = indent;
	lendata = strlen(data);
	if (ylast == lines-1 && lendata >= space)
		lendata = space - 1;
	len = Min(lendata, cols-x);
	while (y <= ylast) {
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
	if (y <= ylast)
		clear_lines(y, ylast);
}

/* 
 * We will first try to get the picture:
 *
 *                  op>>>>>>>>>>>op          oq<<<<<<<<<<<<<<<<<<<<<<<<oq
 *                  ^            ^           ^                         ^
 *           <xskip><-----m1----><----od-----><-----------m2----------->
 *   OLD:   "You're in a maze of twisty little pieces of code, all alike"
 *   NEW:          "in a maze of little twisting pieces of code, all alike"
 *                  <-----m1----><-----nd------><-----------m2----------->
 *                  ^            ^             ^                         ^
 *                  np>>>>>>>>>>>np            nq<<<<<<<<<<<<<<<<<<<<<<<<nq
 * where
 *	op, oq, np, nq are pointers to start and end of Old and New data,
 * and
 *	xskip = length of indent to be skipped,
 *	m1 = length of Matching part at start,
 *	od = length of Differing mid on screen,
 *	nd = length of Differing mid in data to be put,
 *	m2 = length of Matching trail.
 *
 * Then we will try to find a long blank-or-cleared piece in <nd+m2>:
 *
 *    <---m1---><---d1---><---nb---><---d2---><---m2--->
 *              ^         ^         ^        ^         ^
 *              np        bp        bq1      nq        nend
 * where
 *	bp, bq1 are pointers to start and AFTER end of blank piece,
 * and
 *	d1 = length of differing part before blank piece,
 *	nb = length of blank piece to be skipped,
 *	d2 = length of differing part after blank piece.
 * Remarks:
 *	d1 + nb + d2 == nd,
 * and
 *	d2 maybe less than 0.
 */
Hidden int put_line(y, xskip, data, mode, len) int y, xskip; string data; string mode; int len; {
	register char *op, *oq, *mp;
	register char *np, *nq, *nend, *mo;
	char *bp, *bq1, *p, *q;
	int m1, m2, od, nd, delta, dd, d1, nb, d2;
	bool skipping;
	int cost, o_cost; 	/* normal and optimising cost */
	
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
	nq = nend = data + len - 1;
	mo = (mode != NULL ? mode : plain);
	m1 = m2 = 0;
	while (op <= oq && np <= nq && *op == *np && ((*mp)&SOBIT) == *mo) {
		op++, np++, mp++, m1++;
		if (mode != NULL) mo++;
	}
	/* calculate m2, iff we can optimise or line keeps same length: */
	if (flags & CAN_OPTIMISE || (oq-op) == (nq-np)) {
		if (mode != NULL) mo = mode + len - 1;
		mp = &linemode[y][lenline[y]-1];
		while (op <= oq && np <= nq
		       && *oq == *nq && ((*mp)&SOBIT) == *mo) {
			oq--, nq--, mp--, m2++;
			if (mode != NULL) mo--;
		}
	}
	od = oq - op + 1;
	nd = nq - np + 1;
	/* now we have the first picture above */

	if (od==0 && nd==0)
		return;
	delta = nd - od;

	/* find the blank piece */
	p = q = bp = bq1 = np;
	if (mode != NULL) mo = mode + (np - data);
	oq += m2; 		/* back to current eol */
	if (delta == 0) /* if no change in linelength */
		nend -= m2;	/* don't find blanks in m2 */
	if (!has_in) {
		mp = &linemode[y][xskip + (op - (&linedata[y][xskip]))];
		while (p <= nend) {
			while (q <= nend && *q == ' ' && *mo == PLAIN
			       && (op > oq
				   || (*op == ' ' && ((*mp)&SOBIT) == NOCOOK)))
			{
				q++, op++, mp++;
				if (mode != NULL) mo++;
			}
			if (q - p > bq1 - bp)
				bp = p, bq1 = q;
			p = ++q;
			if (mode != NULL) mo++;
			op++, mp++;
		}
	}
	d1 = bp - np;
	nb = bq1 - bp;
	d2 = nq - bq1 + 1;
	
	/* what is cheapest:
	 * ([+m2] means: leave m2 alone if same linelength)
	 *  normal: put nd[+m2];                       (dd = nd[+m2])
	 *  skipping: put d1, skip nb, put d2[+m2];    (dd = d2[+m2])
	 *  optimise: put dd, insert or delete delta.  (dd = min(od,nd))
	 */
	cost = nd + (delta == 0 ? 0 : m2); 	/* normal cost */
	if (nb > abs_cost || (d1 == 0 && nb > 0)) {
		skipping = Yes;
		cost -= nb - (d1>0 ? abs_cost : 0); /* skipping cost */
		dd = d2;
	}
	else {
		skipping = No;
		dd = nd;
	}
	
	if (m2 != 0 && delta != 0) {
		/* try optimising */
		o_cost = Min(od, nd);
		if (delta > 0)
			o_cost += delta * ins_mf + ins_oh;
		else if (delta < 0)
			o_cost += -delta * del_mf + del_oh;
		if (o_cost >= cost) {
			/* discard m2, no optimise */
			dd += m2;
			m2 = 0;
		}
		else {
			dd = Min(od, nd);
			skipping = No;
		}
	}

	/* and now for the real work */
	if (!skipping || d1 > 0)
		move(y, xskip + m1);

	if (has_xs)
		get_so_mode();
	
	if (skipping) {
		if (d1 > 0) {
			set_mode(Normal);
			mo= (mode != NULL ? mode+(np-data) : NULL);
			put_str(np, mo, d1, No);
		}
		if (has_xs && so_mode != Off)
			standend();
		set_blanks(y, xskip+m1+d1, xskip+m1+d1+nb);
		if (dd != 0 || delta < 0) {
			move(y, xskip+m1+d1+nb);
			np = bq1;
		}
	}
	
	if (dd > 0) {
		set_mode(Normal);
		mo= (mode != NULL ? mode+(np-data) : NULL);
		put_str(np, mo, dd, No);
	}
	
	if (m2 > 0) {
		if (delta > 0) {
			set_mode(Insert);
			mo= (mode != NULL ? mode+(np+dd-data) : NULL);
			ins_str(np+dd, mo, delta);
		}
		else if (delta < 0) {
			if (so_mode != Off)
				standend();
				/* Some terminals fill with standout spaces! */
			set_mode(Delete);
			del_str(-delta);
		}
	}
	else {
		if (delta < 0) {
			clr_to_eol();
			return;
		}
	}
	
	lenline[y] = xskip + len;
	if (cur_x == cols) {
		if (!has_mi)
			set_mode(Normal);
		if (!has_ms)
			so_mode = Undefined;
		if (has_am) {
			if (has_xn)
				cur_y= Undefined;
			else
				cur_y++;
		}
		else
			Putstr(cr_str);
		cur_x = 0;
	}
	else if (has_xs) {
		if (m2 == 0) {
			if (so_mode == On)
				standend();
		}
		else {
			if (!(linemode[cur_y][cur_x] & XSBIT)) {
			    if (so_mode != (linemode[cur_y][cur_x] & SOBIT)) {
				    if (so_mode) standend();
				    else standout();
			    }
			}
		}
	}
}

Hidden Procedure set_mode(m) int m; {
	if (m == mode)
		return;
	switch (mode) {
	case Insert:
		Putstr(ei_str);
		break;
	case Delete:
		Putstr(ed_str);
		break;
	case Undefined:
		Putstr(ei_str);
		Putstr(ed_str);
		break;
	}
	switch (m) {
	case Insert:
		Putstr(im_str);
		break;
	case Delete:
		Putstr(dm_str);
		break;
	}
	mode = m;
}

Hidden Procedure get_so_mode() {
	if (cur_x >= lenline[cur_y] || linemode[cur_y][cur_x] == NOCOOK)
		so_mode = Off;
	else
		so_mode = linemode[cur_y][cur_x] & SOBIT;
}

Hidden Procedure standout() {
	Putstr(so_str);
	so_mode = On;
	if (has_xs)
		linemode[cur_y][cur_x] = SOCOOK;
}

Hidden Procedure standend() {
	Putstr(se_str);
	so_mode = Off;
	if (has_xs)
		linemode[cur_y][cur_x] = SECOOK;
}

Hidden Procedure put_str(data, mode, n, inserting) char *data; char *mode; int n; bool inserting; {
	register char ch, mo;
	register short so;
	char *ln_y_x, *ln_y_end;
	
	so = so_mode;
	if (has_xs) {
		ln_y_x = &linemode[cur_y][cur_x];
		ln_y_end = &linemode[cur_y][lenline[cur_y]];
	}
	while (n-- > 0) {
		if (has_xs && ln_y_x <= ln_y_end && ((*ln_y_x)&XSBIT))
			so = so_mode = (*ln_y_x)&SOBIT;
			/* this also checks for the standend cookie AFTER */
			/* the line because off the equals sign in <= */
		ch= *data++;
		mo= (mode != NULL ? *mode++ : PLAIN);
		if (mo != so) {
			so = mo;
			if (so) standout();
			else standend();
		}
		if (inserting)
			Putstr(ic_str);
		put_c(ch, mo);
		if (has_xs)
			ln_y_x++;
	}
}

Hidden Procedure ins_str(data, mode, n) char *data, *mode; int n; {
	int x;
	
	/* x will start AFTER the line, because there might be a cookie */
	for (x = lenline[cur_y]; x >= cur_x; x--) {
		linedata[cur_y][x+n] = linedata[cur_y][x];
		linemode[cur_y][x+n] = linemode[cur_y][x];
	}
	put_str(data, mode, n, Yes);
}

Hidden Procedure del_str(n) int n; {
	int x, xto;
	
	xto = lenline[cur_y] - n; /* again one too far because of cookie */
	if (has_xs) {
		for (x = cur_x + n; x >= cur_x; x--) {
			if (linemode[cur_y][x] & XSBIT)
				break;
		}
		if (x >= cur_x)
			linemode[cur_y][cur_x+n] = linemode[cur_y][x];
	}
	for (x = cur_x; x <= xto; x++) {
		linedata[cur_y][x] = linedata[cur_y][x+n];
		linemode[cur_y][x] = linemode[cur_y][x+n];
	}
	while (n-- > 0)
		Putstr(dc_str);
}

Hidden Procedure put_c(ch, mo) char ch; char mo; {
	char sobit, xs_flag;

	fputc(ch, fp);
	sobit = (mo == STANDOUT ? SOBIT : 0);
	if (has_xs)
		xs_flag = linemode[cur_y][cur_x]&XSBIT;
	else
		xs_flag = 0;
	linedata[cur_y][cur_x] = ch;
	linemode[cur_y][cur_x] = sobit|xs_flag;
	cur_x++;
}

Hidden Procedure clear_lines(yfirst, ylast) int yfirst, ylast; {
	register int y;
	
	if (!has_xs && so_mode != Off)
		standend();
	if (cl_str && yfirst == 0 && ylast == lines-1) {
		Putstr(cl_str);
		cur_y = cur_x = 0;
		for (y = 0; y < lines; ++y) {
			lenline[y] = 0;
			if (has_xs) linemode[y][0] = NOCOOK;
		}
		return;
	}
	for (y = yfirst; y <= ylast; y++) {
		if (lenline[y] > 0) {
			move(y, 0);
			if (ylast == lines-1 && cd_str) {
				Putstr(cd_str);
				while (y <= ylast) {
					if (has_xs) linemode[y][0] = NOCOOK;
					lenline[y++] = 0;
				}
				break;
			}
			else {
				clr_to_eol();
			}
		}
	}
}

Hidden Procedure clr_to_eol() {
	lenline[cur_y] = cur_x;
	if (!has_xs && so_mode != Off)
		standend();
	Putstr(ce_str);
	if (has_xs) {
		if (cur_x == 0)
			linemode[cur_y][0] = NOCOOK;
		else if (linemode[cur_y][cur_x-1]&SOBIT)
			standend();
	}
}

Hidden Procedure set_blanks(y, xfrom, xto) int y, xfrom, xto; {
	register int x;
	
	for (x = xfrom; x < xto; x++) {
		linedata[y][x] = ' '; 
		if (has_xs && lenline[y] >= x && linemode[y][x]&XSBIT)
			linemode[y][x]= SECOOK; 
		else
			linemode[y][x]= NOCOOK;
	}
}

/* 
 * outchar() is used by termcap's tputs.
 */
Hidden int outchar(ch) char ch; {
	fputc(ch, fp);
}

/*
 * Scrolling (part of) the screen up (or down, by<0).
 */

Visible Procedure trmscrollup(yfirst, ylast, by) register int yfirst;
		register int ylast; register int by; {
#ifdef VTRMTRACE
	if (vtrmfp) fprintf(vtrmfp, "\ttrmscrollup(%d, %d, %d);\n", yfirst, ylast, by);
#endif
	check_started("trmscrollup");
	
	if (by == 0)
		return;

	if (yfirst < 0)
		yfirst = 0;
	if (ylast >= lines)
		ylast = lines-1;

	if (yfirst > ylast)
		return;

	if (!has_xs && so_mode != Off)
		standend();
	
	if (by > 0 && yfirst + by > ylast
	    ||
	    by < 0 && yfirst - by > ylast)
	{
		clear_lines(yfirst, ylast);
		return;
	}
	
	if (canscroll) {
		if (by > 0) {
			(*scr_up)(yfirst, ylast, by);
			scr_lines(yfirst, ylast, by, 1);
		}
		else if (by < 0) {
			(*scr_down)(yfirst, ylast, -by);
			scr_lines(ylast, yfirst, -by, -1);
		}
	}
	else {
		scr3up(yfirst, ylast, by);
	}

	VOID fflush(fp);
}

Hidden Procedure scr_lines(yfrom, yto, n, dy) int yfrom, yto, n, dy; {
	register int y;
	char *savedata;
	char *savemode;
	
	while (n-- > 0) {
		savedata = linedata[yfrom];
		savemode = linemode[yfrom];
		for (y = yfrom; y != yto; y += dy) {
			linedata[y] = linedata[y+dy];
			linemode[y] = linemode[y+dy];
			lenline[y] = lenline[y+dy];
		}
		linedata[yto] = savedata;
		linemode[yto] = savemode;
		lenline[yto] = 0;
		if (has_xs) linemode[yto][0] = NOCOOK;
	}
}

Hidden Procedure scr1up(yfirst, ylast, n) int yfirst, ylast, n; {
	move(yfirst, 0);
	dellines(n);
	if (ylast < lines-1) {
		move(ylast-n+1, 0);
		addlines(n);
	}
}


Hidden Procedure scr1down(yfirst, ylast, n) int yfirst, ylast, n; {
	if (ylast == lines-1) {
		clear_lines(ylast-n+1, ylast);
	}
	else {
		move(ylast-n+1, 0);
		dellines(n);
	}
	move(yfirst, 0);
	addlines(n);
}


Hidden Procedure addlines(n) register int n; {
	if (par_al_str && n > 1)
			Putstr(tgoto(par_al_str, n, n));
	else {
		while (n-- > 0)
			Putstr(al_str);
	}
}


Hidden Procedure dellines(n) register int n; {
	if (par_dl_str && n > 1)
		Putstr(tgoto(par_dl_str, n, n));
	else {
		while (n-- > 0)
			Putstr(dl_str);
	}
}


Hidden Procedure scr2up(yfirst, ylast, n) int yfirst, ylast, n; {
	Putstr(tgoto(cs_str, ylast, yfirst));
	cur_y = cur_x = Undefined;
	move(ylast, 0);
	while (n-- > 0) {
		Putstr(sf_str);
		if (has_db && ylast == lines-1)
			clr_to_eol();
	}
	Putstr(tgoto(cs_str, lines-1, 0));
	cur_y = cur_x = Undefined;
}


Hidden Procedure scr2down(yfirst, ylast, n) int yfirst, ylast, n; {
	Putstr(tgoto(cs_str, ylast, yfirst));
	cur_y = cur_x = Undefined;
	move(yfirst, 0);
	while (n-- > 0) {
		Putstr(sr_str);
		if (has_da && yfirst == 0)
			clr_to_eol();
	}
	Putstr(tgoto(cs_str, lines-1, 0));
	cur_y = cur_x = Undefined;
}

/* for dumb scrolling, uses and updates internal administration */

Hidden Procedure scr3up(yfirst, ylast, by) int yfirst, ylast, by; {
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
}

Hidden Procedure lf_scroll(yto, by)
     int yto;
     int by;
{
	register int n = by;

	move(lines-1, 0);
	while (n-- > 0) {
		fputc('\n', fp);
	}
	scr_lines(0, lines-1, by, 1);
	move_lines(lines-1-by, lines-1, lines-1-yto, -1);
	clear_lines(yto-by+1, yto);
}

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
 * Synchronization, move cursor to given position (or previous if < 0).
 */

Visible Procedure trmsync(y, x) int y, x; {
#ifdef VTRMTRACE
	if (vtrmfp) fprintf(vtrmfp, "\ttrmsync(%d, %d);\n", y, x);
#endif
	check_started("trmsync");
	
	if (0 <= y && y < lines && 0 <= x && x < cols) {
		move(y, x);
		if (no_cursor) {
			Putstr(ve_str);
			no_cursor = No;
		}
	}
	else if (no_cursor == No) {
		Putstr(vi_str);
		no_cursor = Yes;
	}
	VOID fflush(fp);
}


/*
 * Send a bell, visible if possible.
 */

Visible Procedure trmbell() {
#ifdef VTRMTRACE
	if (vtrmfp) fprintf(vtrmfp, "\ttrmbell();\n");
#endif
	check_started("trmbell");
	
	Putstr(vb_str);
	VOID fflush(fp);
}


#ifdef SHOW

/*
 * Show the current internal statuses of the screen on vtrmfp.
 * For debugging only.
 */

Visible Procedure trmshow(s) char *s; {
	int y, x;
	
	if (!vtrmfp)
		return;
	fprintf(vtrmfp, "<<< %s >>>\n", s);
	for (y = 0; y < lines; y++) {
		for (x = 0; x <= lenline[y] /*** && x < cols-1 ***/ ; x++) {
			fputc(linedata[y][x], vtrmfp);
		}
		fputc('\n', vtrmfp);
		for (x = 0; x <= lenline[y] && x < cols-1; x++) {
			if (linemode[y][x]&SOBIT)
				fputc('-', vtrmfp);
			else
				fputc(' ', vtrmfp);
		}
		fputc('\n', vtrmfp);
		for (x = 0; x <= lenline[y] && x < cols-1; x++) {
			if (linemode[y][x]&XSBIT)
				fputc('+', vtrmfp);
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
 * Return the next input character, or -1 if read fails.
 * Only the low 7 bits are returned, so reading in RAW mode is permissible
 * (although CBREAK is preferred if implemented).
 * To avoid having to peek in the input buffer for trmavail, we use the
 * 'read' system call rather than getchar().
 * (The interface allows 8-bit characters to be returned, to accomodate
 * larger character sets!)
 */

Hidden int pushback= -1;

Visible int trminput() {
	char c;
	int n;

#ifdef CATCHINTR
	if (trmintrptd) {
		trmintrptd= No;
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
		c= pushback;
		pushback= -1;
		return c;
	}
#ifdef CATCHINTR
	readintrcontext = Yes;
#endif

	n= read(0, &c, 1);

#ifdef CATCHINTR
	readintrcontext = No;
#endif
	if (n <= 0)
		return -1;
	return c & 0377;
}

Hidden Procedure trmpushback(c) int c; {
	pushback= c;
}


/*
 * See if there's input available from the keyboard.
 * The code to do this is dependent on the type of Unix you have
 * (BSD, System V, ...).
 * Return value: 0 -- no input; 1 -- input; -1 -- unimplementable.
 * Note that each implementation form should first check pushback.
 *
 * TO DO:
 *	- Implement it for other than 4.x BSD! (notably System 5)
 */

#ifdef HAS_SELECT

Hidden int dep_trmavail() {
	int nfound, nfds;
	fd_set readfds;
	static struct timeval timeout= {0, 0};

	/* readfds= 1 << 0; */
	FD_ZERO(&readfds);
	FD_SET(0, &readfds);
	nfds= 0+1;
	nfound= select(nfds, &readfds, (fd_set *) NULL, (fd_set *) NULL, &timeout);
	return nfound > 0;
}

#define TRMAVAIL_DEFINED

#else /* !HAS_SELECT */

#ifdef TERMIO

Hidden int dep_trmavail() {
	int n;
	char c;
	
	stty(0, &polltty);
	n= read(0, &c, 1);
	if (n == 1)
		trmpushback((int)c);
	stty(0, &newtty);
	return n > 0;
}

#define TRMAVAIL_DEFINED

#endif /* TERMIO */

#endif /* HAS_SELECT */

#ifndef TRMAVAIL_DEFINED

Hidden int dep_trmavail() {
	return -1;
}

#endif

Visible int trmavail() {
#ifdef CATCHINTR
	if (trmintrptd) return 1;
#endif
	if (pushback >= 0)
		return 1;
#ifndef CANLOOKAHEAD
	return -1;
#else
	return dep_trmavail(); /* dependent code */
#endif
}	

/*
 * Suspend the interpreter or editor.
 * Should be called only after trmend and before trmstart!
 */

Visible int trmsuspend() {
#ifdef SIGTSTP
	SIGTYPE (*oldsig)();
	
	oldsig= signal(SIGTSTP, SIG_IGN);
	if (oldsig == SIG_IGN) {
		return subshell();
	}
	signal(SIGTSTP, oldsig);
	kill(0, SIGSTOP);
	return 1;
#else /*SIGTSTP*/
	return subshell();
#endif /*SIGTSTP*/
}

Hidden int subshell() {
	int r;

	r= system("exec ${SHELL-/bin/sh}");
	if (r == -1 || r == 127)
		return 0;
	else
		return 1;
}


/*
 * DESCRIPTION.
 *
 * This package uses termcap to determine the terminal capabilities.
 *
 * The lines and columns of our virtual terminal are numbered 
 *	y = {0...lines-1} from top to bottom, and
 *	x = {0...cols-1} from left to right,
 * respectively.
 *
 * The Visible Procedures in this package are:
 *
 * trmstart(&lines, &cols, &flags)
 * 	Obligatory initialization call (sets tty modes etc.),
 * 	Returns the height and width of the screen to the integers
 * 	whose addresses are passed as parameters, and a flag that
 *	describes some capabilities.
 *	Function return value: 0 if all went well, an error code if there
 *	is any trouble.  No messages are printed for errors.
 *
 * trmundefined()
 *	Sets internal representation of screen and attributes to undefined.
 *	This is necessary for a hard redraw, which would get optimised to
 *	oblivion,
 *
 * trmsense(sensestring, positionformat, &y, &x)
 *	Returns the cursor position through its last two parameters
 *	after a possible manual change by the user or after a mouse-click.
 *
 * trmputdata(yfirst, ylast, indent, data, mode)
 * 	Fill lines {yfirst..ylast} with data, after skipping the initial
 *	'indent' positions. It is assumed that these positions do not contain
 *	anything dangerous (like standout cookies or null characters).
 *      mode indicates whether each character should be in standout mode or not.
 *
 * trmscrollup(yfirst, ylast, by)
 * 	Shift lines {yfirst..ylast} up by lines (down |by| if by < 0).
 *
 * trmsync(y, x)
 * 	Call to output data to the terminal and set cursor position.
 *
 * trmbell()
 *	Send a (possibly visible) bell, immediately (flushing fp).
 *
 * trmend()
 * 	Obligatory termination call (resets tty modes etc.).
 *
 * You may call these as one or more cycles of:
 * 	+ trmstart
 * 	+    zero or more times any of the other routines
 * 	+ trmend
 * Trmend may be called even in the middle of trmstart; this is necessary
 * to make it possible to write an interrupt handler that resets the tty
 * state before exiting the program.
 *
 * ADDITIONAL SPECIFICATIONS (ROUTINES FOR CHARACTER INPUT)
 *
 * trminput()
 *	Return the next input character (with its parity bit cleared
 *	if any).  This value is a nonnegative int.  Returns -1 if the
 *	input can't be read any more.
 *
 * trmavail()
 *	Return 1 if there is an input character immediately available,
 *	0 if not.  Return -1 if not implementable.
 *
 * trminterrupt()
 *	Return 1 if an interrupt has occurred since the last call to
 *	trminput or trmavail, 0 else.  [Currently not implemented.]
 *	[not used by ABC]
 *
 * trmsuspend()
 *	When called in the proper environment (4BSD with job control
 *	enabled), suspends the editor, temporarily popping back to
 *	the calling shell.  The caller should have called trmend()
 *	first, and must call trmstart again afterwards.
 *	BUG: there is a timing window where keyboard-generated
 *	signals (such as interrupt) can reach the program.
 *	In non 4BSD we try to spawn a subshell.
 *	Returns 1 if suspend succeeded, 0 if it failed.
 *	(-1 iff not implementable).
 */
