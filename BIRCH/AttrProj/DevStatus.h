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
  Declaration of DevStatus class.
 */

/*
  $Id: DevStatus.h 2396 2014-03-13 21:13:27Z wkliao $

  $Log: DevStatus.h,v $
  Revision 1.1  1996/08/23 16:55:31  wenger
  First version that allows the use of Dali to display images (more work
  needs to be done on this); changed DevStatus to a class to make it work
  better; various minor bug fixes.

 */

#ifndef _DevStatus_h_
#define _DevStatus_h_


#include <sys/types.h>

#include "DeviseTypes.h"


// Status information (to be returned by a function, for example) and
// functions to get information about the status.
enum StatusVal {
  StatusInvalid = 0,
  StatusOk = 10000,     // Everything is OK.
  StatusFailed,         // Failure -- the function did not complete.
  StatusWarn,           // There was an error, but the function completed.
  StatusCancel,         // The function was cancelled.
  StatusWarnCancel      // There was a warning, and the function was cancelled.
};


class DevStatus
{
public:
  DevStatus() {_status = StatusInvalid;}
  DevStatus(const StatusVal status) {_status = status;}
  ~DevStatus() {}

  void operator=(const StatusVal &status) {_status = status;}
  void operator=(const DevStatus &status) {_status = status._status;}

  void operator+=(const StatusVal &status2) {_status =
    StatusCombine(_status, status2);}
  void operator+=(const DevStatus &status2) {_status =
    StatusCombine(_status, status2._status);}

  Boolean operator==(const StatusVal &status2) {return _status == status2;}
  Boolean operator==(const DevStatus &status2) {return _status ==
    status2._status;}

  Boolean operator!=(const StatusVal &status2) {return _status != status2;}
  Boolean operator!=(const DevStatus &status2) {return _status !=
    status2._status;}

  // Did the function complete successfully?
  Boolean IsComplete() {return StatIsComplete(_status);}

  // Was there an error?
  Boolean IsError() {return StatIsError(_status);}

  // Was there a warning?
  Boolean IsWarn() {return StatIsWarn(_status);}

  // Was the function cancelled (for example, by the user)?
  Boolean IsCancel() {return StatIsCancel(_status);}

  // Return string equivalent of _status.
  char *Value();

  void Print();

private:
  StatusVal _status;


  Boolean StatIsComplete(StatusVal status) { return (status == StatusOk) ||
    (status == StatusWarn); }

  Boolean StatIsError(StatusVal status) { return (status == StatusFailed) ||
    (status == StatusWarn) || (status == StatusWarnCancel); }

  Boolean StatIsWarn(StatusVal status) { return (status == StatusWarn) ||
    (status == StatusWarnCancel); }

  // Was the function cancelled (for example, by the user)?
  Boolean StatIsCancel(StatusVal status) { return (status == StatusCancel) ||
    (status == StatusWarnCancel); }

  StatusVal StatusCombine(StatusVal status1, StatusVal status2);
};


#endif /* _DevStatus_h_ */

/*============================================================================*/
