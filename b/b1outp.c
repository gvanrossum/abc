/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1990. */

#include "b.h"
#include "bmem.h"

Forward Hidden Procedure c_putchr();

Visible Procedure putstr(file, s)
     FILE *file;
     string s;
{
	if (file == CONSOLE) c_putstr(s);
	else fputs(s, file);
}

Visible Procedure putchr(file, c)
     FILE *file;
     char c;
{
	if (file == CONSOLE) c_putchr(c);
	else putc(c, file);
}

Visible Procedure putnewline(file)
     FILE *file;
{
	if (file == CONSOLE) c_putnewline();
	else putc('\n', file);
}

Visible Procedure doflush(file)
     FILE *file;
{
	if (file == CONSOLE) c_flush();
	else VOID fflush(file);
}

/* the interface between interpreter and editor is rather misty;
 * the editor leaves the cursor at the start of the last line of the screen,
 * see endshow() in bed/e1scrn.c;
 * the interpreter can't use that line for normal output, because
 * the last position isn't written by trmputdata() (some terminals
 * scroll otherwise);
 *
 * The start position for the editor is set in setindentation();
 */

Hidden int winlength = 0;   /* window height */
Hidden int winwidth = 0;    /* window width */

Hidden int wincol = 0;      /* number of chars already on the line;
			     * 0 <= wincol <= winwidth
			      */

Visible Procedure init_interpreter_output(height, width)
     int height;
     int width;
{
	winlength = height;
	winwidth  = width;
	wincol    = 0;
}

Visible Procedure re_interpreter_output()
{
	/* reinitialize after an edit session */
	trmputdata(winlength, winlength, 0, "", (string)0);
	trmscrollup(0, winlength, 1);
	trmsync(winlength-1, wincol = 0);
}

Visible int getwinwidth() {
	return winwidth;
}

Visible int getwincol()
{
	/* returns current output column */
	return wincol; /* 0 <= wincol <= winwidth */
}

#define LINELENGTH 200

Visible Procedure c_putstr(s)
     string s;
{
	char buf[LINELENGTH];
	char *pnl;
	char *line;
	int len;

	for (; *s; s= ++pnl) {
		if ((pnl= strchr(s, '\n')) == NULL) {
			c_putdata(s);
			return;
		}
		len= pnl-s;
		if (len > 0) {
			if (len >= LINELENGTH)
				line= (char *) getmem((unsigned) (len+1));
			else
				line= buf;
			strncpy(line, s, len);
			line[len]= '\0';
			c_putdata(line);
			if (len >= LINELENGTH)
				freestr(line);
		}
		c_putnewline();
	}
}

Visible Procedure c_putdata(data)
	string data;
{
	int lendata;    /* total data length */
	int nlines;     /* number of lines needed */
	int extra = 0;  /* scroll one line extra if currently at end of line,
			 * e.g. wincol == winwidth
			 */
	int len;        /* length of handled data, if data doesn't fit */
	int col;
	
	if (data == NULL || *data == '\0')
	        return;
	if (wincol == winwidth) {
		extra = 1;
		wincol = 0;
	}
	lendata = strlen(data);
	for (;;) {
		nlines = (wincol + lendata - 1) / winwidth + 1;
		if (nlines <= winlength) break;
		/* data doesn't fit in window */
		trmscrollup(0, winlength, winlength-1+extra);
		trmputdata(0, winlength-1, wincol, data, (string)0);
		/* calculate length of handled data,
		 * so that intermediate results <= lendata
		 */
		len = winwidth - wincol + (winlength-1) * winwidth;
		data += len;
		lendata -= len;
		wincol = 0;
		extra = 1;
	}
	if (nlines + extra > 1)
		trmscrollup(0, winlength, nlines + extra - 1);
	trmputdata(winlength-nlines, winlength-1, wincol, data, (string)0);
	col = wincol + lendata;
	wincol = col % winwidth;
	if (wincol == 0 && col > 0)
		wincol = winwidth;
}

Hidden Procedure c_putchr(c)
     char c;
{
	if (c == '\n') {
		c_putnewline();
	}
	else {
		char buf[2];
		buf[0]= c; buf[1]= '\0';
		c_putdata(buf);
	}
}

Visible Procedure c_putnewline() {
	trmscrollup(0, winlength, 1);
	wincol = 0;
}

Visible Procedure c_flush()
{
	trmsync(winlength-1, wincol);
}

