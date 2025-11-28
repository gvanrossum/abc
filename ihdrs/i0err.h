/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1986. */

/* i1fun.c, i1nua.c, i1nur.c */

#define ZERO_DIVIDE	MESS(1800, "in i/j, j is zero")
#define NEG_POWER	MESS(1801, "in 0**y or y root 0, y is negative")
#define NEG_EVEN	MESS(1802, "in x**(p/q) or (q/p) root x, x is negative and q is even")
#define NEG_EXACT	MESS(1803, "in x**y or y root x, x is negative and y is not exact")

/* i2exp.c, i2fix.c, i2tes.c */

#define PRIO		MESS(1804, "ambiguous expression; please use ( and ) to resolve")
#define NO_EXPR		MESS(1805, "no expression where expected")
#define NO_TEST		MESS(1806, "no test where expected")
#define UPTO_EXPR 	MESS(1807, "something unexpected in expression")
#define UPTO_TEST 	MESS(1808, "something unexpected in test")
#define NO_TRIM_TARG	MESS(1809, "misformed address")

/* i2gen.c, i2fix.c */

#define NO_INIT_OR_DEF	MESS(1810, "%s hasn't been initialised or (properly) defined")
#define NO_DEFINITION	MESS(1811, "%s hasn't been (properly) defined")

/* i3in2.c */

#define NO_VALUE	MESS(1812, "%s has not yet received a value")

/* i2gen.c, i3int.c */

#define YIELD_NO_RETURN	MESS(1813, "function returns no value")
#define TEST_NO_REPORT	MESS(1814, "predicate reports no outcome")

/* i2gen.c, i2ana.c */

#define REF_NO_TARGET	MESS(1815, "a refinement may not be used as an address")
#define BAD_WHILE	MESS(1816, "bad node in while")
#define BAD_TESTSUITE	MESS(1817, "bad node in testsuite")

/* i2cmd.c, i2uni.c */
#define WRONG_INDENT	MESS(1818, "indentation not used consistently")
#define SMALL_INDENT	MESS(1819, "indentation must be at least 2")

/* i3loc.c, i3sta.c */

#define SEL_NO_TABLE	MESS(1820, "selection on non-table")
