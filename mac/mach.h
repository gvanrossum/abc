/* char=8 bits, signed */
/* maxshort=32767 (=2**15-1) */
/* maxint=2147483647 (=2**31-1) */
/* maxlong=2147483647 (=2**31-1) */
/* pointers=32 bits */
/* base=2 */
/* Significant base digits=53 */
/* Smallest x such that 1.0-base**x != 1.0=-53 */
/* Small x such that 1.0-x != 1.0=1.11022e-16 */
/* Smallest x such that 1.0+base**x != 1.0=-52 */
/* Smallest x such that 1.0+x != 1.0=2.22045e-16 */
/* Arithmetic chops but uses guard digits */
/* Number of bits used for exponent=11 */
/* Minimum normalised exponent=-1021 */
/* Minimum normalised positive number=2.22507e-308 */
/* The smallest numbers are not kept normalised */
/* Smallest unnormalised positive number=4.94066e-324 */
/* Maximum exponent=1024 */
/* Maximum number=1.79769e+308 */
/* Double arithmetic uses a hidden bit */

/* Numeric package constants */
#define Maxintlet 32767 /* Maximum short */
#define Maxint 2147483647 /* Maximum int */
typedef short digit;
typedef long twodigit;
/* BASE must be a power of ten, BASE**2 must fit in a twodigit */
/* and -2*BASE as well as BASE*2 must fit in a digit */
#define BASE 10000
#define tenlogBASE 4 /*  = log10(BASE) */
#define BIG 9007199254740991.0 /* Maximum integral double */
#define MAXNUMDIG 16 /* The number of decimal digits in BIG */
#define MINNUMDIG 6 /* Don't change: this is here for consistency */
#define Maxreal 1.797693e+308 /* Maximum double */
#define Maxexpo 1024 /* Maximum value such that 2**Maxexpo<=Maxreal */
#define Minexpo (-1021) /* Minimum value such that -2**Minexpo>=Minreal */
#define DBLBITS 53 /* The number of bits in the fraction of a double */
#define LONGBITS 31 /* The number of bits in a long */
#define TWOTO_DBLBITSMIN1 4503599627370496.0 /* 2**(DBLBITS-1) */
#define TWOTO_LONGBITS 2147483648.0 /* 2**LONGBITS */
#define RNDM_LIMIT 1125899906842624.0 /* save limit for choice */

#define HEADER literal type; reftype refcnt; intlet len
#define FILLER
