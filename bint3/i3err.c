/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1986. */

/* B error message handling */

/* There are two kinds of errors:
	1) parsing, when the line in error is in a buffer
	2) execution, when the line in error is a parse-tree, and must
	   therefore be reconstructed.
*/

#include "b.h"
#include "bmem.h"
#include "bint.h"
#include "bobj.h"
#include "i0err.h"
#include "i2par.h"
#include "i3cen.h"
#include "i3env.h"
#include "i3scr.h"
#include "i3sou.h"

#ifdef GFX
#include "bgfx.h"
#endif

extern bool OPTunpack; /* necessary for Mac */

Visible bool still_ok= Yes;
Visible bool mess_ok= Yes;	/* if Yes print error message */
Visible bool interrupted= No;
Visible bool can_interrupt= Yes;

Visible parsetree curline= Vnil;
Visible value curlino;

Hidden FILE *errfile;	        /* may be changed in re_errfile() */
Hidden bool err_interactive;

Forward Hidden Procedure show_line();
Forward Hidden Procedure show_howto();
Forward Hidden bool unit_file();
Forward Hidden Procedure show_f_line();
Forward Hidden Procedure putcerr();

/*********************************************************************/

Hidden Procedure nline() {
	flushout();
	/* should be i3scr.c's ofile, but doesnot matter */
	if (cntxt == In_read && rd_interactive)
		at_nwl= Yes;
	if (!at_nwl)
		putcerr('\n');
	at_nwl= Yes;
}

Hidden Procedure pr_line(at) bool at; {
	/*prints the line that tx is in, with an arrow pointing to the column
	  that tx is at.
	*/
	txptr lx= fcol(); intlet ap= -1, p= 0; char c;
	txptr ax= tx;
	
	if (!at) do ax--; while (Space(Char(ax)));
	while (!Eol(lx) && Char(lx) != Eotc) {
		if (lx == ax) ap= p;
		c= *lx++;
		if (c == '\t') {
			do { putcerr(' '); } while (((++p)%4)!=0);
		} else { putcerr(c); p++; }
	}
	putcerr('\n');
	if (ap < 0) ap= p;
	for (p= 0; p < ap+4; p++) putcerr(' ');
	putserr("^\n");
}

#define IN_COMMAND	MESS(3100, " in your command\n")
#define IN_READ		MESS(3101, " in your expression to be read\n")
#define IN_EDVAL	MESS(3102, " in your edited value\n")
#define IN_TARVAL	MESS(3103, " in your location %s\n")
#define IN_PRMNV	MESS(3104, " in your permanent environment\n")
#define IN_WSGROUP	MESS(3105, " in your workspace index\n")
#define IN_UNIT		MESS(3106, " in your how-to %s\n")
#define IN_CEN_UNIT	MESS(3107, " in your central how-to %s\n")
#define IN_UNIT_LINE	MESS(3108, " in line %d of your how-to %s\n")
#define IN_CEN_UNIT_LINE MESS(3109, " in line %d of your central how-to %s\n")
#define IN_INPUT	MESS(3110, "*** (detected after reading 1 line of your input file standard input)\n")
#define IN_INPUT_LINE	MESS(3111, "*** (detected after reading %d lines of your input file standard input)\n")
#define IN_FILE		MESS(3112, "*** (detected after reading 1 line of your input file %s)\n")
#define IN_FILE_LINE	MESS(3113, "*** (detected after reading %d lines of your input file %s)\n")

Hidden Procedure show_where(in_node, at, node)
	bool in_node, at; parsetree node; {

	int line_no= in_node ? intval(curlino) : lino;
	show_line(in_node, at, node, line_no);
	if ((!interactive && ifile == sv_ifile && !unit_file())
	    || OPTunpack)
		show_f_line();
}

Hidden Procedure show_line(in_node, at, node, line_no)
	bool in_node, at; parsetree node; int line_no; {
	
	switch (cntxt) {
		case In_command: putmess(IN_COMMAND); break;
		case In_read: putmess(IN_READ); break;
		case In_edval: putmess(IN_EDVAL); break;
		case In_tarval:
			putSmess(IN_TARVAL, strval(errtname));
			break;
		case In_prmnv: putmess(IN_PRMNV); break;
		case In_wsgroup: putmess(IN_WSGROUP); break;
		case In_unit: show_howto(line_no); break;
		default:
			putserr("???\n");
			return;
	}
	if (!in_node || Valid(node)) putserr("    ");
	if (in_node) display(errfile, node, Yes);
	else pr_line(at);
}

Hidden value unitname(line_no)
     int line_no;
{
	if (Valid(howtoname) && Is_text(howtoname)) {
		def_perm(last_unit, howtoname);
		cur_env->errlino= line_no;
		return Permname(howtoname);
	}
	else {
		free_perm(last_unit);
		cur_env->errlino = 0;
		return mk_text("");
	}
}

Hidden Procedure show_howto(line_no) int line_no; {
	value name= unitname(line_no);
	int m;

	if (line_no == 1) {
		m = InUsingEnv() ? IN_UNIT : IN_CEN_UNIT;
		putSmess(m, strval(name));
	}
	else {
		m = InUsingEnv() ? IN_UNIT_LINE : IN_CEN_UNIT_LINE;
		putDSmess(m, line_no, strval(name));
	}
	release(name);
}

Hidden bool unit_file() {
	value *aa;
	return cntxt == In_unit &&
		Valid(howtoname) && Is_text(howtoname) && p_exists(howtoname, &aa);
}

Hidden Procedure show_f_line() {
	if (f_lino == 1 && iname == Vnil) 
		putmess(IN_INPUT);
	else if (f_lino == 1)
		putSmess(IN_FILE, strval(iname));
	else if (iname == Vnil)
		putDSmess(IN_INPUT_LINE, f_lino, "");
	else
		putDSmess(IN_FILE_LINE, f_lino, strval(iname));
	if (iname != Vnil && i_lino > 0) {
		if (i_lino == 1)
			putmess(IN_INPUT);
		else
			putDSmess(IN_INPUT_LINE, i_lino, "");
	}
}

#define PROBLEM		MESS(3114, "*** The problem is:")

Visible Procedure syserr(m) int m; {
	static bool beenhere= No;
	if (beenhere) immexit(-1);
	beenhere= Yes;
	nline();
#ifdef DEBUG
#ifdef macintosh
	Debugger();
#endif
#endif
	putmess(MESS(3115, "*** Sorry, ABC system malfunction\n"));
	putmess(PROBLEM);
	putserr(" ");
	putmess(m); 
	putcerr('\n');
	bye(-1);
}

#ifndef MEMEXH_ALERT
/* MacABC uses an alert to make sure the user gets the message */

Visible Procedure memexh() {
	static bool beenhere= No;
	if (beenhere) immexit(-1);
	beenhere= Yes;
	nline();
	putmess(MESS(3116, "*** Sorry, memory exhausted"));
/* show_where(Yes, Yes); don't know if in node or not; to fix */
	putcerr('\n');
	bye(-1);
}

#endif /* MEMEXH_ALERT */

Hidden Procedure message(m1, m2, in_node, at, arg)
	int m1, m2;
	bool in_node, at; 
	value arg;
{
	still_ok= No;
	if (!mess_ok)
		return;
	nline();
	putmess(m1);
	show_where(in_node, at, curline);
	putmess(PROBLEM);
	putserr(" ");
	putSmess(m2, Valid(arg) ? strval(arg) : "");
	putcerr('\n');
	flusherr();
	at_nwl=Yes;
}

#define UNDERSTAND	MESS(3117, "*** There's something I don't understand")

#define RESOLVE		MESS(3118, "*** There's something I can't resolve")

#define COPE		MESS(3119, "*** Can't cope with problem")

#define RECONCILE	MESS(3120, "*** Cannot reconcile the types")

Visible Procedure pprerrV(m, v) int m; value v; {
	if (still_ok)
		message(UNDERSTAND, m, No, No, v);
}

Visible Procedure pprerr(m) int m; {
	if (still_ok)
		message(UNDERSTAND, m, No, No, Vnil);
}

Visible Procedure parerrV(m, v) int m; value v; {
	if (still_ok)
		message(UNDERSTAND, m, No, Yes, v);
}

Visible Procedure parerr(m) int m; {
	if (still_ok)
		message(UNDERSTAND, m, No, Yes, Vnil);
}

Visible Procedure fixerrV(m, v) int m; value v; {
	if (still_ok)
		message(RESOLVE, m, Yes, Yes, v);
}

Visible Procedure fixerr(m) int m; {
	if (still_ok)
		message(RESOLVE, m, Yes, Yes, Vnil);
}

Visible Procedure typerrV(m, v) int m; value v; {
	if (still_ok)
		message(RECONCILE, m, Yes, Yes, v);
}

Visible Procedure interrV(m, v) int m; value v; {
	if (still_ok)
		message(COPE, m, Yes, No, v);
}

Visible Procedure interr(m) int m; {
	if (still_ok)
		message(COPE, m, Yes, No, Vnil);
}

Visible Procedure checkerr() {
	still_ok= No;
	nline();
	putmess(MESS(3121, "*** Your check failed"));
	show_where(Yes, No, curline);
	flusherr();
	at_nwl= Yes;
}

Visible Procedure int_signal() {
	if (can_interrupt) {
		interrupted= Yes; still_ok= No;
		if (cntxt == In_wsgroup || cntxt == In_prmnv)
			immexit(-1);
	}
	nline();
	putmess(MESS(3122, "*** interrupted\n"));
	if (!interactive) {
		if (ifile != stdin) fclose(ifile);
		bye(1);
	}
	flusherr();
	if (can_interrupt) {
		if (cntxt == In_read) {
			set_context(&read_context);
			copy(howtoname);
		}
	}
	at_nwl= Yes;
}

Visible Procedure fpe_signal() {
	interr(MESS(3123, "unexpected arithmetic overflow"));
}

Visible bool testing= No;

Visible Procedure bye(ex) int ex; {
#ifdef GFX
	if (gfx_mode != TEXT_MODE)
		exit_gfx();
#endif
	at_nwl= Yes;
/*	putperm(); */ /* shall be called via endall() */
	endall();
	immexit(ex);
}

extern bool vtrmactive;

Visible Procedure immexit(status) int status; {
	if (vtrmactive)
		endterm();
	exit(status);
}

Visible Procedure initerr() {
	errfile= stderr; /* sp 20010221 */
	still_ok= Yes; interrupted= No; curline= Vnil; curlino= zero;
	err_interactive = f_interactive(stderr);
}

extern bool vtrmactive;

/* error messages to console (via vtrm) if stderr tty and vtrmactive */

Visible Procedure re_errfile() {
	if (err_interactive && vtrmactive)
	  errfile = CONSOLE;
}

/************************************************************************/

#define FMTLENGTH 600

Hidden char *fmtbuf;

Visible Procedure initfmt()
{
	fmtbuf= (char *) getmem(FMTLENGTH);
}

#define FMTINTLEN 100 /* space allocated for int's in formats */

Visible char *getfmtbuf(fmt, n)
     string fmt;
     int n;
{
	static char *fmtstr= NULL;

	n+= strlen(fmt);
	if (fmtstr != NULL)
		freestr(fmtstr);
	if (n >= FMTLENGTH)
		return fmtstr= (char *) getmem((unsigned) n+1);
	return fmtbuf;
}

/**************************************************************************/

Visible Procedure putserr(s)
     string s;
{
	putstr(errfile, s);
}

Hidden Procedure putcerr(c)
     char c;
{
	putchr(errfile, c);
}

Visible Procedure flusherr()
{
	doflush(errfile);
}

/***************************************************************************/

Visible Procedure putsSerr(fmt, s)
     string fmt;
     string s;
{
	char *str= getfmtbuf(fmt, strlen(s));
	sprintf(str, fmt, s);
	putstr(errfile, str);
}

Visible Procedure putsDSerr(fmt, d, s)
     string fmt;
     int d;
     string s;
{
	char *str= getfmtbuf(fmt, FMTINTLEN+strlen(s));
	sprintf(str, fmt, d, s);	
	putstr(errfile, str);
}

Visible Procedure puts2Cerr(fmt, c1, c2)
     string fmt;
     char c1;
     char c2;
{
	char *str= getfmtbuf(fmt, 1+1);
	sprintf(str, fmt, c1, c2);
	putstr(errfile, str);
}

Visible Procedure putsCerr(fmt, c)
     string fmt;
     char c;
{
	puts2Cerr(fmt, c, '\0');
}
