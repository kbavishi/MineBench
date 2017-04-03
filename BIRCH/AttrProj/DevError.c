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
  Implementation of DevError class.
 */

/*
  $Id: DevError.c 2396 2014-03-13 21:13:27Z wkliao $

  $Log: DevError.c,v $
  Revision 1.3  1996/09/09 14:31:39  jussi
  Added #ifdef LIBCS statements to make code compile in the
  ClientServer library target.

  Revision 1.2  1996/06/26 23:55:44  jussi
  Added method to turn on/off error reporting.

  Revision 1.1  1996/05/07 16:03:05  wenger
  Added final version of code for reading schemas from session files;
  added an error-reporting class to improve error info.

 */

#define _DevError_c_

#include "DevError.h"
#ifndef LIBCS
#include "Init.h"
#endif

/*
 * Static global variables.
 */
#if !defined(lint) && defined(RCSID)
static char		rcsid[] = "$RCSfile: DevError.c,v $ $Revision: 2396 $ $State: Exp $";
#endif

static char *	srcFile = __FILE__;

Boolean DevError::_enabled = true;

/*------------------------------------------------------------------------------
 * function: DevError::ReportError
 * Reports an error.  Right now, just prints info to stderr; we might want
 * to do something more sophisticated eventually.
 */
void
DevError::ReportError(char *message, char *file, int line, int err_no)
{
    char * progName = "DEVise";

    if (!_enabled)
        return;

    fprintf(stderr, "\n%s error: %s\n", progName, message);
    fprintf(stderr, "  at %s: %d\n", file, line);
    if (errno != devNoSyserr)
    {
        fprintf(stderr, "  syserr: %d: ", err_no);
        perror(NULL);
    }
    fprintf(stderr, "\n");
}

/*============================================================================*/
