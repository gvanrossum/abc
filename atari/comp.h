/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1991. */

/* Compiler: GNU C cross compiler */

typedef unsigned char literal;  /* to emphasize meaning only */

typedef int expint;		/*The 2nd argument of frexp points to this */
				/*(see manual page frexp(3)).              */
				/*On some 68K systems must be short (foo!) */

#define SETJMP                  /* can #include <setjmp.h> */
#define SIGNAL                  /* can #include <signal.h> */
#define SIGTYPE void     	/* type returned by signal handler function */

/* #define NO_VOID if your compiler doesn't have void */
#undef NO_VOID
/* the NO_VOID define is also used in b.h for Procedure */
/* VOID is used in casts only, for quieter lint */
#ifdef NO_VOID
#define VOID
#else
#define VOID (void)
#endif

#define STRUCTASS       	/* C compiler knows structure assignment */

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

