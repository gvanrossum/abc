/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1991. */

#include "b.h"
#include "getopt.h"
#include "port.h"

Forward Hidden Procedure re_direct();
Forward Hidden char *StrToLower();

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
#endif

#define INCOMP_WS_OPTIONS \
        MESS(6900, "*** incompatible workspace options\n")
#define NO_EDITOR \
        MESS(6901, "*** you have not set your environment variable EDITOR\n")

/* option bits */
/* WARNING: because of a bug in the earlier versions of the Atari ST desktop,
 * causing lower-case letters passed as arguments to be converted to
 * upper-case, we must treat the upper-case versions as equivalent.
 */
#define O_g   1		/* -g option */
#define O_w   2		/* -w option */
#define O_e	  4	/* -e option (with [file ...]) allowed */
#define O_iolprx  8	/* -i, -o -l, -p, -r, -x option */
#define O_u      16	/* -u option (with [file ...]) allowed */
#define O_c      32     /* -c option */

#define O_eiolpurx (O_e | O_iolprx | O_u)

#ifdef __GNUC__
long _stksize = -1L; /*  Stack needs to be huge.... */
#else
long _stksize = 20000; /*  Stack needs to be huge.... */
#endif

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
	while ((c= getopt(argc, argv, "g:w:c:ei:o:lpurxsG:W:C:EI:O:LPURXS")) != EOF) {
#else
	while ((c= getopt(argc, argv, "g:w:c:ei:o:lpurxsdG:W:C:EI:O:LPURXSD")) != EOF) {
#endif

		switch (c) {
		case 'g':
		case 'G':
			if (flags & O_g) usage_error = Yes;
			else {
				flags |= O_g;
				OPTbwsdir = StrToLower(optarg);
			}
			break;
		case 'w':
		case 'W':
			if (flags & O_w) usage_error = Yes;
			else {
				flags |= O_w;
				OPTworkspace = StrToLower(optarg);
			}
			break;
		case 'c':
		case 'C':
			if (flags & O_c) usage_error = Yes;
			else {
				flags |= O_c;
				OPTcentral = StrToLower(optarg);
			}
			break;
		case 'e':
		case 'E':
			if (flags & O_eiolpurx) usage_error = Yes;
			else {
				flags |= O_e;
			}
			break;
                case 'i':
		case 'I':
                	if (flags & O_eiolpurx) usage_error = Yes;
                	else {
                		flags |= O_iolprx;
				abcio_arg = StrToLower(optarg);
				abc_todo= abcioInput;
			}
			break;
		case 'o':
		case 'O':
                	if (flags & O_eiolpurx) usage_error = Yes;
                	else {
                		flags |= O_iolprx;
				abcio_arg = StrToLower(optarg);
				abc_todo = abcioOutput;
			}
			break;
		case 'l':
		case 'L':
                	if (flags & O_eiolpurx) usage_error = Yes;
                	else {
                		flags |= O_iolprx;
				abc_todo = abcioList;
			}
			break;
		case 'p':
		case 'P':
                	if (flags & O_eiolpurx) usage_error = Yes;
                	else {
                		flags |= O_iolprx;
				abc_todo = abcioPack;
			}
			break;
		case 'u':
		case 'U':
                	if (flags & O_eiolpurx) usage_error = Yes;
                	else {
                		flags |= O_u;
				OPTunpack = Yes;
				/* abc_todo == abcProper !!! */
			}
			break;
		case 'r':
		case 'R':
                	if (flags & O_eiolpurx) usage_error = Yes;
                	else {
                		flags |= O_iolprx;
				abc_todo = abcioWSrecover;
			}
			break;
		case 'x':
		case 'X':
                	if (flags & O_eiolpurx) usage_error = Yes;
                	else {
                		flags |= O_iolprx;
				abc_todo = abcioGRrecover;
			}
			break;

		case 's':
		case 'S':
			OPTslowterminal= Yes;
			break;

#ifndef NDEBUG
		case 'd':
		case 'D':
			dflag= Yes;
			break;
#endif
		default:
			usage_error= Yes;
			break;
		}
	}

	argc -= optind;
	argv += optind;
	
	re_direct (&argc, argv);
	/* re_direct() must be called before pre_init(),
	   because the last one sets rd_interactive */

	if (argc > 0 && (flags & O_iolprx))
		usage_error = Yes;
	
	pre_init(); /* set among others 'messfile' */
	
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

Hidden char *StrToLower(s)
     char *s;
{
	register char *p;

	for(p = s; *p; p++) {
		if (isupper(*p)) *p=_tolower(*p);
	}
	return s;

}

/*
 *	r e _ d i r e c t
 *
 *	This procedure scans the argument string 'av' and looks for
 *	occurances of the tokens '<' and '>', optionally preceded by 
 *      a file descriptor (0, 1 or 2). By attaching the 
 *	corresponding files with the standard input, output and error,
 *	I/O redirection within the Desktop becomes possible:
 *              abc [0]<inp [1]>out 2>err
 *
 *	Searching is started at the END of the argument string.
 *
 *	NO spaces between the file descriptor and the token, and
 *      between the token and the file name!
 *
 *	The file names are removed from the argument string and the
 *	the argument count will be adjusted properly.
 *	When run ABC from a shell, these tokens will not 
 *	appear in the argument string and everything will remain as it
 *	was.
 */

Hidden Procedure re_direct(ac_ptr, av)
     int *ac_ptr;		/* Pointer to argument count */
     char *av[];		/* Argument strings array */
{
	int  ac    = *ac_ptr;	/* Argument count */
	char *arg;             /* Argument pointer */
	bool Iopen = No;	/* Standard input attached ? */
	bool Oopen = No;	/* Standard output attached ? */
	bool Eopen = No;        /* Standard error attached ? */
	char *fname;     	/* File name found after '>' or '<' */

	/*
         * Start peeling off arguments at END!
	 */
	if (ac > 0) {		/* If arguments */
		ac--;
		for (;;) {
			arg = av[ac];
			if ((*arg == '0' && *(arg+1) == '<') || *arg == '<') {
				/* Attach 'stdin' with file name */
				if (!Iopen) {
					if (*arg == '0') arg++;
					fname = ++arg;
					if (*fname) {
						freopen(fname, "r", stdin);
						Iopen = Yes;
					}
				}
				ac--;
			}
			else if ((*arg == '1' && *(arg+1) == '>') || *arg == '>') {
				/* Attach 'stdout' with file name */
				if (!Oopen) {
					if (*arg == '1') arg++;
					fname = ++arg;
					if (*fname) {
						freopen (fname, "w", stdout);
						Oopen = Yes;
					}
				}
				ac--;
			}
			else if (*arg == '2' && *(arg+1) == '>') {
				/* Attach 'stderr' with file name */
				if (!Eopen) {
					arg++;
					fname = ++arg;
					if (*fname) {
						freopen (fname, "w", stderr);
						Eopen = Yes;
					}
				}
				ac--;
			}
			else break;
		}
		*ac_ptr = ++ac;	/* Adjust argument count */
		av[ac] = NULL;	/* Adjust argument string */
	}
}
