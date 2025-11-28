/* Apple menu */

resource 'MENU' (1, "Apple", preload) {
	1,
	textMenuProc,
	0x7FFFFFFF,
	enabled,
	apple,
	{
		"About MacABC ...", noIcon, noKey, noMark, plain;
		"-", noIcon, noKey, noMark, plain
	}
};

/* File menu. */

resource 'MENU' (2, "File", preload) {
	2,
	textMenuProc,
	0x7FFF6F6B, /* Enable flags */
	enabled,
	"File",
	{
		"Open TEXT file as table ...", noIcon, noKey, noMark, plain;
		"Save table as TEXT file ...", noIcon, noKey, noMark, plain;
		"-", noIcon, noKey, noMark, plain;
		"Save how-to's as TEXT file...", noIcon, noKey, noMark, plain;
		"-", noIcon, noKey, noMark, plain;
		"Pack workspace as TEXT file...", noIcon, noKey, noMark, plain;
		"Unpack workspace from TEXT file...", noIcon, noKey, noMark, plain;
		"-", noIcon, noKey, noMark, plain;
		"Page Setup ...", noIcon, noKey, noMark, plain;
		"Print location ...", noIcon, noKey, noMark, plain;
		"Print how-to ...", noIcon, noKey, noMark, plain;
		"Print workspace how-to's ...", noIcon, noKey, noMark, plain;
		"-", noIcon, noKey, noMark, plain;
		"Recover current workspace", noIcon, noKey, noMark, plain;
		"Recover workspace-group index", noIcon, noKey, noMark, plain;
		"-", noIcon, noKey, noMark, plain;
		"Quit", noIcon, "Q", noMark, plain
	}
};

resource 'MENU' (3, "Edit", preload) {
	3,
	textMenuProc,
	0x7FFFFFFF, /* Enable flags */
	enabled,
	"Edit",
	 {
		"Undo", noIcon, "Z", noMark, plain;
		"Redo", noIcon, "A", noMark, plain;
		"Cut", noIcon, "X", noMark, plain;
		"Copy", noIcon, "C", noMark, plain;
		"Paste", noIcon, "V", noMark, plain;
		"Clear", noIcon, nokey, noMark, plain
	}
};

resource 'MENU' (4, "Focus", preload) {
	4,
	textMenuProc,
	0x7FFFFDBF, /* Enable flags */
	enabled,
	"Focus",
	 {
		"Widen", noIcon, "W", noMark, plain;
		"Extend", noIcon, "E", noMark, plain;
		"First", noIcon, "F", noMark, plain;
		"Last", noIcon, "L", noMark, plain;
		"Previous", noIcon, "P", noMark, plain;
		"Next", noIcon, "N", noMark, plain;
		"-", noIcon, noKey, noMark, plain;
		"Upline", noIcon, "U", noMark, plain;
		"Downline", noIcon, "D", noMark, plain;
		"Arrow keys:", noIcon, noKey, noMark, plain;
		"Up", noIcon, noKey, noMark, plain;
		"Down", noIcon, noKey, noMark, plain;
		"Left", noIcon, noKey, noMark, plain;
		"Right", noIcon, noKey, noMark, plain
	}
};

resource 'MENU' (5, "Special", preload) {
	5,
	textMenuProc,
	0x7FFFFFDB, /* Enable flags */
	enabled,
	"Special",
	 {
		"Accept", noIcon, nokey, noMark, plain;
		"Newline", noIcon, noKey, noMark, plain;
		"-", noIcon, noKey, noMark, plain;
		"Record", noIcon, "R", noMark, plain;
		"Playback", noIcon, "T", noMark, plain;
		"-", noIcon, noKey, noMark, plain;
		"Exit", noIcon, noKey, noMark, plain
	}
};

resource 'MENU' (6, "Pause", preload) {
	6,
	textMenuProc,
	0x7FFFFFFF, /* Enable flags */
	enabled,
	"Pause",
	 {
		"Interrupt", noIcon, ".", noMark, plain
	}
};

resource 'MENU' (7, "Help", preload) {
	7,
	textMenuProc,
	0x7FFFFF7B, /* Enable flags */
	enabled,
	"Help",
	 {
		"Startup", noIcon, nokey, noMark, plain;
		"File", noIcon, nokey, noMark, plain;
		"ABC Editor:", noIcon, "?", noMark, plain;
		"Visit", noIcon, noKey, noMark, plain;
		"Edit", noIcon, noKey, noMark, plain;
		"Focus", noIcon, noKey, noMark, plain;
		"Special & Pause", noIcon, noKey, noMark, plain;
		"ABC Quick Ref:", noIcon, noKey, noMark, plain;
		"Commands", noIcon, noKey, noMark, plain;
		"How-to's", noIcon, noKey, noMark, plain;
		"Expr's & Addr's", noIcon, noKey, noMark, plain;
		"Tests", noIcon, noKey, noMark, plain;
		"Number Funcs", noIcon, noKey, noMark, plain;
		"Train Funcs", noIcon, noKey, noMark, plain;
		"Misc Funcs", noIcon, noKey, noMark, plain
	}
};
