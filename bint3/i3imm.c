/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1986. */

#include "b.h"
#include "bint.h"
#include "bobj.h"
#include "b0lan.h"
#include "i2par.h"
#include "i3env.h"
#include "i3scr.h"

/* ******************************************************************** */
/*		immediate command					*/
/* ******************************************************************** */

Forward Hidden Procedure special();

#define TERM_COMMAND	MESS(3300, "terminating commands only allowed in how-to's and refinements")
#define SHARE_COMMAND	MESS(3301, "share-command only allowed in a how-to")
#define NO_COMMAND	MESS(3302, "I don't recognise this as a command")

Hidden Procedure imm_command() {
	parsetree codeseq= NilTree;
	parsetree c= NilTree, d= NilTree; 
	int level;
	char *kw;
	txptr tx0;
	
	cntxt= In_command; still_ok= Yes; interrupted= No;
	can_interrupt= Yes;
	terminated= No;
	resexp= Voi; lino= 0;
	level= ilev();
	if (!still_ok) return;
	if (level > 0)
		parerr(MESS(3303, "outer indentation not zero"));
	else if (findceol(), Ceol(tx));
	else if (Char(tx) ==C_COLON || Char(tx) == C_EQUAL ||
			Char(tx) == C_GREATER) {
		if (interactive) special();
		else parerr(MESS(3304, "special commands only interactively"));
	}
	else if (tx0= tx, is_cmdname(ceol, &kw)) {
		if (how_keyword(kw)) {
			tx= tx0;
			create_unit();
		}
		else if (quit_keyword(kw))
			terminated= Yes;
		else if (term_com(kw, &c)) {
			release(c);
			parerr(TERM_COMMAND);
		}
		else if (share_keyword(kw))
			parerr(SHARE_COMMAND);
		else if (control_command(kw, &c) ||
				 simple_command(kw, &c, &d)) {
			/* control_command MUST come before simple above */
			if (still_ok) fix_nodes(&c, &codeseq);
			curline= c; curlino= one;
			execthread(codeseq);
			release(c); release(d);
		}
		else parerr(NO_COMMAND);
	}
	else parerr(NO_COMMAND);
}

Visible Procedure process() {
	re_screen();
	re_env();
	f_lino= 0;
	terminated= No;
	while (!Eof && !terminated) {
		imm_command();
		if (!interactive && !still_ok) bye(1);
	}
}

Hidden Procedure special() {
	switch(Char(tx++)) {
		case ':':       skipsp(&tx);
				if (Char(tx) == C_COLON) {
					lst_uhds();
				}
				else {
					edit_unit();
				}
				break;
		case '=':       skipsp(&tx);
				if (Char(tx) == C_EQUAL) {
					lst_ttgs();
				}
				else {
					edit_target();
				}
				break;
		case '>':       skipsp(&tx);
				if (Char(tx) == C_GREATER) {
					lst_wss();
				}
				else {
					goto_ws();
				}
				break;
		default:	syserr(MESS(3305, "special"));
	}
}

