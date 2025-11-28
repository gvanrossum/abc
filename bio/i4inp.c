/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1988. */

#include "b.h"
#include "bmem.h"
#include "bobj.h"
#include "i4bio.h"

/* read_table_of_texts */

Visible Procedure abcinput(name_arg) char *name_arg; {
	value name;
	value tab, v;
	value n, m;
	bool eoi= No; /* end of input */
	
	name= mk_text(name_arg);
	if (!is_abcname(name)) {
		bioerrV(IO_NAME, name);
		release(name);
		return;
	}
	tab= mk_elt();
	n= zero;
	while (!eoi) {
		eoi= read_ioraw(&v);
		if (Valid(v)) {
			n= sum(m= n, one);
			replace(v, &tab, n);
			release(m); release(v);
		}
	}
	still_ok= Yes; interrupted= No;
	def_target(name, tab);
	release(n);
	release(name);
}
