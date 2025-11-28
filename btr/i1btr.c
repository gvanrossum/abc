/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1986. */

#include "b.h"
#include "bmem.h"
#include "i1btr.h"
#include "i1tlt.h"

/*********************************************************************/
/* grab, copy, release of btree(node)s                               */
/*********************************************************************/

Visible btreeptr
grabbtreenode(flag, it)
	literal flag; literal it;
{
	btreeptr pnode; unsigned syz;
	static intlet isize[]= {
		sizeof(itexnode), sizeof(ilisnode),
		sizeof(itabnode), sizeof(itabnode)};
	static intlet bsize[]= {
		sizeof(btexnode), sizeof(blisnode),
		sizeof(btabnode), sizeof(btabnode)};
	switch (flag) {
	case Inner:
		syz= isize[it];
		break;
	case Bottom:
		syz= bsize[it];
		break;
	case Irange:
	case Crange:
		syz = sizeof(rangenode);
		break;
	}
	pnode = (btreeptr) getmem((unsigned) syz);
	Refcnt(pnode) = 1;
	Flag(pnode) = flag;
	return(pnode);
}

/* ----------------------------------------------------------------- */

Visible btreeptr copybtree(pnode) btreeptr pnode; {
	if (pnode != Bnil && Refcnt(pnode) < Maxrefcnt) ++Refcnt(pnode);
	return(pnode);
}

Visible Procedure uniqlbtreenode(pptr, it) btreeptr *pptr; literal it; {
	if (*pptr NE Bnil && Refcnt(*pptr) > 1) {
		btreeptr qnode = *pptr;
		*pptr = ccopybtreenode(*pptr, it);
		relbtree(qnode, it);
	}
}

Visible btreeptr ccopybtreenode(pnode, it) btreeptr pnode; literal it; {
	intlet limp;
	btreeptr qnode;
	intlet iw;
	
	iw = Itemwidth(it);
	qnode = grabbtreenode(Flag(pnode), it);
	Lim(qnode) = limp = Lim(pnode);
	Size(qnode) = Size(pnode);
	switch (Flag(qnode)) {
	case Inner:
		cpynitms(Piitm(qnode, 0, iw), Piitm(pnode, 0, iw), limp, it);
		cpynptrs(&Ptr(qnode, 0), &Ptr(pnode, 0), limp+1);
		break;
	 case Bottom:
		cpynitms(Pbitm(qnode, 0, iw), Pbitm(pnode, 0, iw), limp, it);
		break;
	case Irange:
	case Crange:
		Lwbval(qnode) = copy(Lwbval(pnode));
		Upbval(qnode) = copy(Upbval(pnode));
		break;
	default:
		syserr(MESS(400, "unknown flag in ccopybtreenode"));
	}
	return(qnode);
}

/* make a new root (after the old ptr0 split) */

Visible btreeptr mknewroot(ptr0, pitm0, ptr1, it)
	btreeptr ptr0, ptr1; itemptr pitm0; literal it;
{
	int r;
	intlet iw = Itemwidth(it);
	btreeptr qnode = grabbtreenode(Inner, it);
	Ptr(qnode, 0) = ptr0;
	movnitms(Piitm(qnode, 0, iw), pitm0, 1, iw);
	Ptr(qnode, 1) = ptr1;
	Lim(qnode) = 1;
	r= Sincr(Size(ptr0));
	Size(qnode) = Ssum(r, Size(ptr1));
	return(qnode);
}

/* ----------------------------------------------------------------- */

/* release btree */

Visible Procedure relbtree(pnode, it) btreeptr pnode; literal it; {
	width iw;
	
	iw = Itemwidth(it);
	if (pnode EQ Bnil)
		return;
	if (Refcnt(pnode) EQ 0) {
		syserr(MESS(401, "releasing unreferenced btreenode"));
		return;
	}
	if (Refcnt(pnode) < Maxrefcnt && --Refcnt(pnode) EQ 0) {
		intlet l;
		switch (Flag(pnode)) {
		case Inner:
			for (l = 0; l < Lim(pnode); l++) {
				relbtree(Ptr(pnode, l), it);
				switch (it) {
				case Tt:
				case Kt:
					release(Ascval(Piitm(pnode, l, iw)));
				case Lt:
					release(Keyval(Piitm(pnode, l, iw)));
				}
			}
			relbtree(Ptr(pnode, l), it);
			break;
		case Bottom:
			for (l = 0; l < Lim(pnode); l++) {
				switch (it) {
				case Tt:
				case Kt:
					release(Ascval(Pbitm(pnode, l, iw)));
				case Lt:
					release(Keyval(Pbitm(pnode, l, iw)));
				}
			}
			break;
		case Irange:
		case Crange:
			release(Lwbval(pnode));
			release(Upbval(pnode));
			break;
		default:
			syserr(MESS(402, "wrong flag in relbtree()"));
		}
		freemem((ptr) pnode);
	}
}

