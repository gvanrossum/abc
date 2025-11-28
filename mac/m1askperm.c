/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1988. */

#include "mac.h"
#include "macabc.h"
#include "rez.h"

/* Two routines, not unlike the Standard File dialogs,
   asking the user for a specific location name.
   The user must always type the full location name, and
   it always resides in the current workspace.
   Asklocation asks for an existing location and loops until
   a valid location is given (or the user clicks Cancel).
   Asknewlocation asks for a location to create; it asks
   confirmation to overwrite if the location already exists.
   
   This code could be seen as a simple exercise in using
   the Dialog Manager: there are simple buttons, static
   text and edit text items. */

Visible char *asknewlocation(namebuf) char *namebuf;
{
	DialogPtr dlg= GetNewDialog(NEWLOCDLOG, (Ptr)NULL, (WindowPtr)(-1));
	short item;
	
	setedittext(dlg, namebuf);
	set_arrow();
	for (;;) {
		ModalDialog((ProcPtr)NULL, &item);
		if (item == OKITEM || item == CANCELITEM) {
			if (item == OKITEM) {
				getedittext(dlg, namebuf);
				if (!goodtag(namebuf)) {
					macerrS(LOCNAME, namebuf);
					setedittext(dlg, namebuf);
					continue;
				}
				if (existinglocation(namebuf)) {
					if (!oktoreplace(namebuf)) {
						setedittext(dlg, namebuf);
						continue;
					}
				}
			}
			DisposDialog(dlg);
			return (item == OKITEM) ? namebuf : NULL;
		}
	}
}

Visible char *asklocation(namebuf) char *namebuf;
{
	DialogPtr dlg= GetNewDialog(LOCATIONDLOG, (Ptr)NULL, (WindowPtr)(-1));
	short item;
	
	setedittext(dlg, namebuf);
	set_arrow();
	for (;;) {
		ModalDialog((ProcPtr)NULL, &item);
		if (item == OKITEM || item == CANCELITEM) {
			if (item == OKITEM) {
				getedittext(dlg, namebuf);
				if (!goodtag(namebuf)) {
					macerrS(LOCNAME, namebuf);
					setedittext(dlg, namebuf);
					continue;
				}
				if (!existinglocation(namebuf)) {
					macerrS(LOCINIT, namebuf);
					setedittext(dlg, namebuf);
					continue;
				}
			}
			DisposDialog(dlg);
			return (item == OKITEM) ? namebuf : NULL;
		}
	}
}

Visible char *askhowto(namebuf) char *namebuf;
{
	DialogPtr dlg= GetNewDialog(HOWTODLOG, (Ptr)NULL, (WindowPtr)(-1));
	short item;
	
	setedittext(dlg, namebuf);
	set_arrow();
	for (;;) {
		ModalDialog((ProcPtr)NULL, &item);
		if (item == OKITEM || item == CANCELITEM) {
			if (item == OKITEM) {
				getedittext(dlg, namebuf);
				if (!goodhowto(namebuf)) {
					macerrS(HOWNAME, namebuf);
					setedittext(dlg, namebuf);
					continue;
				}
				if (!existinghowto(namebuf)) {
					macerrS(HOWINIT, namebuf);
					setedittext(dlg, namebuf);
					continue;
				}
			}
			DisposDialog(dlg);
			return (item == OKITEM) ? namebuf : NULL;
		}
	}
}

Hidden Procedure getedittext(dlg, buf) DialogPtr dlg; char *buf; {
	Handle ih;
	short type;
	Rect box;
	
	GetDItem(dlg, EDITITEM, &type, &ih, &box);
	GetIText(ih, buf);
}

Hidden Procedure setedittext(dlg, buf) DialogPtr dlg; char *buf; {
	Handle ih;
	short type;
	Rect box;
	
	GetDItem(dlg, EDITITEM, &type, &ih, &box);
	SetIText(ih, buf);
	SelIText(dlg, EDITITEM, 0, 32000);
}

Hidden bool oktoreplace(name) char *name; {
	ParamText(name, (char*)NULL, (char*)NULL, (char*)NULL);
	return CautionAlert(OKREPLACEALERT, (ProcPtr)NULL) == OKITEM;
}
