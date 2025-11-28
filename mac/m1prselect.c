/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1988. */

/* Select() routines: select files for printing */

#include "b.h"
#include "bmem.h"
#include "bobj.h"
#include "bint.h"
#include "i3env.h"
#include "i3scr.h"
#include "i3sou.h"
#include "macabc.h"

Visible char **prfile= (char**)NULL;	/* array of filenames to print */
Visible int nprfile= 0;	/* number of same */

Visible bool init_slct(nfile) int nfile; {
	prfile= (char**) getmem((unsigned) (nfile*sizeof(char*)));
	if (prfile == (char**)NULL) {
		macerr(NOMEM);
		return No;
	}
	nprfile= nfile;
	for (--nfile; nfile>=0; nfile--) {
		prfile[nfile]= (char*)NULL;
	}
	return Yes;
}

Visible Procedure end_slct() {
	if ((char*)prfile == (char*)NULL)
		return;
	for (--nprfile; nprfile >= 0; --nprfile) {
		if (prfile[nprfile] != (char*)NULL)
			freemem((ptr) (prfile[nprfile]));
	}
	free((char*) prfile);
}

/* write_table_of_texts */

Hidden bool dumpval(locname) char *locname; {
	value name, pname;
	value *aa;
	value v;
	
	name= mk_text(locname);
	pname= permkey(name, Tar);
	if (!p_exists(pname, &aa)) {
		release(name);
		return No;
	}
	release(pname);
	v= getval(*aa, In_tarval);
	if (!still_ok) {
		release(name);
		release(v);
		return No;
	}
	at_nwl= Yes;
	wri(v, No, No, No);
	newline();
	release(name);
	release(v);
	return Yes;
}

extern bool prtempfile;

Visible bool slct_location() {
	char locname[MAXNAME];
	FILE *ofp;
	bool r;
	
	if (!init_slct(1))
		return No;
	strcpy(locname, "location.name");
	if (asklocation(locname) == NULL)
		return No;
	ofp= fopen(PRTEMPFILE, "w");
	if (ofp != NULL) {
		redirect(ofp);
		r= dumpval(locname);
		redirect(stdout);
		VOID fclose(ofp);
		prtempfile= Yes;
	}
	else r= No;
	prfile[0]= savestr(PRTEMPFILE);
	if (!r)
		macerr(FAILPRTEMP);
	return r;
}

Visible bool slct_howto() {
	char howname[MAXNAME];
	value name;
	value pname;
	value *aa;
	literal type;
	int nf;
	
	if (!init_slct(2))		/* there can be 2 how-to's with the same username! */
		return No;
	strcpy(howname, "HOWTO.NAME");
	if (askhowto(howname) == NULL)
		return No;
	name= mk_text(howname);
	nf= 0;
	for (type=Cmd; type <= Dpd; type++) {	/* hack, hack; using order */
		pname= permkey(name, type);
		if (p_exists(pname, &aa)) {
			prfile[nf]= sstrval(*aa);
			nf++;
		}
		release(pname);
	}
	release(name);
	nprfile= nf;
	return (bool) (nf > 0);
}

Visible bool slct_workspace() {
	intlet k, len;
	value fname;
	int nf;
	
	len= length(b_perm);
	if (!init_slct((int) len))
		return No;
	nf= 0;
	for (k= 0; k<len; ++k) {
		fname= *assoc(b_perm, k);
		if (!Is_text(fname) || !unitfile(fname))
			continue;
		prfile[nf]= sstrval(fname);
		nf++;
	}
	nprfile= nf;
	return (bool) (nf > 0);
}

Visible bool slct_appfiles() {
	int n;

	for (n= 0; n <nappfiles; n++) {
		prfile[n]= appfiles[n];
	}
	nprfile= nappfiles;
	return Yes;
}
