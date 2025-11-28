/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1988. */

/* Interface between mac and abc stuff */

#define NOABOUTPICT MESS(8500, "No About MacABC picture resource (id 128) found")
#define NOTABC		MESS(8501, "File \"%s\" is not an ABC file")
#define NOABCFILESLEFT	MESS(8502, "No proper ABC files left to process")
#define ONLYONEFROMFINDER MESS(8503, "I can open only one ABC file from the Finder; I shall try the first one")
#define NO_ABCWSNAME	MESS(8504, "I cannot retrieve the ABC workspace name for folder \"%s\" from the group index")
#define FAILOPEN	MESS(8505, "Sorry, I failed to open file \"%s\"")
#define ERRHEAD 	MESS(8506, "Improper how-to in file \"%s\"")
#define NOPARAM 	MESS(8507, "Cannot run a how-to with parameters")
#define PRESS_RETURN	MESS(8508, "Press [RETURN] to end session")
#define NO_EXEC		MESS(8509, "Sorry, I failed to execute the how-to in \"%s\" as a command")
#define WRONGTYPE	MESS(8510, "File \"%s\" is not of type 'TEXT'")
#define NOCREATE	MESS(8511, "Sorry, I could not create file \"%s\"")

#define LOCNAME 	MESS(8512, "\"%s\" is not a proper location name")
#define LOCINIT 	MESS(8513, "Location \"%s\" hasn't been initialised")
#define HOWNAME		MESS(8514, "\"%s\" is not a proper how-to name")
#define HOWINIT		MESS(8515, "There is no HOW TO %s in this workspace")

#define TAB2TEX		MESS(8516, "Write lines from table to:")
#define HOWTOLIST	MESS(8517, "Save how-to's listing as:")
#define PACKWS		MESS(8518, "Pack workspace to:")

#define NOMEM 		MESS(8519, "Not enough memory")
#define PRMANAGER 	MESS(8520, "Printing Manager error \"%s\".")
#define COULDNOTOPEN MESS(8521, "*** Sorry, I failed to open file \"%s\".\n")
#define NOPR_INTERNAL MESS(8522, "I won't print the internal ABC file \"%s\"")
#define FAILPRTEMP	MESS(8523, "*** Sorry, I couldn't dump value on file")
#define NOHELPFILE MESS(8524, "Sorry, I could not find the helpfile \"%s\"")
#define NOLW	MESS(8525, "No Laserwriter found\rSelect one with Chooser")

#define MAXNAME 256	/* for asking abcnames in DLOG's (return Str255) */
#define MAXFNAME 64	/* seems to be max in SFReply */

#define HELPFILE "MacABC.help"

#define PRTEMPFILE "@p@r@l@o@c@a@b@c@"
extern int mac_todo;

#define RunABC			1
#define OpenCOPYBUFFER		2
#define OpenBWS			3
#define OpenWSP			4
#define RunHowto		5
#define PrintAppfiles 	6
#define DoExit			0

extern char **appfiles;
extern int nappfiles;
extern short appvrefnum;

int getappfiles();
bool is_howto();
bool is_internal();
bool is_wsgroup();
bool is_wsp();

FILE *asknewfile();
FILE *askfile();
char *asknewlocation();
char *asklocation();
bool goodtag();
bool existinglocation();
bool goodhowto();
bool existinghowto();

extern char **prfile;	/* array of filenames to print */
extern int nprfile;	/* number of same */
bool init_slct();
bool slct_location();
bool slct_howto();
bool slct_workspace();
bool slct_appfiles();
