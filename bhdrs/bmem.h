/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1986. */

/* B memory management */

typedef char *ptr;
#define Nil ((ptr) 0)

ptr getmem();
ptr savestr();
#define freestr(s) (freemem((ptr)(s)))

#ifdef MEMTRACE
typedef unsigned long address;	/* for PC and symbol table (on a tahoe) */
#define F_ALLOC 'A'
#define F_FREE  'F'
#endif

struct bufadm {char *buf, *pbuf, *end; };
typedef struct bufadm bufadm;
