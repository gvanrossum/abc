/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1988. */

extern char code_array[];
extern char invcode_array[];
extern int lastcode;

extern Procedure initcodes();

#define RANGE 128 /* ASCII characters are in {0 .. RANGE-1} */

#define Code(c) code_array[c]
#define Invcode(code) invcode_array[code]
