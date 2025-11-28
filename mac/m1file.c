/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1988. */

#include "mac.h"
#include "macdefs.h"
#include "macabc.h"

#include "bmem.h"
#include "bfil.h"
#include "args.h"

#include <Files.h>

extern char *getwd();

extern FILE *errfile;		/* should be in screen handling module */

/* Return absolute pathname of current working directory */

#define NAMESIZE 256

Hidden char *dirname;	/* allocated below */

Visible char *curdir() {
	return getwd(dirname);
}

/* See whether a given disk is an MFS volume.
   This is always true if we're not running HFS;
   with HFS running, we look at the volume's
   signature returned by PBHGetVInfo. */

#define MFS_SIGNATURE 0xd2d7

Hidden bool ismfsvolume(volname, vrefnum) char *volname; short vrefnum; {
	HVolumeParam pb;
	char namebuf[NAMESIZE];
	int err;
	
	if (!hfsrunning())
		return TRUE;
	pb.ioNamePtr= c2pstr(strcpy(namebuf, volname));
	pb.ioVRefNum= vrefnum;
	pb.ioVolIndex= -1;
	err= PBHGetVInfo(&pb, FALSE);
	if (err != noErr) {
		sprintf(namebuf, "%d", err);
		putSmess(errfile, 
			MESS(8100, "IO Error %s while reading volume info"), 
			namebuf);
		return TRUE;
	}
	return pb.ioVSigWord == MFS_SIGNATURE;
}

Visible Procedure initfile() {
	char volname[NAMESIZE];
	short volrefnum;
	string appdir;
	string pend;
	string ptail;
	
	dirname= (char*) getmem((unsigned) NAMESIZE);
	
	startdir= savepath(curdir());
		/* always the directory that MacABC is in */
	
	if (appvrefnum != 0) {
		/* user opened an ABC file instead off application */
		/* goto parent of that file(s) */
		SetVol((char*)NULL, appvrefnum);
		appdir= savestr(curdir()); /* e.g. "VOLNAME:dir1:dir2:" */
	}
	
	GetVol(volname, &volrefnum);
	strcat(volname, ":");
	bwsdefault= makepath(volname, BWSNAME);
	buffile= makepath(volname, BUFFILE);
	
	if (ismfsvolume(volname, volrefnum)) {
		if (mac_todo != OpenBWS) {
			putmess(errfile, MESS(8101, "*** This is a non-hierarchical volume\n"));
			putmess(errfile, MESS(8102, "*** I shall use the entire volume as your single workspace\n"));
			wsp_arg= savestr(volname); /* abs path - no chdir in initbws */
			disable_grrec();
		}
		else {
			macerr(MESS(8103, "Sorry, I can't handle a group of workspaces on a non-hierarchical volume"));
			immexit(1);
		}
	}
	
	if (mac_todo == OpenCOPYBUFFER) {
		if (strcmp(volname, appdir) != 0) {
			macerr(MESS(8103, "Sorry, I can only handle a copybuffer that is not contained in a folder"));
		}
		mac_todo= RunABC;
	}
	
	switch (mac_todo) {
	case RunABC:
		/* bws_arg and wsp_arg already NULL */
		/* initbws() fills in defaults */
		break;
	case OpenBWS:
		bws_arg= appdir;
		mac_todo= RunABC;
		break;
	case RunHowto:
		wsp_arg= appdir; /* abc pathname */
		break;
	case OpenWSP:
	case PrintAppfiles:
		pend= appdir + strlen(appdir) -1; /* at trailing colon */
		ptail= pend-1;
		while (ptail > appdir && *ptail != ':')
			--ptail;
		if (ptail == appdir)
			wsp_arg == appdir;	/* abs pathname */
		else {
			/* split in bwsarg:wsparg */
			++ptail;		/* after colon */
			*pend= '\0';	/* delete trailing colon */
			wsp_arg= savestr(ptail);
			*ptail= '\0';	/* abs pathname for bws */
			bws_arg= appdir;
		}
		if (mac_todo == OpenWSP)
			mac_todo= RunABC;
		break;
	}
}

Visible Procedure endfile() {
	freepath(startdir);
	freepath(bwsdefault);
	freepath(buffile);
	endabcwd();
}
