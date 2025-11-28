/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1986. */

/* B editor -- read key definitions from file */

/* On the Mac there is no keydefinitions file. 
 * Only the inchar() routine to read keysequences from the keyboard
 * remains.
 *
 * With the advent of menu's, trminput can return menuchoices
 * (which are put in 16 bit for compilers where int's are short);
 * these must be passed to do_menu_choice() by inchar().
 * Do_menu_choice returns HANDLED if it itself perform the action requested
 * (most APPLEMENU and FILEMENU items);
 * otherwise it returns the Code for the EditorOperation to be executed
 * and passes it to the editor.
 * Note that an attempt is made to leave the menu hilited in the latter case
 * for as long as the editor is busy.
 */

#include "b.h"
#include "feat.h"
#include "bmem.h"
#include "bobj.h"
#include "bfil.h"
#include "keys.h"
#include "getc.h"
#include "abcmenus.h"

#include <Types.h>
#include <Menus.h>

/* The following table binds the ascii control characters that
 * can be generated from the keyboard to menuchoices.
 * (Compare IM I-248, Keyboard Events).
 */

typedef struct menuchar {
	int code;
	int choice;
} menuchar;

menuchar chartable[] = {
	{0x03, Menuchoice(SpecialID, ExitItem)},		/* Enter */
	{0x08, Menuchoice(EditID, UndoItem)},			/* Backspace */
	{0x09, Menuchoice(SpecialID, AcceptItem)},		/* Tab */
	{0x0D, Menuchoice(SpecialID, NewlineItem)},		/* Return */
	{0x1B, Menuchoice(EditID, ClearItem)},			/* Clear */
	{0x1C, Menuchoice(FocusID, LeftItem)},			/* Left Arrow */
	{0x1D, Menuchoice(FocusID, RightItem)},			/* Right Arrow */
	{0x1E, Menuchoice(FocusID, UpItem)},			/* Up Arrow */
	{0x1F, Menuchoice(FocusID, DownItem)},			/* Down Arrow */
	{0, 0}	/* CHARTABLE END */
};

/* To handle hiliting of menu title's:
 *
 * If a menu item is choosen, trminput() will leave the menu's title hilited.
 * So, we unhilite before next call to trminput().
 * The only problem is when the editor's operation-loop is ended
 * in editdocument(); this is handled by a call of unhilite() in e1edoc.c.
 * (Note that trminterrupt() in m1trm.c handles it's unhiliting itself.)
 */

Visible Procedure unhilite() {
	HiliteMenu(0);
}

/* Read a command from the keyboard.
 * Note that there are no composite keysequences bound to an operation
 * as in the unix and PC version.
 */

Visible int inchar()
{
	register int c;
	menuchar *m;
	
	for (;;) {
		unhilite();
		c= trminput();
		if (c == EOF) return c;
		
		m= chartable;
		while (m->code != 0) {
			if (c == m->code) {
				c= m->choice;		/* map ascii control to menuchoice */
			}
			m++;
		}
		if (ID(c) == 0) {
			return c;		/* normal char OR GOTO (== mouseclick) */
		}
		else {
			c= do_menu_choice(c);
			if (c != HANDLED) {
				return c;		/* to handle it in editdocument() in e1edoc.c */
			}
			/* else continue to ask input for next operation... */
		}
	}
}

/* Dummy routines on MAC: */

/* Output the terminal's initialization sequence, if any. */
Visible Procedure initgetc() {}

/* Output a sequence, if any, to return the terminal to a 'normal' state. */
Visible Procedure endgetc() {}

Visible Procedure initkeys() {}
