/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1987. */

/* Main program Mac version. */

#include "mac.h"
#include "macabc.h"
#include "rez.h"

Visible string mess_arg= (string) NULL;
Visible string keys_arg= (string) NULL;
Visible char *bws_arg= (char *) NULL;
Visible char *wsp_arg= (char *) NULL;
Visible bool is_gr_reccall= No;

Visible bool use_bed= Yes;
			/* the abc editor will be used, so initbed() etc. */

 /* Note: these options can't be changed in the Mac version. */

Visible bool slowterminal= No;	/* -S: ..... */
Visible bool hushbaby= No;	/* -H: ..... */

#ifndef NDEBUG
Visible bool dflag= No;
#endif
Visible int mac_todo;

extern std_open_hook();

extern void _DataInit();	/* declared to unload %A5Init segment */

main() {
	char *cd;
	extern long std_creator;
	char *savestr();
	
	/* first try ro prevent heap fragmentation 
	 * see Joel West, Programming with the MPW, Bantam Books,
	 * Toronto, November 1987, pp 109 vv */
	UnloadSeg(_DataInit);	/* purge %A5Init segment */
	MaxApplZone();		/* grow heap to maximum */
	/* 256 masters pointers as a first start */
	MoreMasters();
	MoreMasters();
	MoreMasters();
	MoreMasters();
	
	std_creator= 'Mabc';	/* Registration confirmed by Apple Oct 4, 1989. */
	set_open_hook(std_open_hook);
	intercept();
	setvbuf(stdout, (char*) NULL, _IOLBF, BUFSIZ);
	SetFontLock(TRUE);	/* to prevent weird screen before memexh */
	CouldAlert(MEMEXHALERT); /* lock memexh resource */

	
	initcall(0, (char**)NULL);	/* sets interactive */
	
	mac_todo= getappfiles();
	
	if (mac_todo == DoExit)
		immexit(1);	/* error message already given */

	set_vars();
	init((bool) (mac_todo == RunABC));
		
	switch (mac_todo) {
	case PrintAppfiles:
		pr_appfiles();
		break;
	case RunHowto:
		run_howto(appfiles[0]);
		break;
	case RunABC:
		run_abc(0, (char**)NULL);
		break;
	}
	
	bye(0);
}
