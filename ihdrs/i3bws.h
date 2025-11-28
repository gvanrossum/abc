/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1988. */

extern char *bwsdir;
extern value ws_group;
extern bool groupchanges;
extern value curwskey;
extern value lastwskey;
extern value cur_ws;
#ifdef WSP_DIRNAME
value abc_wsname();
#endif
extern char *cur_dir; /* absolute path to current workspace */
