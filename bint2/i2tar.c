/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1986. */

#include "b.h"
#include "bint.h"
#include "bobj.h"
#include "i2par.h"
#include "i2nod.h"

Forward Hidden parsetree singtarg();

Visible parsetree targ(q) txptr q; {
	return collateral(q, singtarg);
}

Hidden parsetree singtarg(q) txptr q; {
	parsetree v; value t;
	skipsp(&tx);
	if (nothing(q, MESS(2500, "nothing where address expected"))) 
		return NilTree;
	if (open_sign) v= compound(q, targ);
	else if (is_tag(&t)) v= node2(TAG, t);
	else {
		parerr(MESS(2501, "no address where expected"));
		tx= q;
		return NilTree;
	}
	selection(q, &v);
	trim_target(q, &v);
	upto1(q, MESS(2502, "something unexpected in address"));
	return v;
}
