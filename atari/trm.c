/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1990. */

/*
 * Atari ST version of vterm virtual terminal package, derived from the
 * generic version.
 *
 * Notes:
 *    - to ease coding for scrolls, the terminal is supposed to
 *	contain blanks at positions that were not written yet;
 *	the unknown rubbish that is initially on the screen can
 *	only be cleared by the caller by scrolling the whole screen up
 *	by one or more lines;
 *    - the number of lines on the terminal is assumed to be 25;
 *	the number of columns is assumed to be 80
 */

/*
 * Includes and data definitions.
 */

#include <linea.h>
#include <osbind.h>

#include "b.h"
#include "trm.h"

Forward Hidden int start_trm();
Forward Hidden int init_trm();
Forward Hidden Procedure put_str();
Forward Hidden Procedure clear_lines();
Forward Hidden Procedure clr_to_eol();
Forward Hidden Procedure lf_scroll();
Forward Hidden Procedure move_lines();
Forward Hidden Procedure move();
Forward Hidden Procedure standout();
Forward Hidden Procedure standend();
Forward Hidden int low_trmstart();
Forward Hidden Procedure low_trmend();
Forward Hidden bool low_trmsense();
Forward Hidden Procedure low_trmscrollup();
Forward Hidden Procedure low_trmbell();
Forward Hidden Procedure low_trmsync();
Forward Hidden Procedure low_clr_to_eol();
Forward Hidden Procedure low_move();
Forward Hidden Procedure low_standout();
Forward Hidden Procedure low_standend();
Forward Hidden Procedure low_home_and_clear();
Forward Hidden int low_trminput();
Forward Hidden int low_trmavail();
Forward Hidden int low_trmsuspend();
Forward Hidden int put_line();

#define Nlines		25
#define Ncols		80

#define Min(a,b) ((a) <= (b) ? (a) : (b))
#define Undefined (-1)

#define BC_CON	2

#define hidemouse() lineaa()
#define showmouse() linea9()

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

/*
 * Starting, Ending and (fatal) Error.
 */

/*
 * Initialization call.
 * Determine terminal mode.
 * Start up terminal and internal administration.
 * Return TE_OK if succeeded, error code (see ehdrs/trm.h) if trouble
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
		if ((linemode = (char**) malloc(MALLOC_ARG(lines * sizeof(char*)))) == NULL)
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
 * If the terminal cannot be asked for the current cursor position,
 * or if the string returned by the terminal is garbled,
 * the position is made Undefined.
 * This scheme can also be used for mouse clicks, if these can be made to
 * send the cursor position; the sense string can normally be empty then.
 *
 * Atari: no need to sense; just return the position after a click.
 */

Visible Procedure trmsense(sense, format, py, px)
	string sense, format; int *py, *px;
{
	bool low_trmsense();

	*py = *px = Undefined;
	if (low_trmsense(sense, format, py, px)) {
		if (*py < 0 || lines <= *py || *px < 0 || cols <= *px)
			*py = *px = Undefined;
	}
	cur_y = Undefined;
	cur_x = Undefined;
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
		hidemouse();
		clear_lines(yfirst, ylast);
		showmouse();
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

#define low_put_char(c) Bconout(BC_CON, (c))

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

/*
 * Terminal input without echo.
 */

Visible int trminput()
{
	return low_trminput();
}

/*
 * Check for character available.
 */

Visible int trmavail()
{
	return low_trmavail();
}

Visible int trmsuspend()
{
	return low_trmsuspend();
}


/* Low-level stuff #################################################### */

/*
 *	Mouse Shape and Mouse Shape Mask definitions
 */

Hidden short mouse_shape [ 16 ] = {
	0x0000,	/* 00 */
	0x7f00,	/* 01 */
	0x4000,	/* 02 */
	0x4000,	/* 03 */
	0x4fe0,	/* 04 */
	0x4800,	/* 05 */
	0x4800,	/* 06 */
	0x49f8,	/* 07 */
	0x0900,	/* 08 */
	0x0900,	/* 09 */
	0x0900,	/* 10 */
	0x0100,	/* 11 */
	0x0100,	/* 12 */
	0x0000,	/* 13 */
	0x0000,	/* 14 */
	0x0000	/* 15 */
};

Hidden short mouse_mask [ 16 ] = {
	0xff80,	/* 00 */
	0x8080,	/* 01 */
	0x8080,	/* 02 */
	0x80f0,	/* 03 */
	0x8010,	/* 04 */
	0x8010,	/* 05 */
	0x801c,	/* 06 */
	0x8004,	/* 07 */
	0xf0fc,	/* 08 */
	0x1080,	/* 09 */
	0x1080,	/* 10 */
	0x1e80,	/* 11 */
	0x0280,	/* 12 */
	0x0380,	/* 13 */
	0x0000,	/* 14 */
	0x0000	/* 15 */
};

#define Mouse_hs_x	1		/* Mouse Hot Spot x position */
#define Mouse_hs_y	1		/* Mouse Hot Spot y position */

/*
 * Mouse Handling
 */

Hidden int	mouse_x,	/* Mouse x coordinate */
		mouse_y;	/* Mouse y cordinate  */

/*
 * escape sequences
 */

#define A_CUP	"\033Y"   /* cursor position */
#define A_SGR0	"\033q"   /* set graphics rendition to normal */
#define A_SGR7	"\033p"   /* set graphics rendition to standout */
#define A_ED	"\033E"   /* erase display (and cursor home) */
#define A_EL	"\033K"   /* erase (to end of) line */

Hidden Procedure low_putstr(s)
     char *s;
{
	for (; *s; ++s)
	  low_put_char(*s);
}


Hidden int low_startmouse()
{
	long aline;				/* Pointer to aline struct */
	short mouse_array[37];			/* For mouse shape data */
	int  i;

	extern short mouse_shape[],		/* Mouse shape and mask */
		   mouse_mask[];

	mouse_array[0] = Mouse_hs_x;		/* X coordinate  hot spot */
	mouse_array[1] = Mouse_hs_y;		/* Y coordinate  hot spot */
	mouse_array[2] =  1;			/* Future use, MUST BE ONE! */
	mouse_array[3] =  0;			/* Color index for mask */
	mouse_array[4] =  1;			/* Color index for data */

	for (i=5 ; i<=20; i++)			/* Mouse mask */
		mouse_array[i] = mouse_mask[i-5];
	for (i=21 ; i<=36; i++)			/* Mouse data */
		mouse_array[i] = mouse_shape[i-21];

	appl_init();				/* Init for AES functions */
	graf_mouse (255, (void *) mouse_array);	/* Change mouse shape */
	appl_exit();				/* Exit for AES functions */

	linea0();				/* Get la_init struct */

	showmouse();

	return TE_OK;
}

Hidden int low_trmstart()
{
	int err;

	flags = 0;

	if ((err= low_startmouse()) != TE_OK) return err;

	/* If, after writing a character to position (y, max)
	   the next character will be written to (y+1, 1),
	   set auto_margins to Yes, otherwise to No
	*/
	auto_margins= Yes; 

	lines = Nlines;
	cols = Ncols;

	return TE_OK;
}

Hidden Procedure low_trmend()
{
	hidemouse();
}

Hidden bool low_trmsense(sense, format, py, px)
     char *sense;
     char *format;
     int *py;
     int *px;
{
/*	Returns the y and x position (in character sizes) of the mouse
 *	at the moment of clicking.
 *	Assumed is that the font used is the system 8x16 font (e.g. 
 *	characters are 8 pixels high and 16 wide. The upper left corner
 *	of the screen has (character) coordinate <0, 0> and the
 *	lower rigth corner <23, 79>. 
 *	The function always returns true.
 */
	*py = mouse_y /16;
	*px = mouse_x / 8;

	return Yes;
}

Hidden Procedure low_trmscrollup(yfirst, ylast, by)
     int yfirst;
     int ylast;
     int by;
{
}

Hidden Procedure low_trmbell()
{
	low_put_char('\007');
}

Hidden Procedure low_trmsync()
{
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
	low_putstr(A_CUP);
	low_put_char(y+' ');
	low_put_char(x+' ');
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

/*
 * Terminal input without echo.
 */

/* 
 *	encode_input().
 *
 *  Encode the value returned by Bconin(). Bconin() will return a
 *  long:
 *	low byte of high word:	the key's raw scan code,
 *	low byte of low word:	ASCII character,
 *				or zero if extended character.
 *
 *  - Extended characters (zero low byte of low word) are returned as their
 *     scan code and the boolean Extended is set True.
 *  - Other characters are passed as is and Extended is set to False.
 */

Hidden bool Extended;				/* Set if extended char */
Hidden unsigned char exbuff;			/* Extension buffer */

Hidden unsigned char encode_input (in_value)
     long in_value;
{
	register short c  = in_value;		/* get low word */
	register unsigned char ch = c;		/* low byte low word */

	if (c == 0) {				/* Extended character */
		ch = in_value >>16;		/* Make ch the scan code */
		Extended = Yes;			/* Set Flag */
	}
	else Extended = No;			/* Reset Flag */

	return ch;				/* Return character */
}

/*
 *	Hidden bool mouse_click( char_ptr )
 *
 *	Return 'True' ('Yes') if a mouse button (or both) is pressed.
 *	If a mouse button is pressed, the mouse's x and y coordinates (in 
 *      pixel format, not in character format) are saved in the global 
 *      variables: 
 *		'mouse_x' and 
 *		'mouse_y'.
 *
 *	The character returned must be regarded as extended and has
 *	the values: 
 *		Char:	Hex	Octal		Button
 *		=======================		======
 *		0x00	0x74 	(\164)    	left
 *		0x00	0x75 	(\165)    	right, left and right
 *
 *	The button state depends on the two lower bits of the status
 *	word for the button:
 *		Status			Mouse Action
 *      	======			============
 *		1			left button pressed
 *		2			right button pressed
 *		3 			both buttons pressed
 *
 *	'False' ('No') is returned if at moment of calling no button is 
 *	pressed.
 *	
 */

Hidden bool mouse_click(char_ptr)
     char *char_ptr;
{
	int mouse_button;		        /* For saving button state */
	
	if (MOUSE_BT != 0) {	        /* Is button pressed? */
		mouse_button = MOUSE_BT;    /* Save button state */
		while (MOUSE_BT != 0)	/* Wait till released */
		  /* Do nothing */;
		mouse_x = CUR_X;   	/* Save x */
		mouse_y = CUR_Y;	/* Save y */
		if (mouse_button == 1)	/* Left pressed ? */
		  *char_ptr = '\164';
		else 
		  *char_ptr = '\165';  	/* Right or both pressed */

		return Yes;		/* Yes, button pressed */
	}
	return No;       		/* No, no button pressed */

} /* End of mouse_click */

/*
 * low_trminput() will return a character, if available. It will test the
 * system type-ahead buffer.
 * Extended characters are handled different: trminput() will return
 * a NUL character if found one and will return the keys scan code
 * when called for the next time.
 */

Hidden int low_trminput()
{
    unsigned char c;

    if (exbuff) {			/* something in extension buffer? */
	c = exbuff;			/* get it out */
	exbuff = 0;			/* clear extension buffer */
	return c;			/* return buffer contents */
    }	

    for (;;) {

	if (Bconstat(BC_CON) == -1) {	/* char ready to be handled */
	    c = encode_input(Bconin(BC_CON));
	    if (Extended) {		/* Is it an extended character ? */
		exbuff = c;		/* Put in extension buffer */
		return 0;
	    }
	    return c;
    	} 
	else {				/* Is there a mouse klick ? */
	    if (mouse_click(&c) == Yes) {
		exbuff = c;
		return 0;
	    }
        }
    }

}

/*
 * Check if another input character is immediately available.
 * (0 -- not available; 1 -- available).
 * The extension buffer and the system type-ahead buffer are checked.
 */

Hidden int low_trmavail()
{
	if (exbuff)	            /* something in extension buffer ... */
	  return 1;
	else if (Bconstat(BC_CON) == -1) /* or in system type-ahead */
	  return 1;
	else
	  return 0;
}

Hidden int low_trmsuspend()
{
	return -1; /* unimplementable */
}
