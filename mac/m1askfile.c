/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1988. */

#include "mac.h"
#include "macdefs.h"
#include "macabc.h"
#include <Packages.h>
#include <Files.h>

/* Two routines interfacing to the Standard File Package
   that open the file and return a stream pointer for it.
   The file must be / is created as a TEXT file.
   
   Askfile asks for an input file; asknewfile asks for
   an output file.  Both return a stream pointer to the file,
   and also return its name (not its vrefnum) in the buffer
   passed as argument, should the calling routine like to know it.
   
   A return of NULL means that the user canceled the request,
   or that the file couldn't be opened after all
   (in which case an error alert is first displayed),
   so then the caller should give up without complaints. */


/* Width and Height of Standard File Dialogs.
 * We only adjust the height to the middle of the actual screen.
 * The horizontal offset is fixed in order to keep near the FileMenu
 * from where these routines are "called" by the user.
 */

#define GETWIDTH	348	/* for documentation only */
#define PUTWIDTH	304

#define MFSGETHEIGHT	136
#define HFSGETHEIGHT	200
#define MFSPUTHEIGHT	104
#define HFSPUTHEIGHT	184

#define LEFT		100	/* more or less centers on normal Mac screen */

#define MGETTOP	((MENUBARHEIGHT + WINHEIGHT(screen) - MFSGETHEIGHT)/2)
#define HGETTOP	((MENUBARHEIGHT + WINHEIGHT(screen) - HFSGETHEIGHT)/2)
#define MPUTTOP	((MENUBARHEIGHT + WINHEIGHT(screen) - MFSPUTHEIGHT)/2)
#define HPUTTOP	((MENUBARHEIGHT + WINHEIGHT(screen) - HFSPUTHEIGHT)/2)


Forward FILE *fvopen();

Visible FILE *askfile(namebuf)  char *namebuf; {
	Point corner;
	short numtypes;
	SFTypeList typelist;
	SFReply reply;
	FILE *fp;
	
	if (hfsrunning())
		SetPt(&corner, LEFT, HGETTOP);
	else
		SetPt(&corner, LEFT, MGETTOP);
	numtypes= 1;
	typelist[0]= 'TEXT';
	SFGetFile(&corner, "", (ProcPtr)NULL,
		numtypes, typelist, (ProcPtr)NULL, &reply);
	set_watch();
	if (!reply.good)
		return NULL;
	strncpy(namebuf, reply.fName.text, reply.fName.length);
	namebuf[reply.fName.length]= '\0';
	fp= fvopen(namebuf, reply.vRefNum, "r");
	if (fp == NULL)
		macerrS(FAILOPEN, namebuf);
	return fp;
}

Visible FILE *asknewfile(prompt, namebuf) int prompt; char *namebuf; {
	static Point corner= {100, 100};
	SFReply reply;
	FILE *fp;
	FInfo finfo;
	
	if (hfsrunning())
		SetPt(&corner, LEFT, HPUTTOP);
	else
		SetPt(&corner, LEFT, MPUTTOP);
	SFPutFile(&corner, getmess(prompt), namebuf, (ProcPtr)NULL, &reply);
	set_watch();
	if (!reply.good)
		return NULL;
	strcpy(namebuf, p2cstr((char*) &reply.fName));
	switch (GetFInfo(namebuf, reply.vRefNum, &finfo)) {
	case noErr: /* file exists */
		if (finfo.fdType != 'TEXT') {
			macerrS(WRONGTYPE, namebuf);
			return NULL;
		}
		break;
	case fnfErr:
		if (Create(namebuf, reply.vRefNum, 'MACA', 'TEXT') != noErr) {
			macerrS(NOCREATE, namebuf);
			return NULL;
		}
		break;
	}
	fp= fvopen(namebuf, reply.vRefNum, "w");
	if (fp == NULL)
		macerrS(NOCREATE, namebuf);
	return fp;
}

/* Fvopen opens a stream given by partial file name and vrefnum.
   This is useful if you want to open a file returned by
   the Standard File Package.
   This version is for ABC only; it doesn't need to use PBHSetVol(). */

Hidden FILE *fvopen(file, vrefnum, mode) char *file; short vrefnum; char *mode; {
	FILE *fp;
	
	SetVol((char*)NULL, vrefnum);
	fp= fopen(file, mode);
	recabcdir();
	return fp;
}
