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
  $Id: Parse.h 2396 2014-03-13 21:13:27Z wkliao $

  $Log: Parse.h,v $
  Revision 1.3  1996/01/11 21:02:16  jussi
  Added ParseSQLTimestamp() function. Added copyright notice.

  Revision 1.2  1995/09/05 21:12:58  jussi
  Added/updated CVS header.
*/

#ifndef Parse_h
#define Parse_h

#include <time.h>

/*
   Parse function with ' ', and \t as separators
*/
extern void Parse(char *, int &, char **&);

/*
   Parse function that takes an array of characters as separators.
   isSeparator == false to parse whitespace.
               == true to parse separators
*/
extern void Parse(char *line, int &num, char **&args, char *separators,
		  int numSeparators, int isSeparator = 0);

/*
   Parse either an integer, or date format, and make a double.
   Return true if parsed OK.
   date format is like: Thu Jan 26 17:27:42 CST 1995
*/
int ParseFloatDate(char *line, double &val);

/*
   Parse a SQL time stamp of the format YYYY-MM-DD HH:MM:SS and return
   corresponding Unix time, or return -1 if argument is incorrect.
*/
time_t ParseSQLTimestamp(char *str);

#endif
