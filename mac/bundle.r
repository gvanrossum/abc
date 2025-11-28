/* 'Bundle' resources for the Finder, and miscellaneous small
   configuration resources. */

/* Signature. */

#include "release.h"

type 'Mabc' as 'STR ';

resource 'Mabc' (0) {
	$$FORMAT("MacABC Release %s, %d-%02d-%d",
		RELEASE, $$Day, $$Month, $$Year)
};

/* Bundle (contains references to ICN# and FREF resources. */

resource 'BNDL' (128) {
	'Mabc',
	0,
	 {	/* array TypeArray: 2 elements */
		/* [1] */
		'ICN#',
		 {	/* array IDArray: 2 elements */
			/* [1] */
			0,
			128;
			/* [2] */
			1,
			129
		};
		/* [2] */
		'FREF',
		 {	/* array IDArray: 2 elements */
			/* [1] */
			0,
			128;
			/* [2] */
			1,
			129
		}
	}
};

resource 'FREF' (128) {
	'APPL',
	0,
	""
};

resource 'FREF' (129) {
	'TEXT',
	1,
	""
};

/* Multifinder and Switcher configuration. */

resource 'SIZE' (-1) {
	saveScreen,
	ignoreSuspendResumeEvents,
	dontDoOwnActivate,
 	524288, /* 512K */
	524288  /* 512K */
};

/* Configuration resource for VTRM (term.c). */

type 'Conf' {
	integer;
	integer;
	integer;
	integer;
	integer;
	integer;
	string[32];
};

resource 'Conf' (0) {
	4, 9,		/* font(4 = Monaco), point size */
	22, 10,		/* font(22 = courier) and size for printing */
	504, 294,	/* window size in pixels horizontal, vertical */
	"The ABC Programming Language"	/* Window title */
};

/* ResEdit template for above Conf resource.
   If you paste this TMPL resource from MacABC into ResEdit,
   you can conveniently edit the Conf resource with ResEdit. */

type 'TMPL' {
	wide array {
		pstring;
		string[4];
	};
};

resource 'TMPL' (5189, "Conf") {
	 {
		"ScreenFont Number", "DWRD";
		"ScreenFont Size", "DWRD";
		"PrintFont Number", "DWRD";
		"PrintFont Size", "DWRD";
		"Horizontal window size", "DWRD";
		"Vertical window size", "DWRD";
		"Window title", "CSTR"
	}
};

/* Icon and mask for the application. */

resource 'ICN#' (128) {
	{	/* array: 2 elements */
		/* [1] */
		$"0001 0000 0002 8000 0004 4000 0008 2000"
		$"0010 1000 0020 0800 0040 0400 0080 0A00"
		$"0100 1900 027E 3080 0402 6040 0802 0020"
		$"13F2 0010 2012 0008 4010 0004 9F90 0002"
		$"4090 0001 2090 0002 1080 0004 0880 0000"
		$"0480 8E1C 0200 8922 0101 4920 0081 4F20"
		$"0042 A8A0 0022 28A2 0012 2F1C 0008 0000"
		$"0004 1000 0002 2000 0001 4000 0000 80",
		/* [2] */
		$"0001 0000 0003 8000 0007 C000 000F E000"
		$"001F F000 003F F800 007F FC00 00FF FE00"
		$"01FF FF00 03FF FF80 07FF FFC0 0FFF FFE0"
		$"1FFF FFF0 3FFF FFF8 7FFF FFFC FFFF FFFE"
		$"7FFF FFFF 3FFF FFFE 1FFF FFFC 0FFF FFFF"
		$"07FF FFFF 03FF FFFF 01FF FFFF 00FF FFFF"
		$"007F FFFF 003F FFFF 001F FFFF 000F FFFF"
		$"0007 F000 0003 E000 0001 C000 0000 80"
	}
};

/* idem for ABC documents */

resource 'ICN#' (129) {
	{	/* array: 2 elements */
		/* [1] */
		$"0FFF FE00 0800 0300 0A22 0280 0911 0240"
		$"0888 8220 0844 4210 0888 83F8 0911 0008"
		$"0A22 3C08 0800 0008 0800 0008 0800 0008"
		$"0800 0008 0800 0008 0800 0008 0800 0008"
		$"0800 0008 0800 0008 0800 0008 0800 0008"
		$"0800 0008 0800 0008 0800 0008 0808 E1C8"
		$"0808 9228 0814 9208 0814 F208 082A 8A08"
		$"0822 8A28 0822 F1C8 0800 0008 0FFF FFF8",
		/* [2] */
		$"0FFF FE00 0FFF FF00 0FFF FF80 0FFF FFC0"
		$"0FFF FFE0 0FFF FFF0 0FFF FFF8 0FFF FFF8"
		$"0FFF FFF8 0FFF FFF8 0FFF FFF8 0FFF FFF8"
		$"0FFF FFF8 0FFF FFF8 0FFF FFF8 0FFF FFF8"
		$"0FFF FFF8 0FFF FFF8 0FFF FFF8 0FFF FFF8"
		$"0FFF FFF8 0FFF FFF8 0FFF FFF8 0FFF FFF8"
		$"0FFF FFF8 0FFF FFF8 0FFF FFF8 0FFF FFF8"
		$"0FFF FFF8 0FFF FFF8 0FFF FFF8 0FFF FFF8"
	}
};
