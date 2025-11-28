/*
 * "Dir.h" for the Macintosh.
 */

#define MAXNAMLEN 31
#define MAXPATH 256

#define DIR  struct _dir

struct _dir {
	long dirid;
	int nextfile;
};

struct direct {
	char d_name[MAXPATH];
};

DIR *opendir();
struct direct *readdir();
closedir();