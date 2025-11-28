/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1986. */

/*
 * B editor -- Parse tree and Focus stack.
 */

#include "b.h"
#include "bedi.h"
#include "etex.h"
#include "bobj.h"
#include "node.h"
#include "bmem.h"

Forward Hidden Procedure repwidth();

#define Register register
	/* Used for registers 4-6.  Define as empty macro on PDP */


/*
 * Lowest level routines for 'node' data type.
 */

#define Isnode(n) ((n) && (n)->type == Nod)

#define Nchildren(n) ((n)->len)
#define Symbol(n) ((n)->n_symbol)
#define Child(n, i) ((n)->n_child[(i)-1])
#define Marks(n) ((n)->n_marks)
#define Width(n) ((n)->n_width)


/*
 * Routines which are macros for the compiler but real functions for lint,
 * so it will check the argument types more strictly.
 */

#ifdef lint
Visible node nodecopy(n)
     node n;
{
	return (node) copy((value) n);
}

Visible Procedure noderelease(n)
     node n;
{
	release((value)n);
}

Visible Procedure nodeuniql(pn)
     node *pn;
{
	uniql((value*)pn);
}
#endif /* lint */

/*
 * Allocate a new node.
 */

Hidden node
mk_node(nch)
	register int nch;
{
	register node n = (node) grab(Nod, nch);
	register int i;

	n->n_marks = 0;
	n->n_width = 0;
	n->n_symbol = 0;
	for (i = nch-1; i >= 0; --i)
		n->n_child[i] = Nnil;
	return n;
}

Visible node
newnode(nch, sym, children)
	register int nch;
	Register int sym;
	register node children[];
{
	register node n = (node) mk_node(nch); /* Must preset with zeros! */

	Symbol(n) = sym;
	for (; nch > 0; --nch)
		Child(n, nch) = children[nch-1];
	Width(n) = evalwidth(n);
	return n;
}

Visible int nodewidth(n) node n; {
	if (Is_etext(n))
		return e_length((value) n);
	else
		return Width(n);
}

/*
 * Macros to change the fields of a node.
 */

#define Locchild(pn, i) \
	(Refcnt(*(pn)) == 1 || (nodeuniql(pn), 1), &Child(*(pn), i))
#define Setmarks(pn, x) \
	(Refcnt(*(pn)) == 1 || (nodeuniql(pn), 1), Marks(*(pn))=(x))
#define Setwidth(pn, w) \
        (Refcnt(*(pn)) == 1 || (nodeuniql(pn), 1), Width(*(pn))=w)


/*
 * Change a child of a node.
 * Like treereplace(), it does not increase the reference count of n.
 */

Visible Procedure
setchild(pn, i, n)
	register node *pn;
	register int i;
	Register node n;
{
	register node *pch;
	register node oldchild;

	Assert(Isnode(*pn));
	pch = Locchild(pn, i);
	oldchild = *pch;
	*pch = n;
	repwidth(pn, oldchild, n);
	noderelease(oldchild);
}


/*
 * Lowest level routines for 'path' data type.
 */

#define NPATHFIELDS 6

#define Parent(p) ((p)->p_parent)
#define Tree(p) ((p)->p_tree)
#define Ichild(p) ((p)->p_ichild)


/*
 * Routines which are macros for the compiler but real functions for lint,
 * so it will check the argument types more strictly.
 */

#ifdef lint
Visible path
pathcopy(p)
	path p;
{
	return (path) copy((value) p);
}

Visible Procedure
pathrelease(p)
	path p;
{
	release((value)p);
}

Visible Procedure
pathuniql(pp)
	path *pp;
{
	uniql((value*)pp);
}
#endif /* lint */

/*
 * Allocate a new path entry.
 */

Hidden path
mk_path()
{
	register path p = (path) grab(Pat, 0);

	p->p_parent = NilPath;
	p->p_tree = Nnil;
	p->p_ichild = 0;
	p->p_ycoord = 0;
	p->p_xcoord = 0;
	p->p_level = 0;
	p->p_addmarks = 0;
	p->p_delmarks = 0;
	return p;
}

Visible path
newpath(pa, n, i)
	register path pa;
	register node n;
	Register int i;
{
	register path p = (path) mk_path();

	Parent(p) = pa;
	Tree(p) = n;
	Ichild(p) = i;
	Ycoord(p) = Xcoord(p) = Level(p) = 0;
	return p;
}


/*
 * Macros to change the fields of a path entry.
 */

#define Uniqp(pp) (Refcnt(*(pp)) == 1 || (pathuniql(pp), 1))

#define Setcoord(pp, y, x, level) (Uniqp(pp), \
	(*(pp))->p_ycoord = y, (*(pp))->p_xcoord = x, (*(pp))->p_level = level)

#define Locparent(pp) (Uniqp(pp), &Parent(*(pp)))

#define Loctree(pp) (Uniqp(pp), &Tree(*(pp)))

#define Addmarks(pp, x) (Uniqp(pp), \
	(*(pp))->p_addmarks |= (x), (*(pp))->p_delmarks &= ~(x))

#define Delmarks(pp, x) (Uniqp(pp), \
	(*(pp))->p_delmarks |= (x), (*(pp))->p_addmarks &= ~(x))

/*
 * The following procedure sets the new width of node *pn when child
 * oldchild is replaced by child newchild.
 * This was added because the original call to evalwidth seemed to
 * be the major caller of noderepr() and fwidth().
 */

Hidden Procedure
repwidth(pn, old, new)
	register node *pn;
	Register node old;
	Register node new;
{
	register int w = Width(*pn);
	register int oldwidth = nodewidth(old);
	register int newwidth = nodewidth(new);

	if (w >= 0) {
		Assert(oldwidth >= 0);
		if (newwidth < 0) {
			Setwidth(pn, newwidth);
			return;
		}
	}
	else {
		if (oldwidth == w && newwidth > 0) {
			w= evalwidth(*pn);
			Setwidth(pn, w);
			return;
		}
		if (oldwidth > 0)
			oldwidth = 0;
		if (newwidth > 0)
			newwidth = 0;
	}
	newwidth -= oldwidth;
	if (newwidth)
		Setwidth(pn, w + newwidth);
}


Visible Procedure
markpath(pp, new)
	register path *pp;
	register markbits new;
{
	register node *pn;
	register markbits old;

	Assert(Is_Node(Tree(*pp)));
	old = Marks(Tree(*pp));
	if ((old|new) == old)
		return; /* Bits already set */

	pn = Loctree(pp);
	Setmarks(pn, old|new);
	Addmarks(pp, new&~old);
}


Visible Procedure
unmkpath(pp, del)
	register path *pp;
	register int del;
{
	register node *pn;
	register markbits old;

	Assert(Is_Node(Tree(*pp)));
	old = Marks(Tree(*pp));
	if ((old&~del) == del)
		return;

	pn = Loctree(pp);
	Setmarks(pn, old&~del);
	Delmarks(pp, del&old);
}


Hidden Procedure
clearmarks(pn)
	register node *pn;
{
	register int i;

	if (!Marks(*pn))
		return;
	if (Isnode(*pn)) {
		Setmarks(pn, 0);
		for (i = Nchildren(*pn); i > 0; --i)
			clearmarks(Locchild(pn, i));
	}
}


/*
 * Replace the focus' tree by a new node.
 * WARNING: n's reference count is not increased!
 * You can also think of this as: treereplace(pp, n) implies noderelease(n).
 * Mark bits are copied from the node being replaced.
 */

Visible Procedure
treereplace(pp, n)
	register path *pp;
	register node n;
{
	register node *pn;
	register markbits old;

	pn = Loctree(pp);
	if (Is_Node(*pn))
		old = Marks(*pn);
	else
		old = 0;
	noderelease(*pn);
	*pn = n;
	if (Is_Node(n)) {
		clearmarks(pn);
		if (old)
			Setmarks(pn, old);
	}
	else if (old)
		Addmarks(pp, old);
}


Visible bool
up(pp)
	register path *pp;
{
	register path p = *pp;
	register path pa = Parent(p);
	register path *ppa;
	register node n;
	register node npa;
	register node *pn;
	node oldchild;
	node *pnpa;
	int i;
	markbits add;
	markbits del;

	if (!pa)
		return No;

	i = ichild(p);
	n = Tree(p);
	if (Child(Tree(pa), i) != n) {
		n = nodecopy(n);
		ppa = Locparent(pp);
		pnpa = Loctree(ppa);
		pn = Locchild(pnpa, i);
		oldchild = *pn;
		*pn = n;
		repwidth(pnpa, oldchild, n);
		noderelease(oldchild);
	
		add = p->p_addmarks;
		del = p->p_delmarks;
		if (add|del) {
			p = *pp;
			p->p_addmarks = 0;
			p->p_delmarks = 0;
			if (add)
				Addmarks(ppa, add);
			npa = *pnpa;
			if (del) {
				for (i = Nchildren(npa); i > 0; --i)
					if (i != ichild(p))
						del &= ~marks(Child(npa, i));
				Delmarks(ppa, del);
			}
			Setmarks(pnpa, Marks(npa)&~del|add);
		}
	}
	/* else: still connected */

	p = pathcopy(Parent(*pp));
	pathrelease(*pp);
	*pp = p;
	return Yes;
}


Visible bool
downi(pp, i)
	register path *pp;
	register int i;
{
	register node n;
	auto int y;
	auto int x;
	auto int level;

	n = Tree(*pp);
	if (!Isnode(n) || i < 1 || i > Nchildren(n))
		return No;

	y = Ycoord(*pp);
	x = Xcoord(*pp);
	level = Level(*pp);
	*pp = newpath(*pp, nodecopy(Child(n, i)), i);
	evalcoord(n, i, &y, &x, &level);
	Setcoord(pp, y, x, level);
	return Yes;
}


Visible bool
downrite(pp)
	register path *pp;
{
	if (!Isnode(Tree(*pp)))
		return No;
	return downi(pp, Nchildren(Tree(*pp)));
}


Visible bool
left(pp)
	register path *pp;
{
	register int i;

	i = ichild(*pp) - 1;
	if (i <= 0)
		return No;
	if (!up(pp))
		return No;
	return downi(pp, i);
}


Visible bool
rite(pp)
	register path *pp;
{
	register int i;
	register path pa = Parent(*pp);

	i = ichild(*pp) + 1;
	if (!pa || i > Nchildren(Tree(pa)))
		return No;
	if (!up(pp))
		return No;
	return downi(pp, i);
}


/*
 * Highest level: small utilities.
 *
 * WARNING: Several of the following routines may change their argument
 * even if they return No.
 * HINT: Some of these routines are not used; they are included for
 * completeness of the provided set of operators only.  If you have
 * space problems (as, e.g., on a PDP-11), you can delete the superfluous
 * ones (lint will tell you which they are).
 */

Visible Procedure
top(pp)
	register path *pp;
{
	while (up(pp))
		;
}

#ifdef NOT_USED
Visible bool
nextnode(pp)
	register path *pp;
{
	while (!rite(pp)) {
		if (!up(pp))
			return No;
	}
	return Yes;
}
#endif

#ifdef NOT_USED
Visible Procedure
firstleaf(pp)
	register path *pp;
{
	while (down(pp))
		;
}
#endif

#ifdef NOT_USED
Visible bool
nextleaf(pp)
	register path *pp;
{
	if (!nextnode(pp))
		return No;
	firstleaf(pp);
	return Yes;
}
#endif

#ifdef NOT_USED
Visible bool
prevnode(pp)
	register path *pp;
{
	while (!left(pp)) {
		if (!up(pp))
			return No;
	}
	return Yes;
}
#endif

#ifdef NOT_USED
Visible Procedure
lastleaf(pp)
	register path *pp;
{
	while (downrite(pp))
			;
}
#endif

#ifdef NOT_USED
Visible bool
prevleaf(pp)
	register path *pp;
{
	if (!prevnode(pp))
		return No;
	lastleaf(pp);
	return Yes;
}
#endif

#ifdef NOT_USED
Visible bool
nextmarked(pp, x)
	register path *pp;
	register markbits x;
{
	do {
		if (!nextnode(pp))
			return No;
	} while (!marked(*pp, x));
	while (down(pp)) {
		while (!marked(*pp, x)) {
			if (!rite(pp)) {
				if (!up(pp)) Abort();
				return Yes;
			}
		}
	}
	return Yes;
}
#endif

Visible bool
firstmarked(pp, x)
	register path *pp;
	register markbits x;
{
	while (!marked(*pp, x)) {
		if (!up(pp))
			return No;
	}
	while (down(pp)) {
		while (Is_etext(tree(*pp)) || !marked(*pp, x)) {
			if (!rite(pp)) {
				if (!up(pp)) Abort();
				return Yes;
			}
		}
	}
	return Yes;
}

#ifdef NOT_USED
Visible bool
prevmarked(pp, x)
	register path *pp;
	register markbits x;
{
	do {
		if (!prevnode(pp))
			return No;
	} while (!marked(*pp, x));
	while (downrite(pp)) {
		while (!marked(*pp, x)) {
			if (!left(pp)) {
				if (!up(pp)) Abort();
				return Yes;
			}
		}
	}
	return Yes;
}
#endif

/*
 * Deliver the path length to the root.
 */


Visible int
pathlength(p)
	register path p;
{
	register int n;

	for (n = 0; p; ++n)
		p = parent(p);
	return n;
}

Visible Procedure
putintrim(pn, head, tail, str)
	register value *pn;
	register int head;
	Register int tail;
	Register string str;
{
	register value v = *pn; 
	value t1, t2, t3;
	int len= e_length(v);

	Assert(head >= 0 && tail >= 0 && head + tail <= len);
	t1= e_icurtail(v, head);
	t2= mk_etext(str);
	t3= e_concat(t1, t2);
	release(t1); release(t2);
	t1= e_ibehead(v, len - tail + 1);
	t2= e_concat(t3, t1);
	release(t3); release(t1);
	release(v);
	*pn = t2;
}

/*
 * Touch the node in focus.
 */

Visible Procedure
touchpath(pp)
	register path *pp;
{
	nodeuniql(Loctree(pp));
}
