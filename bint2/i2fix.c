/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1986. */

/* Fix unparsed expr/test */

#include "b.h"
#include "bint.h"
#include "bobj.h"
#include "i0err.h"
#include "i2exp.h"
#include "i2nod.h"
#include "i2gen.h" /* Must be after i2nod.h */
#include "i2par.h"
#include "i3env.h"

#define S_elmt '1'
#define S_dya  '2'
#define S_mon  '3'

Hidden Procedure f_unparsed(pt, fct) parsetree *pt, (*fct)(); {
	parsetree t= *pt;
	expadm adm;
	struct state v;

	/* Ignore visits done during resolving UNPARSED: */
	hold(&v);
	initexp(&adm, N_EXP_STACK, FIXER);
	t= (*fct)(&adm, *Branch(t, UNP_SEQ));
	release(*pt);
	*pt= t;
	endstack(&adm);
	jumpto(NilTree);
	let_go(&v);
}

Hidden parsetree fix_expr(adm, root) expadm *adm; parsetree root; {
	parsetree w;
	value *p_i, i;
	int state= S_dya;

	for (; Nfld(adm) < Nfields(root); ++Nfld(adm)) {
		p_i= Field(root, Nfld(adm));
		i= copy(*p_i);
		if (!Valid(i)) {
			if (state == S_dya || state == S_mon)
				fixerr(NO_EXPR);
			else if (Prop(adm))
				break;
			else
				fixerr(UPTO_EXPR);
			return NilTree;
		}
		else if (state == S_dya || state == S_mon) {
			if (Is_parsetree(i)) {
				f_expr(p_i);
				release(i); i= copy(*p_i);
				push_item(adm, (parsetree) i);
				state= S_elmt;
			}
			else if (modify_tag(i, &w)) {
				push_item(adm, w);
				state= S_elmt;
			}
			else if (is_monfun(i, Pnil)) {
				push_item(adm, (parsetree) i);
				state= S_mon;
			}
			else {
				if (is_name(i))
					fixerrV(NO_INIT_OR_DEF, i);
				else
					fixerr(NO_EXPR);
				release(i);
				return NilTree;
			}
		}
		else {				/* state == S_elmt */
			if (Dya_opr(i)) {
				release(i);
				i= copy(*Field(i, 0));
			}
			if (is_dyafun(i, Pnil)) {
				do_dya(adm, i);
				state= S_dya;
			}
			else {
				release(i);
				if (Prop(adm)) break;
				else {
					fixerr(UPTO_EXPR);
					return NilTree;
				}
			}
		}
	}
	if (state == S_dya || state == S_mon) {
		fixerr(NO_EXPR);
		return NilTree;
	}
	while ((Sp(adm) - Stack(adm)) > 2)
		reduce(adm);
	return Pop(adm);
}

Hidden parsetree fix_test(adm, root) expadm *adm; parsetree root; {
	parsetree v, w;
	value i, f, *aa;
	int lastn= Nfields(root) - 1;
	
	if (Nfld(adm) > lastn) {
		fixerr(NO_TEST);
		return NilTree;
	}
	i= *Field(root, Nfld(adm));
	if (!Valid(i))
		;
	else if (is_zerprd(i, &f)) {
		if (Nfld(adm) < lastn) {
			fixerr(UPTO_TEST);
			return NilTree;
		}
		return node3(TAGzerprd, copy(i), copystddef(f));
	}
	else if (Is_text(i) && (aa= envassoc(refinements, i))) {
		if (Nfld(adm) == lastn) 
			return node3(TAGrefinement, copy(i), copy(*aa));
	}
	else if (is_monprd(i, &f)) {
		++Nfld(adm);
		v= fix_expr(adm, root);
		return node4(MONPRD, copy(i), v, copystddef(f));
	}
	Prop(adm)= Yes;
	v= fix_expr(adm, root);
	Prop(adm)= No;
	i= Nfld(adm) <= lastn ? *Field(root, Nfld(adm)) : Vnil;
	if (!Valid(i)) {
		fixerr(NO_TEST);
		release(v);
		return NilTree;
	}
	if (Dya_opr(i))
		i= *Field(i, 0);
	if (!is_dyaprd(i, &f)) {
		if (is_name(i))
			fixerrV(NO_DEFINITION, i);
		else
			fixerr(NO_TEST);
		release(v);
		return NilTree;
	}
	++Nfld(adm);
	w= fix_expr(adm, root);
	return node5(DYAPRD, v, copy(i), w, copystddef(f));
}

Visible Procedure f_eunparsed(pt) parsetree *pt; {
	f_unparsed(pt, fix_expr);
}

Visible Procedure f_cunparsed(pt) parsetree *pt; {
	f_unparsed(pt, fix_test);
}

Visible Procedure f_trim_target(v, trim) parsetree v; char trim; {
	parsetree w= *Branch(v, TRIM_RIGHT);
	struct prio *ptrim, *pdya;
	value name;

	if (nodetype(w) == DYAF) {
		pdya= dprio(*Branch(w, DYA_NAME));
		name= mk_text(trim == '@' ? S_BEHEAD : S_CURTAIL);
		ptrim= dprio(name);
		if (!(pdya->L > ptrim->H))
			fixerr(NO_TRIM_TARG);
		release(name);
	}
}
