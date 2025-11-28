/* The equivalent of the Unix 'sync' system call: FlushVol.
   For now, we only flush the default volume
   (since that's the only volume written to by MacB). */

#include "macdefs.h"

int
sync()
{
	if (FlushVol((char*)NULL, 0) == noErr)
		return 0;
	else {
		errno= ENODEV;
		return -1;
	}
}
