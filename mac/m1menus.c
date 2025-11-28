/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1988. */

/* handle menus */

#include "b.h"
#include "keys.h"

#include <Types.h>
#include <Menus.h>
#include <Dialogs.h>
#include "abcmenus.h"

/* the Handles: */

Hidden MenuHandle mh[NMENUS];

/* enabled status of entire menus: */

#define On 1
#define Off 0

Hidden char abcmenus[][NMENUS]= {
	{On, On, On, On, On, On, On},			/* Prompt_menus */
	{On, Off, On, On, On, On, On},			/* Editor_menus */
	{Off, Off, Off, Off, Off, On, Off},		/* Interpreter_menus */
	{On, Off, On, Off, Off, Off, Off}		/* DA_menus */
};

Hidden char cur_menus[NMENUS]= {On, On, On, On, On, On, On};

Visible curmenusstat= 0;

/* Set up the menus and display the menu bar: */

#define DA_S 'DRVR'

Visible Procedure setup_menus() {
	int m;

	for (m =  0; m < NMENUS; m++) {
		mh[m]= GetMenu(AppleID+m);
		if (m == 0) {
			DisableItem(mh[m], 2);
			AddResMenu(mh[m], DA_S);
		}
		InsertMenu(mh[m], 0);
	}
	DrawMenuBar();
}

Hidden Procedure adjust_menus(menusstatus) int menusstatus; {
	char *menus;
	int m;
	bool draw= No;
	
	if (menusstatus == curmenusstat)
		return;
	menus= abcmenus[menusstatus];
	for (m = 0; m < NMENUS; m++) {
		if (cur_menus[m] != menus[m]) {
			if (menus[m] == On)
				EnableItem(mh[m], 0);
			else
				DisableItem(mh[m], 0);
			cur_menus[m]= menus[m];
			draw= Yes;
		}
	}
	if (draw)
		DrawMenuBar();
	curmenusstat= menusstatus;
}

Visible Procedure editor_menus(at_cmdprompt) bool at_cmdprompt; {
	if (at_cmdprompt)
		adjust_menus(Prompt_menus);
	else
		adjust_menus(Editor_menus);
}

Visible Procedure intrprtr_menus() {
	adjust_menus(Interpreter_menus);
}

Visible Procedure disable_grrec() {
	DisableItem(mh[FileID-AppleID], RecWsGroupItem);
}

Visible Procedure adjusteditmenu(athole, buffer, unpasted, undo, redo)
bool athole, buffer, unpasted, undo, redo;
{
	MenuHandle em= mh[EditID-AppleID];
	
	if (athole || unpasted) {
		DisableItem(em, CutItem);
		EnableItem(em, ClearItem);
	}
	else {
		EnableItem(em, CutItem);
		DisableItem(em, ClearItem);
	}
	if (athole)
		DisableItem(em, CopyItem);
	else
		EnableItem(em, CopyItem);
	if (athole && buffer)
		EnableItem(em, PasteItem);
	else
		DisableItem(em, PasteItem);
	if (undo)
		EnableItem(em, UndoItem);
	else
		DisableItem(em, UndoItem);
	if (redo)
		EnableItem(em, RedoItem);
	else
		DisableItem(em, RedoItem);
}

Hidden int abcmenusstat= Prompt_menus;
Hidden unsigned long emflags= 0xffffffff;

Visible Procedure da_menus() {
	MenuHandle em= mh[EditID-AppleID];
	
	emflags= (*em)->enableFlags;
	EnableItem(em, UndoItem);
	DisableItem(em, RedoItem);
	EnableItem(em, CutItem);
	EnableItem(em, CopyItem);
	EnableItem(em, PasteItem);
	EnableItem(em, ClearItem);
	abcmenusstat= curmenusstat;
	adjust_menus(DA_menus);	
}

Visible Procedure unda_menus() {
	MenuHandle em= mh[EditID-AppleID];
	int i;
	
	for (i=UndoItem; i <= ClearItem; i++) {
		if ((emflags>>i) & 0x1)
			EnableItem(em, i);
		else
			DisableItem(em, i);
	}
	adjust_menus(abcmenusstat);
}

/* handle menu choice: */

Visible int do_menu_choice(menuchoice) int menuchoice; {
	int id= ID(menuchoice);
	int item= ITEM(menuchoice);
	int result= HANDLED;
	
	if (menuchoice != 0) {
		
		switch (id) {
		case AppleID:
			result= do_apple_choice(item);
			break;
		case FileID:
			result= do_file_choice(item);
			break;
		case EditID:
			result= do_edit_choice(item);
			break;
		case FocusID:
			result= do_focus_choice(item);
			break;
		case SpecialID:
			result= do_special_choice(item);
			break;
		case PauseID:
			result= do_pause_choice(item);
			break;
		case HelpID:
			result= do_help_choice(item);
			break;
		}
	}
	return result;
}

Hidden int do_apple_choice(item) int item; {
	char acc_name[256];
	int acc_number;
	
	if (item == AboutItem) {
		do_about();
	}
	else {
		GetItem(mh[0], item, acc_name);
		acc_number= OpenDeskAcc(acc_name);
	}
	return HANDLED;
}

Hidden int do_file_choice(item) int item; {
	if (item != QuitItem) {
		adjust_menus(Interpreter_menus);
		clear_perm();	/* to file new locations and free memory */
		if (item != UnpackTEXT2WsItem)
			mess_ok= No;
	}
	switch (item) {
	case TEXT2TableItem:
		mac_input();
		break;
	case Table2TEXTItem:
		mac_output();
		break;
	case Howtos2TEXTItem:
		mac_list();
		break;
	case PackWs2TEXTItem:
		mac_pack();
		break;
	case UnpackTEXT2WsItem:
		mac_unpack();
		return REDRAW; /* hak, hak, to get prompt right */
	case PageSetupItem:
		do_pagesetup();
		break;
	case PrLocationItem:
		pr_location();
		break;
	case PrHowtoItem:
		pr_howto();
		break;
	case PrWsHowtosItem:
		pr_workspace();
		break;
	case RecCurWsItem:
		rec_workspace();
		break;
	case RecWsGroupItem:
		rec_wsgroup();
		break;
	case QuitItem:
		terminated= Yes;
		return EXIT;
	}
	
	adjust_menus(Prompt_menus);
	mess_ok= Yes;
	still_ok= Yes;
	interrupted= No;

	return HANDLED;
}

Hidden int do_edit_choice(item) int item; {
	/* SystemEdit done in trminput() in m1trm.c */
	return EDIT+item;
}

Hidden int do_focus_choice(item) int item; {
	return FOCUS+item;	/* FOCUS in keys.h */
}

Hidden int do_special_choice(item) int item; {
	return SPECIAL+item;
}

Hidden int do_pause_choice(item) int item; {
	return CANCEL;
}

Hidden int do_help_choice(item) int item; {
	sethelptopic(item);
	return HELP;	/* causes edit_document to call help() */
}

#ifdef UNUSED
#define NOTYET MESS(9000, "Sorry, this is still under construction")

Visible Procedure notyet() {
	set_arrow();
	macerr(NOTYET);
}
#endif
