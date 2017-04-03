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
  $Id: Timer.h 2396 2014-03-13 21:13:27Z wkliao $

  $Log: Timer.h,v $
  Revision 1.8  1996/08/02 00:38:12  jussi
  Added variable _nexthop where the length of the next hop
  is stored.

  Revision 1.7  1996/08/01 23:56:19  jussi
  Interval timer is now set for just one interval/interrupt at a
  time. There were occasional problems (SIGALRM's not caught properly)
  with the old scheme.

  Revision 1.6  1996/07/12 18:14:29  jussi
  Rewrote Timer code to use fewer timer interrupts and also
  allow for timer events which should precede any events
  already in the queue.

  Revision 1.5  1996/07/01 19:17:47  jussi
  Made StopTimer() and StartTimer() public.

  Revision 1.4  1996/06/24 19:34:09  jussi
  Fixed small bugs, removed unused code, and added some
  debugging statements.

  Revision 1.3  1996/06/23 20:46:40  jussi
  Cleaned up and added copyright notice.

  Revision 1.2  1995/09/05 21:13:09  jussi
  Added/updated CVS header.
*/

#ifndef Timer_h
#define Timer_h

#include "DeviseTypes.h"

class TimerCallback {
public:
  virtual void TimerWake(int arg) = 0;
};

struct TimerQueueEntry;

class Timer {
public:
  /* Queue timer.
     ms == # of milliseconds from now.
     callback == callback to call when time is up.
     arg == argument given back to callback.
     first == true if timer event should be placed first in queue.
  */
  static void Queue(long ms, TimerCallback *callback, int arg = 0,
                    Boolean first = false);

  /* Cancel timer event */
  static void Cancel(TimerCallback *callback, int arg = 0);

  /* Return current time */
  static long Now() { return _now; }

  /* StopTimer() and StartTimer() should be used in pairs to
     run code that must be run with timer off :
        StopTimer()
	Critical Section code
	StartTimer()
  */
  static void StopTimer();
  static void StartTimer();

private:
  /* Initialize the timer */
  static void InitTimer();

  /* Handler of timer interrupt */
  static void TimerHandler(int arg);

  static Boolean _initialized;  /* TRUE if timer has been initialized */
  static Boolean _inHandler;    /* TRUE if timer handler active */
  static Boolean _timerRunning; /* TRUE if timer running */

  static long _now;	        /* current time in ms from beginning */
  static long _nexthop;	        /* number of ms to hop until next event */

  static TimerQueueEntry *_head;        /* head of timer queue */
  static TimerQueueEntry *_freeHead;	/* head of free list */

  /* Allocate and free entry */
  static TimerQueueEntry *AllocEntry();
  static void FreeEntry(TimerQueueEntry *entry);
};

#endif
