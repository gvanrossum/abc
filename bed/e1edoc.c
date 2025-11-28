/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1986. */

#include "b.h"
#include "bedi.h"
#include "etex.h"
#include "bobj.h"
#include "node.h"
#include "erro.h"
#include "gram.h"
#include "oper.h"
#include "queu.h"
#include "supr.h"
#include "tabl.h"
#include "port.h"

extern bool io_exit;
extern int getoperation();

#define Mod(k) (((k)+MAXHIST) % MAXHIST)
#define Succ(k) (((k)+1) % MAXHIST)
#define Pred(k) (((k)+MAXHIST-1) % MAXHIST)

#define	CANT_SAVE   MESS(6300, "Cannot save how-to on file \"%s\"")

extern environ *tobesaved;
extern string savewhere;

Hidden int highwatmark = Maxintlet;

Visible bool lefttorite;
	/* Saves some time in nosuggtoqueue() for read from file */

Forward Hidden bool execute();
Forward Hidden bool canexit();
Forward Hidden bool findhole();
Forward Hidden Procedure writetext();

/*
 * Edit a unit or target, using the environment offered as a parameter.
 */

Visible bool dofile(ep, filename, linenumber, kind, creating, changed)
     environ *ep;
     string filename;
     int linenumber;
     literal kind;
     bool creating;
     bool *changed;
{
	bool read_bad= No;
	bool readfile();
	
#ifdef SAVEPOS
	if (linenumber <= 0)
		linenumber = getpos(filename);
#endif /* SAVEPOS */
	setroot(kind == '=' ? Target_edit : Unit_edit);
	savewhere = filename;
	tobesaved = (environ*)NULL;
	*changed = No;

	lefttorite = Yes;
	if (!readfile(ep, filename, linenumber, creating)) {
		ederr(READ_BAD);
		read_bad = Yes;
	}
#ifdef USERSUGG
	readsugg(ep->focus);
#endif /* USERSUGG */
	lefttorite = No;

	ep->generation = 0;
	if (!editdocument(ep, read_bad))
		return No;
	if (ep->generation > 0) {
		if (!save(ep->focus, filename))
			ederrS(CANT_SAVE, filename);
		else {
			*changed = Yes;
		}
#ifdef USERSUGG
		writesugg(ep->focus);
#endif /* USERSUGG */
	}
#ifdef SAVEPOS
	savpos(filename, ep);
#endif /* SAVEPOS */
	savewhere = (char*)NULL;
	tobesaved = (environ*)NULL;
	return Yes;
}


/*
 * Call the editor for a given document.
 */

Visible bool canceled= No;

Visible bool suspendabc() {
	int r;
	
	endshow();
	endterm();
	r= trmsuspend();
	initterm();
	return r;
}

Visible bool
editdocument(ep, bad_file)
	environ *ep;
	bool bad_file;
{
	int k;
	int first = 0;
	int last = 0;
	int current = 0;
	int onscreen = -1;
	bool reverse = No;
	environ newenv;
	int cmd;
	bool errors = No;
	int undoage = 0;
	bool done = No;
	int height;
	environ history[MAXHIST];

	Ecopy(*ep, history[0]);

	for (;;) { /* Command interpretation loop */
		if (reverse && onscreen >= 0)
			height = history[onscreen].highest;
		else
			height = history[current].highest;
		if (height < highwatmark) highwatmark = height;
		if (done)
			break;
		if (!canceled && trmavail() <= 0) {
			if (onscreen != current)
				virtupdate(onscreen < 0 ? (environ*)NULL : &history[onscreen],
					&history[current],
					reverse && onscreen >= 0 ?
						history[onscreen].highest : history[current].highest);
			onscreen = current;
			highwatmark = Maxintlet;
			actupdate(history[current].copyflag ?
				history[current].copybuffer : Vnil,
#ifdef RECORDING
				history[current].newmacro != Vnil,
#else /* !RECORDING */
				No,
#endif /* !RECORDING */
				No);
		}
		if (canceled) break;
		cmd = getoperation();
		
		errors = No;
		switch (cmd) {

		case UNDO:
			if (current == first)
				errors = Yes;
			else {
				if (onscreen == current)
					reverse = Yes;
				current = Pred(current);
				undoage = Mod(last-current);
			}
			break;

		case REDO:
			if (current == last)
				errors = Yes;
			else {
				if (current == onscreen)
					reverse = No;
				if (history[Succ(current)].generation <
						history[current].generation)
					ederr(REDO_OLD); /***** Should refuse altogether??? *****/
				current = Succ(current);
				undoage = Mod(last-current);
			}
			break;

#ifdef HELPFUL
		case HELP:
			if (help())
				onscreen = -1;
			break;
#endif /* HELPFUL */

		case SUSPEND:
			errors= !suspendabc();
			onscreen= -1;
			trmundefined();
			if (doctype == D_immcmd)
				cmdprompt(CMDPROMPT);
			if (errors) {
				ederr(SUSP_BAD);
				errors= No;
			}
			break;
			
		case REDRAW:
			onscreen = -1;
			trmundefined();
			break;

		case EOF:
			done = Yes;
			break;

		case CANCEL:
			if (bad_file) {
				return No;
			}
			else if (doctype == D_input)
				canceled= Yes;
			else
				errors= Yes;
			break;

		default:
			Ecopy(history[current], newenv);
			newenv.highest = Maxintlet;
			newenv.changed = No;
			if (cmd != EXIT)
				errors = !execute(&newenv, cmd) || !checkep(&newenv);
			else {
				done = Yes;
				io_exit= Yes;
			}
#ifdef EDITRACE
	dumpev(&newenv, "AFTER EXECUTE");
#endif
			if (errors) {
				switch (cmd) {
				case NEWLINE:
					if (newenv.mode == ATEND && !parent(newenv.focus)) {
						errors = !checkep(&newenv);
						if (!errors) {
#ifdef USERSUGG
							check_last_unit(&newenv, current);
#endif
							done = Yes;
						}
					}
					break;
#ifdef HELPFUL
				case '?':
					cmd = HELP;
					/* FALL THROUGH: */
				case HELP:
					if (help())
						onscreen = -1;
#endif /* HELPFUL */
				}
			}
			if (errors)
				Erelease(newenv);
			else {
#ifndef SMALLSYS
				if (done)
						done = canexit(&newenv);
				if (!done)
					io_exit= No;
#endif /* SMALLSYS */
				if (!done && ev_eq(&newenv, &history[current])) {
					errors= Yes;
					Erelease(newenv);
					break; /* don't remember no.ops */
				}
				if (newenv.changed)
					++newenv.generation;
				last = Succ(last);
				current = Succ(current);
				if (last == first) {
					/* Array full (always after a while). Discard "oldest". */
					if (current == last
						|| undoage < Mod(current-first)) {
						Erelease(history[first]);
						first = Succ(first);
						if (undoage < MAXHIST)
							++undoage;
					}
					else {
						last = Pred(last);
						Erelease(history[last]);
					}
				}
				if (current != last
					&& newenv.highest < history[current].highest)
					history[current].highest = newenv.highest;
				/* Move entries beyond current one up. */
				for (k = last; k != current; k = Pred(k)) {
					if (Pred(k) == onscreen)
						onscreen = k;
					Emove(history[Pred(k)], history[k]);
				}
				Ecopy(newenv, history[current]);
				Erelease(history[current]);
			}
			break; /* default */

		} /* switch */

		if (errors
#ifdef HELPFUL
			&& cmd != HELP
#endif
			) {
			if (!OPTslowterminal && isascii(cmd)
				&& (isprint(cmd) || cmd == ' '))
				ederrC(INS_BAD, cmd);
			else
				ederr(0);
		}
		if (savewhere)
			tobesaved = &history[current];
	} /* for (;;) */

	if (onscreen != current)
		virtupdate(onscreen < 0 ? (environ*)NULL : &history[onscreen],
			&history[current], highwatmark);
	actupdate(Vnil, No, Yes);
	Erelease(*ep);
	Ecopy(history[current], *ep);
	if (savewhere)
		tobesaved = ep;
	for (current = first; current != last; current = Succ(current))
		Erelease(history[current]);
	Erelease(history[last]);
	return Yes;
}

/*
 * Execute a command, return success or failure.
 */

extern bool justgoon;

Hidden bool
execute(ep, cmd)
	register environ *ep;
	register int cmd;
{
	register bool spflag = ep->spflag;
	register int i;
	environ ev;
	char buf[2];
	char ch;
	int len;
#ifdef USERSUGG
	bool sugg;
	int sym= symbol(tree(ep->focus));
	
	sugg = sym == Suggestion;
#define ACKSUGG(ep) if (sugg) acksugg(ep)
#define KILLSUGG(ep) if (sugg) killsugg(ep, (string*)NULL); \
		     else if (sym==Sugghowname) killhowsugg(ep)
#else /* !USERSUGG */
#define ACKSUGG(ep) /* NULL */
#define KILLSUGG(ep) /* NULL */
#endif /* !USERSUGG */

	if (justgoon)
		justgoon = isascii(cmd) && islower(cmd);
	
#ifdef RECORDING
	if (ep->newmacro && cmd != RECORD && cmd != PLAYBACK) {
		value t;
		buf[0] = cmd; buf[1] = 0;
		e_concto(&ep->newmacro, t= mk_etext(buf));
		release(t);
	}
#endif /* RECORDING */
	ep->spflag = No;

	switch (cmd) {

#ifdef RECORDING
	case RECORD:
		ep->spflag = spflag;
		if (ep->newmacro) { /* End definition */
			release(ep->oldmacro);
			if (ep->newmacro && e_length(ep->newmacro) > 0) {
				ep->oldmacro = ep->newmacro;
				edmessage(getmess(REC_OK));
			}
			else {
				release(ep->newmacro);
				ep->oldmacro = Vnil;
			}
			ep->newmacro = Vnil;
		}
		else /* Start definition */
			ep->newmacro = mk_etext("");
		return Yes;

	case PLAYBACK:
		if (!ep->oldmacro || e_length(ep->oldmacro) <= 0) {
			ederr(PLB_NOK);
			return No;
		}
		ep->spflag = spflag;
		len= e_length(ep->oldmacro);
		for (i = 0; i < len; ++i) {
			ch= e_ncharval(i+1, ep->oldmacro);
			Ecopy(*ep, ev);
			if (execute(ep, ch&0377) && checkep(ep))
				Erelease(ev);
			else {
				Erelease(*ep);
				Emove(ev, *ep);
				if (!i)
					return No;
				ederr(0); /* Just a bell */
				/* The error must be signalled here, because the overall
				   command (PLAYBACK) succeeds, so the main loop
				   doesn't ring the bell; but we want to inform the
				   that not everything was done either. */
				return Yes;
			}
		}
		return Yes;
#endif /* RECORDING */

#ifdef GOTOCURSOR
	case GOTO:
		ACKSUGG(ep);
#ifdef RECORDING
		if (ep->newmacro) {
			ederr(GOTO_REC);
			return No;
		}
#endif /* RECORDING */
		return gotocursor(ep);
#endif /* GOTOCURSOR */

#ifdef GOTOCURSOR
	case MOUSE:
		ACKSUGG(ep);
#ifdef RECORDING
		if (ep->newmacro) {
			ederr(GOTO_REC);
			return No;
		}
#endif /* RECORDING */
		return gotomouse(ep);
#endif /* GOTOCURSOR */

	case NEXT:
		ACKSUGG(ep);
		return nextarrow(ep);

	case PREVIOUS:
		ACKSUGG(ep);
		return previous(ep);

	case LEFTARROW:
		ACKSUGG(ep);
		return leftarrow(ep);

	case RITEARROW:
		ACKSUGG(ep);
		return ritearrow(ep);

	case WIDEN:
		ACKSUGG(ep);
		return widen(ep, No);

	case EXTEND:
		ACKSUGG(ep);
		return extend(ep);

	case FIRST:
		ACKSUGG(ep);
		return narrow(ep);

	case LAST:
		ACKSUGG(ep);
		return rnarrow(ep);

	case UPARROW:
		ACKSUGG(ep);
		return uparrow(ep);

	case DOWNARROW:
		ACKSUGG(ep);
		return downarrow(ep);

	case UPLINE:
		ACKSUGG(ep);
		return upline(ep);

	case DOWNLINE:
		ACKSUGG(ep);
		return downline(ep);


	case PASTE:
	case COPY:
		ACKSUGG(ep);
		ep->spflag = spflag;
		return copyinout(ep);

	case CUT:
	case DELETE:
		ACKSUGG(ep);
		return deltext(ep);

	case ACCEPT:
		ACKSUGG(ep);
		return acceptcommand(ep);

	default:
		if (!isascii(cmd) || !isprint(cmd))
			return No;
		ep->spflag = spflag;
		return ins_char(ep, cmd, islower(cmd) ? toupper(cmd) : -1);

	case ' ':
		ep->spflag = spflag;
		return ins_char(ep, ' ', -1);

	case NEWLINE:
		KILLSUGG(ep);
		return ins_newline(ep, No);
	}
}

/*
 * Initialize an environment variable.	Most things are set to 0 or NULL.
 */

Visible Procedure
clrenv(ep)
	environ *ep;
{
	ep->focus = newpath(NilPath, gram(Optional), 1);
	ep->mode = WHOLE;
	ep->copyflag = ep->spflag = ep->changed = No;
	ep->s1 = ep->s2 = ep->s3 = 0;
	ep->highest = Maxintlet;
	ep->copybuffer = Vnil;
#ifdef RECORDING
	ep->oldmacro = ep->newmacro = Vnil;
#endif /* RECORDING */
	ep->generation = 0;
	ep->changed = No;
}

/*
 * Find out if the current position is higher in the tree
 * than `ever' before (as remembered in ep->highest).
 * The algorithm of pathlength() is repeated here to gain
 * some efficiency by stopping as soon as it is clear
 * no change can occur.
 * (Higher() is called VERY often, so this pays).
 */

Visible Procedure
higher(ep)
	register environ *ep;
{
	register path p = ep->focus;
	register int pl = 0;
	register int max = ep->highest;

	while (p) {
		++pl;
		if (pl >= max)
			return;
		p = parent(p);
	}
	ep->highest = pl;
}

#ifndef NDEBUG

/*
 * Issue debug status message.
 */

Visible Procedure
dbmess(ep)
	register environ *ep;
{
#ifndef SMALLSYS
	char stuff[80];
	register string str = stuff;

	switch (ep->mode) {
	case VHOLE:
		sprintf(stuff, "VHOLE:%d.%d", ep->s1, ep->s2);
		break;
	case FHOLE:
		sprintf(stuff, "FHOLE:%d.%d", ep->s1, ep->s2);
		break;
	case ATBEGIN:
		str = "ATBEGIN";
		break;
	case ATEND:
		str = "ATEND";
		break;
	case WHOLE:
		str = "WHOLE";
		break;
	case SUBRANGE:
		sprintf(stuff, "SUBRANGE:%d.%d-%d", ep->s1, ep->s2, ep->s3);
		break;
	case SUBSET:
		sprintf(stuff, "SUBSET:%d-%d", ep->s1, ep->s2);
		break;
	case SUBLIST:
		sprintf(stuff, "SUBLIST...%d", ep->s3);
		break;
	default:
		sprintf(stuff, "UNKNOWN:%d,%d,%d,%d",
			ep->mode, ep->s1, ep->s2, ep->s3);
	}
	sprintf(messbuf,
#ifdef SAVEBUF
		"%s, %s, wi=%d, hi=%d, (y,x,l)=(%d,%d,%d) %s",
		symname(symbol(tree(ep->focus))),
#else /* !SAVEBUF */
		"%d, %s, wi=%d, hi=%d, (y,x,l)=(%d,%d,%d) %s",
		symbol(tree(ep->focus)),
#endif /* SAVEBUF */
		str, nodewidth(tree(ep->focus)), ep->highest,
		Ycoord(ep->focus), Xcoord(ep->focus), Level(ep->focus),
			ep->spflag ? "spflag on" : "");
#endif /* !SMALLSYS */
	edmessage(messbuf);
}

#endif /* NDEBUG */

#ifndef SMALLSYS

Hidden bool
canexit(ep)
	environ *ep;
{
	environ ev;

	if (symbol(tree(ep->focus)) == Suggestion)
		acksugg(ep);
	shrink(ep);
	if (ishole(ep))
		VOID deltext(ep);
	Ecopy(*ep, ev);
	top(&ep->focus);
	higher(ep);
	ep->mode = WHOLE;
	if (findhole(&ep->focus)) {
		Erelease(ev);
		ederr(EXIT_HOLES); /* There are holes left */
		return No;
	}
	Erelease(*ep);
	Emove(ev, *ep);
	return Yes;
}


Hidden bool
findhole(pp)
	register path *pp;
{
	register node n = tree(*pp);

	if (Is_etext(n))
		return No;
	if (symbol(n) == Hole)
		return Yes;
	if (!down(pp))
		return No;
	for (;;) {
		if (findhole(pp))
			return Yes;
		if (!rite(pp))
			break;

	}
	if (!up(pp)) Abort();
	return No;
}

#endif /* !SMALLSYS */

/* ------------------------------------------------------------------ */

#ifdef SAVEBUF

/*
 * Write a node.
 */

#ifdef DUMPING_QUEUES
Visible Procedure
#else
Hidden Procedure
#endif
writenode(n, fp)
	node n;
	FILE *fp;
{
	int nch;
	int i;

	if (!n) {
		fputs("(0)", fp);
		return;
	}
	if (((value)n)->type == Etex) {
		writetext((value)n, fp);
		return;
	}
	nch = nchildren(n);
	fprintf(fp, "(%s", symname(symbol(n)));
	for (i = 1; i <= nch; ++i) {
		putc(',', fp);
		writenode(child(n, i), fp);
	}
	putc(')', fp);
}


Hidden Procedure
writetext(v, fp)
	value v;
	FILE *fp;
{
	intlet k, len;
	int c;

	Assert(v && Is_etext(v));
	len= e_length(v);
	putc('\'', fp);
	for (k= 0; k<len; ++k) {
		c= e_ncharval(k+1, v);
		if (c == ' ' || isprint(c)) {
			putc(c, fp);
			if (c == '\'' || c == '`')
				putc(c, fp);
		}
		else if (isascii(c))
			fprintf(fp, "`$%d`", c);
	}
	putc('\'', fp);
}


Visible bool
savequeue(v, filename)
	value v;
	string filename;
{
	register FILE *fp;
	auto queue q = (queue)v;
	register node n;
	register bool ok;
	register int lines = 0;

	fp = fopen(filename, "w");
	if (!fp)
		return No;
	q = qcopy(q);
	while (!emptyqueue(q)) {
		n = queuebehead(&q);
		writenode(n, fp);
		putc('\n', fp);
		++lines;
		noderelease(n);
	}
	ok = fclose(fp) != EOF;
	if (!lines)
		/* Try to */ unlink(filename); /***** UNIX! *****/
	return ok;
}
#endif /* SAVEBUF */

#ifdef SAVEBUF
#ifdef EDITRACE
extern FILE *dumpfp;

Visible Procedure dumpev(ep, m) register environ *ep; string m;
{
	char stuff[80];
	register string str = stuff;
	path pa;
	node n;
	int ich;
	int y,x,l;
	static int idump;
	
	if (dumpfp == NULL)
		return;
	
	idump++;
	fprintf(dumpfp, "+++ EV %d: %s +++\n", idump, m);
	
	switch (ep->mode) {
	case VHOLE:
		sprintf(str, "VHOLE:%d.%d", ep->s1, ep->s2);
		break;
	case FHOLE:
		sprintf(str, "FHOLE:%d.%d", ep->s1, ep->s2);
		break;
	case ATBEGIN:
		str = "ATBEGIN";
		break;
	case ATEND:
		str = "ATEND";
		break;
	case WHOLE:
		str = "WHOLE";
		break;
	case SUBRANGE:
		sprintf(str, "SUBRANGE:%d.%d-%d", ep->s1, ep->s2, ep->s3);
		break;
	case SUBSET:
		sprintf(str, "SUBSET:%d-%d", ep->s1, ep->s2);
		break;
	case SUBLIST:
		sprintf(str, "SUBLIST...%d", ep->s3);
		break;
	default:
		sprintf(str, "UNKNOWN:%d,%d,%d,%d",
			ep->mode, ep->s1, ep->s2, ep->s3);
	}
	n= tree(ep->focus);
	fprintf(dumpfp,
		"%s, %s, wi=%d, hi=%d, (y,x,l)=(%d,%d,%d) %s %s\n",
		(Is_etext(n) ? "<TEXT> " : symname(symbol(n))),
		str, nodewidth(n), ep->highest,
		Ycoord(ep->focus), Xcoord(ep->focus), Level(ep->focus),
		ep->spflag ? "spflag on" : "",
		ep->changed ? "changed" : "");
	writenode(n, dumpfp);
	pa= parent(ep->focus);
	while (pa != NilPath) {
		ich= ichild(pa);
		y= Ycoord(pa);
		x= Xcoord(pa);
		l= Level(pa);
		n= tree(pa);
		fprintf(dumpfp,
			"\nIN PARENT wi=%d (y,x,l)=(%d,%d,%d) AS CHILD %d:\n",
			nodewidth(n), y, x, l, ich);
		writenode(n, dumpfp);
		pa= parent(pa);
	}
	fprintf(dumpfp, "\n");
	VOID fflush(dumpfp);
}
#endif /*DUMPEV*/
#endif /*SAVEBUF*/
