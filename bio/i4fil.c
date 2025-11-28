/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1988. */

#include "b.h"
#include "bfil.h"
#include "bmem.h"
#include "bobj.h"
#include "i3sou.h"
#include "port.h"
#include "dir.h"

/**************************************************************************/
/* get_names() is used to get at the names of all ABC files/workspaces	  */
/* in a given directory.						  */
/*									  */
/* This version of the file is supposed to work for any kind of Unix	  */
/* and for MS-DOS.							  */
/**************************************************************************/

	/* Note: it uses readdir so isn't portable to non-BSD
	   Unix, unless you also port readdir and friends.
	   Luckily, public-domain versions are available,
	   and one should be distributed with ABC.
	   It works for MS-DOS because I have ported readdir
	   to MS-DOS, too.  Guido. */

Visible value get_names(path, isabc) char *path; bool (*isabc)(); {
	DIR *dp;
	struct direct *dirp;
	value v;
	value name;
	
	dp= opendir(path);
	if (dp == (DIR *) NULL)
		return Vnil;
	v= mk_elt();
	for (;;) {
		dirp= readdir(dp);
		if (dirp == (struct direct *) NULL) {
			closedir(dp);
			break;
		}
		if ((*isabc)(path, dirp->d_name)) {
			name= mk_text(dirp->d_name);
			insert(name, &v);
			release(name);
		}
	}
	return v;
}

/**************************************************************************/
/* Is this the name of a target, a unit or something else?		  */
/*									  */
/* For compatibility, we recognize files starting with =, <, ", > and ',  */
/* and files ending with ".how", ".zer", ".mon", ".dya" and ".tar".	  */
/* Otherwise, unit names must end in ".cmd", ".zfd", ".mfd", ".dfd",	  */
/* ".zpd", ".mpd" or ".dpd",                                              */
/* and target names must end in ".cts" (all ignoring case).		  */
/**************************************************************************/

#define DumClass '\0'

/*Hidden*/ struct class { char *suffix; literal type; };

Hidden struct class classes[]= {
	{".cmd", Cmd},
	{".zfd", Zfd},
	{".mfd", Mfd},
	{".dfd", Dfd},
	{".zpd", Zpd},
	{".mpd", Mpd},
	{".dpd", Dpd},
	{".cts", Tar},
	
	{".CMD", Cmd},
	{".ZFD", Zfd},
	{".MFD", Mfd},
	{".DFD", Dfd},
	{".ZPD", Zpd},
	{".MPD", Mpd},
	{".DPD", Dpd},
	{".CTS", Tar},
	
	{".how", OldHow},
	{".zer", OldHow},
	{".mon", OldHow},
	{".dya", OldHow},
	{".tar", OldTar},

	{".HOW", OldHow},
	{".ZER", OldHow},
	{".MON", OldHow},
	{".DYA", OldHow},
	{".TAR", OldTar}
};

#define NCLASSES (sizeof classes / sizeof classes[0])

Hidden literal classfile(fname) value fname; {
	char *sfname, *end;
	struct class *cp;

	sfname= strval(fname);
	switch (sfname[0]) {
		case '\'': case '<': case '"': case '>':
			return OldHow;
		case '=':
			return OldTar;
		default:
			break;
	}
	end= sfname + strlen(sfname);
	for (cp= classes; cp < &classes[NCLASSES]; ++cp) {
		if (end-strlen(cp->suffix) >= sfname
		    && strcmp(end-strlen(cp->suffix), cp->suffix) == 0)
			return cp->type;
	}
	return DumClass;
}

/*ARGSUSED*/
Visible bool abcfile(path, name) char *path, *name; {
	/* path argument needed, but not used */
	bool isfile;
	value f= mk_text(name);
	
	isfile= classfile(f) != DumClass ? Yes : No;
	release(f);
	return isfile;
}

Visible bool abcworkspace(path, name) char *path, *name; {
	bool isws= No;
	
	if (is_directory(path, name)) {
		char *path1= makepath(path, name);
		char *path2= makepath(path1, permfile);
		isws= F_exists(path2) ? Yes : No;
		free_path(path2);
		free_path(path1);
	}
	return isws;
}

Visible bool targetfile(fname) value fname; {
	switch (classfile(fname)) {
		case Tar: case OldTar:
			return Yes;
		default:
			return No;
	}
}

Visible bool unitfile(fname) value fname; {
	switch (classfile(fname)) {
		case Tar: case OldTar: case DumClass:
			return No;
		default: 
			return Yes;
	}
}

Visible char *base_fname(fname) value fname; {
	char *sname;
	char *base;
	char *pext;
	
	sname= strval(fname);
	switch (*sname) {
		case '\'': case '<': case '"': case '>': case '=':
			++sname;
		default:
			break;
	}
	base= savestr(sname);
	if ((pext= strrchr(base, '.')) != NULL)
		*pext= '\0';
	return base;
}

Visible bool typeclash(pname, fname) value pname, fname; {
	return classfile(fname) != Permtype(pname) ? Yes : No;
}
