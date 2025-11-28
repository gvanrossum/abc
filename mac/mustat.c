/* Minimal 'stat' emulation: tells directories from files and
   gives length and mtime.
   I'm not sure whether the time should be in Mac format or
   converted to Unix format; currently it's Unix format. */

#include "sys_stat.h"
#include "macdefs.h"

/* Bits in ioFlAttrib: */
#define LOCKBIT	(1<<0)		/* File locked */
#define DIRBIT	(1<<4)		/* It's a directory */

char *c2pstr();
char *strcpy();

int
stat(path, buf)
	char *path;
	struct stat *buf;
{
	union {
		DirInfo d;
		FileParam f;
		HFileInfo hf;
	} pb;
	char name[256];
	short err;
	pb.d.ioNamePtr= c2pstr(strcpy(name, path));
	pb.d.ioVRefNum= 0;
	pb.d.ioFDirIndex= 0;
	pb.d.ioDrDirID= 0;
	if (hfsrunning())
		err= PBGetCatInfo(&pb, FALSE);
	else {
		pb.f.ioFVersNum= 0;
		err= PBGetFInfo(&pb, FALSE);
	}
	if (err != noErr)
		return -1;
	if (pb.d.ioFlAttrib & LOCKBIT)
		buf->st_mode= 0444;
	else
		buf->st_mode= 0666;
	if (pb.d.ioFlAttrib & DIRBIT) {
		buf->st_mode |= 0111 | S_IFDIR;
		buf->st_size= pb.d.ioDrNmFls;
		buf->st_mtime= pb.d.ioDrMdDat;
	}
	else {
		buf->st_mode |= S_IFREG;
		buf->st_size= pb.f.ioFlLgLen;
		buf->st_mtime= pb.f.ioFlMdDat;
	}
	return 0;
}
