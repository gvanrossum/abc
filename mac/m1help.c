/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1986. */

/*
 * B editor -- Print help blurb.
 *
 * Macintosh version: the help text on a helpfile HELPFILE ("MacABC.help").
 * The topics are between lines with "=i=" and "===", where i is the topic
 * number.
 */

#include "mac.h"
#include "feat.h"

#ifdef HELPFUL

#include <Files.h>
#include "bfil.h"
#include "macabc.h"
#include "macdefs.h"
#include "rez.h"
#include "bmem.h"
#include "bedi.h"
#include "abcmenus.h"

extern int winheight; /* Bottom line of screen */
extern int winstart; /* Current top line of editing area */
extern bool trminterrupt();
extern char *strchr();

#define BUFLEN 100		/* should be function of winwidth */

#define TLEN 5
Hidden char tstart[TLEN];
#define STARTFORMAT "=%d="
Hidden char *tend= "===";
Hidden int topic= NONMENUKEYS;

Hidden char *blessdir= NULL;

/*
 * Print help blurb.
 */

Visible Procedure sethelptopic(item) int item; {
	topic= item;
}

Visible bool help() {
	FILE *hfp;
	string helpfile;
	char buffer[BUFLEN+1];
	char *cp;
	int c;
	int savemenusstat;
	char *blessedfolder();

	helpfile= makepath(startdir, HELPFILE);
	hfp= fopen(helpfile, "r");
	freepath(helpfile);
	if (hfp == NULL) {
		if (blessdir == NULL)
			blessdir= blessedfolder();
		helpfile= makepath(blessdir, HELPFILE);
		hfp= fopen(helpfile, "r");
		freepath(helpfile);
	}
	if (hfp == NULL) {
		macerrS(NOHELPFILE, HELPFILE);
		topic= NONMENUKEYS;
		return No;
	}
	intrprtr_menus();
	sprintf(tstart, STARTFORMAT, topic);
	while (fgets(buffer, BUFLEN, hfp)) {
		if ((cp= strchr(buffer, '\r')) != NULL) {
			*cp= '\0';
		}
		if (strcmp(tstart, buffer) == 0)
			break;
	}
	while (fgets(buffer, BUFLEN, hfp) && !trminterrupt()) {
		if ((cp= strchr(buffer, '\r')) != NULL) {
			*cp= '\0';
		}
		if (strcmp(tend, buffer) == 0)
			break;
		trmputdata(winheight, winheight, 0, buffer);
		trmscrollup(0, winheight, 1);
		trmsync(winheight, 0);
	}
	fclose(hfp);
	trmputdata(winheight, winheight, 0, "");
	
	if (doctype == D_immcmd)
		cmdprompt(CMDPROMPT);
	else
		winstart= winheight;
	
	topic= NONMENUKEYS;
	return Yes;
}

#define SYSMAP (* (short *) 0xA58)
#define BOOTDRIVE (* (short *) 0x210)

Hidden char *blessedfolder() {	/* TechNotes #67 and #77 */
	char *bldir;
	HVolumeParam hpb;
	WDPBRec wpb;
	OSErr err;
	short sysvref;
	short vrefnum;
	
	if (hfsrunning()) {
		err= GetVRefNum(SYSMAP, &sysvref);
		hpb.ioNamePtr= NULL;
		hpb.ioVRefNum= sysvref;
		hpb.ioVolIndex= 0;
		err= PBHGetVInfo(&hpb, FALSE);
		wpb.ioNamePtr= NULL;
		wpb.ioVRefNum= sysvref;
		wpb.ioWDProcID= 'Mabc';
		wpb.ioWDDirID= hpb.ioVFndrInfo[0];
		err= PBOpenWD(&wpb, FALSE);
		if (err != noErr) return NULL;
		vrefnum= wpb.ioVRefNum;
	}
	else {
		vrefnum= BOOTDRIVE;
	}
	err= SetVol((char*)NULL, vrefnum);
	if (err != noErr) {
		closeblesswd(vrefnum);
		return NULL;
	}
	bldir= savestr(curdir());
	closeblesswd(vrefnum);
	recabcdir();
	return bldir;
}

Hidden Procedure closeblesswd(wdrefnum) short wdrefnum; {
	WDPBRec pb;
	
	if (hfsrunning()) {
		pb.ioVRefNum= wdrefnum;
		PBCloseWD(&pb, FALSE);
	}
}
#endif HELPFUL
