/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1986. */

/*  Keywords
 *  Predefined functions and predicates
 *  See for the displayer strings the file bint2/i2dis.c
 */

#define Indent "   "	/* Output for each indentation level */
#define INDENTSIZE 3	/* Number of spaces in same */

/* *********************** KEYWORDS ******************************************* */

/* R_HOW_TO for bed/e1sugg.c; should conform with boot/lang.h */
#ifdef FRENCH
#define R_HOW_TO	"COMMENT "
#define S_HOW_TO	"COMMENT ?: "
#else
#define R_HOW_TO	"HOW TO "
#define S_HOW_TO	"HOW TO ?: "
#endif

#define K_HOW	 	"HOW"
#define K_TO_how	"TO"
#define K_PUT		"PUT"
#define K_IN_put	"IN"
#define K_INSERT	"INSERT"
#define K_IN_insert	"IN"
#define K_REMOVE	"REMOVE"
#define K_FROM_remove	"FROM"
#define K_SETRANDOM	"SET RANDOM"
#define K_DELETE	"DELETE"
#define K_CHECK 	"CHECK"
#define K_SHARE 	"SHARE"
#define K_PASS		"PASS"
#define K_WRITE 	"WRITE"
#define K_READ		"READ"
#define K_EG		"EG"
#define K_RAW		"RAW"
#define K_IF		"IF"
#define K_WHILE 	"WHILE"
#define K_FOR		"FOR"
#define K_IN_for	"IN"
#define K_SELECT	"SELECT"
#define K_ELSE		"ELSE"
#define K_QUIT		"QUIT"
#define K_RETURN	"RETURN"
#define K_REPORT	"REPORT"
#define K_SUCCEED	"SUCCEED"
#define K_FAIL		"FAIL"
#define K_AND		"AND"
#define K_OR		"OR"
#define K_NOT		"NOT"
#define K_SOME		"SOME"
#define K_EACH		"EACH"
#define K_NO		"NO"
#define K_IN_quant	"IN"
#define K_HAS		"HAS"

#ifdef GFX /* Graphics extension */
#define K_LINEFROM	"LINE FROM"
#define K_TO_line	"TO"
#define K_SPACEFROM	"SPACE FROM"
#define K_TO_space	"TO"
#define K_CLEARSCREEN	"CLEAR SCREEN"
#endif

/* *********************** predefined FUNCTIONS ******************************* */

#define F_pi		"pi"
#define F_e		"e"
#define F_now		"now"
#define F_abs		"abs"
#define F_sign		"sign"
#define F_floor 	"floor"
#define F_ceiling	"ceiling"
#define F_round 	"round"
#define F_mod		"mod"
#define F_root		"root"
#define F_random	"random"
#define F_exactly	"exactly"
#define F_sin		"sin"
#define F_cos		"cos"
#define F_tan		"tan"
#define F_arctan	"arctan"
#define F_angle		"angle"
#define F_radius	"radius"
#define F_exp		"exp"
#define F_log		"log"
#define F_stripped	"stripped"
#define F_split		"split"
#define F_upper		"upper"
#define F_lower		"lower"
#define F_keys		"keys"
#ifdef B_COMPAT
#define F_thof		"th'of"
#endif
#define F_item		"item"
#define F_min		"min"
#define F_max		"max"
#define F_choice	"choice"

/* *********************** predefined PREDICATES ****************************** */

#define P_exact		"exact"
#define P_in		"in"
#define P_notin 	"not.in"

