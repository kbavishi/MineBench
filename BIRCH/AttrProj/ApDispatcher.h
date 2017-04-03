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
  Dummy Dispatcher class for attribute projection.
 */

/*
  $Id: ApDispatcher.h 2396 2014-03-13 21:13:27Z wkliao $

  $Log: ApDispatcher.h,v $
  Revision 1.1  1996/06/17 19:16:44  wenger
  ApDispatcher.h never got committed, either.

*/

#ifndef Dispatcher_h
#define Dispatcher_h

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

#include "DeviseTypes.h"
#include "DList.h"
//#include "VisualArg.h"
//#include "Journal.h"
//#include "Exit.h"

class DispatcherCallback {
public:
  virtual char *DispatchedName() = 0;
  virtual void Run() {}
  virtual void Cleanup() {}
};

class DispatcherTimerCallback {
public:
  virtual void TimeUp() {}
};

typedef unsigned StateFlag;
const unsigned GoState   = 0x1;
const unsigned StopState = 0x2;
const unsigned AllState  = 0xffffffff;

struct DispatcherInfo {
public:
  DispatcherCallback *callBack;
  StateFlag flag;
  int priority;
  int fd;
};

class DeviseWindow;
class Dispatcher;
class View;
class Selection;

DefinePtrDList(DeviseWindowList,DeviseWindow *);
DefinePtrDList(DispatcherInfoList,DispatcherInfo *);
DefinePtrDList(DispatcherTimerCallbackList, DispatcherTimerCallback *);
DefinePtrDList(DispatcherList,Dispatcher *);

class Dispatcher {
public:
  Dispatcher(StateFlag state = GoState ) {};

  static void InsertMarker(int writeFd) {};
  static void FlushMarker(int readFd) {};
  static void CreateMarker(int &readFd,int& writeFd) {};
  static void CloseMarker(int readFd,int writeFd) {};

  virtual ~Dispatcher() {};

  /* Return the current dispatcher */
  static Dispatcher *Current() {return NULL;};

  /* Register to be called by dispatcher on timer up */
  static void RegisterTimer(DispatcherTimerCallback *callback) {};
  
  /* Unregister timer */
  static void UnregisterTimer(DispatcherTimerCallback *callback) {};

  /* Register window */
  void RegisterWindow(DeviseWindow *win) {};
  
  /* Unregister window */
  void UnregisterWindow(DeviseWindow *win) {};

  /* Register callback, all == TRUE if register with ALL dispatchers. */
  void Register(DispatcherCallback *c, int priority = 10,
		StateFlag flag = GoState, Boolean all = false,
		int fd = -1) {}; 
  
  /* Unregister callback */
  void Unregister(DispatcherCallback *c) {}; 

  /* Set/Change current dispatcher */
  static void SetCurrent(Dispatcher *p) {};

  /* Run once, for single step */
  static void SingleStepCurrent() {};

  /* Run continuously, but can return after ReturnCurrent() is called. */
  static void RunCurrent() {};

  /* Run, no return */
  static void RunNoReturn() {};

  /* Switch to next dispatcher */
  static void NextDispatcher() {};

  /* Return from run */
  static void ReturnCurrent() {};
  
  /* Notify dispatcher that we need to quit program */
  static void QuitNotify() {};

  /* Cleanup all dispatchers */
  static void Cleanup() {};

  /* Change the state of the dispatcher */
  void ChangeState(StateFlag flag) {};

  /* CGet the state of the dispatcher */
  StateFlag GetState() {return AllState;};

  /* Clean up before quitting */
  virtual void DoCleanup() {};

  /* Single step */
  virtual void Run1() {};
  
  /* Activate the dispatcher. Default: inform all windows  */
  void ActivateDispatcher() {};
  
  /* Deactivate dispatcher. Default: inform all windows */
  void DeactivateDispatcher() {};
  
  /* Do actual registration of timer */
  void DoRegisterTimer(DispatcherTimerCallback *c) {};

  /* Do actual unregistration of timer */
  void DoUnregisterTimer(DispatcherTimerCallback *c) {};
  
  /* Print what's in the queue */
  void Print() {};
};

#endif
