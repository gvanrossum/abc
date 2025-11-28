/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1990. */

/* Generic version, as basis for porting.
 * See Portability Guide (./PORTING.DOC) for details.
 */

#include "b.h"
#include "main.h" /* for freepath() */
#include "port.h" /* for makepath() */

#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#ifdef HAS_FTIME
#include <sys/timeb.h>
#endif

/* synonyms for current and parent directory */
#define SYN_CURDIR    "."
#define SYN_PARENTDIR ".."

/**************************************************************************/

#ifndef HAS_RENAME

/* rename() - rename a file */

Visible int rename(from, to)
     char *from, *to;
{
}

#endif /* HAS_RENAME */

/**************************************************************************/

#ifndef HAS_MKDIR

/* mkdir() - make a directory */

Visible int mkdir(dir, mode)
     char *dir; int mode;
{
}

/* rmdir() - remove directory */

Visible int rmdir(dir)
     char *dir;
{
}

#endif /* !HAS_MKDIR */

/**************************************************************************/

/* getseed () - initialise the random generator with a random number */

Visible int getseed()
{
	return (int) getpid();   /* or for instance a time() call */
}

/**************************************************************************/

/* getdatetime() - get the date and time */

Visible Procedure getdatetime(year, month, day, hour, minute,
			      sec, fraction, units)
     int *year, *month, *day;
     int *hour, *minute, *sec;
     long *fraction, *units;
{
#ifdef HAS_FTIME

	struct tm *lt;
	struct timeb tt;

	ftime(&tt);
	lt = localtime((long*) &tt.time);
	
	*year     = lt->tm_year + 1900;
	*month    = lt->tm_mon + 1;
	*day      = lt->tm_mday;
	*hour     = lt->tm_hour;
	*minute   = lt->tm_min;
	*sec      = lt->tm_sec;
	*fraction = tt.millitm;
	*units    = 1000;

#else /* !HAS_FTIME */

	long ttt;
	struct tm *lt;

	ttt = time((long*)0);
	lt = localtime(&ttt);

	*year     = lt->tm_year + 1900;
	*month    = lt->tm_mon + 1;
	*day      = lt->tm_mday;
	*hour     = lt->tm_hour;
	*minute   = lt->tm_min;
	*sec      = lt->tm_sec;
	*fraction = *units= 0;
	

#endif /* !HAS_FTIME */
}

/**************************************************************************/

/* curdir() - return the current directory */

Visible Porting char *curdir()
{
	static char buffer[1024];
#ifdef HAS_GETCWD
	char *getcwd();
	return getcwd(buffer, 1024);
#else
	char *getwd();
	return getwd(buffer);
#endif
}

#ifndef HAS_GETWD
#ifndef HAS_GETCWD

/* neither getwd, nor getcwd provided; define own: */

/* getwd() - get working directory */

Hidden char *getwd(buf)
     char *buf;
{
	FILE *fp;

	*buf = 0;
	if ((fp = popen("pwd", "r")) == 0 ) {
		strcpy(buf, "cannot execute pwd");
		return(0);
	}
	if (fgets(buf, 1024, fp) )
		buf[strlen(buf)-1] = 0;
	pclose(fp);
	if (*buf == '/')
		return buf;
	else
		return (char*) NULL;
}

#endif /* !HAS_GETCWD */
#endif /* !HAS_GETWD */

/**************************************************************************/

/* is_path() - is the string argument a path ? */

#define SEPARATOR '/'
#define Issep(c) ((c) == SEPARATOR)

Visible bool is_path(path)
     char *path;
{
	if (path == NULL)
	  return No;
	if (strcmp(path, SYN_CURDIR) == 0 || strcmp(path, SYN_PARENTDIR) == 0)
	  return Yes;
	for (; *path; path++)
	  if (Issep(*path)) return Yes;
	return No;
}

/**************************************************************************/

/* is_abspath() - is the path an absolute path, rather than relative to
 *                the current directory.
 */

Visible bool is_abspath(path)
     char *path;
{
	if (path == NULL) return No;
	else return Issep(*path);
}

/**************************************************************************/

Visible bool is_directory(dir, name)
     char *dir;
     char *name;
{
	struct stat statbuf;
	char *path;

	if (dir == NULL || name == NULL) return No;
	path= makepath(dir, name);
	if (stat(path, &statbuf) == 0 &&
	    ((statbuf.st_mode & S_IFMT) == S_IFDIR) &&
	    (strcmp(name, SYN_CURDIR) != 0 && strcmp(name, SYN_PARENTDIR) != 0)
           ) {
		freepath(path);
		return Yes;
	}
	freepath(path);
	return No;
}

/**************************************************************************/
/**************************************************************************/

/* filemodtime() - returns the file last modify time
 *                 only used inside this directory (edit.c)
 */

Visible Porting long filemodtime(filename)
     char *filename;
{
	struct stat statbuf;
	
	if (stat(filename, &statbuf) == 0)
	  return (long) statbuf.st_mtime;
	else
	  return 0L;
}


