/*
 * Shared data between graphics routines.
 */

typedef struct ivector {
	int x;
	int y;
} ivector;

extern ivector dev_origin; /* Lower left corner of device */
extern ivector dev_corner; /* Upper right corner of device */

extern bool enter_gfx();
extern exit_gfx();

extern int gfx_mode;
#define TEXT_MODE 1
#define GFX_MODE 2
#define SPLIT_MODE 3
