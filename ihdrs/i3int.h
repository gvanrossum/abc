/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1986. */

/* Interpreter */

value pop();
/* Procedure push(); */

#define Stop ((parsetree)zero) /* Legal stop */
#define Halt NilTree /* Illegal stop (loose end of code) */

extern int call_level;

#define _Thread(t) *Branch(t, Nbranches(t)) /* Next instruction */
#define _Thread2(t) *Branch(t, Nbranches(t)+1) /* Alternate next instr. or flag */

#ifdef MSDOS
#ifdef M_I86LM

#define Thread(t) (*_thread(t))
#define Thread2(t) (*_thread2(t))
parsetree *_thread();
parsetree *_thread2();

#endif /* M_I86LM */
#endif /* MSDOS */

#ifndef Thread
#define Thread(t) _Thread(t)
#endif
#ifndef Thread2
#define Thread2(t) _Thread2(t)
#endif
