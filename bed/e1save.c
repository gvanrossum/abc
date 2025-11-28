/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1986. */

/*
 * B editor -- Save Parse tree on file.
 */

#include "b.h"
#include "b0lan.h"
#include "bedi.h"
#include "etex.h"
#include "bmem.h"
#include "bobj.h"
#include "node.h"
#include "gram.h"

Hidden int spaces = 0; /* Saved-up spaces; emitted when non-blank found */

Forward Hidden Procedure sendsave();

/*
 * Write the representation of a node.	If it has children,
 * they are written by recursive calls.
 */

Hidden Procedure savewalk(n, level, bp, fp) node n; int level;
		bufadm *bp; FILE *fp; {
	string *rp;
	string cp;
	int nch;
	int i;
	char c;

	if (Is_etext(n)) {
		for (; spaces > 0; --spaces)
			bufpush(bp, ' ');
		bufcpy(bp, e_strval((value) n));
		return;
	}
	nch = nchildren(n);
	rp = noderepr(n);
	for (i = 0; i <= nch; ++i) {
		if (i)
			savewalk(child(n, i), level, bp, fp);
		cp = rp[i];
		if (!cp) continue;
		for (; c = *cp; ++cp) {
			switch (c) {

			case '\n':
#if '\n' != '\r' /* This condition is TRUE in MPW C! */
			case '\r':
#endif
				bufpush(bp, '\n');
				if (fp) {
					bufpush(bp, '\0');
					fputs(bp->buf, fp);
					bufreinit(bp);
				}
				if (c == '\n')
					for (i = level; i > 0; --i)
						bufcpy(bp, Indent);
				spaces = 0;
				break;

			case '\b':
				--level;
				break;

			case '\t':
				++level;
				break;

			case ' ':
				++spaces;
				break;

			default:
				for (; spaces > 0; --spaces)
					bufpush(bp, ' ');
				bufpush(bp, c);
				break;
			}
		}
	}
}

Visible bool save(p, filename) path p; string filename; {
	bufadm b, *bp;
	FILE *fp = fopen(filename, "w");
	
	if (!fp) return No;
	bp= &b;
	bufinit(bp);
	sendsave(p, bp, fp);
	bufpush(bp, '\0');
	fputs(bp->buf, fp);
	buffree(bp);
	return fclose(fp) != EOF;
}


Hidden Procedure sendsave(p, bp, fp) path p; bufadm *bp; FILE *fp; {
	p = pathcopy(p);
	top(&p);
	spaces = 0;
	savewalk(tree(p), 0, bp, fp);
	bufpush(bp, '\n');
	pathrelease(p);
}

/*
 * Interface to top level.
 */

Visible char *senddoc(p) path p; {
	char *res;
	bufadm b, *bp;
	
	bp= &b;
	bufinit(bp);
	sendsave(p, bp, (FILE *)NULL);
	bufpush(bp, '\0');
	res= savestr(bp->buf);
	buffree(bp);
	return res;
}

