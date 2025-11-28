/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1988. */

#include "mac.h"
#include <Strings.h>
#include <Files.h>
#include "macabc.h"
#include "bmem.h"

Visible char **appfiles;
Visible int nappfiles;
Visible short appvrefnum;	/* used in m1file.c to change to appdir */

/* Open or print documents passed by the Finder. */

Visible int getappfiles() {
	short message;
	short count;
	AppFile a_info;
	short i;
	char *fname;
	FInfo f_info;
	extern long std_type;
	extern long std_creator;
	
	CountAppFiles(&message, &count);
	
	if (count > 0) {
		if (message == appOpen && nappfiles > 1) {
			macerr(ONLYONEFROMFINDER);
		}

		appfiles= (char**) getmem((unsigned) count*sizeof(char*));
		nappfiles= 0;
		for (i= 1; i <= count; ++i) {
			GetAppFiles(i, &a_info);
			if (i == 1)
				appvrefnum= a_info.vRefNum;
			/*** else: seems to be impossible to open files from
			 *** different Folders; so, we hope any other vRefNum
			 *** implies the same directory ***/
			fname= p2cstr(&a_info.fName);
			GetFInfo(fname, a_info.vRefNum, &f_info);
			if ((long)f_info.fdType != std_type
			    || (long)f_info.fdCreator != std_creator)
			{
				macerrS(NOTABC, fname);
			}
			else if (message == appPrint && is_internal(fname)) {
				macerrS(NOPR_INTERNAL, fname);
			}
			else {
				appfiles[nappfiles++]= savestr(fname);
			}
			ClrAppFiles(i);
		}
		if (nappfiles == 0) {
			macerr(NOABCFILESLEFT);
			return DoExit;
		}
		if (message == appPrint)
			return PrintAppfiles;
		else if (is_howto(fname))
			return RunHowto;
		else if (is_wsgroup(fname))
			return OpenBWS;
		else if (is_copybuffer(fname))
			return OpenCOPYBUFFER;
		else
			return OpenWSP;
		/* NOTREACHED */
	}
	else
		return RunABC;
}
