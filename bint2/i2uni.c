/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1986. */

#include "b.h"
#include "bint.h"
#include "bobj.h"
#include "i0err.h"
#include "b0lan.h"
#include "i2par.h"
#include "i2nod.h"
#include "i3env.h"
#include "i3sou.h"

Forward Hidden parsetree cmd_unit();
Forward Hidden value cmd_formals();
Forward Hidden parsetree funprd_unit();
Forward Hidden parsetree fp_formals();
Forward Hidden parsetree fml_operand();
Forward Hidden bool share();

/* ******************************************************************** */
/*		unit							*/
/* ******************************************************************** */

Hidden value formlist, sharelist;
Hidden envtab reftab; 
Visible literal idf_cntxt;

Forward Hidden parsetree ref_suite();

#define unicmd_suite(level) cmd_suite(level, Yes, ucmd_seq)

Visible parsetree unit(heading, editing) bool heading, editing; {
	parsetree v= NilTree;
	char *kw;
	
	if (!heading) {
		lino= 1;
		cntxt= In_unit;
		sethowtoname(Vnil);
	}
	if (is_keyword(&kw) && how_keyword(kw)) {
		need(K_TO_how);
		if (cur_ilev != 0)
			parerr(MESS(2800, "how-to starts with indentation"));
		skipsp(&tx);
		if (is_cmdname(ceol, &kw)) {
			if (return_keyword(kw))
				v= funprd_unit(heading, Yes);
			else if (report_keyword(kw))
				v= funprd_unit(heading, No);
			else v= cmd_unit(kw, heading);
		}
		else parerr(MESS(2801, "no how-to name where expected"));
	}
	else parerr(MESS(2802, "no how-to keyword where expected"));

#ifdef TYPE_CHECK
	if (!heading && !editing) type_check(v);
#endif
	return v;
}

/* ******************************************************************** */
/*		cmd_unit						*/
/* ******************************************************************** */

Hidden parsetree cmd_unit(kw, heading) char *kw; bool heading; {
	parsetree v;
	value w= mk_text(kw);
	value c, f;
	txptr ftx, ttx;
	intlet level= cur_ilev;
	
	formlist= mk_elt(); 
	sethowtoname(permkey(w, Cmd));
	if (in(w, res_cmdnames)) 
		pprerrV(MESS(2803, "%s is a reserved keyword"), w);
	req(S_COLON, ceol, &ftx, &ttx);
	idf_cntxt= In_formal;
	f= cmd_formals(ftx, w); tx= ttx;
	if (!is_comment(&c)) c= Vnil;
	v= node8(HOW_TO, copy(w), f, c, NilTree, NilTree, Vnil, Vnil);
	if (!heading) {
		sharelist= mk_elt();
		*Branch(v, HOW_SUITE)= unicmd_suite(level);
		reftab= mk_elt();
		*Branch(v, HOW_REFINEMENT)= ref_suite(level);
		*Branch(v, HOW_R_NAMES)= reftab;
		release(sharelist);
	}
	release(formlist); 
	return v;
}

Hidden value cmd_formals(q, kw) txptr q; value kw; {
	value t= Vnil, v= Vnil;
	txptr ftx;
	value nkw;

	skipsp(&tx);
	if (!findkw(q, &ftx))
		ftx= q;
	if (Text(ftx))
		t= idf(ftx);
	if (Text(q)) {
		nkw= mk_text(keyword());
		v= cmd_formals(q, nkw);
	}
	return node4(FORMAL, kw, t, v);
}

/* ******************************************************************** */
/*		fun_unit/prd_unit					*/
/* ******************************************************************** */

Hidden parsetree funprd_unit(heading, isfunc) bool heading, isfunc; {
	parsetree v, f; 
	value name, c, adicity;
	txptr ftx, ttx;
	intlet level= cur_ilev;
	
	formlist= mk_elt(); 
	skipsp(&tx);
	req(S_COLON, ceol, &ftx, &ttx);
	f= fp_formals(ftx, isfunc, &name, &adicity); tx= ttx;
	if (!is_comment(&c)) c= Vnil;
	v= node9(isfunc ? YIELD : TEST, copy(name), adicity, f, c, NilTree,
		  NilTree, Vnil, Vnil);
	if (!heading) {
		sharelist= mk_elt();
		*Branch(v, FPR_SUITE)= unicmd_suite(level);
		reftab= mk_elt();
		*Branch(v, FPR_REFINEMENT)= ref_suite(level);
		*Branch(v, FPR_R_NAMES)= reftab;
		release(sharelist);
	}
	release(formlist); 
	return v;
}

/* ******************************************************************** */

#define FML_IN_FML MESS(2804, "%s is already a formal parameter or operand")
#define SH_IN_FML  FML_IN_FML
#define SH_IN_SH   MESS(2805, "%s is already a shared name")
#define REF_IN_FML SH_IN_FML
#define REF_IN_SH  SH_IN_SH
#define REF_IN_REF MESS(2806, "%s is already a refinement name")

Hidden Procedure treat_idf(t) value t; {
	switch (idf_cntxt) {
		case In_formal:	if (in(t, formlist)) 
					pprerrV(FML_IN_FML, t);
				insert(t, &formlist);
				break;
		case In_share:	if (in(t, formlist)) 
					pprerrV(SH_IN_FML, t);
				if (in(t, sharelist)) 
					pprerrV(SH_IN_SH, t);
				insert(t, &sharelist);
				break;
		case In_ref:	if (in(t, formlist)) 
					pprerrV(REF_IN_FML, t);
				if (in(t, sharelist)) 
					pprerrV(REF_IN_SH, t);
				break;
		case In_ranger: break;
		default:	break;
	}
}

#define NO_FUN_NAME	MESS(2807, "cannot find function name")

Hidden parsetree fp_formals(q, isfunc, name, adic) txptr q; bool isfunc;
		value *name, *adic; {
	parsetree v1, v2, v3;

	*name= Vnil;
	idf_cntxt= In_formal;
	v1= fml_operand(q);
	skipsp(&tx);
	if (!Text(q)) { /* zeroadic */
		*adic= zero; 
		if (nodetype(v1) == TAG) {
			*name= *Branch(v1, TAG_NAME);
			sethowtoname(permkey(*name, isfunc ? Zfd : Zpd));
	 	}
	 	else pprerr(MESS(2808, "user defined functions must be names"));
		return v1;
	}

	v2= fml_operand(q);
	skipsp(&tx);
	if (!Text(q)) { /* monadic */
		*adic= one; 
		if (nodetype(v1) == TAG) {
			*name= copy(*Branch(v1, TAG_NAME));
			sethowtoname(permkey(*name, isfunc ? Mfd : Mpd));
		}
		else pprerr(NO_FUN_NAME);
		if (nodetype(v2) == TAG) treat_idf(*Branch(v2, TAG_NAME));
		release(v1);
		return node4(isfunc ? MONF : MONPRD, *name, v2, Vnil);
	}

	v3= fml_operand(q);
	/* dyadic */
	*adic= mk_integer(2);
	if (nodetype(v2) == TAG) {
		*name= copy(*Branch(v2, TAG_NAME));
		sethowtoname(permkey(*name, isfunc ? Dfd : Dpd));
	}
	else pprerr(NO_FUN_NAME);
	upto1(q, MESS(2809, "something unexpected in formula template"));
	if (nodetype(v1) == TAG) treat_idf(*Branch(v1, TAG_NAME));
	if (nodetype(v3) == TAG) treat_idf(*Branch(v3, TAG_NAME));
	release(v2);
	return node5(isfunc ? DYAF : DYAPRD, v1, *name, v3, Vnil);
}

Hidden parsetree fml_operand(q) txptr q; {
	value t;
	skipsp(&tx);
	if (nothing(q, MESS(2810, "nothing instead of expected template operand"))) 
		return NilTree;
	else if (is_tag(&t)) return node2(TAG, t);
	else if (open_sign) return compound(q, idf);
	else {
		parerr(MESS(2811, "no template operand where expected"));
		tx= q;
		return NilTree;
	}
}

/* ******************************************************************** */
/*		unit_command_suite					*/
/* ******************************************************************** */

Visible parsetree ucmd_seq(cil, first, emp) intlet cil; bool first, *emp; {
	value c;
	intlet level= ilev();
	intlet l= lino;

	if (is_comment(&c)) 
		return node5(SUITE, mk_integer(l), NilTree, c,
				ucmd_seq(cil, first, emp));
	if ((level == cil && !first) || (level > cil && first)) {
		parsetree v;
		findceol();
		if (share(ceol, &v, &c)) 
			return node5(SUITE, mk_integer(l), v, c,
					ucmd_seq(level, No, emp));
		veli();
		*emp= No;
		return cmd_suite(cil, first, cmd_seq);
	}
	veli();
	return NilTree;
} 

Hidden bool share(q, v, c) txptr q; parsetree *v; value *c; {
	char *kw;
	txptr tx0= tx;
	
	if (is_cmdname(q, &kw) && share_keyword(kw)) {
		idf_cntxt= In_share;
		*v= node2(SHARE, idf(q));
		*c= tail_line();
		return Yes;
	}
	else tx= tx0;
	return No;
}


/* ******************************************************************** */
/*		refinement_suite					*/
/* ******************************************************************** */

Hidden parsetree  ref_suite(cil) intlet cil; {
	char *kw;
	value name= Vnil;
	bool t;
	txptr tx0;
	
	if (ilev() != cil) {
		parerr(WRONG_INDENT);
		return NilTree;
	}
	tx0= tx;
	findceol();
	if ((t= is_tag(&name)) || is_cmdname(ceol, &kw)) {
		parsetree v, s;
		value w, *aa, r;
		
		skipsp(&tx);
		if (Char(tx) != ':') {
			release(name);
			tx= tx0;
			veli();
			return NilTree;
		}
		/* lino= 1; cntxt= In_ref; */
		tx++;
		if (t) {
			idf_cntxt= In_ref;
			treat_idf(name);
		}
		else name= mk_text(kw);
		if (in_env(reftab, name, &aa)) 
			pprerrV(REF_IN_REF, name);
		if (!is_comment(&w)) w= Vnil;
		s= cmd_suite(cil, Yes, cmd_seq);
		v= node6(REFINEMENT, name, w, s, Vnil, Vnil);
		e_replace(r= mk_ref(v), &reftab, name);
		release(r);
		*Branch(v, REF_NEXT)= ref_suite(cil);
		return v;
	} 
	veli();
	return NilTree;
}

/* ******************************************************************** */
/*		collateral, compound					*/
/* ******************************************************************** */

Hidden parsetree n_collateral(q, n, base) txptr q; intlet n;
		parsetree (*base)(); {
	parsetree v, w; txptr ftx, ttx;
	if (find(S_COMMA, q, &ftx, &ttx)) {
		w= (*base)(ftx); tx= ttx;
		v= n_collateral(q, n+1, base);
	}
	else {
		w= (*base)(q);
		if (n == 1) return w;
		v= mk_compound(n);
	}
	*Field(v, n-1)= w;
	return n > 1 ? v : node2(COLLATERAL, v);
}

Visible parsetree collateral(q, base) txptr q; parsetree (*base)(); {
	return n_collateral(q, 1, base);
}

Visible parsetree compound(q, base) txptr q; parsetree (*base)(); {
	parsetree v; txptr ftx, ttx;
	req(S_CLOSE, q, &ftx, &ttx);
	v= (*base)(ftx); tx= ttx;
	return node2(COMPOUND, v);
}

/* ******************************************************************** */
/*		idf, singidf						*/
/* ******************************************************************** */

Hidden parsetree singidf(q) txptr q; {
	parsetree v;
	skipsp(&tx);
	if (nothing(q, MESS(2812, "nothing instead of expected name")))
		v= NilTree;
	else if (open_sign)
		v= compound(q, idf);
	else if (is_tag(&v)) {
		treat_idf(v);
		v= node2(TAG, v);
	}
	else {
		parerr(MESS(2813, "no name where expected"));
		v= NilTree;
	}
	upto1(q, MESS(2814, "something unexpected in name"));
	return v;
}

Visible parsetree idf(q) txptr q; {
	return collateral(q, singidf);
}
