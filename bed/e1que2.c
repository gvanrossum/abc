/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1986. */

/*
 * B editor -- Manipulate queues of nodes, higher levels.
 */

#include "b.h"
#include "bedi.h"
#include "etex.h"
#include "bobj.h"
#include "node.h"
#include "supr.h"
#include "queu.h"
#include "gram.h"
#include "tabl.h"
#include "code.h"

extern bool lefttorite;
	/* Set by edit() to signal we parse purely left-to-right */

Forward Hidden bool ins_node();
Forward Hidden bool joinnodes();
Forward Hidden bool add_string();
Forward Hidden bool canfitchar();

/*
 * Insert a queue of nodes at the focus
 * (which had better be some kind of a hole).
 * The nodes may also be a text, in which case the individual characters
 * are inserted.
 * Extensive changes to the parse tree may occur, and the node may be
 * broken up in its constituent parts (texts and other nodes) which
 * are then inserted individually.
 */

Visible bool
ins_queue(ep, pq, pq2)
	register environ *ep;
	register queue *pq;
	register queue *pq2;
{
	register bool ok = Yes;
	register node n;
	register queue oldq2;
	environ saveenv;
	int oldindentation = focindent(ep);
	int indentation = oldindentation;
	string str;

	leftvhole(ep);
	while (ok && !emptyqueue(*pq)) {
		n = queuebehead(pq);
		if (Is_etext(n)) {
			str= e_sstrval((value) n);
			ok = ins_string(ep, str, pq2, 0);
			e_fstrval(str);
			
			switch (e_ncharval(e_length((value) n), (value) n)) {
				/* Last char */
			case '\t':
				++indentation;
				break;
			case '\b':
				--indentation;
				break;
			case '\n':
				while (focindent(ep) > indentation) {
					if (!ins_newline(ep, No))
						break;
				}
				break;
			}
		}
		else {
			Ecopy(*ep, saveenv);
			oldq2 = qcopy(*pq2);
			if (!ins_node(&saveenv, n, pq2)) {
				Erelease(saveenv);
				qrelease(*pq2);
				*pq2 = oldq2;
				if (symbol(n) == Hole)
					ok = ins_string(ep, "?", pq2, 0);
				else
					splitnode(n, pq);
			}
			else {
				Erelease(*ep);
				Emove(saveenv, *ep);
				qrelease(oldq2);
			}
		}
		noderelease(n);
	}
#ifndef NDEBUG
	if (!ok)
		qshow(*pq, "ins_queue");
#endif
	qrelease(*pq);
	for (indentation = focindent(ep);
		indentation > oldindentation; --indentation)
		stringtoqueue("\b", pq2); /* Pass on indentation to outer level */
	return ok;
}


/*
 * Subroutine to insert a queue to the right of the focus
 * without affecting the focus position.
 */

Visible bool
app_queue(ep, pq)
	environ *ep;
	queue *pq;
{
	int where;
	static int markbit = 1; /* To properly handle recursive calls */

	if (emptyqueue(*pq))
		return Yes;
	where = focoffset(ep);
	markbit <<= 1;
	markpath(&ep->focus, markbit);
	if (!ins_queue(ep, pq, pq)) {
		markbit >>= 1;
		return No;
	}
	if (!firstmarked(&ep->focus, markbit)) Abort();
	unmkpath(&ep->focus, markbit);
	markbit >>= 1;
	ep->spflag = No;
	fixfocus(ep, where);
	return Yes;
}


/*
 * Advance to next thing after current position.
 */

Visible bool
move_on(ep)
	register environ *ep;
{
	register node n;
	register string *rp;
	register int sym;
	register int ich = ichild(ep->focus);

	if (!up(&ep->focus))
		return No;
	higher(ep);
	n = tree(ep->focus);
	rp = noderepr(n);
	if (Fw_positive(rp[ich])) {
		ep->mode = FHOLE;
		ep->s1 = 2*ich + 1;
		ep->s2 = 0;
		if (ep->spflag) {
			ep->spflag = No;
			if (rp[ich][0] == ' ') {
				++ep->s2;
				if (fwidth(rp[ich]) > 1)
					return Yes;
			}
			else
				return Yes;
		}
		else
			return Yes;
	}
	if (ich < nchildren(n)) {
		s_downi(ep, ich+1);
		sym = symbol(tree(ep->focus));
		if (sym == Hole || sym == Optional)
			ep->mode = WHOLE;
		else
			ep->mode = ATBEGIN;
		return Yes;
	}
	ep->mode = ATEND;
	return Yes;
}


/*
 * Like move_on but moves through fixed texts, skipping only spaces
 * and empty strings.
 * <<<<< This code is a dinosaur and should be revised. >>>>>
 */

Visible bool
fix_move(ep)
	register environ *ep;
{
	register int ich;
	register int i;
	register string *rp;
	register string cp;

	Assert(ep->mode == FHOLE);

	ich = ep->s1/2;
	rp = noderepr(tree(ep->focus));
	cp = rp[ich];
	if (cp) {
		i = ep->s2;
		Assert(i <= Fwidth(cp));
		if (cp[i] == ' ') {
			do {
				++i;
			} while (cp[i] == ' ');
		}
		if (cp[i] == '\b' || cp[i] == '\t') {
			++i;
			Assert(!cp[i]);
		}
		else if (cp[i]) {
			if (i == ep->s2)
				return No;
			ep->s2 = i;
			return Yes;
		}
	}

	if (ich >= nchildren(tree(ep->focus)))
		ep->mode = ATEND;
	else {
		s_downi(ep, ich+1);
		if (symbol(tree(ep->focus)) == Hole
			|| symbol(tree(ep->focus)) == Optional)
			ep->mode = WHOLE;
		else
			ep->mode = ATBEGIN;
	}
	return Yes;
}


/*
 * Insert a node in the parse tree.
 */

Hidden bool
ins_node(ep, n, pq)
	register environ *ep;
	register node n;
	register queue *pq;
{
	register int sym;
	register node nn;
	register markbits x;
	string *rp;
	node fc;

	if (symbol(n) == Optional)
		return Yes;

	for (;;) {
		switch (ep->mode) {

		case FHOLE:
			if (ep->s2 < lenitem(ep) || !fix_move(ep))
				return No;
			continue;

		case VHOLE:
			if (ep->s2 == lenitem(ep)
			    && symbol(tree(ep->focus)) == Name
			    && (symbol(n) == Text1_display
			        || symbol(n) == Text2_display))
			{
				/* enable insertion of name before */
				/* text display with conversion */
				ep->mode= ATEND;
				continue;
			}
			if (ep->s2 < lenitem(ep) || !move_on(ep))
				return No;
			continue;

		case ATBEGIN:
			sym = symbol(tree(ep->focus));
			if (sym == Optional || sym == Hole) {
				ep->mode = WHOLE;
				continue;
			}
			x = marks(tree(ep->focus));
			if (joinnodes(&ep->focus, n, tree(ep->focus), No)) {
				if (x) {
					s_downi(ep, 2);
					markpath(&ep->focus, x);
					s_up(ep);
				}
				s_down(ep);
				ep->mode = ATEND;
				leftvhole(ep);
				return Yes;
			}
			nn = tree(ep->focus);
			rp = noderepr(nn);
			if (nchildren(nn) >= 1 && Fw_zero(rp[0])) {
				fc= firstchild(nn);
				sym = (Is_etext(fc) ? (-1) : symbol(fc));
				if (sym == Hole || sym == Optional) {
					s_down(ep);
					if (fitnode(&ep->focus, n)) {
						ep->mode = ATEND;
						leftvhole(ep);
						return Yes;
					}
					s_up(ep);
				}
			}
			nn = nodecopy(nn);
			if (!fitnode(&ep->focus, n)) {
				addtoqueue(pq, nn);
				noderelease(nn);
				delfocus(&ep->focus);
				ep->mode = WHOLE;
				continue;
			}
			if (downrite(&ep->focus)) {
				if (!Is_etext(tree(ep->focus))) {
					sym = symbol(tree(ep->focus));
					if (sym == Hole || sym == Optional) {
						if (fitnode(&ep->focus, nn)) {
							noderelease(nn);
							nn = Nnil;
						}
					}
				}
				else
					VOID up(&ep->focus);
			}
			if (nn) {
				addtoqueue(pq, nn);
				noderelease(nn);
			}
			ep->mode = ATEND;
			leftvhole(ep);
			return Yes;

		case WHOLE:
			sym = symbol(tree(ep->focus));
			Assert(sym == Optional || sym == Hole);
			do {
				higher(ep); /* Only for second time around */
				if (fitnode(&ep->focus, n)) {
					ep->mode = ATEND;
					leftvhole(ep);
					return Yes;
				}
			} while (resttoqueue(&ep->focus, pq));
			ep->mode = ATEND;
			/* Fall through */
		case ATEND:
			do {
				higher(ep); /* Only for second time around */
				if (joinnodes(&ep->focus, tree(ep->focus), n, ep->spflag)) {
					ep->spflag = No;
					leftvhole(ep);
					return Yes;
				}
			} while (resttoqueue(&ep->focus, pq)
				|| move_on(ep) && ep->mode == ATEND);
			return No;

		default:
			return No;

		}
	}
}

/* Hacked refinements for ins_string below, mainly to fix problems
 * with suggestions and suggestion-rests for commands; timo
 */

Hidden bool softening_builtin(inch, sym) char inch; int sym; {
	/* refinement for ins_string to enable softening of
	 * builtin commands, e.g REMOVE ? FROM ? -> REMOVE M?
	 */
	return (bool) (isupper(inch) && Put <= sym && sym < Check);
}

Hidden bool fits_kwchar(n, i, ch) node n; int i; int ch; {
	/* REPORT i'th char of Keyword(n) == ch */
	string s;
	int si;
	
	Assert(symbol(n) == Keyword && i >= 0);
	s= e_strval((value) firstchild(n));
	if (strlen(s) < i)
		return No;
	/* else: */
	si= s[i];
	return (bool) si == ch;
}

Hidden bool fits_nextkwstart(n, ch) node n; int ch; {
	register int sym= symbol(n);
	
	if (sym == Keyword)
		return fits_kwchar(n, 0, ch);
	else if (sym == Kw_plus)
		return fits_nextkwstart(child(n, 1), ch);
	/* else: */
	return No; /* can't accept Hole '?' as fit of Capital ch */
}

/* Return whether rest of user-defined-command stems from suggestion.
 * There is a problem with tailing Keywords, since we don't know whether
 * they stem from suggestion;
 * if they do, the user can accept them explicitly,
 * or by blind-typing, which has been handled by now;
 * if they don't, we should not kill them now!
 */

Hidden bool is_varsuggrest(n, exphole_seen) node n; bool exphole_seen; {
	register int sym= symbol(n);
	register node n2;
	register int sym2;
	
	if (sym == Kw_plus) {
		n2= child(n, 2);
		sym2= symbol(n2);
		if (sym2 == Hole)
			return Yes;
		else if (sym2 == Kw_plus || sym2 == Exp_plus || sym2 == Keyword)
			return (is_varsuggrest(n2, exphole_seen));
		/* else: FAIL */
	}
	else if (sym == Exp_plus) {
		if (symbol(child(n, 1)) != Hole)
			return No;
		/* else: */
		n2= child(n, 2);
		sym2= symbol(n2);
		if (sym2 == Hole)
			/* Hole is safety-net; cannot happen? */
			return Yes;
		else if (sym2 == Kw_plus || sym2 == Exp_plus || sym2 == Keyword)
			return is_varsuggrest(n2, Yes);
	}
	else if (sym == Keyword && exphole_seen)
		return Yes;
	/* else: */
	return No;
}

/* Focus at end of expr in Exp_plus, or after space in Kw_plus;
 * acknowledge **pstr (== Capital) iff it fits start of Keyword
 * following focus;
 * else, if rest resembles suggestion-rest, kill that.
 */
Hidden bool ack_or_kill_varsuggrest(ep, pstr) environ *ep; string *pstr; {
	node nn= tree(ep->focus);
	
	if (fits_nextkwstart(child(nn, 2), (int)**pstr)) {
		/* accept start_char in Keyword */
		s_downi(ep, 2);
		Assert(symbol(tree(ep->focus)) != Exp_plus);
		if (symbol(tree(ep->focus)) == Kw_plus)
			s_downi(ep, 1);
		Assert(symbol(tree(ep->focus)) == Keyword);
		ep->s1= 2;
		ep->s2= 1;
		ep->mode= VHOLE;
		ep->spflag= No;
		++*pstr;
		return Yes;
	}
	else if (is_varsuggrest(child(nn, 2), No)) {
		/* kill non-matching suggestion-rest */
		s_downi(ep, 2);
		treereplace(&ep->focus, gram(Hole));
		ep->mode= WHOLE;
		ep->spflag= No;
		return Yes;
	}
	return No;
}

/*
 * Another hack to turn {a.?} (where a. is seen as a name)
 * upon receit of a second "." into {a..?}, where a is the Name
 * and .. is seen as a Number (to enable suggestion '?' for subsequent
 * userinput ')
 */

Hidden bool range_hack(ep) register environ *ep; {
	path pa;
	int sympa;
	string str;
	node n1;
	node n2;
	node nn;
	int s2= ep->s2;
	bool r= No;
	
	if (s2 <= 1) return No;
	
	pa= parent(ep->focus);
	sympa= pa ? symbol(tree(pa)) : Rootsymbol;

	if (sympa == List_or_table_display || sympa == List_filler_series) {
		str= e_sstrval((value) firstchild(tree(ep->focus)));
		if (s2 == strlen(str) && str[s2-1] == '.') {
			str[s2-1]= '\0';
			n1= gram(Name);
			setchild(&n1, 1, (node) mk_etext(str));
			n2= gram(Number);
			setchild(&n2, 1, (node) mk_etext(".."));
			nn= gram(Blocked);
			setchild(&nn, 1, n1);
			setchild(&nn, 2, n2);
			treereplace(&ep->focus, nn);
			s_downi(ep, 2);
			ep->mode= ATEND;
			r= Yes;
		}
		e_fstrval(str);
	}
	return r;
}

/*
 * Insert a string in the parse tree.
 *
 * justgoon is Yes if the last key seen was a lower case
 * letter that was inserted as such.  In this case, some
 * code can be skipped, in particular in ins2.c (which
 * amounts to a so called horrible hack).
 */

Visible bool justgoon = No;

#define NEXT_CH (++str, alt_c = 0)

Visible bool
ins_string(ep, str, pq, alt_c)
	register environ *ep;
	/*auto*/ string str;
	register queue *pq;
	int alt_c;
{
	register node nn;
	auto value v;
	char buf[1024];
	register string repr;
	string oldstr;
	register int sym;
	register int len;
	bool inter_active = alt_c != 0;
	path pa;

	if (alt_c < 0)
		alt_c = 0;
	while (*str) {
		switch (*str) {

		case '\n':
			if (!ins_newline(ep, No))
				return No;
			/* Fall through */
		case '\t':
		case '\b':
			NEXT_CH;
			continue;

		}
		switch (ep->mode) {

		case ATBEGIN:
			nn = tree(ep->focus);
			if (Is_etext(nn)) {
				ep->s1 = 2*ichild(ep->focus);
				ep->s2 = 0;
				ep->mode = VHOLE;
				s_up(ep);
				continue;
			}
			sym = symbol(nn);
			if (sym != Optional && sym != Hole) {
				if (fwidth(noderepr(nn)[0]) == 0) {
					if (down(&ep->focus))
						break;
				}
				addtoqueue(pq, nn);
				delfocus(&ep->focus);
			}
			ep->mode = WHOLE;
			/* Fall through */
		case WHOLE:
			nn = tree(ep->focus);
			sym = symbol(nn);
			Assert(sym == Hole || sym == Optional);
			while ((len = fitstring(&ep->focus, str, alt_c)) == 0) {
				if (sym == Optional) {
					if (!move_on(ep)) {
						if (*str == ' ')
							NEXT_CH;
						else
							return No;
					}
					break;
				}
				if (!inter_active && *str == '?') {
					NEXT_CH;
					ep->mode = ATEND;
					break;
				}
				if (resttoqueue(&ep->focus, pq))
					higher(ep);
				else if (spacefix(ep))
					break;
				else if (*str == ' ') {
					NEXT_CH;
					break;
				}
				else if (inter_active)
					return No;
				else {
					ep->mode = ATEND;
					break;
				}
			}
			if (len > 0) {
				str += len;
				alt_c = 0;
				fixfocus(ep, len);
			}
			break;

		case ATEND:
			if ((pa=parent(ep->focus)) &&
			    symbol(tree(pa)) == Exp_plus &&
			    ichild(ep->focus) == 1 &&
			    isupper(*str))
			{	/* at end of expr in Exp_plus */
				s_up(ep);
				if (ack_or_kill_varsuggrest(ep, &str))
					break;
				/* else: undo up */
				s_downi(ep, 1);
			}
			if (add_string(ep, &str)) {
				alt_c = 0;
				break;
			}
			
			len = joinstring(&ep->focus, str, ep->spflag,
				alt_c ? alt_c : inter_active ? -1 : 0, Yes);
			if (len > 0) {
				s_downi(ep, 2);
				ep->spflag = No;
				fixfocus(ep, len);
			}
			else {
				if (resttoqueue(&ep->focus, pq)) {
					higher(ep);
					break;
				}
				if (move_on(ep))
					break;
				if (*str == ' ') {
					NEXT_CH;
					break;
				}
				return No;
			}
			str += len;
			alt_c = 0;
			break;

		case FHOLE:
			nn = tree(ep->focus);
			sym = symbol(nn);
			if (sym == Edit_unit && ep->s1 == 1 && ep->s2 == 1
			    && symbol(child(nn, 1)) == Sugghowname)
			{
				s_downi(ep, 1);
				ep->mode= VHOLE;
				ep->s1= 2; ep->s2= 0;
				break;
			}
			if (sym == Formal_kw_plus 
			    && ep->s1 == 3 && ep->s2 == 1 && alt_c)
			{
				/* force Formal_naming_plus */
				alt_c= 0;
				/* and go_on */
			}
			if ((sym == Kw_plus || sym == Exp_plus)
			    && ep->s1 == 3 && ep->s2 == 1) {
				/* after space before Keyword */
				if (isupper(*str)) {
					if (ack_or_kill_varsuggrest(ep, &str))
						break;
				}
				else if (sym == Exp_plus) {
					/* this wasn't handled properly */
					/* e.g. ADD a >?<TO ?, 
					 *	insert +,
					 *	ADD a>?< + TO ? */
					s_downi(ep, 1);
					ep->mode= ATEND;
					ep->spflag= Yes;
					break;
				}
				else if (sym == Kw_plus && alt_c)
				        	/* force Exp_plus */
				        	alt_c = 0;
				        	/* and go on: */
			}
			repr = noderepr(nn)[ep->s1/2];
			if (ep->s2 >= fwidth(repr)
			    &&
			    (ep->s2 <= 0 || !isalpha(repr[0]) ||
			     ((ep->spflag || repr[ep->s2-1] == ' ')
			      && !softening_builtin(*str, sym)
			   )))
			{	/* At end */
				if (ep->s1/2 < nchildren(nn)) {
					s_downi(ep, ep->s1/2 + 1);
					ep->mode = ATBEGIN; /* Of next child */
				}
				else
					ep->mode = ATEND;
				break;
			}
			if ((*str == ':' || *str == ' ') && *str == repr[ep->s2]) {
				/*****
				 * Quick hack for insertion of test-suites and refinements:
				 *****/
				++ep->s2;
				NEXT_CH;
				continue;
			}
			if (!lefttorite)
				nosuggtoqueue(ep, pq);
			oldstr = str;
			if (resuggest(ep, &str, alt_c) || soften(ep, &str, alt_c)) {
				if (str > oldstr)
					alt_c = 0;
				continue;
			}
			if (fix_move(ep))
				continue;
			return No;

		case VHOLE:
			Assert(!(ep->s1&1));
			nn = tree(ep->focus);
			sym = symbol(nn);
#ifdef USERSUGG
			if (sym == Suggestion) {
				if (*str == '?')
					return No;
				if (newsugg(ep, &str, alt_c))
					alt_c = 0;
				else {
					killsugg(ep, &str);
				}
				continue;
			}
			else if (sym == Sugghowname) {
				if (*str == '?')
					return No;
				if (!newhowsugg(ep, &str, alt_c))
					return No;
				/* else */
				continue;
			}
#endif /* USERSUGG */
			if (sym == Keyword && 
			    (fits_kwchar(nn, ep->s2, (int)*str)
			     ||
			     (ep->s2 > 0 && alt_c > 0 &&
			      fits_kwchar(nn, ep->s2, alt_c)
			   )))
			{
				/* accept next char in Keyword */
				/* required for blind typist rule; timo */
				/* also enables lowercase within KW */
				ep->s2++;
				NEXT_CH;
				break;
			}
			if (sym == Name && *str == '.' && range_hack(ep)) {
				++str;
				break;
			}
			s_downi(ep, ep->s1/2);
			v = copy((value) tree(ep->focus));
			len = 0;
			if (!ep->spflag) {
				while (len < sizeof buf - 1 && str[len]
		&& mayinsert(nn, ep->s1/2, !!(ep->s2 + len), str[len]))
				{
					if ((sym == Name || sym == Keyword)
					    && str[len] == '.' && str[len+1] == '.')
						break; /* no range in Name or Keyword */
					buf[len] = str[len];
					++len;
				}
				justgoon = len > 0 && islower(str[len-1]);
				if (len <= 0 && alt_c
					&& mayinsert(nn, ep->s1/2, !!(ep->s2 + len), alt_c)) {
					buf[0] = alt_c;
					len = 1;
				}
			}
			if (len > 0) { /* Effectuate change */
				str += len;
				alt_c = 0;
				Assert(Is_etext(v));
				buf[len] = 0;
				putintrim(&v, ep->s2, e_length(v) - ep->s2, buf);
				treereplace(&ep->focus, (node) v);
				s_up(ep);
				ep->spflag = No;
				ep->s2 += len;
			}
			else { /* Nothing inserted */
				if (ep->s2 == 0) { /* Whole string rejected */
					addtoqueue(pq, (node)v);
					release(v);
					s_up(ep);
					delfocus(&ep->focus);
					ep->mode = WHOLE;
					break;
				}
				if (ep->s2 < e_length(v)) {
					value w;
/*					addstringtoqueue(pq, e_strval(v) + ep->s2); */
					w= e_ibehead(v, ep->s2 + 1);
					addtoqueue(pq, (node) w);
					release(w);
					/* putintrim(&v, ep->s2, 0, ""); */
					v= e_icurtail(w= v, ep->s2);
					release(w);
					treereplace(&ep->focus, (node) v);
				}
				else
					release(v);
				if (!move_on(ep)) Abort(); /* ==> up, cancelling s_downi! */
			}
			break;

		default:
			Abort();

		} /* end switch (ep->mode) */
	} /* end while (*str) */

	return Yes;
}


/*
 * See if two nodes can be joined in a hole.
 * 'Spflag' indicates whether a space must be present between the nodes
 * (required or forbidden).
 * Either of n1, n2 may actually be the current contents of the hole.
 */

Hidden bool
joinnodes(pp, n1, n2, spflag)
	path *pp;
	node n1;
	node n2;
	bool spflag;
{
	path pa = parent(*pp);
	int sympa = pa ? symbol(tree(pa)) : Rootsymbol;
	struct table *tp = &table[sympa];
	struct classinfo *ci = tp->r_class[ichild(*pp) - 1];
	classptr cp = ci->c_join;
	int sym1 = symbol(n1);
	int sym2 = symbol(n2);
	int symcp;
	int symfound = -1;

	if (!cp)
		return No;
	for (; *cp; cp += 2) {
		if (cp[0] != spflag + 1)
			continue;
		symcp = cp[1];
		tp = &table[symcp];
		if (isinclass(sym1, tp->r_class[0])
			&& isinclass(sym2, tp->r_class[1])) {
			symfound = symcp;
			break;
		}
	}

	if (symfound < 0)
		return No;
	n1 = nodecopy(n1);
	n2 = nodecopy(n2); /* 'Cause one of them may overlap tree(*pp) */
	treereplace(pp, table[symfound].r_node);
	if (!down(pp)) Abort();
	treereplace(pp, n1);
	if (!rite(pp)) Abort();
	treereplace(pp, n2);
	if (!up(pp)) Abort();
	return Yes;
}


/*
 * Try to join a node (implicit as tree(*pp)) with some text.
 * That is, try to replace the node by one with it as first child,
 * (some of) the text as second child, and nothing or a space in between.
 *
 * 'Spflag' indicates whether a space is desirable between the nodes
 * (but if No it is only used as advice).
 *
 * Returns the number of characters consumed from str.
 */

Visible int
joinstring(pp, str, spflag, alt_c, mayindent)
	path *pp;
	register string str;
	register bool spflag;
	int alt_c;
	bool mayindent;
{
	register struct table *tp;
	path pa = parent(*pp);
	node n1;
	struct classinfo *ci;
	register classptr cp;
	int sympa = pa ? symbol(tree(pa)) : Rootsymbol;
	register int sym1;
	register int symcp;
	int symfound;
	int len;
	char buf[2];
	bool inter_active = alt_c != 0;

	if (alt_c < 0)
		alt_c = 0;
	ci = table[sympa].r_class[ichild(*pp) - 1];
	Assert(ci);
	cp = ci->c_join;
	if (!cp)
		return 0;

	n1 = tree(*pp);
	sym1 = symbol(n1);
	symfound = -1;
	for (; *cp; cp += 2) {
		if (cp[0] < spflag + 1)
			continue;
		symcp = cp[1];
		tp = &table[symcp];
		if (!mayindent && tp->r_repr[1] && strchr(tp->r_repr[1], '\t'))
			continue;
		if (isinclass(sym1, tp->r_class[0])
			&& ((canfitchar(str[0], tp->r_class[1]))
				|| str[0] == '?' && !inter_active)) {
			if (cp[0] == spflag + 1) {
				symfound = symcp;
				break;
			}
			if (symfound < 0)
				symfound = symcp;
		}
	}

	if (symfound < 0) { /* 1-level recursion */
		if (!alt_c)
			return 0;
		buf[0] = alt_c;
		buf[1] = 0;
		return joinstring(pp, buf, spflag, 0, mayindent);
	}
	n1 = nodecopy(n1); /* 'Cause it overlaps tree(*pp) */
	treereplace(pp, table[symfound].r_node);
	if (!down(pp)) Abort();
	treereplace(pp, n1);
	if (!rite(pp)) Abort();
	len = fitstring(pp, str, 0);
	if (len == 0 && str[0] == '?')
		len = 1;
	Assert(len > 0); /* Disagreement between canfitchar and fitstring */
	if (!up(pp)) Abort();
	return len;
}


/*
 * Similar to joinstring, but now the string must match the delimiter
 * rather than being acceptable as second child.
 * (Interface has changed to resemble resuggest/soften.)
 */

Hidden bool
add_string(ep, pstr)
	environ *ep;
	string *pstr;
{
	register struct table *tp;
	path pa = parent(ep->focus);
	node n1;
	struct classinfo *ci;
	register classptr cp;
	int sympa = pa ? symbol(tree(pa)) : Rootsymbol;
	register int sym1;
	register int symcp;
	register int c;

	ci = table[sympa].r_class[ichild(ep->focus) - 1];
	Assert(ci);
	cp = ci->c_append;
	if (!cp)
		return No;
	n1 = tree(ep->focus);
	sym1 = symbol(n1);
	c = **pstr;
	for (; *cp; cp += 2) {
		if ((*cp&0177) != c)
			continue;
		symcp = cp[1];
		tp = &table[symcp];
		if (isinclass(sym1, tp->r_class[0]))
			break;
	}
	if (!*cp)
		return No;
	++*pstr;
	if (c == ' ') {
		ep->spflag = Yes;
		return Yes;
	}
	n1 = nodecopy(n1); /* 'Cause it overlaps tree(ep->focus) */
	treereplace(&ep->focus, table[symcp].r_node);
	s_down(ep);
	treereplace(&ep->focus, n1);
	s_up(ep);
	ep->mode = FHOLE;
	ep->s1 = 3;
	ep->s2 = (*cp&0200) ? 2 : 1;
	ep->spflag = No;
	return Yes;
}


/*
 * See whether a character may start a new node in a hole with given class.
 */

Hidden bool
canfitchar(c, ci)
	int c;
	struct classinfo *ci;
{
	register classptr cp;
	register int code = Code(c);

	Assert(ci);
	cp = ci->c_insert;
	Assert(cp);
	for (; *cp; cp += 2) {
		if (cp[0] == code)
			return Yes;
	}
	return No;
}


#ifndef NDEBUG
/*
 * Debug routine to print a queue.
 */

Visible Procedure
qshow(q, where)
	queue q;
	string where;
{
	node n;
	char buf[256];
	string cp;
	string sp;

	sprintf(buf, "%s:", where);
	cp = buf + strlen(buf);
	for (;q; q = q->q_link) {
		n = q->q_data;
		*cp++ = ' ';
		if (Is_etext(n)) {
			*cp++ = '"';
			for (sp = e_strval((value) n); *sp; ++sp) {
				if (isprint(*sp) || *sp == ' ') {
					*cp++ = *sp;
					if (*sp == '"')
						*cp++ = *sp;
				}
				else {
					sprintf(cp, "\\%03o", *sp&0377);
					cp += 4;
				}
			}
			*cp++ = '"';
		}
		else {
			strncpy(cp, table[symbol(n)].r_name, 80);
			cp += strlen(cp);
		}
		if (cp >= buf+80) {
			strcpy(buf+76, "...");
			break;
		}
	}
	*cp = 0;
	debug(buf);
}
#endif /* NDEBUG */
