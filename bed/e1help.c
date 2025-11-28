/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1986. */

/*
 * B editor -- Print help blurb.
 */

#include "b.h"
#include "bedi.h"
#include "bmem.h"
#include "bobj.h"
#include "oper.h"
#include "getc.h"
#include "port.h"
#include "trm.h"

#ifdef HELPFUL

extern int winheight;
extern int llength;
extern int winstart;
/*
   The following array determines the order of the editor operations
   in the helpblurb.
   The names and keyrepresentations are taken from deftab in e1getc.c
   and ?1keys.c the first time help() is called.
   Thereafter the size is checked to determine whether printing in two
   columns is possible.
   Code NOTHING is used to produce an empty place in the second column.
 */
 
int helpcode[]= {
	WIDEN,		EXTEND,
	FIRST,		LAST,
	PREVIOUS,	NEXT,
	UPLINE,		DOWNLINE,
	UPARROW,	DOWNARROW,
	LEFTARROW,	RITEARROW,
#ifdef GOTOCURSOR
	GOTO,		MOUSE,
#endif
	ACCEPT,		NEWLINE,
	UNDO,		REDO,
	COPY,		DELETE,
	RECORD,		PLAYBACK,
	LOOK,		HELP,
	EXIT,		NOTHING,
	CANCEL,		SUSPEND
};

char *helpitem[(sizeof(helpcode))/(sizeof(int))]; /* to save "[name]  repr" */
int nitems= 0;

#define GAPWIDTH 5		/* width between the two columns */
Hidden int maxwidth= 0;		/* width of maximum helpitem */

#define MAXBUFFER 81
Hidden char buffer[MAXBUFFER];
Hidden char modebuffer[MAXBUFFER];

#define MORE MESS(6700, "Press [SPACE] for more, [RETURN] to exit help")
#define NO_MORE MESS(6701, "Press [SPACE] or [RETURN] to exit help")
#define NO_HELPFILE MESS(6702, "*** Cannot find or read help file [%s]")

Forward bool ask_for();
Forward Hidden Procedure start_help();
Forward Hidden Procedure getentryfor();
Forward Hidden char *addstr();
Forward Hidden Procedure more_help();

/*
 * Print help blurb.
 * This is done through the standard screen interface.
 * The user must type [RETURN] to continue.
 */

Visible bool
help()
{
	int len = sizeof buffer;
	bool two_columns;
	int h;
	bool more= Yes;
	int nprinted= 0;
	
	if (nitems == 0)
		start_help();
	if (llength < (sizeof buffer)-1)
		len= llength+1;
	two_columns= len > 2*maxwidth+GAPWIDTH;
	trmscrollup(0, winheight-1, 1);
	for (h= 0; h < nitems && more /****&& !trminterrupt()*****/; h++) {
		trmputdata(winheight-1, winheight-1, 0, helpitem[h], (string)0);
		if (two_columns) {
			h++;
			trmputdata(winheight-1, winheight-1, 
				maxwidth+GAPWIDTH, helpitem[h], (string)0);
		}
		trmscrollup(0, winheight-1, 1);
		trmsync(winheight-1, 0);
		if (++nprinted >= winheight-1) {
			more= ask_for(MORE);
			nprinted= 0;
		}
	}
	if (nprinted > 0)
		more= ask_for(MORE);
	if (more) {
		more_help();
	}
	if (doctype == D_immcmd)
		cmdprompt(CMDPROMPT);
	else
		winstart= winheight-1;
	
	return Yes;
}

Visible bool ask_for(nr) int nr; {
	int c;

	trmputdata(winheight, winheight, 0, "", (string)0);
	strcpy(buffer, getmess(nr));
	if (modebuffer[0] == '\0') {
		for (c=0; c < MAXBUFFER; c++)
			modebuffer[c]= STANDOUT;
	}
	trmputdata(winheight, winheight, 0, buffer, modebuffer);
	trmsync(winheight, strlen(buffer));
	c = trminput();
	while (c != '\n' && c != '\r' && c != ' ' && c != EOF) {
		trmbell();
		c = trminput();
	}
	trmputdata(winheight, winheight, 0, "", (string)0);
	trmsync(winheight, 0);
	return c == ' ' ? Yes : No;
}

Hidden Procedure start_help()
{
	int h;
	int code;
	int w;

	for (h= 0; h < ((sizeof(helpcode))/(sizeof(int))); h++) {
		code= helpcode[h];
		if (code == NOTHING) {
			strcpy(buffer, "");
		}
		else {
			getentryfor(code); /* result in buffer */
		}
		w= strlen(buffer);
		if (maxwidth < strlen(buffer))
			maxwidth= w;
		helpitem[nitems++]= (char*)savestr(buffer);
	}
}

Hidden Procedure getentryfor(code) int code; {
	int d;
	char *bufp= buffer;
	bool first= Yes;
	
	for (d=ndefs; d > 0; d--) {
		if (code == deftab[d].code) {
			if (bufp == buffer) {
				bufp= addstr(bufp, deftab[d].name, 13);
			}
			if (deftab[d].def != NULL &&
			    deftab[d].rep != NULL && (deftab[d].rep)[0] != '\0'
			   ) {
				if (first)
					first= No;
				else
					bufp= addstr(bufp, ", ", 0);
				bufp= addstr(bufp, deftab[d].rep, 0);
			}
		}
	}
	if (first)
		bufp= addstr(bufp, "", 0);
}

Hidden char *addstr(bp, s, minw) char * bp; string s; int minw; {
	while (*s && bp < buffer+MAXBUFFER) {
		*bp++= *s++;
		minw--;
	}
	while (minw > 0 && bp < buffer+MAXBUFFER) {
		*bp++= ' ';
		minw--;
	}
	if (bp >= buffer+MAXBUFFER)
		bp--;
	*bp= '\0';
	return bp;
}

Hidden FILE *helpfp= NULL;

Hidden Procedure more_help() {
	string cp;
	int nprinted= 0;
	bool more= Yes;
	bool len= (llength < sizeof buffer ? llength+1 : sizeof buffer);
	
	if (helpfp == (FILE*) NULL) {
		if (helpfile) helpfp= fopen(helpfile, "r");
		if (helpfp == (FILE*) NULL) {
			ederrS(NO_HELPFILE, (string) helpfile);
			return;
		}
	}
	while (fgets(buffer, len, helpfp) && more /***&& !trminterrupt()***/) {
		if ((cp= strchr(buffer, '\n')) != NULL) {
			*cp= '\0';
		}
		trmputdata(winheight-1, winheight-1, 0, buffer, (string)0);
		trmscrollup(0, winheight-1, 1);
		trmsync(winheight-1, 0);
		if (++nprinted >= winheight-1) {
			more= ask_for(MORE);
			nprinted= 0;
		}
	}
	rewind(helpfp);
	if (nprinted > 0)
		more= ask_for(NO_MORE);
}

#endif /* HELPFUL */
