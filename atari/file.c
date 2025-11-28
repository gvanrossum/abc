/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1990. */

/* get filenames for keysfile, messfile, etc, iff not set in getopt */

#include "b.h"
#include "main.h"  /* for curdir() */
#include "port.h"  /* for MESSFILE, HELPFILE, etc */

Forward Hidden Procedure setstartdir();
Forward Hidden Procedure setbwsdefault();
Forward Hidden Procedure setmessfile();
Forward Hidden Procedure sethelpfile();
Forward Hidden Procedure setkeysfile();
Forward Hidden Procedure setbuffile();
Forward Hidden char *savepath();

/* the directory delimiter in filename paths */
#define DELIM '\\'

#define ROOTDIR "\\"
#define PATH_SEPARATOR ','

Visible char *startdir;   /* the directory that ABC was started up in */
Visible char *bwsdefault; /* the workspaces parent directory */
Visible char *messfile;  /* the error messages file */
Visible char *helpfile;  /* the help file */
Visible char *keysfile;  /* the keys definition file */
Visible char *buffile;   /* the file for storing the copy buffer between sessions */

Visible Procedure initfile()
{
	setstartdir();     
	setbwsdefault();
	setmessfile();
	sethelpfile();
	setkeysfile();
	setbuffile();
}

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
		path = (char *) malloc(MALLOC_ARG (len+1));
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

Visible int Chdir (path) char *path; {
	return chdir(path);
}

Hidden char *savepath(path)
     char *path;
{
	char *path1;

	if (path == NULL) return (char *) NULL;
	path1 = (char *) malloc(MALLOC_ARG (strlen(path) + 1));
	strcpy(path1, path);
	return path1;
}

Visible Porting Procedure freepath(path)
     char *path;
{
	if (path != NULL) free(path);
}

Hidden char *searchfile(base)
     char *base;
{
	char *file;
	char *path;
	char *getenv();
	
	/* search first in startup directory */

	file = makepath(startdir, base);
	if (F_readable(file)) return file;
	freepath(file);

	/* then in bwsdefault directory */

	file = makepath(bwsdefault, base);
	if (F_readable(file)) return file;
	freepath(file);
		
	/* in PATH */

	path = getenv("PATH");
	for (;;) {
		char *end = strchr(path, PATH_SEPARATOR);
		char dir[64];
		int len = (end == NULL) ? strlen(path) : end-path;

		strncpy(dir, path, len);
		dir[len] = '\0';
		file = makepath(dir, base);
		if (F_readable(file)) return file;
		freepath(file);
		if (end == NULL)
			break;
		path = ++end;
	}

	/* else */

	return (char *) NULL;
}

Hidden char *getHOMEdir()
{
	return ROOTDIR;
}

Hidden Procedure setstartdir()
{
	startdir = savepath(curdir());
}

Hidden Procedure setbwsdefault()
{
	bwsdefault = makepath(getHOMEdir(), BWSNAME);
}

Hidden Procedure setmessfile()
{
	messfile = searchfile(MESSFILE);
	if (messfile == NULL)
	  messfile = makepath(ROOTDIR, MESSFILE);
}

Hidden Procedure sethelpfile()
{
	helpfile = searchfile(HELPFILE);
	if (helpfile == NULL)
	  helpfile = makepath(ROOTDIR, HELPFILE);
}

Hidden Procedure setkeysfile()
{
	char *termname;
	char *getenv();

	/* search first the file abc$TERM.key */

	if ((termname = getenv("TERM")) != NULL) {
		char termfile[100];
		sprintf(termfile, FORMAT_KEYSFILE, termname);
		keysfile = searchfile(termfile);
	}

	/* next the file abc.key */

	if (keysfile == NULL)
	  keysfile = searchfile(KEYSFILE);
	
	if (keysfile == NULL)
	  keysfile = makepath(ROOTDIR, KEYSFILE);
}

Hidden Procedure setbuffile()
{
	buffile = makepath(getHOMEdir(), BUFFILE);
}
