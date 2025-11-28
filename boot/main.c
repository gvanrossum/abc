/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1988. */

/*
 * mktables -- Program to create tables for ABC editor from grammar
 *
 * mktables [-s maxsym] [-c maxclass] [-l maxlex] [grammar-file]
 */

#include "b.h"
#include "main.h"

Visible string progname;

FILE *gfp;		/* grammar file */
char *gfile= GRAMMAR;
FILE *tfp;		/* data file for grammar tables */
char *tfile= TABLES;
FILE *ifp;		/* include file for grammar table structure */
char *ifile= INCLUDE;	/*  and Symbol to index-in-table mapping */
char *hfile=HFILE;	/* ultimate include-file name to reference
			 * in datafile */

Visible struct classinfo *classdef;	/* class definitions */
Visible struct syminfo *symdef;		/* Symbol definitions */
Visible struct lexinfo *lexdef;		/* LEXICAL definitions */
Visible struct nameinfo *namelist;	/* class, Symbol or LEXICAL names */

int maxclass= MAXCLASS; /* max number of class definitions in grammar */
int maxsym= MAXSYM;	/* max number of Symbol definitions in grammar */
int maxlex= MAXLEX;	/* max number of LEXICAL definitions grammar */
int maxname= MAXNAME;	/* max number of names (Symbols, classes or LEXICALS) */

int nclass= 0;		/* actual number of definitions in grammar */
int nsym= 0;
int nlex= 0;
int nname= 0;

int lsuggestion= -1; /* index in lexdef[] of definition for SUGGESTION */
		/* also used as bool to check presence of definition */
int lsugghowname= -1; /* idem for SUGGHOWNAME */
int nsuggstnbody;/* index in classdef[] of enveloped definition of SUGGESTION */
int nsugghowbody; /* idem ... */

int nsuggestion;/* index in symdef[] of symboldefinition for Suggestion */
int nsugghowname;/* index in symdef[] of symboldefinition for Sugghowname */
int noptional;	/* index in symdef[] of symboldefinition for Optional */
int nhole;	/* index in symdef[] of symboldefinition for Hole */

int nlexical;	/* to distinguish lexical items from Symbols;
		 * the latter will be represented by the index of their
		 * definition in symdef[], so we save the final value
		 * of 'nsym' in 'nlexical', and add it to the indices of
		 * the lexical items in lexdef[] to get their representation.
		 */

main(argc, argv) int argc; char **argv; {
	int errflg;
	int c;
	extern char *optarg;
	extern int optind;
	int getopt();
	FILE *openfile();

	progname= argv[0];
	errflg= 0;
	while ((c= getopt(argc, argv, "s:c:l:n:g:t:i:h:")) != EOF) {
		switch (c) {
		case 's':
			maxsym= atoi(optarg);
			break;
		case 'c':
			maxclass= atoi(optarg);
			break;
		case 'l':
			maxlex= atoi(optarg);
			break;
		case 'n':
			maxname= atoi(optarg);
			break;
		case 'g':
			gfile= optarg;
			break;
		case 't':
			tfile= optarg;
			break;
		case 'i':
			ifile= optarg;
			break;
		case 'h':
			hfile= optarg;
			break;
		case '?':
		default:
			errflg++;
			break;
		}
	}
	
	if (argc > optind)
		errflg++;
	
	if (errflg)
		fatal(
"usage: %s [-s maxsym] [-c maxclass] [-l maxlex] [-n maxname]\n\
	[-g grammar-file] [-t table-file] [-i include-file]\n",
 			argv[0]);
	
	gfp= openfile(gfile, "r");
	tfp= openfile(tfile, "w");
	ifp= openfile(ifile, "w");
	
	process();
	
	fclose(gfp);
	fclose(tfp);
	fclose(ifp);
	
	exit(0);
}

Hidden FILE *openfile(file, mode) string file; string mode; {
	FILE *fp;
	string s;
	
	switch (*mode) {
	case 'r':
		s= "read";
		break;
	case 'w':
		s= "write";
		break;
	default:
		fatal("wrong mode %s opening file %s", mode, file);
	}
	fp= fopen(file, mode);
	if (fp == NULL) {
		fatal("can't open file \"%s\" to %s it", file, s);
	}
	return fp;
}

Hidden Procedure process() {

	allocate_tables();
	
	read_grammar_into_tables(); /* check repr's immediately? */
	
	fill_and_check_tables();
	
	compute_classes();
	
	dump_files();
}

/* VARARGS 1 */
message(format, arg1, arg2, arg3, arg4, arg5)
	char *format;
	char *arg1, *arg2, *arg3, *arg4, *arg5;
{
	fprintf(stderr, "%s: ", progname);
	fprintf(stderr, format, arg1, arg2, arg3, arg4, arg5);
	putc('\n', stderr);
}

/* VARARGS 1 */
fatal(format, arg1, arg2, arg3, arg4, arg5)
	char *format;
	char *arg1, *arg2, *arg3, *arg4, *arg5;
{
	fprintf(stderr, "%s: ", progname);
	fprintf(stderr, format, arg1, arg2, arg3, arg4, arg5);
	putc('\n', stderr);
	exit(1);
}

char *getmem(len) unsigned len; {
	char *p;
	
	p= malloc(MALLOC_ARG len);
	if (p == NULL)
		fatal("no more memory");
	return p;
}

Visible string savestr(s) string s; {
	string p= (string) getmem((unsigned) (strlen((char*)s) + 1));
	strcpy((char*)p, (char*)s);
	return p;
}

/* return saved copy itemarray pi with ilen items, 
 * the last of which must be a Nilitem.
 */

Visible itemptr savearray(pi, ilen) itemptr pi; int ilen; {
	itemptr pp= (itemptr) getmem((unsigned) (ilen*sizeof(item)));
	itemptr p= pp;
	
	while (--ilen > 0) {
		*p++= *pi++;
	}
	Assert(*pi == Nilitem);
	*p= Nilitem;
	return pp;
}

Visible Procedure asserr(file, line) string file; int line; {
	fatal("assertion error: file %s, line %d", file, line);
}
