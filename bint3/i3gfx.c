/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1986. */

/*
 * Graphics extension to B.
 *
 * Three commands have been added:
 *
 * SPACE'FROM a, b TO c, d
 *	Enters graphics mode; (a, b) is the lower left corner, (c, d) the
 *	upper right corner of screen.  Clears the screen in any case.
 *	A few lines at the bottom of the screen are still used for
 *	normal scrolling text.	If a=c or b=d, the corresponding
 *	scale is taken from the device precision with the origin
 *	in the middle of the screen.
 *
 * LINE'FROM a, b TO c, d
 *	Draws a line (with clipping) from (a, b) to (c, d).
 *	If not already in graphics mode, enter it (with unchanged
 *	coordinate space).
 *
 * CLEAR'SCREEN
 *	If in graphics mode, turns it off.  Clears the screen in any case.
 *
 *
 * Changes have also been made to the editor, parser and interpreter;
 * these are only compiled if '#ifdef GFX' is true.
 */

#include "b.h"
#include "bobj.h"
#include "bgfx.h"

#ifdef GFX

/* Interface for interpreter ----------------------------------------------- */

bool enter_gfx();
do_space();
do_line();


/*
 * Enter graphics mode.  Clear the screen.  Set spacing to given values.
 */

Visible Procedure space_to(v, w) value v, w; {
	do_gfx(v, w, /*&*/do_space);
}


/*
 * Draw a line between given points.
 * If not already in graphics mode, enter it first.
 * (Default spacing is the same as used last time, or (0, 0) TO (100, 100)
 * if no SPACE command was ever issued.)
 */

Visible Procedure line_to(v, w) value v, w; {
	do_gfx(v, w, /*&*/do_line);
}


/*
 * Exit graphics mode.
 * Clear the screen.
 */

Visible Procedure clear_screen() {
	exit_gfx();
}


/* Device-independent code ------------------------------------------------- */

/*
 * Graphics mode.
 */

Visible int gfx_mode= TEXT_MODE;


/*
 * Representation of a vector.
 */

typedef struct vector {
	double x;
	double y;
} vector;


/*
 * Variables describing the user coordinate space.
 * (Can be changed by calls to space_to).
 */

static vector origin= {0.0, 0.0};
static vector corner= {0.0, 0.0};


/*
 * Scale factor for coordinate transformation.
 * (Computed from above variables plus device information by space_to.)
 */

static vector scale;


/*
 * Macros to do the transformation from user to device coordinates.
 */

#define XSCALE(a) (((a) - origin.x) * scale.x)
#define YSCALE(a) (((a) - origin.y) * scale.y)


/*
 * Check to see if a B value is a valid vector (= pair of numbers).
 * If so, extract the value into the vector whose address is passed.
 */

Hidden bool get_point(v, pv) value v; vector *pv; {
	value x, y;

	if (!Is_compound(v) || Nfields(v) != 2)
		return No;
	x= *Field(v, 0);
	y= *Field(v, 1);
	if (!Is_number(x) || !Is_number(y))
		return No;
	pv->x= numval(x);
	pv->y= numval(y);
	return Yes;
}


/*
 * Generic code for graphics routines that have two vector parameters.
 * Check that the arguments are indeed vectors and call the processing code.
 */

Hidden Procedure do_gfx(v, w, proc) value v; value w; int (*proc)(); {
	vector v1, v2;

	if (!get_point(v, &v1) || !get_point(w, &v2)) {
		interr(MESS(8000, "argument to graphics command not a vector"));
		return;
	}
	(*proc)(&v1, &v2);
}


/*
 * Routine to enter graphics mode and set the spacing as desired.
 */

Hidden Procedure do_space(pv1, pv2) vector *pv1, *pv2; {
	double tmp;

	if (gfx_mode != GFX_MODE) {
		if (!enter_gfx()) {
			interr(MESS(8001, "no graphics hardware available"));
			return;
		}
	}
	clipinit(dev_origin.x, dev_origin.y, dev_corner.x, dev_corner.y);
	origin.x= pv1->x;
	origin.y= pv1->y;
	corner.x= pv2->x;
	corner.y= pv2->y;
	if (origin.x > corner.x) {
		tmp= origin.x;
		origin.x= corner.x;
		corner.x= tmp;
	}
	else if (origin.x == corner.x) {
		origin.x= dev_origin.x - (dev_corner.x - dev_origin.x) / 2;
		corner.x= origin.x + (dev_corner.x - dev_origin.x);
	}
	if (origin.y > corner.y) {
		tmp= origin.y;
		origin.y= corner.y;
		corner.y= tmp;
	}
	else if (origin.y == corner.y) {
		origin.y= dev_origin.y - (dev_corner.y - dev_origin.y) / 2;
		corner.y= origin.y + (dev_corner.y - dev_origin.y);
	}
	scale.x= (double) (dev_corner.x - dev_origin.x) /
			(corner.x - origin.x);
	scale.y= (double) (dev_corner.y - dev_origin.y) /
			(corner.y - origin.y);
}


/*
 * Routine to draw a line.
 */

Hidden Procedure do_line(pv1, pv2) vector *pv1, *pv2; {
	int x1, y1, x2, y2;

	if (gfx_mode != GFX_MODE) {
		do_space(&origin, &corner);
		if (gfx_mode != GFX_MODE)
			return;
	}
	x1= XSCALE(pv1->x);
	x2= XSCALE(pv2->x);
	y1= YSCALE(pv1->y);
	y2= YSCALE(pv2->y);
	if (inview2d(x1, y1, x2, y2) || clip2d(&x1, &y1, &x2, &y2))
		draw_line(x1, y1, x2, y2);
}

/* Clipping ---------------------------------------------------------------- */

/* @(#)clip.c	1.2 - 85/10/07 */
/*
 * Fast, 2d, integer clipping plot(3) operations.
 * Clipping algorithm taken from "A New Concept and Method for Line Clipping,"
 * Barsky & Liang, ACM Tran. on Graphics Vol 3, #1, Jan 84.
 * In contrast to the algoritm presented in TOG, this one works
 * on integers only.  The idea is to only do that which is useful
 * for my plot(3) based graphics programs.
 */

/* AUTHOR:
Rob Adams <ima!rob>
Interactive Systems, 7th floor, 441 Stuart st, Boston, MA 02116; 617-247-1155
*/

/*
 * Interface:
 *
 *  clipinit(int xleft, int ybottom, int xright, int ytop)
 *   Send this guy the same things you would send to space().
 *   Don't worry if xleft > xright.
 *
 *  clip2d(int *x0p, int *y0p, int *x1p, int *y1p)
 *   By the time this returns, the points referenced will have
 *   been clipped.  Call this right before line(), with pointers
 *   to the same arguments.  Returns TRUE is the resulting line
 *   can be displayed.
 *
 *  inview2d(int x0,int y0,int x1,int y1)
 *   Does a fast check for simple acceptance.  Returns TRUE if
 *   the segment is intirely in view.  If your program runs too
 *   slowly, consider making this a macro.
 *
 *  Usage of clip2d and inview2d would be something like --
 *	(inview2d(x0,y0, x1,y1) || clip2d(&x0,&y0, &x1,&y1))
 *		&& line(x0,y0,x1,y1);
 *  If inview2d says the segment is safe or clip2d says the clipped
 *  segment is safe, then go ahead and print the line.
 */
static int Xleft, Xright, Ytop, Ybot;

#define TRUE	1
#define FALSE	0
#define bool	int

/*------------------------------- clipinit ----------------------------------*/
clipinit(x0,y0,x1,y1) {
	if ( x0 > x1 ) {
	    Xleft  = x1;
	    Xright = x0;
	} else {
	    Xleft  = x0;
	    Xright = x1;
	}
	if ( y0 > y1 ) {
	    Ytop = y0;
	    Ybot = y1;
	} else {
	    Ytop = y1;
	    Ybot = y0;
	}
}

/*------------------------------- inview2d ----------------------------------*/
bool inview2d(x0,y0, x1,y1) register x0,y0, x1,y1; {
	return	x0 >= Xleft && x0 <= Xright && x1 >= Xleft && x1 <= Xright &&
		y0 >= Ybot  && y0 <= Ytop   && y1 >= Ybot  && y1 <= Ytop;
}

/*-------------------------------- clip2d -----------------------------------*/
bool clip2d(x0p, y0p, x1p, y1p) int *x0p, *y0p, *x1p, *y1p; {
	register int	x0 = *x0p,
			y0 = *y0p,
			x1 = *x1p,
			y1 = *y1p;

	register int	dx, dy;
		 double t0, t1;

	t0 = 0.0, t1 = 1.0;			 /* init parametic equations */
	dx = x1 - x0;
	if ( clipt( -dx, x0 - Xleft, &t0, &t1) &&
	     clipt( dx, Xright - x0, &t0, &t1)) {
	    dy = y1 - y0;
	    if ( clipt( -dy, y0 - Ybot, &t0, &t1) &&
		 clipt( dy, Ytop - y0, &t0, &t1)) {
		if ( t1 < 1 ) {
		    *x1p = x0 + t1 * dx;
		    *y1p = y0 + t1 * dy;
		}
		if ( t0 > 0.0 ) {
		    *x0p = x0 + t0 * dx;
		    *y0p = y0 + t0 * dy;
		}
		return TRUE;
	    }
	}
	return FALSE;
}

/*-------------------------------- clipt ------------------------------------*/
static bool clipt(p, q, t0p, t1p) register int p, q;
		register double *t0p, *t1p; {
	register double r;

	if ( p < 0 ) {
	    r = (double)q / p;
	    if ( r > *t1p )
		return FALSE;
	    if ( r > *t0p )
		*t0p = r;
	} else if (p > 0) {
	    r = (double)q / p;
	    if ( r < *t0p )
		return FALSE;
	    if ( r < *t1p )
		*t1p = r;
	} else if (q < 0)
		return FALSE;
	return TRUE;
}

#endif /* GFX */
