/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1986. */

/* B interpreter -- independent subroutines */

#include "b.h"
#include "bint.h"
#include "bobj.h"
#include "i0err.h"
#include "i3cen.h"
#include "i3env.h"
#include "i3in2.h"
#include "i3sou.h"

/* Newlines for WRITE /// */

Visible Procedure nl(n) value n; {
	value l= size(n); int c= intval(l); release(l);
	while (c--) writnewline();
}


/* Evaluating basic targets */

Visible value v_local(name, number) value name, number; {
	value *aa= envassoc(curnv->tab, number);
	if (aa != Pnil && *aa != Vnil) {
		if (Is_indirect(*aa)) {
			value v= Indirect(*aa)->val;
			if (v == Vnil) interrV(NO_VALUE, name);
			return copy(v);
		}
		else return copy(*aa);
	}
	interrV(NO_VALUE, name);
	return Vnil;
}

Visible value v_global(name) value name; {
	value *aa= envassoc(prmnv->tab, name);
	if (aa != Pnil && *aa != Vnil) {
		if (Is_indirect(*aa)) {
			load_global(*aa, name, Yes);
			return copy(Indirect(*aa)->val);
		}
		else return copy(*aa);
	}
	interrV(NO_VALUE, name);
	return Vnil;
}

Visible Procedure load_global(v, name, err)
     value v;
     value name;
     bool err;
{
	indirect *w= Indirect(v);

	if (!Valid(w->val)) {
		value *aa, pname = permkey(name, Tar);
		wsenvptr oldwse;

		oldwse = setcurenv(use_env);

		if (p_exists(pname, &aa)) {
			w->val = gettarval(*aa, name);
		}
		else if (err)
			interrV(NO_VALUE, name);
		release(pname);

		resetcurenv(oldwse);
	}
}

/* Rangers */

/* An IN-ranger is represented on the stack as a compound of three fields:
   the last index used, the value of the expression after IN, and its length.
   (The latter is redundant, but saves save many calls of 'size()'.)
   When first called, there is, of course, no compound on the stack, but only
   the value of the expression.  As the expression should always be a text,
   list or table, this is recognizable as a special case, and then the
   compound is created.
   Return value is Yes if a new element was available and assigned, No if not.
*/

Visible bool in_ranger(l, pv) loc l; value *pv; {
	value v= *pv, ind, tlt, len, i1, val; bool res;
	if (!Is_compound(v) || Nfields(v) != 3) { /* First time */
		tlt= v;
		if (!Is_tlt(tlt)) {
			interr(MESS(3400, "in ... i IN e, e is not a text, list or table"));
			return No;
		}
		if (empty(tlt)) return No;
		*pv= v= mk_compound(3);
		*Field(v, 0)= ind= one;
		*Field(v, 1)= tlt;
		*Field(v, 2)= len= size(tlt);
		bindlocation(l);
	}
	else {
		ind= *Field(v, 0); tlt= *Field(v, 1); len= *Field(v, 2);
		res= numcomp(ind, len) < 0;
		if (!res) { unbind(l); return No; }
		*Field(v, 0)= ind= sum(i1= ind, one); release(i1);
	}
	put(val= item(tlt, ind), l); release(val);
	return Yes;
}
