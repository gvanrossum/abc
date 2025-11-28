/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1990. */

#include "b.h"

#ifndef HAS_READDIR

/* Mark Williams C */

#include "dir.h"

#include <osbind.h>
#include <stat.h>

#define OPENED	1
#define READ	2

DIR dirid;
DMABUFFER *save_dta;
DMABUFFER buf;

Visible DIR *opendir(filename)
     char *filename;
{
	if (dirid.flags & OPENED) return NULL;
		/* more than one open directory is impossible. This
		 * could be avoided by allocating memory dynamically.
		 * Since we don't need more than one open directories 
		 * at any one time, why bother?
		 */
	strcpy(dirid.name, filename);
	strcat(dirid.name, "\\*.*");	/* Add Wild Card */

	/* Save old DTA */
	save_dta = (struct stat *)Fgetdta();

	/* set the new DTA */
	Fsetdta(&buf);

	dirid.flags = OPENED; /* opened and not read */
	return &dirid;
}

Visible struct direct *readdir(dummy) /* (C)Eddy - dummy not used */
     DIR *dummy;
{
	char *p;
	if (!(dirid.flags & OPENED)) return NULL;
		/* can't read from unopened directory */
	if (!(dirid.flags & READ)) {
		dirid.flags |= READ;
		/* Read the first match for wild card */
		if(Fsfirst(dirid.name, (0x01 | 0x010 | 0x020)) != 0)
			/* No such file(s), simply return */
			return NULL;
	} else 	if (Fsnext() != 0)
		return NULL;
	/* Convert it to lower case */
	for(p = buf.d_fname; *p; p++) {
		if (isupper(*p)) *p=_tolower(*p);
	}
	return ((struct direct *)(buf.d_fname));
}

Visible Procedure closedir(dummy) /* (C)Eddy - dummy not used */
     DIR *dummy;
{
	dirid.flags=0; /* neither opened nor read */
	dirid.name[0]='\0'; /* no name */
	/* reset dta */
	Fsetdta(save_dta);
}

#endif /* !HAS_READDIR */

