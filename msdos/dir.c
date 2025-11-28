/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1990. */

/*
 * MS-DOS version of UNIX directory access package.
 * (opendir, readdir, closedir).
 */

#include "b.h"

#ifndef HAS_READDIR

#include "dir.h"

#include <dos.h>
#include <errno.h>
#include <sys\types.h>
#include <sys\stat.h>

#define MAXPATH 64

Forward Hidden char *firstf();
Forward Hidden char *nextf();
Forward Hidden Procedure fixfile();
Forward Hidden Procedure setdta();

#define EOS '\0'

Hidden char dirpath[MAXPATH+1]= "";
Hidden bool busy= No;

/*
 * Open a directory.
 * This MS-DOS version allows only one directory to be open at a time.
 * The path must exist and be a directory.
 * Then it only saves the file name and returns a pointer to it.
 */

Visible DIR *opendir(path)
     char *path;
{
	struct stat buffer;

	if (dirpath[0] != EOS) {
		 errno= EMFILE;
		 return NULL; /* Only one directory can be open at a time */
	}
	if (stat(path, &buffer) == -1)
		return NULL; /* Directory doesn't exist */
	if ((buffer.st_mode & S_IFMT) != S_IFDIR) {
		errno= ENOENT;
		return NULL; /* Not a directory */
	}
	strcpy(dirpath, path);
	path= dirpath + strlen(dirpath);
	if (path > dirpath && path[-1] != ':' && path[-1] != '/' &&
			path[-1] != '\\')
		*path++ = '\\';
	strcpy(path, "*.*");
	busy= No; /* Tell readdir this is the first time */
	return (DIR *) dirpath;
}

/*
 * Read the next directory entry.
 */

Visible struct direct *readdir(dp)
     char *dp;
{
	static struct direct buffer;
	char *p;

	if (!busy)
		p= firstf((char *)dp);
	else
		p= nextf();
	busy= (p != NULL);
	if (!busy)
		return NULL;
	strcpy(buffer.d_name, p);
	buffer.d_namlen= strlen(p);
	return &buffer;
}


/*
 * Close a directory.
 * We must clear the first char of dirpath to signal opendir that
 * no directory is open.
 */

Visible Procedure closedir(dirp)
     DIR *dirp;
{
	dirp[0]= EOS;
}

/*
 * Low-level implementation -- access to DOS system calls FIRST and NEXT.
 */

Hidden char dta[64]; /* Device Transfer Area */

#define DIRATTR 0x10

Hidden char *firstf(pat)
     char *pat;
{
	static int flags;
	union REGS regs;
	struct SREGS segregs;

	setdta(dta);

	segread(&segregs);
	regs.h.ah = 0x4e;
	regs.x.cx = DIRATTR;
#ifdef M_I86LM /* Large Model -- far pointers */
	regs.x.dx = FP_OFF(pat);
	segregs.ds = FP_SEG(pat);
#else
	regs.x.dx = (int) pat;
#endif
	flags= intdosx(&regs, &regs, &segregs);
	if (regs.x.cflag)
	  return NULL;
	fixfile();
	return dta + 30;
}

Hidden char *nextf()
{
	int flags;
	union REGS regs;

	setdta(dta);

	regs.h.ah = 0x4f;
	flags= intdos(&regs, &regs);
	if (regs.x.cflag)
	  return NULL;
	fixfile();
	return dta + 30;
}

/*
 * Convert the file name in the Device Transfer Area to lower case.
 */

Hidden Procedure fixfile()
{
	char *cp;

	for (cp= dta+30; *cp != EOS; ++cp)
	  *cp= tolower(*cp);
}

/*
 * Tell MS-DOS where the Device Transfer Area is.
 */

Hidden Procedure setdta(dta)
     char *dta;
{
	union REGS regs;
	struct SREGS segregs;

	segread(&segregs);
	regs.h.ah = 0x1a;
#ifdef M_I86LM /* Large Model -- far pointers */
	regs.x.dx = FP_OFF(dta);
	segregs.ds = FP_SEG(dta);
#else
	regs.x.dx = (int) dta;
#endif
	intdosx(&regs, &regs, &segregs);
}

#endif /* !HAS_READDIR */
