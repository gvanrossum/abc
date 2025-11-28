/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */

/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
/* Figure out type (BIOS or ANSI) and size of the screen.		    */
/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */

#include <stdio.h>
#include "dos.h"
#include "ctype.h"

#ifdef lint
#define VOID (void)
#else
#define VOID
#endif

#define Forward
#define Visible
#define Hidden static
#define Procedure

#define Yes 1
#define No 0

#define V_NORMAL 7		/* BIOS video attribute */
#define A_ED	"\033[2J"       /* ANSI erase display */
#define A_CUP	"\033[%d;%dH"   /* cursor position */

#define BIOS 0
#define ANSI 1
char *typename[] = {
	"BIOS",
	"ANSI"
};

int lines;
int cols;
int bcols;			/* what BIOS thinks */
int type;

main(argc, argv)
int argc;
char *argv[];
{
	type = getarg(argc, argv);

	explain();

	lines = ask_lines();
	cols = ask_cols();

	try_type();

	tell_screen();
}

Hidden int getarg(argc, argv)
int argc;
char *argv[];
{
	if (argc == 1)
		return BIOS;

	switch (*(argv[1])) {
	case 'a':
	case 'A':
		return ANSI;
	case 'b':
	case 'B':
		return BIOS;
	default:
		argerr();
		/* NOTREACHED */
	}
}

Hidden Procedure argerr() {
	cprintf("*** Wrong parameter.\n\r");
	cprintf("*** Use \"SCREEN BIOS\" or \"SCREEN ANSI\".\n\r");

	exit(1);
}

Hidden Procedure explain() {
	cputs("\n\r\
This program lets you find out how\n\r\
your computer writes to the screen,\n\r\
and how big the screen is.\n\r\
At the end it should tell you how\n\r\
to inform the ABC system about this.\n\r");
	cprintf("\n\r\
We will test whether your machine\n\r\
is %s compatible.\n\r", typename[type]);
	if (type == BIOS) {
		cputs("\
If your machine is not, this might\n\r\
cause the program to crash without\n\r\
giving an apropriate message.\n\r\
When this happens, you should\n\r\
try \"SCREEN ANSI\".\n\r");
	}
	else {	/* type == ANSI */
		cputs("\n\r\
Since BIOS is better we hope you have\n\r\
tried this program without a parameter.\n\r");
	}
	get_return();
}

int ask_lines() {
	int l;

	cputs("\n\r\
To find out how many lines there are\n\r\
on your screen, we will generate a\n\r\
test pattern.\n\r\
After the question mark at the end of\n\r\
the pattern you should enter the\n\r\
number that appears at the top of the\n\r\
screen.\n\r");
	get_return();

	for (l = 100; l > 1; --l) {
		cprintf("%d\n\r", l);
	}
	cprintf("1  number at top ?");
	l = getnum();
	return l;
}

int ask_cols() {
	int c;

	cputs("\n\r\
The following pattern should help you\n\r\
to count the number of columns\n\r\
on your screen\n\r");

	cprintf("\
....*....1....*....2....*....3....*....4....*....5....*....6....*....7....*....8\
....*....9....*....0....*....1....*....2....*....3....*....4....*....5....*....6\n\r");

	cprintf("How many columns do you count? ");
	c = getnum();

	if (type == BIOS) {
		bcols = bios_cols();
		if (c != bcols) {
			cprintf("BIOS thinks there are %d columns.\r\n", bcols);
			cprintf("Are you sure you counted %d yourself? ", c);
			if (get_yes() == No) {
				return ask_cols();
			}
		}
	}

	return c;
}

Hidden Procedure get_return() {
	char line[BUFSIZ];
	int len;

	cprintf("\n\rPress the [RETURN] key to continue: ");
	len = getline(line, BUFSIZ);
}

Hidden int get_yes() {
	char line[BUFSIZ];
	int len;

	len = getline(line, BUFSIZ);
	if (tolower(line[0]) == 'y') {
		return Yes;
	}
	else {
		return No;
	}
}

Hidden int getnum()
{
	int num;
	char line[BUFSIZ];
	int len;

	len = getline(line, BUFSIZ);
	if (len == 0)
		exit(-1);
	num = atoi(line);
	if (num <= 0) {
		cprintf("\n\rinvalid number, try again: ");
		return getnum();
	}
	else
		return num;
}

int getline(buf, lim)		/* get line into buf of length lim */
char buf[];			/* return length, including '\n' */
int lim;			/*  length==0: EOF */
{				/*  length==lim && buf[lim]!='\n': overflow */
	register char *s;
	register int c;

	s = buf;
	while (--lim > 0 && (c = getchar()) != EOF && c != '\n') {
		*s++ = c;
	}

	if (c == '\n')
		*s++ = c;

	*s = '\0';

	return s - buf;
}

Hidden Procedure try_type() {
	cprintf("\n\rWe will now try whether your screen is\n\r");
	cprintf("compatible with %s.\n\n\r", typename[type]);
	get_return();
	home_clear();
	cputs("\
Does this question appear at the top\n\r\
of an otherwise empty screen? ");
	if (get_yes() == No) {
		cprintf("\n\r\
Your machine appears not te be\n\r\
compatible with %s.\n\r", typename[type]);
		switch (type) {
		case BIOS:
			cputs("\n\r\Try \"SCREEN ANSI\".\n\r");
			break;
		case ANSI:
			cputs("\n\r\
Try to put the command\n\n\r\
    DEVICE=ANSI.SYS\n\n\r\
in your CONFIG.SYS file,\n\r\
restart DOS,\n\r\
and try \"SCREEN ANSI\" again.\n\r");
			break;
		}
		exit(1);
	}
}

/*
 * BIOS video io is called by generating an 8086 software interrupt,
 * using lattice's int86() function.
 * To ease coding, all routines fill in the apropriate parameters in regs,
 * and than call bios10(code), where code is to be placed in ah.
 */

Hidden union REGS regs;

Hidden bios10(code)
int code;
{
	regs.h.ah = code;
	int86(0x10, &regs, &regs);
}

Hidden int bios_cols()
{
	bios10(15);
	return regs.h.ah;
}

home_clear() {
	switch (type) {
	case BIOS:
		/* clear the screen */
		regs.h.al = 0;	/* scroll with al = 0 means blank window */
		regs.h.ch = 0;	/* start line */
		regs.h.cl = 0;	/* start col */
		regs.h.dh = lines-1; /* end line */
		regs.h.dl = cols-1;  /* end col */
		regs.h.bh = V_NORMAL;/* video attribute */
		bios10(6);
		/* move to the top */
		regs.h.dh = 0;	/* line */
		regs.h.dl = 0;	/* col */
		regs.h.bh = 0;	/* page */
		bios10(2);
		break;
	case ANSI:
		cprintf(A_CUP, 1, 1);
		cputs(A_ED);
		break;
	}
}

Hidden Procedure tell_screen() {

	if (type == BIOS && lines == 25 && cols == bcols) {
		cputs("\n\r\
You do not need to set the environment\n\r\
variable SCREEN. The ABC system will\n\r\
fill in the defaults itself.\r\n");
	}
	else {
		cputs("\n\r\
You must set the environment\n\r\
variable SCREEN; enter:\r\n\n");
		cprintf("     SET SCREEN=%s %d %d\r\n", typename[type], lines, cols);
		cputs("\n\
to tell the ABC system what type of\n\r\
screen you have.\r\n\
It is best to put this command in your\n\r\
AUTOEXEC.BAT file and restart DOS.\n\r");
	}
}
