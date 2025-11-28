/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1990. */

/* Generic version, as basis for porting.
 * See Portability Guide (./PORTING.DOC) for details.
 */

#include "b.h"

#ifndef HAS_READDIR

#include "dir.h"

Visible DIR *opendir(name)
     char *name;
{
}

Visible struct direct *readdir(dirp)
     DIR *dirp;
{
}

Visible Procedure closedir(dirp)
     DIR *dirp;
{
}

#endif /* !HAS_READDIR */
