/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1986. */

/* B editor -- read key definitions from file */

#include "b.h"
#include "bmem.h"
#include "bobj.h"
#include "oper.h"
#include "getc.h"
#include "port.h"

extern bool use_bed;

#define ESC '\033'

Forward int maxdeflen;
Forward Hidden Procedure initsense();
Forward Hidden Procedure initmouse();
Forward Hidden bool equalhead();

/*
This file contains a little parser for key definition files.
To allow sufficient freedom in preparing such a file, a simple
grammar has been defined according to which the file is parsed.
The parsing process is extremely simple, as it can be done
top-down using recursive descent.


Lexical conventions:

- Blanks between lexical symbols are ignored.
- From '#' to end of line is comment (except inside strings).
- Strings are delimited by double quotes and
  use the same escape sequences as C strings, plus:
  \e or \E means an ESCape ('\033').
- Commandnames are like C identifiers ([a-zA-Z_][a-zA-Z0-9_]*).
  Upper/lower case distinction is significant.
- Key representations are delimited by double quotes, and may use
  any printable characters.

Syntax in modified BNF ([] mean 0 or 1, * means 0 or more, + means 1 or more):

   file: line*
   line: [def] [comment]
   def: '[' commandname ']' '=' definition  '=' representation
   definition: string


Notes:

- A definition for command "[term-init]" defines a string to be sent
  TO the terminal at initialization time, e.g. to set programmable
  function key definitions.  Similar for "[term-done]" on exiting.
- Command names are conventional editor operations.
- Some bindings are taken from tty-settings, and should not be changed.
  (interrupt and suspend).
*/

#define COMMENT '#' /* Not B-like but very UNIX-like */
#define QUOTE '"'

Hidden FILE *keysfp; /* File from which to read */
Hidden char nextc; /* Next character to be analyzed */
Hidden bool eof; /* EOF seen? */
Hidden int lcount; /* Current line number */
#ifndef KEYS
Hidden int errcount= 0; /* Number of errors detected */
#else
Visible int errcount= 0; /* Number of errors detected */
#endif

Visible int ndefs;

Hidden Procedure err1(m)
	string m;
{
	static char errbuf[MESSBUFSIZE];
		/* since putmess() below overwrites argument m via getmess() */

	if (keysfp != NULL)
		sprintf(errbuf, "%s (%d): %s\n", keysfile, lcount, m);
	else
		sprintf(errbuf, "%s\n", m);

	if (errcount == 0) {
		putmess(MESS(6500, "Errors in key definitions:\n"));
	}
	++errcount;

	putserr(errbuf);
}

Hidden Procedure err(m)
	int m;
{
	err1(getmess(m));
}

Hidden Procedure adv()
{
	int c;

	if (eof)
		return;
	c= getc(keysfp);
	if (c == EOF) {
		nextc= '\n';
		eof= Yes;
	}
	else {
		nextc= c;
	}
}

Hidden Procedure skipspace()
{
	while (nextc == ' ' || nextc == '\t')
		adv();
}

Hidden int lookup(name)
	string name;
{
	int i;

	for (i= 0; i < ndefs; ++i) {
		if (deftab[i].name != NULL && strcmp(name, deftab[i].name) == 0)
			return i;
	}
	return -1;
}

/*
 * Undefine conflicting definitions, i.e. strip them from other commands.
 * Conflicts arise when a command definition is
 * an initial subsequence of another, or vice versa.
 * String definitions (code < 0) are not undefined.
 */
Visible Procedure undefine(code, deflen, def)
	int code;
	int deflen;
	string def;
{
	struct tabent *d, *last= deftab+ndefs;
	string p, q;
	int i;

	if (code < 0) 
		return;
	for (d= deftab; d < last; ++d) {
		if (d->code > 0 && d->deflen > 0 && d->def != NULL) {
			p= def;
			q= d->def;
			for (i=0; *p == *q; ++i, ++p, ++q) {
				if (i == deflen || i == d->deflen) break;
			}
			if (i == deflen || i == d->deflen) {
				d->deflen= 0;
				d->def= NULL;
				d->rep= NULL;
#ifdef KEYS
				bind_changed(d->code);
#endif
			}
		}
	}
}

#ifndef CANLOOKAHEAD

#define ERR1 MESS(6501, "Cannot verify the bindings for the interrupt character")

Hidden bool isIntrBinding(code, deflen, def)
	int code;
	int deflen;
	string def;
{
	return code == CANCEL && intrchar != '\0' &&
	       deflen == 1 && def[0] == intrchar;
}

Hidden Procedure sigundef_intrchar() {
	struct tabent *d, *last= deftab+ndefs;
	string def;
	int deflen;
	int i;

	if (!intrchar) {
		err(ERR1);
		return;
	}
	for (d= deftab; d < last; ++d) {
		if (d->code > 0 && d->deflen > 0 && d->def != NULL) {
			def= d->def;
			deflen= d->deflen;
			if (isIntrBinding(d->code, deflen, def))
				continue;
			for (i= 0; i<deflen; i++) {
				if (def[i] == intrchar) {
					d->deflen= 0;
					d->def= 0;
					d->rep= 0;
					break;
				}
			}
		}
	}
}

#endif /* !CANLOOKAHEAD */

Hidden bool store(code, deflen, def, rep, name) /* return whether stored */
	int code;
	int deflen;
	string def;
	string rep;
	string name;
{
	struct tabent *d, *last= deftab+ndefs;
#ifndef CANLOOKAHEAD
	int i;
#endif

	if (deflen == 0 && def != NULL) {
		deflen= strlen(def);
	}
	if (code < 0) {
		/* find the place matching name to replace definition */
	        for (d= deftab; d < last; ++d) {
			if (strcmp(name, d->name) == 0)
                        	break;
		}
	}
	else {
		/* Check for illegal definition:
		   if a command definition starts with a printable character,
		   OR it contains the interrupt character that must be
		   handled as a signal, because lookahead in the system's
		   input queue isn't possible (!CANLOOKAHEAD)
	 	*/

#define ERR2 MESS(6502, "Definition for command %s is empty.")
		if (deflen == 0) {
			sprintf(messbuf, getmess(ERR2), name);
			err1(messbuf);
			return No;
		}

#define ERR3 MESS(6503, "Definition for command %s starts with '%c'.")
		if (isascii(*def) && (isprint(*def) || *def==' ')) {
			sprintf(messbuf, getmess(ERR3), name, *def);
			err1(messbuf);
			return No;
		}

#ifndef CANLOOKAHEAD

#define ERR4 MESS(6504, "Definition for command %s would produce an interrupt.")
		if (intrchar && !isIntrBinding(code, deflen, def)) {
			for (i= 0; i<deflen; i++) {
				if (def[i] == intrchar) {
					sprintf(messbuf, getmess(ERR4), name);
					err1(messbuf);
					return No;
				}
			}
		}

#endif		
		undefine(code, deflen, def);
		/* New definitions are added at the end, so the last one can be 
		   used in the HELP blurb. */
		d= last;
		/* Extend definition table */
		if (ndefs >= MAXDEFS) {
			err(MESS(6505, "Too many key definitions"));
			return No;
		}
		ndefs++;
	}
	d->code= code;
	d->name= name;
	d->deflen= deflen;
	d->def= def;
	d->rep= rep;
#ifdef MEMTRACE
	fixmem((ptr) name);
	fixmem((ptr) def);
	fixmem((ptr) rep);
#endif
	return Yes;
}

Visible Procedure addkeydef(code, deflen, def, rep, name)
	int code;
	int deflen;
	string def;
	string rep;
	string name;
{
	/* called from porting directory */
	VOID store(code, deflen, def, rep, name);
}

Hidden string getname()
{
	char buffer[20];
	string bp;
	
	if (nextc != '[') {
		err(MESS(6506, "no '[' before name"));
		return NULL;
	}
	bp= buffer;
	*bp++= nextc;
	adv();
	if (!isascii(nextc)
	    ||
	    (!isalpha(nextc) && nextc != '_' && nextc != '-')
	   ) {
		err(MESS(6507, "No name after '['"));
		return NULL;
	}
	while ((isascii(nextc) && isalnum(nextc))
	       || nextc == '_' || nextc == '-'
	      ) {
		if (bp < buffer + sizeof buffer - 1)
			*bp++= (nextc == '_' ? '-' : nextc);
		adv();
	}
	if (nextc != ']') {
		err(MESS(6508, "no ']' after name"));
		return NULL;
	}
	*bp++= nextc;
	adv();
	*bp= '\0';
	return (string) savestr(buffer);
}

Hidden int getstring(pstr) string *pstr;
{
	char buf[256]; /* Arbitrary limit */
	char c;
	int len= 0;
	int i;

	if (nextc != QUOTE) {
		err(MESS(6509, "opening string quote not found"));
		return -1;
	}
	adv();
	while (nextc != QUOTE) {
		if (nextc == '\n') {
			err(MESS(6510, "closing string quote not found in definition"));
			return -1;
		}
		if (nextc != '\\') {
			c= nextc;
			adv();
		}
		else {
			adv();
			switch (nextc) {

			case 'r': c= '\r'; adv(); break;
			case 'n': c= '\n'; adv(); break;
			case 'b': c= '\b'; adv(); break;
			case 't': c= '\t'; adv(); break;
			case 'f': c= '\f'; adv(); break;

			case 'E':
			case 'e': c= ESC; adv(); break;

			case '0': case '1': case '2': case '3':
			case '4': case '5': case '6': case '7':
				c= nextc-'0';
				adv();
				if (nextc >= '0' && nextc < '8') {
					c= 8*c + nextc-'0';
					adv();
					if (nextc >= '0' && nextc < '8') {
						c= 8*c + nextc-'0';
						adv();
					}
				}
				break;

			default: c=nextc; adv(); break;

			}
		}
		if (len >= sizeof buf) {
			err(MESS(6511, "definition string too long"));
			return -1;
		}
		buf[len++]= c;
	}
	adv();
	buf[len]= '\0';
	/* buf can contain null characters, so be careful ! */
	*pstr= (string) getmem((unsigned) (len+1));
	for (i= 0; i<len; i++)
		(*pstr)[i]= buf[i];
	(*pstr)[len]= '\0';

	return len;
}

Hidden string getrep()
{
	char buf[256]; /* Arbitrary limit */
	char c;
	int len= 0;

	if (nextc != QUOTE) {
		err(MESS(6512, "opening string quote not found in representation"));
		return NULL;
	}
	adv();
	while (nextc != QUOTE) {
		if (nextc == '\\')
			adv();
		if (nextc == '\n') {
			err(MESS(6513, "closing string quote not found in representation"));
			return NULL;
		}
		c= nextc;
		adv();
		if (!isprint(c) && c != ' ') {
			err(MESS(6514, "unprintable character in representation"));
			return NULL;
		}
		if (len >= sizeof buf) {
			err(MESS(6515, "representation string too long"));
			return NULL;
		}
		buf[len++]= c;
	}
	adv();
	buf[len]= '\0';
	return savestr(buf);
}

Hidden Procedure get_definition()
{
	string name;
	int d;
	int code;
	int deflen;
	string def;
	string rep;
	
	name= getname();
	if (name == NULL)
		return;
	skipspace();

#define ERR5 MESS(6516, "Name %s not followed by '='")
	if (nextc != '=') {
		sprintf(messbuf, getmess(ERR5), name);
		err1(messbuf);
		freemem((ptr) name);
		return;
	}
	d = lookup(name);

#define ERR6 MESS(6517, "Unknown command name: %s")
	if (d < 0) {
		sprintf(messbuf, getmess(ERR6), name);
		err1(messbuf);
		freemem((ptr) name);
		return;
	}
	code = deftab[d].code;

	adv();
	skipspace();
	deflen= getstring(&def);
	if (deflen < 0) {
		freemem((ptr) name);
		return;
	}
	
	skipspace();

#ifndef CANLOOKAHEAD

#define ERR7 MESS(6518, "Rebinding %s in keysfile not allowed")
	if (code == CANCEL && !isIntrBinding(code, deflen, def)) {
		sprintf(messbuf, getmess(ERR7), name);
		err1(messbuf);
		freemem((ptr) name);
		return;
	}

#endif /* !CANLOOKAHEAD */

#define ERR8 MESS(6519, "No '=' after definition for name %s")
	if (nextc != '=') {
		sprintf(messbuf, getmess(ERR8), name);
		err1(messbuf);
		freemem((ptr) name);
		freemem((ptr) def);
		return;
	}

	adv();
	skipspace();
	rep= getrep();
	if (rep == NULL) {
		freemem((ptr) name);
		freemem((ptr) def);
		return;
	}
	
	if (!store(code, deflen, def, rep, name)) {
		freemem((ptr) def);
		freemem((ptr) rep);
		freemem((ptr) name);
	}
}

Hidden Procedure get_line()
{
	adv();
	skipspace();
	if (nextc != COMMENT && nextc != '\n')
		get_definition();
	while (nextc != '\n')
		adv();
}

#ifdef KEYTRACE

extern FILE *keyfp;

Visible Procedure dumpkeys(where)
	string where;
{
	int i;
	int w;
	string s;

	fprintf(keyfp, "\nDump of key definitions %s.\n\n", where);
	fputs("Code    Name            Definition               Representation\n", keyfp);
	for (i= 0; i < ndefs; ++i) {
		fprintf(keyfp, "%4d    ", deftab[i].code);
		if (deftab[i].name != NULL)
			fprintf(keyfp, "%-15s ", deftab[i].name);
		else
			fputs("                ", keyfp);
		fprintf(keyfp, "%4d:  ", deftab[i].deflen);
		s= deftab[i].def;
		w= 0;
		if (s != NULL) {
			for (; *s != '\0'; ++s) {
				if (isascii(*s) && (isprint(*s) || *s == ' ')) {
					fputc(*s, keyfp);
					w++;
				}
				else {
					fprintf(keyfp, "\\%03o", (int)(*s&0377));
					w+= 4;
				}
			}
		}
		else {
			fputs("NULL", keyfp);
			w= 4;
		}
		while (w++ < 25)
		        fputc(' ', keyfp);
		s= deftab[i].rep;
		fprintf(keyfp, "%s\n", s!=NULL ? s : "NULL");
	}
	fputc('\n', keyfp);
	VOID fflush(keyfp);
}
#endif /* KEYTRACE */

#ifdef KEYS
extern int nharddefs;
#endif

Hidden Procedure setdeflen()  /* and count the defs */
{
	struct tabent *d;

	d= deftab;
	while (d->name != NULL) {
		if (d->deflen == 0 && d->def != NULL)
		        d->deflen= strlen(d->def);
		++d;
		if (d >= deftab+MAXDEFS)
			syserr(MESS(6520, "too many predefined keys"));
	}
	ndefs= d-deftab;
#ifdef KEYS
	nharddefs= ndefs;
#endif
}

Hidden Procedure readkeysfile()
{
#ifdef KEYS
	saveharddefs();
#endif
	if (keysfile != NULL)
		keysfp= fopen(keysfile, "r");
	else
		keysfp= NULL;
	if (keysfp == NULL) {
		return;
	}
/* process: */
	lcount= 1;
	eof= No;
	do {
		get_line();
		lcount++;
	} while (!eof);
/* */
	fclose(keysfp);
#ifdef KEYTRACE
	if (keyfp)
		dumpkeys("after reading keysfile");
#endif
#ifdef KEYS
	savefiledefs();
#endif
}

Visible Procedure initkeys()
{
	setdeflen();   /* and count the defs */
	errcount= 0;
	addspeckeys(); /* in port directories */
#ifndef CANLOOKAHEAD
	sigundef_intrchar();
#endif
	readkeysfile();
	if (errcount > 0)
		flusherr();
#ifndef KEYS
	initsense();
	initmouse();
#endif
}

#ifndef KEYS

extern bool cansense;
extern string gotosense;
extern string gotoformat;
extern string mousesense;
extern string mouseformat;

Hidden string defstring(name) string name; {
	int i;

	i= lookup(name);
	if (i >= 0 && deftab[i].def != NULL && deftab[i].deflen != 0) {
		return deftab[i].def;
	}
	return NULL;
}

Hidden Procedure initsense() {
	gotosense= defstring(S_GSENSE);
	gotoformat= defstring(S_GFORMAT);
	if (gotosense != NULL && gotoformat != NULL) {
		cansense= Yes;
	}
	/* check [goto] defined, and not only one of gsense and gformat ??? */
}

Hidden Procedure initmouse() {
	mousesense= defstring(S_MSENSE);
	mouseformat= defstring(S_MFORMAT);
	/* check [mouse] defined ??? */
}

/* Output a named string to the terminal */

extern bool vtrmactive;

Hidden Procedure outstring(name)
	string name;
{
	int i= lookup(name);

	if (i >= 0) {
		string def= deftab[i].def;
		if (def != NULL && *def != '\0') {
			c_putdata(def);
			c_putnewline();
			c_flush();
		}
	}
}

/* Output the terminal's initialization sequence, if any. */

Visible Procedure initgetc()
{
	outstring(S_TERMINIT);
}


/* Output a sequence, if any, to return the terminal to a 'normal' state. */

Visible Procedure endgetc()
{
	outstring(S_TERMDONE);
}

/* translate keystrokes into editor operations
 *
 * The keystrokes typed by the user are gathered in keystroke[],
 * until it matches one of the definition in deftab.
 * (see bed/e1getc.c and ?*?/?1keys.c)
 * The corresponding editor operation is then returned.
 *
 * Once the interpreter is running, it regularly calls pollinterrupt
 * below, to check whether the user typed [interrupt] or [suspend].
 * This requires a typeahead buffer for operations to be kept.
 */

/* lookahead operation buffer */
#define MAXTYPEAHEAD 100
Hidden int *operation;
Hidden int ophead= 0;
Hidden int optail= 0;
/* lookahead keystroke buffer */
Hidden char *keystroke;
Hidden int nkeys= 0;
Visible int maxdeflen= 0;

Visible Procedure initoperations() {
	struct tabent *d;

	d= deftab;
	while (d->name != NULL && d < deftab+MAXDEFS) {
		if (d->deflen > maxdeflen)
			maxdeflen= d->deflen;
		++d;
	}
	operation= (int*) getmem((unsigned) MAXTYPEAHEAD);
	keystroke= (char*) getmem((unsigned) maxdeflen);
}

/* called from editor: */

Visible int getoperation() {
	int c;
	int oprn;
	struct tabent *d, *last= deftab+ndefs;

	if (ophead != optail) {
		/* user typed ahead while interpreter was running */
		oprn= operation[ophead];
		ophead= (ophead+1) % MAXTYPEAHEAD;
		return oprn;
	}
	/* else */
	for ( ; ; ) {
		c= trminput();
		if (c == -1)
			return EOF;
		keystroke[nkeys]= c;
		nkeys++;
		for (d= deftab; d < last; d++) {
			if (d->code > 0 && d->deflen > 0
			    && (equalhead(keystroke, nkeys, d->def, d->deflen))) {
				break;
			}
		}
		if (d == last) {
			nkeys= 0;
			if (isascii(c) && (isprint(c) || c == ' '))
				return c;
			else
				return 0377; /* rings a bell */
		}
		if (d->deflen == nkeys) {
			nkeys= 0;
			return d->code;
		}
	}
}

/* called from interpreter to enable interrupt and suspend: */

#define FAILSUSP	MESS(6521, "Sorry, I failed to suspend ABC\n")
extern bool suspendabc();

Visible int pollcnt= 0;

Visible Procedure pollinterrupt() {
	int c;
	int op;
	struct tabent *d, *last= deftab+ndefs;

	ENABLE_INTERRUPT();	/* only needed for MS DOS */
	if (intrptd) {
		/* There are two possiblities.
		 * First, keyboard-interrupt and intr character hasn't been
		 * disabled (abc called with redirected input, abc <file).
		 * Second, non-keyboard interrupt
		 */
		intrptd = No;
		int_signal();
		if (use_bed) { /* non-keyboard */
			ophead= optail= 0;
			nkeys= 0;
		}
		return;
	}
	if (!use_bed)
	        return;
	while (trmavail() > 0) {
		c= trminput();
		keystroke[nkeys]= c;
		nkeys++;
		for (d= deftab; d < last; d++) {
			if (d->code > 0 && d->def != NULL
			    && (equalhead(keystroke, nkeys, d->def, d->deflen))) {
				break;
			}
		}
		if (d == last) {
			op= (optail+1) % MAXTYPEAHEAD;
			if (op == ophead) {
				trmbell();
				return;
			}
			if (isascii(c) && (isprint(c) || c == ' '))
				operation[optail]= c;
			else
				operation[optail]= 0377; /* rings a bell */
			optail= op;
			nkeys= 0;
		}
		else if (d->deflen == nkeys) {
			switch (d->code) {
			case CANCEL:
				ophead= optail= 0;
				nkeys= 0;
				int_signal(); /* sets interrupted */
				return;
			case SUSPEND:
				nkeys= 0;
				if (!suspendabc())
					putmess(FAILSUSP);
				break;
			default:
				op= (optail+1) % MAXTYPEAHEAD;
				if (op == ophead) {
					trmbell();
					return;
				}
				operation[optail]= d->code;
				optail= op;
				nkeys= 0;
			}
		}
	}
}

Hidden bool equalhead(keystr, nkey, def, len) string keystr, def; int nkey, len; {
	int i;

	if (nkey > len)
		return No;

	for (i= 0; i < nkey; i++) {
		if (keystr[i] != def[i])
			return No;
	}
	return Yes;
}

#endif /* !KEYS */
