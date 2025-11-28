/* Chdir, mkdir, rmdir for the Macintosh.
   Pathnames must be Macintosh paths, with colons as separators. */

#include "macdefs.h"

/* find working directory reference number for some volume or directory */

static short wdrefnum(path) char *path; {
	union {
		WDPBRec d;
		VolumeParam v;
	} pb;
	char name[MAXPATH];
	short refnum;
	short err;
	
	strncpy(name, path, sizeof name);
	name[MAXPATH-1]= EOS;
	pb.d.ioNamePtr= (StringPtr) c2pstr(name);
	pb.d.ioVRefNum= 0;
	if (hfsrunning()) {
		pb.d.ioWDProcID= 'Mabc';
		pb.d.ioWDDirID= 0;
		err= PBOpenWD(&pb, FALSE);
	}
	else {
		pb.v.ioVolIndex= -1;	/* not 0; see IM IV p. 129 */
		err= PBGetVInfo(&pb, FALSE);
	}
	if (err == noErr)
		refnum= pb.d.ioVRefNum;
	else
		refnum= 0;
	return refnum;
}

/* Change current directory.
 * This has been hacked to work with ABC without using PBHSetVol();
 * timo@cwi.nl Jan 1990 */

static short abcwdrefnum= 0;
	/* abc's current working directory reference number */

/* release working directory.
 * also called from endfile() to free last one used
 * (should be chdir(startdir) from endbws() in abc).
 */
void endabcwd() {
	WDPBRec pb;
	
	if (hfsrunning() && abcwdrefnum != 0) {
		pb.ioVRefNum= abcwdrefnum;
		pb.ioNamePtr= (StringPtr) NULL;
		(void) PBCloseWD(&pb, FALSE);
		abcwdrefnum= 0;
	}
}

static int mchdir(path) char *path; {
	WDPBRec pb;
	char name[MAXPATH];
	
	endabcwd();
	abcwdrefnum= wdrefnum(path);
	if (abcwdrefnum != 0
		&& SetVol((char*)NULL, abcwdrefnum) == noErr)
	{
		return 0;
	}
	/* else: */
	errno= ENOENT;
	return -1;
}

/* save current abcdir;
 * to guard against DA's like chooser that chdir(root)
 */
#include "bmem.h"

char *abcdir= NULL;		/* must be Visible for m1print.c */

int chdir(path) char *path; {
	if (abcdir) {
		freestr(abcdir);
		abcdir= NULL;
	}
	if (mchdir(path) == 0) {
		abcdir= savestr(path);
		return 0;
	}
	/* else */
	return -1;
}

void recabcdir() {
	mchdir(abcdir);
}

/* Create a directory. */

int
mkdir(path)
	char *path;
{
	HFileParam pb;
	char name[MAXPATH];
		
	if (!hfsrunning()) {
		errno= ENODEV;
		return -1;
	}
	strncpy(name, path, sizeof name);
	pb.ioNamePtr= (StringPtr) c2pstr(name);
	pb.ioVRefNum= 0;
	pb.ioDirID= 0;
	if (PBDirCreate(&pb, FALSE) != noErr) {
		errno= EACCES;
		return -1;
	}
	return 0;
}

int
rmdir(path)
	char *path;
{
	HFileParam pb;
	char name[MAXPATH];
	
#ifdef TRACEDIR
putSstr(stdout, "rmdir(%s): ", path);
#endif
	if (!hfsrunning()) {
		errno= ENODEV;
#ifdef TRACEDIR
putstr(stdout, "no hfs\n");
#endif
		return -1;
	}
	strncpy(name, path, sizeof name);
	pb.ioNamePtr= (StringPtr) c2pstr(name);
	pb.ioVRefNum= 0;
	pb.ioDirID= 0;
	if (PBHDelete(&pb, FALSE) != noErr) {
		errno= EACCES;
#ifdef TRACEDIR
putstr(stdout, "failed\n");
#endif
		return -1;
	}
#ifdef TRACEDIR
putstr(stdout, "succeeded\n");
#endif
	return 0;
}

/*
 * Macintosh version of UNIX directory access package
 * (opendir, readdir, closedir).
 */

#include "dir.h"

DIR opened;

/*
 * Open a directory.  This means calling PBOpenWD.
 * The value returned is always the address of opened, or NULL.
 * (I have as yet no use for multiple open directories; this could
 * be implemented by allocating memory dynamically.)
 */

DIR *
opendir(path)
	char *path;
{
	union {
		WDPBRec d;
		VolumeParam v;
	} pb;
	char ppath[MAXPATH];
	short refnum;
	
	if (opened.nextfile != 0)
		return NULL; /* A directory is already open. */
	refnum= wdrefnum(path);
	if (refnum == 0)
		return NULL;
	opened.dirid= refnum;
	opened.nextfile= 1;
	return &opened;
}

/*
 * Close a directory.
 */

closedir(dirp)
	DIR *dirp;
{
	if (hfsrunning()) {
		WDPBRec pb;
		
		pb.ioVRefNum= dirp->dirid;
		(void) PBCloseWD(&pb, FALSE);
	}
	dirp->dirid= 0;
	dirp->nextfile= 0;
}

/*
 * Read the next directory entry.
 */

struct direct *
readdir(dp)
	DIR *dp;
{
	union {
		DirInfo d;
		FileParam f;
		HFileInfo hf;
	} pb;
	short err;
	static struct direct dir;
	
	dir.d_name[0]= 0;
	pb.d.ioNamePtr= dir.d_name;
	pb.d.ioVRefNum= dp->dirid;
	pb.d.ioFDirIndex= dp->nextfile++;
	pb.d.ioDrDirID= 0;
	if (hfsrunning())
		err= PBGetCatInfo(&pb, FALSE);
	else {
		pb.f.ioFVersNum= 0;
		err= PBGetFInfo(&pb, FALSE);
	}
	if (err != noErr)
		return NULL;
	(void) p2cstr(dir.d_name);
	return &dir;
}
