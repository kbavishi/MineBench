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
  $Id: Dispatcher.h 2396 2014-03-13 21:13:27Z wkliao $

  $Log: Dispatcher.h,v $
  Revision 1.24  1996/12/12 22:01:23  jussi
  Cleaned up termination code and added CheckUserInterrupt() method.

  Revision 1.23  1996/08/14 21:22:48  wenger
  Minor dispatcher cleanups, etc.  Fixed release script to release
  statically-linked executables for HP and Sun.

  Revision 1.22  1996/08/07 15:15:19  guangshu
  Add include string.h.

  Revision 1.21  1996/08/04 21:00:41  beyer
  Reorganized and simplified the dispatcher.  Multiple dispatchers are no
  longer allowed.  Inserting and removing callbacks uses one list.

  Revision 1.20  1996/07/29 15:44:36  wenger
  Corrected compile warnings on HP.

  Revision 1.19  1996/07/24 03:36:03  wenger
  Fixed dispatcher to compile for HP.

  Revision 1.18  1996/07/23 19:33:55  beyer
  Changed dispatcher so that pipes are not longer used for callback
  requests from other parts of the code.

  Revision 1.17  1996/06/24 20:04:35  jussi
  Added inclusion of sys/select.h for SOLARIS and AIX.

  Revision 1.16  1996/06/24 19:40:06  jussi
  Cleaned up the code a little.

  Revision 1.15  1996/06/23 20:36:17  jussi
  Minor fix with header files.

  Revision 1.14  1996/06/23 20:31:21  jussi
  Cleaned up marker and pipe mechanism. Moved a couple #defines to
  the .c file so that not all of Devise needs to be recompiled when
  one of them is changed.

  Revision 1.13  1996/06/12 14:55:34  wenger
  Added GUI and some code for saving data to templates; added preliminary
  graphical display of TDatas; you now have the option of closing a session
  in template mode without merging the template into the main data catalog;
  removed some unnecessary interdependencies among include files; updated
  the dependencies for Sun, Solaris, and HP; removed never-accessed code in
  ParseAPI.C.

  Revision 1.12  1996/05/09 18:12:06  kmurli
  No change to this makefile.

  Revision 1.11  1996/04/30 15:58:46  jussi
  Added #ifdef USE_SELECT which can be used to disable the use
  of select(). The display update problem still plagues the
  system when select() is used. The query processor also seems
  slower.

  Revision 1.10  1996/04/20 19:52:07  kmurli
  Changed Viex.c to use a pipe mechanism to call itself if it needs to be
  done again. The view now is not called contiously by the Dispatcher,instead
  only of there is some data in the pipe.
  The pipe mechanism is implemented transparently through static functions
  in the Dispatcher.c (InsertMarker,CreateMarker,CloseMarker,FlushMarker)

  Revision 1.9  1996/04/09 18:56:00  jussi
  Minor change to make this file compile under HP-UX.

  Revision 1.8  1996/04/09 18:04:09  jussi
  Collection of fd's (fdset) now assembled and disassembled in
  Register/Unregister instead of Run1. Callbacks to be deleted
  are first appended to a delete list, and collectively removed
  at the beginning of Run1.

  Revision 1.7  1996/04/08 16:56:14  jussi
  Changed name of DisplaySocketId to more generic 'fd'.

  Revision 1.6  1996/04/04 05:18:26  kmurli
  Major modification: The dispatcher now receives the register command
  from the displays directly (i.e. from XDisplay instead of from
  Display) corrected a bug in call to register function. Also now
  dispatcher uses socket number passed from the XDisplay class to
  select on it and call the relevant functions.

  Revision 1.5  1996/01/27 00:20:31  jussi
  QuitNotify() is now defined in .c file.

  Revision 1.4  1996/01/11 21:57:08  jussi
  Replaced libc.h with stdlib.h.

  Revision 1.3  1995/12/28 18:15:54  jussi
  Added copyright notice and fixed for loop variable scope.

  Revision 1.2  1995/09/05 21:12:42  jussi
  Added/updated CVS header.
*/

#ifndef Dispatcher_h
#define Dispatcher_h

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#if defined(SOLARIS) || defined(AIX)
#include <sys/select.h>
#endif

#include "DeviseTypes.h"
#include "DList.h"
#include "Journal.h"
#include "Exit.h"


class DispatcherCallback {
public:
  virtual char *DispatchedName() = 0;
  virtual void Run() {}
  virtual void Cleanup() {}
};

typedef unsigned StateFlag;
const unsigned GoState   = 0x1;
const unsigned StopState = 0x2;
const unsigned AllState  = 0xffffffff;

struct DispatcherInfo {
public:
  DispatcherCallback *callBack;
  StateFlag flag;		// if flag is zero, this info will be deleted
  int priority;
  int fd;
  bool callback_requested;
};

/* pointers to the DispatcherInfo are used as identifiers when the
   user registers a callback, but they should not be peeked at! */
typedef DispatcherInfo* DispatcherID;

class DeviseWindow;
class Dispatcher;
class View;
class Selection;

/* the one and only global dispatcher */
extern Dispatcher dispatcher;

//DefinePtrDList(DeviseWindowList, DeviseWindow *);
DefinePtrDList(DispatcherInfoList, DispatcherInfo *);


class Dispatcher {
public:
  Dispatcher(StateFlag state = GoState );

  ~Dispatcher() {}

  /* schedule a callback
     note: it doesn't matter how many times this is called, the callback
     will only be made once & one cancel can kill 10 requests.
     Also, once the callback is made, it will not be made again unless
     the callback reschedules itself.
   */
  void RequestCallback(DispatcherID info) {
    DOASSERT(info, "bad dispatcher id");
    if( !(info->callback_requested) ) {
      info->callback_requested = true;
      _callback_requests++;
    } 
  }

  /* cancel a scheduled callback */
  void CancelCallback(DispatcherID info) {
    DOASSERT(info, "bad dispatcher id");
    if( info->callback_requested ) {
      info->callback_requested = false;
      _callback_requests--;
      DOASSERT(_callback_requests >= 0, "callback request count too low");
    } 
  }

  /* Register callback */
  DispatcherID Register(DispatcherCallback *c, int priority = 10,
			StateFlag flag = GoState, 
			Boolean ignored = false, // parameter no longer used
			int fd = -1); 
  
  /* Unregister callback */
  void Unregister(DispatcherCallback *c);

  /* Change the state of the dispatcher */
  void ChangeState(StateFlag flag) { _stateFlag = flag; }

  /* CGet the state of the dispatcher */
  StateFlag GetState() { return _stateFlag; }

  /* Single step */
  void Run1();
  
  /* Print what's in the queue */
  void Print();

  /***********************************************************************
    the following static functions are no longer needed, since there
    is only one dispatcher. I left them here to avoid changing a lot of
    code... KSB
  ***********************************************************************/

  /* Return the current dispatcher */
  static Dispatcher *Current() { return &dispatcher; }

  /* Run once, for single step */
  static void SingleStepCurrent() { dispatcher.Run1(); }

  /* Run, no return */
  static void RunNoReturn();

  /* Run continuously, but can return after ReturnCurrent() is called. */
  static void RunCurrent();

  /* Return from Run() */
  static void ReturnCurrent() { dispatcher._returnFlag = true; }
  
  /* Catch interrupts from the user and terminate program if necessary */
  static void Terminate(int dummy);

  /* Cleanup dispatcher */
  static void Cleanup() { dispatcher.DoCleanup(); }

  /* Check if user has hit interrupt (Control-C) */
  static void CheckUserInterrupt();

  /***********************************************************************/


private:

  /* Clean up before quitting */
  void DoCleanup();

  /* process any callbacks that have an fd set in fdread or fdexc,
     or any callbacks that have callback_requested set */
  void Dispatcher::ProcessCallbacks(fd_set& fdread, fd_set& fdexc);

  long _oldTime;		/* time when clock was read last */
  long _playTime;		/* time last read for playback */
  Boolean _playback;		/* TRUE if doing playback */

  /* Next event to be played back */
  long _playInterval;
  Journal::EventType _nextEvent;
  Selection *_nextSelection;
  View *_nextView;
  VisualFilter _nextFilter;
  VisualFilter _nextHint;

  /* Callback list for this dispatcher */
  DispatcherInfoList _callbacks;
  int                _callback_requests;

  StateFlag _stateFlag;
  Boolean _returnFlag;	/* TRUE if we should quit running and return */
  Boolean _firstIntr;	/* Set to true when dispatcher received interrupt */
  Boolean _quit;	/* Set to true when dispatcher should quit */
  
  /* Set of file descriptors to inspect for potential input */
  fd_set fdset;
  int maxFdCheck;
};

/*********************************************************
A class that automatically registers with the dispatcher
*********************************************************/

class DispatcherAutoRegister: public DispatcherCallback {
public:
  DispatcherAutoRegister() {
    Dispatcher::Current()->Register(this);
  }

  virtual ~DispatcherAutoRegister() {
    Dispatcher::Current()->Unregister(this);
  }
};


#endif
