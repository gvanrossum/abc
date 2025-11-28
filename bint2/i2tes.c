/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1986. */

#include "b.h"
#include "bint.h"
#include "bobj.h"
#include "i0err.h"
#include "b0lan.h"
#include "i2par.h"
#include "i2nod.h"

/* Avoid name conflict with standard header files: */
#define relop b_relop

Forward Hidden bool conjunction();
Forward Hidden bool disjunction();
Forward Hidden bool negation();
Forward Hidden bool quantification();
Forward Hidden bool cl_test();
Forward Hidden bool order_test();
Forward Hidden Procedure upto_test();
Forward Hidden parsetree right_test();
Forward Hidden parsetree tight_test();
Forward Hidden parsetree ref_or_prop();
Forward Hidden typenode relop();

Visible parsetree test(q) txptr q; {
	parsetree v;
	skipsp(&tx);
	if (!(conjunction(q, &v) || disjunction(q, &v))) v= right_test(q);
	return v;
}

Hidden parsetree right_test(q) txptr q; {
	parsetree v;
	char *kw;
	txptr tx0= tx;
	
	skipsp(&tx);
	if (Text(q) && is_keyword(&kw)) {
		if (negation(kw, q, &v) || quantification(kw, q, &v))
			return v;
		else tx= tx0;
	}
	return tight_test(q);
}

Hidden bool conjunction(q, v) txptr q; parsetree *v; {
	txptr ftx, ttx;
	if (find(K_AND, q, &ftx, &ttx)) {
		parsetree t;
		t= tight_test(ftx); tx= ttx;
		if (!conjunction(q, v)) *v= right_test(q);
		*v= node3(AND, t, *v);
		return Yes;
	}
	return No;
}

Hidden bool disjunction(q, v) txptr q; parsetree *v; {
	txptr ftx, ttx;
	if (find(K_OR, q, &ftx, &ttx)) {
		parsetree t;
		t= tight_test(ftx); tx= ttx;
		if (!disjunction(q, v)) *v= right_test(q);
		*v= node3(OR, t, *v);
		return Yes;
	}
	return No;
}

Hidden bool negation(kw, q, v) char *kw; txptr q; parsetree *v; {
	if (not_keyword(kw)) {
		*v= node2(NOT, right_test(q));
		return Yes;
	}
	return No;
}

Hidden bool quantification(kw, q, v) char *kw; txptr q; parsetree *v; {
	bool some, each;
	if ((some= some_keyword(kw)) || (each= each_keyword(kw)) || 
			no_keyword(kw)) {
		parsetree t, w; 
		typenode type;
		txptr utx, vtx, ftx, ttx;
		
		req(K_HAS, ceol, &utx, &vtx);
		if (utx > q) {
			parerr(MESS(2700, "HAS follows colon"));
			/* as in: SOME i IN x: SHOW i HAS a */
			utx= tx; vtx= q;
		}
		req(K_IN_quant, utx, &ftx, &ttx);
		idf_cntxt= In_ranger;
		t= idf(ftx); tx= ttx;
		w= expr(utx); tx= vtx;
		type= some ? SOME_IN : each ? EACH_IN : NO_IN;
		*v= node4(type, t, w, right_test(q));
		return Yes;
	}
	return No;
}

Hidden parsetree tight_test(q) txptr q; {
	parsetree v;
	skipsp(&tx);
	if (nothing(q, MESS(2701, "nothing instead of expected test"))) 
		v= NilTree;
	else if (!(cl_test(q, &v) || order_test(q, &v))) {
		if (Isexpr(Char(tx))) v= ref_or_prop(q);
		else {
			parerr(NO_TEST);
			v= NilTree;
		}
	}
	upto_test(q);
	return v;
}

Hidden bool cl_test(q, v) txptr q; parsetree *v; {
	txptr tx0= tx;
	if (open_sign) { /* (expr) or (test) */
		txptr ftx, ttx, tx1;
		tx1= tx;
		req(S_CLOSE, q, &ftx, &ttx); tx= ttx;
		skipsp(&tx);
		if (!Text(q)) {
			tx= tx1;
			*v= compound(ttx, test);
			return Yes;
		}
	}
	tx= tx0;
	return No;
}

Hidden bool order_test(q, v) txptr q; parsetree *v; {
	txptr ftx;
	if (findrel(q, &ftx)) {
		typenode r;
		*v= singexpr(ftx);
		do {
			r= relop();
			if (!findrel(q, &ftx)) ftx= q;
			*v= node3(r, *v, singexpr(ftx));
		}
		while (ftx < q);
		return Yes;
	}
	return No;
}

Hidden typenode relop() {
	skipsp(&tx);
	return
		at_most_sign		? AT_MOST :
		unequal_sign		? UNEQUAL :
		at_least_sign		? AT_LEAST :
		equals_sign		? IS_EQUAL :
		less_than_sign		? LESS_THAN :
		greater_than_sign	? GREATER_THAN :
		/* psyserr */		  Nonode;
}

/* refined_test or proposition */

Hidden parsetree ref_or_prop(q) txptr q; {
	value t1, t2;
	txptr tx0= tx;
	
	if (tag_operator(q, &t1)) {
		skipsp(&tx);
		if (!Text(q))
			return node2(TAG, t1);
		if (tag_operator(q, &t2)) {
			skipsp(&tx);
			if (!Text(q))
				return node4(MONPRD, t1, node2(TAG, t2), Vnil);
			release(t2);
		}
		release(t1);
	}
	tx= tx0;
	return unp_test(q);
} 

Hidden Procedure upto_test(q) txptr q; {
	skipsp(&tx);
	if (Text(q)) {
		txptr ftx, ttx;
		if (find(K_AND, q, &ftx, &ttx) || find(K_OR, q, &ftx, &ttx)) {
			tx= ftx;
			parerr(PRIO);
		}
		else parerr(UPTO_TEST);
		tx= q;
	}
}
