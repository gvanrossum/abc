/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1988. */

/* Interface to bio files (equivalents of -[iolpurR] options under unix) */

#include "b.h"
#include "bedi.h"
#include "bint.h"
#include "bmem.h"
#include "bobj.h"
#include "i2par.h"
#include "i3bws.h"
#include "i3env.h"
#include "i3scr.h"
#include "i3sou.h"
#include "i4bio.h"
#include "macabc.h"

extern int winheight;

Visible Procedure mac_input() {
	char locname[MAXNAME];
	char fname[MAXFNAME];
	FILE *ifp;
	
	ifp= askfile(fname);
	if (ifp == NULL)
		return;
	clearupdates();
	strcpy(locname, "newtablename");
	if (asknewlocation(locname) == NULL)
		return;
	set_watch();
	clearupdates();
	ifile= ifp;
	rd_interactive= No;
	abcinput(locname);
	VOID fclose(ifp);
	vs_ifile();
	rd_interactive= Yes;
}

Visible Procedure mac_output() {
	char locname[MAXNAME];
	char fname[MAXFNAME];
	FILE *ofp;
	
	strcpy(locname, "tablename");
	if (asklocation(locname) == NULL)
		return;
	clearupdates();
	strncpy(fname, locname, MAXFNAME);
	fname[MAXFNAME-1]= '\0';
	ofp= asknewfile(TAB2TEX, fname);
	if (ofp == NULL)
		return;
	set_watch();
	clearupdates();
	redirect(ofp);
	abcoutput(locname);
	VOID fclose(ofp);
	redirect(stdout);
}

Visible Procedure mac_list() {
	char fname[MAXNAME];
	char *pcolon;
	FILE *ofp;
	
	strncpy(fname, strval(cur_ws), MAXNAME-6);
	fname[MAXNAME-6]= '\0';
	
	/* delete trailing colon if wsname is volumename,
	 * since this confuses the fvopen in m1askfile.
	 */
	if ((pcolon= strchr(fname, ':')) != NULL)
		*pcolon= '\0';
	
	strcat(fname, ".list");
	ofp= asknewfile(HOWTOLIST, fname);
	if (ofp == NULL)
		return;
	set_watch();
	clearupdates();
	redirect(ofp);
	abclist();
	VOID fclose(ofp);
	redirect(stdout);
}

Visible bool unpackoption= No;

Visible Procedure mac_unpack() {
	char locname[MAXNAME];
	char fname[MAXFNAME];
	FILE *ifp;
	
	ifp= askfile(fname);
	if (ifp == NULL)
		return;
	set_watch();
	clearupdates();
	ifile= ifp;
	
	/* prepare for WRITE or error output from packed file: */
	trmputdata(winheight-1, winheight-1, 0, CMDPROMPT);
	trmsync(winheight-1, strlen(CMDPROMPT));
	newline();
	
	/* prepare for errors: */
	f_lino= 0;
	iname= mk_text(fname);
	
	unpackoption= Yes;
	interactive= No;
	while (!Eof && still_ok) {
		imm_command();
	}
	VOID fclose(ifp);
	sv_ifile= ifile= stdin;
	Eof= No;
	interactive= Yes;
	unpackoption= No;
	release(iname);
	iname= Vnil;
	f_lino= 0;
	interrupted= No;
	still_ok= Yes;

	rec_suggestions();
	/* redraw prompt after possible output from packed file: */
	cmdprompt(CMDPROMPT);
	/* do_filemenu will return REDRAW to the editor for this case */
}

Visible Procedure mac_pack() {
	char fname[MAXNAME];
	char *pcolon;
	FILE *ofp;
	
	strncpy(fname, strval(cur_ws), MAXNAME-6);
	fname[MAXNAME-6]= '\0';
	
	/* delete trailing colon if wsname is volumename,
	 * since this confuses the fvopen in m1askfile.
	 */
	if ((pcolon= strchr(fname, ':')) != NULL)
		*pcolon= '\0';
	
	strcat(fname, ".pack");
	ofp= asknewfile(PACKWS, fname);
	if (ofp == NULL)
		return;
	set_watch();
	clearupdates();
	redirect(ofp);
	abcpack(ofp);
	VOID fclose(ofp);
	redirect(stdout);
}

/* macerr uses alerts! */

Visible Procedure bioerr(m) int m; {
	macerrS(m, (char*)NULL);
}

Visible Procedure bioerrV(m, v) int m; value v; {
	macerrS(m, strval(v));
}
