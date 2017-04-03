/*
  ========================================================================
  DEVise Data Visualization Software
  (c) Copyright 1992-1996
  By the DEVise Development Group
  Madison, Wisconsin
  All Rights Reserved.
  ========================================================================

  Under no circumstances is this software to be copied, distributed,
  or altered in any way without prior permission from the DEVise
  Development Group.
*/

/*
  $Id: VisualArg.h 2396 2014-03-13 21:13:27Z wkliao $

  $Log: VisualArg.h,v $
  Revision 1.5  1996/11/13 16:56:16  wenger
  Color working in direct PostScript output (which is now enabled);
  improved ColorMgr so that it doesn't allocate duplicates of colors
  it already has, also keeps RGB values of the colors it has allocated;
  changed Color to GlobalColor, LocalColor to make the distinction
  explicit between local and global colors (_not_ interchangeable);
  fixed global vs. local color conflict in View class; changed 'dali'
  references in command-line arguments to 'tasvir' (internally, the
  code still mostly refers to Dali).

  Revision 1.4  1996/06/15 07:08:14  yuc
  Add Camera structure to the system.

  Revision 1.3  1996/05/31 15:31:24  jussi
  Added VISUAL_RECORD visual argument.

  Revision 1.2  1995/09/05 21:13:23  jussi
  Added/updated CVS header.
*/

#ifndef VisualArg_h
#define VisualArg_h

#include "DeviseTypes.h"
#include "Color.h"
#include "Pattern.h"

/* Index of the attributes */

const unsigned VISUAL_X_INDEX = 0;
const unsigned VISUAL_Y_INDEX = 1;
const unsigned VISUAL_SIZE_INDEX = 2;
const unsigned VISUAL_PATTERN_INDEX = 3;
const unsigned VISUAL_COLOR_INDEX = 4;
const unsigned VISUAL_ORIENTATION_INDEX = 5;
const unsigned VISUAL_SHAPE_INDEX = 6;
const unsigned VISUAL_RECORD_INDEX = 7;

/*
   A VisualFlag is the union of visual attributes.
   It indicates which attributes are changeable or tested in a filter.
*/

typedef unsigned VisualFlag;

const unsigned VISUAL_X           = (1 << VISUAL_X_INDEX);
const unsigned VISUAL_Y           = (1 << VISUAL_Y_INDEX);
const unsigned VISUAL_LOC         = ((1 << VISUAL_X_INDEX)
				     | (1 << VISUAL_Y_INDEX));
const unsigned VISUAL_COLOR       = (1 << VISUAL_COLOR_INDEX);
const unsigned VISUAL_SIZE        = (1 << VISUAL_SIZE_INDEX);
const unsigned VISUAL_PATTERN     = (1 << VISUAL_PATTERN_INDEX);
const unsigned VISUAL_ORIENTATION = (1 << VISUAL_ORIENTATION_INDEX);
const unsigned VISUAL_SHAPE       = (1 << VISUAL_SHAPE_INDEX);
const unsigned VISUAL_RECORD      = (1 << VISUAL_RECORD_INDEX);
const unsigned VISUAL_ALLBITS     = (VISUAL_X | VISUAL_Y | VISUAL_LOC |
				     VISUAL_COLOR | VISUAL_SIZE |
				     VISUAL_PATTERN | VISUAL_ORIENTATION
				     | VISUAL_SHAPE);

/* Complement visual flag */

inline unsigned VisualComplement(VisualFlag flag)
{
  return (flag ^ VISUAL_ALLBITS);
}

/* A visual filter: used to filter symbols inside a view. */

struct VisualFilter {
  VisualFlag flag;	           /* which attribute is to test.
				      set to 0 if no filter  */
  Coord xLow, xHigh;               /* X filter */
  Coord yLow, yHigh;               /* y filter */
  int lastN;		           /* # of records to examine */
  Coord sizeLow, sizeHigh;         /* size filter */
  Pattern patternLow, patternHigh; /* pattern filter */
  GlobalColor colorLow, colorHigh;   /* color filter */
  Coord orientationLow, orientationHigh; /* orientation filter*/
  int shapeLow, shapeHigh;         /* shape filter */
  
  Boolean marked;                  /* TRUE if this is marked in the
				      control panel list box */
};

/* A CameraFlag indicates whether the attributes are changeable or tested */
typedef unsigned CameraFlag;

/* A camera; used to store view point, perspective, etc */
struct Camera {
	CameraFlag flag;    /* true = recompute, false = exit */
	Coord _rho, _phi, _theta;
	Coord _twist_angle;
	int _perspective;
	int _dvs;
	int fix_focus;		/* TRUE = focus is fixed */
					/* FALSE = focus moves with the camera */
	int spherical_coord;/* TRUE = use spherical coordinate */
					/* FALSE = use rectangular coordinate */
	Coord x_, y_, z_;	/* camera location */
	Coord fx, fy, fz;	/* view direction, a point of intereset*/
	Coord H, V;		/* H and V are the translation wrt to
					   the original screen coordiate sys, 
					   original screen coordinate sys is at the
					   upper lefthand corner of the screen */
};

#endif

