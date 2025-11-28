/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1990. */

/* Generic version, as basis for porting.
 * See Portability Guide (./PORTING.DOC) for details.
 */

/* Operating system features */

#include <stdio.h>

#include <math.h>	/* mathematical library */
#include <ctype.h>	/* character classifications */
#include <strings.h>	/* string operations, like strlen() */

#define Mkdir(path) (mkdir(path, DIRMODE))
#define DIRMODE 0777
/* How to make a directory.
 * Remove the 'mode' parameter, if your system's mkdir() doesn't have one.
 */

#include <sys/file.h>	/* for access(2) modes, like R_OK */
/* only used for the macros F_readable etc., below */

#define F_readable(f)	(access(f, R_OK) == 0)
/* Determine whether file (not directory) f exists and is readable;
 * f can be a full pathname, or a filename.
 */

#define F_exists(f)	(access(f, F_OK) == 0)
/* Determine whether file (not directory) f exists;
 * f can be a full pathname, or just a filename.
 */

#define D_writable(f)	(access(f, W_OK) == 0)
/* Determine whether directory f is writable (path or filename). */

#define D_exists(f)	(access(f, F_OK) == 0)
/* Determine whether directory f exists (path or filename). */

#define ENABLE_INTERRUPT()
/* Call system routine to enable interrupts.
 * On some systems, MSDOS for instance, it is necessary to call a system
 * routine to enforce the generation of interrupt signals. If your
 * system is similar, set this macro so that it calls that routine.
 * ENABLE_INTERRUPT is regularly called in pollinterrupt() [bed/e1getc.c].
 */

#define CANLOOKAHEAD
/* Whether trmavail() (in trm.c) can be implemented. */

#define HAS_FTIME	/* ftime() and <sys/timeb.h> available */
/* If not defined, you can use time() in getdatetime() [os.c]. */

#define HAS_MKDIR	/* mkdir() and rmdir() available */
/* If not defined you have to write them [os.c]. */

#define HAS_RENAME	/* rename() available */
/* If not defined you have to write it [os.c]. */

#define HAS_SELECT	/* 4.2 BSD select() system call available */
/* If defined you can use it to implement trmvail(), see unix/trm.c. */

#define HAS_READDIR	/* directory reading routines available */
/* If not defined you have to write routines for opendir(),
 * readdir() and closedir() [dir.c]
 */

#define HAS_GETWD	/* getwd() available */
#define HAS_GETCWD	/* getcwd() available */
/* If both are unavailable, you have to write one of them.
 * Used only in curdir() [os.c].
 */

#ifdef KEYS

#define KNOWN_KEYBOARD
/* If the keyboard is unknown (for instance on Unix, where
 * different keyboards are used), abckeys can't know that, for
 * instance, ESC [-1z has been produced by the HELP key, and so
 * prompts the user for a name. On known keyboards (like MS DOS
 * machines or the Atari ST) abckeys knows which keys send what,
 * and so doesn't need to ask.
 */

#endif /* KEYS */

