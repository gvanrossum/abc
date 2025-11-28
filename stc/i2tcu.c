/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1986. */

/* unification of polytypes */

#include "b.h"
#include "bobj.h"
#include "i2stc.h"

Forward Hidden Procedure u_unify();
Forward Hidden Procedure unify_subtypes();
Forward Hidden Procedure substitute_for();
Forward Hidden Procedure textify();

Hidden bool bad;

Visible Procedure unify(a, b, pu)
polytype a, b, *pu;
{
	bad = No;
	setreprtable();
	starterrvars();
#ifdef TYPETRACE
	s_unify(a, b);
#endif
	u_unify(a, b, pu);
#ifdef TYPETRACE
	e_unify(*pu);
#endif
	if (bad) badtyperr(a, b);
	enderrvars();
	delreprtable();
}

Hidden Procedure u_unify(a, b, pu)
polytype a, b, *pu;
{
	typekind a_kind, b_kind;
	polytype res;
	
	a_kind = kind(a);
	b_kind = kind(b);
	
	if (are_same_types(a, b)) {
		*pu = p_copy(a);
	}
	else if (t_is_var(a_kind) || t_is_var(b_kind)) {
		substitute_for(a, b, pu);
	}
	else if (have_same_structure(a, b)) {
		unify_subtypes(a, b, pu);
	}
	else if (has_number(a_kind) && has_number(b_kind)) {
		*pu = mkt_number();
	}
	else if (has_text(a_kind) && has_text(b_kind)) {
		*pu = mkt_text();
	}
	else if (has_text(a_kind) && t_is_tlt(b_kind)) {
		u_unify(asctype(b), (res = mkt_text()), pu);
		p_release(res);
	}
	else if (has_text(b_kind) && t_is_tlt(a_kind)) {
		u_unify(asctype(a), (res = mkt_text()), pu);
		p_release(res);
	}
	else if ((t_is_list(a_kind) && has_lt(b_kind))
		 ||
		 (t_is_list(b_kind) && has_lt(a_kind))
	)
	{
		u_unify(asctype(a), asctype(b), &res);
		*pu = mkt_list(res);
	}
	else if (t_is_table(a_kind) && has_lt(b_kind)) {
		u_unify(asctype(a), asctype(b), &res);
		*pu = mkt_table(p_copy(keytype(a)), res);
	}
	else if (t_is_table(b_kind) && has_lt(a_kind)) {
		u_unify(asctype(a), asctype(b), &res);
		*pu = mkt_table(p_copy(keytype(b)), res);
	}
	else if ((t_is_tlt(a_kind) && t_is_lt(b_kind))
		 || 
		 (t_is_lt(a_kind) && t_is_tlt(b_kind)))
	{
		u_unify(asctype(a), asctype(b), &res);
		*pu = mkt_lt(res);
	}
	else if (t_is_error(a_kind) || t_is_error(b_kind)) {
		*pu = mkt_error();
	}
	else {
		*pu = mkt_error();
		bad = Yes;
	}
	if (t_is_var(a_kind) && t_is_error(kind(bottomtype(*pu))))
		adderrvar(a);
	if (t_is_var(b_kind) && t_is_error(kind(bottomtype(*pu))))
		adderrvar(b);
}

Hidden Procedure unify_subtypes(a, b, pu)
polytype a, b, *pu;
{
	polytype sa, sb, s;
	intlet nsub, is;
	bool err = No;
	
	nsub = nsubtypes(a);
	*pu = mkt_polytype(kind(a), nsub);
	for (is = 0; is < nsub; is++) {
		sa = subtype(a, is);
		sb = subtype(b, is);
		u_unify(sa, sb, &s);
		putsubtype(s, *pu, is);
		if (t_is_error(kind(s)))
			err = Yes;
	}
	if (err == Yes) {
		p_release(*pu);
		*pu = mkt_error();
	}
}

Forward bool contains();
Forward bool equal_vars();

Hidden Procedure substitute_for(a, b, pu)
polytype a, b, *pu;
{
	typekind a_kind, b_kind;
	polytype ta, tb, tu, tt;
	
	a_kind = kind(a);
	b_kind = kind(b);
	
	ta = bottomtype(a);
	tb = bottomtype(b);
	
	if (!t_is_var(kind(ta)) && !t_is_var(kind(tb)))
		u_unify(ta, tb, &tu);
	else if (!t_is_var(kind(ta)))
		tu = p_copy(ta);
	else
		tu = p_copy(tb);
	
	if (t_is_var(a_kind)) {
		if (contains(tu, bottomvar(a)))
			textify(a, &tu);
	}
	if (t_is_var(b_kind)) {
		if (contains(tu, bottomvar(b)))
			textify(b, &tu);
	}
	
	if (t_is_var(a_kind) && t_is_var(b_kind)
	    && !are_same_types(bottomvar(a), bottomvar(b)))
	{
	    	repl_type_of(bottomvar(a), bottomvar(b));
	}
	
	tt= bottomtype(tu);
	
	if (t_is_var(a_kind)) {
		if (!are_same_types(tt, bottomtype(a)))
			repl_type_of(bottomvar(a), tt);
		*pu= p_copy(a);
	}
	else { /* t_is_var(b_kind) */
		if (!are_same_types(tt, bottomtype(b)))
			repl_type_of(bottomvar(b), tt);
		*pu= p_copy(b);
	}
	
	p_release(tu);
}

Hidden Procedure textify(a, pu)
polytype a, *pu;
{
	polytype ttext, text_hopefully;
	
	ttext = mkt_text();
	u_unify(*pu, ttext, &text_hopefully);
	if (bad == No) {
		p_release(text_hopefully);
		u_unify(a, ttext, &text_hopefully);
	}
	p_release(*pu);
	if (bad == No) {
		*pu = ttext;
	}
	else {
		*pu = mkt_error();
		/* cyclic type errors now reported through normal mechanism */
		p_release(ttext);
	}
	p_release(text_hopefully);
}

Visible bool contains(u, a) polytype u, a; {
	bool result;
	
	result = No;
	if (t_is_var(kind(u))) {
		if (table_has_type_of(u)) {
			result = contains(bottomtype(u), a);
		}
	}
	else {
		polytype s;
		intlet is, nsub;
		nsub = nsubtypes(u);
		for (is = 0; is < nsub; is++) {
			s = subtype(u, is);
			if (equal_vars(s, a) || contains(s, a)) {
				result = Yes;
				break;
			}
		}
	}
	return (result);
}

Visible bool equal_vars(s, a) polytype s, a; {
	return (are_same_types(bottomvar(s), a));
}
