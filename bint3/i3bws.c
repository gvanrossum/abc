/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1988. */

#include "b.h"
#include "bint.h"
#include "bfil.h"
#include "bmem.h"
#include "bobj.h"
#include "i2par.h"
#include "i3bws.h"
#include "i3cen.h"
#include "i3env.h"
#include "i3scr.h"
#include "i3sou.h"
#include "port.h"

Forward Hidden Procedure save_curlast();
Forward Hidden Procedure only_default();
Forward Hidden Procedure put_wsgroup();
Forward Hidden Procedure lst_wsname();
Forward Hidden Procedure wscurrent();
Forward Hidden bool setbwsdir();
Forward Hidden Procedure wsrelease();
Forward Hidden Procedure init_workspace();
Forward Hidden Procedure end_workspace();
Forward Hidden Procedure print_wsname();

/* ******************************************************************** */
/*		workspace routines					*/
/* ******************************************************************** */

Visible char *bwsdir= (char *) NULL;	/* group name workspaces */

Visible value ws_group= Vnil;		/* index workspaces */
Visible bool groupchanges= No;		/* if Yes index has changed */

Visible value curwskey= Vnil;		/* special index key for cur_ws */
Visible value lastwskey= Vnil;		/* special index key for last_ws */

Visible value cur_ws= Vnil;		/* the current workspace */
					/* only visible for m1bio.c */
Hidden value last_ws= Vnil;		/* the last visited workspace */

Hidden bool path_workspace= No;		/* if Yes no workspace change allowed */

extern bool vtrmactive;
Hidden FILE *ws_fp;             /* where to print workspace name */
                                        /* may be changed in initbws */

#ifdef CK_WS_WRITABLE
Hidden bool ckws_once= Yes;             /* flag to give just one read-only
					   warning per workspace change */
#endif

#define gr_exists(name, aa) (in_env(ws_group, name, aa))
#define def_group(name, f)  (e_replace(f, &ws_group, name), groupchanges= Yes)
#define free_group(name)    (e_delete(&ws_group, name), groupchanges= Yes)

/* ******************************************************************** */

Visible char *cur_dir;               /* keeps track of the current directory */

Hidden Procedure setcurdir(path) char *path; {
	if (cur_dir != NULL) freestr(cur_dir);
	cur_dir= savestr(path);
}

Hidden bool ch_dir(path) char *path; {
	if (Chdir(path) == 0) {
		setcurdir(path);
		return Yes;
	}
	else return No;
}

/* ******************************************************************** */

#define CURWSKEY	">"
#define LASTWSKEY	">>"

Hidden Procedure initgroup() {
	wsgroupfile= (string) makepath(bwsdir, WSGROUPFILE);
	curwskey= mk_text(CURWSKEY);
	lastwskey= mk_text(LASTWSKEY);
	if (F_readable(wsgroupfile)) {
		value fname= mk_text(wsgroupfile);
		ws_group= getval(fname, In_wsgroup);
		release(fname);
		if (!still_ok) {
			still_ok= Yes;
			rec_wsgroup();
		}
		
	}
	else ws_group= mk_elt();
	groupchanges= No;
}

extern bool been_interactive;

Hidden Procedure endgroup() {
	if (been_interactive) {
		save_curlast(curwskey, cur_ws);
		save_curlast(lastwskey, last_ws);
	}
	only_default();
	put_wsgroup();
}

Hidden Procedure save_curlast(wskey, ws) value wskey, ws; {
	value *aa;
	
	if (Valid(ws) && (!gr_exists(wskey, &aa) || (compare(ws, *aa) != 0)))
		def_group(wskey, ws);
}

/*
 * removes the default entry if it is the only one;
 * the default is [CURWSKEY]: DEFAULT_WS;
 * this has to be done to create the possibility of removing an empty
 * wsgroupfile and bwsdefault directory;
 * still this will hardly happen (see comments in endbws() )
 */

Hidden Procedure only_default() {
	value *aa;

	if (length(ws_group) == 1 &&
		Valid(curwskey) && gr_exists(curwskey, &aa) 
	   ) {
	   	value defws= mk_text(DEFAULT_WS);
	   	if (compare(defws, *aa) == 0)
	   		free_group(curwskey);
	   	release(defws);
	}
}

Hidden Procedure put_wsgroup()
{
	intlet len;
	
	if (!groupchanges || !Valid(ws_group))
		return;
	len = length(ws_group);
	if (len != 0) {
		putval(ws_group, bwsdir, WSGROUPFILE, In_wsgroup, Yes);
	}
	else { /* Remove the file if empty */
		char *file = makepath(bwsdir, WSGROUPFILE);
		f_delete(file);
		free_path(file);
	}
	groupchanges= No;
}

/* ******************************************************************** */

Hidden bool wschange(ws) value ws; {
	value name, *aa;
	bool new= No, changed;
	char *path;

	if (gr_exists(ws, &aa))
		name= copy(*aa);
	else {
		name= new_fname(ws, Wsp);
		if (!Valid(name))
			return No;
		new= Yes;
	}
	path= makepath(bwsdir, strval(name));
	VOID Mkdir(path);
	changed= ch_dir(path);
	if (changed) {
		if (new) def_group(ws, name);
#ifdef CK_WS_WRITABLE
	        ckws_once= Yes;
#endif
	}
	free_path(path);
	release(name);
	return changed;
}

Hidden bool rm_dir(path) char *path; {
	if (strcmp(startdir, path) == 0) return No;
	else if (rmdir(path) != 0) return No;
	else return Yes;
}

Hidden Procedure wsempty(ws) value ws; {
	char *path, *permpath;
	value *aa;
	
	if (!gr_exists(ws, &aa))
		return;
	path= makepath(bwsdir, strval(*aa));
	permpath= makepath(path, permfile);
	if (F_exists(permpath));
	else if (rm_dir(path) == No);
	else free_group(ws);
	free_path(path);
	free_path(permpath);
}

/* ******************************************************************** */

Visible Procedure goto_ws() {
	value ws= Vnil;
	bool prname; /* print workspace name */

	if (path_workspace) {
		parerr(MESS(2900, "change of workspace not allowed"));
		return;
	}
	if (Ceol(tx)) {
		if (Valid(last_ws))
			ws= copy(last_ws);
		else
			parerr(MESS(2901, "no previous workspace"));
		prname= Yes;
	}
	else if (is_tag(&ws))
		prname= No;
	else
		parerr(MESS(2902, "I find no workspace name here"));
	
	if (still_ok && (compare(ws, cur_ws) != 0)) {
		can_interrupt= No;
		end_workspace(No);
		
		if (wschange(ws)) {
			release(last_ws); last_ws= copy(cur_ws);
			release(cur_ws); cur_ws= copy(ws);
		}
		else {
			parerrV(MESS(2903, "I can't goto/create workspace %s"), ws);
			still_ok= Yes;
			prname= No;
		}
		
		init_workspace(No, prname);
		wsempty(last_ws);
		can_interrupt= Yes;
	}
	release(ws);
}

Visible Procedure lst_wss() {
	value wslist, ws;
	value k, len, m;

	if (path_workspace) {
		print_wsname();
		return;
	}
	wslist= keys(ws_group);
	
	if (!in(cur_ws, wslist))
		insert(cur_ws, &wslist);
	
	k= one; len= size(wslist);
	while (numcomp(k, len) <= 0) {
		ws= item(wslist, k);
		if (compare(ws, curwskey) == 0);
		else if (compare(ws, lastwskey) == 0);
		else if (compare(ws, cur_ws) == 0)
			lst_wsname(ws, Yes);
		else
			lst_wsname(ws, No);
		release(ws);
		k= sum(m= k, one);
		release(m);
	}
	if (numcomp(len, zero) > 0)
		c_putnewline();
	c_flush();

	release(k); release(len);
	release(wslist);
}

Hidden Procedure lst_wsname(ws, current)
	value ws;
	bool current;
{
	if (current) c_putstr(">");
	c_putstr(strval(ws));
	c_putstr(" ");
}

/************************************************************************/

#define NO_PARENT	MESS(2904, "*** I cannot find parent directory\n")
#define NO_WORKSPACE	MESS(2905, "*** I cannot find workspace\n")
#define NO_DEFAULT	MESS(2906, "*** I cannot find your home directory\n")
#define USE_CURRENT	MESS(2907, "*** I shall use the current directory as your single workspace\n")
#define NO_ABCNAME	MESS(2908, "*** %s isn't an ABC name\n")
#define TRY_DEFAULT	MESS(2909, "*** I shall try the default workspace\n")
#define NO_CENTRAL      MESS(2910, "*** I cannot find the central workspace\n")

Hidden Procedure wserr(m, use_cur) int m; bool use_cur; {
	putmess(m);
	if (use_cur)
		wscurrent();
}

Hidden Procedure wserrV(m, v, use_cur) int m; value v; bool use_cur; {
	putSmess(m, strval(v));
	if (use_cur)
		wscurrent();
}

Hidden Procedure wscurrent() {
	putmess(USE_CURRENT);
	path_workspace= Yes;
}

/* ******************************************************************** */

Hidden bool wsinit() {
	value *aa;
	
	initgroup();
	cur_ws= Vnil;
	last_ws= Vnil;
	if (OPTworkspace) {
		/* OPTworkspace is a single name here, not a pathname */
#ifdef WSP_DIRNAME
		/* on the mac OPTworkspace is a mac foldername, not an ABC wsname */
		cur_ws= abc_wsname(OPTworkspace);
		if (!Valid(cur_ws))
			return No;
#else
		/* OPTworkspace is here an ABC workspace name, not a path */
		cur_ws= mk_text(OPTworkspace);
#endif
		if (!is_abcname(cur_ws)) {
			wserrV(NO_ABCNAME, cur_ws, No);
			wserr(TRY_DEFAULT, No);
			release(cur_ws); cur_ws= Vnil;
		}
	}
	if (gr_exists(curwskey, &aa)) {
		if (!Valid(cur_ws))
			cur_ws= copy(*aa);
		else if (compare(cur_ws, *aa) != 0)
			last_ws= copy(*aa);
		if (!Valid(last_ws) && gr_exists(lastwskey, &aa))
			last_ws= copy(*aa);
	}
	if (!Valid(cur_ws))
		cur_ws= mk_text(DEFAULT_WS);
	if (!is_abcname(cur_ws))
		wserrV(NO_ABCNAME, cur_ws, Yes);
	else if (wschange(cur_ws)) {
		path_workspace= No;
		return Yes;
	}
	else wserr(NO_WORKSPACE, Yes);
	return No;
}

Visible Procedure initbws() {
	initcurenv();
	setcurdir(startdir);
	ws_fp= stderr; /*sp 20010221 */
	if (abc_todo == abcioGRrecover) {
		/* recover index of group workspaces */
		if (!setbwsdir() || !D_exists(bwsdir)) {
			wserr(NO_PARENT, No);
			immexit(1);
		}
		initgroup();
		return;
	}
	if (OPTcentral && is_path(OPTcentral)) {
		/* check existence of command line argument central workspace */
		if (is_abspath(OPTcentral))
			cen_dir = (char *) savestr(OPTcentral);
		else
		        cen_dir = makepath(startdir, OPTcentral);
		if (!D_exists(cen_dir)) {
			wserr(NO_CENTRAL, No);
			free_path(cen_dir);
			cen_dir = (char *)0;
		}
	}
	if (is_path(OPTworkspace)) {
		/* !OPTbwsdir already assured in main.c */

		if (OPTcentral && !is_path(OPTcentral)) {
		        /* error; message already given in main.c; */
			;
		}

		if (ch_dir(OPTworkspace) == No)
			wserr(NO_WORKSPACE, Yes);
		else 
			path_workspace= Yes;
	}
	else if (setbwsdir()) {
		if (!D_exists(bwsdir))
			wserr(NO_PARENT, Yes);
		else {
			if (OPTcentral && !is_path(OPTcentral))
				cen_dir = makepath(bwsdir, OPTcentral);
			else if (!OPTcentral)
				cen_dir = makepath(bwsdir, CENTRAL_WS);

			if (!wsinit()) wsrelease();
		}
	}
	else wserr(NO_DEFAULT, Yes);
	if (path_workspace) {
		release(cur_ws);
		cur_ws= mk_text(cur_dir);
	}
	if (vtrmactive) ws_fp= CONSOLE;
	init_workspace(Yes, Yes);
}

Visible Procedure endbws() {
	if (abc_todo != abcioGRrecover) {
		end_workspace(Yes);
		VOID ch_dir(startdir);
		if (path_workspace) {
			release(cur_ws);
			cur_ws= Vnil;
			return;
		}
		else wsempty(cur_ws);
	}
	/* else: only index of group workspaces recovered */

	endgroup();
	/* 
	 * if the bwsdefault directory is used and empty, remove it;
	 * because of the savings of the last two visited workspaces
	 * in the file `bwsdefault`/`wsgroupfile` this will hardly happen;
	 * only if you stays for ever in the default workspace.
	 */
	if (!OPTbwsdir && bwsdefault)
		VOID rm_dir(bwsdefault); /* fails if startdir or not empty */
	wsrelease();
}

Hidden bool setbwsdir() {
	if (OPTbwsdir || bwsdefault) {
		if (!OPTbwsdir) {
			bwsdir= (char *) savestr(bwsdefault);
			/* full path name */
			VOID Mkdir(bwsdir);
		}
		else if (!is_abspath(OPTbwsdir))
			bwsdir= makepath(startdir, OPTbwsdir);
		else
			bwsdir= (char *) savestr(OPTbwsdir);
		return Yes;
	}
	return No;
}

Hidden Procedure wsrelease() {
	release(last_ws); last_ws= Vnil;
	release(cur_ws); cur_ws= Vnil;
	release(lastwskey); lastwskey= Vnil;
	release(curwskey); curwskey= Vnil;
	release(ws_group); ws_group= Vnil;
	free_path(wsgroupfile); wsgroupfile= (string) NULL;
	free_path(bwsdir); bwsdir= (char *) NULL;
}

/************************************************************************/

Hidden Procedure init_workspace(startup, prname)
  bool startup;
	bool prname;
{
	if (startup) {
		initfpr();                  /* init standard funs/prds */
		last_unit = mk_text(":");   /* last edited unit */
		last_target = mk_text("="); /* last edited target */
	}
	initcentralworkspace(startup);
	if (interactive) {
		if (prname) print_wsname();
		at_nwl = Yes; /* Brrr */
	}
	initworkspace();
	if (!still_ok) {
		still_ok= Yes;
		rec_workspace();
	}
}

Visible Procedure initworkspace() {
	initsou();
	initenv();
#ifdef USERSUGG
	initsugg();
#endif
#ifdef SAVEPOS
	initpos();
#endif
#ifdef TYPE_CHECK
	initstc();
#endif
	setprmnv();
	initperm();
}

Hidden Procedure end_workspace(last)
     bool last;
{
	endcentralworkspace(last);
	     /* must be called before endworkspace();
	      * if in central workspace we copy the current perm table first
	      */
	endworkspace();
	if (last) {
#ifdef MEMTRACE
		release(last_unit);
		release(last_target);
#endif
		endfpr();
	}
}

Visible Procedure endworkspace() {
	endperm();
	endsou();
	endenv();
#ifdef USERSUGG
	endsugg();
#endif
#ifdef SAVEPOS
	endpos();
#endif
#ifdef TYPE_CHECK
	endstc();
#endif
	enderro();
}

/************************************************************************/

Hidden Procedure print_wsname() {
	char *fmt, *str;
	char *name = strval(cur_ws);

	str = getfmtbuf(fmt = ">%s\n", strlen(name));
	sprintf(str, fmt, name);
	putstr(ws_fp, str);
	doflush(ws_fp);
}

/************************************************************************/

#ifdef CK_WS_WRITABLE

Hidden bool wsp_writable() {
	char *wsp = cur_dir;
	return D_writable(wsp) ? Yes : No;
}

Visible bool ckws_writable(m) int m; {
	/* check if the workspace is writable and
	 * if it isn't give a warning, once a workspace (change)
	 */
	if (ckws_once == No) return Yes;
	else if (wsp_writable()) return Yes;
	else {
		ckws_once = No;
		return is_intended(m); /* read-only, but display ? */
	}
}

#endif /* CK_WS_WRITABLE */
