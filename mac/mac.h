/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1988. */

/* Interface for mac specific files */

/* There is a problem with including b.h, because the value typedef
 * makes the C compiler botch in ToolUtils.h (and maybe more ...).
 * Therefore, routines using other parts of the ABC system,
 * but only necessary on the Mac, are isolated in m1macabc.c
 * which doesn't include this file.
 * In addition some define's from b.h are repeated here
 * (and must be kept the same).
 */

#define Visible
#define Hidden static
#define Procedure
#define Forward

#define VOID (void)

typedef int bool;
#define Yes ((bool) 1)
#define No  ((bool) 0)

typedef short int intlet;
typedef char *string;

#define ERRBUFSIZE	300
#define MESS(nr, text) nr
string getmess();
extern char *messbuf;

/* Mac CInclude files.  (Actually, case is not important for
   file names; these are the 'official' names.) */

#include <Types.h>
#include <Errors.h>
#include <Resources.h>
#include <Quickdraw.h>
#include <Fonts.h>
#include <Events.h>
#include <Windows.h>
#include <Controls.h>
#include <Menus.h>
#include <TextEdit.h>
#include <Dialogs.h>
#include <Desk.h>
#include <ToolUtils.h>
#include <OSUtils.h>
#include <SegLoad.h>
#include <StdIO.h>

#define MENUBARHEIGHT 20	/* Height of menu bar, in pixels */
#define WTITLEHEIGHT 20		/* Height of window title, in pixels */

#define FALSE 0
#define TRUE 1

/* (Window) configuration record.
   You can add a resource 'Conf', ID=0 to change the defaults. */

struct config {
	short font;	/* Font used; should be constant width, e.g. Courier */
	short size;	/* Size used (preferably an existing size) */
	short pfont;	/* Font used for printing */
	short psize;	/* Size used for printing */
	short hsize;	/* Horizontal size of window in pixels */
	short vsize;	/* Vertical size of window in pixels */
	char title[32];	/* Window title, max. 31 chars plus terminating \0 */
};

/* Configuration defaults. */

extern struct config config;


extern WindowPtr screen;
/* The following two macros assume win->(top, left) is (0, 0): */
#define WINWIDTH(win) ((win)->portRect.right)
#define WINHEIGHT(win) ((win)->portRect.bottom)
