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
  $Id: Parse.c 2396 2014-03-13 21:13:27Z wkliao $

  $Log: Parse.c,v $
  Revision 1.13  1996/12/16 11:14:34  kmurli
  No change

  Revision 1.12  1996/11/23 21:23:29  jussi
  Removed Config.h. Includes Init.h instead.

  Revision 1.11  1996/04/23 15:35:34  jussi
  The parser now handles double-quote-delimited strings as well
  as single-quote-delimited strings.

  Revision 1.10  1996/01/25 17:44:29  jussi
  Added debugging output.

  Revision 1.9  1996/01/12 16:21:05  jussi
  Replaced signed int with unsigned int.

  Revision 1.8  1996/01/11 21:43:40  jussi
  Replaced libc.h with stdlib.h.

  Revision 1.7  1996/01/11 21:02:06  jussi
  Added ParseSQLTimestamp() function.

  Revision 1.6  1996/01/11 17:17:31  jussi
  Removed bug in call to strcmp(). Month hint was returned too
  eagerly.

  Revision 1.5  1996/01/10 18:45:32  jussi
  Removed bugs and improved code.

  Revision 1.4  1995/12/22 20:00:34  jussi
  Fixed time zone item in tm structure.

  Revision 1.3  1995/12/14 17:06:04  jussi
  Added copyright notice and made small fixes.

  Revision 1.2  1995/09/05 21:12:57  jussi
  Added/updated CVS header.
*/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <sys/types.h>
#include <sys/time.h>
#include <string.h>

#include "Init.h"
#include "Exit.h"

//#define DEBUG

static const char *monthNames[] = { "JAN", "FEB", "MAR", "APR",
				    "MAY", "JUN", "JUL", "AUG",
				    "SEP", "OCT", "NOV", "DEC" };

static const int MAXARGS = DEVISE_MAX_TDATA_ATTRS;
static char *args[MAXARGS];

static inline int IsBlank(char c, char *blanks, int numBlanks)
{
  for(int i = 0; i < numBlanks; i++) {
    if (c == blanks[i])
      return 1;
  }
  return 0;
}

static inline char *SkipBlanks(char *str, char *blanks, int numBlanks)
{
  while (IsBlank(*str, blanks, numBlanks))
    str++;
  return str;
}

/* Parse an a blank separated line into fields and return the 
   # of fields parsed. */

void Parse(char *str, int &numArgs, char **&returnArgs, char *blanks,
	   int numBlanks, int isSeparator)
{
  returnArgs = args;
  numArgs = 0;

  if (!isSeparator) {
    str = SkipBlanks(str, blanks, numBlanks);

    while(*str) {
      char *start = str;
      if (*str == '\'') {
	/* reading a literal */
	str++;
	start = str;
	while(*str && *str != '\'')
	  str++;
      } else if (*str == '"') {
	/* reading a literal */
	str++;
	start = str;
	while(*str && *str != '"')
	  str++;
      } else {
	while(*str && !IsBlank(*str, blanks, numBlanks)) 
	  str++;
      }

      if (*str) {
	*str = 0;
	str++;
      }
      
#ifdef DEBUG
      printf("Parsed whitespace separated field \"%s\"\n", start);
#endif

      if (numArgs >= MAXARGS - 1) {
	fprintf(stderr, "parse: too many arguments\n");
	Exit::DoExit(1);
      }
      args[numArgs++] = start;
      str = SkipBlanks(str, blanks, numBlanks);
    }

    return;
  }

  /* process line with separators */
  char *start;
  do {
    start = str;
    while(*str && !IsBlank(*str, blanks, numBlanks))
      str++;

    /* from start to str is the field */
    if (*str) {
      *str = 0;
      str++;
    }
    
#ifdef DEBUG
  printf("Parsed separator separated field \"%s\"\n", start);
#endif

    if (numArgs >= MAXARGS - 1) {
      fprintf(stderr, "parse: too many arguments\n");
      Exit::DoExit(1);
    }
    args[numArgs++] = start;
  } while(*str);
}

void Parse(char *str, int &numArgs, char **&returnArgs)
{
  char blanks[] = {' ', '\t'};
  Parse(str, numArgs, returnArgs, blanks, sizeof blanks, 0);
}

static int GetMonth(char *month)
{
  static int monthHint = -1;
  if (monthHint >= 0 && !strcmp(monthNames[monthHint], month))
    return monthHint;

  for(unsigned int i = 0; i < sizeof monthNames / sizeof monthNames[0]; i++) {
    if (!strcmp(monthNames[i], month)) {
      monthHint = i;
      return i;
    }
  }

  fprintf(stderr, "unknown month %s\n", month);
  Exit::DoExit(2);

  // make compiler happy
  return 0;
}

int ParseFloatDate(char *input, double &val)
{
  char line[256];
  strcpy(line, input);

  // convert string to upper case
  char *ptr = line;
  for(; *ptr; ptr++)
    if (isalpha(*ptr))
      *ptr = toupper(*ptr);

  int numArgs;
  char **args;
  Parse(line, numArgs, args);

  // if there is a single argument, it is the date already in numeric format
  if (numArgs == 1) {
    val = atof(args[0]);
    return 1;
  }

  // otherwise, date must be in the format: MMM DD YYYY HH:MM:SS
  if (numArgs == 4) {
    if (strlen(args[0]) != 3)
      goto error;
    if (strlen(args[2]) != 4)
      goto error;
    if (strlen(args[3]) != 8)
      goto error;
    
    struct tm timestr;
    timestr.tm_mon = GetMonth(args[0]);
    timestr.tm_mday = atoi(args[1]);
    timestr.tm_year = atoi(args[2])- 1900;
    args[3][2] = '\0';
    args[3][5] = '\0';
    timestr.tm_hour = atoi(args[3]);
    timestr.tm_min = atoi(&args[3][3]);
    timestr.tm_sec = atoi(&args[3][6]);
    timestr.tm_isdst = -1;

    val = (double)mktime(&timestr);
    return 1;
  }

 error:
  val = 0.0;
  return 0;
}

time_t ParseSQLTimestamp(char *str)
{
  struct tm t;
  if (sscanf(str, "%d-%d-%d %d:%d:%d", &t.tm_year, &t.tm_mon,
	     &t.tm_mday, &t.tm_hour, &t.tm_min, &t.tm_sec) != 6) {
    fprintf(stderr, "Invalid date field: %s\n", str);
    return (time_t)-1;
  }

  t.tm_year -= 1900;
  t.tm_mon--;
  t.tm_isdst = -1;

#ifdef DEBUG
  printf("Converting time %04d-%02d-%02d %02d:%02d:%02d\n", t.tm_year,
	 t.tm_mon, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec);
#endif

  time_t tim = mktime(&t);

  return tim;
}
