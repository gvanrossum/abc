/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1988. */

/*
 * -- #define features, to make it easy to turn them off.
 */

#define SAVEBUF		/* Save Copy Buffer on file between edit sessions */
#define USERSUGG	/* Give suggestions for user-defined commands */
#define SAVEPOS		/* Save focus position between edit sessions */
#define RECORDING	/* [record] and [playback] commands */
#undef SCROLLBAR	/* Show scroll bar if unit > screen */
#undef SHOWBUF		/* Shows contents of copy buffer if locked */
#define HELPFUL		/* Print help blurb on ESC-? or ? */
#define GOTOCURSOR	/* enable [goto] operation */

#ifdef SMALLSYS
/*
 * The #define SMALLSYS squeezes out some lesser important debugging
 * code. Moreover you can #undef here some of the above mentioned features.
 * They are roughly sorted on amount of code saved, greatest
 * saving first.
 */

#endif /* SMALLSYS */

#undef CANSUSPEND	/* Can suspend abc? */
#define STRUCTASS	/* C compiler knows structure assignment */

#undef EXT_RANGE	/* extend range of approximate arithmetic */
#define TYPE_CHECK	/* do static type checking */
#define PRINT_APPROX	/* write approx on perm env as two power */
#undef CLEAR_MEM	/* remove internal adm. before editing a howto */
#undef SAVE_PERM	/* write perm env after each edit */

/* Mac specific: */
#define MENUS		/* install and use menus */
#define BIO_INTERACTIVE /* bio-calls interactive, not from command-line */
#define WSP_DIRNAME	/* wsp_arg is a directory name, not an ABC name */

#undef TTY_ERRFILE	/* redirect error output from interpreter to /dev/tty */
