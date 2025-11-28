/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1988. */

/* memory handling for ABC values: grabbing, copying and releasing */

#include "b.h"
#include "bint.h"
#include "bedi.h"
#include "bmem.h"
#include "bobj.h"

#define Adj(s) (unsigned) (Hdrsize+(s))
#define Unadj(s) (unsigned) ((s)-Hdrsize)

#define Grabber() {if(len>Maxintlet)syserr(MESS(1500, "big grabber"));}
#define Regrabber() {if(len>Maxintlet)syserr(MESS(1501, "big regrabber"));}

#define Offset(type) (type == Nod ? NodOffset : 0)

/******************************* Grabbing **********************************/

Hidden unsigned getsyze(type, len, pnptrs) literal type; intlet len;
		int *pnptrs; {
	register unsigned syze= 0;
	int nptrs= 0;
	switch (type) {
		case Tex:
		case ELT:
		case Lis:
		case Ran:
		case Tab:
			syze= tltsyze(type, len, &nptrs);
			break;
		case Num:
			syze= numsyze(len, &nptrs);
			break;
		case Ptn:
			syze= ptnsyze(len, &nptrs);
			break;
		case Rangebounds:
		case Com:
			syze= len*sizeof(value); nptrs= len;
			break;
		case Sim:
			syze= sizeof(simploc); nptrs= 1;
			break;
		case Tri:
			syze= sizeof(trimloc); nptrs= 3;
			break;
		case Tse:
			syze= sizeof(tbseloc); nptrs= 2;
			break;
		case How:
			syze= sizeof(how); nptrs= 1;
			break;
		case Ind:
			syze= sizeof(indirect); nptrs= 1;
			break;
		case Fun:
		case Prd:
			syze= sizeof(funprd); nptrs= 1;
			break;
		case Ref:
			syze= sizeof(ref); nptrs= 1;
			break;
		case Nod:
			syze= sizeof(struct node) - Hdrsize - sizeof(node)
				+ len*sizeof(node);
			nptrs= len;
			break;
		case Pat:
			syze= sizeof(struct path) - Hdrsize; nptrs= 2;
			break;
		case Etex:
			syze= (len+1)*sizeof(char); nptrs= 0;
			break;
		default:
#ifndef NDEBUG
			putsCerr("\ngetsyze{%c}\n", type);
#endif
			syserr(MESS(1502, "getsyze called with unknown type"));
	}
	if (pnptrs != NULL) *pnptrs= nptrs;
	return syze;
}

Visible value grab(type, len) literal type; intlet len; {
	unsigned syze= getsyze(type, len, (int*)NULL);
	value v;
	Grabber();
	v= (value) getmem(Adj(syze));
	v->type= type; v->len= len; v->refcnt= 1;
	return v;
}

Visible Procedure regrab(v, len) value *v; intlet len; {
	literal type= (*v)->type;
	unsigned syze= getsyze(type, len, (int*)NULL);
	Regrabber();
	regetmem((ptr *) v, Adj(syze));
	(*v)->len= len;
}

/******************************* Copying and releasing *********************/

Visible value copy(v) value v; {
	if (v != Vnil && !IsSmallInt(v) && Refcnt(v) < Maxrefcnt) 
		++Refcnt(v);
	return v;
}

Visible Procedure release(v) value v; {
	if (v == Vnil || IsSmallInt(v)) return;
	if (Refcnt(v) == 0)
		syserr(MESS(1503, "releasing unreferenced value"));
	if (Refcnt(v) < Maxrefcnt && --Refcnt(v) == 0)
		rel_subvalues(v);
}

Hidden value ccopy(v) value v; {
	literal type= v->type; intlet len; value w;
	int nptrs; unsigned syze; register string from, to, end;
	register value *pp, *pend;
	len= Length(v);
	syze= getsyze(type, len, &nptrs);
	Grabber();
	w= (value) getmem(Adj(syze));
	w->type= type; w->len= len; w->refcnt= 1;
	from= Str(v); to= Str(w); end= to+syze;
	while (to < end) *to++ = *from++;
	pp= (value*) ((char*)Ats(w) + Offset(type));
	pend= pp+nptrs;
	for (; pp < pend; pp++) VOID copy(*pp);
	return w;
}

Visible Procedure uniql(ll) value *ll; {
	if (*ll != Vnil && !IsSmallInt(*ll) && Refcnt(*ll) > 1) {
		value c= ccopy(*ll);
		release(*ll);
		*ll= c;
	}
}

Visible Procedure rrelease(v) value v; {
	literal type= v->type; intlet len= Length(v);
	int nptrs; register value *pp, *pend;
	VOID getsyze(type, len, &nptrs);
	pp= (value*) ((char*)Ats(v) + Offset(type));
	pend= pp+nptrs;
	while (pp < pend) release(*pp++);
	v->type= '\0';
	freemem((ptr) v);
}
