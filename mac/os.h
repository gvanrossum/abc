/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1988. */

/* Operating system dependent ABC configuration */

#include <Math.h>
#include <CType.h>
#include <String.h>
#include <Strings.h>
#include <Types.h>
#include "sys_stat.h"
#include "sys_types.h"

#define VOID (void)	/* VOID is used in casts only */

#undef SIGNAL
#undef SIGTYPE

#undef SETJMP

extern int pollcnt;
#define POLLDELAY 100
#define Interrupted() \
	((pollcnt > POLLDELAY ? poll_interrupt(), pollcnt= 0 : pollcnt++), interrupted)

#define DELIM ':'
#define SIZE_PATH 256

#define SEPARATOR ':'
#define CURDIR ":"
#define PARENTDIR "::"
#define Issep(c) ((c) == SEPARATOR)
#define Isanysep(c) Issep(c)
#define Isabspath(path) ((!(Issep(*(path)))) && (strchr((path),':') != NULL))

#define F_readable(f)	(access(f, 4) == 0)
#define F_writable(f)	(access(f, 2) == 0)
#define F_exists(f)	(access(f, 0) == 0)
#define D_exists(d)	(1)

#define Mkdir(path)	(mkdir(path))

/***************** code only for the mac ********************************/

/*#define mkdir(p,m) (mkdir(p)) *** nested too deeply?*/

/* Call Macsbug */

pascal void Debugger() extern 0xA9FF;
/* a call of the following needs #include <strings.h> for c2pstr() */
/*pascal void DebugStr(aString) char *aString; extern 0xABFF;*/
/* use as DebugStr(c2pstr("Entered power loop")); */

