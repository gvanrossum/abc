/* Macintosh emulation of Unix 'access()' system call.

   This version ignores the mode flags; it assumes files can always
   be read or written when they exist.  This is more or less true,
   except on write-protected volumes and maybe in a shared file system
   situation.  Note that the Finder's 'locked' bit is ignored by
   the file system so you can still write such files from within
   an application.
   Execute permission might check the file type and return Yes
   if this is APPL, but I have no use for it right now anyway,
   so why bother. */

#include "macdefs.h"

int
access(path, mode)
	char *path;
	int mode;
{
	union {
		DirInfo d;
		FileParam f;
		HFileInfo hf;
	} pb;
	char name[MAXPATH];
	int err;
	
	strncpy(name, path, sizeof name);
	pb.d.ioNamePtr= (StringPtr) c2pstr(name);
	pb.d.ioVRefNum= 0;
	pb.d.ioFDirIndex= 0;
	pb.d.ioDrDirID= 0;
	if (hfsrunning())
		err= PBGetCatInfo(&pb, FALSE);
	else {
		pb.f.ioFVersNum= 0;
		err= PBGetFInfo(&pb, FALSE);
	}
	if (err != noErr) {
		errno= ENOENT;
		return -1;
	}
	return 0;
}