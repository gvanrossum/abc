/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1986. */

/* Prepare for code generation -- find out which tags are targets */

#include "b.h"
#include "bint.h"
#include "bobj.h"
#include "i0err.h"
#include "i2nod.h"
#include "i2gen.h" /* Must be after i2nod.h */
#include "i3env.h"
#include "i3sou.h"


Visible int nextvarnumber; /* Counts local targets (including formals) */
Hidden int nformals; /* nr of formals */
Hidden bool bound; /* flag to recognise bound tags */

Visible value locals, globals, mysteries, refinements;

Forward Hidden Procedure unit_context();

Visible value *setup(t) parsetree t; {
	typenode n= Nodetype(t);
	bool in_prmnv= !Unit(n);
	nextvarnumber= 0;
	mysteries= mk_elt();
	if (in_prmnv) {
		globals= copy(prmnv->tab);
		locals= Vnil;
		refinements= mk_elt();
		return Command(n) ? &globals : Pnil;
	} else {
		globals= mk_elt();
		locals= mk_elt();
		refinements= *Branch(t, n == HOW_TO ? HOW_R_NAMES : FPR_R_NAMES);
		VOID copy(refinements);
		unit_context(t);
		return &locals;
	}
}

Hidden Procedure unit_context(t) parsetree t; {
	cntxt= In_unit;
	sethowtoname(get_pname(t));
}

Visible Procedure cleanup() {
	release(locals);
	release(globals);
	release(mysteries);
	release(refinements);
}

/* ********************************************************************	*/

/* Analyze parse tree, finding the targets and formal parameters.
   Formal parameters are found in the heading and stored as local targets.
   Global targets are also easily found: they are mentioned in a SHARE command.
   Local targets appear on their own or in collateral forms after PUT IN
   or as bound tags after FOR, SOME, EACH or NO.
   Note that DELETE x, REMOVE e FROM x, or PUT e IN x[k] (etc.) don't
   introduce local targets, because in all these cases x must have been
   initialized first.  This speeds up our task of finding targets,
   since we don't have to visit all nodes: only nodes that may contain
   commands or tests, and the positions mentioned here, need be visited.
   (And of course unit headings).
   We don't have to look for refinements since these are already known
   from the unit heading.
 */

Hidden Procedure a_tag(name, targs) value name; value *targs; {
	value *aa; int varnumber;
	if (locals != Vnil && envassoc(locals, name) != Pnil);
	else if (envassoc(globals, name) != Pnil);
	else if (envassoc(refinements, name) != Pnil) {
		if (targs != &mysteries)
			fixerr(REF_NO_TARGET);
	}
	else {
		aa= envassoc(mysteries, name);
		if (aa != Pnil && targs == &mysteries);
		else {
			if (aa != Pnil) {
				varnumber= SmallIntVal(*aa);
				e_delete(&mysteries, name);
			}
			else if (targs != &globals)
				varnumber= nextvarnumber++;
			else varnumber= 0;
			e_replace(MkSmallInt(varnumber), targs, name);
		}
	}
	if (bound && locals != Vnil) {
		aa= envassoc(locals, name);
		if (aa == Pnil || SmallIntVal(*aa) < nformals)
			fixerr(MESS(4400, "in ... i IN e, i contains a non-local name"));
	}
}

Hidden Procedure a_fpr_formals(t) parsetree t; {
	typenode n= nodetype(t);
	switch (n) {
	case TAG:
		break;
	case MONF: case MONPRD:
		analyze(*Branch(t, MON_RIGHT), &locals);
		break;
	case DYAF: case DYAPRD:
		analyze(*Branch(t, DYA_LEFT), &locals);
		analyze(*Branch(t, DYA_RIGHT), &locals);
		break;
	default: syserr(MESS(1900, "a_fpr_formals"));
	}
}

Visible Procedure analyze(t, targs) parsetree t; value *targs; {
	typenode nt; string s; char c; int n, k, len; value v;
	if (!Is_node(t) || !still_ok) return;
	nt= Nodetype(t);
	if (nt < 0 || nt >= NTYPES) syserr(MESS(1901, "analyze bad tree"));
	s= gentab[nt];
	if (s == NULL) return;
	n= First_fieldnr;
	while ((c= *s++) != '\0' && still_ok) {
		switch (c) {
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			n= (c - '0') + First_fieldnr;
			break;
		case 'c':
			v= *Branch(t, n);
			if (v != Vnil) {
				len= Nfields(v);
				for (k= 0; k < len; ++k)
					analyze(*Field(v, k), targs);
			}
			++n;
			break;
		case '#':
			curlino= *Branch(t, n);
			/* Fall through */
		case 'l':
		case 'v':
			++n;
			break;
		case 'm':
			analyze(*Branch(t, n), &mysteries);
			++n;
			break;
		case 'g':
			analyze(*Branch(t, n), &globals);
			++n;
			break;
		case 'b':
			bound= Yes;
			analyze(*Branch(t, n),
				locals != Vnil ? &locals : &globals);
			bound= No;
			++n;
			break;
		case 'x':
			curline= *Branch(t, n);
			/* Fall through */
		case 'a':
		case 'u':	
			analyze(*Branch(t, n), targs);
			++n;
			break;
		case 't':
			analyze(*Branch(t, n), Pnil);
			++n;
			break;
		case 'f':
			a_fpr_formals(*Branch(t, n));
			nformals= nextvarnumber;
			++n;
			break;
		case 'h':
			v= *Branch(t, n);
			analyze(v, &locals);
			nformals= nextvarnumber;
			++n;
			break;
		case '=':
			*Branch(t, n)= MkSmallInt(nextvarnumber);
			++n;
			break;
		case ':':	/* code for WHILE loop */
			curlino= *Branch(t, WHL_LINO);
			analyze(*Branch(t, WHL_TEST), Pnil);
			v= *Branch(t, WHL_SUITE);
			if (nodetype((parsetree) v) != COLON_NODE)
				syserr(BAD_WHILE);
			analyze(*Branch(v, COLON_SUITE), targs);
			break;
		case ';':	/* code for TEST_SUITE */
			curlino= *Branch(t, TSUI_LINO);
			curline= *Branch(t, TSUI_TEST);
			analyze(curline, Pnil);
			v= *Branch(t, TSUI_SUITE);
			if (nodetype((parsetree) v) != COLON_NODE)
				syserr(BAD_TESTSUITE);
			analyze(*Branch(v, COLON_SUITE), targs);
			analyze(*Branch(t, TSUI_NEXT), targs);
			break;
		case 'T':
			if (targs != Pnil)
				a_tag((value)*Branch(t, TAG_NAME), targs);
			break;
		}
	}
}

/* ********************************************************************	*/

/* Table describing the actions of the fixer for each node type */


/*
	LIST OF CODES AND THEIR MEANING

	char	fix		n?	analyze

	0-9			n= c-'0'

	#	set curlino	++n	set curlino
	=			++n	set to nextvarnum
	a	locate		++n	analyze
	b	locate		++n	analyze bound tags
	c	collateral	++n	analyze collateral
	f	fpr_formals	++n	a_fpr_formals
	g			++n	global
	h			++n	how'to formal
	l	locate		++n
	m	actual param	++n	mystery
	t	test		++n	analyze; set targs= 0
	u	unit		++n	analyze
	v	evaluate	++n
	x	execute		++n	analyze

	:	special code for WHILE loop
	;	special code for TEST_SUITE
	?	special code for UNPARSED
	@	special check for BEHEAD target
	|	special check for CURTAIL target
	C	special code for comparison
	D	special code for DYAF
	E	special code for DYAPRD
	F	make number
	G	jumpto(l1)
	H	here(&l1)
	I	if (*Branch(t, n) != NilTree) jump2here(t)
	J	jump2here(t)
	K	hold(&st)
	L	let_go(&st)
	M	special code for MONF
	N	special code for MONPRD
	Q	if (*Branch(t, n) != NilTree) visit(t);
	R	if (!reachable()) "command cannot be reached"
	S	jumpto(Stop)
	T	special code for TAG
	U	special code for user-defined-command
	V	visit(t)
	W	visit2(t, seterr(1))
	X	visit(t) or lvisit(t) depending on flag
	Y	special code for YIELD/TEST
	Z	special code for refinement
	 
*/


Visible string gentab[NTYPES]= {

	/* HOW_TO */ "1h3xSu6=",
	/* YIELD */ "2fV4xYu7=",
	/* TEST */ "2fV4xYu7=",
	/* REFINEMENT */ "H2xZSu",

	/* Commands */

	/* SUITE */ "#RQx3x",
	/* PUT */ "vaV",
	/* INSERT */ "vlV",
	/* REMOVE */ "vlV",
	/* SET_RANDOM */ "vV",
	/* DELETE */ "lV",
	/* CHECK */ "tV",
	/* SHARE */ "g",
	/* PASS */ "",

	/* WRITE */ "1vV",
	/* WRITE1 */ "1vV",
	/* READ */ "avV",
	/* READ_RAW */ "aV",

	/* IF */ "tV2xJ",
	/* WHILE */ ":",	/* old: "HtV2xGJ" */
	/* FOR */ "bvHV3xGJ",

	/* SELECT */ "1x",
	/* TEST_SUITE */ ";",	/* old: "#tW3xKIxL" */
	/* ELSE */ "#2x",

	/* QUIT */ "VS",
	/* RETURN */ "vVS",
	/* REPORT */ "tVS",
	/* SUCCEED */ "VS",
	/* FAIL */ "VS",

	/* USER_COMMAND */ "1mUV",

/* the next three are only used when GFX has been defined */
	/* SPACE */ "vvV",
	/* LINE */ "vvV",
	/* CLEAR */ "V",

	/* EXTENDED_COMMAND */ "1cV",

	/* Expressions, targets, tests */

	/* TAG */ "T",
	/* COMPOUND */ "a",

	/* Expressions, targets */

	/* COLLATERAL */ "cX",
	/* SELECTION */ "lvX",
	/* BEHEAD */ "lv@X",
	/* CURTAIL */ "lv|X",

	/* Expressions, tests */

	/* UNPARSED */ "?",

	/* Expressions */

	/* MONF */ "M1vV",
	/* DYAF */ "Dv2vV",
	/* NUMBER */ "FV",
	/* TEXT_DIS */ "1v",
	/* TEXT_LIT */ "1vV",
	/* TEXT_CONV */ "vvV",
	/* ELT_DIS */ "V",
	/* LIST_DIS */ "cV",
	/* RANGE_ELEM */ "vvV",
	/* TAB_DIS */ "cV",

	/* Tests */

	/* AND */ "tVtJ",
	/* OR */ "tVtJ",
	/* NOT */ "tV",
	/* SOME_IN */ "bvHVtGJ",
	/* EACH_IN */ "bvHVtGJ",
	/* NO_IN */ "bvHVtGJ",
	/* MONPRD */ "N1vV",
	/* DYAPRD */ "Ev2vV",
	/* LESS_THAN */ "vvCV",
	/* AT_MOST */ "vvCV",
	/* GREATER_THAN */ "vvCV",
	/* AT_LEAST */ "vvCV",
	/* EQUAL */ "vvCV",
	/* UNEQUAL */ "vvCV",
	/* Nonode */ "",

	/* TAGformal */ "T",
	/* TAGlocal */ "T",
	/* TAGglobal */ "T",
	/* TAGrefinement */ "T",
	/* TAGzerfun */ "T",
	/* TAGzerprd */ "T",

	/* ACTUAL */ "1mm",
	/* FORMAL */ "1hh",

	/* COLON_NODE */ ""
};
