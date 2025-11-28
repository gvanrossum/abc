/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1990. */

#ifndef HAS_READDIR

/* slightly edited for ABC from public domain source */

#ifndef DEV_BSIZE
#define	DEV_BSIZE	512
#endif
#define DIRBLKSIZ	DEV_BSIZE
#define	MAXNAMLEN	255

struct	direct {
	long	d_ino;			/* inode number of entry */
	short	d_reclen;		/* length of this record */
	short	d_namlen;		/* length of string in d_name */
	char	d_name[MAXNAMLEN + 1];	/* name must be no longer than this */
};

/*
 * The DIRSIZ macro gives the minimum record length which will hold
 * the directory entry.  This requires the amount of space in struct direct
 * without the d_name field, plus enough space for the name with a terminating
 * null byte (dp->d_namlen+1), rounded up to a 4 byte boundary.
 */
#undef DIRSIZ
#define DIRSIZ(dp) \
    ((sizeof (struct direct) - (MAXNAMLEN+1)) + (((dp)->d_namlen+1 + 3) &~ 3))

/*
 * Definitions for library routines operating on directories.
 */
typedef struct _dirdesc {
	int	dd_fd;
	long	dd_loc;
	long	dd_size;
	char	dd_buf[DIRBLKSIZ];
} DIR;

#ifndef NULL
#define NULL 0
#endif

Visible DIR *opendir();
Visible struct direct *readdir();
Visible Procedure closedir();

#endif /* !HAS_READDIR */
