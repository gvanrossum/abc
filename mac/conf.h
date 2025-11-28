/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1986. */

/* Configuration file: some easy changes to the system.                 */

/* Most machine-dependent changes are done automatically by mkconfig,	*/
/* the results of which can be found in config.h.			*/
/* You only need to change this file conf.h under rare circumstances.	*/

/* Miscellaneous definitions*/
typedef int expint;		/*The 2nd argument of frexp points to this */
				/*(see manual page frexp(3)).              */
				/*On some 68K systems must be short (foo!) */

#define Maxtrig 1e16		/*Max x for sin(x), cos(x), tan(x)         */
				/*(Can anybody find a way to compute this  */
				/*automatically?)                          */

#define MaxSmallInt (BASE-1) /* This must be so! */
#define MinSmallInt (-BASE) /* This must be so!!! */

#define SEED getseed()		/*Any suitable random int (eg date or time) */
				/*to start the random number generator with */
