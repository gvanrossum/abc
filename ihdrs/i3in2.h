/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1986. */

/* Interpreter utilities */

value v_local();
value v_global();
loc local_loc();
loc global_loc();
loc trim_loc();
loc tbsel_loc();
value pre_fun();
extern value resval;

value evalthread();

#define Changed_formal(v) (v == Vnil || !Is_indirect(v))

value locvalue();
