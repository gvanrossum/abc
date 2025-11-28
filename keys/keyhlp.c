/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1989. */

/*
 * ABC keys -- Print the bindings.
 */

#include "b.h"
#include "bmem.h"
#include "oper.h"
#include "getc.h"

Forward Hidden string getname();
Forward Hidden Procedure extendwithspaces();
Forward Hidden Procedure getbindings();
Forward Hidden bool addbinding();

/*
   The following array determines the order of the editor operations
   in the helpblurb.
   The names and keyrepresentations are taken from deftab in e1getc.c
   and ?1keys.c.
   Printing is done in two columns.
   Code NOTHING is used to produce an empty place in the second column.
 */
 
int helpcode[]= {
	WIDEN,		EXTEND,
	FIRST,		LAST,
	PREVIOUS,	NEXT,
	UPLINE,		DOWNLINE,
	UPARROW,	DOWNARROW,
	LEFTARROW,	RITEARROW,
#ifdef GOTOCURSOR
	GOTO,		MOUSE,
#endif
	ACCEPT,		NEWLINE,
	UNDO,		REDO,
	COPY,		DELETE,
	RECORD,		PLAYBACK,
	LOOK,		HELP,
	EXIT,		NOTHING,
	CANCEL,		SUSPEND,
	TERMINIT,	TERMDONE,
	IGNORE,		NOTHING
};

Hidden struct helpitem {
	string data;	/* "[name] repr's string" */
	int bindmark;	/* position in data of more bindings marker */
	bool changed;	/* status of item */
} helpitem[(sizeof(helpcode))/(sizeof(int))];

Hidden int nitems= 0;

Hidden int namewidth;		/* width of name field */
#define GAP_FIELDS 1		/* nr of spaces between two fields */
/*Hidden int bindwidth;*/	/* width of bindings field */

Hidden int helpwidth;		/* width of a column */
#define GAP_COLUMNS 1		/* nr of spaces between the two columns */

#define BINDMARK '*'		/* set after name if too many bindings */
Hidden int bindstart;		/* offset bindings field */
#define BINDSEP ", "		/* separator bindings */

/*
 * Print the bindings.
 */

Visible Procedure putbindings(yfirst) int yfirst; {
	int h;
	bool h_changed;
	
	for (h= 0; h < nitems; h+= 2, yfirst++) {

		if (h_changed= helpitem[h].changed) {
			getbindings(h);
			trmputdata(yfirst, yfirst, 0, helpitem[h].data, (string)0);
		}
		if (h+1 < nitems) {
			if (helpitem[h+1].changed)
				getbindings(h+1);
			else if (!h_changed)
				continue;
			trmputdata(yfirst, yfirst,
			   helpwidth+GAP_COLUMNS, helpitem[h+1].data, (string)0);
		}
	}
	trmsync(yfirst, 0);
}

Visible Procedure setup_bindings(width, nlines) int width, *nlines; {
	int h;
	int code;
	int len;
	string buffer;
	string name;
	string getname();

	helpwidth= (width - GAP_COLUMNS)/2;
	nitems= ((sizeof(helpcode))/(sizeof(int)));
	namewidth= 0;

	for (h= 0; h < nitems; h++) {
		buffer= (string) getmem((unsigned) helpwidth+1);
		code= helpcode[h];
		name= getname(code);
		strcpy(buffer, name);
		len= strlen(buffer);
		if (len > namewidth) /* find max name length */
			namewidth= len;
		helpitem[h].data= buffer;
		helpitem[h].bindmark= len;
		helpitem[h].changed= Yes;
		confirm_operation(code, name);
	}

	namewidth++;
		/* one extra space for a marker after the name
		 * if there are too many bindings to show
		 */
	bindstart= namewidth + GAP_FIELDS;
/*	bindwidth= helpwidth - bindstart; */

	/* extend with spaces */
	for (h= 0; h < nitems; h++)
		extendwithspaces(helpitem[h].data, bindstart);
	
	/* set nlines */

	*nlines= (nitems+1)/2;
}

#ifdef MEMTRACE

Visible Procedure fini_bindings() {
	int h;
	
	for (h= 0; h < nitems; h++) {
		free(helpitem[h].data);
	}
}

#endif /* MEMTRACE */

Hidden string getname(code) int code; {
	tabent *d;
	
	for (d= deftab; d < deftab+ndefs; d++) {
		if (code == d->code)
			return d->name;
	}
	return "";
}

Hidden Procedure extendwithspaces(buffer, bound) string buffer; int bound; {
	int len= strlen(buffer);
	string pbuf= buffer+len;

	for (; len < bound; len++)
		*pbuf++= ' ';
	*pbuf= '\0';
}

Visible Procedure bind_changed(code) int code; {
	int h;
	
	for (h= 0; h < nitems; h++) {
		if (code == helpcode[h]) {
			helpitem[h].changed= Yes;
			break;
		}
	}
}

Visible Procedure bind_all_changed() { /* for redrawing the screen */
	int h;
	
	for (h= 0; h < nitems; h++) {
		helpitem[h].changed= Yes;
	}
}
	

#define Def(d)	((d)->def != NULL)
#define Rep(d)	((d)->rep != NULL && (d)->rep[0] != '\0')

Hidden Procedure getbindings(h) int h; {
	tabent *d;
	int code= helpcode[h];
	string buffer= helpitem[h].data;
	bool all_showed= Yes;
	
	buffer[bindstart]= '\0';
	for (d= deftab+ndefs-1; d >= deftab; d--) {

		if (code != d->code || !Def(d) || !Rep(d))
			continue;
		if (!addbinding(d->rep, buffer))
			all_showed= No;
	}
	/* set marker */
	buffer[helpitem[h].bindmark]= !all_showed ? BINDMARK : ' ';

	helpitem[h].changed= No;
}

Hidden bool addbinding(repr, buffer) string repr, buffer; {
	string sep= buffer[bindstart] == '\0' ? "" : BINDSEP;
	
	if (strlen(buffer) + strlen(sep) + strlen(repr) > helpwidth)
		return No;
	strcat(buffer, sep);
	strcat(buffer, repr);
	return Yes;
}
