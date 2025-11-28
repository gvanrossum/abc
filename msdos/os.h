/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1990. */

/* Operating system dependent ABC configuration */

#include <stdio.h>
#include <math.h>
#include <ctype.h>
#include <string.h>

#define HAS_FTIME	        /* ftime() available */
#define HAS_MKDIR	        /* mkdir() and rmdir() available */
#define HAS_RENAME	        /* rename() available */
#undef  HAS_SELECT	        /* 4.2 BSD select() system call available */
#undef  HAS_READDIR	        /* Berkeley style directory reading routines */
			        /* opendir(), readdir() and closedir(); */
/* define either one of the next two, or neither: */
#undef  HAS_GETWD	        /* getwd() available */
#define HAS_GETCWD	        /* getcwd() available instead */

/* how to make a directory; some functions don't have a 'mode' parameter */
#define Mkdir(path)	(mkdir(path))

/* access checks for files/directories */
#define F_readable(f)	(access(f, 4) == 0)
#define D_writable(f)	(access(f, 2) == 0)
#define F_exists(f)	(access(f, 0) == 0)
#define D_exists(f)	(access(f, 0) == 0)

/* Do we have to call a system routine to enable interrupts? (MSDOS only) */
#define ENABLE_INTERRUPT() kbhit()

/* Can you lookahead in the system's input queue, and so can implement
 * trmavail() [trm.c] ?
 */
#define CANLOOKAHEAD

#ifdef KEYS
/* do we know the keyboard codes ? */
#define KNOWN_KEYBOARD
#endif /* KEYS */

