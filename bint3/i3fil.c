/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1986. */

/* Facilities supplied by the file system */

#include "b.h"
#include "bmem.h"
#include "bint.h"
#include "bobj.h"
#include "i2nod.h"
#include "i2par.h"
#include "i3scr.h"
#include "i3sou.h"

Forward Hidden bool fnm_extend();
Forward Hidden bool fnm_narrow();
Forward Hidden Procedure conv_fname();

/* f_rename() changes the name of a file via the system call rename();
 * rename() requires that both files are on the same file system, so
 * be careful when calling f_rename()
 */

Visible Procedure f_rename(fname, nfname)
     value fname, nfname;
{
	char *f1, f2[100];
	
	strcpy(f2, strval(nfname));
	unlink(f2);
	f1= strval(fname);
	VOID rename(f1, f2); 
	/* what if it fails??? */
}

Visible Procedure f_delete(file)
     char *file;
{
	unlink(file);
}

Visible unsigned f_size(file) FILE *file; {
	long s, ftell();
	fseek(file, 0l, 2);
	s= ftell(file);
	fseek(file, 0l, 0); /* rewind */
	return s;
}

Visible Procedure f_close(usrfile) FILE *usrfile; {
	bool ok= fflush(usrfile) != EOF;
	if (fclose(usrfile) == EOF || !ok)
		interr(MESS(3700, "write error (disk full?)"));
}

Visible bool f_interactive(file) FILE *file; {
	return isatty(fileno(file));
}

/* f_getline() returns a line from a file with the newline character */

#define LINESIZE 200

Visible char *f_getline(file) FILE *file; {
	char line[LINESIZE];
	char *pline= NULL;
	
	while (fgets(line, LINESIZE, file) != NULL) {
		if (pline == NULL)
			pline= (char *) savestr(line);
		else {
			int len= strlen(pline) + strlen(line) + 1;
			regetmem(&pline, (unsigned) len);
			strcat(pline, line);
		}
		if (strchr(pline, '\n') != NULL)
			return pline;
	}
	if (pline != NULL)
		freestr(pline);
	return NULL;
}

/*Hidden*/ struct class { literal type; char *suffix; };

Hidden struct class classes[]= {
	{Cmd, Cmd_ext},
	{Zfd, Zfd_ext},
	{Mfd, Mfd_ext},
	{Dfd, Dfd_ext},
	{Zpd, Zpd_ext},
	{Mpd, Mpd_ext},
	{Dpd, Dpd_ext},
	{Tar, Cts_ext},
	{Wsp, Wsp_ext}
};

#define NCLASSES (sizeof classes / sizeof classes[0])

Hidden char *filesuffix(type) literal type; {
	register struct class *cp;

	for (cp= classes; cp < &classes[NCLASSES]; ++cp) {
		if (type == cp->type)
			return cp->suffix;
	}
	return "";
}

/*
 * the following constants were moved here from all os.h's
 * to use more portable filenames;
 * e.g. MSDOS conventions, since these are the most limited.
 */
#define FNMLEN 8
#define SUFFIXLEN 4

Visible value new_fname(name, type) value name; literal type; {
	char fname[FNMLEN + SUFFIXLEN + 1];
	char *suffix= filesuffix(type);
	string sname= strval(name);
	char *sp= strchr(sname, ' ');
	intlet len= sp ? sp-sname : strlen(sname);
		/* if a command name only the first keyword */
	
	if (len > FNMLEN) len= FNMLEN;
	strncpy(fname, sname, len); fname[len]= '\0';
	strcat(fname, suffix);
	/* convert also if not MSDOS, to make abc-ws's portable: */
	conv_fname(fname, suffix);
	if (type != Wsp &&
		F_exists(fname) &&
		!fnm_extend(fname, len, suffix) &&
		!fnm_narrow(fname, len)
	   )
		return Vnil;
	return mk_text(fname);
}

Hidden bool fnm_extend(fname, n, suffix) char *fname, *suffix; int n; {
	/* e.g. "ABC.cmd" => "ABC1.cmd" */
	int m;
	int k= n;
	
	do {
		for (m= k-1; fname[m] == '9'; --m);
		if (isdigit(fname[m])) {
			++fname[m];
			while (++m < k) fname[m]= '0';
		}
		else if (k >= FNMLEN) {
			/* reset */
			fname[n]= '\0';
			strcat(fname, suffix);
			return No;
		}
		else {
			fname[++m]= '1';
			while (++m <= k) fname[m]= '0';
			fname[++k]= '\0';
			strcat(fname, suffix);
		}
	}
	while (F_exists(fname));
	return Yes;
}

Hidden bool fnm_narrow(fname, n) char *fname; int n; {
	/* e.g. "ABC.cmd" => "AB1.cmd" */
	int m;
	
	do {
		for (m= n-1; ; --m) {
			if (m < 1)
				return No;
			else if (!isdigit(fname[m])) { 
				fname[m]= '1'; 
				break; 
			}
			else if (fname[m] != '9') { 
				++fname[m]; 
				break; 
			}
			else fname[m]= '0';
		}
	}
	while (F_exists(fname));
	return Yes;
}

/* Conversion of characters:
 *  . uppercase to lowercase
 *  . point to CONVP_SIGN
 *  . double quote to CONVDQ_SIGN
 *  . single quote can stay
 *  the latter is as portably unspecial as possible.
 */

Hidden Procedure conv_fname(fname, suffix) char *fname, *suffix; {
	char *ext_point= fname + strlen(fname) - strlen(suffix);
	
	while (fname < ext_point) {
		if (isupper(*fname)) 
			*fname= tolower(*fname);
		else if (*fname == C_QUOTE)
			*fname= CONVDQ_SIGN;
		else if (*fname == C_POINT)
			*fname= CONVP_SIGN;
		fname++;
	}
}

/* recover location or workspace name from filename */

Visible value mkabcname(name) char *name; {
	char *p;
	
	for (p= name; *p != '\0'; ++p) {
		if (Cap(*p))
			*p= tolower(*p);
		else if (*p == CONVP_SIGN)
			*p= (*(p+1) == '\0' ? '\0' : C_POINT);
		else if (*p == CONVDQ_SIGN)
			*p= C_QUOTE;
		else if (!Tagmark(p))
			*p= C_QUOTE;
	}
	return mk_text(name);
}
