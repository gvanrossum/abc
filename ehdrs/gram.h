/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1986. */

/*
 * Routines defined in "gram.c".
 */

string *noderepr();
node gram();
node suggestion();
node variable();
string symname();
bool allows_colon();

/*
 * Macros for oft-used functions.
 */

#define Fwidth(str) ((str) ? fwidth(str) : 0)

#define Fw_zero(str) (!(str) || strchr("\b\t", (str)[0]))
#define Fw_positive(str) ((str) && (str)[0] >= ' ')
#define Fw_negative(str) ((str) && (str)[0] == '\n')

#define MAXNBUILTIN 50	/* should be calculated by boot/mktable */
