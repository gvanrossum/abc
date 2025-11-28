/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1988. */

/* Mac error handling. */

#include "mac.h"
#include "rez.h"
#include "macabc.h"

/* For error reporting from the FileMenu we use alerts because:
 * (1) the FileMenu behaves more Mac-like and the user expects alerts;
 * (2) we can be in the middle of editing an immediate-command
 *     and don't want to spoil the screen.
 */

Visible Procedure macerrS(m, s) int m; string s; {
	char *fmt;
	char buf[ERRBUFSIZE];
	char *getmess();
	
	fmt= getmess(m);
	while (*fmt == '*') fmt++;	/* skip *** for alerts */
	if (s == (char*)NULL)
		strcpy(buf, fmt);
	else
		sprintf(buf, fmt, s);
	ParamText(buf, (char*)NULL, (char*)NULL, (char*)NULL);
	set_arrow();
	
	Alert(MACERRALERT, (ProcPtr) NULL);
}

Visible Procedure macerr(m) int m; {
	macerrS(m, "");
}

/* Memory exhausted.  Stop the application. */

Visible Procedure memexh() {
	static bool beenhere= No;
	
	if (beenhere) immexit(-1);		
	beenhere= Yes;
	
	set_arrow();
	StopAlert(MEMEXHALERT, (ProcPtr)NULL);
	bye(-1);
}
