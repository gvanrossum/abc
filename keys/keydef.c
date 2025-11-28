/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1989. */

/* abckeys -- create a key definitions file interactively */

#include "b.h"
#include "bfil.h"
#include "bmem.h"
#include "oper.h"
#include "getc.h"
#include "trm.h"
#include "release.h"
#include "keydef.h"
#include "port.h"

char *getenv();

Forward Hidden Procedure endprocess();
Forward Hidden Procedure usage();
Forward Hidden Procedure Init();
Forward Hidden Procedure fini();
Forward Hidden Procedure init_strings();
Forward Hidden Procedure sendinistring();
Forward Hidden Procedure init_term();
Forward Hidden Procedure Re_errfile();
Forward Hidden Procedure Re_outfile();
Forward Hidden Procedure checkwinsize();
Forward Hidden Procedure clearscreen();
Forward Hidden Procedure init_bindings();
Forward Hidden Procedure fini_term();
Forward Hidden Procedure putdata();
Forward Hidden Procedure nextline();
Forward Hidden Procedure C_flush();
Forward Hidden Procedure asktocontinue();
Forward Hidden Procedure init_buffers();
Forward Hidden Procedure savputrepr();
Forward Hidden bool illegal();
Forward Hidden bool unlawful();
Forward Hidden bool rep_in_use();
Forward Hidden Procedure Process();
Forward Hidden bool is_quit();
Forward Hidden bool is_init();
Forward Hidden Procedure scrolloffheading();
Forward Hidden Procedure definebinding();
Forward Hidden Procedure showbindings();
Forward Hidden Procedure delbindings();
Forward Hidden int asktodelete();
Forward Hidden Procedure delentry();
Forward Hidden string makereprofdef();
Forward Hidden Procedure delprompt();
Forward Hidden Procedure delhelp();
Forward Hidden Procedure definemouse();
Forward Hidden bool defmouse();
Forward Hidden bool toformat();
Forward Hidden Procedure prcnt2toprcnt3();
Forward Hidden Procedure nomouse();
Forward Hidden Procedure init_ignore();
Forward Hidden string findoldrepr();
Forward Hidden Procedure putkeydefs();
Forward Hidden Procedure init_newfile();
Forward Hidden Procedure openkeyfile();
Forward Hidden Procedure closekeyfile();
Forward Hidden Procedure put_table();
Forward Hidden Procedure put_strings();
Forward Hidden Procedure put_def();
Forward Hidden Procedure help();
Forward Hidden Procedure shorthelp();
Forward Hidden bool morehelp();
Forward Hidden Procedure longhelp();

Visible bool vtrmactive= No;

Hidden char fmtbuf[BUFSIZ];	/* to make formatted messages */

Visible Procedure immexit(status) int status; {
	endprocess(status);
}

Hidden FILE *errfile;
Hidden FILE *outfile;

Visible Procedure flushout() {
	if (outfile == CONSOLE) C_flush();
	else VOID fflush(outfile);
}

Visible Procedure flusherr() {
	if (errfile == CONSOLE) C_flush();
	else VOID fflush(errfile);
}

Visible Procedure putserr(s) string s; {
	if (errfile != CONSOLE) {
		fputs(s, errfile);
	}
	else {
		char *line = NULL;
		char *nl;
		int len;

		for (; *s; s = ++nl) {
			if ((nl = strchr(s, '\n')) == NULL) {
				putdata(s, Yes);
				break;
			}
			len= nl - s;
			if (line == NULL)
				line= (char *) getmem((unsigned) (strlen(s)+1));
			strncpy(line, s, len);
			line[len]= '\0';
			putdata(line, Yes);
			nextline();
		}
		if (line != NULL) freemem((ptr) line);
	}
}

#ifdef VTRMTRACE
Visible FILE *vtrmfp= NULL;
	/* -V vtrmfile: trace typechecker on vtrmfile; abc only */
#endif

#ifdef KEYTRACE
Visible FILE *keyfp= NULL;
       /* -K keyfile: dump keybindings at various stages to keyfile; abc only */
#endif

extern int errcount; /* Number of errors detected in key definitions */

/******************************************************************/

#define SNULL ((string) NULL)

/*
 * definitions in deftab[0..nharddefs-1] are determined in ../???/keys.c;
 * hardcoded, read in from termcap, and/or taken from tty-chars
 */

Visible int nharddefs;

/*
 * definitions in deftab[nharddefs..nfiledefs-1] come from current keysfile
 * (read in e1getc.c)
 */

Hidden int nfiledefs;

/*
 * The new definitions the user supplies in this program are keep()ed
 * in deftab[nfiledefs..ndefs-1]
 */


/* 
 * The table can than be written to the new keydefinitions file:
 * first the definitions from the old keydefinitions file
 * that are still valid, in [nharddefs.. nfiledefs-1],
 * then the new ones, in [nfiledefs..ndefs-1].
 */

typedef struct oper {
	int code;		/* returned by getoperation() in ABC editor */
	string name;		/* operation name */
	int allowed;		/* may process */
	string descr;		/* long description */
} operation;

Hidden operation oplist[]= {
	{WIDEN,		S_WIDEN,	0, "Widen focus"},
	{EXTEND,	S_EXTEND,	0, "Extend focus"},
	{FIRST,		S_FIRST,	0, "Focus to first contained item"},
	{LAST,		S_LAST,		0, "Focus to last contained item"},
	{PREVIOUS,	S_PREVIOUS,	0, "Focus to previous item"},
	{NEXT,		S_NEXT,		0, "Focus to next item"},
	{UPLINE,	S_UPLINE,	0, "Focus to whole line above"},
	{DOWNLINE,	S_DOWNLINE,	0, "Focus to whole line below"},
	{UPARROW,	S_UPARROW,	0, "Make hole, move up"},
	{DOWNARROW,	S_DOWNARROW,	0, "Make hole, move down"},
	{LEFTARROW,	S_LEFTARROW,	0, "Make hole, move left"},
	{RITEARROW,	S_RITEARROW,	0, "Make hole, move right"},
	{GOTO,		S_GOTO,		0, "New focus at cursor position"},
	{MOUSE,		S_MOUSE,	0, "New focus at position of mouse click"},
	{ACCEPT,	S_ACCEPT,	0, "Accept suggestion, goto hole"},
	{NEWLINE,	S_NEWLINE,	0, "New line, or decrease indent"},
	{UNDO,		S_UNDO,		0, "Undo effect of last key pressed"},
	{REDO,		S_REDO,		0, "Redo last UNDOne key"},
	{COPY,		S_COPY,		0, "Copy focus to/from buffer"},
	{DELETE,	S_DELETE,	0, "Delete focus (to buffer if empty)"},
	{RECORD,	S_RECORD,	0, "Start/stop recording keystrokes"},
	{PLAYBACK,	S_PLAYBACK,	0, "Play back recorded keystrokes"},
	{REDRAW,	S_LOOK,		0, "Redisplay the screen"},
	{HELP,		S_HELP,		0, "Display summary of keys"},
	{EXIT,		S_EXIT,		0, "Finish unit or execute command"},
	{CANCEL,	S_INTERRUPT,	0, "Interrupt a computation"},
	{SUSPEND,	S_SUSPEND,	0, "Suspend the process"},
	{IGNORE,	S_IGNORE,	0, "Unbind this key sequence"},
	{TERMINIT,	S_TERMINIT,	0, "string to be sent to the screen at startup"},
	{TERMDONE,	S_TERMDONE,	0, "string to be sent to the screen upon exit"},
	/* last entry, op->name == SNULL : */
	{0, 		SNULL, 		0, SNULL} 
};

Hidden int stringcode[]= {
	TERMINIT, TERMDONE, GSENSE, GFORMAT, MFORMAT
};

#define ONULL ((operation *) NULL)

Hidden operation *findoperation(name) string name; {
	operation *op;

	for (op= oplist; op->name != SNULL; op++) {
		if (strcmp(op->name, name) == 0)
			return op;
	}
	return ONULL;
}

Visible Procedure confirm_operation(code, name) int code; string name; {
	operation *op;

	for (op= oplist; op->name != SNULL; op++) {
		if (code == op->code) {
			op->allowed= 1;
			op->name= name; /* to be sure */
		}
	}
}

#define Printable(c)	(isascii(c) && (isprint(c) || (c) == ' '))
#define CRLF(c)		(Creturn(c) || Clinefeed(c))
#define Creturn(c)	((c) == '\r')
#define Clinefeed(c)	((c) == '\n')
#define Cbackspace(c)	((c) == '\b')
#define Ctab(c)		((c) == '\t')
#define Cspace(c)	((c) == ' ')

#define Empty(d)	(strlen(d) == 0)
#define Val(d)		((d) != SNULL && !Empty(d))

#define ValDef(d)       ((d)->def != SNULL && (d)->deflen > 0)
#define ValRep(d)       ((d)->rep != SNULL && strlen((d)->rep) > 0)

#define Equal(s1, s2)	(strcmp(s1, s2) == 0)

/****************************************************************************/

Hidden string newfile= SNULL;	/* name for new keydefinitions file */

main(argc, argv) int argc; char *argv[]; {
	if (argc != 1) /* no arguments allowed */
		usage();

	Init();
	
	Process();
	
	fini();
	
	return 0;
}

/****************************************************************************/

/* immediate exit */

Hidden Procedure endprocess(status) int status; {
	fini_term();
	exit(status);
}

Hidden Procedure usage() {
	putserr("*** abckeys: no arguments allowed\n");
	endprocess(-1);
}

Visible Procedure syserr(m) int m; {
	sprintf(fmtbuf, "*** System error: %s\n", getmess(m));
	putserr(fmtbuf);
	endprocess(-1);
}

Visible Procedure memexh() {
	static bool beenhere= No;
	if (beenhere) endprocess(-1);
	beenhere= Yes;
	putserr("*** Sorry, memory exhausted\n");
	endprocess(-1);
}

/****************************************************************************/

Hidden Procedure Init() {
#ifdef MEMTRACE
	initmem();
#endif
	errfile= stderr; /* sp 20010221 */
	outfile= stdout; /* sp 20010221 */
	initmess();
	initfile();
	initkeys();		/* fills deftab and ndefs in e1getc.c */
	nfiledefs= ndefs;
	
	init_newfile();
	init_ignore();
	init_term();
	init_strings();
	init_bindings();
	init_buffers();
}

Hidden Procedure fini() {
#ifdef MEMTRACE
	fini_buffers();
#endif
	fini_term();
}

/****************************************************************************/

#define DNULL (tabent *) NULL

Hidden tabent *findstringentry(code) int code;  {
	tabent *d;

	for (d= deftab+ndefs-1; d >= deftab; d--) {
		if (code == d->code)
			return d;
	}
	return DNULL;
}

Hidden Procedure init_strings() {
	sendinistring();
}

/* Output a string to the terminal */

Hidden Procedure outstring(str) string str; {
	putdata(str, Yes);
	nextline();
}

Hidden bool inisended= No;

Hidden Procedure sendinistring() {
	tabent *d;

	if (((d=findstringentry(TERMINIT)) != DNULL) && ValDef(d)) {
		outstring(d->def);
		inisended= Yes;
	}
}

Hidden Procedure sendendstring() {
	tabent *d;

	if (!inisended)
		return;
	if (((d=findstringentry(TERMDONE)) != DNULL) && ValDef(d)) {
		outstring(d->def);
	}
}

/****************************************************************************/

/* screen stuff */

Hidden struct screen {
	int yfirst, ylast;
	int width;
	int y, x;
} win;

Hidden Procedure init_term() {
	int height, width, flags;
	int err;

	err= trmstart(&height, &width, &flags);
	if (err != TE_OK) {
		if (err <= TE_DUMB)
			putserr("*** Bad $TERM or termcap, or dumb terminal\n");
		else if (err == TE_BADSCREEN)
			putserr("*** Bad SCREEN environment\n");
		else
			putserr("*** Cannot reach keyboard or screen\n");

		exit(1);
	}
	vtrmactive= Yes;
	Re_errfile();
	Re_outfile();
	win.yfirst= 0;
	win.ylast= height-1;
	win.width= width;
	win.x= 0;
	win.y= win.ylast-1;
	

	if (errcount != 0) /* errors found reading definitions */
		asktocontinue();

	trmscrollup(win.yfirst, win.ylast, 1);
	checkwinsize(width, height);   /* can exit */
	clearscreen(); 
}

#define f_interactive(file) (isatty(fileno(file)))

Hidden Procedure Re_errfile() {
	if (f_interactive(errfile) && vtrmactive) {
		errfile= CONSOLE;
	}
}

Hidden Procedure Re_outfile() {
	if (f_interactive(outfile) && vtrmactive) {
		outfile= CONSOLE;
	}
}

#define MINWIDTH 75
#define MINHEIGHT 24

Hidden Procedure checkwinsize(width, height) int width, height; {
	if (width < MINWIDTH || height < MINHEIGHT) {
		sprintf(fmtbuf,
"*** Sorry, too small screen size; needed at least %dx%d; giving up\n",
		MINHEIGHT, MINWIDTH);
		putserr(fmtbuf);
		endprocess(-1);
	}

}

/* 
 * clearing the screen is done by scrolling instead of putting empty data
 * because there are systems (MSDOS, ANSI) where the latter leaves rubbish
 * on the screen
 */
 
Hidden Procedure clearscreen() {
	trmscrollup(0, win.ylast, win.ylast + 1);
	win.yfirst= win.y= 0;
	win.x= 0;
}

Hidden int hlp_yfirst;
Hidden int hlp_nlines;

#define Upd_bindings() putbindings(hlp_yfirst)

Hidden Procedure init_bindings() {
	setup_bindings(win.width, &hlp_nlines);
}

Hidden Procedure set_windows(yfirst) int yfirst; {
	hlp_yfirst= yfirst;
	win.yfirst= hlp_yfirst + hlp_nlines + 1;
	win.y= win.yfirst;
	win.x= 0;
}

Hidden Procedure clearwindow() {
	trmputdata(win.yfirst, win.ylast, 0, "", (string)0);
	win.y= win.yfirst;
	win.x= 0;
	trmsync(win.y, win.x);
}

Hidden Procedure redrawscreen() {
	clearscreen();
	set_windows(0);
	bind_all_changed();
	Upd_bindings();
}

Hidden Procedure fini_term() {
	if (vtrmactive) {
#ifdef MEMTRACE
		fini_bindings();
#endif
		sendendstring();
		trmend();
	}
	vtrmactive= No;
}

#define WinCRLF(xzero) (win.x == win.width || ((xzero) == Yes && win.x > 0))

Hidden Procedure setstartposition(xzero)
     bool xzero;
     /* If Yes, start in first line position, next line if win.x > 0;
      * if No, from position win.x of the current line
      */
{
	if (WinCRLF(xzero)) {
		++win.y;
		win.x = 0;
	}
}

Hidden Procedure getstartposition(xzero, y, x)
     bool xzero;
     int *y, *x;
{
	if (WinCRLF(xzero)) {
		*y = win.y+1;
		*x = 0;
	}
	else {
		*y = win.y;
		*x = win.x;
	}
}

Hidden Procedure putdata(data, xzero)
     string data;
     bool xzero; /* if Yes, start in first line position */
{
	int lendata;    /* total data length */
	int nlines;     /* number of lines needed */
	int len;        /* length of handled data */
	int col;
	int ylast;
	
	if (data == NULL)
	        return;
	setstartposition(xzero);
	lendata = strlen(data);
	nlines = (win.x + lendata - 1) / win.width + 1;
	for (;;) {
		if (nlines <= win.ylast - win.y)
			break;

		if ((win.y == win.yfirst && win.x == 0) ||
		    (win.y < win.ylast && win.x > 0)
		   ) {
			ylast = win.x == 0 ? win.ylast - 1 : win.y;
			trmputdata(win.y, ylast, win.x, data, (string)0);
			nlines -= ylast - win.y + 1;
			len = (ylast - win.y + 1) * win.width - win.x;
			data += len;
			lendata -= len;
			win.x = 0;
			win.y = ylast + 1;
		}
		asktocontinue();
		clearwindow();
	}
	trmputdata(win.y, win.y + nlines - 1, win.x, data, (string)0);
	win.y += nlines - 1;
	col = win.x + lendata;
	win.x = col % win.width;
	if (win.x == 0 && col > 0)
		win.x = win.width;
	trmsync(win.y, win.x);
}

#define SPACESIZE 100   /* initial number of needed spaces */

/* message() calls putdata() but first it insert spaces in the data to avoid
 * the split of words across lines.
 */

Hidden Procedure message(data, xzero)
     string data;
     bool xzero; /* if Yes, start in first line position */
{
	char *buf, *pbuf;
	int lendata, len;
	char *p, *q;
	int nspaces = 0;
	int nextend;
	int y, x;

	if (data == NULL)
		return;
	getstartposition(xzero, &y, &x);
	lendata = strlen(data);
	buf = (char *) getmem((unsigned) (lendata+SPACESIZE+1));
	pbuf = buf;
	len = win.width - x; /* space on rest of line */
	for (;;) {
		if (lendata <= len) {
			strcpy(pbuf, data);
			break;
		}
		p = q = data + len;
		/* search a breakpoint (space) */
		while (q > data && *q != ' ') --q;
		if (q == p || q == data) {
			strncpy(pbuf, data, len);
			pbuf += len;
		}
		else {
			len = q - data;
			strncpy(pbuf, data, len);
			pbuf += len;
			/* extend with spaces */
			nspaces += p-q;
			if (nspaces >= SPACESIZE) {
				*pbuf = '\0';
				nextend = strlen(buf) + SPACESIZE;
				regetmem(&buf, (unsigned) (nextend+1));
				pbuf = buf + strlen(buf);
				nspaces = 0;
			}
			while (q++ < p) *pbuf++ = ' ';
		}
		data += len;
		lendata -= len;
		/* no spaces at begin of line */
		while (lendata > 0 && *data == ' ') {
			++data;
			--lendata;
		}

		len = win.width;
	}
	putdata(buf, xzero);
	freemem((ptr) buf);
}

Hidden Procedure nextline() {
	if (win.y >= win.ylast)
		trmscrollup(win.yfirst, win.ylast, 1);
	else
		++win.y;
	trmsync(win.y, win.x = 0);
}

Hidden Procedure C_flush() {
	trmsync(win.y, win.x);
}

#define MAXBUFFER 81

Hidden string mkstandout(data) string data; {
	static char buffer[MAXBUFFER];
	string cp= buffer;
	
	while (*data) {
		*cp= STANDOUT;
		cp++;
		data++;
	}

	return (string) buffer;
}

#define CONTINUE_PROMPT "Press [SPACE] to continue "

Hidden Procedure asktocontinue() {
	int c;
	int y= win.y;
	string data= CONTINUE_PROMPT;


	if (y < win.ylast) ++y;
	  
	trmputdata(y, y, 0, data, mkstandout(data));
		/*
		 * putdata() isn't called to avoid a call of nextline();
		 * there is no harm in that if the data can fit on one line
		 */
	trmsync(y, strlen(data));
	for (;;) {
		c= trminput();
		if (Cspace(c) || c == EOF)
			break;
		trmbell();
	}
	trmputdata(y, y, 0, "", (string)0);
}

/****************************************************************************/

/* buffer stuff */

Hidden bufadm definpbuf;	/* to save definitions from input */
Hidden bufadm repinpbuf;	/* to save representations from input */
Hidden bufadm reprbuf;		/* to save reprs from defs */

Hidden Procedure init_buffers() {
	bufinit(&definpbuf);
	bufinit(&repinpbuf);
	bufinit(&reprbuf);
}

#ifdef MEMTRACE

Hidden Procedure fini_buffers() {
	buffree(&definpbuf);
	buffree(&repinpbuf);
	buffree(&reprbuf);
}

#endif

Hidden string getbuf(bp) bufadm *bp; {
	bufpush(bp, '\0');
	return (string) bp->buf;
}

Hidden string savedef(def, len) string def; int len; {
	ptr p= (ptr) getmem((unsigned) len + 1);
	ptr q= p;
	while (len > 0) {
		*q++= *def++;
		len--;
	}
	return p;
}

Hidden bool equdef(len1, def1, len2, def2) int len1, len2; string def1, def2; {
	register int i;
	
	if (len1 != len2)
		return No;
	for (i= 0; i < len1; i++)
		if (def1[i] != def2[i])
			return No;
	return Yes;
}

/****************************************************************************/

#define MAXAVAILABLE 100

Hidden int available[MAXAVAILABLE];	/* save chars from trmavail() */
Hidden int navailable= 0;		/* nr of available chars */
Hidden int iavailable= 0;		/* next available character */

/*
 * attempt to recognize a key, as a sequence of char's using trmavail();
 * it works if the user presses the keys one after another not too fast;
 * be careful: if trmavail() isn't implemented it still has to work!
 * returns -1 for EOF, 0 for extended chars, >0 for 'normal' chars.
 */

#define Single 1
#define Multiple 2

Hidden int inkey(pc) int *pc; {
	int c;

	if (iavailable != navailable) {		/* char in buffer */
		*pc= available[iavailable++];
		if (iavailable == navailable)
			iavailable= navailable= 0;
		return Single;
	}

	c= trminput();

	while (c != EOF && trmavail() == 1) {
		available[navailable++]= c;
		c= trminput();
	}
	if (navailable == 0) {		/* only a single char available */
		*pc= c;
		return Single;
	}
	else {
		available[navailable++]= c;
		return Multiple;
	}
}

Hidden bool equal(l1, s1, l2, s2) int l1, l2; string s1, s2; {
	if (l1 != l2)
		return No;
	while (l1 > 0) {
		if (*s1++ != *s2++)
			return No;
		--l1;
	}
	return Yes;
}

Hidden string findrepr(len, def) int len; string def; {
	tabent *d;

	for (d= deftab+ndefs-1; d >= deftab; d--) {
		if (ValDef(d)
		    && equal(d->deflen, d->def, len, def)
		    && ValRep(d))
			return d->rep;
	}
	return findoldrepr(len, def);
}

/*
 * try to find a representation for the whole sequence in the buffer
 */

Hidden bool knownkeysequence(pnkey, key, rep) int *pnkey; string *key, *rep; {
	string pkey;
	int n;

	if (navailable < 2)			/* no sequence */
		return No;

	/* make sequence */
	*key= pkey= (string) getmem((unsigned) (navailable+1));
	for (n= 0; n < navailable; n++)
		*pkey++= available[n];
	*pkey= '\0';
	*pnkey= navailable;
	if ((*rep= findrepr(*pnkey, *key)) != SNULL) {
		iavailable= navailable= 0; 	/* empty buffer */
		return Yes;
	}
	freemem((ptr) *key);
	return No;
}

/****************************************************************************/

/*
 * get a key sequence from input, delimited by \r or \n
 * if you want that delimiter in your binding,
 * enclose the entire binding with single or double quotes
 */

#define NEW_KEY	"Press new key(s) for %s (%s)"

#define Quote(c) ((c) == '\"' || (c) == '\'')

Hidden string ask_definition(op, pdeflen, prepr)
	operation *op; int *pdeflen; string *prepr;
{
	int c;
	string def;
	int deflen;
	string repr;
	bufadm *dp= &definpbuf;
	bufadm *rp= &reprbuf;
	char quot_repr[20];
	bool quoting= No;
	bool first= Yes;

	sprintf(fmtbuf, NEW_KEY, op->name, op->descr);
	putdata(fmtbuf, Yes);
	nextline();

	bufreinit(dp);
	bufreinit(rp);

	for (;; first= No) {

		if (inkey(&c) == Multiple) {
			if (knownkeysequence(&deflen, &def, &repr)) {
				savputrepr(rp, repr);	/* save and put repr */
				while (deflen > 0) {	/* save key */
					bufpush(dp, *def);
					deflen--; def++;
				}
				freemem((ptr) def);
				continue;
			}
			else inkey(&c); /* get char out of buffer */
		}

		if (c == EOF)
			break;
		if (CRLF(c)) {		/* end of key sequence */
			if (!quoting)
				break;
			if (Equal(repr, quot_repr)) {
					/* pop quote from key buffer: */	
				--(dp->pbuf);
					/* pop quote from rep buffer: */
				rp->pbuf-= strlen(repr) + 1;
				break;
			}
		}
		if (first && Quote(c)) {
			quoting= Yes;
			repr= reprchar(c);
			strcpy(quot_repr, repr);
			putdata(repr, No);	/* no save */
			putdata(" ", No);	
			repr= "";		/* to prevent equality above */
		}
		else {
			repr= reprchar(c);
			savputrepr(rp, repr);	/* save and put repr */
			bufpush(dp, c);		/* save key */
		}
	}
	*prepr= getbuf(rp);

	*pdeflen= dp->pbuf - dp->buf;
	return dp->buf;
}

/* save and put the representation */

Hidden Procedure savputrepr(rp, repr) bufadm *rp; string repr; {
	if (strlen(repr) > 0) {
		/* save */
		if (rp->pbuf != rp->buf) /* not the first time */
			bufpush(rp, ' '); 
		bufcpy(rp, repr);

		/* put */
		putdata(repr, No);
		putdata(" ", No);
	}
}

Hidden string new_definition(op, pdeflen, prepr)
	operation *op; int *pdeflen; string *prepr;
{
	string def;

	if (op == ONULL)
		return SNULL;
	for (;;) {
		def= ask_definition(op, pdeflen, prepr);
		if (op->code < 0) /* string-valued */
			return def;
		if (!illegal(def, *pdeflen))
			return def;
	}
}

Hidden bool illegal(def, deflen) string def; int deflen; {
	if (deflen == 0)
		return No;
	if  (Printable(*def)) {
		sprintf(fmtbuf, E_ILLEGAL, *def);
		putdata(fmtbuf, Yes);
		return Yes;
	}
#ifndef CANLOOKAHEAD
	if (intrchar != '\0') {
		int i;
		for (i= 0; i<deflen; i++) {
			if (intrchar == def[i]) {
				putdata(E_INTERRUPT, Yes);
				return Yes;
			}
		}
	}
#endif
	return No;
}

/****************************************************************************/

Hidden bool headingonscreen= No;

/*
 * getinput() reads characters from input delimited by \r or \n 
 */
 
Hidden string getinput(bp)  bufadm *bp; {
	int c;
	char echo[2];

	echo[1]= '\0';
	bufreinit(bp);
	for (;;) {
		c= trminput();
		if (c == EOF || CRLF(c))
			break;

		if (Cbackspace(c)) {
			if (bp->pbuf == bp->buf)		/* no chars */
				trmbell();
			else {
				if (win.x == 0) {	/* begin of line */
					if (win.y > win.yfirst)
						--win.y;
					win.x= win.width;
				}
				--win.x;
				putdata("", No);
				--(bp->pbuf);	/* pop character from buffer */
			}
		}
		else if (Printable(c)) {
			echo[0]= c;
			if (headingonscreen && win.x == win.width &&
			    win.y == win.ylast - 1) {
				scrolloffheading();
			}
			putdata(echo, No);
			bufpush(bp, c);
		}
		else trmbell();
	}
	return getbuf(bp);
}

/****************************************************************************/

#define ALPHA_REP "Enter an alpha-numeric representation for this definition"

#define DFLT_REP "[default %s] "

Hidden string ask_representation(dfltrep) string dfltrep; {
	int len= strlen(DFLT_REP) + strlen(dfltrep);
	char *dflt= (char *) getmem((unsigned) (len+1));
	/* we don't use fmtbuf, because the 'dfltrep' can be very long */

	putdata(ALPHA_REP, Yes);
	sprintf(dflt, DFLT_REP, dfltrep);
	putdata(dflt, Yes);
	freemem((ptr) dflt);
	return getinput(&repinpbuf);
}

Hidden string new_representation(dfltrep, deflen, def)
	string dfltrep, def; int deflen;
{
	string repr;

	for (;;) {
		repr= ask_representation(dfltrep);

		if (Empty(repr)) /* accept default */
			return dfltrep;
		if (unlawful(repr) || rep_in_use(repr, deflen, def))
			continue; 
		return repr;
	}
}

Hidden bool unlawful(rep) string rep; {
	for (; *rep; rep++) {
		if (!Printable(*rep)) {
			putdata(E_UNLAWFUL, Yes);
			return Yes;
		}
	}

	return No;
}

Hidden bool rep_in_use(rep, deflen, def) string rep, def; int deflen; {
	tabent *d;

	for (d= deftab; d < deftab+ndefs; d++) {
		if (ValRep(d) && Equal(rep, d->rep)
		    &&
		    ValDef(d) && !equal(deflen, def, d->deflen, d->def)
		    &&
		    d->code != DELBIND
		   ) {
			sprintf(fmtbuf, E_IN_USE, d->name);
			putdata(fmtbuf, Yes);
			return Yes;
		}
	}
	return No;
}

/****************************************************************************/

Hidden Procedure keep(code, name, deflen, def, rep)
	int code, deflen; string name, def, rep;
{
	if (ndefs == MAXDEFS) {
		putdata(E_TOO_MANY, Yes);
		return;
	}
	undefine(code, deflen, def);
	deftab[ndefs].code= code;
	deftab[ndefs].name= name;
	deftab[ndefs].deflen= deflen;
	deftab[ndefs].def= (string) savedef(def, deflen);
	deftab[ndefs].rep= (string) savestr(rep);
	ndefs++;
}

Hidden Procedure store(code, name, deflen, def, rep)
	int code, deflen; string name, def, rep;
{
	tabent *d;

	if (code > 0) {
		keep(code, name, deflen, def, rep);
	}
	else {	/* code < 0; string-valued entry */
		/* find the place matching name to replace definition */
	        for (d= deftab; d < deftab+ndefs; ++d) {
			if (code == d->code) {
				d->deflen= deflen;
	                       	d->def= (string) savedef(def, deflen);
	                       	d->rep= (string) savestr(rep);
	                       	break;
			}
		}
	}
	bind_changed(code);
}

/****************************************************************************/

#define I_OP_PROMPT "Enter operation [? for help]: "
#define OP_PROMPT   "Enter operation: "

Hidden string ask_name(prompt) string prompt; {
	putdata(prompt, Yes);
	return getinput(&definpbuf);
}

Hidden int nheadlines= 0;

Hidden Procedure print_heading() {
	sprintf(fmtbuf, ABC_RELEASE, RELEASE);
	message(fmtbuf, Yes);
	message(COPYRIGHT, Yes);
	message(HEADING, Yes);
	nextline();
	nextline();
	headingonscreen= Yes;
	nheadlines= win.y;
}

Hidden Procedure Process() {
	operation *op;
	string name;
	bool show;
	bool del;
	bool first= Yes;

	clearscreen();
	
	print_heading();

	/* hlp_nlines > win.ylast - win.yfirst: error !! */
	
	if (win.y + hlp_nlines + 1 >= win.ylast - win.yfirst) {
		/* critical situation; scroll off heading first */
		asktocontinue();
		scrolloffheading();
	}

	set_windows(win.y);
	Upd_bindings();

	for (;;) {
		if (first) {
			name= ask_name(I_OP_PROMPT);
			scrolloffheading();
			first= No;
		}
		else {
			name= ask_name(OP_PROMPT);
		}
		if (Empty(name)) {
			clearwindow();
			continue;
		}
		if (Equal(name, "?")) {
			help();
			continue;
		}
		show= *name == '=';
		del= *name == '-';
		if (show || del) name++;

		if (is_quit(name)) {
			if (!del)
				putkeydefs();
			break;
		}
		else if (is_init(name)) {
			sendinistring();
			redrawscreen();
			continue;
		}

		sprintf(fmtbuf, "[%s]", name);
		op= findoperation(fmtbuf);

		if (op == ONULL || !op->allowed) {
			putdata(E_UNKNOWN, Yes);
			continue;
		}

#ifndef CANLOOKAHEAD
		if (!show && op->code == CANCEL) {
			sprintf(fmtbuf, E_NOTALLOWED, name);
			putdata(fmtbuf, Yes);
			continue;
		}
#endif

		if (show)
			showbindings(op);
		else if (del)
			delbindings(op);
		else if (op->code == MOUSE)
			definemouse();
		else
			definebinding(op);
	}
}

Hidden bool is_quit(name) string name; {
	if (Equal(name, "q") || Equal(name, "quit"))
		return Yes;
	return No;
}

Hidden bool is_init(name) string name; {
	if (Equal(name, "init"))
		return Yes;
	return No;
}

Hidden Procedure scrolloffheading() {
	int y, x;

	if (headingonscreen == No)
		return;

	y = win.y; x = win.x;		/* save old values */
	trmscrollup(0, win.ylast, nheadlines);
	set_windows(0);
	win.y = y - nheadlines;
	win.x = x;
	headingonscreen = No;
	nheadlines = 0;
}

/****************************************************************************/

Hidden Procedure definebinding(op) operation *op; {
	string def, rep;
	int deflen;

	clearwindow();
	def= new_definition(op, &deflen, &rep);
	if (deflen == 0)
		return;

#ifndef KNOWN_KEYBOARD
	rep= new_representation(rep, deflen, def);
#else
	if (op->code == TERMINIT || op->code == TERMDONE)
		rep= new_representation(rep, deflen, def);
#endif

	store(op->code, op->name, deflen, def, rep);
	Upd_bindings();
}

#define SHOW_PROMPT "Showing the bindings for %s (%s):"

Hidden Procedure showbindings(op) operation *op; {
	tabent *d;

	clearwindow();
	sprintf(fmtbuf, SHOW_PROMPT, op->name, op->descr);
	putdata(fmtbuf, Yes);

	for (d= deftab+ndefs-1; d >= deftab; d--) {
		if (d->code != op->code || !ValDef(d) || !ValRep(d))
			continue;
		putdata(d->rep, Yes);
	}
}

/****************************************************************************/

#define DelYes     1     /* delete this entry; goon asking */
#define DelNo      2     /* don't delete this entry; goon asking */
#define DelAll     4     /* delete this entry and all entries left; stop asking */
#define DelCancel  8     /* don't delete this entry; stop asking */
#define DelHelp   16     /* explain the options above */

Hidden bool outofsync;   /* Yes, if the 'prompt' isn't in the right place anymore */

Hidden Procedure delbindings(op) operation *op; {
	tabent *d;
	int flags = 0;

	clearwindow();
	outofsync = No;

	for (d = deftab+ndefs-1; d >= deftab; d--) {
		if (d->code == op->code && (ValDef(d) || ValRep(d))) {
			if (flags & DelAll) {
				delentry(d);
				continue;
			}
			while ((flags = asktodelete(d)) & DelHelp)
				delhelp();
			if (flags & (DelYes | DelAll)) {
				delentry(d);
			}
			else if (flags & DelCancel) {
				break;
			}
		}
	}
	clearwindow();
}

Hidden int asktodelete(d) tabent *d; {
	string rep;
	int ans;
	int c;
	int y, x;

	rep = ValRep(d) ? d->rep : makereprofdef(d->deflen, d->def);
	delprompt(rep, d->name, &y, &x);
	trmsync(y, x);
	for (;;) {
		c= trminput();
		if (c == 'y') 	   { ans = DelYes; break; }
		else if (c == 'n') { ans = DelNo; break; }
		else if (c == 'a') { ans = DelAll; break; }
		else if (c == 'c') { ans = DelCancel; break; }
		else if (c == '?') { ans = DelHelp; break; }
		else trmbell();
	}
	return ans;
}

Hidden Procedure delentry(d) tabent *d; {
	if (d->code < 0) {         /* string entry */
		d->def = d->rep = "";
		d->deflen = 0;
	}
	else {
		store(DELBIND, S_IGNORE, d->deflen, d->def, d->rep);
		d->def = d->rep = SNULL;
		d->deflen = 0;
	}
	bind_changed(d->code);
	Upd_bindings();
}

Hidden string makereprofdef(deflen, def) int deflen; string def; {
	int i;
	bufadm *rp= &reprbuf;

	bufreinit(rp);
	for (i = 0; i<deflen; i++) {
		if (i > 0) bufpush(rp, ' ');
		bufcpy(rp, reprchar(def[i]));
	}
	return getbuf(rp);
}

#define DEL_QUESTION "Do you want to remove this binding for %s ?"
#define DEL_CHOICE "[y/n/a/c/?] " 
#define DEL_HELP "(Type ? for help) "
Hidden bool helpmessage = Yes;          /* write previous message only once */

Hidden Procedure delprompt(rep, name, y, x) string rep, name; int *y, *x; {
	if (outofsync) {
		clearwindow();
		outofsync = No;
	}
	win.y = win.yfirst;
	win.x = 0;
	putdata(rep, Yes);
	sprintf(fmtbuf, DEL_QUESTION, name);
	putdata(fmtbuf, Yes);
	putdata(DEL_CHOICE, Yes);
	if (helpmessage == Yes) {
		putdata(DEL_HELP, No);
		helpmessage = No;
	}
	if (strlen(rep) > win.width)
		outofsync = Yes;
	*y = win.y;
	*x = win.x;
}

Hidden Procedure delhelp() {
	clearwindow();
	putdata("y (yes)   : remove this binding", Yes);
	putdata("n (no)    : do not remove this binding", Yes);
	putdata("a (all)   : remove this binding, and all the non-asked ones", Yes);
	putdata("c (cancel): do not remove this binding, and stop asking", Yes);
	asktocontinue();
	outofsync = Yes;
}

/****************************************************************************/

Hidden Procedure definemouse() {
	bool r;

	r= defmouse();
	redrawscreen();
	if (!r) nomouse();
}

Forward Hidden string ask_to_click();
Hidden string fmt;
Hidden string pfmt;
Hidden int headlen= 0;

Hidden Procedure startfmt(syz) int syz; {
	fmt= (string) getmem((unsigned) syz);
	pfmt= fmt;
}

Hidden Procedure chr2fmt(ch) char ch; {
	*pfmt++= ch;
}

Hidden Procedure str2fmt(s) string s; {
	while (*s) {
		*pfmt++= *s;
		s++;
	}
}

Hidden Procedure endfmt() {
	freemem(fmt);
	fmt= NULL;
}

Hidden bool defmouse() {
	int len, lenx, leny;
	char *p, *px, *py;
	bool wanty;
	string rep;
	string dfltrep;

	p= ask_to_click(0, 0, "in the upper left", &len);
	px= ask_to_click(0, 19, "right from the upper left", &lenx);
	py= ask_to_click(19, 0, "down from the upper left", &leny);

	startfmt(2*len); /* that seems a save size */

	wanty= 1;
	while (len-- > 0) {
		if (*p == *px && *p == *py) {
			chr2fmt(*p);
			p++; px++; py++;
		}
		else if (*p == *px) {	/* on y parameter */
			if (headlen == 0)
				headlen= pfmt-fmt;
			if (!toformat(&p, &py, &px, len==leny))
				return No;
			wanty= 1-wanty;
		}
		else if (*p == *py) {	/* on x parameter */
			if (headlen == 0)
				headlen= pfmt-fmt;
			if (wanty)
				str2fmt("%r");
			if (!toformat(&p, &px, &py, len==lenx))
				return No;;
			wanty= 1-wanty;
		}
		else {
			return No;
		}
	} /* end while len > 0 */

	prcnt2toprcnt3();

	win.y= 21; win.x= 0;
	dfltrep= findrepr(headlen, fmt);
	if (dfltrep == SNULL)
	        dfltrep= "mouse-click";
	rep= new_representation(dfltrep, headlen, fmt);
	store(MOUSE, S_MOUSE, headlen, fmt, rep);
	store(MFORMAT, S_MFORMAT, pfmt-fmt-headlen, fmt+headlen, "mouse-format");
	Upd_bindings();

	endfmt();

	return Yes;
}

Hidden bool toformat(pp, ppf, ppo, diflen) char **pp, **ppf, **ppo; bool diflen;
{
	if (**pp == '0' && **ppf == '1' && *(*ppf+1) == '9') {
		if (diflen) {
			str2fmt("%d");
			(*pp)++; (*ppf)+=2; (*ppo)++;
		}
		else if (*(*pp+1) == '0') {
			str2fmt("%2");
			(*pp)+=2; (*ppf)+=2; (*ppo)+=2;
		}
		else
			return No;
	}
	else if (**pp == '0' && **ppf == '2' && *(*ppf+1) == '0') {
		if (diflen) {
			str2fmt("%i%d");
			(*pp)++; (*ppf)+=2; (*ppo)++;
		}
		else if (*(*pp+1) == '1') {
			str2fmt("%i%2");
			(*pp)+=2; (*ppf)+=2; (*ppo)+=2;
		}
		else
			return No;
	}
	else if ((**pp)+19 == **ppf) {
		str2fmt("%+"); chr2fmt(**pp);
		(*pp)++; (*ppf)++; (*ppo)++;
	}
	else if ((**pp)-19 == **ppf) {
		str2fmt("%-"); chr2fmt(**pp);
		(*pp)++; (*ppf)++; (*ppo)++;
	}
	else
		return No;
	return Yes;
}

Hidden Procedure prcnt2toprcnt3() {
	/* change each occurrence of "0%2" into "0%3",
	 * and of "0%r%2" into "%r%3".
	 */
	register int i, j, len;

	len= pfmt-fmt;
	for (i=0; i<len; i++) {
		if (fmt[i] == '0' && fmt[i+1] == '%' && fmt[i+2] == '2') {
			fmt[i+2]= '3';
			--len; --pfmt;
			for (j=i; j<len; j++)
				fmt[j]= fmt[j+1];
			if (headlen == i+1) headlen--;
		}
		else if (fmt[i] == '0' && fmt[i+1] == '%' && fmt[i+2] == 'r'
			 && fmt[i+3] == '%' && fmt[i+4] == '2') {
			fmt[i+4]= '3';
			--len; --pfmt;
			for (j=i; j<len; j++)
				fmt[j]= fmt[j+1];
			if (headlen == i+1) headlen--;
		}
	}
}

Hidden char inverse[1]= {STANDOUT};

Hidden char buf[MAXBUFFER];

Hidden string ask_to_click(y, x, prompt, plen) int y, x; string prompt; int *plen; {
	char *pbuf= buf;
	int c;

	clearscreen();
	trmputdata(y, y, x, "O", inverse);
	trmputdata(19, 19, 19, "Please click the mouse at the O ", (string)0);
	trmputdata(20, 20, 19, prompt, (string)0);
	if (trmavail() < 0) {
		trmputdata(21, 21, 19, "After that, press the [RETURN] key", (string)0);
	}
	trmsync(y, x);
	c= trminput();
	while (trmavail() > 0 || (trmavail() < 0 && !CRLF(c))) {
		*pbuf++= c;
		c= trminput();
	}
	if (trmavail() >= 0)
		*pbuf++= c;
	*pbuf= '\0';
	*plen= pbuf - buf;
	return savedef(buf, *plen);
}

Hidden Procedure nomouse() {
	putdata("*** Sorry, I couldn't decode the mouse clicks", Yes);
}

/****************************************************************************/

Hidden tabent savedeftab[MAXDEFS];
Hidden int nsaveharddefs= 0;
Hidden int nsavefiledefs= 0;


Visible Procedure saveharddefs() {
	tabent *d, *h;
	
	for (d= deftab, h= savedeftab; d < deftab+nharddefs; d++) {
		if (d->code < 0 || (Val(d->name) && ValDef(d))) {
			h->code= d->code;
			h->name= d->name;
			h->deflen= d->deflen;
			h->def= d->def;
			h->rep= d->rep;
			h++;
		}
	}
	nsaveharddefs= h-savedeftab;
}

Visible Procedure savefiledefs() {
	tabent *d, *h;
	
	d= deftab + nharddefs;
	h= savedeftab + nsaveharddefs;
	for (; d < deftab + ndefs; d++) {
		if (Val(d->name) && ValDef(d)) {
			h->code= d->code;
			h->name= d->name;
			h->deflen= d->deflen;
			h->def= d->def;
			h->rep= d->rep;
			h++;
		}
	}
	nsavefiledefs= h - (savedeftab + nsaveharddefs);
}

Hidden bool a_harddef(d) tabent *d; {
	tabent *h;

	if (!ValDef(d))
		return No;
	for (h= savedeftab; h < savedeftab+nsaveharddefs; h++) {
		if (equal(d->deflen, d->def, h->deflen, h->def) && 
			Equal(d->rep, h->rep) &&	/* TODO: needed ? */
			(d->code == h->code ||
			 d->code == IGNORE ||
			 d->code == DELBIND
			)
		   )
			return Yes;
	}
	return No;
}

Hidden Procedure init_ignore() {
	tabent *d;
	
	for (d= deftab+nharddefs; d < deftab+ndefs; d++) {
		if (d->code == IGNORE && a_harddef(d))
			/* don't show it in the bindings window */
			d->code= DELBIND;
	}
}

Hidden string findoldrepr(deflen, def) int deflen; string def; {
	tabent *h;

	h= savedeftab + (nsaveharddefs + nsavefiledefs) - 1;
	for (; h >= savedeftab; h--) {
		if (ValDef(h)
		    && equal(h->deflen, h->def, deflen, def)
		    && ValRep(h))
			return h->rep;
	}
	return SNULL;
}

Hidden tabent *findsavedhardstringentry(code) int code;  {
	tabent *d;

	for (d= savedeftab+nsaveharddefs-1; d >= savedeftab; d--) {
		if (code == d->code)
			return d;
	}
	return DNULL;
}

/****************************************************************************/

Hidden FILE *key_fp;			/* fileptr for key definitions file */

Hidden Procedure putkeydefs() {
	openkeyfile();
	put_table();
	put_strings();
	closekeyfile();
}

Hidden Procedure init_newfile()
{
	char *termname;
	string termfile;
	
	if ((termname= getenv("TERM")) != NULL) {
		termfile= (string) getmem((unsigned) strlen(FORMAT_KEYSFILE) +
					  strlen(termname));
		sprintf(termfile, FORMAT_KEYSFILE, termname);
	}
	else termfile= savestr(NEWKEYSFILE);
	
	if (bwsdefault
	    && (D_exists(bwsdefault) || Mkdir(bwsdefault) == 0)
	    && D_writable(bwsdefault))
	{
		newfile= makepath(bwsdefault, termfile);
	}
	else {
		sprintf(fmtbuf,
	"Cannot use directory \"%s\" for private keydefinitions file\n",
			bwsdefault);
		putserr(fmtbuf);
		newfile= termfile;
	}
}

#define MAKE_KEYFILE "Producing key definitions file %s."

Hidden Procedure openkeyfile() {
	key_fp= fopen(newfile, "w");
	if (key_fp == NULL) {
		sprintf(fmtbuf, E_KEYFILE, newfile);
		putdata(fmtbuf, Yes);
		key_fp= stdout;
	}
	else {
		sprintf(fmtbuf, MAKE_KEYFILE, newfile);
		putdata(fmtbuf, Yes);
	}
	nextline();
	freemem(newfile);
}

Hidden Procedure closekeyfile() {
	fclose(key_fp);
}

Hidden Procedure put_table() {
	tabent *d;
	
	for (d= deftab+nharddefs; d < deftab+ndefs; d++) {
		if (ValDef(d)) {
			if (d->code != IGNORE) {
				if (d->code == DELBIND) {
					if (!a_harddef(d))
						continue;
				}
				else if (a_harddef(d))
					continue;
			}
			put_def(d->name, d->deflen, d->def, d->rep);
		}
	}
}

Hidden Procedure put_strings() {
	int i;
	tabent *old, *new;
	string oldrep, newrep;

	for (i= 0; i < sizeof(stringcode)/sizeof(int); i++) {
		old= findsavedhardstringentry(stringcode[i]);
		oldrep= ValRep(old) ? old->rep : "";
		new= findstringentry(stringcode[i]);
		newrep= ValRep(new) ? new->rep : "";
		if (equdef(old->deflen, old->def, new->deflen, new->def) == No
		    || strcmp(oldrep, newrep) != 0) {
			if (ValDef(new)) 
				put_def(new->name, new->deflen, new->def, newrep);
			else
				put_def(new->name, 0, "", "");
		}
	}
}

#define NAMESPACE 15 /* TODO: e1getc.c accepts until 20 */

Hidden Procedure put_def(name, deflen, def, rep)
	string name, def, rep; int deflen;
{
	int i;
	string s;

	i= 0;
	for (s= name; *s; s++) {
		putc(*s, key_fp);
		i++;
	}
	while (i < NAMESPACE) {
		putc(' ', key_fp);
		i++;
	}
	fputs(" = ", key_fp);
	putc('"', key_fp);
	s= def;
	for (i= 0; i < deflen; ++i) {
		if (*s == '"')
			putc('\\', key_fp);
		if (Printable(*s))
			putc(*s, key_fp);
		else
			fprintf(key_fp, "\\%03o", (int) (*s&0377));
		s++;
	}
	putc('"', key_fp);
	fprintf(key_fp, " = \"%s\"\n", rep);
/* HACK: */
	if (key_fp == stdout && outfile == CONSOLE) {
		putc('\r', key_fp);
	}
}

/****************************************************************************/

#define HELP_PROMPT	"Press [SPACE] to continue, [RETURN] to exit help" 

Hidden Procedure help() {
	clearwindow();
	shorthelp();
	if (morehelp()) {
		clearwindow();
		longhelp();
	}
	else
		clearwindow();
}

Hidden Procedure shorthelp() {
	message(" name: (re)define binding for \"name\",", Yes);
	message("-name: remove a binding for \"name\"", Yes);
	message("=name: show all the bindings for \"name\"", Yes);
	message(" quit: exit this program, saving the changes", Yes);
	message("-quit: exit this program", Yes);
	message(" init: send term-init string to screen", Yes);
}

Hidden bool morehelp() {
	int c;
	int y= win.y+1;
	string prompt= HELP_PROMPT;
	bool ans;

	if (y < win.ylast)
		y++;
	trmputdata(y, y, 0, prompt, mkstandout(prompt));
	trmsync(y, strlen(prompt));

	for (;;) {
		c= trminput();
		if (c == EOF || CRLF(c))
			{ ans= No; break; }
		else if (Cspace(c))
			{ ans= Yes; break; }
		else
			trmbell();
	}
	trmputdata(y, y, 0, "", (string)0);
	return ans;
}

Hidden Procedure longhelp() {

message("    While (re)defining a binding, the program will ask you to enter \
a key sequence; end it with [RETURN]. ", Yes);

message("If you want [RETURN] in your binding, enclose the whole binding \
with single or double quotes. ", No);

#ifndef KNOWN_KEYBOARD

message("It will then ask you how to represent this key in the bindings \
window; the default can be accepted with [RETURN].", No);

#endif /* KNOWN_KEYBOARD */

message("    [term-init] and [term-done] are the names for the strings that \
should be sent to the screen upon startup and exit, respectively (for \
programming function keys or setting background colours etc).", Yes);

#ifndef CANLOOKAHEAD

if (intrchar != '\0') {

sprintf(fmtbuf,
"    This program will not allow you to use your interrupt character (%s) in \
any keybinding, since the ABC system always binds this to %s. ",
	reprchar(intrchar), S_INTERRUPT);
message(fmtbuf, Yes);

message("You can use this idiosyncrasy to cancel a binding while typing \
by including your interrupt character.", No);

}

#endif /* !CANLOOKAHEAD */

message("    The space in the window above sometimes isn't sufficient to \
show all the bindings. You will recognize this situation by a marker \
('*') after the name. Hence the option '=name'.", Yes);

}
