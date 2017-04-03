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
  Implementation of DevStatus class.
 */

/*
  $Id: DevStatus.c 2396 2014-03-13 21:13:27Z wkliao $

  $Log: DevStatus.c,v $
  Revision 1.1  1996/08/23 16:55:30  wenger
  First version that allows the use of Dali to display images (more work
  needs to be done on this); changed DevStatus to a class to make it work
  better; various minor bug fixes.

 */

#define _DevStatus_c_

#include <stdio.h>

#include "DevStatus.h"


/*
 * Static global variables.
 */
#if !defined(lint) && defined(RCSID)
static char		rcsid[] = "$RCSfile: DevStatus.c,v $ $Revision: 2396 $ $State: Exp $";
#endif

static char *	srcFile = __FILE__;

/*------------------------------------------------------------------------------
 * function: DevStatus::Value
 * Returns string equivalent of _status value.
 */
char *
DevStatus::Value()
{
  char *result = NULL;
 
  switch (_status)
  {
  case StatusInvalid:
    result = "StatusInvalid";
    break;

  case StatusOk:
    result = "StatusOk";
    break;

  case StatusFailed:
    result = "StatusFailed";
    break;

  case StatusWarn:
    result = "StatusWarn";
    break;

  case StatusCancel:
    result = "StatusCancel";
    break;

  case StatusWarnCancel:
    result = "StatusWarnCancel";
    break;

  default:
    result = "illegal status value";
    break;
  }

  return result;
}

/*------------------------------------------------------------------------------
 * function: DevStatus::Print
 * Prints this object.
 */
void
DevStatus::Print()
{
//  printf("Status: %s\n", Value());
}

/*------------------------------------------------------------------------------
 * function: DevStatus::StatusCombine
 * Combines two StatusVals.
 */
StatusVal
DevStatus::StatusCombine(StatusVal status1, StatusVal status2)
{
  StatusVal result = StatusOk;

  if ((status1 == StatusInvalid) || (status2 == StatusInvalid))
  {
    result = StatusInvalid;
  }
  else if ((status1 == StatusFailed) || (status2 == StatusFailed))
  {
    result = StatusFailed;
  }
  else if (StatIsCancel(status1) || StatIsCancel(status2))
  {
    if (StatIsWarn(status1) || StatIsWarn(status2))
    {
      result = StatusWarnCancel;
    }
    else
    {
      result = StatusCancel;
    }
  }
  else if (StatIsWarn(status1) || StatIsWarn(status2))
  {
    result = StatusWarn;
  }
  else if ((status1 == StatusOk) && (status2 == StatusOk))
  {
    result = StatusOk;
  }
  else
  {
    result = StatusInvalid;
  }

  return result;
}

/*============================================================================*/
