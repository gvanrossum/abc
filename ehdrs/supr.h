/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1986. */

/*
 * B editor -- Superstructure for fine focusing.
 */

/*
 * Interpretation of s1, s2, s3 of focus (where n == tree(ep->focus)):
 *
 * s:	1	2	3	4	5	6	7	8	9
 *	fixed	varia	fixed	varia	fixed	varia	fixed	varia	fixed
 *
 * the fixed fields are the r_repr[i] fields of grammar rule table[symbol(n)],
 * with i == s/2 (and i is odd), thus:
 *
 *	repr[0]		repr[1]		repr[2]		repr[3]		repr[4]
 *
 * the variable fields are the children of the node, with i == s/2 and s even:
 *
 *		child(n,1)	child(n,2)	child(n,3)	child(n,4)
 *
 * (and child(n,i) == n->n_child[i-1]:-)
 *
 * Interpretation of mode of focus:
 *
 * WHOLE: whole node is the focus;
 * SUBSET: s1/2, s2/2 are first and last child number under focus;
 *          odd means fixed text, even means child node;
 * SUBRANGE: focus is part of fixed text or child; in C terms:
 *          s1 odd: repr[s1/2][s2..s3];
 *          s1 even: e_strval(child(n, s1/2))[s2..s3];
 * VHOLE: s1/2 is fixed text number; volatile hole before char s2;
 *          if s1 is even, ditto for child which must be "text".
 * ATEND: a volatile hole just after the entire node.
 * ATBEGIN: ditto just before it.
 * SUBLIST: s3 indicates how many times downrite() bring us
 *          beyond the focus (i.e., the focus is the subtree below
 *          ep->focus EXCLUDING the subtree reached after s3 times
 *          downrite().  Note s3 > 0.
 * FHOLE: Like VHOLE but in Fixed text.
 *
 * It is assumed that if the focus is a substring of fixed text
 * (SUBRANGE, VHOLE), it does not begin or end with lay-out of spaces.
 */

#define WHOLE	'W'
#define SUBSET	'S'
#define SUBRANGE	'R'
#define VHOLE	'V'
#define ATEND	'E'
#define ATBEGIN	'B'
#define SUBLIST	'L'
#define FHOLE	'F'

typedef struct {
	path focus;
	char mode;
	char /*bool*/ copyflag;
	char /*bool*/ spflag;
	char /*bool*/ changed;
	short /*0..2*MAXCHILD+1*/ s1;
	short s2;
	short s3;
	short highest;
	value copybuffer; /* Actually, a queue */
	value oldmacro; /* A text */
	value newmacro; /* A text, too */
	int generation;
} environ;

#ifdef STRUCTASS
#define Emove(e1, e2) ((e2) = (e1))
#else /* !STRUCTASS */
#define Emove(e1, e2) emove(&(e1), &(e2))
#endif /* !STRUCTASS */
#define Ecopy(e1, e2) ecopy(&(e1), &(e2))
#define Erelease(e) erelease(&(e))

bool ishole();
