/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1990. */

/* Generic version, as basis for porting.
 * See Portability Guide (./PORTING.DOC) for details.
 */

/* C compiler dependencies */

typedef char literal;	/* to emphasize meaning only */

typedef int expint;	/* What the 2nd arg of frexp points to. */
/* On some 68K systems must be short (foo!). */

#define SETJMP          /* Can #include <setjmp.h> */

#define SIGNAL		/* Can #include <signal.h> */

#define SIGTYPE void	/* Type returned by signal handler function */

/* #define NO_VOID if your compiler doesn't have void */
#undef NO_VOID
/* the NO_VOID define is also used in b.h for Procedure */
/* VOID is used in casts only, for quieter lint */
#ifdef NO_VOID
#define VOID
#else
#define VOID (void)
#endif

#define STRUCTASS	/* C compiler can do structure assignment */

/* malloc() family */
#ifdef __STDC__
#include <stddef.h>
#define MALLOC_ARG (size_t)
void *malloc();
void *realloc();
#else
#define MALLOC_ARG (unsigned)
char *malloc();
char *realloc();
#endif

#define strchr index    /* Use index for strchr */
#define strrchr rindex  /* Use rindex for strrchr */


