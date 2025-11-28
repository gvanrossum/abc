/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1987. */

/*
 * B editor -- Save focus position.
 */

#include "b.h"

#ifdef SAVEPOS

#include "bedi.h"
#include "bobj.h"
#include "bfil.h"
#include "node.h"
#include "supr.h"
#include "bmem.h"

/*
 * Keep a simple database of file name vs. line number.
 * The database is kept in most-recently-used-first order.
 */

typedef struct pc { char *fname; int line; struct pc *next; } poschain;
typedef poschain *pos;

#define PNULL ((pos) NULL)

Hidden pos poshead= PNULL;

Hidden bool poschanges;

Hidden pos new_pos(fname, line) char *fname; int line; {
	pos new= (pos) getmem((unsigned) sizeof(poschain));
	new->fname= (char *) savestr(fname);
	new->line= line;
	new->next= PNULL;
	return new;
}

Hidden Procedure free_pos(filpos) pos filpos; {
	freestr(filpos->fname);
	freemem((ptr) filpos);
}

Hidden int del_pos(fname) char *fname; {
	pos filpos= poshead;
	pos prev= PNULL;
	int line= 1;
	
	while (filpos != PNULL) {
		if (strcmp(fname, filpos->fname) == 0) {
			line= filpos->line;
			if (prev)
				prev->next= filpos->next;
			else
				poshead= filpos->next;
			free_pos(filpos);
			poschanges= Yes;
			break;
		}
		prev= filpos;
		filpos= filpos->next;
	}
	return line;
}

Hidden Procedure sav_pos(fname, line) char *fname; int line; {
	pos new;
	
	VOID del_pos(fname);
	new= new_pos(fname, line);
	new->next= poshead;
	poshead= new;
	poschanges= Yes;
}

Visible Procedure initpos() {
	FILE *file;
	char *buffer;
	char *fname;
	int line;
	pos tail, new;
	
	poshead= tail= PNULL;
	poschanges= No;
	file= fopen(posfile, "r");
	if (!file)
		return;
	while ((buffer= f_getline(file)) != NULL) {
		fname= (char *) getmem((unsigned) (strlen(buffer) + 1));

		if (sscanf(buffer, "%s\t%d", fname, &line) == 2) {
			if (F_exists(fname)) {
				new= new_pos(fname, line);
				if (!tail)
					poshead= tail= new;
				else {
					tail->next= new;
					tail= new;
				}
			}
		}
		freemem((ptr) fname);
		freemem((ptr) buffer);
	}
	fclose(file);
}

Hidden Procedure wripos() {
	FILE *fp;
	pos filpos;
	
	if (!poschanges)
		return;
	poschanges= No;
	if (poshead == PNULL) {
		unlink(posfile);
		return;
	}
	fp= fopen(posfile, "w");
	if (!fp)
		return;
	filpos= poshead;
	while (filpos != PNULL) {
		fprintf(fp, "%s\t%d\n", filpos->fname, filpos->line);
		filpos= filpos->next;
	}
	fclose(fp);
}

Visible Procedure endpos() {
	pos prev;

	wripos();
	while (poshead != PNULL) {
		prev= poshead;
		poshead= poshead->next;
		free_pos(prev);
	}
}

/* getpos() is called from editor */

Visible int getpos(fname) char *fname; {
	pos filpos= poshead;
	
	while (filpos != PNULL) {
		if (strcmp(fname, filpos->fname) == 0)
			return filpos->line;
		filpos= filpos->next;
	}
	return 0; /* editor expects 0 as default */
}

/* savpos() is called from editor */

Visible bool savpos(fname, ep) char *fname; environ *ep; {
	sav_pos(fname, lineno(ep) + 1);
}

/* delpos() is called from interpreter */

Visible Procedure delpos(fname) char *fname; {
	VOID del_pos(fname);
}

/* movpos() is called from interpreter */

Visible Procedure movpos(ofname, nfname) char *ofname, *nfname; {
	int n_line= del_pos(ofname);
	sav_pos(nfname, n_line);
}
	
#endif /* SAVEPOS */
