/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1986. */

#include "b.h"
#include "getc.h"
#include "oper.h"

/* struct tabent {int code; int deflen; string def, rep, name;} in getc.h */

/* Table of key definitions, filled by the following defaults
   and by reading definitions from a file.

   For the code field the following holds:
   code > 0:
       command definitions,
       new defs will be added, eliminating conflicting ones
   code < 0:
       strings to be send to the terminal,
       any new defs overwrite the old ones

   On the Atari, 'extended codes' are by convention a null character
   followed by another character (usually the scan code).

   E.g., F1 is 0-3B (hexadecimal), which we encode as \0\073 (since
   \073 is octal for 3B hexadecimal).  
   For the exact codes, see for instance the Mark Williams C Manual entry
   for `Keyboard'.

   ==== All values in the table are given in octal ====
*/

Visible struct tabent deftab[MAXDEFS] = {
	{IGNORE,	0,	NULL,		NULL,		S_IGNORE},
		/* Entry to ignore a key */
	{WIDEN, 	2,	"\0\073",	"F1",		S_WIDEN},
	{EXTEND,	2,	"\0\074",	"F2",		S_EXTEND},
	{FIRST,		2,	"\0\075",	"F3",		S_FIRST},
	{LAST,		2,	"\0\076",	"F4",		S_LAST},
	{PREVIOUS,	2,	"\0\077",	"F5",		S_PREVIOUS},
	{NEXT,		2,	"\0\100",	"F6",		S_NEXT},
	{UPLINE,	2,	"\0\122",	"Insert",	S_UPLINE},
	{DOWNLINE,	2,	"\0\107",	"Clr Home",	S_DOWNLINE},
	{UPARROW,	2,	"\0\110",	"^",		S_UPARROW},
	{DOWNARROW,	2,	"\0\120",	"v",		S_DOWNARROW},
	{LEFTARROW,	2,	"\0\113",	"<-",		S_LEFTARROW},
	{RITEARROW,	2,	"\0\115",	"->",		S_RITEARROW},
	{ACCEPT,	0,	"\011",		"Tab",		S_ACCEPT},
	{NEWLINE,	0,	"\012",		"Linefeed",	S_NEWLINE},
	{NEWLINE,	0,	"\015",		"Return",	S_NEWLINE},
	{UNDO,		0,	"\010",		"Backspace",	S_UNDO},
	{UNDO,		2,	"\0\141",	"Undo",		S_UNDO},
	{REDO,		0,	"\022",		"Control-R",	S_REDO},
	{COPY,		2,	"\0\103",	"F9",		S_COPY},
	{DELETE,	0,	"\177",		"Delete",	S_DELETE},

#ifdef RECORDING
	/*
	 * As on the IBM PC the RECORD and PLAYBACK keys
	 * are mapped on the function keys F7-F8
	 */
	{RECORD,	2,	"\0\101",	"F7",		S_RECORD},
	{PLAYBACK,	2,	"\0\102",	"F8",		S_PLAYBACK},
#endif /* RECORDING */

	{REDRAW,	0,	"\014",	"Control-L",		S_LOOK},

#ifdef HELPFUL
	{HELP,		2,	"\0\142",	"Help",		S_HELP},
#endif /* HELPFUL */

	{GOTO,		0,	"\007",		"Control-G",	S_GOTO},
	{MOUSE,		2,	"\0\164",	"L-Click",	S_MOUSE},
	{MOUSE,		2,	"\0\165",	"R-Click",	S_MOUSE},
	{EXIT,		0,	"\033\033",	"Esc Esc",	S_EXIT},
	{CANCEL,	0,	"\003",		"Control-C",	S_INTERRUPT},
	{SUSPEND,	0,	NULL,		NULL,		S_SUSPEND},

/* string-valued: */
	{TERMINIT,	0,	"\033v",	"ESC v",	S_TERMINIT},
	{TERMDONE,	0,	"",		NULL,		S_TERMDONE},
	{MFORMAT,	0,	"",		NULL,		S_MFORMAT},

	{0,		0,	NULL,		NULL,		NULL}
};

Visible Procedure addspeckeys() {
}

#ifdef KEYS

Hidden string extcode[]= {
	NULL,			/*   0 (0x00) */
	NULL,			/*   1 (0x01) */
	NULL,			/*   2 (0x02) */
	"Ctrl-2",		/*   3 (0x03) */
	NULL,			/*   4 (0x04) */
	NULL,			/*   5 (0x05) */
	NULL,			/*   6 (0x06) */
	NULL,			/*   7 (0x07) */
	NULL,			/*   8 (0x08) */
	NULL,			/*   9 (0x09) */
	NULL,			/*  10 (0x0A  */
	NULL,			/*  11 (0x0B) */
	NULL,			/*  12 (0x0C) */
	NULL,			/*  13 (0x0D) */
	NULL,			/*  14 (0x0E) */
	NULL,              	/*  15 (ox0F) */
	"Alt-Q",		/*  16 (0x10) */
	"Alt-W",		/*  17 (0x11) */
	"Alt-E",		/*  18 (0x12) */
	"Alt-R",		/*  19 (0x13) */
	"Alt-T",		/*  20 (0x14) */
	"Alt-Y",		/*  21 (0x15) */
	"Alt-U",		/*  22 (0x16) */
	"Alt-I",		/*  23 (0x17) */
	"Alt-O",		/*  24 (0x18) */
	"Alt-P",                /*  25 (0x19) */
	NULL,			/*  26 (0x1A) */
	NULL,			/*  27 (0x1B) */
	NULL,			/*  28 (0x1C) */
	NULL,			/*  29 (0x1D) */
	"Alt-A",                /*  30 (0x1E) */
	"Alt-S",		/*  31 (0x1F) */
	"Alt-D",		/*  32 (0x20) */
	"Alt-F",		/*  33 (0x21) */
	"Alt-G",		/*  34 (0x22) */
	"Alt-H",		/*  35 (0x23) */
	"Alt-J",		/*  36 (0x24) */
	"Alt-K",		/*  37 (0x25) */
	"Alt-L",                /*  38 (0x26) */
	NULL,			/*  39 (0x27) */
	NULL,			/*  40 (0x28) */
	"Ctrl-\\",		/*  41 (0x29) */
	NULL,			/*  42 (0x2A) */
	NULL,			/*  43 (0x2B) */
	"Alt-Z",                /*  44 (0x2C) */
	"Alt-X",		/*  45 (0x2D) */
	"Alt-C",		/*  46 (0x2E) */
	"Alt-V",		/*  47 (0x2F) */
	"Alt-B",		/*  48 (0x30) */
	"Alt-N",		/*  49 (0x31) */
	"Alt-M",                /*  50 (0x32) */
	NULL,			/*  51 (0x33) */
	NULL,			/*  52 (0x34) */
	NULL,			/*  53 (0x35) */
	NULL,			/*  54 (0x36) */
	NULL,			/*  55 (0x37) */
	NULL,			/*  56 (0x38) */
	"Ctrl-Space",		/*  57 (0x39) */
	NULL,			/*  58 (0x3A) */
	"F1",			/*  59 (0x3B) */
	"F2",			/*  60 (0x3C) */
	"F3",			/*  61 (0x3D) */
	"F4",			/*  62 (0x3E) */
	"F5",			/*  63 (0x3F) */
	"F6",			/*  64 (0x40) */
	"F7",			/*  65 (0x41) */
	"F8",			/*  66 (0x42) */
	"F9",			/*  67 (0x43) */
	"F10",                  /*  68 (0x44) */
	NULL,			/*  69 (0x45) */
	NULL,			/*  70 (0x46) */
	"Clr Home",		/*  71 (0x47) */
	"^",			/*  72 (0x48) */
	NULL,			/*  73 (0x49) */
	NULL,			/*  74 (0x4A) */
	"<-",			/*  75 (0x4B) */
	NULL,			/*  76 (0x4C) */
	"->",			/*  77 (0x4D) */
	NULL,			/*  78 (0x4E) */
	NULL,			/*  79 (0x4F) */
	"v",	         	/*  80 (0x50) */
	NULL,			/*  81 (0x51) */
	"Insert",		/*  82 (0x52) */
	NULL,			/*  83 (0x53) */
	"Shift-F1",             /*  84 (0x54) */
	"Shift-F2",		/*  85 (0x55) */
	"Shift-F3",		/*  86 (0x56) */
	"Shift-F4",		/*  87 (0x57) */
	"Shift-F5",		/*  88 (0x58) */
	"Shift-F6",		/*  89 (0x59) */
	"Shift-F7",		/*  90 (0x5A) */
	"Shift-F8",		/*  91 (0x5B) */
	"Shift-F9",		/*  92 (0x5C) */
	"Shift-F10",		/*  93 (0x5D) */
	NULL,              	/*  94 (0x5E) */
	NULL,			/*  95 (0x5F) */
	NULL,			/*  96 (0x60) */
	"Undo",			/*  97 (0x61) */
	"Help",			/*  98 (0x62) */
	NULL,			/*  99 (0x63) */
	NULL,			/* 100 (0x64) */
	NULL,			/* 101 (0x65) */
	NULL,			/* 102 (0x66) */
	NULL,			/* 103 (0x67) */
	NULL,               	/* 104 (0x68) */
	NULL,			/* 105 (0x69) */
	NULL,			/* 106 (0x6A) */
	NULL,			/* 107 (0x6B) */
	NULL,			/* 108 (0x6C) */
	NULL,			/* 109 (0x6D) */
	NULL,			/* 110 (0x6E) */
	NULL,			/* 111 (0x6F) */
	NULL,			/* 112 (0x70) */
	NULL,			/* 113 (0x71) */
	NULL,           	/* 114 (0x72) */
	NULL,			/* 115 (0x73) */
	"L-Click",		/* 116 (0x74) */
	"R-Click",		/* 117 (0x75) */
	NULL,			/* 118 (0x76) */
	NULL,			/* 119 (0x77) */
	"Alt-1",                /* 120 (0x78) */
	"Alt-2",		/* 121 (0x79) */
	"Alt-3",		/* 122 (0x7A) */
	"Alt-4",		/* 123 (0x7B) */
	"Alt-5",		/* 124 (0x7C) */
	"Alt-6",		/* 125 (0x7D) */
	"Alt-7",		/* 126 (0x7E) */
	"Alt-8",		/* 127 (0x7F) */
	"Alt-9",		/* 128 (0x80) */
	"Alt-0",		/* 129 (0x81) */
	"Alt--",		/* 130 (0x82) */
	"Alt-=",                /* 131 (0x83) */
	NULL			/* 132 (0x84) */
};

/* assumption: extended keys are handled via two consecutive calls */

Visible string reprchar(c) int c; {
	static char str[20];
	static bool extended= No;
	

	c= c&0377;

	if (extended == Yes) {
		extended = No;
		if (c >= 0 && c < sizeof(extcode) && extcode[c] != NULL)
			return extcode[c];
		else {
			sprintf(str, "Ext-\\%03o", c);
			return str;
		}
	}
	else if (c == '\000') { 			/* Ext. char */
		extended = Yes;
		return "";
	}
	else if ('\000' < c && c < '\040') {		/* Control char? */
		switch (c) {
			case '\010':
				return "Backspace";
			case '\011':
				return "Tab";
			case '\012':
				return "Linefeed";
			case '\015':
				return "Return";
			case '\033':
				return "Esc";
			default:
				sprintf(str, "Control-%c", c|0100);
				return str;
		}
	}
	else if (c == '\040') {				/* Space? */
		return "Space";
	}
	else if ('\041' <= c && c < '\177') {		/* Printable? */
		str[0]= c; str[1]= '\0';
		return str;
	}
	else if ( c == '\177') {			/* Delete? */
		return "Delete";
	}
	else {
		sprintf(str, "\\%03o", c);		/* octal value */
		return str;
	}
}

#endif /* KEYS */
