/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1986. */

/*
 * B editor -- Delete and copy commands.
 */

#include "b.h"
#include "bedi.h"
#include "etex.h"
#include "bobj.h"
#include "erro.h"
#include "node.h"
#include "gram.h"
#include "supr.h"
#include "queu.h"
#include "tabl.h"

extern bool lefttorite;

Forward Hidden bool delvarying();
Forward Hidden bool delfixed();
Forward Hidden bool delsubset();
Forward Hidden bool delwhole();
Forward Hidden bool delhole();
Forward Hidden Procedure nonewline();
Forward Hidden Procedure balance();
Forward Hidden bool copyin();
Forward Hidden bool colonhack();
Forward Hidden bool allright();

Forward value copyout();

/*
 * DELETE and COPY currently share a buffer, called the copy buffer.
 * (Physically, there is one such a buffer in each environment.)
 * In ordinary use, the copy buffer receives the text deleted by the
 * last DELETE command (unless it just removed a hole); the COPY command
 * can then be used (with the focus on a hole) to copy it back.
 * When some portion of text must be held while other text is deleted,
 * the COPY command again, but now with the focus on the text to be held,
 * copies it to the buffer and deleted text won't overwrite the buffer
 * until it is copied back at least once.
 * If the buffer holds text that was explicitly copied out but not yet
 * copied back in, it is saved on a file when the editor exits, so it can
 * be used in the next session; but this is not true for text implicitly
 * placed in the buffer through DELETE.
 */

/*
 * Delete command -- delete the text in the focus, or delete the hole
 * if it is only a hole.
 */

Visible bool
deltext(ep)
	register environ *ep;
{
	higher(ep);
	shrink(ep);
	if (ishole(ep))
		return delhole(ep);
	if (!ep->copyflag) {
		release(ep->copybuffer);
		ep->copybuffer = copyout(ep);
	}
	return delbody(ep);
}


/*
 * Delete the focus under the assumption that it contains some text.
 */

Visible bool
delbody(ep)
	register environ *ep;
{
	ep->changed = Yes;

	subgrow(ep, No, Yes); /* Don't ignore spaces */
	switch (ep->mode) {

	case SUBRANGE:
		if (ep->s1&1)
			return delfixed(ep);
		return delvarying(ep);

	case SUBSET:
		return delsubset(ep, Yes);

	case SUBLIST:
		return delsublist(ep);

	case WHOLE:
		return delwhole(ep);

	default:
		Abort();
		/* NOTREACHED */
	}
}


/*
 * Delete portion (ep->mode == SUBRANGE) of varying text ((ep->s1&1) == 0).
 */

Hidden bool
delvarying(ep)
	register environ *ep;
{
	auto queue q = Qnil;
	register node n = tree(ep->focus);
	auto value v;
	value t1, t2;

	v = (value) child(n, ep->s1/2);
	Assert(ep->mode == SUBRANGE && !(ep->s1&1)); /* Wrong call */
	Assert(Is_etext(v)); /* Inconsistent parse tree */
	if (ep->s2 == 0) {
		/* strval(v)[ep->s3 + 1] */
		if (!mayinsert(tree(ep->focus), ep->s1/2, 0, e_ncharval(ep->s3 + 2, v))) {
			/* Cannot do simple substring deletion. */
/*			stringtoqueue(strval(v) + ep->s3 + 1, &q); */
			t1= e_ibehead(v, ep->s3 + 2);
			preptoqueue((node) t1, &q);
			release(t1);
			delfocus(&ep->focus);
			ep->mode = WHOLE;
			return app_queue(ep, &q);
		}
	}
	v = copy(v);
	/* putintrim(&v, ep->s2, len - ep->s3 - 1, ""); */
	t1= e_icurtail(v, ep->s2);
	t2= e_ibehead(v, ep->s3 + 2);
	release(v);
	v= e_concat(t1, t2);
	release(t1); release(t2);
	s_downi(ep, ep->s1/2);
	treereplace(&ep->focus, (node) v);
	s_up(ep);
	ep->mode = VHOLE;
	return Yes;
}


/*
 * Delete portion (ep->mode == SUBRANGE) of fixed text ((ep->s1&1) == 1).
 */

Hidden bool
delfixed(ep)
	register environ *ep;
{
	register node n = tree(ep->focus);
	char buf[15]; /* Long enough for all fixed texts */
	string *nr= noderepr(n);
	register string repr = nr[ep->s1/2];
	register int len;
	queue q = Qnil;
	bool ok;

	Assert(ep->mode == SUBRANGE && (ep->s1&1));
	if (ep->s1 > 1) {
		ep->mode = FHOLE;
		return Yes;
	}
	else if (symbol(n) == Select && ep->s2 == 0 && repr[ep->s3+1] == ':') {
		/* hack to prevent asserr in app_queue below */
		ep->s3++;
	}
	Assert(fwidth(repr) < sizeof buf - 1);
	len = ep->s2;
	ep->s2 = ep->s3 + 1;
	ep->mode = FHOLE;
	nosuggtoqueue(ep, &q);
	strcpy(buf, repr);
	if (nchildren(tree(ep->focus)) > 0)
		buf[len] = 0;
	else
		strcpy(buf+len, buf+ep->s2);
	delfocus(&ep->focus);
	ep->mode = WHOLE;
	markpath(&ep->focus, 1);
	ok = ins_string(ep, buf, &q, 0);
	if (!ok) {
		qrelease(q);
		return No;
	}
	if (!firstmarked(&ep->focus, 1)) Abort();
	unmkpath(&ep->focus, 1);
	fixfocus(ep, len);
	return app_queue(ep, &q);
}

/*
 * refinement for delsubset and delsublist
 * to delete an initial KEYWORDS part before an expression
 * (the latter being sent to qq)
 */

Hidden bool hole_ify_keywords(ep, qq)
	register environ *ep;
	queue *qq;
{
	treereplace(&ep->focus, gram(Kw_plus));
	ep->mode= VHOLE;
	ep->s1= 4;
	ep->s2= 0;
	if (app_queue(ep, qq)) {
		ep->mode= FHOLE;
		ep->s1= 1;
		ep->s2= 0;
		return Yes;
	}
	return No;
}

/*
 * Delete focus if ep->mode == SUBSET.
 */

Hidden bool
delsubset(ep, hack)
	register environ *ep;
	bool hack;
{
	auto queue q = Qnil;
	auto queue q2 = Qnil;
	register node n = tree(ep->focus);
	register node nn;
	register string *rp = noderepr(n);
	register int nch = nchildren(n);
	register int i;
	bool res;
	int sym= symbol(n);
	
	if (hack) {
		shrsubset(ep);
		if (ep->s1 == ep->s2 && !(ep->s1&1)) {
			nn = child(tree(ep->focus), ep->s1/2);
			if (fwidth(noderepr(nn)[0]) < 0) {
				/* It starts with a newline, leave the newline */
				s_downi(ep, ep->s1/2);
				ep->mode = SUBSET;
				ep->s1 = 2;
				ep->s2 = 2*nchildren(nn) + 1;
				return delsubset(ep, hack);
			}
		}
		subgrsubset(ep, No); /* Undo shrsubset */
		if (ep->s2 == 3 && rp[1] && !strcmp(rp[1], "\t"))
			--ep->s2; /* Hack for deletion of unit-head or if/for/wh. head */
	}
	if (ep->s1 == 1 && Fw_negative(rp[0]))
		++ep->s1; /* Hack for deletion of test-suite or refinement head */

	if (Fw_zero(rp[0]) ? (ep->s2 < 3 || ep->s1 > 3) : ep->s1 > 1) {
		/* No deep structural change */
		for (i = (ep->s1+1)/2; i <= ep->s2/2; ++i) {
			s_downi(ep, i);
			delfocus(&ep->focus);
			s_up(ep);
		}
		if (ep->s1&1) {
			ep->mode = FHOLE;
			ep->s2 = 0;
		}
		else if (Is_etext(child(tree(ep->focus), ep->s1/2))) {
			ep->mode = VHOLE;
			ep->s2 = 0;
		}
		else {
			s_downi(ep, ep->s1/2);
			ep->mode = ATBEGIN;
		}
		return Yes;
	}

	balance(ep); /* Make balanced \t - \b pairs */
	subsettoqueue(n, 1, ep->s1-1, &q);
	subsettoqueue(n, ep->s2+1, 2*nch+1, &q2);
	nonewline(&q2); /* Wonder what will happen...? */
	
	if (ep->s1 == 1 && Fw_positive(rp[0]) && allowed(ep->focus, Kw_plus)
	    && (sym != If && sym != While && sym != For && sym != Select))
	{
		Assert(emptyqueue(q));
		return hole_ify_keywords(ep, &q2);
	}
	delfocus(&ep->focus);
	ep->mode = ATBEGIN;
	leftvhole(ep);
	if (!ins_queue(ep, &q, &q2)) {
		qrelease(q2);
		return No;
	}
	res= app_queue(ep, &q2);
#ifdef USERSUGG
	if (symbol(tree(ep->focus)) == Suggestion)
		killsugg(ep, (string*)NULL);
#endif
	return res;
}


/*
 * Delete the focus if ep->mode == SUBLIST.
 */

delsublist(ep)
	register environ *ep;
{
	register node n;
	register int i;
	register int sym;
	queue q = Qnil;
	bool flag;

	Assert(ep->mode == SUBLIST);
	n = tree(ep->focus);
	flag = fwidth(noderepr(n)[0]) < 0;
	for (i = ep->s3; i > 0; --i) {
		n = lastchild(n);
		Assert(n);
	}
	if (flag) {
		n = nodecopy(n);
		s_down(ep);
		do {
			delfocus(&ep->focus);
		} while (rite(&ep->focus));
		if (!allowed(ep->focus, symbol(n))) {
			ederr(0); /* The remains wouldn't fit */
			noderelease(n);
			return No;
		}
		treereplace(&ep->focus, n);
		s_up(ep);
		s_down(ep); /* I.e., to leftmost sibling */
		ep->mode = WHOLE;
		return Yes;
	}
	sym = symbol(n);
	if (sym == Optional || sym == Hole) {
		delfocus(&ep->focus);
		ep->mode = WHOLE;
	}
	else if (!allowed(ep->focus, sym)) {
		preptoqueue(n, &q);
		if (symbol(tree(ep->focus)) == Kw_plus) {
			return hole_ify_keywords(ep, &q);
		}
		delfocus(&ep->focus);
		ep->mode = WHOLE;
		return app_queue(ep, &q);
	}
	else {
		treereplace(&ep->focus, nodecopy(n));
		ep->mode = ATBEGIN;
	}
	return Yes;
}


/*
 * Delete the focus if ep->mode == WHOLE.
 */

Hidden bool
delwhole(ep)
	register environ *ep;
{
	register int sym = symbol(tree(ep->focus));

	Assert(ep->mode == WHOLE);
	if (sym == Optional || sym == Hole)
		return No;
	delfocus(&ep->focus);
	return Yes;
}


/*
 * Delete the focus if it is only a hole.
 * Assume shrink() has been called before!
 */

Hidden bool
delhole(ep)
	register environ *ep;
{
	node n;
	int sym;
	bool flag = No;

	switch (ep->mode) {
	
	case ATBEGIN:
	case VHOLE:
	case FHOLE:
	case ATEND:
		return widen(ep, Yes);

	case WHOLE:
		Assert((sym = symbol(tree(ep->focus))) == Optional || sym == Hole);
		if (ichild(ep->focus) != 1)
			break;
		if (!up(&ep->focus))
			return No;
		higher(ep);
		ep->mode = SUBSET;
		ep->s1 = 2;
		ep->s2 = 2;
		if (fwidth(noderepr(tree(ep->focus))[0]) < 0) {
			flag = Yes;
			ep->s2 = 3; /* Extend to rest of line */
		}
	}

	ep->changed = Yes;
	grow(ep, Yes);
	
	if (!parent(ep->focus) && colonhack(ep, Yes))
		ep->mode= WHOLE; /* to delete a sequence of hole's below */
	
	switch (ep->mode) {

	case SUBSET:
		if (!delsubset(ep, No))
			return No;
		if (!flag)
			return widen(ep, Yes);
		leftvhole(ep);
		oneline(ep);
		return Yes;

	case SUBLIST:
		n = tree(ep->focus);
		n = lastchild(n);
		sym = symbol(n);
		if (!allowed(ep->focus, sym) 
		    && sym != Exp_plus && symbol(tree(ep->focus)) != Kw_plus) {
		    /* previous line enables deletion of emptied KEYWORD */
			ederr(0); /* The remains wouldn't fit */
			return No;
		}
		flag = samelevel(sym, symbol(tree(ep->focus)));
		treereplace(&ep->focus, nodecopy(n));
		if (flag) {
			ep->mode = SUBLIST;
			ep->s3 = 1;
		}
		else
			ep->mode = WHOLE;
		return Yes;

	case WHOLE:
		Assert(!parent(ep->focus)); /* Must be at root! */
		sym= symbol(tree(ep->focus));
		if (sym != Optional && sym != Hole) {
			/* delete sequence of Hole's */
			delfocus(&ep->focus);
			return Yes;
		}
		return No;

	default:
		Abort();
		/* NOTREACHED */

	}
}


/*
 * Subroutine to delete the focus.
 */

Visible Procedure
delfocus(pp)
	register path *pp;
{
	register path pa = parent(*pp);
	register int sympa = pa ? symbol(tree(pa)) : Rootsymbol;

	treereplace(pp, child(gram(sympa), ichild(*pp)));
}


/*
 * Copy command -- copy the focus to the copy buffer if it contains
 * some text, copy the copy buffer into the focus if the focus is
 * empty (just a hole).
 */

Visible bool
copyinout(ep)
	register environ *ep;
{
	shrink(ep);
	if (!ishole(ep)) {
		release(ep->copybuffer);
		ep->copybuffer = copyout(ep);
		ep->copyflag = !!ep->copybuffer;
		return ep->copyflag;
	}
	else {
		fixit(ep); /* Make sure it looks like a hole now */
		if (!copyin(ep, (queue) ep->copybuffer))
			return No;
		ep->copyflag = No;
		return Yes;
	}
}


/*
 * Copy the focus to the copy buffer.
 */

Visible value
copyout(ep)
	register environ *ep;
{
	auto queue q = Qnil;
	auto path p;
	register node n;
	register value v;
	char buf[15];
	register string *rp;
	register int i;
	value w;

	switch (ep->mode) {
	case WHOLE:
		preptoqueue(tree(ep->focus), &q);
		break;
	case SUBLIST:
		p = pathcopy(ep->focus);
		for (i = ep->s3; i > 0; --i)
			if (!downrite(&p)) Abort();
		for (i = ep->s3; i > 0; --i) {
			if (!up(&p)) Abort();
			n = tree(p);
			subsettoqueue(n, 1, 2*nchildren(n) - 1, &q);
		}
		pathrelease(p);
		break;
	case SUBSET:
		balance(ep);
		subsettoqueue(tree(ep->focus), ep->s1, ep->s2, &q);
		break;
	case SUBRANGE:
		Assert(ep->s3 >= ep->s2);
		if (ep->s1&1) { /* Fixed text */
			Assert(ep->s3 - ep->s2 + 1 < sizeof buf);
			rp = noderepr(tree(ep->focus));
			Assert(ep->s2 < Fwidth(rp[ep->s1/2]));
			strncpy(buf, rp[ep->s1/2] + ep->s2, ep->s3 - ep->s2 + 1);
			buf[ep->s3 - ep->s2 + 1] = 0;
			stringtoqueue(buf, &q);
		}
		else { /* Varying text */
			v = (value) child(tree(ep->focus), ep->s1/2);
			Assert(Is_etext(v));
/*			v = trim(v, ep->s2, Length(v) - ep->s3 - 1); */
			w= e_icurtail(v, ep->s3 + 1);
			v= e_ibehead(w, ep->s2 + 1);
			release(w);
			preptoqueue((node)v, &q);
			release(v);
		}
		break;
	default:
		Abort();
	}
	nonewline(&q);
	return (value)q;
}


/*
 * Subroutine to ensure the copy buffer doesn't start with a newline.
 */

Hidden Procedure
nonewline(pq)
	register queue *pq;
{
	register node n;
	register int c;

	if (!emptyqueue(*pq)) {
		for (;;) {
			n = queuebehead(pq);
			if (Is_etext(n)) {
				if (e_ncharval(1, (value)n) != '\n')
					preptoqueue(n, pq);
				noderelease(n);
				break;
			}
			else {
				c = nodechar(n);
				if (c != '\n')
					preptoqueue(n, pq);
				else
					splitnode(n, pq);
				noderelease(n);
				if (c != '\n')
					break;
			}
		}
	}
}


/*
 * Refinement for copyout, case SUBSET: make sure that \t is balanced with \b.
 * Actually it can only handle the case where a \t is in the subset and the
 * matching \b is immediately following.
 */

Hidden Procedure
balance(ep)
	environ *ep;
{
	string *rp = noderepr(tree(ep->focus));
	int i;
	int level = 0;

	Assert(ep->mode == SUBSET);
	for (i = ep->s1/2; i*2 < ep->s2; ++i) {
		if (rp[i]) {
			if (strchr(rp[i], '\t'))
				++level;
			else if (strchr(rp[i], '\b'))
				--level;
		}
	}
	if (level > 0 && i*2 == ep->s2 && rp[i] && strchr(rp[i], '\b'))
		ep->s2 = 2*i + 1;
}


/*
 * Copy the copy buffer to the focus.
 */

Hidden bool
copyin(ep, q)
	register environ *ep;
	/*auto*/ queue q;
{
	auto queue q2 = Qnil;
	bool res;
	
	if (!q) {
		ederr(COPY_EMPTY); /* Empty copy buffer */
		return No;
	}
	ep->changed = Yes;
	q = qcopy(q);
	lefttorite= Yes;
	if (!ins_queue(ep, &q, &q2)) {
		qrelease(q2);
		lefttorite= No;
		return No;
	}
	res= app_queue(ep, &q2);
	lefttorite= No;
#ifdef USERSUGG
	if (symbol(tree(ep->focus)) == Suggestion)
		killsugg(ep, (string*)NULL);
#endif
	return res;
}


/*
 * Find out whether the focus looks like a hole or if it has some real
 * text in it.
 * Assumes shrink(ep) has already been performed.
 */

Visible bool
ishole(ep)
	register environ *ep;
{
	register int sym;

	switch (ep->mode) {
	
	case ATBEGIN:
	case ATEND:
	case VHOLE:
	case FHOLE:
		return Yes;

	case SUBLIST:
	case SUBRANGE:
		return No;

	case SUBSET:
		return colonhack(ep, No);

	case WHOLE:
		sym = symbol(tree(ep->focus));
		return sym == Optional || sym == Hole;

	default:
		Abort();
		/* NOTREACHED */
	}
}


/*
 * Amendment to ishole so that it categorizes '?: ?' as a hole.
 * This makes deletion of empty refinements / alternative-suites
 * easier (Steven).
 * Hacked to enable deletion of sequence of hole's at outer level.
 */

Hidden bool
colonhack(ep, all)
	environ *ep;
{
	node n = tree(ep->focus);
	node n1;
	string *rp = noderepr(n);
	int i0, ii, i;
	int sym;
	
	if (all) {
		/* hack to delete sequence of hole's on outer level */
		i0= 1; ii= 2*nchildren(n) + 1;
	}
	else {
		/* original code: */
		i0= ep->s1; ii= ep->s2;
	}
	for (i = i0; i <= ii; ++i) {
		if (i&1) {
			if (!allright(rp[i/2]))
				return No;
		}
		else {
			n1 = child(n, i/2);
			if (Is_etext(n1))
				return No;
			sym = symbol(n1);
			if (sym != Hole && sym != Optional)
				return No;
		}
	}
	return Yes;
}


/*
 * Refinement for colonhack.  Recognize strings that are almost blank
 * (i.e. containing only spaces, colons and the allowed control characters).
 */

Hidden bool
allright(repr)
	string repr;
{
	if (repr) {
		for (; *repr; ++repr) {
			if (!strchr(": \t\b\n\r", *repr))
				return No;
		}
	}
	return Yes;
}
