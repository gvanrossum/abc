/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1986. */

#include "b.h"
#include "bmem.h"

#include <Types.h>
#include <Events.h>
#include <Files.h>
#include <IOCtl.h>
#include <Errors.h>
#include <ErrNo.h>

/* Check for pending interrupt */

int pollcnt= 0;

Visible Procedure poll_interrupt() {
	if (trminterrupt())
		int_signal();
}


/* Rename a file. */

Visible int rename(f, g) char *f, *g; {
	OSErr err= Rename(f, 0, g);
	return err == noErr ? 0 : -1;
}


/* Return appropriate number to start the random generator. */

Visible int getseed() {
	return TickCount();
}


/* Is a file descriptor interactive? */

Visible bool isatty(fd) int fd; {
	return ioctl(fd, FIOINTERACTIVE, (long*)0) == 0;
}

/* Assertion error. */

Visible asserr(file, line) char *file; int line; {
	char mess[255];
	sprintf(mess, "Assertion botched in file %s, line %d.",
		file, line);
	DebugStr(mess);
	immexit(2);
}

/* Abort. */


