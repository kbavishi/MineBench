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
  $Id: Util.h 2396 2014-03-13 21:13:27Z wkliao $

  $Log: Util.h,v $
  Revision 1.13  1996/12/03 20:24:22  jussi
  Added readn() and writen().

  Revision 1.12  1996/10/07 22:53:51  wenger
  Added more error checking and better error messages in response to
  some of the problems uncovered by CS 737 students.

  Revision 1.11  1996/08/23 16:55:45  wenger
  First version that allows the use of Dali to display images (more work
  needs to be done on this); changed DevStatus to a class to make it work
  better; various minor bug fixes.

  Revision 1.10  1996/07/18 01:25:28  jussi
  Added definition of strdup for Ultrix.

  Revision 1.9  1996/07/12 18:24:30  wenger
  Fixed bugs with handling file headers in schemas; added DataSourceBuf
  to TDataAscii.

  Revision 1.8  1996/06/19 19:56:26  wenger
  Improved UtilAtof() to increase speed; updated code for testing it.

  Revision 1.7  1996/04/30 15:30:36  wenger
  Attrproj code now reads records via TData object; interface to Birch
  code now in place (but not fully functional).

  Revision 1.6  1996/04/16 19:49:39  jussi
  Replaced assert() calls with DOASSERT().

  Revision 1.5  1996/04/15 19:32:28  wenger
  Consolidated the two (nearly identical) functions for
  reading/parsing physical schema, and cleaned up the
  ParseCat.c module quite a bit.  As part of this, I added
  a new enum DevStatus (StatusOk, StatusFailed, etc.).

  Revision 1.4  1995/12/28 21:29:02  jussi
  Got rid of strings.h and stuck with string.h.

  Revision 1.3  1995/09/22 22:07:23  jussi
  Fixed problem with very high-precision numbers. Atoi of the decimal
  part overflowed, while counting the number of digits after the
  decimal point was correct, resulting in incorrect conversions.

  Revision 1.2  1995/09/05 21:13:13  jussi
  Added/updated CVS header.
*/

#ifndef Util_h
#define Util_h

using namespace std;

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <ctype.h>
#include <iostream>
#include <math.h>

#include "DeviseTypes.h"
#include "Exit.h"

/* get the name file was last modified */
extern long ModTime(char *fname);

/* Read the contents of a file into a buffer (buffer allocated by this
 * function). */
extern DevStatus ReadFile(char *filename, int &size, char *&buffer);

extern char *CopyString(char *str);

#ifdef ULTRIX
#define strdup(s) CopyString(s)
#endif

/*
   Read/write specified number of bytes. Recover from interruped system calls.
*/

extern int readn(int fd, char *buf, int nbytes);
extern int writen(int fd, char *buf, int nbytes);

/* Clear contents of directory */
extern void ClearDir(char *dir);
extern void CheckAndMakeDirectory(char *dir, int clear = 0);

/* Check space available in a directory. */
extern void CheckDirSpace(char *dirname, char *envVar,
                          int warnSize, int exitSize);

/* strip file of path name */
inline char *StripPath(char *name) {
  char *last;
  if ((last = strrchr(name,'/')) == (char *)NULL)
    return name;
  return last + 1;
}

/* Strip the trailing newline (if any) from a string. */
inline void StripTrailingNewline(char *string)
{
  int len = strlen(string);
  if (len > 0 && string[len - 1] == '\n')
    string[len - 1] = '\0';
}

/* Determine whether a string is blank (consists only of whitespace
 * characters). */
inline Boolean IsBlank(char *string)
{
    while (*string != '\0')
    {
	if (!isspace(*string)) return false;
	string++;
    }

    return true;
}

/* convert double to string */
char *DateString(time_t tm);
inline char *DateString(double d) {
  return DateString((time_t)d);
}

/* Return true if a number, set num to the converted number.
   A number is defined as: [0-9]+[.[0-9]*]  or .[0-9]+ */
inline int ConvertNum(char *str, double &num) {
  int numArgs;
  char temp;
  numArgs = sscanf(str, "%lf%c", &num, &temp);
  if (numArgs == 1)
    return 1;
  return 0;
}

/* Round num to boundary of wordWidth */
inline int WordBoundary(int num, int wordWidth) {
  if ((num % wordWidth) != 0)
    num = wordWidth * ((num / wordWidth) + 1);
  return num;
}

const double _UtilPower10[] = { 1, 10, 100, 1000, 10000, 100000,
				1e6, 1e7, 1e8, 1e9, 1e10, 1e11,
			        1e12, 1e13 };
const int _UtilMaxDecimals = sizeof _UtilPower10 / sizeof _UtilPower10[0];

inline double UtilAtof(char *str)
{
  /* Deal with leading +/- sign, if any. */
  int sign = 1;
  if (*str == '-')
  {
    sign = -1;
    str++;
  }
  else if (*str == '+')
  {
    str++;
  }

  /* Deal with the digits before the decimal point. */
  double wholePart = 0.0;
  while(isdigit(*str))
  {
    wholePart *= 10.0;
    wholePart += *str - '0';
    str++;
  }

  if (*str != '.' && *str != 'e')
  {
    double value = sign * wholePart;
    return value;
  }

  /* Deal with digits after the decimal point. */
  double fractPart = 0.0;
  double placeValue = 0.1;

  if (*str == '.')
  {
    str++;
    while(isdigit(*str))
    {
      fractPart += placeValue * (*str - '0');
      placeValue *= 0.1;
      str++;
    }
  }
  if (*str != 'e')
  {
    double value = sign * (wholePart + fractPart);
    return value;
  }

  /* Deal with the exponent... */
  str++;

  /* ...sign of exponent... */
  int expSign = 1;
  if (*str == '-')
  {
    expSign = -1;
    str++;
  }
  else if (*str == '+')
  {
    str++;
  }

  /* ...value of exponent... */
  long exponent = 0;
  while(isdigit(*str))
  {
    exponent *= 10;
    exponent += (*str - '0');
    str++;
  }

  /* ...10**exponent. */
  double scale = 1.0;
  while (exponent >= _UtilMaxDecimals) {
    scale *= _UtilPower10[_UtilMaxDecimals - 1];
    exponent -= _UtilMaxDecimals - 1;
  }
  scale *= _UtilPower10[exponent];
  if (expSign < 0) scale = 1.0 / scale;

  double value = sign * (wholePart + fractPart) * scale;
  return value;
}


#ifdef DEBUG
#define DO_DEBUG(stuff) stuff
#else
#define DO_DEBUG(stuff)
#endif


#endif
