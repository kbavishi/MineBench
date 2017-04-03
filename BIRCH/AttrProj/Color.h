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
  $Id: Color.h 2396 2014-03-13 21:13:27Z wkliao $

  $Log: Color.h,v $
  Revision 1.9  1996/11/13 16:56:02  wenger
  Color working in direct PostScript output (which is now enabled);
  improved ColorMgr so that it doesn't allocate duplicates of colors
  it already has, also keeps RGB values of the colors it has allocated;
  changed Color to GlobalColor, LocalColor to make the distinction
  explicit between local and global colors (_not_ interchangeable);
  fixed global vs. local color conflict in View class; changed 'dali'
  references in command-line arguments to 'tasvir' (internally, the
  code still mostly refers to Dali).

  Revision 1.8  1996/06/27 22:54:50  jussi
  Added XorColor value.

  Revision 1.7  1996/06/20 16:49:03  jussi
  Replaced green1 with DarkSeaGreen.

  Revision 1.6  1996/02/02 21:54:25  jussi
  Removed reference to HightlightColor. All highlighting is
  done with xor'ing so foreground color won't matter.

  Revision 1.5  1996/01/30 21:15:27  jussi
  Changed HighlightColor.

  Revision 1.4  1996/01/29 23:57:28  jussi
  Added a few colors and removed duplicate ones.

  Revision 1.3  1995/12/05 21:57:55  jussi
  Added some 20 missing colors which were defined in ColorMgr.c
  but not listed here. Now Devise can refer to all colors by the
  color constants. Also added the copyright notice.

  Revision 1.2  1995/09/05 21:12:27  jussi
  Added/updated CVS header.
*/

#ifndef Color_h
#define Color_h

/* Note: a GlobalColor is the "color number" referred to in the user
 * interface.  A GlobalColor _cannot_ be used directly by X, PostScript,
 * etc.; it must first be converted into a LocalColor, RGB values, or
 * whatever. */

/* Note: the following values are not _always_ the only legal values for
 * GlobalColor.  However, values outside this range may not produce the
 * expected results.  Mainly, I defined GlobalColor as an enum so that
 * the compiler will complain about (illegitimate) assigments of a LocalColor
 * to a GlobalColor.  It is sometimes okay to cast an int to a GlobalColor;
 * however, it is never okay to cast a LocalColor to a global color.  Even
 * if this doesn't cause a crash, it will give incorrect results.
 * RKW 11/11/96. */

enum GlobalColor {
  BlackColor		= 0,
  WhiteColor		= 1,
  RedColor		= 2,
  BlueColor		= 3,
  OrangeColor		= 4,
  TanColor		= 5,
  GreenColor		= 6,
  PurpleColor		= 7,
  AquamarineColor	= 8,
  PeruColor		= 9,
  ChocolateColor	= 10,
  TomatoColor		= 11,
  PinkColor		= 12,
  PlumColor		= 13,
  AzureColor		= 14,
  CyanColor		= 15,
  SeaGreenColor		= 16,
  KhakiColor		= 17,
  GoldenRodColor	= 18,
  YellowColor		= 19,
  SiennaColor		= 20,
  LightCoralColor	= 21,
  AntiqueWhiteColor	= 22,
  LemonChiffonColor	= 23,
  LightGrayColor	= 24,
  LavenderColor		= 25,
  LavenderBlushColor	= 26,
  MistyRoseColor	= 27,
  NavyBlueColor		= 28,
  SlateBlueColor	= 29,
  MediumBlueColor	= 30,
  DeepSkyBlueColor	= 31,
  SkyBlueColor		= 32,
  RedColor1		= 33,
  RedColor2		= 34,
  DarkSeaGreen		= 35,
  GreenColor2		= 36,
  BlueColor1		= 37,
  BlueColor2		= 38,
  BlueColor3		= 39,
  BlueColor4		= 40,
  GoldColor1		= 41,
  GoldColor2		= 42,
  XorColor		= 1000,
};

const GlobalColor ForegroundColor	= BlackColor;
const GlobalColor BackgroundColor	= AntiqueWhiteColor;



/* Note: LocalColor values are actually only meaningful at the X level,
 * so this definition should probably be moved to someplace like
 * XDisplay.h.  However, there are a number of places right now outside
 * of the X code that use local colors.  Until those parts of the code
 * are fixed, this definition probably has to stay here.  RKW 11/11/96. */

typedef unsigned long LocalColor;


#endif
