/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1986. */

/* process type unification errors */

#include "b.h"
#include "bmem.h"
#include "bobj.h"
#include "i2stc.h"

#define FOUND_EG	GMESS(2600, "I found type EG %s where I expected %s")
#define FOUND		GMESS(2601, "I found type %s where I expected %s")

#define THOUGHT_EG	GMESS(2602, "I thought %s was of type EG %s")
#define THOUGHT		GMESS(2603, "I thought %s was of type %s")

#define INCOMPATIBLE	GMESS(2604, "incompatible type for %s")

#define LT_OF		GMESS(2605, "list or table of ")
#define LT		GMESS(2606, "list or table")
#define T_OR_LT_OF_T	GMESS(2607, """, or list or table of """)
#define TLT		GMESS(2608, "text or list or table")

/* 
 * The variables from the users line are inserted in var_list.
 * This is used to produce the right variable names
 * in the error message.
 * Call start_vars() when a new error context is established
 * with the setting of curline.
 */

Hidden value var_list;

Visible Procedure start_vars() {
	var_list = mk_elt();
}

Visible Procedure add_var(tvar) polytype tvar; {
	insert(tvar, &var_list);
}

Hidden bool in_vars(t) polytype t; {
	return in(t, var_list);
}

Visible Procedure end_vars() {
	release(var_list);
}

/* t_repr(u) is used to print polytypes when an error
 * has occurred.
 * Because the errors are printed AFTER unification, the variable 
 * polytypes in question have changed to the error-type.
 * To print the real types in error, the table has to be 
 * saved in reprtable.
 * The routines are called in unify().
 */

Hidden value reprtable;
extern value ptype_of; 		/* defined in i2tp.c */

Visible Procedure setreprtable() {
	reprtable = copy(ptype_of);
}

Visible Procedure delreprtable() {
	release(reprtable);
}

/* variables whose type is in error are gathered in errvarlist */

Hidden value errvarlist;

Visible Procedure starterrvars() {
	errvarlist= mk_elt();
}

Visible Procedure adderrvar(t) polytype t; {
	if (in_vars(t) && !in(t, errvarlist))
		insert(t, &errvarlist);
}

Visible Procedure enderrvars() {
	release(errvarlist);
}

/* miscellaneous procs */

Visible value conc(v, w) value v, w; {
	value c;
	c = concat(v, w);
	release(v); release(w);
	return c;
}

Hidden bool newvar(u) polytype u; {
	value u1;
	char ch;
	u1 = curtail(ident(u), one);
	ch = charval(u1);
	release(u1);
	return (bool) ('0' <= ch && ch <= '9');
}

#define Known(tu) (!t_is_var(kind(tu)) && !t_is_error(kind(tu)))

Hidden polytype oldbottomtype(u) polytype u; {
	polytype tu= u;
	while (t_is_var(kind(tu)) && in_keys(ident(tu), reprtable))
		tu= *adrassoc(reprtable, ident(tu));
	return tu; /* not a copy, just a pointer! */
}

Hidden value t_repr(u) polytype u; {
	typekind u_kind;
	polytype tau;
	value c;
	
	u_kind = kind(u);
	if (t_is_number(u_kind)) {
		return mk_text("0");
	}
	else if (t_is_text(u_kind)) {
		return mk_text("\"\"");
	}
	else if (t_is_tn(u_kind)) {
		return mk_text("\"\" or 0");
	}
	else if (t_is_compound(u_kind)) {
		intlet k, len = nsubtypes(u);
		c = mk_text("(");
		for (k = 0; k < len - 1; k++) {
			c = conc(c, t_repr(subtype(u, k)));
			c = conc(c, mk_text(", "));
		}
		c = conc(c, t_repr(subtype(u, k)));
		return conc(c, mk_text(")"));
	}
	else if (t_is_error(u_kind)) {
		return mk_text("?");
	}
	else if (t_is_var(u_kind)) {
		value tu;
		tu = oldbottomtype(u);
		if (Known(tu))
			return t_repr(tu);
		else if (newvar(u))
			return mk_text("?");
		else
			return copy(ident(u));
	}
	else if (t_is_table(u_kind)) {
		c = conc(mk_text("{["),
			t_repr(keytype(u)));
		c = conc(c, mk_text("]: "));
		c = conc(c, t_repr(asctype(u)));
		return conc(c, mk_text("}"));
	}
	else if (t_is_list(u_kind)) {
		c = conc(mk_text("{"), t_repr(asctype(u)));
		return conc(c, mk_text("}"));
	}
	else if (t_is_lt(u_kind)) {
		tau = oldbottomtype(asctype(u));
		if (Known(tau))
			return conc(mk_text(LT_OF),
				    t_repr(tau));
		else
			return mk_text(LT);
	}
	else if (t_is_tlt(u_kind)) {
		tau= oldbottomtype(asctype(u));
		if (Known(tau)) {
			if (t_is_text(kind(tau)))
				return mk_text(T_OR_LT_OF_T);
			else
				return conc(mk_text(LT_OF), t_repr(tau));
		}
		else
			return mk_text(TLT);
	}
	else {
		return mk_text("***"); /* cannot happen */
	}
}

Hidden value typmess(format, s1, s2) string format, s1, s2; {
	unsigned len;
	string bf;
	value m;

	len= strlen(format)+strlen(s1)+strlen(s2)+10;
	bf= (string)getmem(len);
	sprintf(bf, format, s1, s2);
	m= mk_text(bf);
	freemem(bf);
	return m;
}

/* now, the real error messages */

Visible Procedure badtyperr(a, b) polytype a, b; {
	value t;
	value nerrs;
	polytype te, bte;
	string sa= NULL;
	string sb= NULL;
	value ta= Vnil;
	value tb= Vnil;

	nerrs= size(errvarlist);
	
	if (compare(nerrs, one) < 0) {
		sa= sstrval(ta=t_repr(a));
		sb= sstrval(tb=t_repr(b));
		if (!has_lt(kind(a)))
			t= typmess(FOUND_EG, sa, sb);
		else
			t= typmess(FOUND, sa, sb);
	}
	else {
		te= (polytype) item(errvarlist, one);
		bte= oldbottomtype(te);
		sa= sstrval(ta=copy(ident(te)));
		sb= sstrval(tb=t_repr(bte));
		if (Known(bte) && !has_lt(kind(bte))) {
			t= typmess(THOUGHT_EG, sa, sb);
		}
		else {
			t= typmess(INCOMPATIBLE, sa, "");
		}
	}

	release(nerrs);
	fstrval(sa); fstrval(sb); release(ta); release(tb);

	typerrV(MESS(2609, "%s"), t);
	release(t);
}

#ifdef TYPETRACE
#include "i2nod.h"
char *treename[NTYPES] = { /* legible names for debugging */
	"HOW TO",
	"HOW TO RETURN",
	"HOW TO REPORT",
	"REFINEMENT",

/* Commands */

	"SUITE",
	"PUT",
	"INSERT",
	"REMOVE",
	"SET RANDOM",
	"DELETE",
	"CHECK",
	"SHARE",
	"PASS",

	"WRITE",
	"WRITE1",
	"READ",
	"READ_RAW",

	"IF",
	"WHILE",
	"FOR",

	"SELECT",
	"TEST_SUITE",
	"ELSE",

	"QUIT",
	"RETURN",
	"REPORT",
	"SUCCEED",
	"FAIL",

	"USER_COMMAND",

/* the next three are only used when GFX has been defined */
	"SPACE",
	"LINE",
	"CLEAR",

	"EXTENDED_COMMAND",

/* Expressions, targets, tests */

	"TAG",
	"COMPOUND",

/* Expressions, targets */

	"COLLATERAL",
	"SELECTION",
	"BEHEAD",
	"CURTAIL",

/* Expressions, tests */

	"UNPARSED",

/* Expressions */

	"MONF",
	"DYAF",
	"NUMBER",
	"TEXT_DIS",
	"TEXT_LIT",
	"TEXT_CONV",
	"ELT_DIS",
	"LIST_DIS",
	"RANGE_BNDS",
	"TAB_DIS",

/* Tests */

	"AND",
	"OR",
	"NOT",
	"SOME_IN",
	"EACH_IN",
	"NO_IN",
	"MONPRD",
	"DYAPRD",
	"LESS_THAN",
	"AT_MOST",
	"GREATER_THAN",
	"AT_LEAST",
	"EQUAL",
	"UNEQUAL",
	"Nonode",

	"TAGformal",
	"TAGlocal",
	"TAGglobal",
	"TAGrefinement",
	"TAGzerfun",
	"TAGzerprd",

	"ACTUAL",
	"FORMAL",

	"COLON_NODE",

};

extern FILE *stc_fp;

Visible Procedure t_typecheck(nt, t) int nt; string t; {
	if (stc_fp == NULL)
		return;
	fprintf(stc_fp, "TC NODE %s, CODE %s\n", treename[nt], t);
	VOID fflush(stc_fp);
}

Visible Procedure s_unify(a, b) polytype a, b; {
	value t;
	
	if (stc_fp == NULL)
		return;
	t= mk_text("START UNIFY ");
	if (t_is_var(kind(a))) {
		t= conc(t, copy(ident(a)));
		t= conc(t, mk_text("="));
	}
	t= conc(t, convert((value)oldbottomtype(a), No, No));
	t= conc(t, mk_text(" WITH "));
	if (t_is_var(kind(b))) {
		t= conc(t, copy(ident(b)));
		t= conc(t, mk_text("="));
	}
	t= conc(t, convert((value)oldbottomtype(b), No, No));
	fprintf(stc_fp, "%s\n", strval(t));
	release(t);
	t= mk_text("USING ");
	t= conc(t, convert(ptype_of, No, No));
	fprintf(stc_fp, "%s\n", strval(t));
	release(t);
	VOID fflush(stc_fp);
}

Visible Procedure e_unify(c) polytype c; {
	value t;
	
	if (stc_fp == NULL)
		return;
	t= mk_text("GIVING ");
	if (t_is_var(kind(c))) {
		t= conc(t, copy(ident(c)));
		t= conc(t, mk_text("="));
	}
	t= conc(t, convert((value)oldbottomtype(c), No, No));
	fprintf(stc_fp, "%s\n", strval(t));
	release(t);
	t= mk_text("PRODUCING ");
	t= conc(t, convert(ptype_of, No, No));
	fprintf(stc_fp, "%s\n", strval(t));
	release(t);
	VOID fflush(stc_fp);
}
#endif /* TYPETRACE */
