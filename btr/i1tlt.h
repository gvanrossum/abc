/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1986. */

/* Private definitions for B texts, lists and tables */

typedef struct telita {
    HEADER; btreeptr root;
} a_telita, *telita;

#define Itemtype(v) (((telita) (v))->len) /* Itemtype */
#define Root(v) (((telita) (v))->root)
#define Tltsize(v) (Root(v) EQ Bnil ? 0 : Size(Root(v)))

#define Character(v)	((bool) (Type(v) EQ Tex && Tltsize(v) EQ 1))
