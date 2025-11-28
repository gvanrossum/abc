/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1988. */

/* Interface to ABC system */

#include "b.h"
#include "bmem.h"
#include "bfil.h"
#include "bint.h"
#include "feat.h"
#include "bobj.h"
#include "b0lan.h"
#include "i2par.h" 
#include "i2nod.h"
#include "i3bws.h"
#include "i3scr.h"
#include "i3sou.h"
#include "i3env.h"
#include "keys.h"
#include "abcmenus.h"
#include "macabc.h"

/* for AppFiles: */

Visible bool is_howto(fname) char *fname; {
	char *p;
	
	p= strchr(fname, C_POINT);
	if (p == NULL)
		return No;
	else
		return strcmp(p, Cmd_ext) == 0;
}

value get_unit();

Visible Procedure run_howto(fname) char *fname; {
	value v;
	value pname;
	parsetree howto;
	parsetree formals;
	string uname;
	parsetree codeseq= NilTree;
	parsetree c= NilTree, d= NilTree; 

	/* get at first line of how-to for parser */
	ifile= fopen(fname, "r");
	if (ifile == NULL) {
		macerrS(FAILOPEN, fname);
		return;
	}
	first_ilev();		/* get first line */
	
	/* parse and define unit */
	v= get_unit(&pname, No, No);
	if (still_ok) {
		def_unit(pname, v);
		howto= How_to(v)->unit;
	}
	
	/* close files for parser */
	fclose(ifile);
	vs_ifile();
	
	/* check proper heading; no parameters */
	if (!still_ok) {
		macerrS(ERRHEAD, fname);
		return;
	}
	formals= *Branch(howto, HOW_FORMALS);
	if (*Branch(formals, FML_TAG) != NilTree) {
		macerr(NOPARAM);
		return;
	}
	uname= sstrval((value) (*Branch(howto, UNIT_NAME)));
	
	/* run as user defined command */
	cntxt= In_command;
	still_ok= Yes;
	interrupted= No;
	can_interrupt= Yes;
	terminated= No;
	resexp= Voi;
	lino= 0;
	if (simple_command(uname, &c, &d)) {
		intrprtr_menus();
		if (still_ok) fix_nodes(&c, &codeseq);
		curline= c; curlino= one;
		execthread(codeseq);
		release(c); release(d);
		wait_for_return();
	}
	else macerrS(NO_EXEC, fname);	/* cannot happen? */
	immexit(0);
}

#define SOBIT 0200

extern int winheight;

Hidden Procedure wait_for_return() {
	char buffer[80];
	string cp;
	int c;
	int llength;
	int flags;

	putnewline(stdout);
	trmputdata(winheight, winheight, 0, "");
	strcpy(buffer, getmess(PRESS_RETURN));
	for (cp = buffer; *cp; )
		*cp++ |= SOBIT;
	trmputdata(winheight, winheight, 0, buffer);
	trmsync(winheight, cp - buffer);
	c = trminput();
	while (c != '\003' && c != '\r'
	       && c != Menuchoice(FileID, QuitItem)
		   && c != Menuchoice(PauseID, InterruptItem))
	{
		trmbell();
		unhilite();
		c = trminput();
	}
	trmputdata(winheight, winheight, 0, "");
	trmsync(winheight, 0);
}

Visible bool is_internal(fname) char *fname; {
	int len= strlen(fname);
	return len >= 4 && strcmp(fname+len-4, ".abc") == 0;
}

Visible bool is_wsgroup(fname) char *fname; {
	return strcmp(fname, WSGROUPFILE) == 0;
}

Visible bool is_copybuffer(fname) char *fname; {
	return 	strcmp(fname, BUFFILE) == 0;
}

#ifdef UNUSED
Visible bool is_wsp(fname) char *fname; {
	return 	strcmp(fname, PERMFILE) == 0
			|| strcmp(fname, POSFILE) == 0
			|| strcmp(fname, SUGGFILE) == 0;
}
#endif

/* for i3bws.c: */

/* Replace mac folder name (without path components) into ABC wsname */

Visible value abc_wsname(wsp) char *wsp; {
	value macname;
	value abcname;
	value name;
	intlet k, len;
	
	/* try to find the ABC worspace name via the index table */
	macname= mk_text(wsp);
	abcname= Vnil;
	len= Valid(ws_group) ? length(ws_group) : 0;
	for (k= 0; k<len; ++k) {
		if (compare(*assoc(ws_group, k), macname) == 0) {
			name= *key(ws_group, k);
			if (compare(name, curwskey) == 0
			    || compare(name, lastwskey) == 0)
				continue;
			abcname= copy(name);
			break;
		}
	}
	release(macname);
	if (!Valid(abcname)) {
		macerrS(NO_ABCWSNAME, wsp);
		abcname= mkabcname(wsp);
	}
	return abcname;
}
	
/* for m1print.c */

Visible string curwsname() {
	if (Valid(cur_ws))
		return strval(cur_ws);
	else
		return " ";
}

/* for m1askperm.c: */

Visible bool goodtag(name) char *name; {
	value v;
	bool r;
	
	r= is_abcname(v= mk_text(name));
	release(v);
	return r;
}

Visible bool existinglocation(str) char *str; {
	value name;
	value pname;
	value *aa;
	bool r;
	
	name= mk_text(str);
	pname= permkey(name, Tar);
	r= p_exists(pname, &aa);
	release(name); release(pname);
	return r;
}

Visible bool goodhowto(name) char *name; {
	char *kw;
	
	tx= name;
	ceol= name+strlen(name);
	return is_cmdname(ceol, &kw) || goodtag(name);
}

Visible bool existinghowto(str) char *str; {
	value name;
	value pname;
	bool r;
	value *aa;
	literal type;
	
	name= mk_text(str);
	r= No;
	for (type=Cmd; type <= Dpd && r == No; type++) {	/* hack, hack; using order */
		pname= permkey(name, type);
		if (p_exists(pname, &aa))
			r= Yes;
		release(pname);
	}
	release(name);
	return r;
}

/* needed from e1help.c for lst_uhds in i3sou.c: */

#define MAXBUFFER 81
extern int winheight;

Visible bool ask_for(nr) int nr; {
	string cp;
	int c;
	char buffer[MAXBUFFER];
	
	trmputdata(winheight, winheight, 0, "");
	strcpy(buffer, getmess(nr));
	for (cp = buffer; *cp; )
		*cp++ |= SOBIT;
	trmputdata(winheight, winheight, 0, buffer);
	trmsync(winheight, cp - buffer);
	c = trminput();
	while (c != '\n' && c != '\r' && c != ' ' && c != EOF) {
		trmbell();
		c = trminput();
	}
	trmputdata(winheight, winheight, 0, "");
	trmsync(winheight, 0);
	return c == ' ' ? Yes : No;
}
