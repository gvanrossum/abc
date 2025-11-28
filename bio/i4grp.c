/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1988. */

#include "b.h"
#include "bfil.h"
#include "bobj.h"
#include "i3bws.h"	
#include "i3sou.h"
#include "i4bio.h"

Forward Hidden Procedure rec_dirname();
Forward Hidden bool is_wsname();
Forward Hidden Procedure mk_groupentry();
Forward Hidden Procedure rec_curlast();
Forward Hidden Procedure grperrV();

/* code to recover the index of a group of workspaces */

Visible bool gr_recovered= No;

Hidden bool rec_ok= Yes;

Hidden value newgroup;

Visible Procedure rec_wsgroup() {
	value lis, dname;
	value k, len, m;

	gr_recovered= No;
	rec_ok= Yes;
	
	newgroup= mk_elt();
	
	lis= get_names(bwsdir, abcworkspace);
	k= one; len= size(lis);
	while (numcomp(k, len) <= 0) {
		dname= item(lis, k);
		rec_dirname(dname);
		release(dname);
		k= sum(m= k, one);
		release(m);
	}
	release(k); release(len);
	release(lis);

	rec_curlast();
	
	release(ws_group);
	ws_group= newgroup;
	groupchanges= Yes;
	
	gr_recovered= Yes;
}

Hidden Procedure rec_dirname(dname) value dname; {
	value name;
	intlet k, len;
	
	/* try to find a name via the old index table */
	name= Vnil;
	len= Valid(ws_group) ? length(ws_group) : 0;
	for (k= 0; k<len; ++k) {
		if (compare(*assoc(ws_group, k), dname) == 0) {
			name= copy(*key(ws_group, k));
			if (is_wsname(name))
				break;
			release(name); name= Vnil;
		}
	}
	if (!Valid(name)) { /* make a new name */
		char *base= sstrval(dname);
		name= mkabcname(base);
		fstrval(base);
	}
	if (!is_abcname(name)) {
		grperrV(G_DNAME, dname);
		release(name);
		return;
	}
	mk_groupentry(name, dname);
	release(name);
}

Hidden bool is_wsname(name) value name; {
	if (!is_abcname(name))
		return No;
	if (compare(name, curwskey) == 0 || compare(name, lastwskey) == 0)
		return No;
	return Yes;
}

Hidden Procedure mk_groupentry(name, dname) value name, dname; {
	if (in_keys(name, newgroup)) {
		grperrV(G_EXIST, dname);
		return;
	}
	replace(dname, &newgroup, name);
}

Hidden Procedure rec_curlast() {
	value *aa;
	if (!Valid(ws_group))
		return;
	if (in_env(ws_group, curwskey, &aa))
		replace(*aa, &newgroup, curwskey);
	if (in_env(ws_group, lastwskey, &aa))
		replace(*aa, &newgroup, lastwskey);
}

Hidden Procedure grperrV(m, v) int m; value v; {
	if (rec_ok) {
		bioerr(G_ERROR);
		rec_ok= No;
	}
	bioerrV(m, v);
}
