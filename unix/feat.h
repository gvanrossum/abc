/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1990. */

/*
 * -- #define features, to make it easy to turn them off.
 */

#define	EXT_RANGE	/* extend range of approximate arithmetic */
#define	TYPE_CHECK	/* do static type checking */
#undef	CLEAR_MEM	/* remove internal adm. before editing a howto */
#undef	SAVE_PERM	/* write perm env after each edit */

#define MAXHIST 101     /* One more than the number of UNDO's allowed. */

typedef short reftype;	/* type used for reference counts */
#define Maxrefcnt Maxintlet /* Maxintlet is calculated in mach.h */

#define SAVEBUF		/* Save Copy Buffer on file between edit sessions */
#define USERSUGG	/* Give suggestions for user-defined commands */
#define SAVEPOS		/* Save focus position between edit sessions */
#define RECORDING	/* [record] and [playback] commands */
#define SCROLLBAR	/* Show scroll bar if unit > screen */
#define SHOWBUF		/* Shows contents of copy buffer if locked */
#define HELPFUL		/* Print help blurb on ESC-? or ? */
#define GOTOCURSOR	/* enable [goto] operation */
#define CK_WS_WRITABLE  /* give warning if workspace is read-only */
