/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1988. */

/* interpreter values */

/* Types */

#define Ptn 'T'		/* parsetree */
#define How 'h'		/* command howto */
#define Fun '+'		/* function */
#define Ref 'r'		/* refinement */
#define Prd 'i'		/* predicate */
#define Sim 'S'		/* simple location */
#define Tri '@'		/* trimmed text location */
#define Tse '['		/* table selection location */
#define Rangebounds 'B'	/* for range as list_item */
#define Ind 'p'		/* indirection node for targets and formals */

/************************************************************************/
/* environment								*/
/************************************************************************/

typedef value envtab;
typedef struct ec{envtab tab; struct ec *inv_env;} envchain;
typedef envchain *env;

/************************************************************************/
/* parsetrees								*/
/************************************************************************/

typedef value parsetree;
#define NilTree ((parsetree) Vnil)
#define Is_parsetree(v) (Type(v) == Ptn)
#define ValidTree(v) ((v) != NilTree)

/************************************************************************/
/*                                                                      */
/* A function or predicate is modelled as a compound consisting of      */
/* (i)  Zer/Mon/Dya for zero-, mon- or dyadicity;                       */
/* (ii) If a predefined function, an identifying number, otherwise -1   */
/* (iii)  If a user-defined function/predicate, its parse-tree          */
/*                                                                      */
/************************************************************************/

typedef struct {
	parsetree unit; 
	char /* bool */ unparsed;
	char /* bool */ filed; 
	parsetree code;
} how;

typedef struct {
	parsetree unit; 
	char /* bool */ unparsed;
	char /* bool */ filed; 
	parsetree code;
	literal adic; 
	intlet pre;
} funprd;

/* The first four fields of hows and funprds must be the same. */
#define Use (-1) /* funprd.pre==Use for user-defined funprds */

#define How_to(u)  ((how *)Ats(u))
#define Funprd(f)  ((funprd *)Ats(f))

typedef value fun;
typedef value prd;

fun mk_fun();
prd mk_prd();
value mk_how();

#define Is_howto(v) (Type(v) == How)
#define Is_function(v) (Type(v) == Fun)
#define Is_predicate(v) (Type(v) == Prd)

/************************************************************************/
/* refinements								*/
/************************************************************************/

typedef struct{parsetree rp;} ref;
#define Refinement(r) ((ref *)Ats(r))
#define Is_refinement(v) (Type(v) == Ref)
value mk_ref();

/************************************************************************/
/*                                                                      */
/* Locations                                                            */
/*                                                                      */
/* A simple location is modelled as a pair basic-identifier and         */
/*     environment, where a basic-identifier is modelled as a text      */
/*     and an environment as a pointer to a pair (T, E), where T is a   */
/*     table with basic-identifiers as keys and content values as       */
/*     associates, and E is the invoking environment or nil.            */
/*                                                                      */
/* A trimmed-text location is modelled as a triple (R, B, C).           */
/*                                                                      */
/* A compound location is modelled as a compound whose fields are       */
/*     locations, rather than values.                                   */
/*                                                                      */
/* A table-selection location is modelled as a pair (R, K).             */
/*                                                                      */
/************************************************************************/

typedef value loc;
#define Lnil ((loc) Vnil)

typedef value basidf;
typedef struct{basidf i; env e;} simploc;
typedef struct{loc R; value B, C;} trimloc;
typedef struct{loc R; value K;} tbseloc;

#define Simploc(l) ((simploc *)Ats(l))
#define Tbseloc(l) ((tbseloc *)Ats(l))
#define Trimloc(l) ((trimloc *)Ats(l))

loc mk_simploc();
loc mk_trimloc();
loc mk_tbseloc();

#define Is_locloc(v) IsSmallInt(v)
#define Is_simploc(v) (Type(v) == Sim)
#define Is_tbseloc(v) (Type(v) == Tse)
#define Is_trimloc(v) (Type(v) == Tri)

/************************************************************************/
/* rangebounds								*/
/************************************************************************/

#define R_LWB(v) ((value) *Field((v), 0))
#define R_UPB(v) ((value) *Field((v), 1))
#define Is_rangebounds(v) (Type(v) == Rangebounds)
value mk_rbounds();

/************************************************************************/
/* indirection								*/
/************************************************************************/

typedef struct{value val;} indirect;
#define Indirect(v) ((indirect *)Ats(v))
#define Is_indirect(v) (Type(v) == Ind)
value mk_indirect();

/************************************************************************/
