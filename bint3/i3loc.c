/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1986. */

/* B locations and environments */
#include "b.h"
#include "bint.h"
#include "bobj.h"
#include "i0err.h"
#include "i3env.h" /* for bndtgs */
#include "i3in2.h"

Forward Hidden bool in_locenv();

#define TAR_NO_INIT	MESS(3600, "location not initialised")
#define TARNAME_NO_INIT	MESS(3601, "%s hasn't been initialised")
#define NO_KEY_OF_TABLE	MESS(3602, "key not in table")
#define INS_NO_LIST	MESS(3603, "inserting in non-list")
#define REM_NO_LIST	MESS(3604, "removing from non-list")
#define REM_EMPTY_LIST	MESS(3605, "removing from empty list")
#define SEL_EMPTY	MESS(3606, "selection on empty table")

#define Is_local(t)	(Is_compound(t))
#define Is_global(t)	(Is_table(t))

#define Loc_indirect(ll) ((ll) != Pnil && *(ll) != Vnil && Is_indirect(*(ll)))

Hidden value* location(l, err) loc l; bool err; {
	value *ll= Pnil, lv;
	
	if (Is_locloc(l)) {
		if (!in_locenv(curnv->tab, l, &ll) && err)
			interr(TAR_NO_INIT);
		return ll;
	}
	else if (Is_simploc(l)) {
		simploc *sl= Simploc(l);
		value ta= sl->e->tab, ke= sl->i;
		
		if (!in_locenv(ta, ke, &ll)) {
			if (Loc_indirect(ll) && Is_global(ta))
				load_global(*ll, ke, err);
			else if (err) {
				if (Is_locloc(ke))
					interr(TAR_NO_INIT);
				else 
					interrV(TARNAME_NO_INIT, ke);
			}
		}
		return ll;
	}
	else if (Is_tbseloc(l)) {
		tbseloc *tl= Tbseloc(l);

		lv= locvalue(tl->R, &ll, err);
		if (lv != Vnil) {
			if (!Is_table(lv)) {
				if (err) interr(SEL_NO_TABLE);
				ll= Pnil;
			}
			else {
				ll= adrassoc(lv, tl->K);
				if (ll == Pnil && err) 
					interr(NO_KEY_OF_TABLE);
			}
		}
		return ll;
	}
	else {
		syserr(MESS(3607, "call of location with improper type"));
		return (value *) Dummy;
	}
}

Visible value locvalue(l, ll, err) loc l; value **ll; bool err; {
	*ll= location(l, err);
	if (*ll == Pnil || **ll == Vnil)
		return Vnil;
	else if (Is_indirect(**ll))
		return Indirect(**ll)->val;
	else return **ll;
}

Hidden bool in_locenv(t, k, ll) value t, k, **ll; {
	*ll= envassoc(t, k);
	if (*ll == Pnil || **ll == Vnil)
		return No;
	else if (Is_indirect(**ll) && Indirect(**ll)->val == Vnil)
		return No;
	else return Yes;
}

Visible Procedure uniquify(l) loc l; {
	if (Is_simploc(l)) {
		simploc *sl= Simploc(l);
		value *ta= &(sl->e->tab), ke= sl->i;
		value *aa;

		check_location(l);
		uniql(ta);
		if (still_ok) {
			if (Is_local(*ta))
				uniql(aa= Field(*ta, SmallIntVal(ke)));
			else {
				VOID uniq_assoc(*ta, ke);
				aa= adrassoc(*ta, ke);
			}
			if (*aa != Vnil && Is_indirect(*aa))
				uniql(&(Indirect(*aa)->val));
		}
	}
	else if (Is_tbseloc(l)) {
		tbseloc *tl= Tbseloc(l);
		value ta, ke, *ll;
		
		uniquify(tl->R);
		if (still_ok) {
			ta= locvalue(tl->R, &ll, Yes);
			ke= tl->K;
			if (!Is_table(ta)) interr(SEL_NO_TABLE);
			else if (empty(ta)) interr(SEL_EMPTY);
			else if (!in_keys(ke, ta)) interr(NO_KEY_OF_TABLE);
			else VOID uniq_assoc(ta, ke);
		}
	}
	else if (Is_trimloc(l)) {
		syserr(MESS(3608, "uniquifying text-selection location"));
	}
	else if (Is_compound(l)) {
		syserr(MESS(3609, "uniquifying comploc"));
	}
	else syserr(MESS(3610, "uniquifying non-location"));
}

Visible Procedure check_location(l) loc l; {
	VOID location(l, Yes);
	/* location may produce an error message */
}

Hidden value content(l) loc l; {
	value *ll;
	value lv= locvalue(l, &ll, Yes);
	return still_ok ? copy(lv) : Vnil;
}

#define TRIM_TARG_TYPE MESS(3611, "text-selection (@ or |) on non-text")
#define TRIM_TARG_TEXT MESS(3612, "in the location t@p or t|p, t does not contain a text")
#define TRIM_TARG_BND  MESS(3613, "in the location t@p or t|p, p is out of bounds")

Visible loc trim_loc(l, N, sign) loc l; value N; char sign; {
	loc root, res= Lnil;
	value text, B, C;
	
	if (Is_simploc(l) || Is_tbseloc(l)) {
		root= l;
		B= zero; C= zero;
	}
	else if (Is_trimloc(l)) {
		trimloc *rr= Trimloc(l);
		root= rr->R;
		B= rr->B; C= rr->C;
	}
	else {
		interr(TRIM_TARG_TYPE);
		return Lnil;
	}
	text= content(root);
	if (!still_ok);
	else if (!Is_text(text))
		interr(TRIM_TARG_TEXT);
	else {
		value n= size(text), w;
		value Bnew= Vnil, Cnew= Vnil;
		bool changed= No;
		
		if (sign == '@') { 	/* behead: B= max{N-1+B, B} */
			Bnew= sum(B, w= diff(N, one));
			if (changed= (compare(Bnew, B) > 0))
				B= Bnew;
		}
		else {			/* curtail: C= max{n-N-B, C} */
			Cnew= diff(w= diff(n, N), B);
			if (changed= (compare(Cnew, C) > 0))
				C= Cnew;
		}
		if (changed) {
			value b_plus_c= sum(B, C);
 			if (still_ok && compare(b_plus_c, n) > 0)
				interr(TRIM_TARG_BND);
			release(b_plus_c);
		}
		if (still_ok) res= mk_trimloc(root, B, C);
		release(Bnew); 
		release(Cnew);
		release(w);
		release(n);
	}
	release(text);
	return res;
}

Visible loc tbsel_loc(R, K) loc R; value K; {
	if (Is_simploc(R) || Is_tbseloc(R)) return mk_tbseloc(R, K);
	else interr(MESS(3614, "selection on location of improper type"));
	return Lnil;
}

Visible loc local_loc(i) basidf i; { return mk_simploc(i, curnv); }

Visible loc global_loc(i) basidf i; { return mk_simploc(i, prmnv); }

Hidden Procedure put_trim(v, tl) value v; trimloc *tl; {
	value rr, nn, head, tail, part, *ll;
	value B= tl->B, C= tl->C, len, b_plus_c, tail_start;
	
	rr= locvalue(tl->R, &ll, Yes);
	len= size(rr);
	b_plus_c= sum(B, C);
 	if (compare(b_plus_c, len) > 0)
		interr(MESS(3615, "text-selection (@ or |) out of bounds"));
	else {
		if (compare(B, zero) < 0) B= zero;
		tail_start= sum(len, one);
		if (compare(C, zero) > 0) {
			tail_start= diff(nn= tail_start, C);
			release(nn);
		}
		head= curtail(rr, B); /* rr|B */
		tail= behead(rr, tail_start); /* rr@(#rr-C+1) */
		release(tail_start);
		part= concat(head, v); release(head);
		nn= concat(part, tail); release(part); release(tail);
		put(nn, tl->R); release(nn);
	}
	release(len); release(b_plus_c);
}

Hidden Procedure rm_indirection(l) loc l; {
	for (; Is_tbseloc(l); l= Tbseloc(l)->R)
		;
	if (Is_simploc(l)) {
		simploc *sl= Simploc(l);
		value *ll= envassoc(sl->e->tab, sl->i);
		
		if (Loc_indirect(ll)) {
			value v= copy(Indirect(*ll)->val);
			release(*ll);
			*ll= v;
		}
	}
}

Visible Procedure put(v, l) value v; loc l; {
	if (Is_locloc(l)) {
		e_replace(v, &curnv->tab, l);
	}
	else if (Is_simploc(l)) {
		simploc *sl= Simploc(l);
 		e_replace(v, &(sl->e->tab), sl->i);
	}
	else if (Is_trimloc(l)) {
		if (!Is_text(v)) interr(MESS(3616, "putting non-text in text-selection (@ or |)"));
		else put_trim(v, Trimloc(l));
	}
	else if (Is_compound(l)) {
		intlet k, len= Nfields(l);
		if (!Is_compound(v))
		    interr(MESS(3617, "putting non-compound in compound location"));
		else if (Nfields(v) != Nfields(l))
		    interr(MESS(3618, "putting compound in compound location of different length"));
		else k_Overfields { put(*Field(v, k), *Field(l, k)); }
	}
	else if (Is_tbseloc(l)) {
		tbseloc *tl= Tbseloc(l);
		uniquify(tl->R);
		if (still_ok) {
			value *ll, lv;
			lv= locvalue(tl->R, &ll, Yes);
			if (!Is_table(lv))
				interr(SEL_NO_TABLE);
			else {
				rm_indirection(tl->R);
				replace(v, ll, tl->K);
			}
		}
	}
	else interr(MESS(3619, "putting in non-location"));
}

/* Check for correct effect of multiple put-command: catches PUT 1, 2 IN x, x.  
   The assignment cannot be undone, but this is not considered a problem.
   For trimmed-texts, no checks are made because the language definition
   itself causes problem (try PUT "abc", "" IN x@2|1, x@3|1). */

Hidden bool putck(v, l) value v; loc l; {
	intlet k, len;
	value *ll, lv;
	if (!still_ok) return No;
	if (Is_compound(l)) {
		if (!Is_compound(v) || Nfields(v) != (len= Nfields(l)))
			return No; /* Severe type error */
		k_Overfields
			{ if (!putck(*Field(v, k), *Field(l, k))) return No; }
		return Yes;
	}
	if (Is_trimloc(l)) return Yes; /* Don't check trim locations */
	lv= locvalue(l, &ll, No);
	return lv != Vnil && compare(v, lv) == 0;
}

/* The check can't be called from within put because put is recursive,
   and so is the check: then, for the inner levels the check would be done
   twice.  Moreover, we don't want to clutter up put, which is called
   internally in, many places. */

Visible Procedure put_with_check(v, l) value v; loc l; {
	intlet i, k, len; bool ok;
	put(v, l);
	if (!still_ok || !Is_compound(l))
		return; /* Single target can't be wrong */
	len= Nfields(l); ok= Yes;
	/* Quick check for putting in all different local targets: */
	k_Overfields {
		if (!IsSmallInt(*Field(l, k))) { ok= No; break; }
		for (i= k-1; i >= 0; --i) {
			if (*Field(l, i) == *Field(l, k)) { ok= No; break; }
		}
		if (!ok) break;
	}
	if (ok) return; /* All different local basic-targets */
	if (!putck(v, l))
		interr(MESS(3620, "putting different values in same location"));
}


#define DEL_NO_TARGET	MESS(3621, "deleting non-location")
#define DEL_TRIM_TARGET	MESS(3622, "deleting text-selection (@ or |) location")

Hidden bool l_exists(l) loc l; {
	if (Is_simploc(l)) {
		simploc *sl= Simploc(l);
		value ta= sl->e->tab, *ll;
		return in_locenv(ta, sl->i, &ll) ||
			Loc_indirect(ll) && Is_global(ta);
	}
	else if (Is_trimloc(l)) {
		interr(DEL_TRIM_TARGET);
		return No;
	}
	else if (Is_compound(l)) {
		intlet k, len= Nfields(l);
		k_Overfields { if (!l_exists(*Field(l, k))) return No; }
		return Yes;
	}
	else if (Is_tbseloc(l)) {
		tbseloc *tl= Tbseloc(l);
		value *ll;
		value lv= locvalue(tl->R, &ll, Yes);
		if (still_ok) {
			if (!Is_table(lv))
				interr(SEL_NO_TABLE);
			else
				return in_keys(tl->K, lv);
		}
		return No;
	}
	else {
		interr(DEL_NO_TARGET);
		return No;
	}
}

/* Delete a location if it exists */

Visible Procedure l_del(l) loc l; {
	if (Is_simploc(l)) {
		simploc *sl= Simploc(l);
		e_delete(&(sl->e->tab), sl->i);
		if (sl->e == prmnv)
			del_target(sl->i);
	}
	else if (Is_trimloc(l)) {
		interr(DEL_TRIM_TARGET);
	}
	else if (Is_compound(l)) {
		intlet k, len= Nfields(l);
		k_Overfields { l_del(*Field(l, k)); }
	}
	else if (Is_tbseloc(l)) {
		tbseloc *tl= Tbseloc(l);
		value *ll, lv;
		uniquify(tl->R);
		if (still_ok) {
			lv= locvalue(tl->R, &ll, Yes);
			if (in_keys(tl->K, lv)) {
				rm_indirection(tl->R);
				delete(ll, tl->K);
			}
		}
	}
	else interr(DEL_NO_TARGET);
}

Visible Procedure l_delete(l) loc l; {
	if (l_exists(l)) l_del(l);
	else interr(MESS(3623, "deleting non-existent location"));
}

Visible Procedure l_insert(v, l) value v; loc l; {
	value *ll, lv;
	uniquify(l);
	if (still_ok) {
		lv= locvalue(l, &ll, Yes);
		if (!Is_list(lv))
			interr(INS_NO_LIST);
		else {
			rm_indirection(l);
			insert(v, ll);
		}
	}
}

Visible Procedure l_remove(v, l) value v; loc l; {
	value *ll, lv;
	uniquify(l);
	if (still_ok) {
		lv= locvalue(l, &ll, Yes);
		if (!Is_list(lv))
			interr(REM_NO_LIST);
		else if (empty(lv))
			interr(REM_EMPTY_LIST);
		else {
			rm_indirection(l);
			remove(v, ll);
		}
	}
}

Visible Procedure bindlocation(l) loc l; {
	if (*bndtgs != Vnil) {
		if (Is_simploc(l)) {
			simploc *ll= Simploc(l);
			if (!in(ll->i, *bndtgs)) /* kludge */ /* what for? */
				insert(ll->i, bndtgs);
		}
		else if (Is_compound(l)) {
			intlet k, len= Nfields(l);
			k_Overfields { bindlocation(*Field(l, k)); }
		}
		else interr(MESS(3624, "binding non-location"));
	}
	l_del(l);
}

Visible Procedure unbind(l) loc l; {
	if (*bndtgs != Vnil) {
		if (Is_simploc(l)) {
			simploc *ll= Simploc(l);
			if (in(ll->i, *bndtgs))
				remove(ll->i, bndtgs);
		}
		else if (Is_compound(l)) {
			intlet k, len= Nfields(l);
			k_Overfields { unbind(*Field(l, k)); }
		}
		else interr(MESS(3625, "unbinding non-location"));
	}
	l_del(l);
}
