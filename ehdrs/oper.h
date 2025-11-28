/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1986. */

/*
 * Command key definitions as returned by getoperation() in e1getc.c.
 *
 * Old kludges removed.
 * The values must differ from the ones returned by getoperation():
 *	32..126 (for printable chars == ABC alphabet)
 *	EOF (for end of file)
 *	0377 (for ignore, rings a bell?).
 *
 * New kludge: the order is used for mapping menuchoices 
 * to editoroperations (saves static data on the Mac:-);
 * the "holes" are caused by deviding lines in the menus
 * and later used for operations not on any menu.
 * So, look at mhdrs/abcmenus.h and mac/m1menus.c if you change this.
 */

#define IGNORE 0377

/* Focus menu: */
#define 	FOCUS 0 /* one less than WIDEN */
#define WIDEN		1
#define EXTEND		2
#define FIRST		3
#define LAST		4
#define PREVIOUS	5
#define NEXT		6

#define UPLINE		8
#define DOWNLINE	9

#define UPARROW		11
#define DOWNARROW	12
#define LEFTARROW	13
#define RITEARROW	14

/* Edit menu: */
#define		EDIT 14 /* one less than UNDO */
#define UNDO		15
#define REDO		16
#define CUT		17
#define COPY		18
#define PASTE		19
#define DELETE		20

/* Special menu: */
#define 	SPECIAL 20 /* one less than ACCEPT */
#define ACCEPT		21
#define NEWLINE		22

#define RECORD		24
#define PLAYBACK	25

#define EXIT		27

/* Pause menu: */
#define CANCEL		28

/* not on menus: */

#define GOTO		7
#define MOUSE		10

#define SUSPEND		23

#define REDRAW		26
#define LOOK		REDRAW

#define HELP		29

#ifdef KEYS
#define DELBIND		30
#endif

/* string-valued codes; must be < 0 */

#define TERMINIT 	-1
#define TERMDONE	-2
#define GSENSE		-3
#define GFORMAT		-4
#define MSENSE          -5
#define MFORMAT		-6

/* the next one is only used in printing the helpblurb
   and should differ from all others above.
 */
#define NOTHING		0

/* the same for menus handled by do_menu_choice(),
 * instead of passed to editdocument().
 */
#define HANDLED		0


/* the strings belonging to the codes above: */

#define S_IGNORE	"[ignore]"

#define S_WIDEN		"[widen]"
#define S_EXTEND	"[extend]"
#define S_FIRST		"[first]"
#define S_LAST		"[last]"
#define S_PREVIOUS	"[previous]"
#define S_NEXT		"[next]"
#define S_UPLINE	"[upline]"
#define S_DOWNLINE	"[downline]"
#define S_UPARROW	"[up]"
#define S_DOWNARROW	"[down]"
#define S_LEFTARROW	"[left]"
#define S_RITEARROW	"[right]"
#define S_GOTO		"[goto]"
#define S_MOUSE		"[mouse]"
#define S_ACCEPT	"[accept]"
#define S_NEWLINE	"[newline]"
#define S_RETURN	"[return]"
#define S_UNDO		"[undo]"
#define S_REDO		"[redo]"
#define S_COPY		"[copy]"
#define S_DELETE	"[delete]"
#define S_RECORD	"[record]"
#define S_PLAYBACK	"[playback]"
#define S_REDRAW	"[redraw]"
#define S_LOOK		"[look]"
#define S_HELP		"[help]"
#define S_EXIT		"[exit]"
#define S_INTERRUPT	"[interrupt]"
#define S_CANCEL	"[cancel]"
#define S_SUSPEND	"[suspend]"
#define S_PASTE		"[paste]"
#define S_CUT		"[cut]"

/* The following are not key defs but string-valued options: */

#define S_TERMINIT	"[term-init]"
#define S_TERMDONE	"[term-done]"
#define S_GSENSE	"[cursor-sense]"
#define S_GFORMAT	"[cursor-format]"
#define S_MSENSE        "[mouse-sense]"
#define S_MFORMAT	"[mouse-format]"
#define S_NOTHING	""
