/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1988. */

/*
 * Allocate the arrays holding the tables.
 */

#include "b.h"
#include "main.h"

Visible Procedure allocate_tables() {
	
	symdef= (struct syminfo *)
		getmem((unsigned) maxsym*sizeof(struct syminfo));
	
	classdef= (struct classinfo *)
		getmem((unsigned) maxclass*sizeof(struct classinfo));
	
	lexdef= (struct lexinfo *)
		getmem((unsigned) maxlex*sizeof(struct lexinfo));

	namelist= (struct nameinfo *)
		getmem((unsigned) maxname*sizeof(struct nameinfo));
}
