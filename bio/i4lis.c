/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1988. */

#include "b.h"
#include "bmem.h"
#include "bobj.h"
#include "bfil.h"
#include "i3cen.h"
#include "i3sou.h"
#include "i4bio.h"

/* list howto's */

Visible Procedure abclist(ofp) FILE *ofp; {
	intlet k, len;
	FILE *fp;
	value fname;
	char *line;
	
	len= length(cur_env->perm);
	for (k= 0; k<len && !Interrupted(); ++k) {
		fname= *assoc(cur_env->perm, k);
		if (!Is_text(fname) || !unitfile(fname))
			continue;
		fp= fopen(strval(fname), "r");
		if (fp == NULL)
			continue;
		while (!interrupted && (line= f_getline(fp)) != NULL) {
			fputs(line, ofp); /* no vtrm, so no CR worries */
			VOID fflush(ofp);
			freestr(line);
		}
		fclose(fp);
		putc('\n', ofp); /* no vtrm, so no CR worries */
		VOID fflush(ofp);
	}
}
