/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1988. */

/* Operating system dependent ABC configuration */
/* This contains the things ../mkconfig.c needs */

#include <stdio.h>

typedef unsigned char literal;	/* to emphasize meaning only */

typedef short reftype;		/* type used for reference counts */
#define Maxrefcnt Maxintlet	/* Maxintlet is calculated in config.h */
