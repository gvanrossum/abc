/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1986. */

#include "b.h"
#include "bint.h"
#include "bmem.h"
#include "bobj.h"
#include "b0lan.h"
#include "i2par.h"
#include "i3scr.h"
#include "i3env.h"

Forward Hidden bool NEXT_keyword();
Forward Hidden bool spec_firstkeyword();

extern bool OPTunpack;

#define TABSIZE 8 /* Number of spaces assumed for a tab on a file.
		     (Some editors insist on emitting tabs wherever
		     they can, and always assume 8 spaces for a tab.
		     Even when the editor can be instructed not to
		     do this, beginning users won't know about this,
		     so we'll always assume the default tab size.
		     Advanced users who used to instruct their editor
		     to set tab stops every 4 spaces will have to
		     unlearn this habit.  But that's the price for
		     over-cleverness :-)
		     The indent increment is still 4 spaces!
		     When the B interpreter outputs text, it never uses
		     tabs but always emits 4 spaces for each indent level.
		     Note that the B editor also has a #defined constant
		     which sets the number of spaces for a tab on a file.
		     Finally the B editor *displays* indents as 3 spaces,
		     but *writes* them to the file as 4, so a neat
		     lay-out on the screen may look a bit garbled
		     when the file is printed.  Sorry.  */

Visible txptr tx, ceol;

Visible Procedure skipsp(tx0) txptr *tx0; {
	while(Space(Char(*tx0))) (*tx0)++;
}

#define Keyletmark(c) \
	(Cap(c) || Dig(c) || (c) == C_APOSTROPHE || (c) == C_QUOTE)

Hidden bool keymark(ty) txptr ty; {
	if (Keyletmark(Char(ty)))
		return Yes;
	else if (Char(ty) == C_POINT &&
			 Keyletmark(Char(ty-1)) && Keyletmark(Char(ty+1)))
		return Yes;
	return No;
}

/* ******************************************************************** */
/*		cr_text							*/
/* ******************************************************************** */

Visible value cr_text(p, q) txptr p, q; {
	/* Messes with the input line, which is a bit nasty,
	   but considered preferable to copying to a separate buffer */
	value t;
	char save= Char(q);
	Char(q)= '\0';
	t= mk_text(p);
	Char(q)= save;
	return t;
}

/* ******************************************************************** */
/*		find, findceol, req, findrel				*/
/* ******************************************************************** */

#define Txnil	((txptr) NULL)

Hidden bool search(find_kw, s, q, ftx, ttx) bool find_kw; string s;
		txptr q, *ftx, *ttx; {
	intlet parcnt= 0; bool outs= Yes, kw= No; char aq;
	txptr lctx= Txnil;
	
	while (*ftx < q) {
		if (outs) {
			if (parcnt == 0) {
				if (find_kw) {
					if (Cap(Char(*ftx)))
						return Yes;
				}
				else if (Char(*ftx) == *s) {
					string t= s+1;
					*ttx= (*ftx)+1;
					while (*t && *ttx < q) {
						if (*t != Char(*ttx)) break;
						else { t++; (*ttx)++; }
					}
					if (*t);
					else if (Cap(*s) &&
						 (kw || keymark(*ttx) ));
					else return Yes;
				}
			}
			switch (Char(*ftx)) {
				case C_OPEN: 
				case C_CUROPEN:
				case C_SUB:
					parcnt++; break;
				case C_CLOSE:
				case C_CURCLOSE:
				case C_BUS:	
					if (parcnt > 0) parcnt--; break;
				case C_APOSTROPHE:
				case C_QUOTE:
					if (lctx == Txnil || !Keytagmark(lctx)) {
						outs= No; aq= Char(*ftx);
					}
					break;
				default:
					break;
			}
			lctx= *ftx;
			if (kw)
				kw= keymark(*ftx);
			else
				kw= Cap(Char(lctx));
		}
		else {
			if (Char(*ftx) == aq)
				{ outs= Yes; kw= No; lctx= Txnil; }
			else if (Char(*ftx) == C_CONVERT) {
				(*ftx)++;
				if (!search(No, S_CONVERT, q, ftx, ttx)) 
					return No;
			}
		}
		(*ftx)++;
	}
	return No;
}

/* ********************************************************************	*/

Visible bool find(s, q, ftx, ttx) string s; txptr q, *ftx, *ttx; {
	return search(No, s, q, (*ftx= tx, ftx), ttx);
}

Forward Hidden txptr lcol();

Visible Procedure findceol() {
	txptr q= lcol(), ttx;
	if (!find(S_COMMENT, q, &ceol, &ttx)) ceol= q;
}

Visible Procedure req(s, q, ftx, ttx) string s; txptr q, *ftx, *ttx; {
	if (!find(s, q, ftx, ttx)) {
		value v= mk_text(s);
		parerrV(MESS(2400, "cannot find expected %s"), v);
		release(v);
		*ftx= tx; *ttx= q;
	}
}

Hidden bool relsearch(s, q, ftx) string s; txptr q, *ftx; {
	txptr ttx;
	*ftx= tx;
	while (search(No, s, q, ftx, &ttx))
		switch (Char(*ftx)) {
			case C_LESS:
				if (Char(*ftx+1) == C_LESS)
					*ftx= ++ttx;
				else if (Char((*ftx)-1) == C_GREATER) 
					*ftx= ttx;
				else return Yes;
				break;
			case C_GREATER:
				if (Char((*ftx)+1) == C_LESS) 
					*ftx= ++ttx;
				else if (Char((*ftx)+1) == C_GREATER) 
					*ftx= ++ttx;
				else return Yes;
				break;
			case C_EQUAL:
				return Yes;
			default:
				return No;
		}
	return No;
}

Visible bool findrel(q, ftx) txptr q, *ftx; {
	txptr ttx;
	*ftx= q;
	if (relsearch(S_LESS, *ftx, &ttx)) *ftx= ttx;
	if (relsearch(S_GREATER, *ftx, &ttx)) *ftx= ttx;
	if (relsearch(S_EQUAL, *ftx, &ttx)) *ftx= ttx;
	return *ftx < q;
}

Visible bool findtrim(q, first) txptr q, *first; {
	txptr ftx, ttx;
	*first= q;
	if (find(S_BEHEAD, *first, &ftx, &ttx)) *first= ftx;
	if (find(S_CURTAIL, *first, &ftx, &ttx)) *first= ftx;
	return *first < q;
}

/* ******************************************************************** */
/*		tag, keyword, findkw					*/
/* ******************************************************************** */

Hidden value tag() {
	txptr tx0= tx;
	if (!Letter(Char(tx))) parerr(MESS(2401, "no name where expected"));
	else while (Tagmark(tx)) tx++;
	return cr_text(tx0, tx);
}

Visible bool is_tag(v) value *v; {
	if (!Letter(Char(tx))) return No;
	*v= tag();
	return Yes;
}

Visible bool is_abcname(name) value name; {
	string s= strval(name);
	
	if (!Letter(*s))
		return No;
	for (; *s != '\0'; ++s) {
		if (!Tagmark(s))
			return No;
	}
	return Yes;
}

Visible char *keyword() {
	txptr tx0= tx;
	static char *kwbuf;
	int len;

	if (!Cap(Char(tx))) parerr(MESS(2402, "no keyword where expected"));
	else while (keymark(tx)) tx++;
	len= tx-tx0;
	if (kwbuf) freemem((ptr) kwbuf);
	kwbuf= (char *) getmem((unsigned) (len+1));
	strncpy(kwbuf, tx0, len);
	kwbuf[len]= '\0';
	return kwbuf;
}

Visible bool is_keyword(kw) char **kw; {
	if (!Cap(Char(tx))) return No;
	*kw= keyword();
	return Yes;
}

Visible bool is_cmdname(q, name) txptr q; char **name; {
	static char *cmdbuf;
	char *kw;
	int len;

	if (!is_keyword(&kw)) return No;
	if (cmdbuf) freemem((ptr) cmdbuf);
	cmdbuf= (char *) savestr(kw);
	if (!spec_firstkeyword(kw)) {
		while (NEXT_keyword(q, &kw)) {
			len= strlen(cmdbuf) + 1 + strlen(kw);
			regetmem((ptr *) &cmdbuf, (unsigned) (len+1));
			strcat(cmdbuf, " ");
			strcat(cmdbuf, kw);
		}
	}
	*name= cmdbuf;
	return Yes;
}

/* only those immediately following the FIRST keyword */

Hidden bool NEXT_keyword(q, kw) txptr q; char **kw; {
	txptr ftx;
	skipsp(&tx);
	if (!findkw(q, &ftx))
		return No;
	if (Text(ftx)) /* there is a parameter */
		return No;
	return is_keyword(kw);
}

/* The reserved keywords that a user command may not begin with:
 * e.g. HOW TO HOW ARE YOU isn't allowed
 */

Hidden char *firstkw[] = {
	K_IF, K_WHILE, K_CHECK, K_HOW, K_RETURN, K_REPORT,
	""
};

Hidden bool spec_firstkeyword(fkw) char *fkw; {
	char **kw;
	for (kw= firstkw; **kw != '\0'; kw++) {
		if (strcmp(fkw, *kw) == 0)
			return Yes;
	}
	return No;
}

Visible bool findkw(q, ftx) txptr q, *ftx; {
	txptr ttx;
	*ftx= tx;
	return search(Yes, "", q, ftx, &ttx);
}

/* ******************************************************************** */
/*		upto, nothing, ateol, need				*/
/* ******************************************************************** */

Visible Procedure upto(q, s) txptr q; string s; {
	skipsp(&tx);
	if (Text(q)) {
		value v= mk_text(s);
		parerrV(MESS(2403, "something unexpected following %s"), v);
		release(v);
		tx= q;
	}
}

Visible Procedure upto1(q, m) txptr q; int m; {
	skipsp(&tx);
	if (Text(q)) {
		parerr(m);
		tx= q;
	}
}

Visible bool nothing(q, m) txptr q; int m; {
	if (!Text(q)) {
		if (Char(tx-1) == ' ') tx--;
		parerr(m);
		return Yes;
	}
	return No;
}

Visible bool i_looked_ahead= No;
Hidden  bool o_looked_ahead= No;

Visible intlet cur_ilev;

Visible bool ateol() {
	if ((ifile == sv_ifile && i_looked_ahead)
	    || (ifile != sv_ifile && o_looked_ahead)) return Yes;
	skipsp(&tx);
	return Eol(tx);
}

Visible Procedure need(s) string s; {
	string t= s;
	skipsp(&tx);
	while (*t)
		if (*t++ != Char(tx++)) {
			value v= mk_text(s);
			tx--;
		parerrV(MESS(2404, "according to the syntax I expected %s"), v);
			release(v);
			return;
		}
}

/* ******************************************************************** */
/*		buffer handling						*/
/* ******************************************************************** */

Visible txptr first_col;

Visible txptr fcol() { /* the first position of the current line */
	return first_col;
}

Hidden txptr lcol() { /* the position beyond the last character of the line */
	txptr ax= tx;
	while (!Eol(ax)) ax++;
	return ax;
}

Visible intlet ilev() {
	intlet i;
	if (ifile == sv_ifile && i_looked_ahead) {
		if (!interactive && ifile == sv_ifile) 
			f_lino++;
		i_looked_ahead= No;
		return cur_ilev;
	}
	else if (ifile != sv_ifile && o_looked_ahead) {
		if (OPTunpack)
			f_lino++;
		o_looked_ahead= No;
		return cur_ilev;
	}
	else {
		first_col= tx= get_line();
		if (ifile == sv_ifile)
			i_looked_ahead= No;
		else
			o_looked_ahead= No;
		lino++;
		if (!interactive && ifile == sv_ifile)
			f_lino++;
		i= 0;
		while (Space(Char(tx))) {
			if (Char(tx++) == ' ') i++;
			else i= (i/TABSIZE+1)*TABSIZE;
		}
		if (Char(tx) == C_COMMENT) return cur_ilev= 0;
		if (Char(tx) == '\n') return cur_ilev= 0;
		return cur_ilev= i;
	}
}

Visible Procedure veli() { /* After a look-ahead call of ilev */
	if (!interactive && ifile == sv_ifile || OPTunpack)
		f_lino--;
	if (ifile == sv_ifile)
		i_looked_ahead= Yes;
	else
		o_looked_ahead= Yes;
}

Visible Procedure first_ilev() { /* initialise read buffer for new input */
	o_looked_ahead= No;
	VOID ilev();
	findceol();
}

/* ********************************************************************	*/

Visible value res_cmdnames;

/* The reserved command names;
 * e.g. HOW TO PUT IN x is allowed, but HOW TO PUT x OUT isn't
 */

Hidden string reserved[] = {
	K_SHARE, K_CHECK, K_DELETE, K_FAIL, K_FOR,
	K_HOW, K_IF, K_INSERT, K_PASS, K_PUT, K_QUIT, K_READ, K_REMOVE,
	K_REPORT, K_RETURN, K_SELECT, K_SETRANDOM, K_SUCCEED,
	K_WHILE, K_WRITE,
#ifdef GFX
	K_SPACEFROM, K_LINEFROM, K_CLEARSCREEN,
#endif
	""
};

Visible Procedure initsyn() {
	value v;
	string *kw;
	
	res_cmdnames= mk_elt();
	for (kw= reserved; **kw != '\0'; kw++) {
		insert(v= mk_text(*kw), &res_cmdnames);
		release(v);
	}
}

Visible Procedure endsyn() {
#ifdef MEMTRACE
	release(res_cmdnames); res_cmdnames= Vnil;
#endif
}

/* ******************************************************************** */
/*		signs							*/
/* ********************************************************************	*/

Hidden bool la_denum(tx0) txptr tx0; {
	char l, r;
	switch (l= Char(++tx0)) {
		case C_OVER:	r= C_TIMES; break;
		case C_TIMES:	r= C_OVER; break;
		default:	return Yes;
	}
	do if (Char(++tx0) != r) return No; while (Char(++tx0) == l);
	return Yes;
}

Visible bool _nwl_sign() {
	if (_sign_is(C_NEWLINE))
		return !la_denum(tx-2) ? Yes : (tx--, No);
	return No;
}

Visible bool _times_sign() {
	if (_sign_is(C_TIMES))
		return la_denum(tx-1) ? Yes : (tx--, No);
	return No;
}

Visible bool _over_sign() {
	if (_sign_is(C_OVER))
		return la_denum(tx-1) ? Yes : (tx--, No);
	return No;
}

Visible bool _power_sign() {
	if (_sign2_is(S_POWER))
		return la_denum(tx-1) ? Yes : (tx-= 2, No);
	return No;
}

Visible bool _numtor_sign() {
	if (_sign2_is(S_NUMERATOR))
		return la_denum(tx-1) ? Yes : (tx-= 2, No);
	return No;
}

Visible bool _denomtor_sign() {
	if (_sign2_is(S_DENOMINATOR))
		return la_denum(tx-1) ? Yes : (tx-= 2, No);
	return No;
}

Visible bool _join_sign() {
	if (_sign_is(C_JOIN))
		return !_sign_is(C_JOIN) ? Yes : (tx-= 2, No);
	return No;
}

Visible bool _less_than_sign() {
	if (_sign_is(C_LESS))
		return !_sign_is(C_LESS) && !_sign_is(C_EQUAL)
			&& !_sign_is(C_GREATER) ? Yes : (tx--, No);
	return No;
}

Visible bool _greater_than_sign() {
	if (_sign_is(C_GREATER))
		return !_sign_is(C_LESS) && !_sign_is(C_EQUAL)
			&& !_sign_is(C_GREATER)  ? Yes : (tx--, No);
	return No;
}

Visible bool dyamon_sign(v) value *v; {
	string s;
	if (plus_sign) s= S_PLUS;
	else if (minus_sign) s= S_MINUS;
	else if (number_sign) s= S_NUMBER;
	else return No;
	*v= mk_text(s);
	return Yes;
}

Visible bool dya_sign(v) value *v; {
	string s;
	if (times_sign) s= S_TIMES;
	else if (over_sign) s= S_OVER;
	else if (power_sign) s= S_POWER;
	else if (behead_sign) s= S_BEHEAD;
	else if (curtl_sign) s= S_CURTAIL;
	else if (join_sign) s= S_JOIN;
	else if (reptext_sign) s= S_REPEAT;
	else if (leftadj_sign) s= S_LEFT_ADJUST;
	else if (center_sign) s= S_CENTER;
	else if (rightadj_sign) s= S_RIGHT_ADJUST;
	else return No;
	*v= mk_text(s);
	return Yes;
}

Visible bool mon_sign(v) value *v; {
	string s;
	if (about_sign) s= S_ABOUT;
	else if (numtor_sign) s= S_NUMERATOR;
	else if (denomtor_sign) s= S_DENOMINATOR;
	else return No;
	*v= mk_text(s);
	return Yes;
}

Visible bool texdis_sign(v) value *v; {
	string s;
	if (apostrophe_sign) s= S_APOSTROPHE;
	else if (quote_sign) s= S_QUOTE;
	else return No;
	*v= mk_text(s);
	return Yes;
}
