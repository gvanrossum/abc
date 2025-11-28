/* Alert and dialog resources. */

#include "rez.h"

/*
 * macerr() alert:
 *
 *     + DH + TS-DH +        TH                     + DH +
 * DV  |                                                 |
 * TV  |    + ***   + MacABC encountered an error:  +    |
 * DV  |                                                 |
 * TV  |            + param text for this particular+    |
 * TV  |            + message                       +    |
 * DV  |                                                 |
 * OKV |                                    OKBUTTON     |
 * DV  |                                                 |
 *     + + + + + + + + + + + + + + + + + + + + + + + + + +
 */

#define DH  20	/* delta horizontal */
#define TS  50	/* text start (horizontal) */
#define TH 400	/* text width (horizontal) */
#define OKH 60	/* OK button horizontal */

#define DV  10	/* delta vertical */
#define TV  16	/* text vertical */
#define OKV 20	/* OK button vertical */

#define ERRWIDTH	TS+TH+DH		/* 470 */
#define ERRHEIGHT	4*DV+3*TV+OKV	/* 108 */

#define OKPOS	 {ERRHEIGHT-OKV-DV, ERRWIDTH-OKH-DH, ERRHEIGHT-DV, ERRWIDTH-DH}
#define STARPOS	 {DV, DH, DV+TV, TS}
#define FIXEDPOS {DV, TS, DV+TV, ERRWIDTH-DH}
#define PARAMPOS {2*DV+TV, TS, 2*DV+3*TV, ERRWIDTH-DH}

/* Center in upper third of Mac screen (342x512): */
#define LEFT	21 /* (512-ERRWIDTH)/2; seems to hard for Rez :-[ */
#define TOP		91 /* 20+((342-20-ERRHEIGHT)/3) */
#define MACERRPOS {TOP, LEFT, TOP+ERRHEIGHT, LEFT+ERRWIDTH}

resource 'ALRT' (MACERRALERT, preload) {
	MACERRPOS,
	MACERRALERT,
	{
		OK, visible, 1,
		OK, visible, 1,
		OK, visible, 1,
		OK, visible, 1
	}
};

resource 'DITL' (MACERRALERT, purgeable, preload) {
	 {	/* array DITLarray: 2 elements */
		/* [1] */
		OKPOS,
		Button {
			enabled,
			"OK"
		};
		/* [2] */
		STARPOS,
		StaticText {
			disabled,
			"*** "
		};
		/* [3] */
		FIXEDPOS,
		StaticText {
			disabled,
			"MacABC encountered an error:"
		};
		/* [4] */
		PARAMPOS,
		StaticText {
			disabled,
			"^0"
		}
	}
};

/* Memexh alert. (PRELOAD, NOT PURGABLE!) */

/* (These coordinates are copied from alerts in the Finder!) */

#define ALERTPOS	{80, 64, 188, 448}
#define BUTTON1POS	{78, 20, 98, 80}
#define BUTTON2POS	{78, 100, 98, 160}
#define STATTEXTPOS	{10, 110, 74, 374}

resource 'ALRT' (MEMEXHALERT, nonpurgeable, preload) {
	ALERTPOS,
	MEMEXHALERT,
	{
		OK, visible, 1,
		OK, visible, 1,
		OK, visible, 1,
		OK, visible, 1
	}
};

resource 'DITL' (MEMEXHALERT, preload) {
	 {	/* array DITLarray: 2 elements */
		/* [1] */
		BUTTON1POS,
		Button {
			enabled,
			"Quit"
		};
		/* [2] */
		STATTEXTPOS,
		StaticText {
			disabled,
			"Sorry, memory exhausted."
		}
	}
};

/* Dialogs for "askperm.c". */

#define DLOGLEFT	116
#define DLOGTOP		82

#define DLOGWIDTH	266
#define DLOGHEIGHT	120

#define DLOGPOS	{DLOGTOP, DLOGLEFT, DLOGTOP+DLOGHEIGHT, DLOGLEFT+DLOGWIDTH}

#define OKPOS	{92, 24, 92+20, 24+60}
#define CANPOS	{92, DLOGWIDTH-24-60, 92+20, DLOGWIDTH-24}
#define EDITPOS	{60, 24, 60+16, DLOGWIDTH-24}
#define STATPOS	{8, 24, 8+3*16, DLOGWIDTH-24}

/* Dialog asking for new location name. */

resource 'DLOG' (NEWLOCDLOG, purgeable, preload) {
	DLOGPOS,
	dBoxProc,
	visible,
	noGoAway,
	0x0,
	NEWLOCDLOG,
	""
};

resource 'DITL' (NEWLOCDLOG, purgeable, preload) {
	 {	/* array DITLarray: 4 elements */
		/* [1] */
		OKPOS,
		Button {
			enabled,
			"OK"
		};
		/* [2] */
		CANPOS,
		Button {
			enabled,
			"Cancel"
		};
		/* [3] */
		EDITPOS,
		EditText {
			enabled,
			""
		};
		/* [4] */
		STATPOS,
		StaticText {
			disabled,
			"Enter new location name:"
		}
	}
};

/* Dialog asking for existing location name. */

resource 'DLOG' (LOCATIONDLOG, purgeable, preload) {
	DLOGPOS,
	dBoxProc,
	visible,
	noGoAway,
	0x0,
	LOCATIONDLOG,
	""
};

resource 'DITL' (LOCATIONDLOG, purgeable, preload) {
	 {	/* array DITLarray: 4 elements */
		/* [1] */
		OKPOS,
		Button {
			enabled,
			"OK"
		};
		/* [2] */
		CANPOS,
		Button {
			enabled,
			"Cancel"
		};
		/* [3] */
		EDITPOS,
		EditText {
			enabled,
			""
		};
		/* [4] */
		STATPOS,
		StaticText {
			disabled,
			"Enter an existing location's name:"
		}
	}
};

/* Dialog asking for existing how-to name. */

resource 'DLOG' (HOWTODLOG, purgeable, preload) {
	DLOGPOS,
	dBoxProc,
	visible,
	noGoAway,
	0x0,
	HOWTODLOG,
	""
};

resource 'DITL' (HOWTODLOG, purgeable, preload) {
	 {	/* array DITLarray: 4 elements */
		/* [1] */
		OKPOS,
		Button {
			enabled,
			"OK"
		};
		/* [2] */
		CANPOS,
		Button {
			enabled,
			"Cancel"
		};
		/* [3] */
		EDITPOS,
		EditText {
			enabled,
			""
		};
		/* [4] */
		STATPOS,
		StaticText {
			disabled,
			"Enter the name of an existing how-to:"
		}
	}
};

/* Alert for overwrite existing location. */

resource 'ALRT' (OKREPLACEALERT, preload) {
	ALERTPOS,
	OKREPLACEALERT,
	{
		Cancel, visible, 1,
		Cancel, visible, 1,
		Cancel, visible, 1,
		Cancel, visible, 1
	}
};

resource 'DITL' (OKREPLACEALERT, preload) {
	 {	/* array DITLarray: 3 elements */
		/* [1] */
		BUTTON1POS,
		Button {
			enabled,
			"Yes"
		};
		/* [2] */
		BUTTON2POS,
		Button {
			enabled,
			"No"
		};
		/* [3] */
		STATTEXTPOS,
		StaticText {
			disabled,
			"OK to replace existing location \"^0\" ?"
		}
	}
};

/* for m1print.c: tell user what is going on, and howto cancel. */

resource 'DLOG' (PRINTDLOG, purgeable, preload) {
	{100, 144, 164, 368},
	dBoxProc,
	visible,
	noGoAway,
	0x0,
	PRINTDLOG,
	""
};

resource 'DITL' (PRINTDLOG, purgeable) {
	{	/* array DITLarray: 2 elements */
		/* [1] */
		{8, 10, 32, 216},
		StaticText {
			disabled,
			"Printing in progress"
		};
		/* [2] */
		{36, 10, 60, 216},
		StaticText {
			disabled,
			"Use \021-. to Cancel"
		}
	}
};

resource 'ALRT' (PRABORTALERT, purgeable, preload) {
	ALERTPOS,
	PRABORTALERT,
	{
		OK, visible, 1,
		OK, visible, 1,
		OK, visible, 1,
		OK, visible, 1
	}
};

resource 'DITL' (PRABORTALERT, preload) {
	 {	/* array DITLarray: 2 elements */
		/* [1] */
		BUTTON1POS,
		Button {
			enabled,
			"OK"
		};
		/* [2] */
		STATTEXTPOS,
		StaticText {
			disabled,
			"Printing process interrupted"
		}
	}
};
