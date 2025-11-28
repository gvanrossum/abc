/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1986. */

/*
 * B editor -- All routines referencing the grammar table are in this file.
 */

#include "b.h"
#include "bedi.h"
#include "etex.h"
#include "bmem.h"
#include "bobj.h"
#include "node.h"
#include "gram.h"
#include "supr.h"
#include "tabl.h"
#include "code.h"	/* not strictly necessary, only for initcodes() */
#include "port.h"

Forward Hidden Procedure fix_refcnt();
Forward Hidden Procedure makesugg();

/*
 * Test whether sym is in the given class.
 */

Visible bool
isinclass(sym, ci)
	register int sym;
	struct classinfo *ci;
{
	register classptr cp;

	Assert(ci && ci->c_class);
	if (sym == Hole)
		return !isinclass(Optional, ci);
	for (cp = ci->c_class; *cp; ++cp)
		if (sym == *cp)
			return Yes;
	return No;
}


/*
 * Deliver the representation array for the given node.
 * If the node is actually just a "text" value, construct
 * one in static storage -- which is overwritten at each call.
 * In this case there are two deficiencies: the next call to
 * noderepr which uses the same feature overwrites the reply
 * value of the previous call, AND if the text value itself
 * is changed, the representation may change, too.
 * In practical use this is no problem at all, however.
 */

Visible string *
noderepr(n)
	register node n;
{
	register int sym;

	if (n && Is_etext(n)) {
		static string buf[2];
		if (buf[0]) e_fstrval(buf[0]);
		buf[0] = e_sstrval((value)n);
		return buf;
	}
	sym = symbol(n);
	return table[sym].r_repr;
}

Visible Procedure endnoderepr() { /* hack to free noderepr static store */
#ifdef MEMTRACE
	value v= mk_etext("dummy");
	string *s= noderepr((node)v);
	freemem((ptr) s[0]);
	release(v);
#endif
}

/*
 * Deliver the prototype node for the given symbol.
 */

Visible node
gram(sym)
	register int sym;
{
	Assert(0 <= sym && sym < TABLEN);
	return table[sym].r_node;
}

#ifdef SAVEBUF

/*
 * Deliver the name of a symbol.
 */

Visible string
symname(sym)
	int sym;
{
	static char buf[20];

	if (sym >= 0 && sym < TABLEN && table[sym].r_name)
		return table[sym].r_name;
	sprintf(buf, "%d", sym);
	return buf;
}


/*
 * Find the symbol corresponding to a given name.
 * Return -1 if not found.
 */

Visible int
nametosym(str)
	register string str;
{
	register int sym;
	register string name;

	for (sym = 0; sym < TABLEN; ++sym) {
		name = table[sym].r_name;
		if (name && !strcmp(name, str))
			return sym;
	}
	return -1;
}

#endif /* SAVEBUF */

/*
 * Test whether `sym' may replace the node in the path `p'.
 */

Visible bool
allowed(p, sym)
	register path p;
	register int sym;
{
	register path pa = parent(p);
	register int ich = ichild(p);
	register int sympa = pa ? symbol(tree(pa)) : Rootsymbol;

	Assert(sympa >= 0 && sympa < TABLEN && ich > 0 && ich <= MAXCHILD);
	return isinclass(sym, table[sympa].r_class[ich-1]);
}


/*
 * Initialize (and verify) the grammar table.
 * (sets refcnt to infinity)
 */

Visible Procedure
initgram()
{
	register int sym;
	register int nch;
	register struct classinfo **cp;
	register struct classinfo *sp;
	node ch[MAXCHILD];

	/* Set the node pointers in the table and check the representations.
	   The code assumes Optional and Hole are the last
	   symbols in the table, i.e. the first processed by the loop. */

	for (sym = TABLEN-1; sym >= 0; --sym) {
		cp = table[sym].r_class;
		for (nch = 0; nch < MAXCHILD && (sp = cp[nch]); ++nch)
			ch[nch] =
				table[sp->c_class[0] == Optional ? 
					Optional : Hole].r_node;
		table[sym].r_node = newnode(nch, sym, ch);
		fix_refcnt(table[sym].r_node);
	}
	initcodes();
}

/*
 * Set a node's refcnt to infinity, so it will never be released.
 */

Hidden Procedure
fix_refcnt(n)
	register node n;
{
	Assert(n->refcnt > 0);
	n->refcnt = Maxrefcnt;
#ifdef MEMTRACE
	fixmem((ptr) n);
#endif
}

/*
 * Add built-in commands to the suggestion tables.
 */

Visible Procedure
initclasses()
{
#ifdef USERSUGG
	register struct table *tp;
	
	tp= &table[Rootsymbol];
	Assert(isinclass(Suggestion, tp->r_class[0]));
	makesugg(tp->r_class[0]->c_class);
#endif /* USERSUGG */
}

#ifdef USERSUGG

/*
 * Extract suggestions from class list.
 */

Hidden Procedure makesugg(cp) classptr cp; {
	struct table *tp;
	string *rp;
	char buffer[1000];
	string bp;
	string sp;
	int i;
	int nch;

	for (; *cp; ++cp) {
		if (*cp >= TABLEN)
			continue;
		Assert(*cp > 0);
		tp = &table[*cp];
		rp = tp->r_repr;
		if (rp[0] && isupper(rp[0][0])) {
			bp = buffer;
			nch = nchildren(tp->r_node);
			for (i = 0; i <= nch; ++i) {
				if (rp[i]) {
					for (sp = rp[i]; *sp >= ' '; ++sp)
						*bp++ = *sp;
				}
				if (i < nch && !isinclass(Optional, tp->r_class[i]))
					*bp++ = '?';
			}
			if (bp > buffer) {
				*bp = 0;
				addsugg(buffer, (int) *cp);
			}
		}
	}
}

#endif /* USERSUGG */

/*
 * Set the root of the grammar to the given symbol.  It must exist.
 */

Visible Procedure
setroot(isym) int isym; {	/* symbols defined in tabl.h */
	register int ich;

	table[Rootsymbol].r_name = table[isym].r_name;
	for (ich = 0; ich < MAXCHILD; ++ich) {
		table[Rootsymbol].r_repr[ich] = table[isym].r_repr[ich];
		table[Rootsymbol].r_class[ich] = table[isym].r_class[ich];
	}
	table[Rootsymbol].r_repr[ich] = table[isym].r_repr[ich];
	table[Rootsymbol].r_node = table[isym].r_node;
}

/*
 * The remainder of this file is specific for the currently used grammar.
 */

/*
 * Table indicating which symbols are used to form lists of items.
 * Consulted via predicate 'issublist'.
 */

Hidden classelem Asublists[] = {
	Exp_plus, Formal_naming_plus,
	And, And_kw, Or, Or_kw,
	0
};

Hidden struct classinfo sublists[] = {{Asublists, 0, 0, 0}};


/*
 * Predicate telling whether two symbols can form lists together.
 * This is important for list whose elements must alternate in some
 * way, as is the case for [KEYWORD [expression] ]*.
 *
 * This code must be in this file, otherwise the names and values
 * of the symbols would have to be made public.
 */

Visible bool
samelevel(sym, sym1)
	register int sym;
	register int sym1;
{
	register int zzz;

	if (sym1 == sym)
		return Yes;
	if (sym1 < sym)
		zzz = sym, sym = sym1, sym1 = zzz; /* Ensure sym <= sym1 */
	/* Now always sym < sym1 */
	return sym == Kw_plus && sym1 == Exp_plus
		|| sym == Formal_kw_plus && sym1 == Formal_naming_plus
		|| sym == And && sym1 == And_kw
		|| sym == Or && sym1 == Or_kw;
}


/*
 * Predicate to tell whether a symbol can form chained lists.
 * By definition, all right-recursive symbols can do so;
 * in addition, those listed in the class 'sublists' can do
 * it, too (this is used for lists formed of alternating members
 * such as KW expr KW ...).
 */

Visible bool
issublist(sym)
	register int sym;
{
	register int i;
	register string repr;

	Assert(sym < TABLEN);
	if (isinclass(sym, sublists))
		return Yes;
	repr = table[sym].r_repr[0];
	if (Fw_positive(repr))
		return No;
	for (i = 0; i < MAXCHILD && table[sym].r_class[i]; ++i)
		;
	if (i <= 0)
		return No;
	repr = table[sym].r_repr[i];
	if (!Fw_zero(repr))
		return No;
	return isinclass(sym, table[sym].r_class[i-1]);
}

/* true iff parent allows a command with a colon (a control-command);
 * this is false for grammar constructs allowing simple-commands
 * following a colon.
 * sym == symbol(tree(parent(ep->focus)))
 */
Visible bool allows_colon(sym) int sym; {
	switch (sym) {
	case Short_comp:
	case Test_suite:
	case Short_unit:
	case Refinement:
		return No;
	default:
		return Yes;
	}
	/*NOTREACHED*/
}
