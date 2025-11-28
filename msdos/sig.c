/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1990. */

/* Handle signals */

#include "b.h"

#ifdef SIGNAL

#include <signal.h>
#include <float.h>	/* for _fpreset() */

Visible bool intrptd = No;

Hidden SIGTYPE intsig()
{
	intrptd = Yes;
	signal(SIGINT, intsig);
}

Hidden SIGTYPE fpesig()
{
	signal(SIGFPE, SIG_IGN);
	fpe_signal();
	_fpreset();	/* reinitialize floating-point package */
	signal(SIGFPE, fpesig);
}

Visible Procedure initsig()
{
	signal(SIGINT, intsig);
	signal(SIGFPE, fpesig);
}

#endif /* SIGNAL */
