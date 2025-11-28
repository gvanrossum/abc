/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1986. */

#include "b.h"
#include "bedi.h"
#include "bobj.h"
#include "etex.h"
#include "bmem.h"

Visible int e_length(v) value v; {
	return Length(v);
}

Visible value mk_etext(m) string m; {
	value v; intlet len= strlen(m);
	v= grab(Etex, len);
	strcpy(Str(v), m);
	return v;
}

Visible char e_ncharval(n, v) int n; value v; {
	return *(Str(v)+n-1);
}

Visible string e_strval(v) value v; {
	return Str(v);
}


Visible string e_sstrval(v) value v; {
	return (string) savestr(Str(v));
}

Visible Procedure e_fstrval(s) string s; {
	freestr(s);
}


Visible value e_icurtail(v, k) value v; int k; {
	value w= grab(Etex, k);
	strncpy(Str(w), Str(v), k);
	*(Str(w) + k)= '\0';
	return w;
}


Visible value e_ibehead(v, k) value v; int k; {
	value w= grab(Etex, Length(v) - (k - 1));
	strcpy(Str(w), Str(v) + k - 1);
	return w;
}



Visible value e_concat(s, t) value s, t; {
	value v= grab(Etex, Length(s) + Length(t));
	strcpy(Str(v), Str(s));
	strcpy(Str(v) + Length(s), Str(t));
	return v;
}

Visible Procedure e_concto(s, t) value *s, t; {
	value v= *s;
	*s= e_concat(*s, t);
	release(v);
}
