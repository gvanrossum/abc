/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1988. */

#include "b.h"
#include "bint.h"
#include "bobj.h"
#include "bfil.h"
#include "port.h"
#include "release.h"
#include "i3env.h"
#include "i3scr.h"
#include "i3sou.h"
#include "port.h"

#define CANT_OPEN MESS(1700, "can't open input file %s\n")

Visible Procedure checkfileargs(argc, argv) int argc; char **argv; {
	bool filearg= No;
	
	/* check call: */
	while (argc > 0) {
		if (argv[0][0] == '-' && argv[0][1] == '\0');
		else if (!F_readable(argv[0])) {
			putSmess(CANT_OPEN, argv[0]);
			exit(-1);
		}
		else filearg= Yes;
		++argv; --argc;
	}
	/* initial setting flag interactive: */
	interactive= !filearg && f_interactive(stdin);
}

Visible bool been_interactive= No;

Visible Procedure abc(argc, argv) int argc; char **argv; {
	bool filearg= argc > 0;
	
	i_lino= 0;
	while (argc >= 0) {
		if (argc == 0 || (argv[0][0] == '-' && argv[0][1] == '\0')) {
			if (argc == 0 && filearg) break;
			if (need_rec_suggestions) {
				rec_suggestions();
				need_rec_suggestions= No;
			}
			release(iname);
			iname = Vnil;
			ifile = stdin;
			process();
			been_interactive= Yes;
		}
		else {
			filearg= Yes;
			release(iname);
			iname = mk_text(argv[0]);
			if (is_abspath(argv[0]))
				ifile= fopen(argv[0], "r");
			else {
				char *path= makepath(startdir, argv[0]);
				ifile= fopen(path, "r");
				free_path(path);
			}
			if (ifile != NULL) {
				process();
				fclose(ifile);
			}
			else {
				putSmess(CANT_OPEN, argv[0]);
				immexit(-1);
			}
		}
		++argv; --argc;
	}
	if (need_rec_suggestions) {
		rec_suggestions();
	}
}

Visible Procedure pre_init() {
	initmess();	/* set messbuf */
	initfmt();	/* set fmtbuf */
	initfile();	/* locate files (messages, keydefs, copybuffer, etc) */
	init_scr();	/* set outeractive and rd_interactive */
	initerr();	/* set err_interactive */
}

extern bool vtrmactive;

Hidden Procedure print_heading() {
	FILE *fp;
	char *fmt, *str;

	if (!rd_interactive)
		return;
	fp = vtrmactive ? CONSOLE : stderr;
	str = getfmtbuf(fmt ="ABC version %s, CWI, Amsterdam.\n",
			strlen(RELEASE));
	sprintf(str, fmt, RELEASE);
	putstr(fp, str);
	putstr(fp, 
	 "Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1991.\n");
}

Visible bool in_init= Yes;

Visible bool use_bed = Yes;

Visible Procedure init()
{
	bool prompt_help;

	use_bed = rd_interactive &&
		(abc_todo == abcProper || abc_todo == abcioInput);
	prompt_help = abc_todo == abcProper;
#ifdef SIGNAL
	initsig();	/* catch signals */
#endif /* SIGNAL */
	if (use_bed) {
		init_erro();	/* buffers for error messages from editor */
		initkeys();	/* read key definitions from termcap and file */
		initterm();	/* start trm module */
		initoperations(); /* buffers for typeahead */
		initgram();	/* edi's grammar tables */
		initclasses();	/* suggestions for builtins */
		initbed();	/* top-ep's admin; read copybuffer */
	}

	re_outfile(); /* reset outfile if stdout tty and vtrmactive */
	re_errfile(); /* reset errfile if stderr tty and vtrmactive */
		
	initnum();
	initsyn();
#ifdef TYPE_CHECK
	initpol();
#endif
	initprmnv();
	/* start first output here,
	 * since trm module may start alternative screen ... */
	print_heading();
	if (interactive && prompt_help)
		c_putstr(GMESS(1701, "Type '?' for help.\n"));
	/* ... and initbws() prints ">current_ws_name" */

	initfpr();
	initbws();

	in_init= No;
}

Visible Procedure endall() {
	/* real tasks: */
	endbws();
	if (use_bed) {
		endbed();	/* save editor parsetree and copybuffer */
		endterm();	/* reset terminal */
	}

	/* the following only free memory: */

	endstrval();	/* hack to free strval static store */
	endnoderepr();	/* hack to free noderepr static store */
#ifdef TYPE_CHECK
	endpol();
#endif
	endsta();
	endsyn();
	endnum();
#ifdef USERSUGG
	endclasses();
#endif
	end_erro();
	end_scr();
	endfile();
	endmess();
}

Visible Procedure crashend() {
	if (cntxt != In_wsgroup && cntxt != In_prmnv)
		endbws();
}
