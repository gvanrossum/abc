/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1990. */

/* Generic version, as basis for porting.
 * See Portability Guide (./PORTING.DOC) for details.
 */

#include "b.h"
#include "getc.h"
#include "oper.h"

/* struct tabent {int code; int deflen; string def, rep, name;} in getc.h */

Visible struct tabent deftab[MAXDEFS] = {

	{IGNORE,	0,	NULL,		NULL,		S_IGNORE},
		/* Entry to ignore a key */

	{WIDEN,		0,	"\033w",	"ESC w",	S_WIDEN},
	{EXTEND,	0,	"\033e",	"ESC e",	S_EXTEND},
	{FIRST,		0,	"\033f",	"ESC f",	S_FIRST},
	{LAST,		0,	"\033l",	"ESC l",	S_LAST},
	{PREVIOUS,	0,	"\033p",	"ESC p",	S_PREVIOUS},
	{NEXT,		0,	"\033n",	"ESC n",	S_NEXT},
	{UPARROW,	0,	"\033k",	"ESC k",	S_UPARROW},
	{DOWNARROW,	0,	"\033j",	"ESC j",	S_DOWNARROW},
	{LEFTARROW,	0,	"\033,",	"ESC ,",	S_LEFTARROW},
	{RITEARROW,	0,	"\033.",	"ESC .",	S_RITEARROW},
	{UPLINE,	0,	"\033u",	"ESC u",	S_UPLINE},
	{DOWNLINE,	0,	"\033d",	"ESC d",	S_DOWNLINE},
	{COPY,		0,	"\033c",	"ESC c",	S_COPY},
	{GOTO,		0,	"\033g",	"ESC g",	S_GOTO},
	{MOUSE,		0,	"\033[M ",	"l-click",	S_MOUSE},
	{MOUSE,		0,	"\033[M!",	"m-click",	S_MOUSE},
	{MOUSE,		0,	"\033[M\"",	"r-click",	S_MOUSE},
	{ACCEPT,	0,	"\011",		"TAB",		S_ACCEPT},
	{NEWLINE,	0,	"\012",		"LINEFEED",	S_NEWLINE},
	{NEWLINE,	0,	"\015",		"RETURN",	S_NEWLINE},
	{UNDO,		0,	"\010",		"BACKSP",	S_UNDO},
	{REDO,		0,	"\025",		"Ctrl-U",	S_REDO},
	{DELETE,	0,	"\004",		"Ctrl-D",	S_DELETE},
	{RECORD,	0,	"\022",		"Ctrl-R",	S_RECORD},
	{PLAYBACK,	0,	"\020",		"Ctrl-P",	S_PLAYBACK},
	{REDRAW,	0,	"\014",		"Ctrl-L",	S_LOOK},
	{HELP,		0,	"\033?",	"ESC ?",	S_HELP},
	{EXIT,		0,	"\030",		"Ctrl-X",	S_EXIT},
	{CANCEL,	0,	"\003",         "Ctrl-C",       S_INTERRUPT},
	{SUSPEND,	0,	"\032",         "Ctrl-Z",	S_SUSPEND},

/* string-valued: */
	{TERMINIT,	0,	"\033[?9h",	"init mouse",  	S_TERMINIT},
	{TERMDONE,	0,	"\033[?9l", 	"end mouse",    S_TERMDONE},
	{GSENSE,	0,	"",		"sense cursor",	S_GSENSE},
	{GFORMAT,	0,	"",		"cursor format",S_GFORMAT},
	{MSENSE,	0,	"",		"sense mouse",	S_MSENSE},
	{MFORMAT,	0,	"%r%+!%+!",	"mouse format",	S_MFORMAT},

	{0, 0, NULL, NULL, NULL} /* keep this entry as the last one! */
};

/*
 * For the mystical things concerning MOUSE, MFORMAT and initialising/ending
 * mouse in TERMINIT and TERMDONE, see trmsense() in trm.c.
 */

Visible string reprchar(c)
     int c;
{
	static char str[20];

	c&= 0377;

	if (c == '\000') {				/* null char */
		return "NULL";
	}
	else if ('\000' < c && c < '\040') {		/* control char */
		switch (c) {
			case '\010':
				return "BACKSP";
			case '\011':
				return "TAB";
			case '\012':
				return "LINEFEED";
			case '\015':
				return "RETURN";
			case '\033':
				return "ESC";
			default:
				sprintf(str, "Ctrl-%c", c|0100);
				return str;
			}
		}
	else if (c == '\040') {				/* space */
		return "SPACE";
	}
	else if ('\041' <= c && c < '\177') {		/* printable char */
		str[0]= c; str[1]= '\0';
		return str;
	}
	else {
		sprintf(str, "\\%03o", c);		/* octal value */
		return str;
	}
}

#ifndef CANLOOKAHEAD
Visible char intrchar;
#endif

Visible Procedure addspeckeys()
{
	/* First, if you can't lookahead in the input queue,
	 * set the variable 'intrchar' to the keyboard interrupt character
	 */
#ifndef CANLOOKAHEAD
	/* ..... */
#endif

	/* Next, for every entry that you want to add to the keybindings
	 * table, call the routine addkeydef() for the five elements:
	 *          addkeydef(code, deflen, def, rep, name)
	 */

	/* ...... */
}
