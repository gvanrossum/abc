/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1986. */

/* environments and context */

typedef struct {
	value howtoname;
	env curnv;
	value r_names, *bndtgs;
	literal cntxt, resexp;
	parsetree cur_line;
	value cur_lino;
} context;

#define Enil ((env) NULL)

/* contexts: */
#define In_command 'c'
#define In_read '?'
#define In_unit 'u'
#define In_edval 'e'
#define In_tarval 't'
#define In_prmnv 'p'
#define In_wsgroup 'w'

/* results */
#define Ret 'V'
#define Rep '+'
#define Voi ' '

value* envassoc();

extern env curnv; extern value *bndtgs;
extern literal cntxt, resexp; extern value howtoname;
extern value errtname;
extern intlet lino;
extern intlet f_lino;
extern intlet i_lino;

extern context read_context;

extern envtab prmnvtab;
extern envchain prmnvchain;
extern env prmnv;

extern parsetree curline;
extern value curlino;

