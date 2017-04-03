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
  $Id: Init.h 2396 2014-03-13 21:13:27Z wkliao $

  $Log: Init.h,v $
  Revision 1.13  1996/12/15 20:21:42  wenger
  Added '-noshm' command line flag to allow user to disable shared memory;
  temporarily disabled RTree stuff.

  Revision 1.12  1996/12/03 20:24:09  jussi
  Removed unused command line parameters. Changed BufPolicies().

  Revision 1.11  1996/11/23 21:23:11  jussi
  Removed variables and methods not used anywhere.

  Revision 1.10  1996/10/07 22:53:50  wenger
  Added more error checking and better error messages in response to
  some of the problems uncovered by CS 737 students.

  Revision 1.9  1996/09/12 18:37:45  wenger
  Added optional delay before drawing images.

  Revision 1.8  1996/09/05 20:00:12  jussi
  Added screenWidth and screenHeight command line arguments.

  Revision 1.7  1996/09/04 21:24:49  wenger
  'Size' in mapping now controls the size of Dali images; improved Dali
  interface (prevents Dali from getting 'bad window' errors, allows Devise
  to kill off the Dali server); added devise.dali script to automatically
  start Dali server along with Devise; fixed bug 037 (core dump if X is
  mapped to a constant); improved diagnostics for bad command-line arguments.

  Revision 1.6  1996/08/23 16:55:36  wenger
  First version that allows the use of Dali to display images (more work
  needs to be done on this); changed DevStatus to a class to make it work
  better; various minor bug fixes.

  Revision 1.5  1996/04/18 18:12:59  jussi
  Replaced 'postScript' member variable and method with more appropriate
  name 'batchFile.'

  Revision 1.4  1996/02/05 23:56:55  jussi
  Added DEVise logo display.

  Revision 1.3  1996/01/27 00:22:11  jussi
  Added _postScript variable.

  Revision 1.2  1995/09/05 21:12:53  jussi
  Added/updated CVS header.
*/

#ifndef Init_h
#define Init_h

#include "DeviseTypes.h"
#include "BufPolicy.h"

const int DEVISE_MAX_TDATA_ATTRS = 512;

class Init {
  public:
    static void DoInit(int &argc, char **argv);

    static Boolean SavePopup() { return _savePopup;}

    static char *PlaybackFileName(){ return _playbackFile; }
    static Boolean DoPlayback() { return _doPlayback; }
  
    /*
       Get buffer manager policies.
       bufSize = buffer size, in # of pages
       policy : buffer policy to use
    */
    static void BufParams(int &bufSize, int &streamBufSize,
                          BufPolicy::policy &policy) {
        bufSize = _bufferSize;
        streamBufSize = _streamBufSize;
        policy = _policy;
    }
    static BufPolicy::policy Policy() { return _policy; }
	
    static Boolean TDataQuery(){ return _tdataQuery; }
    static Boolean ConvertGData() { return _convertGData; }
    static int MaxGDataPages() { return _gdataPages; }
    static Boolean Randomize(){ return _randomize;}

    static char *ProgName() { return _progName; }
    static long ProgModTime() { return _progModTime; }
    static Boolean DisplayLogo() { return _dispLogo; }
    static Boolean DoAbort() { return _abort; }

    static char *WorkDir() { return _workDir; }
    static char *TmpDir() { return _tmpDir; }
    static char *CacheDir() { return _cacheDir; }
  
    static char *SessionName() { return _sessionName; }
    static void SetSessionName(char *name) { _sessionName = name; }
    static Boolean Restore() { return _restore; }
    static Boolean Iconify() { return _iconify; }

    static int PageSize() { return _pageSize; }
    static Boolean ElimOverlap() { return _elimOverlap; }

    static Boolean UseSimpleInterpreter() { return _simpleInterpreter; }
    static Boolean PrintTDataAttr() { return _printTDataAttr; }
    static Boolean DispGraphics() { return _dispGraphics; }
    static Boolean BatchRecs() { return _batchRecs; }
    static Boolean PrintViewStat() { return _printViewStat; }
    
    static char *BatchFile() { return _batchFile; }
    
    static char *DaliServer() { return _daliServer; }
    static Boolean DaliQuit() { return _daliQuit; }
    static int ImageDelay() { return _imageDelay; }
    
    static int ScreenWidth() { return _screenWidth; }
    static int ScreenHeight() { return _screenHeight; }

    static Boolean UseSharedMem() { return _useSharedMem; }
    
protected:
    static Boolean _savePopup;     /* true if pop-up window should be saved and
                                      wait for button even to remove it */

    static char *_playbackFile;    /* name of the playback file */
    static Boolean _doPlayback;    /* true if playback requested */

    static int _bufferSize;        /* buffer size */
    static int _streamBufSize;     /* streaming buffer size */
    static BufPolicy::policy _policy;

    static Boolean _tdataQuery; 
    static Boolean _convertGData;  /* true if bg GData conversion enabled */
    static int _gdataPages;        /* amount of GData space available */
    static Boolean _randomize;     /* TRUE if TData retrieval is randomized */

    static char *_progName;        /* name of program */
    static long _progModTime;      /* last time program was modified */
    static Boolean _dispLogo;      /* true if Devise logo displayed */
    static Boolean _abort;         /* true if Devise should abort on error */

    static char *_workDir;         /* name of work directory */
    static char *_tmpDir;          /* name of temp directory */
    static char *_cacheDir;        /* name of cache directory */

    static char *_sessionName;     /* name of current session */
    static Boolean _restore;       /* true if Devise is in restore mode */
    static Boolean _iconify;       /* true if session restored iconified */

    static int _pageSize;          /* page size */
    static Boolean _elimOverlap;   /* true if overlap elimination enabled */

    static Boolean _simpleInterpreter;/* true if simple mapping interpreter */
    static Boolean _printTDataAttr;/* true if TData attributes printed */
    static Boolean _dispGraphics;  /* true if graphics should be displayed */
    static Boolean _batchRecs;     /* true if records processed in batches */
    static Boolean _printViewStat; /* true if view statistics printed */
    
    static char *_batchFile;       /* name of batch file */
    
    static char *_daliServer;      /* host name of Tasvir image server */
    static Boolean _daliQuit;      /* true if Tasvir abort requested */
    static int _imageDelay;        /* delay before displaying Tasvir image */
    
    static int _screenWidth;       /* requested screen width */
    static int _screenHeight;      /* requested screen height */

    static Boolean _useSharedMem;  /* use shared memory */
};

#endif
