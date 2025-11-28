/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1988. */

/* About MacABC ...*/

#include "mac.h"
#include "rez.h"
#include "macabc.h"

/* Display a simple "About this program..." box.
   For now, this is simply a picture resource
   drawn in the simplest possible window, and we
   wait till a key or mouse down event occurs
   (this is not unlike what the Finder does).
   Most work is getting the window and picture
   at the right spot. */

Visible Procedure do_about() {
	PicHandle picture;
	WindowPtr win;
	Rect r;
	EventRecord e;
	GrafPtr saveport;
	
	GetPort(&saveport);
	picture= (PicHandle) GetResource('PICT', ABOUTPICTID);
	if (picture == NULL) {
		macerr(NOABOUTPICT);
		return;
	}
	r= (*picture)->picFrame;
	OffsetRect(&r, (WINWIDTH(screen) - (r.right+r.left)) / 2,
		(WINHEIGHT(screen) - (r.bottom+r.top)) / 2);
	win= NewWindow((Ptr)NULL, &r, "", true, plainDBox,
		(WindowPtr) (-1), false, 0L);
	
	SetPort(win);
	DrawPicture(picture, &win->portRect);
	SetPort(saveport);
	ReleaseResource(picture);
	set_arrow();
	while (!GetNextEvent(mDownMask|keyDownMask, &e))
		;
	DisposeWindow(win);
}
