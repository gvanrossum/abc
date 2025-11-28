/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1990. */

/* Generic version, as basis for porting.
 * See Portability Guide (./PORTING.DOC) for details.
 */

#ifdef HAS_READDIR

#include <sys/dir.h>

#else /* !HAS_READDIR */

struct direct {
	char d_name[ /* some value */ ];
	/* other fields */
};

typedef /* whatever */ DIR;

Visible DIR *opendir();
Visible struct direct *readdir();
Visible Procedure closedir();

#endif /* !HAS_READDIR */
