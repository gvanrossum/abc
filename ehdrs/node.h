/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1986. */

/*
 * B editor -- Parse tree and Focus stack.
 */

/*
 * Assertion macro.
 *
 * This one differs from the one in #include <assert.h> in that it
 * is usable as an expression operand, e.g. up(ep) || Assert(No).
 * The function asserr() must unconditionally terminate the program.
 * If the accumulated __FILE__ data wastes too much of your data
 * space, omit them and change the code in asserr() that uses them.
 * You better trust your code then, because unless compiled with "-g"
 * it's difficult to dig the line number information from the core dump.
 *
 * There is also a variant called Abort() which is equivalent to Assert(No).
 */

#ifdef NDEBUG
#define Abort() abort() /* Always fail */
#define Assert(cond) 0 /* Dummy expression */
#else /* NDEBUG */
#define Abort() asserr(__FILE__, __LINE__)
#define Assert(cond) ((cond) || (Abort(), 1))
#endif /* NDEBUG */

node newnode();

#ifndef NDEBUG
#define symbol(n) (Assert(Is_Node(n)), (n)->n_symbol)
#define nchildren(n) (Assert(Is_Node(n)), Length(n))
#define marks(n) (Assert(Is_Node(n)), (n)->n_marks)
#define child(n, i) \
	(Assert(Is_Node(n) && (i)>0 && (i)<=Length(n)), (n)->n_child[(i)-1])
#define lastchild(n) \
	(Assert(Is_Node(n) && Length(n)>0), (n)->n_child[Length(n)-1])
#define firstchild(n) \
	(Assert(Is_Node(n) && Length(n)>0), (n)->n_child[0])
#else /* NDEBUG */
#define symbol(n) ((n)->n_symbol)
#define nchildren(n) (Length(n))
#define marks(n) ((n)->n_marks)
#define child(n, i) ((n)->n_child[(i)-1])
#define lastchild(n) ((n)->n_child[Length(n)-1])
#define firstchild(n) ((n)->n_child[0])
#endif /* NDEBUG */

int nodewidth();
#define marked(p, x) (marks(tree(p))&(x))

path newpath();
int pathlenght();

#define parent(p) ((p)->p_parent)
#define tree(p) ((p)->p_tree)
#define ichild(p) ((p)->p_ichild)

#define Ycoord(p) ((p)->p_ycoord)
#define Xcoord(p) ((p)->p_xcoord)
#define Level(p) ((p)->p_level)

/* Procedure markpath(); */
/* Procedure unmkpath(); */
/* Procedure treereplace(); */
bool up();
bool downi();

#define down(n) downi(n, 1)

bool downrite();
bool left();
bool rite();
/* Procedure top(); */
bool nextnode();
/* Procedure firstleaf(); */
bool nextleaf();
bool prevnode();
/* Procedure lastleaf(); */
bool prevleaf();
bool nextmarked();
bool prevmarked();

/*
 * The following are routines for lint, but macros for CC.
 * This way lint can detect wrong arguments passed.
 */

#ifdef lint

Visible node nodecopy();
Visible Procedure noderelease();
Visible Procedure nodeuniql();

Visible path pathcopy();
Visible Procedure pathrelease();
Visible Procedure pathuniql();

#else

#define nodecopy(n) ((node)copy((value)n))
#define noderelease(n) release((value) n)
#define nodeuniql(pn) uniql((value *)pn)

#define pathcopy(p) ((path)copy((value)p))
#define pathrelease(p) release((value)p)
#define pathuniql(pp) uniql((value *)pp)

#endif

node grab_node();
path grab_path();

