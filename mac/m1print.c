/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1988. */

#include "mac.h"
#include "bfil.h"
#include "macabc.h"
#include "rez.h"
#include <Printing.h>

Hidden THPrint prrec= NULL;

/* To check Printing Manager Errors we call PrError().
 * Only outside of the printingloop (see TechNote ???) we
 * use an alert (for the same reason as in m1bio.c) to report the error.
 */

#define check_prerr() (VOID no_prerr())

Hidden bool no_prerr() {
	int err= PrError();
	char buf[20];
	
	if (err == noErr)
		return Yes;

	if (err == iPrAbort) {
		VOID Alert(PRABORTALERT, (ProcPtr) NULL);
	}
	else if (err == -4101) { /* technote#72 */
			macerr(NOLW);
	}
	else {
		sprintf(buf, "%d", err);
		macerrS(PRMANAGER, buf);
	}

	return No;
}

Hidden bool valid_prrec() {
	if (prrec == NULL) {
		prrec= (THPrint) NewHandle(sizeof(**prrec));
		if (prrec == NULL) {
			macerr(NOMEM);
			return No;
		}
		PrintDefault(prrec);
		if (!no_prerr()) {
			DisposHandle(prrec);
			prrec= NULL;
			return No;
		}
	}
	else
		VOID PrValidate(prrec);
	return Yes;
}

Visible Procedure do_pagesetup() {
	PrOpen();
	if (no_prerr() && valid_prrec()) {
		set_arrow();
		if (PrStlDialog(prrec))
			check_prerr();
	}
	PrClose();
	recabcdir();	/* since PrOpen() does chdir(root) (or boot?) */
}

/* The various print commands have a generic printloop.
 * The only difference is in selecting which file(s) to print.
 * This is done by the various slct_xxx() routines (which can fail!).
 */

Visible bool prtempfile;

Visible Procedure pr_location() {
	prtempfile= No;
	if (slct_location()) {
		clearupdates();
		do_print();
	}
	if (prtempfile) unlink(PRTEMPFILE);
}

Visible Procedure pr_howto() {
	if (!slct_howto())
		return;
	clearupdates();
	do_print();
}

Visible Procedure pr_workspace() {
	if (!slct_workspace())
		return;
	do_print();
}

Visible Procedure pr_appfiles() {
	if (!slct_appfiles())
		return;
	do_pagesetup();
	clearupdates();
	do_print();
}

/* generic printing */

Hidden Procedure do_print() {
	GrafPtr prevport;

	init_linebuf(); /* getmem */
	GetPort(&prevport);
	PrOpen();
	if (no_prerr() && valid_prrec()) {
		set_arrow();
		if (PrJobDialog(prrec) && no_prerr()) {
			clearupdates();
			set_watch();
			prdlog_and_loop();
		}
	}
	PrClose();
	SetPort(prevport);
	end_slct();		/* to clear the memory */
	recabcdir();
}

Hidden Procedure prdlog_and_loop() {
	DialogPtr prdlog;
	
	prdlog= GetNewDialog(PRINTDLOG, (WindowRecord*)NULL, (WindowPtr)-1);
	DrawDialog(prdlog);
	printloop();
	DisposDialog(prdlog);
}

/* The real printloop */

Hidden int pfile;	/* index of file currently printed */
Hidden FILE *pfp;	/* file ptr of same */
Hidden int page;
Hidden bool lastpage;

#define Spooling() ((*prrec)->prJob.bJDocLoop == bSpoolLoop)

Hidden Procedure printloop() {
	pfile= 0;
	pfp= NULL;
	page= 1;
	lastpage= No;
	
	while (lastpage == No)
		print_and_spool();
}

Hidden Procedure print_and_spool() {
	TPPrPort prport;
	char buf[25];
	TPrStatus prstatus;
	bool drawpage();
	
	prport= PrOpenDoc(prrec, (TPPrPort)NULL, (Ptr)NULL);
	while (PrError() == noErr) {
		PrOpenPage(prport, (TPRect)NULL);
		if (PrError() == noErr)
			lastpage= drawpage((GrafPtr) prport);
		else
			lastpage= Yes;
		PrClosePage(prport);
		page++;
		if (page%iPFMaxPgs == 0 || lastpage == Yes)
			break;
	}
	PrCloseDoc(prport);
	if (Spooling() && PrError() == noErr) {
		PrPicFile(prrec, (TPPrPort)NULL, (Ptr)NULL, (Ptr)NULL, &prstatus);
	}
	if (!no_prerr())
		lastpage= Yes;
}

#define LEFTMARGIN 10
#define RIGHTMARGIN 10
#define TOPMARGIN 15
#define BOTTOMMARGIN 15

#define LINESIZE 256

extern char *abcdir;
Forward char *getpartline();

/* draw a page, reading lines from files prfile[pfile++];
 * returns whether this was the last page (EOF AND no more files left)
 */
Hidden bool drawpage(prport)
	GrafPtr prport;
{
	FontInfo info;
	int h, v;
	char *line;
	int linewidth;
	int lineheight;
	char lbuf[LINESIZE];
	char *pfname;
	string getmess();
	string curwsname();
	
	TextFont(config.pfont);
	TextSize(config.psize);
	GetFontInfo(&info);
	
	h= (prport->portRect).left+LEFTMARGIN;
	v= (prport->portRect).top + info.ascent + info.leading + TOPMARGIN;
	linewidth= (((prport->portRect).right - RIGHTMARGIN) - h)/CharWidth('n');
	lineheight= info.leading + info.ascent + info.descent;
	if (linewidth > LINESIZE)
		linewidth= LINESIZE;
	
	MoveTo(h, v);

	sprintf(lbuf, "MacABC  >%-*.*s Page %d",
			linewidth-20, linewidth-20, curwsname(), page);
	DrawString(lbuf);
	v += 2*lineheight;
	MoveTo(h, v);

	for (;;) {
		/* we can print at least one line on current page */
		if (pfp == NULL) {
			/* no open file */
			if (pfile >= nprfile) {
				/* no files left */
				return Yes;
			}
			pfname= makepath(abcdir, prfile[pfile]);
			pfp= fopen(pfname, "r");
			freepath(pfname);
		}
		/* now we have an open file OR an unopenable one */
		if (pfp == NULL) {
			sprintf(lbuf, getmess(COULDNOTOPEN), pfname);
			line= lbuf;
		}
		else
			line= getpartline(pfp, linewidth);
		if (line != NULL)
			DrawString(line);
		if (line == NULL || pfp == NULL) {
			/* EOF or unopenable file; go to next one */
			if (pfp != NULL)
				fclose(pfp);
			pfp= NULL;
			pfile++;
			if (pfile < nprfile) {
				if (v + 3*lineheight + BOTTOMMARGIN
				    > prport->portRect.bottom)
				{
					/* prevent widows; skip to next page */
					return No;
				}
				/* else: an empty line */
				/***v += lineheight;***/
			}
			else return Yes; /* no more files */
		}
		v += lineheight;
		if (v + info.descent + BOTTOMMARGIN > (prport->portRect).bottom)
			return No;
		MoveTo(h, v);
	}
	return No; /* not done */
}

Hidden char *linebuf= NULL;

Hidden Procedure init_linebuf() {
	if (linebuf == NULL)
		linebuf= (char*) getmem(LINESIZE);
}

Hidden char *getpartline(fp, maxlen) FILE *fp; int maxlen; {
	char *eol;
	char *strchr();
	
	if (fgets(linebuf, maxlen, fp) != NULL) {
		if ((eol=strchr(linebuf, '\r')) != NULL)
			*eol= '\0';
		return linebuf;
	}
	/*else*/
	return (char*) NULL;
}
