/* The structure of the 'device switch' used by the standard I/O library.
   It is possible to install your own versions of selected routines
   by storing function pointers into this table.  The structure of
   the control block for the dev_write function is also given.

   Careful! this information was gathered by disassembling parts
   of the library.  There are no guarantees that the same code will
   work in future versions of MPW.
   Part of it has been tested with versions 1.0B1 and 1.0B2. */

typedef int (*funcptr)();	/* Pointer to integer function */

struct device {
	long	dev_name;	/* 'FSYS', 'CONS' or 'SYST' */
	funcptr	dev_faccess;
	funcptr dev_close;
	funcptr dev_read;
	funcptr dev_write;
	funcptr dev_ioctl;
};

extern struct device _StdDevs[];

#define DEV_FSYS 0
#define DEV_CONS 1
#define DEV_SYST 2

/* Control block for dev_write (arg 1 is a pointer to this).
   You might guess that dev_read is similar. */

struct controlblock {
	long io_filler1;	/* Flags? */
	long io_filler2;	/* Some pointer */
	long io_filler3;	/* Zero */
	long io_nbytes;		/* Number of bytes to write */
				/* (Reset this to zero after writing) */
	char *io_data;		/* Start of data buffer */
};

#define IO_OK 0			/* Return value from dev_write */
