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
  Description of module.
 */

/*
  $Id: Version.h 2396 2014-03-13 21:13:27Z wkliao $

  $Log: Version.h,v $
  Revision 1.2  1996/09/10 20:07:25  wenger
  High-level parts of new PostScript output code are in place (conditionaled
  out for now so that the old code is used until the new code is fully
  working); changed (c) (tm) in windows so images are not copyrighted
  by DEVise; minor bug fixes; added more debug code in the course of working
  on the PostScript stuff.

  Revision 1.1  1996/07/09 16:00:25  wenger
  Added master version number and compile date to C++ code (also displayed
  in the user interface); added -usage and -version command line arguments;
  updated usage message.

 */

#ifndef _Version_h_
#define _Version_h_


class Version
{
public:
    static const char *Get();
    static const char *GetCopyright();
    static const char *GetWinLogo();
    static void PrintInfo();
};


#endif /* _Version_h_ */

/*============================================================================*/
