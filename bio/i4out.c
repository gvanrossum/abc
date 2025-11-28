/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1988. */

#include "b.h"
#include "bint.h"
#include "bmem.h"
#include "bobj.h"
#include "i3env.h"
#include "i3scr.h"
#include "i3sou.h"
#include "i4bio.h"

/* write_table_of_texts */

Visible Procedure abcoutput(name_arg) char *name_arg; {
	value name, pname;
	value *aa;
	value v;
	intlet k, len;
	
	name= mk_text(name_arg);
	if (!is_abcname(name)) {
		bioerrV(IO_NAME, name);
		release(name);
		return;
	}
	pname= permkey(name, Tar);
	if (!p_exists(pname, &aa)) {
		bioerrV(O_INIT, name);
		release(name);
		release(pname);
		return;
	}
	release(pname);
	v= gettarval(*aa, name);
	if (!still_ok) {
		release(name);
		release(v);
		return;
	}
	if (!Is_table(v)) {
		wri(stdout, v, No, Yes, No);
/*		bioerrV(O_TABLE, name);*/
		release(name);
		release(v);
		return;
	}
	at_nwl= Yes;
	len= length(v);
	for (k= 0; k<len && !Interrupted(); ++k) {
		wri(stdout, *assoc(v, k), No, Yes, No);
		putc('\n', stdout); /* no vtrm, so no CarriageReturn worries */
		at_nwl= Yes;
	}
	release(name);
	release(v);
}
