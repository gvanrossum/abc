/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1986. */

/*
 * B editor -- Handle error messages.
 */

#include "b.h"
#include "bedi.h"
#include "bmem.h"
#include "bobj.h"
#include "erro.h"
#include "node.h"
#include "trm.h"
#include "port.h"

string querepr();

extern int winheight; /* From scrn.c */
extern int winstart; /* From scrn.c */
extern int llength; /* From scrn.c */

#define MAXMSG 1000
#define MAXBUF 50
static char *msgbuffer;
static char *msgmode;
static bool ringbell;
static int priority;

#define M_RECORDING	MESS(6400, "Recording")
#define M_COPYBUF	MESS(6401, "Copy buffer")

static char *mrecbuf;
static char *mcopybuf;

Forward Hidden int addscrollbar();

/*
 * Status line.  A combination of scroll bar, error message etc.
 * Put the message on the screen and clear the buffers for next time.
 * If there is no message, show status and copy buffer and recording mode.
 */

Visible Procedure
stsline(totlines, topline, scrlines, copybuffer, recording)
	int totlines;
	int topline;
	int scrlines;
	value copybuffer;
	bool recording;
{
	register string bp;
	char *msg_mode= NULL;

	if (ringbell)
		trmbell();
	if (msgbuffer[0]) {
		msgbuffer[llength-1] = '\0'; /* Truncate */
		msgmode[llength-1] = '\0';
		if (ringbell) {
			msg_mode= msgmode;
		}
	}
	else {
		bp = msgbuffer;
#ifdef SCROLLBAR
		bp += addscrollbar(totlines, topline, scrlines);
#endif /* SCROLLBAR */
		if (recording) {
			if (!mrecbuf[0])
				strcpy(mrecbuf, getmess(M_RECORDING));
			sprintf(bp, "[%s] ", mrecbuf);
			while (*bp)
				++bp;
		}
		if (copybuffer) {
			if (!mcopybuf[0])
				strcpy(mcopybuf, getmess(M_COPYBUF));
#ifdef SHOWBUF
			sprintf(bp, "[%s: %.80s]", mcopybuf, querepr(copybuffer));
			while (*bp)
				++bp;
			if (bp >= msgbuffer+80)
				strcpy(msgbuffer+75, "...]");
#else /* !SHOWBUF */
			sprintf(bp, "[%s]", mcopybuf);
#endif /* !SHOWBUF */
		}
	}
	trmputdata(winheight, winheight, 0, msgbuffer, msg_mode);
	msgbuffer[0] = '\0';
	priority = 0;
	ringbell = No;
}

#ifdef SCROLLBAR

/*
 * Paint a beautiful scroll bar so the user can see about what part of the
 * unit is visible on the screen (considering logical lines).
 */

Hidden int
addscrollbar(totlines, topline, scrlines)
	int totlines;
	int topline;
	int scrlines;
{
	int endline;
	register int i;

	if (winstart > 0 || scrlines > totlines)
		return 0; /* Nothing outside screen */
	if (totlines <= 0)
		totlines = 1; /* Don't want to divide by 0 */
	topline = topline*winheight / totlines;
	endline = topline + (scrlines*winheight + totlines-1) / totlines;
	if (endline > winheight)
		endline = winheight;
	if (topline >= endline)
		topline = endline-1;
	for (i = 0; i < topline; ++i)
		msgbuffer[i] = '-';
	for (; i < endline; ++i)
		msgbuffer[i] = '#';
	for (; i < winheight; ++i)
		msgbuffer[i] = '-';
	msgbuffer[i++] = ' ';
	msgbuffer[i] = '\0';
	return i;
}

#endif /* SCROLLBAR */

/*
 * Issue an error message.  These have highest priority.
 * Once an error message is in the buffer, further error messages are ignored
 * until it has been displayed.
 */

Hidden Procedure
ederr1(s)
	string s;
{
	ringbell = Yes;
	if (s && priority < 3) {
		priority = 3;
		strcpy(msgbuffer, s);
	}
}

Visible Procedure
ederr(m)
	int m;
{
	if (m == 0) ringbell= Yes;
	else ederr1(getmess(m));
}

Visible Procedure
ederrS(m, s)
	int m;
	string s;
{
	sprintf(messbuf, getmess(m), s);
	ederr1(messbuf);	
}

Visible Procedure
ederrC(m, c)
	int m;
	char c;
{
	sprintf(messbuf, getmess(m), c);
	ederr1(messbuf);
}

/*
 * Issue an informative message.  These have medium priority.
 * Unlike error messages, the last such message is displayed.
 */

Visible Procedure
edmessage(s)
	string s;
{
	if (s && priority <= 2) {
		priority = 2;
		strcpy(msgbuffer, s);
	}
}

#ifndef NDEBUG

/* Assertion error */

Visible Procedure asserr(file, line)
     string file;
     int line;
{
	char mess[255];
	sprintf(mess, "Assertion botched in file %s, line %d.", file, line);
	putserr(mess);
	bye(-1);
}

#endif /* !NDEBUG */

/*
 * Issue a debugging message.  These  have lowest priority and
 * are not shown to ordinary users.
 */

#ifndef NDEBUG

Visible bool dflag = No;

/* VARARGS 1 */
Visible Procedure
debug(fmt, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10)
	string fmt;
{
	if (fmt && priority <= 1) {
		priority = 1;
		sprintf(msgbuffer,
			fmt, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10);
	}
}

#endif /* NDEBUG */

/*
 * Dump any error message still remaining to console or stderr.
 */

Visible Procedure
enderro()
{
	if (!msgbuffer)
		return;
	if (msgbuffer[0])
		putsSerr("%s\n", msgbuffer);
	msgbuffer[0] = '\0';
	priority = 0;
	ringbell = No;
}

Visible Procedure init_erro() {
	register int i;

	msgbuffer= (char*) getmem(MAXMSG);
	msgbuffer[0]= '\0';
	msgmode= (char*) getmem(MAXMSG);
	for (i = 0; i < MAXMSG; i++)
		msgmode[i]= STANDOUT;
	mrecbuf= (char*) getmem(MAXBUF);
	mrecbuf[0]= '\0';
	mcopybuf= (char*) getmem(MAXBUF);
	mcopybuf[0]= '\0';
}

Visible Procedure end_erro() {
#ifdef MEMTRACE
	freemem((ptr) msgbuffer);
	freemem((ptr) mrecbuf);
	freemem((ptr) mcopybuf);
#endif
}
