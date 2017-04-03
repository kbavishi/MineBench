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
  $Id: ApInit.c 2396 2014-03-13 21:13:27Z wkliao $

  $Log: ApInit.c,v $
  Revision 1.4  1996/10/07 22:53:41  wenger
  Added more error checking and better error messages in response to
  some of the problems uncovered by CS 737 students.

  Revision 1.3  1996/08/02 15:53:31  wenger
  Added AttrProj member functions for reading entire records (no projection).

  Revision 1.2  1996/05/22 18:50:39  wenger
  Greatly simplified Init::DoInit() to only do what's necessary for
  attribute projection; other minor changes.

  Revision 1.1  1996/04/22 18:01:43  wenger
  First version of "attribute projection" code.  The parser (with
  the exception of instantiating any TData) compiles and runs.

*/

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>

//#include "Dispatcher.h"
#include "Exit.h"
#include "ApInit.h"
#include "Time.h"
#include "Timer.h"
#include "BufPolicy.h"
#include "Util.h"


/*************************************************************
Create unique temporary file name 
**************************************************************/

static char uniqueFileName[100];
static char *CreateUniqueFileName(char *progname)
{
  progname = StripPath(progname);
  pid_t pid = getpid();
  for(char char1 = 'a'; char1 <= 'z'; char1++) {
    for(char char2 = 'a'; char2 <= 'z'; char2++) {
      sprintf(uniqueFileName, "work/%s_%05ld%c%c", progname,
	      (long)pid, char1, char2);
      int fd = open(uniqueFileName, O_WRONLY, 0666);
      if (fd < 0)
	return uniqueFileName;
      close(fd);
    }
  }

  DOASSERT(0, "Cannot create unique temporary file name");

  return NULL; /* keep compiler happy */
}

Boolean Init::_savePopup = false; /* TRUE if save pop up window 
						and wait for button event */
Boolean Init::_doPlayback = false; /* TRUE if do journal playback */
char *Init::_playbackFile = "";	/* name of playback file */
Boolean Init::_prefetch = true; /* TRUE if buffer manager does prefetch */
int Init::_bufferSize= 1536;		/* size of buffer */
BufPolicy::policy Init::_policy= BufPolicy::FIFO; /* policy for buffer manager*/
/* TRUE if existing buffers should be checked first in query processing */
Boolean Init::_existing = true;	
Boolean Init::_tdataQuery = false; /* TRUE if file is tdata, else gdata */
Boolean Init::_convertGData = true; /* true if TData is converted into G
									while system is idle */
Boolean Init::_abort = false; /* TRUE if abort instead of exit() on program 
							exit */
Boolean Init::_iconify= false; /* TRUE if windows are iconified when
							restoring a session */
int Init::_gdataPages = -1; /* max # of disk pages for gdata */
char *Init::_progName = 0; /* name of program */
char *Init::_workDir = 0; /* name of work directory */
char *Init::_tmpDir = 0;/* name of temp directory */
char *Init::_cacheDir = 0;/* name of cache directory */
char *Init::_sessionName = "session.tk";	/* name of program */
Boolean Init::_dispLogo = true;
char *Init::_batchFile = 0;
char *Init::_qpName = "default"; /* name of query processor */
Boolean Init::_restore = false; /* TRUE if we need to restore a session file */
long Init::_progModTime;	/* when program was modified */
Boolean Init::_randomize = true; /* true if TData fetches are randomized */
int Init::_pageSize = 16384;	/* size of page */
Boolean Init::_hasXLow=false, Init::_hasYLow = false;
Boolean Init::_hasXHigh=false, Init::_hasYHigh = false;
Coord Init::_xLow, Init::_yLow, Init::_xHigh, Init::_yHigh;
Boolean Init::_simpleInterpreter = true; /* true if interpreted
	mapping should use its own interpreter to process
	simple commands instead of calling tcl */
Boolean Init::_printTDataAttr = false; /* true to print
	TData attribute list when it's created */
Boolean Init::_elimOverlap = true; /* true if overlapping
	GData should be eliminated while drawing */
Boolean Init::_dispGraphics = true; /* true to display graphics */
Boolean Init::_batchRecs = true; /* true if batching records */
Boolean Init::_printViewStat = false;  /* true to print view statistics */

/**************************************************************
Remove positions from index to index+len-1 from argv
Update argc.
***************************************************************/

static void MoveArg(int &argc, char **argv, int index, int len)
{
  DOASSERT(index + len <= argc, "Argument too long");

  for(int j = index + len; j < argc; j++)
    argv[j - len] = argv[j];

  argc -= len;
}

static void Usage(char *prog)
{
  fprintf(stderr, "Usage: %s [options]\n", prog);
  fprintf(stderr, "\nOptions are:\n");
  fprintf(stderr, "\t-journal file: name of journal file\n");
  fprintf(stderr, "\t-play file: journal file to play back\n");
  fprintf(stderr, "\t-buffer size: buffer size in pages\n");
  fprintf(stderr, "\t-prefetch yes_no: do prefetch or not\n");
  fprintf(stderr, "\t-policy policy: buffer replacement policy, one of:\n");
  fprintf(stderr, "\t                lru, fifo, lifo, focal, or rnd\n");
  fprintf(stderr, "\t-existing yes_no: use existing buffers first or not\n");
  fprintf(stderr, "\t-norandom: don't randomize record retrieval\n");
  fprintf(stderr, "\t-batch file: batch file to execute\n");
  Exit::DoExit(1);
}

static void CatchInt(int)
{
#if 0
  Dispatcher::QuitNotify();
#endif
}

void Init::DoInit()
{
#if 0
  /* set up interrupt handling for INTR */
  (void)signal(SIGINT, CatchInt);
#endif

#if 1
  /* Create work directory, if needed */
  char *workDir = "/tmp";
  _workDir = CopyString(workDir);
#endif

  /* Get name of cache directory. */
  char *cacheDir = getenv("DEVISE_CACHE");
  if (!cacheDir) cacheDir = ".";
  _cacheDir = CopyString(cacheDir);

#if 0
  char *journalName = NULL;
#define MAXARGS 512
  char *args[512];

  DOASSERT(argc <= MAXARGS, "Too many arguments");
#endif

#if 0
  for(int j = 0; j < argc; j++)
    args[j] = argv[j];
  
  /* init current time */
  DeviseTime::Init();

#if 0
  Timer::InitTimer();
#endif

  _progName = CopyString(argv[0]);
  _progModTime = ModTime(argv[0]);

  char *tmpDir =  getenv("DEVISE_TMP");
  if (!tmpDir)
    tmpDir = "tmp";

  CheckAndMakeDirectory(tmpDir);
  pid_t pid = getpid();
  char buf[512];
  DOASSERT(strlen(tmpDir) + 20 <= 512, "String space too small");
  sprintf(buf, "%s/DEVise_%ld", tmpDir, (long)pid);
  CheckAndMakeDirectory(buf, true);
  _tmpDir = CopyString(buf);

  /* parse parameters */
  int i = 1;
  while (i < argc) {
    if (argv[i][0] == '-') {

      if (strcmp(&argv[i][1], "queryProc") == 0) {
	if (i >= argc-1)
	  Usage(argv[0]);
	_qpName = CopyString(argv[i+1]);
	MoveArg(argc,argv,i,2);
      }

      else if (strcmp(&argv[i][1], "batch") == 0) {
	if (i >= argc - 1)
	  Usage(argv[0]);
	_batchFile = CopyString(argv[i + 1]);
	MoveArg(argc,argv,i,2);
      }

      else if (strcmp(&argv[i][1], "session") == 0) {
	if (i >= argc-1)
	  Usage(argv[0]);
	_sessionName = CopyString(argv[i + 1]);
	_restore = true;
	MoveArg(argc,argv,i,2);
      }

      else if (strcmp(&argv[i][1], "playback") == 0) {
	if (i >= argc-1)
	  Usage(argv[0]);
	_playbackFile = CopyString(argv[i+1]);
	_doPlayback = true;
	MoveArg(argc,argv,i,2);
      }

      else if (strcmp(&argv[i][1], "journal") == 0) {
	if (i >= argc -1)
	  Usage(argv[0]);
	journalName = CopyString(argv[i+1]);
	MoveArg(argc,argv,i,2);
      }

      else if (strcmp(&argv[i][1], "buffer") == 0) {
	if (i >= argc -1)
	  Usage(argv[0]);
	_bufferSize = atoi(argv[i+1]);
	if (_bufferSize <= 0) {
	  fprintf(stderr, "invalid buffer size %d\n", _bufferSize);
	  Usage(argv[0]);
	}
	MoveArg(argc,argv,i,2);
      }

      else if (strcmp(&argv[i][1], "pagesize") == 0) {
	if (i >= argc -1)
	  Usage(argv[0]);
	_pageSize = atoi(argv[i+1]);
	if (_pageSize <= 0) {
	  fprintf(stderr, "invalid buffer size %d\n", _pageSize);
	  Usage(argv[0]);
	}
	if ((_pageSize % 4096) != 0) {
	  fprintf(stderr, "page %d must be multiple of 4096\n", _pageSize);
	  Usage(argv[0]);
	}
	MoveArg(argc,argv,i,2);
      }

      else if (strcmp(&argv[i][1], "policy") == 0) {
	if (i >= argc -1)
	  Usage(argv[0]);
	if (strcmp(argv[i+1], "lru") == 0)
	  _policy = BufPolicy::LRU;
	else if (strcmp(argv[i+1], "fifo") == 0)
	  _policy = BufPolicy::FIFO;
	else if (strcmp(argv[i+1], "lifo") == 0)
	  _policy = BufPolicy::LIFO;
	else if (strcmp(argv[i+1], "rnd") == 0)
	  _policy = BufPolicy::RND;
	else if (strcmp(argv[i+1], "focal") == 0)
	  _policy = BufPolicy::FOCAL;
	else {
	  fprintf(stderr, "unknown policy %s\n", argv[i+1]);
	  Usage(argv[0]);
	}
	MoveArg(argc,argv,i,2);
      }

      else if (strcmp(&argv[i][1], "prefetch") == 0) {
	if (i >= argc -1)
	  Usage(argv[0]);
	_prefetch = !(atoi(argv[i+1]) == 0);
	MoveArg(argc,argv,i,2);
      }

      else if (strcmp(&argv[i][1], "printViewStat") == 0) {
	if (i >= argc -1)
	  Usage(argv[0]);
	_printViewStat = !(atoi(argv[i+1]) == 0);
	MoveArg(argc,argv,i,2);
      }

      else if (strcmp(&argv[i][1], "batchRecs") == 0) {
	if (i >= argc -1)
	  Usage(argv[0]);
	_batchRecs = !(atoi(argv[i+1]) == 0);
	MoveArg(argc,argv,i,2);
      }

      else if (strcmp(&argv[i][1], "dispGraphics") == 0) {
	if (i >= argc -1)
	  Usage(argv[0]);
	_dispGraphics = !(atoi(argv[i+1]) == 0);
	MoveArg(argc,argv,i,2);
      }

      else if (strcmp(&argv[i][1], "elimOverlap") == 0) {
	if (i >= argc -1)
	  Usage(argv[0]);
	_elimOverlap = !(atoi(argv[i+1]) == 0);
	MoveArg(argc,argv,i,2);
      }

      else if (strcmp(&argv[i][1], "existing") == 0) {
	if (i >= argc -1)
	  Usage(argv[0]);
	_existing = !(atoi(argv[i+1]) == 0);
	MoveArg(argc,argv,i,2);
      }

      else if (strcmp(&argv[i][1], "gdata") == 0) {
	if (i >= argc -1)
	  Usage(argv[0]);
	_tdataQuery = (atoi(argv[i+1]) == 0);
	MoveArg(argc,argv,i,2);
      }

      else if (strcmp(&argv[i][1], "gdatapages") == 0) {
	if (i >= argc -1)
	  Usage(argv[0]);
	_gdataPages = atoi(argv[i+1]);
	MoveArg(argc,argv,i,2);
      }

      else if (strcmp(&argv[i][1], "convert") == 0) {
	if (i >= argc -1)
	  Usage(argv[0]);
	_convertGData = !(atoi(argv[i+1]) == 0);
	MoveArg(argc,argv,i,2);
      }

      else if (strcmp(&argv[i][1], "iconify") == 0) {
	if (i >= argc -1)
	  Usage(argv[0]);
	_iconify = !(atoi(argv[i+1]) == 0);
	MoveArg(argc,argv,i,2);
      }

      else if (strcmp(&argv[i][1], "printTDataAttr") == 0) {
	if (i >= argc -1)
	  Usage(argv[0]);
	_printTDataAttr = !(atoi(argv[i+1]) == 0);
	MoveArg(argc,argv,i,2);
      }

      else if (strcmp(&argv[i][1], "simpleInterpreter") == 0) {
	if (i >= argc -1)
	  Usage(argv[0]);
	_simpleInterpreter = !(atoi(argv[i+1]) == 0);
	MoveArg(argc,argv,i,2);
      }

      else if (strcmp(&argv[i][1], "nologo") == 0) {
	_dispLogo = false;
	MoveArg(argc,argv,i,1);
      }

      else if (strcmp(&argv[i][1], "abort") == 0) {
	_abort = true;
	MoveArg(argc,argv,i,1);
      }

      else if (strcmp(&argv[i][1], "savePopup") == 0) {
	_savePopup = true;
	MoveArg(argc,argv,i,1);
      }

      else if (strcmp(&argv[i][1], "norandom") == 0) {
	_randomize = false;
	MoveArg(argc,argv,i,1);
      }

      else if (strcmp(&argv[i][1], "xlow") == 0) {
	if (i >= argc -1)
	  Usage(argv[0]);
	_xLow = atof(argv[i+1]);
	_hasXLow = true;
	MoveArg(argc,argv,i,2);
      }

      else if (strcmp(&argv[i][1], "ylow") == 0) {
	if (i >= argc -1)
	  Usage(argv[0]);
	_yLow = atof(argv[i+1]);
	_hasYLow = true;
	MoveArg(argc,argv,i,2);
      }

      else if (strcmp(&argv[i][1], "xhigh") == 0) {
	if (i >= argc -1)
	  Usage(argv[0]);
	_xHigh = atof(argv[i+1]);
	_hasXHigh = true;
	MoveArg(argc,argv,i,2);
      }

      else if (strcmp(&argv[i][1], "yhigh") == 0) {
	if (i >= argc -1)
	  Usage(argv[0]);
	_yHigh = atof(argv[i+1]);
	_hasYHigh = true;
	MoveArg(argc,argv,i,2);
      }
      else i++;
    }
    else i++;
  }
#endif

#if 0
  if (!journalName)
    journalName = CreateUniqueFileName(argv[0]);
#endif

#if 0
  Journal::Init(journalName, argc, args);
#endif
}

void Init::BufPolicies(int &bufSize, Boolean &prefetch,
		       BufPolicy::policy &policy, Boolean &existing)
{
  bufSize = _bufferSize;
  prefetch = _prefetch;
  policy = _policy;
  existing = _existing;
}

long Init::ProgModTime()
{
  return _progModTime;
}

int Init::PageSize()
{
  return _pageSize;
}

BufPolicy::policy Init::Policy()
{
  return _policy;
}

Boolean Init::GetXLow(Coord &xLow)
{ 
  xLow = _xLow;
  return _hasXLow;
}

Boolean Init::GetYLow(Coord &yLow)
{
  yLow = _yLow;
  return _hasYLow;
}

Boolean Init::GetXHigh(Coord &xHigh)
{
  xHigh = _xHigh;
  return _hasXHigh;
}

Boolean Init::GetYHigh(Coord &yHigh)
{
  yHigh = _yHigh;
  return _hasYHigh;
}
