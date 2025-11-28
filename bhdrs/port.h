/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1990. */

/* externals of which the definitions are in a port directory */

Visible bool ed_file();            /* edit.c */

Visible Procedure initfile();      /* file.c */
Visible char *makepath();
Visible int Chdir();

extern char *startdir;
extern char *bwsdefault;
extern char *messfile;
extern char *helpfile;
extern char *keysfile;
extern char *buffile;

#define BWSNAME		"abc"
#define MESSFILE	"abc.msg"
#define KEYSFILE	"abc.key"
#define HELPFILE	"abc.hlp"
#define BUFFILE		"copybuf.abc"
#define FORMAT_KEYSFILE "abc%s.key"    /* abc$TERM.key */

Visible string reprchar();              /* keys.c */
#ifndef CANLOOKAHEAD
extern char intrchar;
#endif
Visible Procedure addspeckeys();
/* extern struct tabent deftab[];  in getc.h */

extern char *OPTbwsdir;                /* main.c */
extern char *OPTworkspace;
extern char *OPTcentral;
extern char *OPTeditor;
extern bool OPTslowterminal;

extern int abc_todo;
#define abcProper       1             /* run ABC normally */
#define abcioInput      2             /* run abc -i (input a table) */
#define abcioOutput     3             /* run abc -o (output a table) */
#define abcioList       4             /* run abc -l (list how-to's) */
#define abcioPack	7	      /* run abc -p (pack workspace) */
#define abcUnpack	8	      /* run abc -u (unpack workspace) */
#define abcioWSrecover  5             /* run abc -r (recover a workspace) */
#define abcioGRrecover  6             /* run abc -x (recover ws parent) */

Visible int getseed();                 /* os.c */
Visible Procedure getdatetime();
Visible bool is_path();
Visible bool is_abspath();
Visible bool is_directory();

extern bool intrptd;                 /* sig.c */
Visible Procedure initsig();

Visible Procedure abc_usage();       /* usage.c */
