/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1986. */

/* Units */

typedef intlet typenode;

#define _Nodetype(len)	 ((len) & 0377)
#define _Nbranches(len)  ((len) >> 8)
#define Nodetype(v)   _Nodetype((v)->len)
#define Nbranches(v)  _Nbranches((v)->len)
#define Branch(v, n)  ((Ats(v)+(n)))

#define Unit(n)       (n>=HOW_TO && n<=REFINEMENT)
#define Command(n)    (n>=SUITE && n<=EXTENDED_COMMAND)
#define Expression(n) ((n>=TAG && n<=TAB_DIS)||(n>=TAGformal && n<=TAGzerprd))
#define Comparison(n) (n>=LESS_THAN && n<=UNEQUAL)

#define HOW_TO			0
#define YIELD			1
#define TEST			2
#define REFINEMENT		3

/* Commands */

#define SUITE			4
#define PUT			5
#define INSERT			6
#define REMOVE			7
#define SET_RANDOM		8
#define DELETE			9
#define CHECK			10
#define SHARE			11
#define PASS			12

#define WRITE			13 /* collateral expression */
#define WRITE1			14 /* single expression */
#define READ			15
#define READ_RAW		16

#define IF			17
#define WHILE			18
#define FOR			19

#define SELECTNODE		20
#define TEST_SUITE		21
#define ELSE			22

#define QUIT			23
#define RETURN			24
#define REPORT			25
#define SUCCEED 		26
#define FAIL			27

#define USER_COMMAND		28

/* the next three are only used when GFX has been defined */
#define SPACE			29
#define LINE			30
#define CLEAR			31

#define EXTENDED_COMMAND	32

/* Expressions, targets, tests */

#define TAG			33
#define COMPOUND		34

/* Expressions, targets */

#define COLLATERAL		35
#define SELECTION		36
#define BEHEAD			37
#define CURTAIL 		38

/* Expressions, tests */

#define UNPARSED		39

/* Expressions */

#define MONF			40
#define DYAF			41
#define NUMBER			42
#define TEXT_DIS		43
#define TEXT_LIT		44
#define TEXT_CONV		45
#define ELT_DIS 		46
#define LIST_DIS		47
#define RANGE_BNDS		48
#define TAB_DIS 		49

/* Tests */

#define AND			50
#define OR			51
#define NOT			52
#define SOME_IN 		53
#define EACH_IN 		54
#define NO_IN			55
#define MONPRD			56
#define DYAPRD			57
#define LESS_THAN		58
#define AT_MOST 		59
#define GREATER_THAN		60
#define AT_LEAST		61
#define IS_EQUAL		62
#define UNEQUAL 		63
#define Nonode			64

#define TAGformal		65
#define TAGlocal		66
#define TAGglobal		67
#define TAGrefinement		68
#define TAGzerfun		69
#define TAGzerprd		70

#define ACTUAL			71
#define FORMAL			72

#define COLON_NODE		73
	/* special node on top of suite inside WHILE or TEST_SUITE */

#define NTYPES			74
	/* number of nodetypes */

value node1();
value node2();
value node3();
value node4();
value node5();
value node6();
value node8();
value node9();
typenode nodetype();
/* Procedure display(); */
/* Procedure fix_nodes(); */

#define First_fieldnr	0

#define UNIT_NAME	First_fieldnr
#define HOW_FORMALS	First_fieldnr + 1	/* HOW'TO */
#define HOW_COMMENT	First_fieldnr + 2
#define HOW_SUITE	First_fieldnr + 3
#define HOW_REFINEMENT	First_fieldnr + 4
#define HOW_R_NAMES	First_fieldnr + 5
#define HOW_NLOCALS	First_fieldnr + 6
#define FPR_ADICITY	First_fieldnr + 1	/* YIELD, TEST */
#define FPR_FORMALS	First_fieldnr + 2
#define FPR_COMMENT	First_fieldnr + 3
#define FPR_SUITE	First_fieldnr + 4
#define FPR_REFINEMENT	First_fieldnr + 5
#define FPR_R_NAMES	First_fieldnr + 6
#define FPR_NLOCALS	First_fieldnr + 7

#define FML_KEYW	First_fieldnr		/* FORMALS HOW'TO */
#define FML_TAG 	First_fieldnr + 1
#define FML_NEXT	First_fieldnr + 2

#define SUI_LINO	First_fieldnr		/* SUITE */
#define SUI_CMD 	First_fieldnr + 1
#define SUI_COMMENT	First_fieldnr + 2
#define SUI_NEXT	First_fieldnr + 3
#define REF_NAME	First_fieldnr		/* REFINEMENT */
#define REF_COMMENT	First_fieldnr + 1
#define REF_SUITE	First_fieldnr + 2
#define REF_NEXT	First_fieldnr + 3
#define REF_START	First_fieldnr + 4

#define PUT_EXPR	First_fieldnr		/* PUT */
#define PUT_TARGET	First_fieldnr + 1
#define INS_EXPR	First_fieldnr		/* INSERT */
#define INS_TARGET	First_fieldnr + 1
#define RMV_EXPR	First_fieldnr		/* REMOVE */
#define RMV_TARGET	First_fieldnr + 1
#define SET_EXPR	First_fieldnr		/* SET'RANDOM */
#define DEL_TARGET	First_fieldnr		/* DELETE */
#define CHK_TEST	First_fieldnr		/* CHECK */
#define SHR_TARGET	First_fieldnr		/* SHARE */

#define WRT_L_LINES	First_fieldnr		/* WRITE */
#define WRT_EXPR	First_fieldnr + 1
#define WRT_R_LINES	First_fieldnr + 2
#define RD_TARGET	First_fieldnr		/* READ */
#define RD_EXPR 	First_fieldnr + 1
#define RDW_TARGET	First_fieldnr		/* READ'RAW */

#define IF_TEST 	First_fieldnr		/* IF */
#define IF_COMMENT	First_fieldnr + 1
#define IF_SUITE	First_fieldnr + 2
#define WHL_LINO	First_fieldnr		/* WHILE */
#define WHL_TEST	First_fieldnr + 1
#define WHL_COMMENT	First_fieldnr + 2
#define WHL_SUITE	First_fieldnr + 3
#define FOR_TARGET	First_fieldnr		/* FOR */
#define FOR_EXPR	First_fieldnr + 1
#define FOR_COMMENT	First_fieldnr + 2
#define FOR_SUITE	First_fieldnr + 3

#define SLT_COMMENT	First_fieldnr		/* SELECT */
#define SLT_TSUITE	First_fieldnr + 1
#define TSUI_LINO	First_fieldnr		/* TEST SUITE */
#define TSUI_TEST	First_fieldnr + 1
#define TSUI_COMMENT	First_fieldnr + 2
#define TSUI_SUITE	First_fieldnr + 3
#define TSUI_NEXT	First_fieldnr + 4
#define ELSE_LINO	First_fieldnr		/* ELSE */
#define ELSE_COMMENT	First_fieldnr + 1
#define ELSE_SUITE	First_fieldnr + 2

#define RTN_EXPR	First_fieldnr		/* RETURN */
#define RPT_TEST	First_fieldnr		/* REPORT */

#define UCMD_NAME	First_fieldnr		/* USER COMMAND */
#define UCMD_ACTUALS	First_fieldnr + 1
#define UCMD_DEF	First_fieldnr + 2
#define ACT_KEYW	First_fieldnr		/* ACTUALS USER COMMAND */
#define ACT_EXPR	First_fieldnr + 1
#define ACT_NEXT	First_fieldnr + 2

#define ECMD_NAME	First_fieldnr		/* EXTENDED COMMAND */
#define ECMD_ACTUALS	First_fieldnr + 1

#define COMP_FIELD	First_fieldnr		/* COMPOUND */
#define COLL_SEQ	First_fieldnr		/* COLLATERAL */
#define MON_NAME	First_fieldnr		/* MONADIC FUNCTION */
#define MON_RIGHT	First_fieldnr + 1
#define MON_FCT 	First_fieldnr + 2
#define DYA_NAME	First_fieldnr + 1	/* DYADIC FUNCTION */
#define DYA_LEFT	First_fieldnr
#define DYA_RIGHT	First_fieldnr + 2
#define DYA_FCT 	First_fieldnr + 3
#define TAG_NAME	First_fieldnr		/* TAG */
#define TAG_ID		First_fieldnr + 1
#define NUM_VALUE	First_fieldnr		/* NUMBER */
#define NUM_TEXT	First_fieldnr + 1
#define XDIS_QUOTE	First_fieldnr		/* TEXT DIS */
#define XDIS_NEXT	First_fieldnr + 1
#define XLIT_TEXT	First_fieldnr		/* TEXT LIT */
#define XLIT_NEXT	First_fieldnr + 1
#define XCON_EXPR	First_fieldnr		/* TEXT CONV */
#define XCON_NEXT	First_fieldnr + 1
#define LDIS_SEQ	First_fieldnr		/* LIST DIS */
#define TDIS_SEQ	First_fieldnr		/* TAB_DIS */
#define SEL_TABLE	First_fieldnr		/* SELECTION */
#define SEL_KEY 	First_fieldnr + 1
#define TRIM_LEFT	First_fieldnr		/* BEHEAD, CURTAIL */
#define TRIM_RIGHT	First_fieldnr + 1
#define UNP_SEQ 	First_fieldnr		/* UNPARSED */
#define UNP_TEXT	First_fieldnr + 1

#define AND_LEFT	First_fieldnr		/* AND */
#define AND_RIGHT	First_fieldnr + 1
#define OR_LEFT 	First_fieldnr		/* OR */
#define OR_RIGHT	First_fieldnr + 1
#define NOT_RIGHT	First_fieldnr		/* NOT */
#define QUA_TARGET	First_fieldnr		/* QUANTIFICATION */
#define QUA_EXPR	First_fieldnr + 1
#define QUA_TEST	First_fieldnr + 2
#define REL_LEFT	First_fieldnr		/* ORDER TEST */
#define REL_RIGHT	First_fieldnr + 1

#ifdef GFX
#define SPACE_FROM	First_fieldnr
#define SPACE_TO	First_fieldnr + 1
#define LINE_FROM	First_fieldnr
#define LINE_TO 	First_fieldnr + 1
#endif

#define COLON_SUITE	First_fieldnr		/* COLON_NODE */

