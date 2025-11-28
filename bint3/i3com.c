/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1986. */

#include "b.h"
#include "bmem.h"
#include "bobj.h"
#include "bcom.h"
#include "i3scr.h"
#include "port.h"

/****************************************************************************/

/* Edit a file.  Parameters are the file name (as ABC value), a line
   number where the focus should appear (may be 0 if not applicable; may
   be ignored by some editors), and the kind of file: 't' for targets,
   'u' for units.
   Returns a bool: Yes if the file has been modified.
*/

Visible bool f_edit(fname, errline, kind, creating)
     value fname;
     intlet errline;
     literal kind;
     bool creating;
{
	string filename = sstrval(fname);
	bool changed;

	if (OPTeditor != (char *) NULL) { /* another editor */
		changed = ed_file(OPTeditor, (char *) filename, (int) errline);
		re_interpreter_output();
	}
	else
		abced_file(filename, errline, kind, creating, &changed);
#ifdef macintosh
	sync();
#endif
	fstrval(filename);
	still_ok= Yes; /* ignore interrupts that occurred */

	return changed;
}

/****************************************************************************/

Visible bool cmdline(kind, bp, indent) literal kind; bufadm *bp; int indent; {
	static char *edfirst= NULL;
	static char *edbuf;
	char *ed_line();
	char *edlast;

	for (;;) {
		if (edfirst == NULL) {
			if (kind == R_cmd && outeractive) {
				oline();
				at_nwl= No;
			}
			edbuf= ed_line(kind, indent);
			if (edbuf == NULL) { /* editor interrupted */
				still_ok= No;
				return No;
			}
			edfirst= edbuf;
		}
		if (*edfirst == '\0') { /* at the end of edbuf */
			edfirst= NULL;
			freemem((ptr) edbuf);
			if (kind != R_cmd)
				continue;
			bufcpy(bp, "\n");
			return No;
		}
		edlast= strchr(edfirst, '\n');
		if (edlast == NULL)
			syserr(MESS(4500, "in cmdline()"));
		bufncpy(bp, edfirst, edlast - edfirst + 1);
		edfirst= ++edlast;
#ifdef MEMTRACE
		if (strcmp(edbuf, "QUIT\n") == 0)
			freemem(edbuf);
#endif
		return Yes;
	}
}

/* delete file from positions file */

Visible Procedure idelpos(fname) value fname; {
	string file= sstrval(fname);
	delpos(file);
	fstrval(file);
}	

/* move position in positions file */

Visible Procedure imovpos(ofname, nfname) value ofname, nfname; {
	string o_file= sstrval(ofname);
	string n_file= sstrval(nfname);
	movpos(o_file, n_file);
	fstrval(o_file);
	fstrval(n_file);
}
