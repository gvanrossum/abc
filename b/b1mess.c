/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1986. */

/* B error message handling */

/* All error messages are collected in a file, both to save data space
   and to ease translation to other languages.	The English version
   of the database can be recreated from the program sources by scanning
   for the pattern "MESS".  This is a macro whose first argument is
   the message number and whose second number is the message string;
   this macro expands to only the message number which is passed to
   the error routines.	The error routines then dig the message from
   the error message file, or just print the number if the file can't be
   opened.  There is also a way to pass a message that is determined
   at runtime.
*/

#include "b.h"
#include "bmem.h"
#include "bobj.h"
#include "port.h"

/* While we are reading the Messages file, we build an index.
   probe[k] contains the first message number found in block k.
   blocks are BUFSIZ in size. */

#define FILESIZE 22454 /* Approximated current size of Messages file */
#define MAXPROBE (10 + FILESIZE/BUFSIZ) /* Allow some growth */

Hidden short probe[MAXPROBE];
Hidden int nprobes= 1;

#define NOT_OPENED ((FILE*)(-1))
#define NO_MESSFILE "*** Cannot find or read messages file; using numbers\n"
Hidden FILE *messfp= NOT_OPENED;

char *messbuf; /* used for messages with arguments */
Hidden char buf[MESSBUFSIZE];

Visible string getmess(nr) int nr;  {
	int last, c; char *cp= NULL;
	bool new; int block; long ftell();
	static int last_nr= 0;

	if (nr <= 0) 
		return nr == -1 ? "%s" : nr == -2 ? "%s%s" : "";
	if (messfp == NOT_OPENED) {
		if (messfile)
			messfp= fopen(messfile, "r");
		else
			messfp= NULL;
		if (messfp == NULL) {
			flushout();
			putserr(NO_MESSFILE);
			flusherr();
		}
	}
	if (nr == last_nr) {
		cp= strchr(buf, '\t');
		if (cp != NULL)
		    return cp+1;
	}
	if (messfp) {
		for (block= nprobes-1; block > 0; --block) {
			if (probe[block] <= nr)
				break;
		}
		new= block == nprobes-1;
		fseek(messfp, (long)block*BUFSIZ, 0);
		last= 0;
		while (last < nr) {
			if (new) block= ftell(messfp) / BUFSIZ;
			cp= buf;
			while ((c= getc(messfp)) != EOF && c != '\n') {
				if (cp >= buf + MESSBUFSIZE - 2) break;
				if (c != '\\')
					*cp= c;
				else {
					c= getc(messfp);
					if (c == EOF || c == '\n') break;
					switch (c) {
					case 'n': *cp= '\n'; break;
					case 'r': *cp= '\r'; break;
					case 't': *cp= '\t'; break;
					case 'b': *cp= '\b'; break;
					default: *cp++= '\\'; *cp= c; break;
					}
				}
				cp++;
			}
			*cp= '\0';
			if (c == EOF) break;
			last= atoi(buf);
			if (last <= 0)
				continue;
			if (new && block >= nprobes && nprobes < MAXPROBE) {
				probe[block]= last;
				nprobes= block+1;
			}
		}
		if (last == nr) {
			cp= strchr(buf, '\t');
			if (cp != NULL) {
				last_nr= nr;
				return cp+1;
			}
		}
	}
	sprintf(buf, " (message %d) ", nr);
	last_nr= 0;
	return buf;
}

Visible Procedure initmess() {
	messbuf= (char*) getmem(MESSBUFSIZE);
}

Visible Procedure endmess() {
#ifdef MEMTRACE
	freemem((ptr) messbuf);
#endif
}

/***************************************************************************/

Visible Procedure putmess(m)
     int m;
{
	putserr(getmess(m));
	flusherr();
}

#ifndef KEYS

Visible Procedure putSmess(m, s)
     int m;
     string s;
{
	putsSerr(getmess(m), s);
	flusherr();
}

Visible Procedure putDSmess(m, d, s)
     int m;
     int d;
     string s;
{
	putsDSerr(getmess(m), d, s);
	flusherr();
}

Visible Procedure put2Cmess(m, c1, c2)
     int m;
     char c1;
     char c2;
{
	puts2Cerr(getmess(m), c1, c2);
	flusherr();
}

#endif /* !KEYS */
