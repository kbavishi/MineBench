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
  $Id: Exit.h 2396 2014-03-13 21:13:27Z wkliao $

  $Log: Exit.h,v $
  Revision 1.4  1996/10/08 21:49:01  wenger
  ClassDir now checks for duplicate instance names; fixed bug 047
  (problem with FileIndex class); fixed various other bugs.

  Revision 1.3  1996/04/16 19:45:35  jussi
  Added DoAbort() method and DOASSERT macro.

  Revision 1.2  1995/09/05 21:12:45  jussi
  Added/updated CVS header.
*/

#ifndef Exit_h
#define Exit_h

#define DOASSERT(c,r)    { if (!(c)) Exit::DoAbort(r, __FILE__, __LINE__); }

class Exit {
public:
  static void DoExit(int code = 0);
  static void DoAbort(char *reason, char *file, int line);
};

#endif
