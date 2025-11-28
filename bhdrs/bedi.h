/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1988. */

#define CMDPROMPT ">>> " /* Prompt user for immediate command */

/* Types */

#define Nod 'N'
#define Pat 'P'
#define Etex 'E'
	/* text values in the kernel of the editor are stored
	 * according to the lineair model.
	 */

#define Is_Node(n) (Type(n) == Nod)
#define Is_Pat(p) (Type(p) == Pat)
#define Is_etext(v) (Type(v) == Etex)

typedef struct node *node;
typedef struct path *path;
typedef int markbits;

struct node {
	HEADER;
	markbits	n_marks;
	intlet	n_width;
	intlet	n_symbol;
	node	n_child[1];
};

#define Nnil ((node) Vnil)

#define NodOffset (sizeof(int) + 2*sizeof(intlet))

struct path {
	HEADER;
	path	p_parent;
	node	p_tree;
	intlet	p_ichild;
	intlet	p_ycoord;
	intlet	p_xcoord;
	intlet	p_level;
	markbits	p_addmarks;
	markbits	p_delmarks;
};

#define NilPath ((path) Vnil)


extern int doctype;	/* type of document edited by editdocument() */
#define D_perm 0	/* a how-to definition or permanent location */
#define D_input 1	/* input for READ or question */
#define D_immcmd 2	/* editing immediate command */
