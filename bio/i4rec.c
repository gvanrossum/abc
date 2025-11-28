/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1988. */

#include "b.h"
#include "bint.h"
#include "bfil.h"
#include "bmem.h"
#include "bobj.h"
#include "i2nod.h"
#include "i2par.h"
#include "i3bws.h"
#include "i3cen.h"
#include "i3scr.h"
#include "i3sou.h"
#include "i4bio.h"
#include "port.h"

Forward Hidden Procedure rec_target();
Forward Hidden Procedure rec_unit();
Forward Hidden Procedure mk_permentry();
Forward Hidden Procedure mk_suggitem();
Forward Hidden Procedure rec_current();
Forward Hidden Procedure recperm();
Forward Hidden Procedure recsugg();
Forward Hidden Procedure recpos();
Forward Hidden Procedure recerrV();
Forward Hidden Procedure cantwrite();

/*
 * Code to recover the contents of an ABC workspace.
 *
 * It constructs two completely new files:
 * 	perm.abc for the permanent environment, and
 *	suggest.abc for the user suggestions.
 * Files with an extension of ".cts" or ".CTS" are taken to be targets;
 * all others are assumed to contain units (if they contain garbage,
 * they are ignored).
 * For units, the name, type and adicity are extracted from the source;
 * for targets, the target name is either taken from the old perm.abc or
 * taken to be the file name with all illegal characters converted to double
 * quote (") and uppercase to lowercase.
 *
 * BUGS:
 * - target names can get truncated when the original target name was longer
 *   than what fits in a legal file name.
 */

Visible bool ws_recovered= No;
Hidden bool rec_ok= Yes;

Hidden value old_perm;
Hidden value sugglis;

Visible Procedure rec_workspace() {
	value lis, fname;
	value k, len, m;

	ws_recovered= No;
	rec_ok= Yes;
	
	old_perm= copy(cur_env->perm);
	endworkspace();

	cur_env->perm= mk_elt();
	sugglis= mk_elt();
	
	lis= get_names(cur_dir, abcfile);
	k= one; len= size(lis);
	while (numcomp(k, len) <= 0) {
		fname= item(lis, k);
		if (targetfile(fname))
			rec_target(fname);
		else if (unitfile(fname))
			rec_unit(fname, Yes);
		release(fname);
		k= sum(m= k, one);
		release(m);
	}
	release(k); release(len);
	release(lis);

	rec_current(last_unit);
	rec_current(last_target);
	
	recperm();
	recsugg();
	recpos();
#ifdef TYPE_CHECK
	rectypes();
#endif

	release(cur_env->perm); cur_env->perm = Vnil;
	release(sugglis);
	release(old_perm);
	
	initworkspace();
	if (!still_ok)
		return;
		
	ws_recovered= Yes;
}

Visible Procedure rec_suggestions() {
	value lis, fname;
	value k, len, m;

	rec_ok= No;
	
	endworkspace();

	sugglis= mk_elt();
	
	lis= get_names(cur_dir, abcfile);
	k= one; len= size(lis);
	while (numcomp(k, len) <= 0) {
		fname= item(lis, k);
		if (unitfile(fname))
			rec_unit(fname, No);
		release(fname);
		k= sum(m= k, one);
		release(m);
	}
	release(k); release(len);
	release(lis);

	recsugg();

	release(sugglis);
	
	initworkspace();
}

Hidden Procedure rec_target(fname) value fname; {
	value pname;
	value name;
	intlet k, len;

	/* try to find a name via the old perm table */
	name= Vnil;
	len= Valid(old_perm) ? length(old_perm) : 0;
	for (k= 0; k<len; ++k) {
		if (compare(*assoc(old_perm, k), fname) == 0) {
			name= Permname(*key(old_perm, k));
			if (is_abcname(name))
				break;
			release(name); name= Vnil;
		}
	}
	if (!Valid(name)) { /* make a new name */
		char *base= base_fname(fname);
		name= mkabcname(base);
		freestr(base);
	}
	if (!is_abcname(name)) {
		recerrV(R_TNAME, fname);
		release(name);
		return;
	}
	pname= permkey(name, Tar);
	mk_permentry(pname, fname);
	release(pname);
	release(name);
}

Hidden Procedure rec_unit(fname, all) value fname; bool all; {
	FILE *fp;
	char *line;
	value pname;
	parsetree u;
	txptr tx_save;

	fp= fopen(strval(fname), "r");
	if (fp == NULL) {
		recerrV(R_FREAD, fname);
		return;
	}
	line= f_getline(fp);
	fclose(fp);
	if (line == NULL) {
		recerrV(R_UNAME, fname);
		return;
	}
	tx_save= tx;
	tx= line;
	findceol();
	
	mess_ok= No; /* do it silently */
	u= unit(Yes, No);
	still_ok= Yes;
	mess_ok= Yes;
	tx= tx_save;

	pname= u == NilTree ? Vnil : get_pname(u);
	if (Valid(pname)) {
		if (all) mk_permentry(pname, fname);
		mk_suggitem(u);
	}
	else recerrV(R_UNAME, fname);
	freestr(line);
	release(pname);
	release((value) u);
}

Hidden Procedure mk_permentry(pname, fname) value pname, fname; {
	value fn;
	
	if (in_keys(pname, cur_env->perm)) {
		recerrV(R_EXIST, fname);
		return;
	}
	if (!typeclash(pname, fname))
		fn= copy(fname);
	else {
		value name= Permname(pname);
		literal type= Permtype(pname);
		
		fn= new_fname(name, type);
		if (Valid(fn))
			f_rename(fname, fn);
		else
			recerrV(R_RENAME, fname);
		release(name);
		
	}
	if (Valid(fn))
		replace(fn, &(cur_env->perm), pname);
	release(fn);
}

Hidden Procedure mk_suggitem(u) parsetree u; {
	value formals, k, t, next, v;
	value sugg, sp_hole, sp;
	
	switch (Nodetype(u)) {
	case HOW_TO:
		sugg= mk_text("");
		sp_hole= mk_text(" ?");
		sp= mk_text(" ");
		formals= *Branch(u, HOW_FORMALS);
		while (Valid(formals)) {
			k= *Branch(formals, FML_KEYW);
			t= *Branch(formals, FML_TAG);
			next= *Branch(formals, FML_NEXT);
			sugg= concat(v= sugg, k);
			release(v);
			if (Valid(t)) {
				sugg= concat(v= sugg, sp_hole);
				release(v);
			}
			if (Valid(next)) {
				sugg= concat(v= sugg, sp);
				release(v);
			}
			formals= next;
		}
		release(sp_hole);
		release(sp);
		break;
	case YIELD:
	case TEST:
		sugg= copy(*Branch(u, UNIT_NAME));
		break;
	default:
		return;
	}
	insert(sugg, &sugglis);
	release(sugg);
}

Hidden Procedure rec_current(curr) value curr; {
	value *pn;
	
	if (in_keys(curr, old_perm)
	    && Valid(*(pn= adrassoc(old_perm, curr)))
	    && in_keys(*pn, cur_env->perm))
	{
		replace(*pn, &(cur_env->perm), curr);
	}
}

Hidden Procedure recperm() {
	cur_env->permchanges= Yes;
	put_perm();
}

Hidden Procedure recsugg() {
	FILE *fp;
	value k, len, m;
	value sugg;
	
	len= size(sugglis);
	if (numcomp(len, zero) <= 0) {
		unlink(suggfile);
		release(len);
		return;
	}
	fp= fopen(suggfile, "w");
	if (fp == NULL) {
		cantwrite(suggfile);
		release(len);
		return;
	}
	k= one;
	while (numcomp(k, len) <= 0) {
		sugg= item(sugglis, k);
		fprintf(fp, "%s\n", strval(sugg));
		release(sugg);
		k= sum(m= k, one);
		release(m);
	}
	fclose(fp);
	release(k); release(len);
}

Hidden Procedure recpos() {
	/* to be done */
	/* since the number of filenames remembered is limited
	 * any filenames disappeared in recovering will
	 * eventually disappear, however.
	 */
}


Hidden Procedure recerrV(m, v) int m; value v; {
	if (rec_ok) {
		bioerr(R_ERROR);
		rec_ok= No;
	}
	bioerrV(m, v);
}

Hidden Procedure cantwrite(file) string file; {
	value fn= mk_text(file);
	bioerrV(R_FWRITE, fn);
	release(fn);
}
