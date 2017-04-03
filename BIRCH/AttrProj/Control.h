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
  $Id: Control.h 2396 2014-03-13 21:13:27Z wkliao $

  $Log: Control.h,v $
  Revision 1.18  1996/11/20 20:34:51  wenger
  Fixed bugs 062, 073, 074, and 075; added workaround for bug 063; make
  some Makefile improvements so compile works first time; fixed up files
  to correspond to new query catalog name.

  Revision 1.17  1996/09/05 21:31:12  jussi
  Moved DestroySessionData to Control.c.

  Revision 1.16  1996/08/29 22:00:50  guangshu
  Added functions OpenDataChannel and getFd for DEVise Server to open
  data channel and get the fd of that socket.

  Revision 1.15  1996/08/07 19:25:32  jussi
  Added methods which allow query processor to control when
  a synchronization message is sent to client.

  Revision 1.14  1996/08/04 20:57:22  beyer
  Added Raise() method to bring the control panel to the top of the stacking
  order.

  Revision 1.13  1996/07/18 01:19:02  jussi
  DestroySessionData() now resets the _batchMode flag.

  Revision 1.12  1996/05/22 21:03:56  jussi
  ControlPanel::_controlPanel is now set by main program.

  Revision 1.11  1996/05/15 16:43:44  jussi
  Added support for the new server synchronization mechanism.

  Revision 1.10  1996/05/13 21:58:18  jussi
  Moved initialization of _mode to Control.c. Added support
  for changing _batchMode and querying it.

  Revision 1.9  1996/05/13 18:05:38  jussi
  Changed type of "flag" argument to ReturnVal().

  Revision 1.8  1996/05/11 19:10:30  jussi
  Added virtual function prototypes for replica management.

  Revision 1.7  1996/05/11 17:28:45  jussi
  Reorganized the code somewhat in order to match the ParseAPI
  interface.

  Revision 1.6  1996/05/11 03:11:32  jussi
  Removed all unnecessary ControlPanel methods like FileName(),
  FileAlias() etc.

  Revision 1.5  1996/05/09 18:12:02  kmurli
  No change to this makefile.

  Revision 1.4  1996/04/16 19:45:11  jussi
  Added DoAbort() method.

  Revision 1.3  1996/01/27 00:21:45  jussi
  Added ExecuteScript() method.

  Revision 1.2  1995/09/05 21:12:32  jussi
  Added/update CVS header.
*/

#ifndef Control_h
#define Control_h

#include <sys/types.h>

#include "DeviseTypes.h"
#include "VisualArg.h"
#include "DList.h"
#include "ClassDir.h"

class Dispatcher;

struct CPViewList;
class View;

DefinePtrDList(CPViewListList,CPViewList *);

class ClassInfo;

class ControlPanelCallback;
class MapInterpClassInfo;
class GroupDir;

DefinePtrDList(ControlPanelCallbackList ,ControlPanelCallback *);

class DeviseDisplay;

class ControlPanel  {
public:
  enum Mode { DisplayMode, LayoutMode };

  void InsertCallback(ControlPanelCallback *callback);
  void DeleteCallback(ControlPanelCallback *callback);
  
  /* Register class with control panel.
     transient == true if it's a transient class to be removed
     when closing a session.*/
  static void RegisterClass(ClassInfo *cInfo, Boolean transient = false);

  /* Make view the current view */
  virtual void SelectView(View *view) = 0;

  /* Find pointer to instance with given name */
  static void *FindInstance(char *name) {
    return GetClassDir()->FindInstance(name);
  }

  /* Get/set current mode */
  virtual Mode GetMode() { return _mode; }
  virtual void SetMode(Mode mode) { _mode = mode; }

  /* Set busy status, should be called in pairs. */
  virtual void SetBusy() = 0;
  virtual void SetIdle() = 0;

  /* Get current busy status */
  virtual Boolean IsBusy() = 0;

  /* Start/restart session */
  virtual void StartSession() {}
  virtual void DestroySessionData();
  virtual void RestartSession() {}

  /* Get/set batch mode */
  virtual Boolean GetBatchMode() { return _batchMode; }
  virtual void SetBatchMode(Boolean mode) { _batchMode = mode; }

  /* Set/clear/get sync notify status */
  virtual void SetSyncNotify() { _syncNotify = true; }
  virtual void ClearSyncNotify() { _syncNotify = false; }
  virtual Boolean GetSyncNotify() { return _syncNotify; }
  virtual void SyncNotify() {}

  /* Set/clear/get sync allowed status */

  virtual void SetSyncAllowed() { _syncAllowed = true; }
  virtual void ClearSyncAllowed() { _syncAllowed = false; }
  virtual Boolean GetSyncAllowed() { return _syncAllowed; }

  /* Raise the control panel */
  virtual void Raise() {}

  /* Instantiate control panel into display */
  static void InsertDisplay(DeviseDisplay *disp,
			    Coord x = 0.0, Coord y = 0.4, 
			    Coord w = 0.15, Coord h = 0.59) {
    Instance()->SubclassInsertDisplay(disp, x, y, w, h);
  }

  /* Init control panel, before dispatcher starts running.
     Create control panel if not already created.
     Update contronl panel state to reflect current dispatcher
     */
  static void Init() {
    Instance()->SubclassDoInit();
  }

  /* return the one and only instance of control panel */
  static ControlPanel *Instance();

  /* report mode change */
  void ReportModeChange(Mode mode);

  /* quit */
  virtual void DoQuit();

  /* abort */
  virtual void DoAbort(char *reason) {}

  /* return one or multiple values to caller of API */
  virtual int ReturnVal(u_short flag, char *result) = 0;
  virtual int ReturnVal(int argc, char **argv) = 0;
  
  /* Get ClassDir info */
  static ClassDir *GetClassDir();
  
  /* Get GroupDir info */
  virtual GroupDir *GetGroupDir() = 0;
  
  /* Get MapInterpClassInfo info */
  virtual MapInterpClassInfo *GetInterpProto() = 0;

  /* Add replica server */
  virtual int AddReplica(char *hostName, int port) = 0;

  /* Remove replica server */
  virtual int RemoveReplica(char *hostName, int port) = 0;

  virtual void OpenDataChannel(int port) = 0;
  virtual int getFd() = 0;

  /* Control panel instance */
  static ControlPanel *_controlPanel;

protected:
  friend class Dispatcher;

  virtual void SubclassInsertDisplay(DeviseDisplay *disp,
				     Coord x, Coord y, 
				     Coord w, Coord h) = 0;
  virtual void SubclassDoInit() = 0;

  /* helper functions for derived classes */

  /* return */
  void DoReturn();

  /* Change context and reset current view */
  void DoContext() {}

  /* do go/stop */
  void DoGo(Boolean state);

  /* do single step */
  void DoStep();

  /* display symbols or connectors */
  void DoDisplaySymbols(Boolean state) {}
  void DoDisplayConnectors(Boolean state) {}

  /* display/not display axes for current view. */
  void DoDisplayCurrentAxes(Boolean stat) {}
  
  /* display/not display axes for all views in the dispatcher.*/
  void DoDisplayAxes(Boolean stat) {}

  /* change one of the visual filters. 
     on == TRUE means turn on the filter, otherwise, turn off the filter.
     flag is one of: VISUAL_COLOR or VISUAL_PATTERN */
  void ChangeIntVisualFilter(Boolean on, VisualFlag flag,int minVal,
			     int maxVal) {}

  /* change one of the visual filters. 
     on == TRUE means turn on the filter, otherwise, turn off the filter.
     flag is one of: VISUAL_SIZE or VISUAL_ORIENTATION.
     Note: input orientations are in degrees.
     */
  void ChangeFloatVisualFilter(Boolean on, VisualFlag flag,double minVal,
			       double maxVal) {}

  /* scrol current view by the amount proportional to its
     current width.
     0 <= abs(amount) <= 1, amount >0 means scroll right,
     otherwise, scroll left
     */
  void DoScrollX(double amount) {}
  void DoScrollY(double amount) {}

  /* zoom current width by the given amount */
  void DoZoomXY(double amount) {}
  void DoZoomX(double amount) {}
  void DoZoomY(double amount) {}

  /* do initialize before dispatche starts running. This is used
     to get control panel's notion of current dispatcher in synch
     with the dispatchers' notion of current dispatcher.
     Return control panel switched internally to a new current dispatcher */
  Boolean DoInit() { return false; }

  ControlPanel();
  static Mode _mode;                    // layout or display mode
  static Boolean _batchMode;            // true if we're in batch mode
  static Boolean _syncNotify;           // true if sync notify needed
  static Boolean _syncAllowed;          // true when qp allowed to synchronize

private:
  void UpdateNewDispatcher() {}

  static ClassDir *_classDir; 

  ControlPanelCallbackList *_callbacks;
};

class ControlPanelCallback {
public:
  virtual void ModeChange(ControlPanel::Mode mode) = 0;
};

extern ControlPanel *GetNewControl();

#endif
