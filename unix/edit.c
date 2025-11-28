/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1990. */

#include "b.h"
#include "main.h" /* for filemodtime() */

#define COML 200
Hidden char com_line[COML];
#define At_eos(s) ((s)+= strlen(s))

Forward Hidden Procedure app_fname();

Visible bool ed_file(editor, fname, line)
     char *editor;
     char *fname;
     int line;
{
	/* return Yes if file has been modified */
	string cl= com_line;
	long oldtime;

	strcpy(cl, editor);
	if (*(cl+strlen(cl)-1) == '+') {
		if (line != 0) {At_eos(cl); sprintf(cl, "%d", line);}
		else *(cl+strlen(cl)-1)= ' ';
	}
	At_eos(cl);
	app_fname(cl, fname);

	oldtime= filemodtime(fname);
	system(com_line);
	if (oldtime == 0L)
	  /* can't get the file's modtime */
	  return Yes;
	else if (filemodtime(fname) != oldtime)   /* changed ! */
	  return Yes;
	else
	  return No;
}

Hidden Procedure app_fname(ceos, fname)
     string ceos;
     string fname;
{
	intlet k, len= strlen(fname);
	*ceos++= ' ';
	for (k= 0; k<len; ++k) {
		*ceos++= '\\';
		*ceos++= *fname++;
	}
	*ceos= '\0';
}
