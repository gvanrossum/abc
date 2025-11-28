/* boolean arrays with /en/dis/able status of menus */

#define Prompt_menus		0
#define Editor_menus		1
#define Interpreter_menus	2
#define DA_menus		3

extern int curmenusstat;
int do_menu_choice();

/* Long menuchoices as returned by MenuKey() and MenuSelect()
 * are translated to (possibly shorter) int's for the interface
 * between trminput() and inchar()
 */
#define LID(lmc)	((lmc)>>16)
#define ITEM(mc)	((mc)&0xFF)
#define Menuchoice(id,it)	(((id)<<8)|(it))
#define ID(mc)		((mc)>>8)

/* ID's and Item's of ABC's menu's */

#define AppleID 1
#define		AboutItem 1

#define FileID 2
#define		TEXT2TableItem 1
#define		Table2TEXTItem 2
/* --- */
#define		Howtos2TEXTItem 4
/* --- */
#define		PackWs2TEXTItem 6
#define		UnpackTEXT2WsItem 7
/* --- */
#define		PageSetupItem 9
#define		PrLocationItem 10
#define		PrHowtoItem 11
#define		PrWsHowtosItem 12
/* --- */
#define		RecCurWsItem 14
#define		RecWsGroupItem 15
/* --- */
#define 	QuitItem 17

#define EditID 3
#define 	UndoItem 1
#define		RedoItem 2
#define		CutItem 3
#define		CopyItem 4
#define		PasteItem 5
#define		ClearItem 6

#define FocusID 4
#define		WidenItem 1
#define		ExtendItem 2
#define		FirstItem 3
#define		LastItem 4
#define		PreviousItem 5
#define		NextItem 6
/* --- */
#define		UplineItem 8
#define		DownlineItem 9
/* --- */
#define		UpItem 11
#define		DownItem 12
#define		LeftItem 13
#define		RightItem 14

#define SpecialID 5
#define		AcceptItem 1
#define		NewlineItem 2
/* --- */
#define		RecordItem 4
#define		PlaybackItem 5
/* --- */
#define		ExitItem 7

#define PauseID 6
#define		InterruptItem 1

#define HelpID 7
#define		NONMENUKEYS 3	/* disabled menu item; done for ? */
#define		StartupItem 1
#define		FileItem 2
/* --- ABC Editor --- */
#define		VisitItem 4
#define		EditItem 5
#define		FocusItem 6
#define		SpecialPauseItem 7
/* --- ABC Quick Ref --- */
#define		CommandsItem 9
#define		HowTosItem 10
#define		ExprAddrItem 11
#define		TestsItem 12
#define		NumberFuncsItem 13
#define		TrainFuncsItem 14
#define		MiscFuncsItem 15

#define NMENUS 7
