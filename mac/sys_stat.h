struct stat {
	unsigned short st_mode;
	unsigned long st_size;
	unsigned long st_mtime;
};

#define S_IFMT	0170000l
#define S_IFDIR	0040000l
#define S_IFREG 0100000l
#define S_IREAD    0400
#define S_IWRITE   0200
#define S_IEXEC    0100
