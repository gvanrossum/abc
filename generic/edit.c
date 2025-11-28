/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1990. */

/* Generic version, as basis for porting.
 * See Portability Guide (./PORTING.DOC) for details.
 */

#include "b.h"
#include "main.h"  /* for filemodetime() */

#define COML 200
Hidden char com_line[COML];
#define At_eos(s) ((s)+= strlen(s))

Visible bool ed_file(editor, fname, line)
     char *editor;
     char *fname;
     int line;
{
	/* return Yes if file has been modified */
	char *cl = com_line;
	long oldtime;

	strcpy(cl, editor);
	At_eos(cl);
	if (*(cl - 1) == '+') {
		if (line > 0) {                   /* add linenumber */
			sprintf(cl, "%d", line);
			At_eos(cl);
		}
		else *(cl - 1) = ' ';             /* remove '+' */
	}
	*cl++ = ' ';
	strcpy(cl, fname);

	oldtime = filemodtime(fname);             /* save modtime file */
	system(com_line);                         /* call editor */
	if (oldtime == 0L)
	  /* can't get the file's modtime */
	  return Yes;
	else if (filemodtime(fname) != oldtime)   /* changed ! */
	  return Yes;
	else
	  return No;
}
