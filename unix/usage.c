/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1990. */

#include "b.h"

#define USE00 MESS(7200, "\nUsage: abc [-g ws.group] [-w ws.name] [-c ws.name]\n")
#define USE01 MESS(7201, "   [ -e [files] | -i tab | -o tab | -l | -p | -u [files] | -r | -x | file ...]\n")
#define USE02 MESS(7202, "\nWorkspace Options:\n")
#define USE03 MESS(7203, "   -g dir       use group of workspaces in 'dir' (default $HOME/abc)\n")
#define USE04 MESS(7204, "   -w name      start in workspace 'name' (default: last workspace)\n")
#define USE05 MESS(7205, "   -w path      use 'path' as current workspace (no -g or -c 'name' allowed)\n")
#define USE06 MESS(7206, "   -c name      use 'name` as central workspace (default: $HOME/abc/abc)\n")
#define USE07 MESS(7207, "   -c path      use 'path' as central workspace\n")
#define USE08 MESS(7208, "Run Options:\n")
#define USE09 MESS(7209, "   -e [files]   Use ${EDITOR} as editor to edit definitions\n")
#define USE10 MESS(7210, "   file ...     Read commands from file(s) ('-' for standard input)\n")
#define USE11 MESS(7211, "Special tasks:\n")
#define USE12 MESS(7212, "   -i tab       Fill table 'tab' with text lines from standard input\n")
#define USE13 MESS(7213, "   -o tab       Write text lines from table 'tab' to standard output\n")
#define USE14 MESS(7214, "   -l           List the how-to's in a workspace on standard output\n")
#define USE15 MESS(7215, "   -p           Pack how-to's and locations from workspace to standard output\n")
#define USE16 MESS(7216, "   -u [files]   Unpack workspace from files or standard input\n")
#define USE17 MESS(7217, "   -r           Recover a workspace when its index is lost\n")
#define USE18 MESS(7218, "   -x           Recover the index of a group of workspaces\n")
#define USE19 MESS(7219, "\nUse 'abckeys' to change key bindings\n")

Visible Procedure abc_usage()
{
	putmess(USE00);
	putmess(USE01);
	putmess(USE02);
	putmess(USE03);
	putmess(USE04);
	putmess(USE05);
	putmess(USE06);
	putmess(USE07);
	putmess(USE08);
	putmess(USE09);
	putmess(USE10);
	putmess(USE11);
	putmess(USE12);
	putmess(USE13);
	putmess(USE14);
	putmess(USE15);
	putmess(USE16);
	putmess(USE17);
	putmess(USE18);
	putmess(USE19);
	
	exit(-1);
}

