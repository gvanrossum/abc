/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1990. */

#include "b.h"
#include "getopt.h"
#include "port.h"

Visible char *OPTbwsdir= (char *) NULL;
			/* -g OPTbwsdir: group name workspaces */
Visible char *OPTworkspace= (char *) NULL;
			/* -w OPTworkspace: start workspace */
Visible char *OPTcentral= (char *) NULL;
                        /* -c OPTcentral: central workspace */

Visible char *OPTeditor= (char *) NULL;
			/* -e: use ${EDITOR} instead of ABC-editor */

Visible bool OPTunpack= No;
                       /* -u: unpack workspace */

Visible bool OPTslowterminal= No;
			/* -s: do not tell "cannot insert" on slow terminal */

Visible int abc_todo;
                       /* task todo according to the arguments */
                       /* possible values are in port.h */

#ifndef NDEBUG

extern bool dflag;

/* use -DVTRMTRACE during compilation to enable these flag. */

#ifdef VTRMTRACE
Visible string vtrmfile= NULL;
Visible FILE *vtrmfp= NULL;
		/* -V vtrmfile: trace typechecker on vtrmfile */
#endif
#endif /* NDEBUG */

#define INCOMP_WS_OPTIONS \
    	MESS(6900, "*** incompatible workspace options\n")
#define NO_EDITOR \
        MESS(6901, "*** you have not set your environment variable EDITOR\n")

/* option bits */
#define O_g   1		/* -g option */
#define O_w   2		/* -w option */
#define O_e	  4	/* -e option (with [file ...]) allowed */
#define O_iolprx  8	/* -i, -o -l, -p, -r, -x option */
#define O_u      16	/* -u option (with [file ...]) allowed */
#define O_c      32     /* -c option */

#define O_eiolpurx (O_e | O_iolprx | O_u)

main(argc, argv)
     int argc;
     char **argv;
{
	int c;
	int flags = 0;
	bool usage_error= No;
	char *abcio_arg= (char *) NULL;

	abc_todo = abcProper;

#ifdef  NDEBUG
	while ((c= getopt(argc, argv, "g:w:c:ei:o:lpurxs")) != EOF) {
#else
	while ((c= getopt(argc, argv, "g:w:c:ei:o:lpurxsdV:")) != EOF) {
#endif
		switch (c) {
		case 'g':
			if (flags & O_g) usage_error = Yes;
			else {
				flags |= O_g;
				OPTbwsdir = optarg;
			}
			break;
		case 'w':
			if (flags & O_w) usage_error = Yes;
			else {
				flags |= O_w;
				OPTworkspace = optarg;
			}
			break;
		case 'c':
			if (flags & O_c) usage_error = Yes;
			else {
				flags |= O_c;
				OPTcentral = optarg;
			}
			break;
		case 'e':
			if (flags & O_eiolpurx) usage_error = Yes;
			else {
				flags |= O_e;
			}
			break;
		case 'i':
                	if (flags & O_eiolpurx) usage_error = Yes;
                	else {
                		flags |= O_iolprx;
				abcio_arg = optarg;
				abc_todo= abcioInput;
			}
			break;
		case 'o':
                	if (flags & O_eiolpurx) usage_error = Yes;
                	else {
                		flags |= O_iolprx;
				abcio_arg = optarg;
				abc_todo = abcioOutput;
			}
			break;
		case 'l':
                	if (flags & O_eiolpurx) usage_error = Yes;
                	else {
                		flags |= O_iolprx;
				abc_todo = abcioList;
			}
			break;
		case 'p':
                	if (flags & O_eiolpurx) usage_error = Yes;
                	else {
                		flags |= O_iolprx;
				abc_todo = abcioPack;
			}
			break;
		case 'u':
                	if (flags & O_eiolpurx) usage_error = Yes;
                	else {
                		flags |= O_u;
				OPTunpack = Yes;
				/* abc_todo == abcProper !!! */
			}
			break;
		case 'r':
                	if (flags & O_eiolpurx) usage_error = Yes;
                	else {
                		flags |= O_iolprx;
				abc_todo = abcioWSrecover;
			}
			break;
		case 'x':
                	if (flags & O_eiolpurx) usage_error = Yes;
                	else {
                		flags |= O_iolprx;
				abc_todo = abcioGRrecover;
			}
			break;

		case 's':
			OPTslowterminal= Yes;
			break;

#ifndef NDEBUG
		case 'd':
			dflag= Yes;
			break;
#ifdef VTRMTRACE
		case 'V':
			vtrmfile= optarg;
			break;
#endif
#endif /* NDEBUG */

		default:
			usage_error= Yes;
			break;

		}
	}

	argc -= optind;
	argv += optind;
	
	if (argc > 0 && (flags & (O_iolprx)))
		usage_error = Yes;

#ifndef NDEBUG
#ifdef VTRMTRACE
	if (vtrmfile != NULL)
		vtrmfp= fopen(vtrmfile, "w");
#endif
#endif /* NDEBUG */
	
	pre_init();	/* set among others 'messfile' */
	
	if (is_path(OPTworkspace)
	    && ((flags & O_g) || ((flags & O_c) && !is_path(OPTcentral)))) {
		putmess(INCOMP_WS_OPTIONS);
		usage_error = Yes;
	}
	
	if (flags & O_e) {
		char *getenv();
		OPTeditor= getenv("EDITOR");
		if (OPTeditor == (char *)NULL || *OPTeditor == '\0') {
			putmess(NO_EDITOR);
			usage_error= Yes;
		}
	}
	
	if (usage_error)
		abc_usage();	/* exits */
	
	checkfileargs(argc, argv);
	
	init();

	if (abc_todo == abcProper)
	        abc(argc, argv);   /* also handles -u option !!! */
	else
		abcio(abc_todo, abcio_arg);

	bye(0);
}
