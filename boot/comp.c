/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1988. */

/*
 * Compute classinfo from filled-in tables.
 */

#include "b.h"
#include "main.h"
#include "code.h"

Visible Procedure compute_classes() {
	
	initcodes();
	
	comp_classes();
}

/*
 * Initialization routine for the 'struct classinfo' stuff.
 *
 * Now that the c_syms[] array of each class has been read and replaced
 * by the correct index in the symdef[] table, we can compute the c_insert,
 * c_append and c_join arrays.
 *
 * Classes "suggestion-body" and "sugghowname-body" are skipped:
 * what can be inserted there is not computed from this table.
 */

Hidden Procedure comp_classes()
{
	int iclass;
	struct classinfo *pclass;

	for (iclass= 0; iclass < nclass; iclass++) {
		pclass = &classdef[iclass];
		if (iclass == nsuggstnbody || iclass == nsugghowbody)
			continue; /* Dead entry */
		defclass(pclass);
	}
}

Forward int fwidth();

Hidden Procedure defclass(pclass) struct classinfo *pclass; {
	itemptr psymbol;
	struct syminfo *psym;
	string rep0;
	item class0;
	string rep1;
	int fw1;
	itemptr psubsym;
	item insert[1024];
	item append[1024];
	item join[1024];
	int inslen = 0;
	int applen = 0;
	int joinlen = 0;
	int c;


	psymbol= pclass->c_syms;

	for (; !Isnilitem(*psymbol); ++psymbol) {
		if (*psymbol == noptional)
			continue;
		if (*psymbol >= nlexical) { /* Insert direct lexical item */
			for (c= 1; c <= lastcode; c++) {
				if (maystart(Invcode(c), *psymbol)) {
		Assert(inslen+3 < sizeof insert / sizeof insert[0]);
					insert[inslen] = c;
					insert[inslen+1] = *psymbol;
					inslen += 2;
				}
			}
			continue;
		}
		/* else: Sym: "rep0", class0, "rep1", class1, ... */
		psym= &symdef[*psymbol];
		rep0= psym->s_repr[0];
		if (rep0 != 0 && strchr("\b\t", rep0[0]) == NULL) {
			/* Insert fixed text */
			c = Code(rep0[0]);
		Assert(inslen+3 < sizeof insert / sizeof insert[0]);
			insert[inslen] = c;
			insert[inslen+1] = *psymbol;
			inslen += 2;
			continue;
		}
		/* else: "rep0" was empty; try start of class0 */
		Assert(!Isnilitem(psym->s_class[0]));
		class0= psym->s_class[0];
		psubsym= classdef[class0].c_syms;
		for (; !Isnilitem(*psubsym); psubsym++) {
			if (*psubsym < nlexical)
				continue;
			for (c= 1; c <= lastcode; ++c) { 
				/* Insert indirect lexical items */
				if (maystart(Invcode(c), *psubsym)) {
		Assert(inslen+3 < sizeof insert / sizeof insert[0]);
					insert[inslen]= c;
					insert[inslen+1]= *psymbol;
					inslen += 2;
				}
			}
		}
		rep1= psym->s_repr[1];
		fw1= (rep1 == 0 ? 0 : fwidth(rep1));
		if (fw1) { /* Append */
			c= rep1[0];
			Assert(c > 0 && c < RANGE);
			if (c == ' ') {
				c= rep1[1];
				if (!c || c == '\b' || c == '\t')
					c= ' ';
				else
					c|= 0200;
			}
			Assert(applen+3 < sizeof append / sizeof append[0]);
			append[applen]= c;
			append[applen+1]= *psymbol;
			applen += 2;
		}
		if ((!fw1 || fw1 == 1 && rep1[0] == ' ')
		    &&
		    !Isnilitem(psym->s_class[1]))
		{ /* Join */
			Assert(joinlen+3 < sizeof join / sizeof join[0]);
			join[joinlen]= 1 + fw1;
			join[joinlen+1]= *psymbol;
			joinlen += 2;
		}
	}

	Assert(inslen); /* Dead alley */
	insert[inslen]= Nilitem;
	pclass->c_insert= savearray(insert, inslen + 1);
	if (applen) {
		append[applen]= Nilitem;
		pclass->c_append= savearray(append, applen + 1);
	}
	if (joinlen) {
		join[joinlen]= Nilitem;
		pclass->c_join= savearray(join, joinlen + 1);
	}
}

Visible bool maystart(c, ilex) char c; item ilex; {
	string cp;

	ilex -= nlexical;
	Assert(ilex >= 0);
	if (ilex >= nlex || !isascii(c) || c != ' ' && !isprint(c))
		return No;
	cp= lexdef[ilex].l_start;
	if (*cp == '^')
		return !strchr(cp+1, c);
	return strchr(cp, c) != 0;
}

/*
 * Yield the width of a piece of fixed text, excluding \b or \t.
 * If \n or \r is found, -1 is returned.
 * It assumes that \n or \r only occur as first
 * character, and \b or \t only as last.
 */

Hidden int fwidth(str) string str; {
	register int c;
	register int n = 0;

	if (!str)
		return 0;
	c = str[0];
	if (c == '\r' || c == '\n')
		return -1;
	for (; c; c = *++str)
		++n;
	if (n > 0) {
		c = str[-1];
		if (c == '\t' || c == '\b')
			--n;
	}
	return n;
}
