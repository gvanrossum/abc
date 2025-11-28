/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1989. */

#include "b.h"
#include "bobj.h"
#include "i1num.h"

#include <Types.h>
#include <OSUtils.h>

Visible value nowisthetime() {
	long ttt;
	DateTimeRec date;
	value now;

	GetDateTime(&ttt);
	Secs2Date(ttt, &date);
			
	now= mk_compound(6);

	*Field(now, 0)= mk_integer(date.year);
	*Field(now, 1)= mk_integer(date.month);
	*Field(now, 2)= mk_integer(date.day);
	*Field(now, 3)= mk_integer(date.hour);
	*Field(now, 4)= mk_integer(date.minute);
	*Field(now, 5)= mk_integer(date.second);
	return now;
}
