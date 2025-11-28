/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1990. */

#include "b.h"
#include "main.h"
#include "port.h"

#include <sys\types.h>
#include <sys\stat.h>
#include <time.h>
#include <sys\timeb.h>

/* synonyms for current and parent directory */
#define SYN_CURDIR "."
#define SYN_PARENTDIR ".."

/**************************************************************************/

/* getseed () - initialise the random generator with a random number */

Visible int getseed() {
	return (int) time((time_t *)0);
}

/**************************************************************************/

Visible Procedure getdatetime(year, month, day, hour, minute,
			      sec, fraction, units)
     int *year, *month, *day;
     int *hour, *minute, *sec;
     long *fraction, *units;
{
	struct tm *lt;
	struct timeb tt;

	ftime(&tt);
	lt = localtime((long*) &tt.time);
	
	*year = lt->tm_year + 1900;
	*month = lt->tm_mon + 1;
	*day = lt->tm_mday;
	*hour = lt->tm_hour;
	*minute = lt->tm_min;
	*sec = lt->tm_sec;
	*fraction = tt.millitm;
	*units = 1000;
}

/**************************************************************************/

Visible Porting char *curdir()
{
	static char dir[64];
	extern char *getcwd();

	return getcwd(dir, sizeof dir - 1);
}

/**************************************************************************/

#define SEPARATOR '\\'
#define ALT_SEP   '/'
#define Issep(c) ((c) == SEPARATOR || (c) == ALT_SEP)
#define Isanysep(c) (Issep(c) || (c) == DRIVE_SEP)

Visible bool is_path(path)
     char *path;
{
	if (path == NULL) return No;
	if (strcmp(path, SYN_CURDIR) == 0 || strcmp(path, SYN_PARENTDIR) == 0)
	  return Yes;
	for (; *path; path++) {
		if (Isanysep(*path)) return Yes;
	}
	return No;
}

/**************************************************************************/

Visible bool is_abspath(path)
     char *path;
{
	if (path == NULL) return No;
	if (Issep(*path)) return Yes;
	if (path[0] != '\0' && path[1] == DRIVE_SEP && Issep(path[2]))
	  return Yes;

	 return No;
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

Visible Porting long filemodtime(filename)
     char *filename;
{
	struct stat statbuf;
	
	if (stat(filename, &statbuf) == 0)
	  return (long) statbuf.st_mtime;
	else
	  return 0L;
}

