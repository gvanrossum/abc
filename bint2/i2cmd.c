/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1986. */

#include "b.h"
#include "bint.h"
#include "bobj.h"
#include "i0err.h"
#include "b0lan.h"
#include "i2par.h" 
#include "i2nod.h"
#include "i3env.h"

Forward Hidden bool chk_indent();
Forward Hidden Procedure suite_command();
Forward Hidden bool bas_com();
Forward Hidden value cr_newlines();
Forward Hidden bool udr_com();
Forward Hidden value hu_actuals();
Forward Hidden parsetree alt_suite();
Forward Hidden parsetree alt_seq();

/* ******************************************************************** */
/*		command_suite						*/
/* ******************************************************************** */

Visible parsetree cmd_suite(cil, first, suite) intlet cil; bool first;
		parsetree (*suite)(); {
	parsetree v= NilTree;
	
	if (ateol()) {
		bool emp= Yes;

		v= (*suite)(cil, first, &emp);
		if (emp) parerr(MESS(2000, "no command suite where expected"));
		return v;
	}
	else {
		value c= Vnil;
		intlet l= lino;
		
		suite_command(&v, &c);
		return node5(SUITE, mk_integer(l), v, c, NilTree);
	}
}

Visible parsetree cmd_seq(cil, first, emp) intlet cil; bool first, *emp; {
	value c= Vnil;
	intlet level, l;
	
	level= ilev(); l= lino;
	if (is_comment(&c)) 
		return node5(SUITE, mk_integer(l), NilTree, c,
				cmd_seq(cil, first, emp));
	if (chk_indent(level, cil, first)) {
		parsetree v= NilTree;
		
		findceol();
		suite_command(&v, &c);
		*emp= No;
		return node5(SUITE, mk_integer(l), v, c, cmd_seq(level, No, emp));
	}
	veli();
	return NilTree;
}

Hidden bool chk_indent(nlevel, olevel, first) intlet nlevel, olevel;
		bool first; {
	if (nlevel > olevel) {
		if (!first) parerr(WRONG_INDENT);
		else if (nlevel - olevel == 1) parerr(SMALL_INDENT);
		return Yes;
	}
	return nlevel == olevel && !first ? Yes : No;
}

Hidden Procedure suite_command(v, c) parsetree *v; value *c; {
	char *kw;
	
	if (!is_cmdname(ceol, &kw) || !control_command(kw, v) && 
			!simple_command(kw, v, c) ) 
		parerr(MESS(2001, "no command where expected"));
}

/* ******************************************************************** */
/*		is_comment, tail_line					*/
/* ******************************************************************** */

Visible bool is_comment(v) value *v; {
	txptr tx0= tx;
	skipsp(&tx);
	if (comment_sign) {
		while (Space(Char(tx0-1))) tx0--;
		while (!Eol(tx)) tx++;
		*v= cr_text(tx0, tx);
		return Yes;
	}
	tx= tx0;
	return No;
}

Visible value tail_line() {
	value v;
	if (is_comment(&v)) return v;
	if (!ateol()) parerr(MESS(2002, "something unexpected in this line"));
	return Vnil;
}

/* ******************************************************************** */
/*		simple_command						*/
/*									*/
/* ******************************************************************** */

Visible bool simple_command(kw, v, c) char *kw; parsetree *v; value *c; {
	return bas_com(kw, v) || term_com(kw, v) || udr_com(kw, v)
		? (*c= tail_line(), Yes) : No;
}

/* ******************************************************************** */
/*		basic_command						*/
/* ******************************************************************** */

Hidden bool bas_com(kw, v) char *kw; parsetree *v; {
	parsetree w, t;
	txptr ftx, ttx; 

	if (check_keyword(kw)) {			/* CHECK */
		*v= node2(CHECK, test(ceol));
	}
	else if (delete_keyword(kw))			/* DELETE */
		*v= node2(DELETE, targ(ceol));
	else if (insert_keyword(kw)) {			/* INSERT */
		req(K_IN_insert, ceol, &ftx, &ttx);
		w= expr(ftx); tx= ttx;
		*v= node3(INSERT, w, targ(ceol));
	}
	else if (pass_keyword(kw)) {			/* PASS */
		upto(ceol, K_PASS);
		*v= node1(PASS);
	}
	else if (put_keyword(kw)) {			/* PUT */
		req(K_IN_put, ceol, &ftx, &ttx);
		w= expr(ftx); tx= ttx;
		*v= node3(PUT, w, targ(ceol));
	}
	else if (read_keyword(kw)) {			/* READ */
		if (find(K_RAW, ceol, &ftx, &ttx)) {
			*v= node2(READ_RAW, targ(ftx)); tx= ttx;
			upto(ceol, K_RAW);
		} 
		else {
			req(K_EG, ceol, &ftx, &ttx);
			t= targ(ftx); tx= ttx;
			*v= node3(READ, t, expr(ceol));
		}
	}
	else if (remove_keyword(kw)) {			/* REMOVE */
		req(K_FROM_remove, ceol, &ftx, &ttx);
		w= expr(ftx); tx= ttx;
		*v= node3(REMOVE, w, targ(ceol));
	}
	else if (setrandom_keyword(kw)) 		/* SET RANDOM */
		*v= node2(SET_RANDOM, expr(ceol));
	else if (write_keyword(kw)) {			/* WRITE */
		intlet b_cnt= 0, a_cnt= 0;
		
		skipsp(&tx);
		if (Ceol(tx))
			parerr(MESS(2003, "no parameter where expected"));
		while (nwl_sign) {b_cnt++; skipsp(&tx); }
		if (Ceol(tx)) w= NilTree;
		else {
			ftx= ceol;
			while (Space(Char(ftx-1)) || Char(ftx-1) == '/')
				if (Char(--ftx) == '/') a_cnt++;
			skipsp(&tx);
			w= ftx > tx ? expr(ftx) : NilTree;
		}
		*v= node4(w == NilTree || Nodetype(w) != COLLATERAL
			? WRITE1 : WRITE,
			cr_newlines(b_cnt), w, cr_newlines(a_cnt));
		tx= ceol;
#ifdef GFX
	}
	else if (spacefrom_keyword(kw)) {		/* SPACE FROM */
		req(K_TO_space, ceol, &ftx, &ttx);
		w= expr(ftx); tx= ttx;
		*v= node3(SPACE, w, expr(ceol));
	}
	else if (linefrom_keyword(kw)) {		/* LINE FROM */
		req(K_TO_line, ceol, &ftx, &ttx);
		w= expr(ftx); tx= ttx;
		*v= node3(LINE, w, expr(ceol));
	}
	else if (clearscreen_keyword(kw)) {		/ CLEAR SCREEN */
		upto(ceol, K_CLEARSCREEN);
		*v= node1(CLEAR);
#endif
	}
	else return No;
	return Yes;
}

Hidden value cr_newlines(cnt) intlet cnt; {
	value v, t= mk_text(S_NEWLINE), n= mk_integer(cnt);
	v= repeat(t, n);
	release(t); release(n);
	return v;
}

/* ******************************************************************** */
/*		terminating_command					*/
/* ******************************************************************** */

Visible bool term_com(kw, v) char *kw; parsetree *v; {
	if (fail_keyword(kw)) {				/* FAIL */
		upto(ceol, K_FAIL);
		*v= node1(FAIL);
	}
	else if (quit_keyword(kw)) {			/* QUIT */
		upto(ceol, K_QUIT);
		*v= node1(QUIT);
	}
	else if (return_keyword(kw))			/* RETURN */
		*v= node2(RETURN, expr(ceol));
	else if (report_keyword(kw))			/* REPORT */
		*v= node2(REPORT, test(ceol));
	else if (succeed_keyword(kw)) {			/* SUCCEED */
		upto(ceol, K_SUCCEED);
		*v= node1(SUCCEED);
	}
	else return No;
	return Yes;
}

/* ******************************************************************** */
/*		user_defined_command; refined_command			*/
/* ******************************************************************** */

Hidden bool udr_com(kw, v) char *kw; parsetree *v; {
	value w= mk_text(kw);
	
	if (!in(w, res_cmdnames)) {
		*v= node4(USER_COMMAND, copy(w), hu_actuals(ceol, w), Vnil);
		return Yes;
	}
	release(w);
	return No;
}

Hidden value hu_actuals(q, kw) txptr q; value kw; {
	parsetree t= NilTree;
	value v= Vnil, nkw;
	txptr ftx;
	
	skipsp(&tx);
	if (!findkw(q, &ftx))
		ftx= q;
	if (Text(ftx))
		t= expr(ftx);
	if (Text(q)) {
		nkw= mk_text(keyword());
		v= hu_actuals(q, nkw);
	}
	return node4(ACTUAL, kw, t, v);
}

/* ******************************************************************** */
/*		control_command						*/
/* ******************************************************************** */

Visible bool control_command(kw, v) char *kw; parsetree *v; {
	parsetree s, t; 
	value c;
	txptr ftx, ttx, utx, vtx;
	
	skipsp(&tx);
	if (if_keyword(kw)) {				/* IF */
		req(S_COLON, ceol, &utx, &vtx);
		t= test(utx); tx= vtx;
		if (!is_comment(&c)) c= Vnil;
		*v= node4(IF, t, c, cmd_suite(cur_ilev, Yes, cmd_seq));
	}
	else if (select_keyword(kw)) {			/* SELECT */
		need(S_COLON);
		c= tail_line();
		*v= node3(SELECTNODE, c, alt_suite());
	}
	else if (while_keyword(kw)) {			/* WHILE */
		intlet l= lino;
		
		req(S_COLON, ceol, &utx, &vtx);
		t= test(utx); tx= vtx;
		if (!is_comment(&c)) c= Vnil;
		s= node2(COLON_NODE, cmd_suite(cur_ilev, Yes, cmd_seq));
		*v= node5(WHILE, mk_integer(l), t, c, s);
	}
	else if (for_keyword(kw)) {			/* FOR */
		req(S_COLON, ceol, &utx, &vtx);
		req(K_IN_for, ceol, &ftx, &ttx);
		if (ttx > utx) {
			parerr(MESS(2005, "IN after colon"));
			ftx= utx= tx; ttx= vtx= ceol;
		}
		idf_cntxt= In_ranger;
		t= idf(ftx); tx= ttx;
		s= expr(utx); tx= vtx;
		if (!is_comment(&c)) c= Vnil;
		*v= node5(FOR, t, s, c, cmd_suite(cur_ilev, Yes, cmd_seq));
	}
	else return No;
	return Yes;
}

/* ******************************************************************** */
/*		alternative_suite					*/
/* ******************************************************************** */

Hidden parsetree alt_suite() {
	parsetree v;
	bool emp= Yes;
	 
	v= alt_seq(cur_ilev, Yes, No, &emp);
	if (emp) parerr(MESS(2006, "no alternative suite for SELECT"));
	return v;
}

Hidden parsetree alt_seq(cil, first, else_encountered, emp) 
		bool first, else_encountered, *emp; intlet cil; {
	value c;
	intlet level, l;
	char *kw;
	
	level= ilev(); l= lino;
	if (is_comment(&c)) 
		return node6(TEST_SUITE, mk_integer(l), NilTree, c,
				node2(COLON_NODE, NilTree),
				alt_seq(cil, first, else_encountered, emp));
	if (chk_indent(level, cil, first)) {
		parsetree v, s;
		txptr ftx, ttx, tx0= tx;
		
		if (else_encountered)
			parerr(MESS(2007, "after ELSE no more alternatives allowed"));
		findceol();
		req(S_COLON, ceol, &ftx, &ttx);
		*emp= No;
		if (is_keyword(&kw) && else_keyword(kw)) {
			upto(ftx, K_ELSE); tx= ttx;
			if (!is_comment(&c)) c= Vnil;
			s= cmd_suite(level, Yes, cmd_seq);
			release(alt_seq(level, No, Yes, emp));
			return node4(ELSE, mk_integer(l), c, s);
		}
		else tx= tx0;
		v= test(ftx); tx= ttx;
		if (!is_comment(&c)) c= Vnil;
		s= node2(COLON_NODE, cmd_suite(level, Yes, cmd_seq));
		return node6(TEST_SUITE, mk_integer(l), v, c, s,
				alt_seq(level, No, else_encountered, emp));
	}
	veli();
	return NilTree;
}
