/*
  $Id: DeviseTypes.h 2396 2014-03-13 21:13:27Z wkliao $

  $Log: DeviseTypes.h,v $
  Revision 1.12  1996/12/18 18:52:01  wenger
  Devise requests Tasvir not to use subwindows for its images; sizing of
  Tasvir images now basically works like a RectX, except that the aspect
  ratio is fixed by the image itself; fixed a bug in action of +/- keys
  for RectX shapes; Devise won't request Tasvir images smaller than two
  pixels (causes error in Tasvir).

  Revision 1.11  1996/08/23 16:55:32  wenger
  First version that allows the use of Dali to display images (more work
  needs to be done on this); changed DevStatus to a class to make it work
  better; various minor bug fixes.

  Revision 1.10  1996/08/05 19:51:36  wenger
  Fixed compile errors caused by some of Kevin's recent changes; changed
  the attrproj stuff to make a .a file instead of a .o; added some more
  TData file writing stuff; misc. cleanup.

  Revision 1.9  1996/08/05 17:28:58  beyer
  Added is_safe() which checks to see if a double is safe value (ie, not
  NaN or Infinity).  Made SetVisualFilter check the new filter for safety.

  Revision 1.8  1996/08/04 23:33:16  beyer
  Added #define for NULL

  Revision 1.7  1996/07/31 19:34:30  wenger
  Added AttrProj member functions for reading entire records (no projection).

  Revision 1.6  1996/07/13 04:59:31  jussi
  Added conditional for Linux.

  Revision 1.5  1996/06/21 19:29:11  jussi
  Cleaned up file and replaced MinMax class with simpler MIN/MAX macros.

  Revision 1.4  1996/05/20 18:44:40  jussi
  Replaced PENTIUM flag with SOLARIS.

  Revision 1.3  1996/04/15 19:32:05  wenger
  Consolidated the two (nearly identical) functions for
  reading/parsing physical schema, and cleaned up the
  ParseCat.c module quite a bit.  As part of this, I added
  a new enum DevStatus (StatusOk, StatusFailed, etc.).

  Revision 1.2  1995/09/05 21:12:39  jussi
  Added/update CVS header.
*/

#ifndef DeviseTypes_h
#define DeviseTypes_h

#include <values.h>


typedef double Coord;
typedef char Boolean;

#if defined(HPUX) || defined(SUN) || defined(SOLARIS)
inline int trunc(float num) {
  return (int)num;
}
inline int trunc(double num) {
  return (int)num;
}
#endif

#ifndef MIN
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif

#ifndef MIN3
#define MIN3(a,b,c) ((a) < (b) ? MIN(a,c) : MIN(b,c))
#endif

#ifndef MAX
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#endif

#ifndef MAX3
#define MAX3(a,b,c) ((a) > (b) ? MAX(a,c) : MAX(b,c))
#endif

#ifndef ABS
#define ABS(x) ((x) >= 0 ? (x) : -(x))
#endif



#include "DevStatus.h"


#ifndef NULL
#define NULL 0
#endif


inline bool is_safe(double x)
{
  return x < MAXDOUBLE && x > -MAXDOUBLE;
}


#endif
