/* A procedure to install a hook in the MPW C 'open' library function.
   This is useful because you may want to create files automatically
   with type 'TEXT' without having to change all the 'open' or 'fopen'
   calls in a large C program you are trying to port.  A standard hook
   procedure for this purpose is also provided.   

   Call:
   	set_open_hook(proc);
   This installs the hook proc, or restores the default situation if
   proc is NULL.
   The hook procedure will be called immediately *after* a successful
   open call, with the following parameters:
   	proc(filename, oflag, fd)
		char *filename;		The file name
		int oflag;		Mode passed to open
		int fd;			Return value from open

   Note: this only works when the program is linked as an application
   (type APPL); for tools (type MPST) the device switch is located
   in the Shell's memory.
   
   Careful! this information was gathered by disassembling parts
   of the library.
   There are no guarantees that the same code will work in future
   versions of MPW.  It has been tested with version 1.0B2. */

#include <fcntl.h>

#include <Types.h>
#include <Files.h>

#include "intercept.h"

#define ERRFLAG 0x40000000

static ProcPtr open_hook;

/* The hook for faccess, installed in the device switch.
   This will be called with cmd == F_OPEN from 'open',
   but also from 'faccess', with other values for cmd.
   The open_hook is only called if cmd == F_OPEN.
   It is not necessary to check whether open_hook is NULL,
   since we are only installed after open_hook is set non-NULL. */

static long
my_faccess(file, cmd, arg)
	char *file;
	int cmd;
	short *arg;
{
	long res= _fsFAccess(file, cmd, arg);
	
	if (cmd == F_OPEN && !(res&ERRFLAG)) {
		(void) (*open_hook)(file, *arg, (int)res);
	}
	return res;
}

/* Standard open hook, to set type and creator of created files.
   It will not change existing non-zero type or creator fields.
   It returns an error code even though this is ignored by the
   calling routine; you might want to call it yourself in a more
   fancyful hook, and test the error code.
   This routine can be customized by changing 'std_type' or
   'std_creator'. */

OSType std_type=	'TEXT';
OSType std_creator=	'MPS ';

int
std_open_hook(file, mode, fd)
	char *file;
	int mode;
	int fd;
{
	FInfo info;
	int err= noErr;
	
	switch (mode & 03) {
	
	case O_RDWR:
	case O_WRONLY:
		err= GetFInfo(file, 0, &info);
		if (err != noErr)
			return err;
		if (info.fdType == 0) {
			info.fdType= std_type;
			++err; /* Use 'err' as a flag to call SetFInfo */
		}
		if (info.fdCreator == 0) {
			info.fdCreator= std_creator;
			++err;
		}
		if (err != noErr)
			err= SetFInfo(file, 0, &info);
	
	}
	return err;
}

/* The procedure to install the hook.
   Note: this assumes nobody else is also installing hooks
   for faccess, otherwise we would have to save and restore
   the old function, instead of blindly assuming _fsFAccess. */

set_open_hook(hook)
	ProcPtr hook;
{
	if (hook == NULL)
		_StdDevs[DEV_FSYS].dev_faccess= _fsFAccess;
	else {
		open_hook= hook;
		_StdDevs[DEV_FSYS].dev_faccess= my_faccess;
	}
}

/* A trivial test program will be included if you #define MAIN: */

#ifdef MAIN

#include <stdio.h>

extern int std_open_hook();

main()
{
	set_open_hook(std_open_hook);
	fclose(fopen("ABC", "a"));
}

#endif
