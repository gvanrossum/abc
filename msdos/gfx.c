/*
 * Device-dependent code for graphics (version for IBM-PC Color Monitor).
 * See b3gfx.c for device-independent code and explanation.
 *
 * Routines are:
 *
 *	int enter_gfx();
 *		Tries to enter graphics mode and clears the screen.
 *		Returns Yes if successful, No if no color card.
 *		Also fills the global variables dev_origin and dev_corner
 *		with the dimensions of the graphics device.
 *
 *	exit_gfx();
 *		Leaves graphics mode.
 *
 *	draw_line(int x1, y1, x2, y2);
 *		Draws a line from (x1, y1) to (x2, y2).
 *		Should only be called with coordinates in range and when
 *		graphics mode has successfully been entered.
 */


/*
 * Instead of including b.h:
 */

typedef char bool; /* Only used for function results */
#define Yes 1
#define No 0

#define Hidden static
#define Visible
#define Procedure


/*
 * Include files.
 */

#include "bgfx.h"
#include <dos.h>

#ifdef GFX

/*
 * Variables holding the device dimensions.
 */

ivector dev_origin;
ivector dev_corner;


/*
 * Remember old video mode.
 */

Hidden int old_mode= -1;


/*
 * BIOS video I/O is called by generating an 8086 software interrupt,
 * using lattice's int86() function.
 * To ease coding, all routines fill in the apropriate parameters in regs,
 * and than call bios10(code), where code is to be placed in ah.
 * (This routine is identical to the one in ptrm.c but I didn't want
 * to change it from Hidden to Visible there.)
 */

Hidden union REGS regs;

Hidden Procedure bios10(code) int code; {
	regs.h.ah= code;
	int86(0x10, &regs, &regs);
}


/*
 * Enter graphics mode.
 * This may fail if there is no graphics hardware; then emit an error
 * message and return No.
 * If successful: clear the screen, set device parameters,
 * call clipinit, set 'gfx_mode' to GFX_MODE and return Yes.
 */

Visible bool enter_gfx() {
	int i;

	if (gfx_mode != TEXT_MODE)
		exit_gfx();

	bios10(15); /* Read video mode */
	old_mode= regs.h.al;

	switch (old_mode) {

	case 0: /* 40x25 BW */
	case 1: /* 40x25 Color */
	case 4: /* 320x200 Color */
	case 5: /* 320x200 BW */
		dev_corner.x= 320 - 1;
		regs.h.al= 4;
		break;

	case 2: /* 80x25 BW */
	case 3: /* 80x25 Color */
	case 6: /* 640x200 BW */
		dev_corner.x= 640 - 1;
		regs.h.al= 6;
		break;

	default:
		/* Unrecognized mode; probably 7, Monochrome Monitor
		   (which doesn't have graphics capabilities). */
		old_mode= -1;
		return No;

	}

	bios10(0); /* Set video mode */

	dev_corner.y= 200 - 1;
	dev_origin.x= 0;
	dev_origin.y= 0;

	gfx_mode= GFX_MODE;
	return Yes;
}


/*
 * Exit from graphics mode.
 */

Visible Procedure exit_gfx() {
	if (old_mode >= 0) {
		regs.h.al= old_mode;
		bios10(0); /* Set video mode */
		old_mode= -1;
	}
	gfx_mode= TEXT_MODE;
	biosscrollup(0, 24, 0); /* Scroll '0' lines --> clear area */
}


/* ------------------------------------------------------------------------- */

/*
 * Graphics routines for IBM PC with color/graphics card.
 * Taken and munged without permission from the CI-86 library sources.
 */

/*
 * Draw a line in graphics mode on an IBM color monitor.
 */

#define COLOR 1

Visible Procedure draw_line(x1, y1, x2, y2) int x1, y1, x2, y2; {
  int dx;
  int dy;
  int xinc;
  int yinc;
  int tv;
  int sflag;
  int a;
  int b;

  y1=199-y1; y2=199-y2; /* Reverse y coordinates -- GvR */
  regs.x.ax=0x0C00|(COLOR&0xf);    /* set to write dot and set color */
  dx=x2-x1;	/* delta x and direction increment */
  xinc=1;
  if(dx<0){
    xinc= -1;	/* negative increment */
    dx=x1-x2;	/* make delta x positive */
  }
  dy=y2-y1;	/* delta y and direction increment */
  yinc=1;
  if(dy<0){
    yinc= -1;	/* negative increment */
    dy=y1-y2;	/* make delta y positive */
  }
  sflag=0;
  if(dy>dx){
    sflag=1;
    { int tmp= dy; dy= dx; dx= tmp; } /* dx must be greater of the two */
  }
  regs.x.cx=x1;    /* coordinates of first point */
  regs.x.dx=y1;
  tv=(dy<<1)-dx;	/* test variable for the move */
  a=dy<<1;		/* factor to add after each horizontal move */
  b=(dy-dx)<<1; 	/* factor to add after each vertical move */
  for(dx++;dx;dx--){
    bios10(12); /* Write dot */
    if(tv<0)tv+=a;
    else{
      tv+=b;
      if(sflag)regs.x.cx+=xinc;
      else regs.x.dx+=yinc;
    }
    if(sflag)regs.x.dx+=yinc;
    else regs.x.cx+=xinc;
  }
}

/*
 * The following is nearly identical to a routine in ptrm.c except here
 * it computes 'cols' and there it also updates the internal administration.
 *
 * Note the BIOS quirk (which we actually use!) that a count of zero
 * means clear the window!
 */

#define V_NORMAL 7

/* scrolling for BIOS */

Hidden Procedure biosscrollup(yfirst, ylast, by) int yfirst, ylast, by; {
	int cols;

	bios10(15);
	cols= regs.h.ah;

	regs.h.al = (by < 0 ? -by : by);
	regs.h.ch = yfirst;
	regs.h.cl = 0;
	regs.h.dh = ylast;
	regs.h.dl = cols-1;
	regs.h.bh = V_NORMAL;
	bios10(by < 0 ? 7 : 6);
}

#endif /* GFX */

