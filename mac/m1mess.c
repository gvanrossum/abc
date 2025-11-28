/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1986. */

/* B error message handling, Macintosh version. */

/* Error messages are packed in STR# resources.  Message 100*k + i
   is found in STR# resource with ID=k and has number i+1 (because
   the strings number starting at 1).  The English version
   of the database can be recreated from the program sources by scanning
   for the pattern "MESS".  This is a macro whose first argument is
   the message number and whose second number is the message string;
   this macro expands to only the message number which is passed to
   the error routines.	The error routines then dig the message from
   the resources, or just print the number if the file can't be
   opened.  There is also a way to pass a message that is determined
   at runtime.
*/

#include "b.h"
#include "bmem.h"
#include "bobj.h"

char *strchr();
string strcpy();

#define MESSBUFSIZE 256

Visible char *messbuf= NULL; /* used for messages with arguments */
Hidden  char *buf= NULL;

Visible string 
getmess(nr)
	int nr; 
{
	if (buf == NULL)
		return "MEMORY?";
		/* see below */
	if (nr <= 0) 
		return nr == -1 ? "%s" : nr == -2 ? "%s%s" : "";
	GetIndString(buf, nr/100, (nr%100)+1);
	if (buf[0] == '\0')
		sprintf(buf, "(%d)", nr);
	return buf;
}

Visible Procedure
initmess()
{
	if (buf == NULL)
		buf= (char*) getmem(MESSBUFSIZE);
		/* potentially dangerous, since getmem calls getmess
		   in memexh(); but we had too much static data for MPW */
	if (messbuf == NULL)
		messbuf= (char*) getmem(MESSBUFSIZE);
}

Visible Procedure
endmess()
{
}
