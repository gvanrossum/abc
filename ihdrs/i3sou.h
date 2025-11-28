/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1987. */

Visible bool is_zerfun();
Visible bool is_monfun();
Visible bool is_dyafun();
Visible bool is_zerprd();
Visible bool is_monprd();
Visible bool is_dyaprd();
Visible bool is_loaded();
Visible bool u_exists();
Visible bool p_exists();
Visible Procedure def_unit();
Visible Procedure def_std_howto();
Visible Procedure def_perm();
Visible Procedure free_perm();
Visible unsigned f_size();
Visible value get_unit();

extern value last_unit;
extern value last_target;
extern bool need_rec_suggestions;

#define Permname(pname) (behead(pname, MkSmallInt(2)))
#define Permtype(pname) (strval(pname)[0])
	/* possible types:
	 *	1-8 [Cmd ... Tar]	(order used in mac/m1print.c!)
	 *	: [last_unit]
	 *	= [last_target]
	 */

#define Cmd '1'
#define Zfd '2'
#define Mfd '3'
#define Dfd '4'
#define Zpd '5'
#define Mpd '6'
#define Dpd '7'
#define Tar '8'
#define OldHow '+'	/* old how-to type (used in bio) */
#define OldTar '-'	/* old target type (used in bio) */
#define Wsp '.'		/* workspace type */

#define	Cmd_ext ".cmd"
#define Zfd_ext ".zfd"
#define Mfd_ext ".mfd"
#define Dfd_ext ".dfd"
#define Zpd_ext ".zpd"
#define Mpd_ext ".mpd"
#define Dpd_ext ".dpd"
#define Cts_ext ".cts"
#define Wsp_ext ""

Visible value permkey();
Visible value get_pname();
Visible value getval();
Visible value gettarval();

Visible value new_fname();	/* devise a filename for a unit or target */
Visible value mkabcname();	/* vice versa for recovering target name */
#define CONVP_SIGN '_'		/* to map point */
#define CONVDQ_SIGN '@'		/* to map double quote */

Visible bool ckws_writable();
