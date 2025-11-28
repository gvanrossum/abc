/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1986. */

#include "b.h"
#include "getc.h"
#include "oper.h"

/* struct tabent {int code; int deflen; string def, rep, name;} in getc.h */

/* Table of key definitions, filled by the following defaults
   and by reading definitions from a file.

   For the code field the following holds:
   code > 0:
       command definitions,
       new defs will be added, eliminating conflicting ones
   code < 0:
       strings to be send to the terminal,
       any new defs overwrite the old ones

   On the IBM PC, 'extended codes' are by convention a null character
   followed by another character (usually the scan code).
   Therefore you must fill in the length of the string in the second column.
   E.g., F1 is 0-59, which we encode as 2, "\0\073" (since \073 is
   octal for 59 decimal).
   For the exact codes, see for instance the BASIC 2.0 manual,
   appendix G, or the XT Technical Reference, page 2-14.
*/

Visible struct tabent deftab[MAXDEFS] = {
	{IGNORE,	0,	NULL,		NULL,		S_IGNORE},
		/* Entry to ignore a key */
	{WIDEN,		2,	"\0\073",	"F1",		S_WIDEN},
	{EXTEND,	2,	"\0\074",	"F2",		S_EXTEND},
	{FIRST,		2,	"\0\075",	"F3",		S_FIRST},
	{LAST,		2,	"\0\076",	"F4",		S_LAST},
	{PREVIOUS,	2,	"\0\077",	"F5",		S_PREVIOUS},
	{NEXT,		2,	"\0\100",	"F6",		S_NEXT},
	{UPLINE,	2,	"\0\111",	"PgUp",		S_UPLINE},
	{DOWNLINE,	2,	"\0\121",	"PgDn",		S_DOWNLINE},
	{UPARROW,	2,	"\0\110",	"^",		S_UPARROW},
	{DOWNARROW,	2,	"\0\120",	"v",		S_DOWNARROW},
	{LEFTARROW,	2,	"\0\113",	"<-",		S_LEFTARROW},
	{RITEARROW,	2,	"\0\115",	"->",		S_RITEARROW},
	{ACCEPT,	0,	"\011",		"Tab",		S_ACCEPT},
	{NEWLINE,	0,	"\012",		"Linefeed",	S_NEWLINE},
	{NEWLINE,	0,	"\015",		"Return",	S_NEWLINE},
	{UNDO,		0,	"\010",		"Backspace",	S_UNDO},
	{REDO,		0,	"\177",		"Ctrl-Backsp",	S_REDO},
	{COPY,		2,	"\0\103",	"F9",		S_COPY},
	{DELETE,	2,	"\0\123",	"Del",		S_DELETE},
#ifdef RECORDING
	/*
	 * The IBM-PC has a problem here in ANSI.SYS mode: ctrl-P is
	 * unusable because it means Print Screen, and alt-R is unusable
	 * because it transmits 0, 19 but 19 is ctrl-S which means stop
	 * output :-(.
	 * The only reasonable place to put the things is on function keys.
	 */
	{RECORD,	2,	"\0\101",	"F7",		S_RECORD},
	{PLAYBACK,	2,	"\0\102",	"F8",		S_PLAYBACK},
#endif /* RECORDING */
	{REDRAW,	0,	"\014",		"Ctrl-L",	S_LOOK},
#ifdef HELPFUL
	{HELP,		2,	"\0\104",	"F10",		S_HELP},
#endif /* HELPFUL */
	{GOTO,		0,	"\007",		"Ctrl-G",	S_GOTO},
	{MOUSE,		0,	NULL,		NULL,		S_MOUSE},
	{EXIT,		0,	"\033\033",	"Esc Esc",	S_EXIT},
	{CANCEL,	0,	"\003",		"Ctrl-C",	S_INTERRUPT},
	{SUSPEND,       0,      "\032",         "Ctrl-Z",       S_SUSPEND},

/* string-valued: */
	{TERMINIT,	0,	"",		NULL,		S_TERMINIT},
	{TERMDONE,	0,	"",		NULL,		S_TERMDONE},
	{MFORMAT,	0,	"",		NULL,		S_MFORMAT},

	{0,		0,	NULL,		NULL,		NULL}
};

Visible Procedure addspeckeys() {
}

#ifdef KEYS

Hidden string extcode[]= {

/* 0 */		NULL,
		NULL,
		NULL,
/* 3 */		"Null",
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
/* 15 */	"BACKTAB",
		"Alt-Q",
		"Alt-W",
		"Alt-E",
		"Alt-R",
		"Alt-T",
		"Alt-Y",
		"Alt-U",
		"Alt-I",
		"Alt-O",
		"Alt-P",
/* 26 */	NULL,
		NULL,
		NULL,
		NULL,
/* 30 */	"Alt-A",
		"Alt-S",
		"Alt-D",
		"Alt-F",
		"Alt-G",
		"Alt-H",
		"Alt-J",
		"Alt-K",
		"Alt-L",
/* 39 */	NULL,
		NULL,
		NULL,
		NULL,
		NULL,
/* 44 */	"Alt-Z",
		"Alt-X",
		"Alt-C",
		"Alt-V",
		"Alt-B",
		"Alt-N",
		"Alt-M",
/* 51 */	NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
/* 59 */	"F1",
		"F2",
		"F3",
		"F4",
		"F5",
		"F6",
		"F7",
		"F8",
		"F9",
		"F10",
/* 69 */	NULL,
		NULL,
/* 71 */	"Home",
		"^",			/* Up */
		"PgUp",
		NULL,
/* 75 */	"<-",			/* Left */
		NULL,
/* 77 */	"->",			/* Right */
		NULL,
/* 79 */	"End",
		"v",			/* Down */
		"PgDn",
		"Ins",
		"Del",
/* 84 */	"Shift-F1",
		"Shift-F2",
		"Shift-F3",
		"Shift-F4",
		"Shift-F5",
		"Shift-F6",
		"Shift-F7",
		"Shift-F8",
		"Shift-F9",
		"Shift-F10",
/* 94 */	"Ctrl-F1",
		"Ctrl-F2",
		"Ctrl-F3",
		"Ctrl-F4",
		"Ctrl-F5",
		"Ctrl-F6",
		"Ctrl-F7",
		"Ctrl-F8",
		"Ctrl-F9",
		"Ctrl-F10",
/* 104 */	"Alt-F1",
		"Alt-F2",
		"Alt-F3",
		"Alt-F4",
		"Alt-F5",
		"Alt-F6",
		"Alt-F7",
		"Alt-F8",
		"Alt-F9",
		"Alt-F10",
/* 114 */	"Ctrl-PrtSc",
		"Ctrl-<-",		/* Ctrl-Left */
		"Ctrl-->",		/* Ctrl-Right */
		"Ctrl-End",
		"Ctrl-PgDn",
		"Ctrl-Home",
/* 120 */	"Alt-1",
		"Alt-2",
		"Alt-3",
		"Alt-4",
		"Alt-5",
		"Alt-6",
		"Alt-7",
		"Alt-8",
		"Alt-9",
		"Alt-0",
		"Alt--",
		"Alt-=",
/* 132 */	"Ctrl-PgUp"
};

/* assumption: extended keys are handled via two consecutive calls */

Visible string reprchar(c) int c; {
	static char str[20];
	static bool extended= No;
	

	c&= 0377;

	if (extended == Yes) {
		extended = No;
		if (c >= 0 && c < sizeof(extcode) && extcode[c] != NULL)
			return extcode[c];
		else {
			sprintf(str, "Ext-\\%03o", c);
			return str;
		}
	}
	else if (c == '\000') {	 			/* Ext. char */
		extended = Yes;
		return "";
	}
	else if ('\000' < c && c < '\040') {		/* Control char? */
		switch (c) {
			case '\010':
				return "Backspace";
			case '\011':
				return "Tab";
			case '\012':
				return "Linefeed";
			case '\015':
				return "Return";
			case '\033':
				return "Esc";
			default:
				sprintf(str, "Ctrl-%c", c|0100);
				return str;
		}
	}
	else if (c == '\040') {				/* Space? */
		return "Space";
	}
	else if ('\041' <= c && c < '\177') {		/* Printable? */
		str[0]= c; str[1]= '\0';
		return str;
	}
	else if (c == '\177') {				/* Ctrl-Backspace? */
		return "Ctrl-Backsp";
	}
	else {
		sprintf(str, "\\%03o", c);		/* Print octal value */
		return str;
	}
}

#endif /* KEYS */
