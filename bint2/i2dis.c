/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1990. */

#include "b.h"
#include "bint.h"
#include "bmem.h"
#include "bobj.h"
#include "b0lan.h"
#include "i2par.h"
#include "i2nod.h"

typedef struct {
	FILE *file;       /* display file */
	intlet ilevel;    /* indentation level */
	bufadm line;      /* char adm */
	bool atnwl;       /* at newline */
	bool comment;     /* in comment */
	bool one_line;     /* display one line */
	bool stop;        /* stop display */
	value quote;      /* quote in text displays */
} displadm;

#define Dfile(da)    (da->file)
#define Dilevel(da)  (da->ilevel)
#define Dline(da)    (da->line)
#define Datnwl(da)   (da->atnwl)
#define Dcomment(da) (da->comment)
#define Doneline(da) (da->one_line)
#define Dstop(da)    (da->stop)
#define Dquote(da)   (da->quote)

Forward Hidden Procedure d_linetofile();
Forward Hidden Procedure displ();
Forward Hidden Procedure d_special();
Forward Hidden Procedure indent();
Forward Hidden Procedure d_comment();
Forward Hidden Procedure d_textdis();
Forward Hidden Procedure d_textlit();
Forward Hidden Procedure d_tabdis();
Forward Hidden Procedure d_collateral();
Forward Hidden Procedure d_listdis();
Forward Hidden Procedure d_actfor_compound();
Forward Hidden Procedure d_write();
Forward Hidden Procedure d_dyaf();
Forward Hidden Procedure d_monf();

Hidden Procedure set_ilevel(da)
     displadm *da;
{
	intlet i;
	intlet ilevel = Dilevel(da);

	for (i = 0; i<ilevel; i++) {
		bufcpy(&Dline(da), Indent);
	}
}

Hidden Procedure d_string(s, da)
     string s;
     displadm *da;
{
	if (Datnwl(da) && !Dcomment(da)) set_ilevel(da);
	bufcpy(&Dline(da), s);
	Datnwl(da) = No;
}

Hidden Procedure d_char(c, da)
     char c;
     displadm *da;
{
	if (Datnwl(da) && !Dcomment(da)) set_ilevel(da);
	bufpush(&Dline(da), c);
	Datnwl(da) = No;
}

#define d_space(da)	d_char(' ', da)

Hidden Procedure d_newline(da)
     displadm *da;
{
	d_linetofile(da);
	putnewline(Dfile(da));
	Datnwl(da) = Yes;
}

Hidden Procedure d_linetofile(da)
     displadm *da;
{
	bufadm *bp = &Dline(da);

	if (bp->pbuf == bp->buf) return; /* empty buffer */
	bufpush(bp, '\0');
	putstr(Dfile(da), bp->buf);
	bufreinit(bp);
}

/* ******************************************************************** */

Visible Procedure display(file, v, one_line)
     FILE *file;
     parsetree v;
     bool one_line;
{
	displadm d, *da;

	da = &d;

	Dfile(da)    = file;
	Dilevel(da)  = 0;
	Datnwl(da)   = !one_line;
	Dcomment(da) = No;
	Doneline(da) = one_line;
	Dstop(da)    = No;
	Dquote(da)   = Vnil;

	bufinit(&Dline(da));

	displ((value) v, da);

	if (!Datnwl(da)) d_newline(da);
	d_linetofile(da);

	buffree(&Dline(da));
}

/* ******************************************************************** */

Hidden char *text[NTYPES] = {
	/* HOW_TO */		"HOW TO #h1:#c2#b34",
	/* YIELD */		"HOW TO RETURN 2:#c3#b45",
	/* TEST */		"HOW TO REPORT 2:#c3#b45",
	/* REFINEMENT */	"0:#c1#b23",
	/* SUITE */		"1#c23",

	/* PUT */		"PUT 0 IN 1",
	/* INSERT */		"INSERT 0 IN 1",
	/* REMOVE */		"REMOVE 0 FROM 1",
	/* SET_RANDOM */	"SET RANDOM 0",
	/* DELETE */		"DELETE 0",
	/* CHECK */		"CHECK 0",
	/* SHARE */		"SHARE 0",
	/* PASS */		"PASS",

	/* WRITE */		"WRITE #j",
	/* WRITE1 */		"WRITE #j",
	/* READ */		"READ 0 EG 1",
	/* READ_RAW */		"READ 0 RAW",

	/* IF */		"IF 0:#c1#b2",
	/* WHILE */		"WHILE 1:#c2#b3",
	/* FOR */		"FOR 0 IN 1:#c2#b3",

	/* SELECT */		"SELECT:#c0#b1",
	/* TEST_SUITE */	"1#d:#c2#b34",
	/* ELSE */		"ELSE:#c1#b2",

	/* QUIT */		"QUIT",
	/* RETURN */		"RETURN 0",
	/* REPORT */		"REPORT 0",
	/* SUCCEED */		"SUCCEED",
	/* FAIL */		"FAIL",

	/* USER_COMMAND */	"#h1",

/* the next three are only used when GFX has been defined */
	/* SPACE */		"SPACE FROM a TO b",
	/* LINE */		"LINE FROM a TO b",
	/* CLEAR */		"CLEAR SCREEN",

	/* EXTENDED_COMMAND */	"0 ...",

	/* TAG */		"0",
	/* COMPOUND */		"(0)",
	/* COLLATERAL */	"#a0",
	/* SELECTION */ 	"0[1]",
	/* BEHEAD */		"0@1",
	/* CURTAIL */		"0|1",
	/* UNPARSED */		"1",
	/* MONF */		"#l",
	/* DYAF */		"#k",
	/* NUMBER */		"1",
	/* TEXT_DIS */		"#e",
	/* TEXT_LIT */		"#f",
	/* TEXT_CONV */ 	"`0`1",
	/* ELT_DIS */		"{}",
	/* LIST_DIS */		"{#i0}",
	/* RANGE_BNDS */ 	"0..1",
	/* TAB_DIS */		"{#g0}",
	/* AND */		"0 AND 1",
	/* OR */		"0 OR 1",
	/* NOT */		"NOT 0",
	/* SOME_IN */		"SOME 0 IN 1 HAS 2",
	/* EACH_IN */		"EACH 0 IN 1 HAS 2",
	/* NO_IN */		"NO 0 IN 1 HAS 2",
	/* MONPRD */		"0 1",
	/* DYAPRD */		"0 1 2",
	/* LESS_THAN */ 	"0 < 1",
	/* AT_MOST */		"0 <= 1",
	/* GREATER_THAN */	"0 > 1",
	/* AT_LEAST */		"0 >= 1",
	/* EQUAL */		"0 = 1",
	/* UNEQUAL */		"0 <> 1",
	/* Nonode */		"",

	/* TAGformal */ 	"0",
	/* TAGlocal */		"0",
	/* TAGglobal */ 	"0",
	/* TAGrefinement */	"0",
	/* TAGzerfun */ 	"0",
	/* TAGzerprd */ 	"0",

	/* ACTUAL */		"",
	/* FORMAL */		"",

	/* COLON_NODE */	"0"

};

#define Fld(v, t)     ((value) *Branch(v, (*(t) - '0') + First_fieldnr))

Hidden Procedure displ(v, da)
     value v;
     displadm *da;
{
	string t;
	
	if (!Valid(v)) {
		return;
	}
	else if (Is_text(v)) {
		d_string(strval(v), da);
	}
	else if (Is_parsetree(v)) {
		parsetree w = (parsetree) v;

		t= text[nodetype(w)];
		while (*t) {
			if (isdigit(*t)) {
				displ(Fld(w, t), da);
			}
			else if (*t == '#') {
				d_special(w, &t, da);
				if (Dstop(da)) {
					return;
				}
			}
			else d_char(*t, da);
			t++;
		}
	}
}

Hidden Procedure d_special(v, t, da)
     parsetree v;
     string *t;
     displadm *da;
{
	(*t)++;
	switch (**t) {
	      case 'a':
		d_collateral(Fld(v, ++*t), da);
		break;

	      case 'b':
		indent(Fld(v, ++*t), da);
		break;

	      case 'c':
		d_comment(Fld(v, ++*t), da);
		break;

	      case 'd': /* test suite */
		(*t)++;
		if (!Datnwl(da)) /* there was a command */
		  d_char(**t, da);
		break;

	      case 'e': 
		d_textdis(v, da);
		break;

	      case 'f':
		d_textlit(v, da);
		break;

	      case 'g':
		d_tabdis(Fld(v, ++*t), da);
		break;

	      case 'h':
		d_actfor_compound((parsetree) Fld(v, ++*t), da);
		break;

	      case 'i':
		d_listdis(Fld(v, ++*t), da);
		break;

	      case 'j':
		d_write(v, da);
		break;

	      case 'k':
		d_dyaf(v, da);
		break;

	      case 'l':
		d_monf(v, da);
		break;
	}
}

Hidden Procedure indent(v, da)
     value v;
     displadm *da;
{
	if (Doneline(da)) {
		Dstop(da) = Yes;
		return;
	}
	Dilevel(da)++;
	displ(v, da);
	Dilevel(da)--;
}

Hidden bool no_space_before_comment(v)
     value v;
{
	return ncharval(1, v) == '\\';
}


Hidden Procedure d_comment(v, da)
     value v;
     displadm *da;
{
	if (Valid(v)) {
		Dcomment(da) = Yes;
		if (!Datnwl(da) && no_space_before_comment(v)) {
			d_space(da);
		}
		displ(v, da);
		Dcomment(da) = No;
	}
	if (!Datnwl(da)) d_newline(da);
}

Hidden Procedure d_textdis(v, da)
     parsetree v;
     displadm *da;
{
	value old_quote = Dquote(da);

	Dquote(da) = (value) *Branch(v, XDIS_QUOTE);
	displ(Dquote(da), da);
	displ((value) *Branch(v, XDIS_NEXT), da);
	displ(Dquote(da), da);
	Dquote(da) = old_quote;
}

Hidden Procedure d_textlit(v, da)
     parsetree v;
     displadm *da;
{
	value w = (value) *Branch(v, XLIT_TEXT);

	displ(w, da);
	if (Valid(w) && character(w)) {
		value c = mk_text("`");

		if (compare(Dquote(da), w) == 0 || compare(c, w) == 0) {
			displ(w, da);
		}
		release(c);
	}
	displ((value) *Branch(v, XLIT_NEXT), da);
}

Hidden Procedure d_tabdis(v, da)
     value v;
     displadm *da;
{
	intlet k, len= Nfields(v);

	for (k= 0; k < len; k++) {
		if (k>0) d_string("; ", da);
		d_string("[", da);
		displ(*Field(v, k), da);
		d_string("]: ", da);
		displ(*Field(v, ++k), da);
	}
}

Hidden Procedure d_collateral(v, da)
     value v;
     displadm *da; 
{
	intlet k, len= Nfields(v);

	for (k= 0; k < len; k++) {
		if (k>0) d_string(", ", da);
		displ(*Field(v, k), da);
	}
}

Hidden Procedure d_listdis(v, da)
     value v;
     displadm *da;
{
	intlet k, len= Nfields(v);

	for (k= 0; k < len; k++) {
		if (k>0) d_string("; ", da);
		displ(*Field(v, k), da);
	}
}

Hidden Procedure d_actfor_compound(v, da)
     parsetree v;
     displadm *da;
{
	while (v != NilTree) {
		displ((value) *Branch(v, ACT_KEYW), da);
		if (Valid((value) *Branch(v, ACT_EXPR))) {
			d_space(da);
			displ((value) *Branch(v, ACT_EXPR), da);
		}
		v = *Branch(v, ACT_NEXT);
		if (v != NilTree) d_space(da);
	}
}

Hidden Procedure d_write(v, da)
     parsetree v;
     displadm *da;
{
	value l_lines = (value) *Branch(v, WRT_L_LINES);
	value w = (value) *Branch(v, WRT_EXPR);
	value r_lines = (value) *Branch(v, WRT_R_LINES);

	displ(l_lines, da);
	if (Valid(w)) {
		value n = size(l_lines);
		if (intval(n) > 0) d_space(da);
		release(n);
		displ(w, da);
		n = size(r_lines);
		if (intval(n) > 0) d_space(da);
		release(n);
	}
	displ(r_lines, da);
}

#define is_b_tag(v) (Valid(v) && Letter(ncharval(1, v)))

Hidden Procedure d_dyaf(v, da)
     parsetree v;
     displadm *da;
{
	parsetree l = *Branch(v, DYA_LEFT);
	parsetree r = *Branch(v, DYA_RIGHT);
	value name = (value) *Branch(v, DYA_NAME);

	displ((value) l, da);
	if (is_b_tag(name) || nodetype(r) == MONF) {
		d_space(da);
		displ(name, da);
		d_space(da);
	}
	else displ(name, da);
	displ((value) r, da);
}

Hidden Procedure d_monf(v, da)
     parsetree v;
     displadm *da;
{
	parsetree r = *Branch(v, MON_RIGHT);
	value name = (value) *Branch(v, MON_NAME);

	displ(name, da);
	if (is_b_tag(name)) {
		switch (nodetype(r)) {
			case MONF:
				name = (value) *Branch(r, MON_NAME);
				if (!is_b_tag(name))
					break;
			case SELECTION:
			case BEHEAD:
			case CURTAIL:
			case TAG:
			case TAGformal:
			case TAGlocal:
			case TAGglobal:
			case TAGrefinement:
			case TAGzerfun:
			case TAGzerprd:
			case NUMBER:
			case TEXT_DIS:
				d_space(da);
				break;
			default:
				break;
		}
	}
	displ((value) r, da);
}
