/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1988. */

value get_names();
bool abcfile();
bool abcworkspace();
bool unitfile();
bool targetfile();
char *base_fname();
bool typeclash();

extern bool ws_recovered;
extern bool gr_recovered;

#define IO_NAME		MESS(4600, "*** %s isn't the name of a location\n")
#define O_INIT		MESS(4601, "*** %s hasn't been initialised\n")
#define O_TABLE		MESS(4602, "*** %s isn't a table\n")
#define R_ERROR		MESS(4603, "*** Errors while recovering workspace:\n")
#define R_TNAME		MESS(4604, "*** %s: cannot derive a location name\n")
#define R_FREAD		MESS(4605, "*** %s: cannot read this file\n")
#define R_UNAME		MESS(4606, "*** %s: cannot derive a how-to name\n")
#define R_RENAME	MESS(4607, "*** %s: cannot rename this file\n")
#define R_EXIST		MESS(4608, "*** %s: the ABC name for this file is already in use\n")
#define R_FWRITE	MESS(4609, "*** %s: cannot create this file\n")
#define G_ERROR		MESS(4610, "*** Errors while recovering the workspace index\n")
#define G_DNAME		MESS(4611, "*** %s: cannot derive an ABC name for this workspace\n")
#define G_EXIST		MESS(4612, "*** %s: the ABC name for this workspace is already in use\n")
