/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1989. */

#define ABC_RELEASE "ABC Release %s."
#define COPYRIGHT "Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1989."
#define HEADING "This program allows you to redefine the key bindings for the ABC editor operations, producing a key definitions file."

/****************************************************************************/

#define E_ILLEGAL	"*** You are not allowed to start a definition with '%c' since that would make that character unavailable."

#define E_TOO_MANY 	"*** Sorry, can't remember more key definitions"

#define E_UNLAWFUL	"*** It may not contain an unprintable character"

#define E_IN_USE	"*** That representation is in use for %s."

#define E_UNKNOWN	"*** unknown operation name"

#define E_KEYFILE	"*** Can't open key definitions file %s for writing; writing to standard output instead."

#ifndef CANLOOKAHEAD

#define E_INTERRUPT	"*** You cannot include your interrupt character."

#define E_NOTALLOWED	"*** you are not allowed to change \"%s\""

#endif

