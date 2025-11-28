/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1986. */

#define Is_node(t) ((t) != NilTree && Is_parsetree(t))

extern int nextvarnumber; /* Counts local targets (including formals) */
extern value locals, globals, mysteries, refinements;
extern string gentab[];

struct state {
	parsetree h_last;
	parsetree *h_wanthere;
	parsetree h_bpchain;
};

#define f_expr(p) fix(p, 'v') /* "evaluate" */
#define f_targ(p) fix(p, 'l') /* "locate" */

Visible value copystddef();
Visible bool modify_tag();
Visible bool is_name();
