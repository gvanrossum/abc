/* VTRM -- a standard 'terminal' interface.

   An implementation of the VTRM interface for the Macintosh, using the
   MPW C compiler.  (A complete specification of the VTRM interface is
   provided at the end of this file.)
   
   This code is Copyright (c) 1986 by Stichting Mathematisch Centrum,
   Amsterdam.
   
   Written by Guido van Rossum, fall 1986.
   Author's addess:
   	CWI, Dept. AA
	P.O.Box 4079 Amsterdam, 1009 AB The Netherlands
   Usenet e-mail: <guido@cwi.nl> or <guido@mcvax.uucp>
   
   Mac-specific features include:
   - Movable window
   - Vertical scroll bar (no grow box, and no continuous scrolling; yet)
   - Optionally intercepts all 'console' output (stdout, stderr) and
     directs it to the window
   - Uses the mouse to pass coordinates to trmsense
   - Supports Desk Accessories and Switcher (but no Clipboard actions)
   - Font, point size, window position and title configuration resource.
   
   Use tab size 8 to list this file.
   
   Caveat - you are not supposed to understand this code without having
   studied Inside Macintosh thoroughly.
*/

/* About the scrolling feature.

   There are now three coordinate systems (at least for y coordinates):
   internal (indexing in lbuf), window (what's on the screen) and
   application (used for the input and output parameters of the
   external interface: trmputdata and friends).
   I have tried to put comments in most relevant places telling which
   coordinate system is used.
*/

#include "mac.h"	/* includes Standard Mac CIncludes */

/* Standard C include file(s). */

#include <ctype.h>

/* Interface definition include files. */

#include "trm.h"
#include "intercept.h"
#include "abcmenus.h"
#include "keys.h"	/* for GOTO */
/* A random collection of standard declarations. */

#define EOS '\0'		/* End of string constant */

extern char *strchr();
extern char *strrchr();
extern char *malloc();


/* Some parametrization definitions. */

#define BLINKINTVAL GetCaretTime() /* Time between cursor flips, in ticks */
	/* (A tick is 1/60 of a second; see also TickCount.) */
	/* GetCaretTime() returns the blink rate set with the Control Panel */

#define MAXCOLS 200		/* Size of line buffer in redisplay() */
	/* Somewhat larger than necessary to accomodate larger screens */

#define LBUF_SIZE 200		/* Number of lines buffered */
				/* Should depend on available memory! */
#define MAXLOOKAHEAD 100	/* Maximum lookahead for typed characters */

/* Global variables.  All file-static. */

static Boolean setup= false;	/* Set once initializations are performed */
static char **lbuf;		/* lbuf[y][x] is char at pos x of line y */
Visible WindowPtr screen;	/* The Window Manager's GrafPort */
static WindowRecord mywinrec;	/* Our window's data */
#define mywin ((WindowPtr)(&mywinrec))	/* A pointer to our window */
static ControlHandle myvbar;	/* Vertical scroll bar */
static Boolean active= false;	/* Set if our window is active */

/* Various values relating to window dimensions: */
static int base;		/* Baseline of first line of text */
static int margin= 4;		/* Left margin of text */
static int cheight;		/* Character height = distance between lines */
static int cwidth;		/* Character width */
static int lines;		/* Total number of lines */
static int wintop;		/* Line number of top window line */
static int winlines;		/* Number of lines in the window */
static int apptop;		/* Line of top line for application */
static int applines;		/* Number of lines for application */
static int cols;		/* Number of columns */

/* Cursor state: */
static int cury= 0;		/* Cursor line (internal coordinates) */
static int curx= 0;		/* Cursor column */
static Boolean curon= false;	/* True if cursor currently displayed */
static long lastinv;		/* Time when last inverted, in ticks */

/* Special cursor pattern for use above application area.
   This is only a default, it can be overridde by putting a 'CURS'
   resource in the application with ID=128. */

Cursor select_cursor= {
	{0x0000, 0x0000, 0x0ff0, 0x0810,
	 0x0810, 0x0810, 0x0810, 0x0810,
	 0x0810, 0x0810, 0x0810, 0x0810,
	 0x0810, 0x0810, 0x0ff0, 0x0000},	/* Data */
	 
	{0, 0, 0, 0,
	 0, 0, 0, 0,
	 0, 0, 0, 0,
	 0, 0, 0, 0},	/* Mask */
	 
	{8, 8}					/* Hot spot */
};


/* Variables controlling look-ahead. */

static unsigned char *lookahead= NULL;	/* Circular lookahead buffer */
static int nlookahead= 0;		/* End of lookahead */
static int ilookahead= 0;		/* Next lookahead character */

/* Pixel to line/col coordinate conversions.  Internal coordinates. */

#define pixel2line(v) ((v)/cheight + wintop)
#define pixel2col(h)  (((h)-margin)/cwidth)


/* Initialize everything.  Called only once. */

static int
initialize()
{
	/* Standard Init calls in order prescribed by Inside Macintosh. */
	InitGraf(&qd.thePort);
	InitFonts();
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs((ProcPtr)NULL);
	
	/* Display menu(s) and window. */
	InitCursor();
	set_watch();
	setup_menus();
	GetWMgrPort(&screen);
	create_window();
	
	/* Allocate memory for text buffer.
	   (Must be done after create_window!) */
	if (!alloc_buffers())
		return TE_NOMEM;
	
	/* Signal successful initialization. */
	setup= true;
	return TE_OK;
}

/* Allocate memory for the text buffer lbuf and for lookahead buffer.
   If all memory can't be allocated, memory already allocated is freed
   and TE_NOMEM is returned. */

static int
alloc_buffers()
{
	int i;
	
	lookahead= (unsigned char*) malloc(MAXLOOKAHEAD*sizeof(unsigned char));
	if (lookahead == NULL)
		return false;
	lbuf= (char**) malloc(lines*sizeof(char*));
	if (lbuf == NULL) {
		free((char*) lookahead);
		lookahead= NULL;
		return false;
	}
	for (i= 0; i < lines; ++i) {
		if ((lbuf[i]= malloc(cols+1)) == NULL) {
			while (--i >= 0)
				free(lbuf[i]);
			free((char*) lbuf);
			lbuf= NULL;
			free((char*) lookahead);
			lookahead= NULL;
			return false;
		}
		lbuf[i][0]= EOS;
	}
	return true;
}

/* Configuration defaults. */
/* You can add a resource 'Conf', ID=0 to change the defaults. */

Visible struct config config=
	{monaco, 9, courier, 10, 504, 294, "The ABC Programming Language"};
/* that will give a proper window with scrollbar,
 * filling almost the entire screen below the menubar on a MacPlus,
 * with 26 lines of 80 characters;
 * for those that misjudge in ResEdit we use the following:
 */
#define MINHSIZE 250
#define MINVSIZE 100
#define MAXHSIZE (r.right-r.left-4)
#define MAXVSIZE (r.bottom-r.top-4)

/* Create and display the window, and set the variables derived from
   its dimensions.  Also initialize the 'select' cursor. */

static
create_window()
{
	Rect r;
	FontInfo info;
	struct config **confp;
	Cursor **cursp;
	
	/* Get configuration resource. */
	confp= (struct config **) GetResource('Conf', 0);
	if (confp != NULL) {
		config= **confp;
		ReleaseResource((Handle)confp);
	}
	
	/* Get alternate cursor for 'select_cursor'. */
	cursp= GetCursor(128);
	if (cursp != 0) {
		select_cursor= **cursp;
		ReleaseResource((Handle)cursp);
	}
	
	/* fiddle to get windowsize */
	r= screen->portRect;
	r.top += MENUBARHEIGHT + WTITLEHEIGHT;
	if (config.hsize < MINHSIZE)
		config.hsize= MINHSIZE;
	if (config.hsize > MAXHSIZE)
		config.hsize= MAXHSIZE;
	if (config.vsize < MINVSIZE)
		config.vsize= MINVSIZE;
	if (config.vsize > MAXVSIZE)
		config.vsize= MAXVSIZE;
	InsetRect(&r,
		(r.right-r.left-config.hsize)/2, /* horizontal inset */
		(r.bottom-r.top-config.vsize)/2  /* vertical inset */
	);
	
	/* Create the window. */
	(void) NewWindow(
		&mywinrec,		/* Window storage */
		&r,			/* Bounds (global coordinates) */
		config.title,		/* Title */
		true,			/* Visible */
		noGrowDocProc,		/* Window type */
		(WindowPtr)(-1),	/* Behind no window */
		false,			/* No go-away box */
		0L			/* RefCon (unused by me) */
		);
	
	/* Select the window, erase it, and set font and point size. */
	SetPort(mywin);
	EraseRect(&mywin->portRect);
	TextFont(config.font);
	TextSize(config.size);
	
	/* Extract dimensions. */
	GetFontInfo(&info);
	base= info.ascent + info.leading/2;
	cheight= info.ascent + info.descent + info.leading;
	cwidth= CharWidth('n'); /* can't use widMax, e.g., for Courier */
	winlines= (mywin->portRect.bottom - mywin->portRect.top) / cheight;
	cols= (mywin->portRect.right - 2*margin - 15 -
			mywin->portRect.left) / cwidth;
		/* NB - leaves space for vertical scroll bar */
	if (cols > MAXCOLS)
		cols= MAXCOLS;
		
	/* Set some experimental values here. */
	lines= LBUF_SIZE; /* Some large value */
	if (lines < winlines)
		lines= winlines; /* Huge window, adjust buffer size */
	applines= winlines;
	apptop= lines-applines;
	wintop= apptop;
	cury= lines-1;
	create_scrollbar();
}

/* Create a vertical scroll bar. */

static
create_scrollbar()
{
	Rect r;
	
	r= mywin->portRect;
	--r.top;
	++r.bottom; /* r.bottom -= 15 if you want a grow icon */
	++r.right;
	r.left= r.right-16;
	myvbar= NewControl(
		mywin,			/* In which window */
		&r,			/* Bounds */
		"",			/* Title (unused for scroll bars) */
		true,			/* Yes, show it immediately */
		wintop,			/* Current indicator value */
		0, apptop,		/* Min, Max value */
		scrollBarProc,		/* Type of control */
		0L			/* RefCon (unused by me) */
		);
}

/* Set the mouse cursor shape to the standard watch.
   This shape is used whenever the package is not waiting for input. */

Visible Procedure
set_watch()
{
	SetCursor(*GetCursor(watchCursor));
}

/* Set the mouse cursor shape to the standard arrow.
   This shape is used when the package is ready for input. */

Visible Procedure
set_arrow()
{
	SetCursor(&qd.arrow);
}

/* Set the mouse cursor shape to our special 'select' cursor. */

Hidden Procedure
set_select()
{
	SetCursor(&select_cursor);
}

/* Main initialization routine.
   This should be called by the application before any other routine.
   Returns an error code, 0 if all went well.
   Passes screen dimensions (application coordinates, of course)
   and some flag bits back via parameters. */

int
trmstart(plines, pcols, pflags)
	int *plines;
	int *pcols;
	int *pflags;
{
	if (!setup) {
		int err= initialize();
		if (err != TE_OK)
			return err;	/* Not initialized successfully. */
	}
	*plines= applines;
	*pcols= cols;
	*pflags= HAS_STANDOUT | CAN_SCROLL | CAN_SENSE;
	return TE_OK;
}

/* Flip the character cursor.
   It is drawn in XOR mode, so the same code can be used to show or
   hide it.  A flag is also maintained telling whether it is shown. */

static
invcursor()
{
	Rect r;
	
	curon= !curon;
	lastinv= TickCount();
	if (cury < wintop || cury >= wintop+winlines ||
			curx < 0 || curx >= cols)
		return;
	SetPort(mywin);
	SetRect(&r, margin + curx*cwidth, (cury-wintop)*cheight,
		margin + (curx+1)*cwidth, (cury-wintop+1)*cheight);
	InvertRect(&r);
}

/* Make sure the cursor is hidden. */

static
rmcursor()
{
	if (curon)
		invcursor();
}

/* Make sure the cursor is shown. */

static
setcursor()
{
	if (!curon)
		invcursor();
}

/* Draw a thin horizontal line above the top line of the application area.
   The line is 'stippled' by using the standard gray pattern. */

static Boolean appborderon= false;

static
invappborder()
{
	int v;
	
	appborderon= !appborderon;
	if (apptop > wintop+winlines)
		return;
	SetPort(mywin);
	PenPat(qd.gray);
	PenMode(patXor);
	v= (apptop-wintop)*cheight - 1;
	MoveTo(margin, v);
	LineTo(margin + cols*cwidth, v);
	PenNormal();
}

rmappborder()
{
	if (appborderon)
		invappborder();
}

setappborder()
{
	if (!appborderon)
		invappborder();
}

/* Say that the screen's state is undefined. */

trmundefined()
{
	rmcursor();
}

/* 'Sense' cursor position.  Returns application coordinates.
   In this implementation, it is actually the mouse position.
   (Shouldn't it report the moust position of the last mouse-up event?) */

int
trmsense(py, px)
	int *py;
	int *px;
{
	Point pt;

	SetPort(mywin);
	GetMouse(&pt);
	*py= pixel2line(pt.v) - apptop;
	*px= pixel2col(pt.h);
	rmcursor();
	return *py >= 0 && *py < applines && *px >= 0 && *px < cols;
}

/* Redisplay a certain range. Internal coordinates. */

static
redisplay(yfirst, ylast, indent)
	int yfirst, ylast;
	int indent;
{
	char *data;
	int i;
	unsigned char c;
	int thislen;
	Boolean needinv= false;
	Rect r;
	char buffer[MAXCOLS];
	
	if (yfirst < wintop)
		yfirst= wintop;
	if (ylast >= wintop+winlines)
		ylast= wintop+winlines-1;
	if (yfirst > ylast)
		return;
	SetPort(mywin);
	rmcursor();
	if (yfirst < apptop)
		rmappborder();
	for (; yfirst <= ylast; ++yfirst, indent= 0) {
		SetRect(&r,
			margin + indent*cwidth, (yfirst-wintop)*cheight,
			margin + cols*cwidth,
			(yfirst-wintop+1)*cheight);
		data= lbuf[yfirst];
		thislen= strlen(data)-indent;
		EraseRect(&r);
		if (thislen > 0) {
			data += indent;
			for (i= 0; i < thislen; ++i) {
				c= data[i];
				buffer[i]= c & 0177;
				if (c & 0200)
					needinv= true;
			}
			MoveTo(margin + indent*cwidth,
				base + (yfirst-wintop)*cheight);
			DrawText(buffer, 0, thislen);
			if (needinv) {
				for (i= 0; i <= thislen; ++i) {
					if ((data[i] & 0200) && i < thislen) {
						if (needinv) {
						  r.left= margin +
						  	(indent+i)*cwidth;
						  needinv= false;
						}
					}
					else if (!needinv) {
						r.right= margin +
							(indent+i)*cwidth;
						InvertRect(&r);
						needinv= true;
					}
				}
			}
		}
	}
	setappborder();
}

/* Redisplay the lines that (partly) fall in the given region. */

static
rgndisplay(rgn)
	RgnHandle rgn;
{
	redisplay(pixel2line((*rgn)->rgnBBox.top),
		pixel2line((*rgn)->rgnBBox.bottom), 0);
}

/* Output a string to the screen.  Application coordinates. */

trmputdata(yfirst, ylast, indent, data)
	int yfirst;
	int ylast;
	int indent;
	char *data;
{
	trmnputdata(yfirst+apptop, ylast+apptop, indent, data, strlen(data));
}

/* Variant on trmputdata with length parameter, used internally.
   Internal coordinates.
   First scroll the application area in view. */

trmnputdata(yfirst, ylast, indent, data, len)
	int yfirst;
	int ylast;
	int indent;
	char *data;
	int len;
{
	int y;
	int x;
	int howmany;
	
	if (yfirst < 0)
		yfirst= 0;
	if (ylast >= lines)
		ylast= lines-1;
	if (indent < 0)
		indent= 0;
	if (indent > cols)
		indent= cols;
	for (y= yfirst, x= indent; y <= ylast; ++y, x= 0) {
		if (x > 0 && x > strlen(lbuf[y])) {
			int i;
			for (i= strlen(lbuf[y]); i < x; ++i)
				lbuf[y][i]= ' ';
		}
		howmany= cols-x;
		if (howmany > len)
			howmany= len;
		strncpy(lbuf[y]+x, data, howmany);
		lbuf[y][x+howmany]= EOS;
		data += howmany;
		len -= howmany;
	}
	redisplay(yfirst, ylast, indent);
}

/* Scroll lines yfirst..ylast up by n lines (down if n < 0).
   Application coordinates.
   There is a special case when scrolling up and yfirst is 0,
   so that lines scrolled out of sight in this way can be
   reviewed by the user. */

trmscrollup(yfirst, ylast, n)
	int yfirst;
	int ylast;
	int n;
{
	if (yfirst != 0 || n < 0)
		yfirst += apptop;
	ylast += apptop;
	trmnscrollup(yfirst, ylast, n);
}

/* Ditto, internal coordinates.
   First scroll the application area in view. */

static
trmnscrollup(yfirst, ylast, n)
	int yfirst;
	int ylast;
	int n;
{
	RgnHandle rgn;
	Rect r;
	int yf, yl; /* yfirst and ylast converted to window coordinates */

	if (yfirst < 0)
		yfirst= 0;
	if (ylast >= lines)
		ylast= lines-1;
	if (yfirst > ylast || n == 0)
		return;
	if (yfirst < apptop)
		rmappborder();
	yf= yfirst-wintop;
	yl= ylast-wintop;
	if (yf < 0)
		yf= 0;
	if (yl >= winlines)
		yl= winlines-1;
	SetPort(mywin);
	rmcursor();
	rgn= NewRgn();
	SetRect(&r, margin, yf*cheight,
		margin + cols*cwidth, (yl+1)*cheight);
	ScrollRect(&r, 0, -n*cheight, rgn);
	if (n > 0) {
		scrollbuf(yfirst+n, ylast+1-(yfirst+n), n, 1);
		clearlbuf(ylast-n+1, ylast);
		
	}
	else {
		scrollbuf(ylast+n, ylast+n+1-yfirst, n, -1);
		clearlbuf(yfirst, yfirst-n-1);
	}
	/* Redisplay lines 'scrolled in' from outside the screen. */
	rgndisplay(rgn);
	DisposeRgn(rgn);
	setappborder();
}

/* Move lines in lbuf around to cause a scroll in the text buffer.
   Internal coordinates. */

static
scrollbuf(y, count, n, direction)
	int y;		/* First line to move */
	int count;	/* How many lines to move */
	int n;		/* Scrolling amount */
	int direction;	/* Increment for y */
{
	for (; --count >= 0; y += direction) {
		char *temp= lbuf[y-n];
		lbuf[y-n]= lbuf[y];
		lbuf[y]= temp;
	}
}

/* Clear lines in the text buffer.  Internal coordinates. */

static
clearlbuf(yfirst, ylast)
	int yfirst, ylast;
{
	for (; yfirst <= ylast; ++yfirst)
		lbuf[yfirst][0]= EOS;
}

/* Scroll the window back so the top is at the given line (internal
   coordinates).
   The value is clipped between 0 and (lines-winlines).
   If no change occurs, no calls are made, so it is safe to call this
   routine often. */

static
set_wintop(new)
	int new;
{
	if (new < 0)
		new= 0;
	else if (new >= lines-winlines)
		new= lines-winlines;
	if (new != wintop) {
		int delta= wintop-new;
		RgnHandle rgn= NewRgn();
		Rect r;
		
		rmcursor();
		SetRect(&r, margin, 0,
			margin + cols*cwidth, winlines*cheight);
		ScrollRect(&r, 0, delta*cheight, rgn);
		wintop= new;
		rgndisplay(rgn);
		DisposeRgn(rgn);
		setcursor();
	}
	if (wintop != GetCtlValue(myvbar))
		SetCtlValue(myvbar, wintop);
}

/* Scroll the window if necessary to make the cursor visible.
   Note: this relies on the clipping done by set_wintop(). */

static
display_cursor()
{
	if (cury < wintop)
		set_wintop(cury);
	else if (cury >= wintop+winlines)
		set_wintop(cury - winlines/2);
}

/* Flush output and place character cursor at desired position.
   Application coordinates. */

trmsync(y, x)
	int y;
	int x;
{
	/* In the Macintosh version there is nothing to flush. */
	rmcursor();
	if (y >= 0 && y < applines && x >= 0 && x < cols) {
		cury= y+apptop;
		curx= x;
		setcursor();
		set_wintop(apptop);
	}
}

/* Sound the bell or flash the screen. */

trmbell()
{
	SysBeep(5);
}

/* End communication through this package. */

trmend()
{
	/* In the Macintosh version there is nothing to do here. */
}


/* Input section. */

/* Check if another input character is immediately available. */

int
trmavail()
{
	EventRecord e;

	if (nlookahead != ilookahead)
		return true;
	return EventAvail(mDownMask|keyDownMask /*|autoKeyMask*/, &e);
}

/* Encode the character found in an event record.
   Characters not in the 7-bit ASCII set are returned as 0.
   Characters with the command key held are converted to upper case
   (if a letter) and get their high bit set.
   Other characters are passed as is (these are all ASCII characters).
   Note that the handling of the high bit is a kludge, necessary
   because the original application was written for Unix,
   and the trmputdata interface uses it for character inversion.
   Here we use it to indicate whether the CmdKey was pressed in trminterrupt.
   This should change eventually. */

#define CmdKey 0x80
#define CharMask 0x7F

static int
encodechar(ep)
	EventRecord *ep;
{
	char c= ep->message & charCodeMask;
	
	if (c & CmdKey)
		return 0; /* Change Option characters to 0 */
	else if (ep->modifiers & cmdKey) {
		/* Map to upper case and set high bit. */
		if (islower(c))
			c= toupper(c);
		return c | CmdKey;
	}
	else
		return c; /* Return normal char */
}

Hidden int hackerskey(c) int c; {
	/* discard disabling of Edit-menu items for ABC hackers */
	/* also enable Cmd-? and Cmd-/ (same without Shiftkey) for HELP */
	switch (c) {
	case 'z':
		return Menuchoice(EditID, UndoItem);
	case 'a':
		return Menuchoice(EditID, RedoItem);
	case 'x':
		return Menuchoice(EditID, CutItem);
	case 'c':
		return Menuchoice(EditID, CopyItem);
	case 'v':
		return Menuchoice(EditID, PasteItem);
	case '?':
	case '/':
		return Menuchoice(HelpID, NONMENUKEYS);
	} /*default:*/
	return 0;
}

/* Return next input character or menuchoice. */

/* The caller must handle the menuchoice and unhiliting */
/* Long menuchoices are translated to (possibly shorter) int's */ 

int
trminput()
{
	EventRecord e;
	long mc; /* menuchoice */
	int hc; /* ABC hackers keys */
	
	while (ilookahead != nlookahead) {
		int c= lookahead[ilookahead];
		ilookahead= (ilookahead+1) % MAXLOOKAHEAD;
		if (c&CmdKey) {
			mc= MenuKey((char) c&CharMask);
			if (LID(mc) == 0) {
				if ((hc= hackerskey(c&CharMask)) != 0)
					return hc;
				trmbell();
				continue; /* with next typed_ahead or normal input */
			}
			else
				return Menuchoice(LID(mc), ITEM(mc));
		}
		else
			return c;
	}
	
	set_arrow();
	for (;;) {
		if (!GetNextEvent(everyEvent, &e)) {
			if (e.what == nullEvent) {
				SystemTask();
				if (FrontWindow() == mywin) {
					SetPort(mywin);
					choose_mouse();
				}
			}
		}
		else switch (e.what) {
		
		case activateEvt:
			if ((WindowPtr) e.message != mywin)
				break;	/* added by Timo; ask Guido!! */
						/* guard against improper DA's that don't handle activate's */
			active= (e.modifiers & activeFlag) != 0;
			setcursor();
			if (active) {
				recabcdir();	/* guard against DA's that chdir(root) */
				ShowControl(myvbar);
				unda_menus();
			}
			else {
				HideControl(myvbar);
				da_menus();
			}
			break;
		
		case updateEvt:
			set_watch();
			rmcursor();
			mywin_update((WindowPtr) e.message);
			setcursor();
			set_arrow();
			break;

		case autoKey:
		case keyDown:
			set_watch();
			if ((e.modifiers & cmdKey) != 0) {
				mc= MenuKey(e.message&charCodeMask);
				if (LID(mc) == 0) {
					if ((hc= hackerskey((int) e.message&charCodeMask)) != 0)
						return hc;
					/* else: */
					break;
				}
				return Menuchoice(LID(mc), ITEM(mc));
			}
			else {
				ObscureCursor();
				return e.message&charCodeMask;
			}
			break;	/*NOTREACHED*/
			
		case mouseDown:
		    {
			WindowPtr win;
			Rect r;
			Point pt;
			int id;
			int it;
			
			switch (FindWindow(&e.where, &win)) {
			
			case inMenuBar:
				mc= MenuSelect(&e.where);
				id= LID(mc); it= ITEM(mc);
				if (id == 0)
					break;
				if (id == EditID && it <= 6 && it != 2 &&
					SystemEdit(it-1))
				{
					HiliteMenu(0);
					break; /* handled by desk accessory */
				}
				return Menuchoice(id, it);
					
				break;
			   
			case inSysWindow:
				SystemClick(&e, win);
				break;
			
			case inContent:
			    {
				int partcode;
				ControlHandle ch;
				
			    	if (win != FrontWindow()) {
					SelectWindow(win);
					break;
				}
				if (win != mywin)
					break;
				SetPort(mywin);
				pt= e.where;
				GlobalToLocal(&pt);
				partcode= FindControl(&pt, mywin, &ch);
				if (partcode != 0) {
					if (ch == myvbar)
						trackvbar(&pt, partcode);
					break;
				}
				while (StillDown())
					choose_mouse();
				GetMouse(&pt);
				if (PtInRect(&pt, &mywin->portRect)) {
					set_watch();
					return GOTO; /* ^G */
				}
				break;
			    }
			
			case inDrag:
				r= screen->portRect;
				r.top += MENUBARHEIGHT;
				InsetRect(&r, 4, 4);
				DragWindow(win, &e.where, &r);
				break;
			
			}
			break;
		    }

		}
		if (active && TickCount() >= lastinv + BLINKINTVAL)
			invcursor(); /* Blink cursor */
	}
}

mywin_update(win) WindowPtr win; {
	if (win != mywin)
		return;
	BeginUpdate(mywin);
	DrawControls(mywin);
	rgndisplay(mywin->visRgn);
	EndUpdate(mywin);
}

clearupdates() {
	EventRecord e;
	
	for (;;) {
		if (GetNextEvent(updateMask, &e))
			mywin_update((WindowPtr) e.message);
		else if (e.what != updateEvt)
			break;
	}
}

/* Set the correct mouse cursor according to which part of the window
   it is on.  Call only when mywin is active and the current GrafPort. */

static
choose_mouse()
{
	Point pt;
	int y, x;
	
	GetMouse(&pt);
	if (PtInRect(&pt, &mywin->portRect)) {
		y= pixel2line(pt.v);
		x= pixel2col(pt.h);
		if (y >= apptop && y < apptop+applines &&
				x >= 0 && x < cols) {
			set_select();
			return;
		}
	}
	set_arrow();
}

/* Track the vertical scroll bar.
   (Should use continuous scrolling, but for now...) */

/* Key repeat constants in low memory (set by the Control Panel),
   in ticks (used here to control the continuous scrolling speed). */

#define KeyThresh	(* (short*)0x18e)	/* Delay until repeat starts */
#define KeyRepThresh	(* (short*)0x190)	/* Repeat rate */

static long deadline;	/* Time when next step may start */

static pascal void action(ControlHandle ch, short partcode); /* Forward */

static
trackvbar(pwhere, partcode)
	Point *pwhere;
	int partcode;
{
	int step;
	
	deadline= 0;
	if (partcode == inThumb) {
		action(myvbar,
			TrackControl(myvbar, pwhere, (ProcPtr)NULL));
	}
	else {
		(void) TrackControl(myvbar, pwhere, action);
	}
}

/* The action procedure for continuous scrolling. */

static pascal void
action(ch, partcode)
	ControlHandle ch;
	short partcode;
{
	int step;
	long now= TickCount();
	
	if (now < deadline || partcode == 0)
		return;
	if (deadline == 0 && KeyThresh > KeyRepThresh)
		deadline= now + KeyThresh; /* Longer delay for first time */
	else
		deadline= now + KeyRepThresh;
	switch (partcode) {
	case inUpButton:	step= -1; break;
	case inDownButton:	step= 1; break;
	case inPageUp:		step= 1-winlines; break;
	case inPageDown:	step= winlines-1; break;
	default:		step= 0; break;
	}
	set_wintop(GetCtlValue(ch) + step);
}

/* Check whether interrupt (Command-period) has been pressed.
   If so, previous input is thrown away.
   Unfortunately, we may have to wade through all typed-ahead
   input to find an interrupt. */

bool
trminterrupt()
{
	EventRecord e;
	int ch;
	int n;

	for (;;) {
		
		GetNextEvent(keyDownMask|mDownMask, &e);
		switch (e.what) {

		case keyDown:
			ch= encodechar(&e);
			if (ch == (CmdKey|'.')) {
				/* Found an interrupt. */
				ilookahead= nlookahead; /* Empty lookahead buffer */
				HiliteMenu(0);
				return Yes;
			}
			else {
				/* Found a normal char (or menuselection?). */
				n= (nlookahead+1) % MAXLOOKAHEAD;
				if (n == nlookahead) {
					/* Buffer overflow. */
					SysBeep(1);
				}
				else {
					lookahead[nlookahead]= ch;
					nlookahead= n;
				}
			}
			break;

		case mouseDown:
		    {
				WindowPtr win;
				Rect r;
				Point pt;
				int mc;
				
				switch (FindWindow(&e.where, &win)) {
				
				case inMenuBar:
					mc= MenuSelect(&e.where); /* automatic Pause */
					if (LID(mc) == PauseID && ITEM(mc) == InterruptItem) {
						ilookahead= nlookahead;
						HiliteMenu(0);
						return Yes;
					}
					/* ignore other menuchoices */
					break;
				
				case inSysWindow:
				default:
					break;
					/* ignore too */
				}
				break;
			}
		
		default:
			HiliteMenu(0);
			return No;
		}
	}
}

/* Console output section. */


/* Interface to cwrite for 0-terminated strings.
   This is visible to other files as a debugging aid for programs that
   don't want to use the standard I/O library.
   (The name is the same as a similar function for the IBM PC.)
   Uses internal coordinate system. */

cputs(s)
	char *s;
{
	if (!setup) {
		if (initialize() != TE_OK)
			return;
	}
	cwrite(s, strlen(s));
}

/* Write characters to console. */

static
cwrite(s, length)
	char *s;
	int length;
{
	char *end;
	int len;
	char *tail= s+length;

	display_cursor();
	rmcursor();
	while (length > 0) {
		end= s;
		while (*end != '\n' && end < tail)
			++end;
		len= end-s;
		if (curx + len > cols)
			len= cols-curx;
		trmnputdata(cury, cury, curx, s, len);
		s += len;
		curx += len;
		length -= len;
		if (length > 0 && *s == '\n') {
			curx= cols;
			++s;
			--length;
		}
		if (curx >= cols) {
			curx= 0;
			++cury;
			if (cury >= lines) {
				trmnscrollup(0, lines-1, cury-lines+1);
				cury= lines-1;
			}
		}
	}
	display_cursor();
	setcursor();
}

/* Routine to substitute for the standard console write routine. */

static int
my_write(pb)
	struct controlblock *pb;
{
	cwrite(pb->io_data, (int) pb->io_nbytes);
	pb->io_nbytes= 0;
	return IO_OK;
}

/* Start intercepting console output.
   May be called before or after trmstart.
   The effect is permanent.
   Returns TE_OK if ok, error code otherwise. */

int
intercept()
{
	if (!setup) {
		int err= initialize();
		if (err != TE_OK)
			return err;
	}
	_StdDevs[DEV_CONS].dev_write= my_write;
	return TE_OK;
}


/*
 * The official VTRM specification.
 *
 * The lines and columns of our virtual terminal are numbered
 *      y = {0...lines-1} from top to bottom, and
 *      x = {0...cols-1} from left to right,
 * respectively.
 *
 * The Visible Procedures and functions in this package are:
 *
 * trmstart(&lines, &cols, &flags)
 *      Obligatory initialization call (sets tty modes etc.),
 *      Returns the height and width of the screen to the integers
 *      whose addresses are passed as parameters, and a flag that
 *      describes some capabilities.
 *      Function return value: Yes if all went well, No if the terminal
 *      is not supported.  An error message has already been displayed.
 *
 * trmundefined()
 *      Sets internal representation of screen and attributes to undefined.
 *      This is necessary for a hard redraw, which would get optimised to
 *      oblivion,
 *
 * trmsense(&y, &x)
 *      Returns the cursor position through its parameters
 *      after a possible manual change by the user.
 *
 * trmputdata(yfirst, ylast, indent, data)
 *      Fill lines {yfirst..ylast} with data, after skipping the initial
 *      'indent' positions. It is assumed that these positions do not contain
 *      anything dangerous (like standout cookies or null characters).
 *
 * trmscrollup(yfirst, ylast, by)
 *      Shift lines {yfirst..ylast} up by lines (down |by| if by < 0).
 *
 * trmsync(y, x)
 *      Call to output data to the terminal and set cursor position.
 *
 * trmbell()
 *      Send a (possibly visible) bell, immediately (flushing stdout).
 *
 * trmend()
 *      Obligatory termination call (resets tty modes etc.).
 *
 * You may call these as one or more cycles of:
 *      + trmstart
 *      +    zero or more times any of the other routines
 *      + trmend
 * To catch interrupts and the like, you may call trmend even in the middle
 * of trmstart.
 *
 * PM: trminterrupt, trminput, trmavail, trmsuspend.
 */

