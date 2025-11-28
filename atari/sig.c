/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1990. */

/* Handle interrupts */

#include "b.h"

#ifdef SIGNAL

#include <signal.h>

Visible bool intrptd= No; /* currently not used */

Hidden SIGTYPE intsig(sig)
     int sig;
{
	signal(SIGINT, intsig);
	intrptd= Yes;
}

Visible Procedure initsig() {
	signal(SIGINT, intsig);
}

#endif /* SIGNAL */


