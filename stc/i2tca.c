/* Copyright (c) Stichting Mathematisch Centrum, amsterdam, 1988. */

/* ABC type check */

#include "b.h"
#include "bmem.h"
#include "bfil.h"
#include "bint.h"
#include "bobj.h"
#include "b0lan.h"
#include "i2nod.h"
#include "i2par.h"
#include "i2stc.h"
#include "i3bws.h"
#include "i3cen.h"
#include "i3env.h"	/* for curline and curlino */
#include "i3sou.h"
#include "port.h"

Forward Hidden Procedure tc_node();
Forward Hidden Procedure put_code();
Forward Hidden Procedure set_ret_name();
Forward Hidden Procedure pts_init();
Forward Hidden Procedure pts_free();
Forward Hidden Procedure pt_push();

#define WRONG_ARGUMENT	MESS(2300, "wrong argument of type_check()")
#define WARNING_DUMMY	MESS(2301, "next line must be impossible as a refinement name, e.g. with a space:")
#define RETURNED_VALUE	GMESS(2302, "returned value")
#define WRONG_RETURN	MESS(2303, "RETURN not in function or expression refinement")
#define EMPTY_STACK	MESS(2304, "Empty polytype stack")

/* ******************************************************************** */

char *tc_code[NTYPES] = {	/* Type checker table; */
				/* see comment below for meaning of codes */
/* How-to's */

	/* HOW_TO */ "-s-csH",
	/* YIELD */ "--p-YcysF",
	/* TEST */ "--p-csP",
	/* REFINEMENT */ "--Rcys",

/* Commands */

	/* SUITE */ "Lc-c",
	/* PUT */ "eeU",
	/* INSERT */ "e}eU",
	/* REMOVE */ "e}eU",
	/* SET_RANDOM */ "e*",
	/* DELETE */ "e*",
	/* CHECK */ "t*",
	/* SHARE */ "",
	/* PASS */ "",

	/* WRITE */ "-?e*",
	/* WRITE1 */ "-?e*",
	/* READ */ "eeU",
	/* READ_RAW */ "e'U",

	/* IF */ "t*-c",
	/* WHILE */ "Lt*-c",
	/* FOR */ "e#eU-c",

	/* SELECT */ "-c",
	/* TEST_SUITE */ "L?t*-cc",
	/* ELSE */ "L-c",

	/* QUIT */ "",
	/* RETURN */ "erU",
	/* REPORT */ "t*",
	/* SUCCEED */ "",
	/* FAIL */ "",

	/* USER_COMMAND */ "A-sC",

/* the next three are only used when GFX has been defined */
	/* SPACE */ "eeU",
	/* LINE */ "eeU",
	/* CLEAR */ "",

	/* EXTENDED_COMMAND */ "",

/* Expressions, targets, tests */

	/* TAG */ "T",
	/* COMPOUND */ "e",

/* Expressions, targets */

	/* COLLATERAL */ ":(<e,>)",
	/* SELECTION */ "we~e~]U",
	/* BEHEAD */ "e'UenU'",
	/* CURTAIL */ "e'UenU'",

/* Expressions, tests */

	/* UNPARSED */ "v",

/* Expressions */

	/* MONF */ "-eM",
	/* DYAF */ "e-eD",
	/* NUMBER */ "n",
	/* TEXT_DIS */ "-s'",
	/* TEXT_LIT */ "-s",
	/* TEXT_CONV */ "e*s",
	/* ELT_DIS */ "v{",
	/* LIST_DIS */ ":e<eu>}",
	/* RANGE_BNDS */ "e.ueu",
	/* TAB_DIS */ ":ee<~eu~eu>]",

/* Tests */

	/* AND */ "t*t",
	/* OR */ "t*t",
	/* NOT */ "t",
	/* SOME_IN */ "e#eUt",
	/* EACH_IN */ "e#eUt",
	/* NO_IN */ "e#eUt",
	/* MONPRD */ "-em",
	/* DYAPRD */ "e-ed",
	/* LESS_THAN */ "eeu",
	/* AT_MOST */ "eeu",
	/* GREATER_THAN */ "eeu",
	/* AT_LEAST */ "eeu",
	/* EQUAL */ "eeu",
	/* UNEQUAL */ "eeu",
	/* Nonode */ "",

	/* TAGformal */ "T",
	/* TAGlocal */ "T",
	/* TAGglobal */ "T",
	/* TAGrefinement */ "T",
	/* TAGzerfun */ "Z",
	/* TAGzerprd */ "z",

	/* ACTUAL */ "-?aes",
	/* FORMAL */ "-?fes",

	/* COLON_NODE */ "c"

};

/************************************************************************/

Hidden char *zerf[]= {
	F_pi, "n",
	F_e, "n",
	F_random, "n",
	F_now, "(6n,0n,1n,2n,3n,4n,5)",
	NULL
};

Hidden char *monf[]= {
	S_ABOUT, "nUn",
	S_PLUS, "nUn",
	S_MINUS, "nUn",
	S_NUMERATOR, "nUn",
	S_DENOMINATOR, "nUn",
	F_root, "nUn",
	F_abs, "nUn",
	F_sign, "nUn",
	F_floor, "nUn",
	F_ceiling, "nUn",
	F_round, "nUn",
	F_exactly, "nUn",
	F_sin, "nUn",
	F_cos, "nUn",
	F_tan, "nUn",
	F_arctan, "nUn",
	F_exp, "nUn",
	F_log, "nUn", 
	F_lower, "'U'",
	F_upper, "'U'",
	F_stripped, "'U'",
	F_split, "'Un']",
	F_keys, "wv]%U}",
	S_NUMBER, "v#Un",
	F_min, "w#%U",
	F_max, "w#%U",
	F_choice, "w#%U",
	F_radius, "(2n,0n,1)Un",
	F_angle, "(2n,0n,1)Un",
	NULL
};

Hidden char *dyaf[]= {
	S_PLUS, "nUnUn",
	S_MINUS, "nUnUn",
	S_TIMES, "nUnUn",
	S_OVER, "nUnUn",
	S_POWER, "nUnUn", 
	F_root, "nUnUn", 
	F_round, "nUnUn",
	F_mod, "nUnUn",
	F_sin, "nUnUn",
	F_cos, "nUnUn",
	F_tan, "nUnUn",
	F_arctan, "nUnUn",
	F_log, "nUnUn",
	S_JOIN, "'U'U'",
	S_BEHEAD, "nU'U'",
	S_CURTAIL, "nU'U'",
	S_REPEAT, "nU'U'",
	S_LEFT_ADJUST, "nU*'",
	S_CENTER, "nU*'",
	S_RIGHT_ADJUST, "nU*'",
	S_NUMBER, "~#Un",
	F_min, "~#ux",
	F_max, "~#ux",
	F_item, "nUw%#U",
	F_angle, "(2n,0n,1)UnUn",
#ifdef B_COMPAT
	F_thof, "~nUw%#U",
#endif
	NULL
};

Hidden char *zerp[]= {
	NULL
};

Hidden char *monp[]= {
	P_exact, "nu",
	NULL
};

Hidden char *dyap[]= {
	P_in, "~#u",
	P_notin, "~#u",
	NULL
};

/*********************************************************************

Meaning of codes:

H,F,P	calculate and store typecode for
	(H)command, F(unction), or P(redicate) definition
f	count a formal parameter for a command definition
p	set number of formal parameters for a function or predicate definition
	(also register that a next M,D,m or d concern the parameters
	 and not a use of the function or predicate
	 [the parstree's for FPR_FORMALS and e.g. MONF's are identical:-])

C	typecheck user defined command, actuals are on the stack
A,a	initialize/augment number of actual parameters for a used
	user defined command
q,Q	check for one/excessive actual parameter(s)
	(these are only used in typecodes for command definitions)
Z,M,D,z,m,d
	if (this if the FPR_FORMALS subtree 
		of a function or predicate definition)
	then
		interchange formals on the stack for d,D
		return
	else
		replace codestring t by the proper one for this
		(user defined or predefined) function or predicate;
		(the actual parameters are already on the stack)

V[0-9]+	push a new external type, with ident="NN.nn"
	where NN is the current ext_level and nn is the value of [0-9]+
	(this code only occurs in typecode's of how-to definitions)

c,s,e,t typecheck c(ommand), s(ubnode), e(xpression) or t(est)
        in subnode Fld(v, f++)
        As side effects, c sets curline for error messages,
        and e and t push a polytype on the stack.
-       skip subnode f++
L       curlino= subnode f++

u       pop(x); pop(y); push(unify(x, y)); p_release(x); p_release(y);
U       pop(x); pop(y); p_release(unify(x, y))); p_release(x); p_release(y);

Y       set returned value name for Yield
R       set returned value name for Refinement
y       release returned value name for yield/refinement
r       push(type of returned value);

*       pop(x); p_release(x)
?       skip code "e*" or "t*" if subnode f is NilTree
~       interchange: pop(x); pop(y); push(x); push(y);
%	pop(u); interchange like ~; push(u)
'       push(mk_text());
n       push(mk_number());
.       push(mk_text_or_number());
{       push(mk_elt());
}       pop(x); push(mk_list(x));
#       pop(x); push(mk_tlt(x));
]       pop(a); pop(k); push(mk_table(k, a));
T       push(tag(subnode f++));
w       x= mk_newvar(); push(x); push(copy(x));
v       push(mk_newvar());


Simple loop facility:
:       init loop over subnode f; f=FF and nf=Nfields(subnode)
<       indicator for start of loop body; if f>=nf goto ">"
>       indicator for end of loop body; if f<nf, go back to "<"

Coumpound types: (N is a number of digits, with decimal value N)
(N      push(mkt_compound(N))
,>      pop subtype, pop compound, putsubtype f in compound, push compound
,N      pop subtype, pop compound, putsubtype N in compound, push compound
)	no action, used for legibility,
        e.g. (2(2n,0n,1),1n,2) for compound in compound.
COLLATERALS don't use N, but combine with the loop facility, as indicated.

*************************************************************************/

Hidden value ret_name= Vnil;
/*
 * if in commandsuite of expression- or test-refinement: 
 *	holds refinement name;
 * if in commandsuite of yield unit:
 * 	holds ABC-text RETURNED_VALUE 
 *		(used in error messages, 
 *		 no confusion with refinement names should be possible)
 * else
 *	Vnil
 * Used in tc_node(RETURN expr)
 */

/************************************************************************/

/* For the inter-unit typecheck we need codes 
 * for "externally used variable types".
 * These codes look like "V1", "V2", etc., for the first, second etc used
 * external variable type.
 * When used in user defined commands, functions or precidate calls,
 * we turn these into types (kind="Variable", id="N.1" or "N.2" etc)
 * where N stands for the number of the currently used user defined;
 * N is augmented for every use of some user defined command, function
 * or predicate, and is kept in ext_level.
 */
Hidden int ext_level= 0;

/* nformals counts the number of formal parameters of a how-to.
 * For functions and predicate definitions it also acts
 * as a boolean to know when a MONF (etc) is an FPR_FORMAL,
 * or part of an expression.
 */
#define FPR_PARAMETERS (-1)
Hidden int nformals= 0;
Hidden int nactuals= 0;

/************************************************************************/

/************************************************************************/

Forward Hidden polytype pt_pop();
Forward Hidden polytype external_type();

Forward Hidden string get_code();
Forward Hidden string fpr_code();

Visible Procedure type_check(v) parsetree v; {
	typenode n;

	if (!still_ok || v == NilTree)
		return;
	n= nodetype(v);
	curline= v; curlino= one;
	pts_init();
	usetypetable(mk_elt());
	start_vars();
	ret_name= Vnil;
	ext_level= 0;
	nformals= 0;
	if (Unit(n) || Command(n) || Expression(n)) {
		tc_node(v);
		if (!interrupted && Expression(n))
			p_release(pt_pop());
	}
	else syserr(WRONG_ARGUMENT);
	end_vars();
	deltypetable();
	pts_free();
}

#define FF First_fieldnr
#define Fld(v, f) (*(Branch(v, f)))

Hidden Procedure tc_node(v) parsetree v; {
	string t;
	string t_saved= NULL;
	int f;
	int nf;
	int len;	/* length of compound */
	polytype x, y, u;
	
	if (v == NilTree)
		return;
	
	t= tc_code[nodetype(v)];
	f= FF;
	
#ifdef TYPETRACE
	t_typecheck((int)nodetype(v), t);
#endif
	
    while (*t) {
	
	switch (*t) {
	
	case 'p':	/* formal parameter(s) of func or pred */
		switch (nodetype(Fld(v, f))) {
		case TAG:
			nformals= 0;
			break;
		case MONF: case MONPRD:
			nformals= FPR_PARAMETERS;
			tc_node(Fld(v, f));
			nformals= 1;
			break;
		case DYAF: case DYAPRD:
			nformals= FPR_PARAMETERS;
			tc_node(Fld(v, f));
			nformals= 2;
			break;
		}
		f++;
		break;
	case 'f':	/* formal parameter of command definition */
		nformals++;
		break;
	case 'H':
	case 'F':
	case 'P':
		put_code(v, *t);
		break;
	
	case 'A':
		nactuals= 0;
		break;
	case 'a':
		nactuals++;
		break;
	case 'C':
		/* user defined Command, actuals are on the stack */
		ext_level++;
		t= get_code(Fld(v, UNIT_NAME), Cmd, cur_env);
		if (t != NULL)
			t_saved= t;
		else
			t= "Q";
		continue;	/* skips t++ */
	case 'q':
		if (nactuals <= 0)
			return;	/* breaks loop over formals in excess */
		/* else: */
		nactuals--;
		break;
	case 'Q':
		while (nactuals > 0) {
			p_release(pt_pop());
			nactuals--;
		}
		break;
	
	case 'Z':
		ext_level++;
		t_saved= t= fpr_code(Fld(v, TAG_NAME), Zfd, zerf, "T");
		continue;	/* skips t++ */
	case 'M':
		if (nformals == FPR_PARAMETERS)
			return;
		ext_level++;
		t_saved= t= fpr_code(Fld(v, MON_NAME), Mfd, monf, "*v");
		continue;	/* skips t++ */
	case 'D':
		if (nformals == FPR_PARAMETERS) {
			return;
		}
		ext_level++;
		t_saved= t= fpr_code(Fld(v, DYA_NAME), Dfd, dyaf, "**v");
		continue;	/* skips t++ */
	case 'z':
		ext_level++;
		t_saved= t= fpr_code(Fld(v, TAG_NAME), Zpd, zerp, "T");
		continue;	/* skips t++ */
	case 'm':
		if (nformals == FPR_PARAMETERS)
			return;
		ext_level++;
		t_saved= t= fpr_code(Fld(v, MON_NAME), Mpd, monp, "");
		continue;	/* skips t++ */
	case 'd':
		if (nformals == FPR_PARAMETERS) {
			return;
		}
		ext_level++;
		t_saved= t= fpr_code(Fld(v, DYA_NAME), Dpd, dyap, "*");
		continue;	/* skips t++ */
	
	case 'V':
		x= external_type(&t);
		pt_push(x);
		continue;	/* skipping t++ ! */
	
	case 'c':
		curline= Fld(v, f);
		end_vars();
		start_vars();
		/* FALLTHROUGH */
	case 's': /* just subnode, without curline setting */
	case 'e': /* 'e' and 't' leave polytype on stack */
	case 't':
		tc_node(Fld(v, f));
		f++;
		break;
	case '-':
		f++;
		break;
	case 'Y':
		ret_name= mk_text(RETURNED_VALUE);
		break;
	case 'y':
		if (ret_name != Vnil)
			release(ret_name);
		ret_name= Vnil;
		break;
	case 'R':
		set_ret_name((value) Fld(v, REF_NAME));
		break;
	case 'r':
		if (ret_name != Vnil) {
			pt_push(mkt_var(copy(ret_name)));
		}
		else {
			interr(WRONG_RETURN);
			/* skip final U in tc_code for RETURN: */
			p_release(pt_pop());
			return;
		}
		break;
	case 'L':
		curlino= Fld(v, f);
		f++;
		break;
	case '?':
		if (Fld(v, f) == NilTree) {
			/* skip tc_code "t*" or "e*" */
			t+=2;
			f++;
			/* to prevent p_release(not pushed e or t) */
		}
		break;
	case 'U':
	case 'u':
		y= pt_pop();
		x= pt_pop();
		unify(x, y, &u);
		p_release(x);
		p_release(y);
		if (*t == 'U')
			p_release(u);
		else
			pt_push(u);
		break;
	case '*':
		p_release(pt_pop());
		break;
	case '\'':
		pt_push(mkt_text());
		break;
	case 'n':
		pt_push(mkt_number());
		break;
	case '.':
		pt_push(mkt_tn());
		break;
	case '{':
		pt_push(mkt_lt(pt_pop()));
		break;
	case '}':
		pt_push(mkt_list(pt_pop()));
		break;
	case '#':
		pt_push(mkt_tlt(pt_pop()));
		break;
	case ']':
		y= pt_pop();
		x= pt_pop();
		pt_push(mkt_table(x, y));
		break;
	case 'x':
		x= pt_pop();
		if (t_is_error(kind(x)))
			pt_push(mkt_error());
		else
			pt_push(p_copy(asctype(bottomtype(x))));
		p_release(x);
		break;
	case 'v':
		pt_push(mkt_newvar());
		break;
	case 'w':
		x= mkt_newvar();
		pt_push(x);
		pt_push(p_copy(x));
		break;
	case '~':
		x= pt_pop();
		y= pt_pop();
		pt_push(x);
		pt_push(y);
		break;
	case '%':
		u= pt_pop();
		x= pt_pop();
		y= pt_pop();
		pt_push(x);
		pt_push(y);
		pt_push(u);
		break;
	case 'T':
		x= mkt_var(copy(Fld(v, f)));
		add_var(x);
		pt_push(x);
		/* f++ unnecessary */
		break;
	case ':':	/* initialize loop over subnode */
		/* f == FF */
		v= Fld(v, f);
		nf= Nfields(v);
		break;
	case '<':	/* start of loop body (after init part) */
		if (f >= nf) /* init part ate the one-and-only subfield */
			while (*t != '>') ++t;
		break;
	case '>':	/* end of loop body */
		if (f < nf)
			while (*t != '<') --t;
		break;
	case '(':
		++t;
		if (*t == '<') {
			/* COLLATERAL above */
			len= nf;
		}
		else {
			/* code for compound in fpr_code */
			len= 0;
			while ('0' <= *t && *t <= '9') {
				len= 10*len + *t - '0';
				++t;
			}
		}
		pt_push(mkt_compound(len));
		continue;
	case ',':
		++t;
		if (*t == '>') {
			len= f-1;
		}
		else {
			len= 0;
			while ('0' <= *t && *t <= '9') {
				len= 10*len + *t - '0';
				++t;
			}
		}
		x= pt_pop();
		u= pt_pop();
		putsubtype(x, u, len);
		pt_push(u);
		continue;
	case ')':
		/* just there to end number in compound in compound */
		break;

	} /* end switch (*t) */
	
	t++;
	
    } /* end while (*t) */

	if (t_saved != NULL)
    		freestr(t_saved);
}

/************************************************************************/

/* table mapping pname's to type_code's for how-to definitions */

/* def_typeode(): add/replace the typecode mapping of a how-to */

Hidden Procedure def_typecode(pname, tc, wse)
     value pname;
     value tc;
     wsenvptr wse;
{
	e_replace(tc, &(wse->abctypes), pname);
	wse->typeschanges = Yes;
}

/* del_typecode(): remove the typecode mapping of a how-to */

Hidden Procedure del_typecode(pname, wse)
     value pname;
     wsenvptr wse;
{
	e_delete(&(wse->abctypes), pname);
	wse->typeschanges = Yes;
}

/* tc_exists(): search the typecode mapping of a how-to */

Hidden bool tc_exists(pname, cc, wse)
     value pname;
     value **cc;
     wsenvptr wse;
{
	return in_env(wse->abctypes, pname, cc);
}

/* get and put table mapping pname's to typecode's of how-to's
 * to file when entering or leaving workspace.
 */
Visible Procedure initstc() {
	value fn;
	
	if (Valid(cur_env->abctypes)) {
		release(cur_env->abctypes);
		cur_env->abctypes= Vnil;
	}
	if (F_exists(typesfile)) {
		fn= mk_text(typesfile);
		cur_env->abctypes= getval(fn, In_prmnv);
		if (!still_ok) {
			if (Valid(cur_env->abctypes))
				release(cur_env->abctypes);
			cur_env->abctypes= mk_elt();
			still_ok= Yes;
		}
		release(fn);
	}
	else cur_env->abctypes= mk_elt();
	cur_env->typeschanges= No;
}

Visible Procedure put_types()
{
	char *dir;
	int len;
	value v = cur_env->abctypes;
	
	if (!cur_env->typeschanges || !Valid(v))
		return;
	dir = InUsingEnv() ? cur_dir : cen_dir;
	len = length(v);
	if (len != 0) {
		putval(v, dir, typesfile, In_prmnv, Yes);
	}
	else { /* Remove the file if the typecode table is empty */
		char *file = makepath(dir, typesfile);
		f_delete(file);
		free_path(file);
	}
	cur_env->typeschanges= No;
}

Visible Procedure endstc()
{
	put_types();
	if (terminated) return;
	release(cur_env->abctypes); cur_env->abctypes= Vnil;
}

Visible Procedure rectypes()
{
	if (Valid(cur_env->abctypes))
		release(cur_env->abctypes);
	cur_env->abctypes= mk_elt();
	if (F_readable(typesfile)) {
		f_delete(typesfile);
	}
}

/************************************************************************/

Visible value stc_code(pname) value pname; {
	value *tc;
	
	if (tc_exists(pname, &tc, cur_env))
		return copy(*tc);
	/* else: */
	return Vnil;
}	

Hidden value old_abctypes;
Hidden bool old_typeschanges;

Visible Procedure del_types() {
	old_abctypes= copy(cur_env->abctypes);
	old_typeschanges= cur_env->typeschanges;
	release(cur_env->abctypes);
	cur_env->abctypes= mk_elt();
	cur_env->typeschanges= Yes;
}

Visible Procedure adjust_types(no_change) bool no_change; {
	if (no_change) {
		/* recover old inter-unit typetable */
		release(cur_env->abctypes);
		cur_env->abctypes= old_abctypes;
		cur_env->typeschanges= old_typeschanges;
	}
	else {
		release(old_abctypes);
	}
}

/************************************************************************/

/* Calculate code for how-to definition and put into typetable */
/* formals are on the stack */

Forward Hidden value type_code();

Hidden Procedure put_code(v, type) parsetree v; char type; {
	value howcode, fmlcode;
	value pname, *tc;
	polytype x;
	int f;
	
	pname= get_pname(v);
	if (tc_exists(pname, &tc, cur_env))
		del_typecode(pname, cur_env);
		/* do not use old code for possibly edited how-to */
	
	new_externals();
	
	howcode= mk_text("");
	for (f= nformals; f > 0; f--) {
		if (type == 'H') {
			howcode= conc(howcode, mk_text("q"));
		}
		fmlcode= type_code(x=pt_pop()); p_release(x);
		howcode= conc(howcode, fmlcode);
		howcode= conc(howcode, mk_text("U"));
	}
	if (type == 'H') {
		howcode= conc(howcode, mk_text("Q"));
	}
	else if (type == 'P')
		howcode= conc(howcode, mk_text("v"));
	else {
		x= mkt_var(mk_text(RETURNED_VALUE));
		howcode= conc(howcode, type_code(x));
		p_release(x);
	}
	
	def_typecode(pname, howcode, cur_env);
	release(pname); release(howcode);
}

Hidden value type_code(p) polytype p; {
	typekind p_kind;
	polytype tp;
	polytype ext;
	value tc;
	intlet k, len;
	char buf[20];
	
	p_kind = kind(p);
	if (t_is_number(p_kind)) {
		return mk_text("n");
	}
	else if (t_is_text(p_kind)) {
		return mk_text("'");
	}
	else if (t_is_tn(p_kind)) {
		return mk_text(".");
	}
	else if (t_is_compound(p_kind)) {
		len= nsubtypes(p);
		tc= mk_text("(");
		sprintf(buf, "%d", len);
		tc= conc(tc, mk_text(buf));
		for (k = 0; k < len; k++) {
			tc= conc(tc, type_code(subtype(p, k)));
			sprintf(buf, ",%d", k);
			tc= conc(tc, mk_text(buf));
		}
		return conc(tc, mk_text(")"));
	}
	else if (t_is_error(p_kind)) {
		return mk_text("v");
	}
	else if (t_is_table(p_kind)) {
		tc = type_code(keytype(p));
		tc = conc(tc, type_code(asctype(p)));
		return conc(tc, mk_text("]"));
	}
	else if (t_is_list(p_kind)) {
		tc = type_code(asctype(p));
		return conc(tc, mk_text("}"));
	}
	else if (t_is_lt(p_kind)) {
		tc = type_code(asctype(p));
		return conc(tc, mk_text("{"));
	}
	else if (t_is_tlt(p_kind)) {
		tc = type_code(asctype(p));
		return conc(tc, mk_text("#"));
	}
	else if (t_is_var(p_kind)) {
		tp = bottomtype(p);
		if (!t_is_var(kind(tp)))
			return type_code(tp);
		else {
			ext= mkt_ext();
			repl_type_of(tp, ext);
			return type_code(ext);
		}
	}
	else if (t_is_ext(p_kind)) {
		return conc(mk_text("V"), convert(ident(p), No, Yes));
	}
	else {
		return mk_text("v"); /* cannot happen */
	}
	/* NOTREACHED */
}

/************************************************************************/

/* retrieve the codes for user defined commands and for
 * user defined and predefined functions and predicates
 * from the respective tables
 */

Hidden string get_code(name, type, wse)
     value name;
     int type;
     wsenvptr wse;
{
	value pname;
	value *aa;

	pname= permkey(name, type);
	if (tc_exists(pname, &aa, wse))
		return savestr(strval(*aa));
	else if (IsUsingEnv(wse) && tc_exists(pname, &aa, cen_env))
		return savestr(strval(*aa));
	/* else: */
	return NULL;
}

Hidden string pre_fpr_code(fn, func) value fn; char *func[]; {
	int i;
	string f= strval(fn);
	
	for (i= 0;  ; i+=2) {
		if (func[i] == NULL)
			return NULL;
		if (strcmp(f, func[i]) == 0)
			return (string) savestr(func[i+1]);
	}
	/*NOTREACHED*/
}

Hidden string fpr_code(name, type, functab, defcode)
     value name;
     literal type;
     char *functab[];
     string defcode;
{
	string t;
	wsenvptr wse;

	if (is_unit(name, type, PPnil, &wse) && !IsStandardEnv(wse))
		t = get_code(name, type, wse);
	else
		t= pre_fpr_code(name, functab);
	
	if (t == NULL)
		t= savestr(defcode);
	
	return t;
}

/************************************************************************/

Hidden polytype external_type(pt) string *pt; {
	int n;
	string t;
	polytype x;
	char buf[20];
	
	n= 0;
	t= *pt;
	for (++t; '0' <= *t && *t <= '9'; t++) {
		n= n*10 + *t-'0';
	}
	sprintf(buf, "%d.%d", ext_level, n);
	x= mkt_var(mk_text(buf));
	*pt= t;
	return x;
}

/************************************************************************/

Hidden Procedure set_ret_name(name) value name; {
	value n1;
	
	n1= curtail(name, one);
		/* should check for expression refinement */
	if (!Cap(charval(n1)))
		ret_name= copy(name);
	release(n1);
}

/************************************************************************/

/* PolyTypes Stack */

#define STACKINCR 100

Hidden polytype *pts_start;
Hidden polytype *pts_top;
Hidden polytype *pts_end;

Hidden Procedure pts_init() {
	pts_start= (polytype *) getmem((unsigned) (STACKINCR * sizeof(polytype)));
	pts_top= pts_start;
	pts_end= pts_start + STACKINCR;
	*(pts_top)= (polytype) Vnil;
}

Hidden Procedure pts_free() {
	if (interrupted) {
		for (--pts_top; pts_top >= pts_start; --pts_top) {
			p_release(*pts_top);
		}
	}
	freemem((ptr) pts_start);
}

Hidden Procedure pts_grow() {
	int oldtop= pts_top - pts_start;
	int syze= (pts_end - pts_start) + STACKINCR;
	
	regetmem((ptr *) &(pts_start), (unsigned) (syze * sizeof(polytype)));
	pts_top= pts_start + oldtop;
	pts_end= pts_start + syze;
}

Hidden Procedure pt_push(pt) polytype pt; {
	if (pts_top >= pts_end)
		pts_grow();
	*pts_top++= pt;
}

Hidden polytype pt_pop() {
#ifndef NDEBUG
	if (pts_top <= pts_start)
		syserr(EMPTY_STACK);
#endif
	return *--pts_top;
}
