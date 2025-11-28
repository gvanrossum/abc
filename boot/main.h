/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1988. */

typedef short item;	/* used for indexing xxxdef[] arrays;
			 * each grammatical item will be represented
			 * by its index in the applicable table.
			 */
typedef item *itemptr;

/* to indicate end of classinfo arrays: */
#define Nilitem ((item) -1)
#define Isnilitem(i) (((item) (i)) == ((item) -1))
/* (We can't use zero, because an item can indicate  the zero's entry
 * of a xxxdef[] array.
 */
 
struct classinfo {	/* the itemptr's here indicate 0-terminated array's */
	string c_name;
		/* only lower-case */
	itemptr c_syms;
		/* list of possible Symbols or LEXICALS */
	itemptr c_insert;
		/* list of pairs (char, class) for insertion */
	itemptr c_append;
		/* ditto for append to child already there */
	itemptr c_join;
		/* ditto for join of child with new node */
};

#define MAXCHILD 4	/* should actually be computed from input-grammar */
			/* but this makes live easier ... */
struct syminfo {
	string s_name;
		/* first char upper-case, rest lower */
	string s_repr[MAXCHILD+1];
		/* fixed-text parts */
		/* entries are [0..nch] inclusive */
	item s_class[MAXCHILD];
		/* index of child into classdef[] */
};

struct lexinfo {
	string l_name;	/* representing NAME, only Capitals */
	string l_start; /* char's that may start this lexical */
	string l_cont;	/* char's that may continue this lexical;
			 * only used in abc-editor, not in mktable.
			 */
	int l_body;	/* index of enveloping class in classdef[] */
	int l_sym;	/* index in symdef[], for use in class-definition */
	int l_class;	/* index in classdef[], for use in Symbol-definition */
};

struct nameinfo {
	string n_name;
	item n_index;	/* into classdef[] | symdef[] | lexdef[] */
	char n_type;	/* Class or Sym or Lex */
};

#define Class ('C')
#define Sym ('S')
#define Lex ('L')
#define Errtype ('E')

/* MAX's to allocate space; can be overwritten with commandline options */
#define MAXSYM 127
#define MAXCLASS 100
#define MAXLEX 20
#define MAXNAME 300

#define NAMELEN 100	/* maximum significant part of name */
#define STRINGLEN 256	/* maximum string length */
#define SYMLEN 200	/* maximum number of alternative symbols in
			 * any class definition */

extern struct classinfo *classdef;
extern struct syminfo *symdef;
extern struct lexinfo *lexdef;
extern struct nameinfo *namelist;

extern int maxclass;
extern int maxsym;
extern int maxlex;
extern int maxname;

extern int nclass;
extern int nsym;
extern int nlex;
extern int nname;

extern int nsuggstnbody;
extern int lsuggestion;
extern int nsuggestion;

extern int nsugghowbody;
extern int lsugghowname;
extern int nsugghowname;

extern int noptional;
extern int nhole;
extern int nlexical;

#define GRAMMAR "grammar"
#define TABLES "tabl.c.out"
#define INCLUDE "tabl.h.out"
#define HFILE "tabl.h"

extern string progname;
extern FILE *gfp;
extern char *gfile;
extern FILE *tfp;
extern char *tfile;
extern FILE *ifp;
extern char *ifile;
extern char *hfile;

char *getmem();
string savestr();
itemptr savearray();

#define Assert(cond) ((cond) || asserr(__FILE__, __LINE__))
