/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1988. */

#include "b.h"
#include "bmem.h"  /* for getmem(), etc */
#include "file.h"  /* for ABCLIB */
#include "main.h"  /* for curdir() */
#include "port.h"  /* for MESSFILE, HELPFILE, etc */

extern char *getenv();

/* the directory delimiter in filename paths */
#define DELIM '/'

Visible char *startdir;   /* the directory that ABC was started up in */
Visible char *bwsdefault; /* the workspaces parent directory */
Visible char *messfile;  /* the error messages file */
Visible char *helpfile;  /* the help file */
Visible char *keysfile;  /* the keys definition file */
Visible char *buffile;   /* the file for storing the copy buffer between sessions */

Forward Hidden char *savepath();

Visible char *makepath(path1, path2)
     char *path1;
     char *path2;
{
	if (path1 == NULL || *path1 == '\0')
		return savepath(path2);
	else if (path2 == NULL || *path2 == '\0')
		return savepath(path1);
	else {
		char *path, *ppath;
		int len;
	
		len = strlen(path1) + 1 + strlen(path2);
		path = (char *) getmem((unsigned)(len+1));
		strcpy(path, path1);
		ppath = path + strlen(path);
		if (ppath[-1] == DELIM && *path2 == DELIM)
			--ppath;
		else if (ppath[-1] != DELIM && *path2 != DELIM)
			*ppath++= DELIM;
		strcpy(ppath, path2);
		return path;
	}
}

Hidden char *savepath(path)
     char *path;
{
	if (path == NULL) return (char *) NULL;
	return savestr(path);
}

Visible Porting Procedure freepath(path)
     char *path;
{
	if (path != NULL) freestr(path);
}

Hidden string searchfile(base, abclib) char *base, *abclib; {
	char *file;
	
	/* search first in startup directory */
	file= makepath(startdir, base);
	if (F_readable(file))
		return file;
	freepath(file);

	/* then in bwsdefault */
	if (bwsdefault != NULL) {
		file= makepath(bwsdefault, base);
		if (F_readable(file))
			return file;
		freepath(file);
	}
		
	/* next first in abclib */
	file= makepath(abclib, base);
	if (F_readable(file))
		return file;
	freepath(file);

	/* else fail */
	return (string) NULL;
}

Visible Procedure initfile()
{
	char *homedir= getenv("HOME");
	char *termname;
	char *termfile;
	
	startdir= savepath(curdir());
	bwsdefault= homedir ? makepath(homedir, BWSNAME) : (char *) NULL;
	messfile= searchfile(MESSFILE, ABCLIB);
	helpfile= searchfile(HELPFILE, ABCLIB);
	buffile= homedir ? makepath(homedir, BUFFILE) : savepath(BUFFILE);

	if ((termname= getenv("TERM")) != NULL) {
		termfile= (char *) getmem((unsigned) strlen(FORMAT_KEYSFILE) +
					  strlen(termname));
		sprintf(termfile, FORMAT_KEYSFILE, termname);
		keysfile= searchfile(termfile, ABCLIB);
		freemem((ptr) termfile);
	}
	if (keysfile == (string)NULL) {
		keysfile= searchfile(KEYSFILE, ABCLIB);
	}
	/* for compatability with Release 1.02.01 : */
#define KEYSPREFIX "abckeys_"
	if (keysfile == NULL && (termname= getenv("TERM")) != NULL) {
		termfile= (char *) getmem((unsigned) strlen(KEYSPREFIX) +
					  strlen(termname) + 1);
		strcpy(termfile, KEYSPREFIX);
		strcat(termfile, termname);
		keysfile= searchfile(termfile, ABCLIB);
		freemem((ptr) termfile);
	}
	
}

Visible int Chdir (path) char *path; {
	return chdir(path);
}
