/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1986. */

#include "b.h"
#include "bmem.h"
#include "getc.h"
#include "oper.h"
#include "port.h"

char *getenv();

/* struct tabent {int code; int deflen; string def, rep, name;} in getc.h */

/* Table of key definitions, filled by the following defaults
   and by reading definitions from a file.

   For the code field the following holds:
   code > 0:
       definitions for editor operations,
       new defs from keydefs file will be added in bed/e1getc.c,
        eliminating conflicting ones;
   code < 0:
       strings to be send to the terminal,
       any new defs from keydefs file overwrite the old ones

   Not all control characters can be freely used:
   ^Q and ^S are used by the Unix operating system
   for output flow control, and ^Z is used by BSD
   Unix systems for `job control'.
   Also note that ^H, ^I and ^M (and somtimes ^J) have their
   own keys on most keyboards and thus usually have a strong
   intuitive meaning.

   'def' fields initialized with a string starting with '=' are termcap names,
   and are replaced by the corresponding termcap entry (NULL if none);
   
   'def' fields initialized with a string starting with "&" are
   special characters for unix, and taken from tty structures.

*/

Visible struct tabent deftab[MAXDEFS] = {
	{IGNORE,	0,	NULL,		NULL,		S_IGNORE},
		/* Entry to ignore a key */

	/* if there are no or too few function or arrow keys: */
	{WIDEN,		0,	"\033w",	"ESC w",	S_WIDEN},
	{EXTEND,	0,	"\033e",	"ESC e",	S_EXTEND},
	{FIRST,		0,	"\033f",	"ESC f",	S_FIRST},
	{LAST,		0,	"\033l",	"ESC l",	S_LAST},
	{PREVIOUS,	0,	"\033p",	"ESC p",	S_PREVIOUS},
	{NEXT,		0,	"\033n",	"ESC n",	S_NEXT},
	{UPARROW,	0,	"\033k",	"ESC k",	S_UPARROW},
	{DOWNARROW,	0,	"\033j",	"ESC j",	S_DOWNARROW},
	{LEFTARROW,	0,	"\033,",	"ESC ,",	S_LEFTARROW},
		/* , below < */
	{RITEARROW,	0,	"\033.",	"ESC .",	S_RITEARROW},
		/* . below > */
	{UPLINE,	0,	"\033u",	"ESC u",	S_UPLINE},
	{DOWNLINE,	0,	"\033d",	"ESC d",	S_DOWNLINE},
	{COPY,		0,	"\033c",	"ESC c",	S_COPY},
		/* in case ^C is interrupt */

	/* function and arrow keys as in termcap;
	 * these must follow, because the first key in the helpblurb
	 * will be the last one */
	{WIDEN,		0,	"=k1",		"F1",		S_WIDEN},
	{EXTEND,	0,	"=k2",		"F2",		S_EXTEND},
	{FIRST,		0,	"=k3",		"F3",		S_FIRST},
	{LAST,		0,	"=k4",		"F4",		S_LAST},
	{PREVIOUS,	0,	"=k5",		"F5",		S_PREVIOUS},
	{NEXT,		0,	"=k6",		"F6",		S_NEXT},
	{UPLINE,	0,	"=k7",		"F7",		S_UPLINE},
	{DOWNLINE,	0,	"=k8",		"F8",		S_DOWNLINE},
	{COPY,		0,	"=k9",		"F9",		S_COPY},
	{UPARROW,	0,	"=ku",		"^",		S_UPARROW},
	{DOWNARROW,	0,	"=kd",		"v",		S_DOWNARROW},
	{LEFTARROW,	0,	"=kl",		"<-",		S_LEFTARROW},
	{RITEARROW,	0,	"=kr",		"->",		S_RITEARROW},
#ifdef GOTOCURSOR
	{GOTO,		0,	"\033g",	"ESC g",	S_GOTO},
	{GOTO,		0,	"\007",		"Ctrl-g",	S_GOTO},
	{MOUSE,		0,	NULL,		NULL,		S_MOUSE},
#endif
	{ACCEPT,	0,	"\011",		"TAB",		S_ACCEPT},
	{NEWLINE,	0,	"\012",		"LINEFEED",	S_NEWLINE},
	{NEWLINE,	0,	"\015",		"RETURN",	S_NEWLINE},
	{UNDO,		0,	"\010",		"BACKSP",	S_UNDO},
	{REDO,		0,	"\025",		"Ctrl-U",	S_REDO},
	{COPY,		0,	"\003",		"Ctrl-C",	S_COPY},
	{DELETE,	0,	"\004",		"Ctrl-D",	S_DELETE},
#ifdef RECORDING
	{RECORD,	0,	"\022",		"Ctrl-R",	S_RECORD},
	{PLAYBACK,	0,	"\020",		"Ctrl-P",	S_PLAYBACK},
#endif
	{REDRAW,	0,	"\014",		"Ctrl-L",	S_LOOK},
#ifdef HELPFUL
	{HELP,		0,	"\033?",	"ESC ?",	S_HELP},
	{HELP,		0,	"=k0",		"F10",		S_HELP},
#endif
	{EXIT,		0,	"\030",		"Ctrl-X",	S_EXIT},
	{EXIT,		0,	"\033\033",	"ESC ESC",	S_EXIT},
	
	/* These three are taken from stty settings: */
	
	{CANCEL,	0,	"&\003",	NULL,		S_INTERRUPT},
		/* take from intr char */
	{SUSPEND,	0,	"&\032",	NULL,		S_SUSPEND},
		/* take from susp char */
	{UNDO,		0,	"&\b",		NULL,		S_UNDO},
		/* take from erase char */
	
	/* The following are not key defs but string-valued options: */
	
	{TERMINIT,	0,	"=ks",		"ks (termcap)",	S_TERMINIT},
	{TERMDONE,	0,	"=ke",		"ke (termcap)",	S_TERMDONE},
#ifdef GOTOCURSOR
	{GSENSE,	0,	"",		"sense cursor",	S_GSENSE},
	{GFORMAT,	0,	"",		"cursor format",S_GFORMAT},
	{MSENSE,	0,	"",		"sense mouse",	S_MSENSE},
	{MFORMAT,	0,	"",		"mouse format",	S_MFORMAT},
#endif

	{0,		0,	NULL,		NULL,		NULL}
};

/* Merge key definitions from termcap into the default table. */

Hidden Procedure readtermcap() {
	string tgetstr();
	char buffer[1024]; /* Constant dictated by termcap manual entry */
	static char area[1024];
	string endarea= area;
	string anentry;
	struct tabent *d, *last;

	switch (tgetent(buffer, getenv("TERM"))) {

	default:
		putmess(MESS(6800, "*** Bad tgetent() return value.\n"));
		/* Fall through */
	case -1:
		putmess(MESS(6801, "*** Can't read termcap.\n"));
		/* Fall through again */
	case 0:
	putmess(MESS(6802, "*** No description for your terminal.\n"));
		immexit(1);

	case 1:
		break;
	}

	last= deftab+ndefs;
	for (d= deftab; d < last; ++d) {
		if (d->def != NULL && d->def[0] == '=') {
			anentry= tgetstr(d->def+1, &endarea);
			if (anentry != NULL && anentry[0] != '\0') {
				undefine(d->code, strlen(anentry), anentry);
				d->deflen= strlen(anentry);
				d->def= anentry;
			}
			else {
				d->def= d->rep= NULL;
				d->deflen= 0;
			}
		}
	}
}

/* Code to get the defaults for interrupt, suspend and undo/erase_char
 * from tty structs.
 */

Hidden char *intr_char= NULL;
Hidden char *susp_char= NULL;
Hidden char *erase_char= NULL;

#ifndef TERMIO
#include <sgtty.h>
#else
#include <termio.h>
#endif
#ifdef SIGNAL
#include <signal.h>
#endif

Hidden Procedure getspchars() {
#ifndef TERMIO
	struct sgttyb sgbuf;
#ifdef TIOCGETC
	struct tchars tcbuf;
#endif
	static char str[6];
	
	if (gtty(0, &sgbuf) == 0) {
		if ((int)sgbuf.sg_erase != -1 
		    &&
		    !(isprint(sgbuf.sg_erase) || sgbuf.sg_erase == ' ')
		) {
			str[0]= sgbuf.sg_erase;
			erase_char= &str[0];
		}
	}
#ifdef TIOCGETC
	if (ioctl(0, TIOCGETC, (char*)&tcbuf) == 0) {
		if ((int)tcbuf.t_intrc !=  -1) {
			str[2]= tcbuf.t_intrc;
			intr_char= &str[2];
		}
	}
#endif
#if defined(TIOCGLTC) && defined(SIGTSTP)
	{
		struct ltchars buf;
		SIGTYPE (*handler)();

		handler= signal(SIGTSTP, SIG_IGN);
		if (handler != SIG_IGN) {
			/* Shell has job control */
			signal(SIGTSTP, handler); /* Reset original handler */
			if (ioctl(0, TIOCGLTC, (char*) &buf) == 0 &&
					(int)buf.t_suspc != -1) {
				str[4]= buf.t_suspc;
				susp_char= &str[4];
			}
		}
	}
#endif /* TIOCGLTC && SIGTSTP */
#else /* TERMIO */
	struct termio sgbuf;
	static char str[6];
	
	if (ioctl(0, TCGETA, (char*) &sgbuf) == 0) {
		if ((int) sgbuf.c_cc[VERASE] != 0377
		    &&
		    !(isprint(sgbuf.c_cc[VERASE]))
		) {
			str[0]= sgbuf.c_cc[VERASE];
			erase_char= &str[0];
		}
		if ((int) sgbuf.c_cc[VINTR] != 0377) {
			str[2]= sgbuf.c_cc[VINTR];
			intr_char= &str[2];
		}
#ifdef VSUSP
		if ((int) sgbuf.c_cc[VSUSP] != 0377) {
			str[4]= sgbuf.c_cc[VSUSP];
			susp_char= &str[4];
		}
#endif
	}
#endif /* TERMIO */
}

/* The following is needed for the helpblurb */

Visible string reprchar(c) int c; {

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
	else if (c == '\177') {				/* delete */
		return "DEL";
	}
	else {
		sprintf(str, "\\%03o", c);		/* octal value */
		return str;
	}
}

Hidden Procedure add_special_chars() {
	string anentry;
	struct tabent *d, *last;
	
	last= deftab+ndefs;
	for (d= deftab; d < last; ++d) {
		if (d->def != NULL && d->def[0] == '&') {
			if (d->def[1] == '\003') /* interrupt */
				anentry= intr_char;
			else if (d->def[1] == '\b') /* undo/backspace */
				anentry= erase_char;
			else if (d->def[1] == '\032') /* suspend */
				anentry= susp_char;
			else
				anentry= NULL;
			if (anentry != NULL && anentry[0] != '\0') {
				undefine(d->code, strlen(anentry), anentry);
				d->deflen= strlen(anentry);
				d->def= anentry;
				d->rep= (string) savestr(reprchar(anentry[0]));
#ifdef MEMTRACE
				fixmem((ptr) d->rep);
#endif
			}
			else {
				d->def= d->rep= NULL;
				d->deflen= 0;
			}
		}
	}
}

#ifndef CANLOOKAHEAD
Visible char intrchar;
#endif

#ifdef KEYTRACE
extern FILE *keyfp;
#endif

Visible Procedure addspeckeys() {
	getspchars();
#ifndef CANLOOKAHEAD
	if (intr_char != NULL && *intr_char != '\0')
		intrchar = *intr_char;
#endif
#ifdef KEYTRACE
	if (keyfp)
		dumpkeys("before termcap");
#endif
	readtermcap();
#ifdef KEYTRACE
	if (keyfp)
		dumpkeys("after termcap");
#endif
	add_special_chars();
#ifdef KEYTRACE
	if (keyfp)
		dumpkeys("after special chars");
#endif
}
