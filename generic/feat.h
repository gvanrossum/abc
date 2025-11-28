/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1990. */

/* Generic version, as basis for porting.
 * See Portability Guide (./PORTING.DOC) for details.
 */

/* Features of the ABC system */

#define	EXT_RANGE	/* extend range of approximate arithmetic */
/* ABC approximate numbers are implemented with C doubles. If this
 * symbol is defined, the accuracy remains the same, but the
 * exponents are bigger, allowing larger approximate numbers (but
 * slower of course).
 */

#undef	CLEAR_MEM	/* remove internal adm. before editing */
/* Defining this causes the system to free as much memory as
 * possible before it calls the editor. It also writes the
 * workspace administration file, perm.abc. This is mainly of use
 * on machines where main memory is at a premium.  The trade-off is
 * in disk accesses, which go up as a result (permanent locations,
 * and perm.abc have to be written out).  Unfortunately, machines
 * with little memory usually have slow disks too.
 */

#undef	SAVE_PERM	/* write perm env after each edit */
/* If SAVE_PERM is on, the ABC workspace internal administration
 * file (perm.abc) is written after each edit (see also CLEAR_MEM above).
 * Costs more disk writes (only perm.abc in this case);
 * the gain is more safety in the case of a crash.
 */


#define MAXHIST 101     /* maximun length of undo history */
/* One more than the number of UNDO's allowed. */

#define SHORTREFTYPE    /* or CHARREFTYPE, or UCHARREFTYPE; see below */
/* Each abc value has a header that includes a reference count.
 * You can choose between an (unsigned if possible) char and a
 * short for the reference count.
 */

#ifdef SHORTREFTYPE
typedef short reftype;      /* type used for reference counts */
#define Maxrefcnt Maxintlet /* Maxintlet is calculated in mach.h */
#endif

#ifdef CHARREFTYPE
typedef char reftype;
#define Maxrefcnt 255
#endif

#ifdef UCHARREFTYPE
typedef unsigned char reftype;
#define Maxrefcnt 255
#endif

/* The trade-off is as follows: if it is a char, then each value
 * uses less space, but if the reference count ever reaches the
 * value Maxrefcnt (i.e. the maximum positive value that can be
 * held in the reference count), the value will never be
 * released, so you could get a build-up of garbage (however, few
 * reference counts ever get that high).
 *
 * If memory is not at a premium, use a short; if it is, consider
 * using a char. Maxrefcnt should be set accordingly to the
 * maximum positive value that a refcnt can hold.
 */

/* The following are historical features and don't need to be changed. */

#define SAVEBUF		/* Save Copy Buffer on file between edit sessions */
#define USERSUGG	/* Give suggestions for user-defined commands */
#define SAVEPOS		/* Save focus position between edit sessions */
#define RECORDING	/* [record] and [playback] commands */
#define SCROLLBAR	/* Show scroll bar if unit > screen */
#define SHOWBUF		/* Shows contents of copy buffer if locked */
#define HELPFUL		/* Print help blurb */
#define GOTOCURSOR	/* enable [goto] operation */
#define CK_WS_WRITABLE  /* give warning if workspace is read-only */
#define	TYPE_CHECK	/* do static type checking */
