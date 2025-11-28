/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1988. */

/*
 * fill the read-in tables, checking their consistency.
 *
 * All references are still in terms of indices in namelist[],
 * and must be replaced by indices in classdef[] or symdef[].
 *
 * The lexical names are replaced by the enveloping class or Symbol,
 * if they occur in a Symbol definition, or a class definition, respectively.
 * The enveloping class and Symbol definitions themselves are still
 * nillified at the start of this process, and can be filled only
 * after the other definitions have been handled; otherwise, any filled-in
 * indices would be considered as indices in namelist[], and be subject
 * to the replacement above.
 *
 */

#include "b.h"
#include "main.h"

Hidden int errors;

Visible Procedure fill_and_check_tables() {
	
	errors= 0;
	
	check_defined();
	
	check_representations();
	
	if (errors)
		fatal("giving up");

	fill_classdefinitions();
	
	fill_symboldefinitions();
	
	fill_lexicals();
	
	fill_special_definitions();
}

Hidden Procedure check_defined() {
	struct nameinfo *pn;
	int iname;
	
	for (iname= 0; iname < nname; iname++) {
		pn= &namelist[iname];
		if (Isnilitem(pn->n_index)) {
			message("name '%s' not defined by any rule", 
				pn->n_name);
			errors++;
		}
	}
}

/* Check the fixed-string representations in the grammar.
 * The code assumes that Optional and Hole are the last two in the table
 */

Hidden Procedure check_representations() {
	struct syminfo *psym;
	int isym;
	int ich;
	
	for (isym= 0; isym < noptional; isym++) {
		psym= &symdef[isym];
		for (ich= 0; ich < MAXCHILD; ich++) {
			checkstring(psym->s_repr[ich], ich, psym->s_name);
			if (ich == MAXCHILD || Isnilitem(psym->s_class[ich]))
				break;	/* for ich */
		}
	}
}

/*
 * Check a representation string.
 */

Hidden Procedure checkstring(s, ich, sname) string s; int ich; string sname; {
	int i;
	
	if (s == NULL)
		return;
	for (i = 0; s[i] != '\0'; i++) {
		switch (s[i]) {
		case '\n':
		case '\r':
			if (i || ich) {
				errors++;
		message("badly placed \\n/\\r for symbol %s, child %d",
					sname, ich);
			}
			break;
		case '\t':
		case '\b':
			if (s[i+1]) {
				errors++;
		message("badly placed \\t/\\b for symbol %s, child %d",
					sname, ich);
			}
			break;
		default:
			if (s[i] < ' ' || s[i] >= 0177) {
				errors++;
		message("illegal control char for symbol %s, child %d",
					sname, ich);
			}
		}
	}
}

Hidden Procedure fill_classdefinitions() {
	struct classinfo *pclass;
	int iclass;
	int i;
	int iname;
	struct nameinfo *pname;
	
	for (iclass= 0; iclass < nclass; iclass++) {
		pclass= &classdef[iclass];
		for (i= 0; ; i++) {
			iname= pclass->c_syms[i];
			if (Isnilitem(iname))
				break; /* for i */
			pname= &namelist[iname];
			switch (pname->n_type) {
			case Sym:
				/* replace by index in symdef[] */
				pclass->c_syms[i]= pname->n_index;
				break;
			case Lex:
				/* replace by enveloping Symbol definition */
				pclass->c_syms[i]= lexdef[pname->n_index].l_sym;
				break;
			default:
				message("can't happen");
			}
		}
	}
}

Hidden Procedure fill_symboldefinitions() {
	struct syminfo *psym;
	int isym;
	int ich;
	int iname;
	struct nameinfo *pname;
	
	for (isym= 0; isym < nsym; isym++) {
		psym= &symdef[isym];
		for (ich= 0; ich < MAXCHILD; ich++) {
			iname= psym->s_class[ich];
			if (Isnilitem(iname))
				break;	/* for ich */
			pname= &namelist[iname];
			switch (pname->n_type) {
			case Class:
				/* replace by index in classdef[] */
				psym->s_class[ich]= pname->n_index;
				break;
			case Lex:
				/* replace by enveloping class definition */
				psym->s_class[ich]= 
					lexdef[pname->n_index].l_class;
				break;
			default:
				message("can't happen");
			}
		}
	}
}

Hidden Procedure fill_lexicals() {
	struct lexinfo *plex;
	int ilex;
	struct classinfo *pbody;
	struct syminfo *psym;
	struct classinfo *pclass;
	
	nlexical= nsym;	/* ensure lexicals > Symbols */
	
	/* The enveloping class- and Symbol-definitions have already
	 * been malloc'ed and filled with Nil's in getlexdef().
	 * Here we only fill the real indices.
	 */
	for (ilex= 0; ilex < nlex; ilex++) {
		plex= &lexdef[ilex];
		
		pbody= &classdef[plex->l_body];
		pbody->c_syms[0]= nlexical + ilex;
		
		if (ilex == lsuggestion || ilex == lsugghowname)
			continue; /* see comment in read.c in getlexdef()*/
		
		psym= &symdef[plex->l_sym];
		psym->s_class[0]= plex->l_body;
		
		pclass= &classdef[plex->l_class];
		pclass->c_syms[0]= plex->l_sym;
	}
}

Hidden Procedure fill_special_definitions() {

	if (lsuggestion >= 0 )		/* SUGGESTION defined */
		symdef[nsuggestion].s_class[0]= nsuggstnbody;
	if (lsugghowname >= 0)		/* SUGGHOWNAME defined */
		symdef[nsugghowname].s_class[0]= nsugghowbody;
	
	/* Optional and Hole need no further filling */
}
