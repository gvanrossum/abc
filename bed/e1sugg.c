/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1986. */

/*
 * B editor -- Suggestion handling module.
 */

#include "b.h"

#ifdef USERSUGG

#include "b0lan.h"
#include "bmem.h"
#include "bedi.h"
#include "etex.h"
#include "bobj.h"
#include "bfil.h"
#include "node.h"
#include "supr.h"
#include "gram.h"
#include "tabl.h"
#include "queu.h"
#include "port.h"

extern bool lefttorite;
extern bool incentralws;
extern char *cen_dir;

#define SUGGBUFSIZE 128
#define NSUGGSIZE 64

Hidden value *sugg;
Hidden int *symsugg;
Hidden int nsugg= 0;
Hidden int maxnsugg= 0;
Hidden int nbuiltin= 0;
Hidden int ncentral= 0;
Hidden bool suggchanges= No;

Forward Hidden Procedure getsugg();
Forward Hidden Procedure savsugg();
Forward Hidden int findsugg();
Forward Hidden bool getpattern();
Forward Hidden Procedure addnode();
Forward Hidden Procedure addstr();

/*
 * sugg[0..nbuiltin-1]:
 *   holds the suggestions for builtin commands;
 *   added in order by initclasses();
 * sugg[nbuiltin..ncentral-1]:
 *   holds the suggestions for how-to's in the central workspace;
 *   added once by initcensugg();
 *   not present if we are in the central workspace itself;
 * sugg[ncentral..nsugg-1]:
 *   holds the suggestions for how-to's in the current workspace;
 *   added by initsugg();
 */
 
Visible Procedure initcensugg() {
	char *censuggfile;

	censuggfile= makepath(cen_dir, SUGGFILE);
	getsugg(censuggfile);
	freepath(censuggfile);
	ncentral= nsugg;
}

Visible Procedure
initsugg()
{
	if (incentralws)
		ncentral= nbuiltin;
	else {
		getsugg(suggfile);
		suggchanges= No;
	}
}

/*
 * Read the suggestion table from (central or current workspace) file.
 */

Hidden Procedure getsugg(sgfile) char *sgfile; {
	char *line;
	register FILE *fp;

	fp = fopen(sgfile, "r");
	if (!fp)
		return;
	while ((line= f_getline(fp)) != NULL) {
		addsugg(line, -1);
	}
	fclose(fp);
}

Visible Procedure endsugg() {
	int i;

	savsugg();

	if (incentralws)
		ncentral= nsugg;
	else {
		for (i = ncentral; i < nsugg; ++i)
			release(sugg[i]);
		nsugg= ncentral;
		if (maxnsugg > NSUGGSIZE) {
			regetmem((ptr*)&sugg, (unsigned) (NSUGGSIZE * sizeof(value)));
			maxnsugg= NSUGGSIZE;
		}
	}
}

/*
 * refinements for addsugg():
 */

/*
 * Make sure a line looks like a suggestion, return No if not.
 * Replace the trailing newline or comment-sign by a zero byte.
 * ***** Should check more thoroughly. *****
 */

Hidden bool
checksugg(bp)
	string bp;
{
	if (!isascii(*bp))
		return No;
	if (isupper(*bp)) {
		while (*bp && *bp != '\n' && *bp != '\\') {
			if (maycontinue(*bp, KEYWORD)
			    || strchr(" ?:", *bp) != NULL)
				++bp;
			else
				return No;
		}
		*bp = 0;
	}
	else if (islower(*bp)) {
		while (*bp && *bp != '\n' && *bp != '\\') {
			if (maycontinue(*bp, NAME))
				++bp;
			else
				return No;
		}
		*bp = 0;
	}
	return Yes;
}

/*
 * check that first keyword not forbidden.
 * slight variation on bint2/i2syn.c
 */

Hidden char *firstkw[] = {
	K_IF, K_WHILE, K_CHECK, K_HOW, K_RETURN, K_REPORT,
	NULL
};

Hidden bool res_firstkeyword(str) string str; {
	char *fkw;
	char *fkwend;
	string *kw;
	bool r= No;
	
	fkw= savestr(str);
	if ((fkwend=strchr(fkw, ' ')) != NULL)
		*fkwend= '\0';
	for (kw= firstkw; *kw != NULL; kw++) {
		if (strcmp(fkw, *kw) == 0) {
			r= Yes;
			break;
		}
	}
	freestr(fkw);
	return r;
}

/*
 * Procedure to add a suggestion to the suggestion table.
 * builtin > 0: adding builtin command with this Symbol value;
 *		these are inserted in order at the start of sugg[] table;
 * builtin == 0: adding new user defined command;
 *		 these are inserted after builtin's, and kept sorted;
 * builtin == -1: adding user defined commands from (current or central ws)
 *                suggestion file (already sorted!).
 */

Visible Procedure addsugg(str, builtin) string str; int builtin; {
	int i;
	int j;
	int len;
	int cmp;
	string suggi;
	int where = (builtin == -1) ? nsugg : ncentral;

	if (builtin <= 0 && (!checksugg(str) || res_firstkeyword(str)))
		return;
	for (	len = 0;
		str[len] && (str[len] != ' ' || isupper(str[len+1]));
		++len)
		;
	for (i = nsugg-1; i >= 0; --i) {
		suggi = e_strval(sugg[i]);
		cmp = strncmp(str, suggi, len);
		if (cmp < 0)
			continue;
		if (cmp > 0) {
			if (i >= where)
				where = i+1;
			continue;
		}
		if (suggi[len] 
		    &&
		    (suggi[len] != ' '
		     ||
		     (isascii(suggi[len+1]) && isupper(suggi[len+1]))
		    )
		)
			continue; /* No match, just prefix */
		if (i < nbuiltin)
			return; /* Cannot replace built-in */
		if (i < ncentral)
			continue; /* should add homonymous with central ws */
		if (!strcmp(str+len, suggi+len))
			return; /* Ignore exact duplicates */
		/* Replacement */
		release(sugg[i]);
		sugg[i] = mk_etext(str);
		suggchanges = Yes;
		return;
	}
	/* Insertion */
	if (nsugg == 0) {
		symsugg= (int*) getmem((unsigned) (MAXNBUILTIN * sizeof(int)));
		sugg= (value*) getmem((unsigned) (NSUGGSIZE * sizeof(value)));
		maxnsugg= NSUGGSIZE;
	}
	if (nsugg >= maxnsugg) {
		regetmem((ptr*)&sugg,
			 (unsigned) ((maxnsugg+NSUGGSIZE) * sizeof(value)));
		maxnsugg += NSUGGSIZE;
	}
	if (builtin > 0) {
		symsugg[nbuiltin] = builtin;
		++nbuiltin; ++ncentral;
	}
	for (j = nsugg; j > where; --j)
		sugg[j] = sugg[j-1];
	++nsugg;
	sugg[where] = mk_etext(str);
	suggchanges = Yes;
}


/*
 * Procedure to delete a suggestion from the suggestion table.
 * Must supply the whole string as argument.
 */

Hidden Procedure
delsugg(str)
	string str;
{
	int i;

	for (i = ncentral; i < nsugg; ++i) {
		if (strcmp(str, e_strval(sugg[i])) == 0) {
			release(sugg[i]);
			--nsugg;
			for (; i < nsugg; ++i)
				sugg[i] = sugg[i+1];
			suggchanges = Yes;
			return;
		}
	}
}

#define CANT_FINISH  MESS(7000, "*** can't finish writing suggestion file [%s]")
/*
 * Procedure to save the suggestion file if it has been changed.
 */

Hidden Procedure savsugg() {
	FILE *fp;
	int i;

	if (!suggchanges)
		return;
	suggchanges = No;
	fp = fopen(suggfile, "w");
	if (!fp)
		return;
	for (i = ncentral; i < nsugg; ++i)
		fprintf(fp, "%s\n", e_strval(sugg[i]));
	if (fclose(fp) == EOF) {
		ederrS(CANT_FINISH, suggfile);
		return;
	}
	/* Remove the file if it's empty: */
	if (ncentral >= nsugg) unlink(suggfile);
}

/*
 * Return a suitable suggestion which matches str for len characters,
 * followed by new_c.
 * First we lookup the last suggestion given to start the circular
 * search of the suggestion list from the next entry.
 * Nnil is returned if no entry matches.
 */

static int lastisugg= -1; /* keep track of last suggestion */
                          /* initialised by firstsugg() */

Hidden node nextsugg(str, len, new_c, in_sugghowname, colon_allowed)
string str; int len; int new_c; bool in_sugghowname; bool colon_allowed;
{
	string sg;
	int i;
	int istop;
	string sugg_i;
	
	str[len]= new_c;
	
	i= lastisugg+1;
	if (in_sugghowname && i < ncentral) {
		i= ncentral;
		istop= nsugg;
	}
	else if (i == 0)
		istop= nsugg;
	else
		istop= i;

	do {
		if (i == nsugg) i= (in_sugghowname ? ncentral : 0);
		sugg_i= e_strval(sugg[i]);
		if (strncmp(str, sugg_i, len+1) == 0) {
			if (colon_allowed || strchr(sugg_i, ':') == NULL) {
				lastisugg= i;
				return (node) sugg[i];
			}
		}
		++i;
	} while (i != istop);

	lastisugg= -1;
	return Nnil;
}

/*
 * Place an initial suggestion in a node.
 */

Hidden node firstsugg(s, startsugg, colon_allowed)
string s; int startsugg; bool colon_allowed;
{
	int i;
	string sugg_i;
	
	for (i= startsugg ; i < nsugg; i++) {
		sugg_i= e_strval(sugg[i]);
		if (strncmp(s, sugg_i, strlen(s)) == 0) {
			if (colon_allowed || strchr(sugg_i, ':') == 0) {
				lastisugg= i;
				return (node) sugg[i];
			}
		}
	}
	return Nnil;
}

Visible bool
setsugg(pp, c, ep, colon_allowed)
	path *pp;
	char c;
	environ *ep;
	bool colon_allowed;
{
	char buf[2];
	node n;
	string s;
	string lastunitname();

	if (lefttorite)
		return No;
	n= Nnil;
	if (c == ':') {
		if (nsugg <= ncentral)
			return No;	/* no suggestions for howto name */
		s= lastunitname();
		if (s != NULL) {
			n= firstsugg(s, ncentral, No);
			freestr(s);
		}
		if (n == Nnil) {
			n= (node) mk_etext("");
		}
	}
	else {
		buf[0] = islower(c) ? toupper(c) : c;
		buf[1] = '\0';
		n = firstsugg(buf,
			(parent(*pp) == NilPath ? 0 : 1),
			  /* skip "H?OW TO ?:" if not at root */
			colon_allowed);
	}
	if (n == Nnil) {
		lastisugg= -1;
		return No;
	}
	n= nodecopy(n);
	if (c == ':') {
		n= newnode(1, Sugghowname, &n);
		n= newnode(1, Edit_unit, &n);
	}
	else {
		n= newnode(1, Suggestion, &n);
	}
	treereplace(pp,	n);
	ep->mode = VHOLE;
	ep->s1 = 2;
	ep->s2 = (c == ':' ? 0 : 1);
	return Yes;
}

Hidden bool fits_how_to(str, pstr, alt_c)
string str; string *pstr; int alt_c;
{
	if (strcmp(str, S_HOW_TO) == 0) {
		if (alt_c)
			**pstr= alt_c;
		return Yes;
	}
	return No;
}

/*
 * Find a new suggestion or advance in the current one.
 * Interface styled like resuggest: string pointer is advanced here.
 */

Visible bool newsugg(ep, pstr, alt_c) environ *ep; string *pstr; int alt_c; {
	string str;
	node n = tree(ep->focus);
	node nn;
	int sym = symbol(n);
	path pa= parent(ep->focus);
	int sympa= pa ? symbol(tree(pa)) : Rootsymbol;

	Assert(pstr && *pstr);
	if (sym != Suggestion || ep->mode != VHOLE || ep->s1 != 2)
		return No;

	str= e_sstrval((value) firstchild(n));
	
	if (str[ep->s2-1] == ' ' 
	    && (!isupper(**pstr) || res_firstkeyword(str)))
	{
		/* require CAPITAL after space */
		/* uses that Keywords start with such */
		if (str[ep->s2] == '?'  /* pattern fits expr or loc */
		    || fits_how_to(str, pstr, alt_c))
		{
			/* so acknowledge: */
			acksugg(ep);	/* if insertion fails rest of editor */
			e_fstrval(str);	/* restores suggestion state */
			return Yes;
		}
		/* else: pattern wrong */
		e_fstrval(str);
		return No;	/* implies killsugg in caller */
	}
	
	nn= nextsugg(str, ep->s2, (!alt_c ? (int)(**pstr) : alt_c), No,
		allows_colon(sympa));
	
	e_fstrval(str);
	
	if (!nn)
		return No;
	
	if (nn != firstchild(n)) {
		s_down(ep);
		treereplace(&ep->focus, nodecopy(nn));
		s_up(ep);
	}

	++ep->s2;
	if (**pstr == ':') {
		/* must be "SELECT:" */
		acksugg(ep);
		Assert(symbol(tree(ep->focus)) == Select);
	}
	++*pstr;

	return Yes;
}


/*
 * Kill suggestion -- only the part to the left of the focus is kept.
 */

Visible Procedure killsugg(ep, pstr) environ *ep; string *pstr;
{
	node n = tree(ep->focus);
	node nc;
	value vstr;
	
	Assert(ep->mode == VHOLE && ep->s1 == 2 && symbol(n) == Suggestion);
	Assert(ep->s2 <= Length((value)firstchild(n)));
	
	nc = (node)e_icurtail((value)firstchild(n), ep->s2);
	if (e_ncharval(ep->s2, (value)firstchild(n)) == ' ' 
	    && pstr != (string*)NULL) {
	    	/* fix for e.g. APPEND WORD >?<TO ?, inserting X */
	    	/* acksugg threw the space after WORD away */
		e_concto((value*) &nc, vstr=mk_etext(*pstr));
		ep->s2 += e_length(vstr);
		release(vstr);
		**pstr= '\0';
	};
	s_down(ep);
	treereplace(&ep->focus, nc);
	s_up(ep);
	acksugg(ep);
}

/*
 * Acknowledge a suggestion -- turn it into real nodes.
 */

Visible Procedure acksugg(ep) environ *ep; {
	node n = tree(ep->focus);
	int s2 = ep->s2;
	int isugg;
	string str;
	node nn;
	node n1;
	string rest;
	queue q = Qnil;
	node r;

	Assert(symbol(n) == Suggestion && ep->mode == VHOLE && ep->s1 == 2);

	str= e_sstrval((value) firstchild(n));
	isugg= lastisugg;
	
	if (0 <= isugg && isugg < nbuiltin) {	/* builtin command */
		nn= gram(symsugg[isugg]);
		treereplace(&ep->focus, nodecopy(nn));
		ep->mode= FHOLE;
		ep->s1= 1;
		/* s2 in or at end of repr[0] */
		Assert(s2 <= Fwidth(table[symsugg[isugg]].r_repr[0]));
	}
	else if ((rest= strchr(str, ' ')) == NULL) { /* just one keyword */
		nn= gram(Keyword);
		setchild(&nn, 1, (node) mk_etext(str));
		treereplace(&ep->focus, nn);
		/* mode VHOLE and s1, s2 allright */
	}
	else {			/* Keyword plus ... */
		/* split off first keyword */
		*rest++ = '\0';
		n1= gram(Keyword);
		setchild(&n1, 1, (node) mk_etext(str));
		
		/* hang in Kw_plus */
		nn= gram(Kw_plus);
		setchild(&nn, 1, n1);
		
		/* set focus at hole after space after first keyword */
		treereplace(&ep->focus, nn);
		/* ep->mode= VHOLE; */
		ep->s1= 4;
		ep->s2= 0;
		
		/* rest of suggestion to q */
		r= (node) mk_etext(rest);
		preptoqueue(r, &q);
		noderelease(r);
		
		/* append to first keyword and restore focus position */
		app_queue(ep, &q);
		fixfocus(ep, s2);
	}
	
	e_fstrval(str);
}

/*
 * newsugg, adv_howsugg and acksugg for Sugghowname.
 * Note that a howsugg is never kiled, just advanced in;
 * ackhowsugg is only used for [newline] and [accept].
 */

Forward Hidden node adv_howsugg();

Visible bool newhowsugg(ep, pstr, alt_c) environ *ep; string *pstr; int alt_c; {
	string str;
	string qm;
	node n = tree(ep->focus);
	int sym = symbol(n);
	node nn;
	int newc;

	Assert(pstr && *pstr);
	if (sym != Sugghowname || ep->mode != VHOLE || ep->s1 != 2)
		return No;

	str= e_sstrval((value) firstchild(n));
	
	if (isupper(str[0])) {
		/* ucmd suggestion */
		qm= strchr(str, '?');
		if (qm && qm-str < ep->s2) {
			e_fstrval(str);
			return No; /* refuse insert after questionmark */
		}
	}
	else if (islower(str[0])) {
		/* fpr suggestion */
		if (**pstr == ' ') {
			e_fstrval(str);
			return No; /* refuse space in name */
		}
	}
	
	if (ep->s2 == 0 || !alt_c || !isupper(str[0]))
		newc= (int)(**pstr);
	else
		newc= alt_c;
	
	nn = nextsugg(str, ep->s2, newc, Yes, No);
	
	e_fstrval(str);
	
	if (!nn) {
	 	if (ep->s2 == 0) {	/* nothing suggested */
			treereplace(&ep->focus, gram(Optional));
			ep->mode= ATBEGIN;
			return Yes;	/* ins_string will continue */
		}
		nn= adv_howsugg(ep, str[ep->s2-1], (char)newc);
		if (!nn)
			return No;
	}
	
	if (nn != firstchild(n)) {
		s_down(ep);
		treereplace(&ep->focus, nodecopy(nn));
		s_up(ep);
	}

	++ep->s2;
	++*pstr;

	return Yes;
}

Hidden node adv_howsugg(ep, prev_c, new_c)
environ *ep; char prev_c; char new_c;
{
	int s2= ep->s2;
	char buf[2];
	value hd;
	value tl;
	
	Assert(ep->s2 <= Length((value)firstchild(tree(ep->focus))));
	
	if (isalpha(new_c) ||
	    (prev_c != ' ' && strchr("012345679'\". ", new_c) != NULL))
	{
		buf[0]= new_c;
		buf[1]= '\0';
	
		hd= e_icurtail((value) firstchild(tree(ep->focus)), s2);
		tl= mk_etext(buf);
		e_concto(&hd, tl);
		release(tl);
	
		return (node) hd;
	}
	/* else */
	return Nnil;
}

/*
 * Acknowledge a how-to name suggestion -- but do NOT turn it into real nodes.
 */

Visible Procedure ackhowsugg(ep) environ *ep; {
	ep->mode= VHOLE;
	ep->s1= 2;
	ep->s2= strlen(e_strval((value) firstchild(tree(ep->focus))));
}

/*
 * Kill a how-to name suggestion -- but do NOT turn it into real nodes.
 */

Visible Procedure killhowsugg(ep) environ *ep; {
	int s2= ep->s2;
	value hd;
	
	if (s2 == Length((value)firstchild(tree(ep->focus))))
		return;
	hd= e_icurtail((value) firstchild(tree(ep->focus)), s2);
	s_down(ep);
	treereplace(&ep->focus, nodecopy(hd));
	s_up(ep);
}

/*
 * Leave a single ':' (edit last unit) as immediate command if
 * the user did not try to edit the suggested last_unit name;
 * this is done to avoid the which_funpred dialog in the interpreter.
 */

Visible Procedure check_last_unit(ep, curr) environ *ep; int curr; {
	if (curr != 1 
	    || symbol(tree(ep->focus)) != Edit_unit
	    || symbol(firstchild(tree(ep->focus))) != Sugghowname)
	    	return;
	/* else */
	s_down(ep);
	treereplace(&ep->focus, gram(Optional));
	s_up(ep);
}


/*
 * Procedure called when a unit is read in.
 * It tries to update the suggestion database.
 * It also remembers the suggestion so that it can be removed by writesugg
 * if that finds the unit was deleted or renamed.
 */

Hidden char *lastsugg= NULL;	/* the buffer */
Hidden char *pbuf;
Hidden int buflen= 0;

Visible Procedure
readsugg(p)
	path p;
{
	p = pathcopy(p);
	top(&p);
	if (getpattern(tree(p)))
		addsugg(lastsugg, 0);
	else {
		freemem((ptr) lastsugg);
		lastsugg= NULL;
	}
	pathrelease(p);
}


/*
 * Procedure called when a unit is saved.
 * It tries to update the suggestion database.
 * Since renaming a unit now deletes to old name, we always delete
 * 'lastsugg'; we add it again if the unit is not empty.
 */

Visible Procedure
writesugg(p)
	path p;
{
	p = pathcopy(p);
	top(&p);
	if (lastsugg != NULL)
		delsugg(lastsugg);
	if (nodewidth(tree(p)) != 0) {
		if (getpattern(tree(p)))
			addsugg(lastsugg, 0);
	}
	if (lastsugg != NULL) {
		freemem((ptr) lastsugg);
		lastsugg= NULL;
	}
	pathrelease(p);
}


/*
 * Procedure to find out the suggestion that fits the current unit.
 * For user defined commands it just replaces stretches of non-keywords
 * with a single '?'; for functions and predicates it tries to
 * get the name.
 * It uses intimate knowledge about the abc grammar for formal-cmd and
 * formal_formula in ../boot/grammar.abc (such as the separation of 
 * keywords and other stuff by spaces).
 */

Hidden bool
getpattern(n)
	node n;
{
	string *rp = noderepr(n);
	int sym;
	int sym1;
	int sym2;
	
	if (lastsugg == NULL) {
		lastsugg= (char*) getmem(SUGGBUFSIZE);
		buflen= SUGGBUFSIZE;
	}
	pbuf= lastsugg;

	while (Fw_zero(rp[0])) {
		if (nchildren(n) == 0)
			return No;
		n = firstchild(n);
		rp = noderepr(n);
	}
	if (strcmp(rp[0], R_HOW_TO) || nchildren(n) < 1)
		return No;
	n= firstchild(n);
	sym= symbol(n);
	if (sym == Formal_kw_plus || sym == Keyword) {
		for (;;) {
			switch (sym) {
			case Formal_kw_plus:
				addnode(firstchild(n));
				addstr(" ");
				break;
			case Keyword:
				addnode(n);
				*pbuf= '\0';
				return Yes;
			case Formal_naming_plus:
				addstr("? ");
				break;
			case Name:
			case Multiple_naming:
			case Compound_naming:
				addstr("?");
				*pbuf= '\0';
				return Yes;
			default:
				Assert(No);
				return No;
			}
			n= child(n, 2);
			sym= symbol(n);
		}
		/* NOTREACHED */
	}
	else {
		Assert(sym==Formal_return || sym== Formal_report);
		n= firstchild(n);
		sym= symbol(n);
		if (sym == Blocked_ff || sym == Grouped_ff) {
			sym1= symbol(child(n, 1));
			sym2= symbol(child(n, 2));
			if (sym2 == Name || sym2 == Compound_naming) {
				n= child(n, 1);
				if (sym1 == Blocked_ff)
					n= child(n, 2);
			}
			else if (sym1 == Name || sym1 == Compound_naming) {
				/* sym2 == Blocked_ff || Grouped_ff */
				n= child(n, 2);
				n= firstchild(n);
			}
		}
		if (symbol(n) == Name) {
			addnode(n);
			*pbuf= '\0';
			return Yes;
		}
		/* else */
		return No;
	}
	/*NOTREACHED*/
}

Hidden Procedure addnode(n) node n; {
	string s;
	
	Assert(symbol(n) == Keyword || symbol(n) == Name);
	
	s= e_strval((value) firstchild(n));
	addstr(s);
}

Hidden Procedure addstr(s) string s; {
	while (*s) {
		*pbuf++ = *s++;
		if (pbuf >= lastsugg + buflen) {
			regetmem((ptr*)&lastsugg, 
				 (unsigned) (buflen+SUGGBUFSIZE));
			pbuf= lastsugg+buflen;
			buflen += SUGGBUFSIZE;
		}
	}
}

Visible Procedure
endclasses()
{
#ifdef MEMTRACE
	int i;
	for (i= 0; i < nbuiltin; ++i)
		release(sugg[i]);
	nbuiltin= nsugg= 0;
	freemem((ptr) sugg);
#endif
}

#endif /* USERSUGG */
