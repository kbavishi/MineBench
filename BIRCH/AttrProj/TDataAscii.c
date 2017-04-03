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
  $Id: TDataAscii.c 2396 2014-03-13 21:13:27Z wkliao $

  $Log: TDataAscii.c,v $
  Revision 1.50  1996/12/03 20:32:58  jussi
  Updated to reflect new TData interface. Added support for concurrent I/O.

  Revision 1.49  1996/11/23 21:14:23  jussi
  Removed failing support for variable-sized records.

  Revision 1.48  1996/11/22 20:41:08  flisakow
  Made variants of the TDataAscii classes for sequential access,
  which build no indexes.

  ReadRec() method now returns a status instead of void for every
  class that has the method.

  Revision 1.47  1996/11/19 15:23:46  wenger
  Conditionaled out some debug code.

  Revision 1.46  1996/11/18 22:50:15  jussi
  Commented out debugging statement.

  Revision 1.45  1996/11/18 22:29:31  jussi
  Added estimation of total number of records in data set.

  Revision 1.44  1996/11/18 18:10:54  donjerko
  New files and changes to make DTE work with Devise

  Revision 1.43  1996/10/08 21:49:09  wenger
  ClassDir now checks for duplicate instance names; fixed bug 047
  (problem with FileIndex class); fixed various other bugs.

  Revision 1.42  1996/10/07 22:53:59  wenger
  Added more error checking and better error messages in response to
  some of the problems uncovered by CS 737 students.

  Revision 1.41  1996/10/04 17:24:16  wenger
  Moved handling of indices from TDataAscii and TDataBinary to new
  FileIndex class.

  Revision 1.40  1996/10/02 15:23:50  wenger
  Improved error handling (modified a number of places in the code to use
  the DevError class).

  Revision 1.39  1996/08/27 19:03:26  flisakow
    Added ifdef's around some informational printf's.

  Revision 1.38  1996/08/04 21:59:53  beyer
  Added UpdateLinks that allow one view to be told to update by another view.
  Changed TData so that all TData's have a DataSource (for UpdateLinks).
  Changed all of the subclasses of TData to conform.
  A RecFile is now a DataSource.
  Changed the stats buffers in ViewGraph to be DataSources.

  Revision 1.37  1996/07/13 01:32:33  jussi
  Moved initialization of i to make older compilers happy.

  Revision 1.36  1996/07/13 00:22:39  jussi
  Fixed bug in Initialize().

  Revision 1.35  1996/07/12 23:42:51  jussi
  Derived data sources are compiled in only for non-ATTRPROJ executables.

  Revision 1.34  1996/07/12 21:54:00  jussi
  Buffer data sources are not checkpointed.

  Revision 1.33  1996/07/12 19:36:08  jussi
  Modified code to handle derived data sources whose data is
  in a buffer but whose 'type' is one of many possible derived
  data types.

  Revision 1.32  1996/07/12 18:24:52  wenger
  Fixed bugs with handling file headers in schemas; added DataSourceBuf
  to TDataAscii.

  Revision 1.31  1996/07/05 15:20:01  jussi
  Data source object is only deleted in the destructor. The dispatcher
  now properly destroys all TData objects when it shuts down.

  Revision 1.30  1996/07/03 23:13:55  jussi
  Added call to _data->Close() in destructor. Renamed
  _fileOkay to _fileOpen which is more accurate.

  Revision 1.29  1996/07/02 22:48:33  jussi
  Removed unnecessary dispatcher call.

  Revision 1.28  1996/07/01 20:23:15  jussi
  Added #ifdef conditionals to exclude the Web data source from
  being compiled into the Attribute Projection executable.

  Revision 1.27  1996/07/01 19:28:05  jussi
  Added support for typed data sources (WWW and UNIXFILE). Renamed
  'cache' references to 'index' (cache file is really an index).
  Added support for asynchronous interface to data sources.

  Revision 1.26  1996/06/27 18:12:37  wenger
  Re-integrated most of the attribute projection code (most importantly,
  all of the TData code) into the main code base (reduced the number of
  modules used only in attribute projection).

  Revision 1.25  1996/06/27 15:49:31  jussi
  TDataAscii and TDataBinary now recognize when a file has been deleted,
  shrunk, or has increased in size. The query processor is asked to
  re-issue relevant queries when such events occur.

  Revision 1.24  1996/06/24 19:45:42  jussi
  TDataAscii no longer passes the fd of the open file to the
  Dispatcher. TDataAscii only needs the Dispatcher to call
  it when it's time to shut down; Cleanup() -> Checkpoint()
  functions get called at that time.

  Revision 1.23  1996/06/04 19:58:47  wenger
  Added the data segment option to TDataBinary; various minor cleanups.

  Revision 1.22  1996/06/04 14:21:39  wenger
  Ascii data can now be read from session files (or other files
  where the data is only part of the file); added some assertions
  to check for pointer alignment in functions that rely on this;
  Makefile changes to make compiling with debugging easier.

  Revision 1.21  1996/05/31 15:02:25  jussi
  Replaced a couple of occurrences of _data->isTape with _data->isTape().

  Revision 1.20  1996/05/22 17:52:13  wenger
  Extended DataSource subclasses to handle tape data; changed TDataAscii
  and TDataBinary classes to use new DataSource subclasses to hide the
  differences between tape and disk files.

  Revision 1.19  1996/05/07 16:43:00  jussi
  Cache file name now based on file alias (TData name). Added parameter
  to the Decode() function call.

  Revision 1.18  1996/04/20 19:56:56  kmurli
  QueryProcFull now uses the Marker calls of Dispatcher class to call
  itself when needed instead of being continuosly polled by the Dispatcher.

  Revision 1.17  1996/04/18 17:12:04  jussi
  Added missing #include <errno.h>.

  Revision 1.16  1996/04/18 17:04:59  jussi
  Fixed Checkpoint() which produced an unnecessary error message
  when a very small file (less than FILE_CONTENT_COMPARE_BYTES)
  was checkpointed.

  Revision 1.15  1996/04/16 20:38:50  jussi
  Replaced assert() calls with DOASSERT macro.

  Revision 1.14  1996/03/27 15:31:01  jussi
  Small fixes for tape TData.

  Revision 1.13  1996/03/26 21:18:43  jussi
  Merged with TDataTape. Added magic number to cache file.

  Revision 1.12  1996/03/05 22:06:04  jussi
  Minor fix in debugging output.

  Revision 1.11  1996/02/01 18:04:41  jussi
  Disabled 'Ignoring invalid record.' because this message shouldn't
  appear for ignored comments.

  Revision 1.10  1996/01/25 20:22:34  jussi
  Improved support for data files that grow while visualization
  is being performed.

  Revision 1.9  1996/01/12 15:24:45  jussi
  Replaced libc.h with stdlib.h.

  Revision 1.8  1996/01/09 16:35:00  jussi
  Improved console output messages.

  Revision 1.7  1995/12/28 19:59:41  jussi
  Small fixes to remove compiler warnings.

  Revision 1.6  1995/12/14 21:19:31  jussi
  Replaced 0x%x with 0x%p.

  Revision 1.5  1995/12/14 17:57:37  jussi
  Small fixes to get rid of g++ -Wall warnings.

  Revision 1.4  1995/11/24 21:34:55  jussi
  Added _currPos scheme to eliminate most of the fseek() calls.
  This appears to speed up execution significantly.

  Revision 1.3  1995/11/22 17:51:35  jussi
  Added copyright notice and cleaned up the code. Optimized some
  routines a la TDataTape.C.

  Revision 1.2  1995/09/05 22:15:49  jussi
  Added CVS header.
*/

using namespace std;

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <limits.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "Parse.h"
#include "TDataAscii.h"
#include "Exit.h"
#include "Util.h"
#include "DataSourceFileStream.h"
#include "DataSourceSegment.h"
#include "DataSourceTape.h"
#include "DataSourceBuf.h"
#include "DevError.h"
#include "DataSeg.h"
#include "QueryProc.h"

#ifdef ATTRPROJ
#   include "ApInit.h"
#else
#   include "Init.h"
#   include "DataSourceWeb.h"
#endif

# define  _STREAM_COMPAT

#define DEBUGLVL 0

static const int LINESIZE = 4096;         /* maximum size of a record */

/* We cache the first FILE_CONTENT_COMPARE_BYTES from the file.
   The next time we start up, this cache is compared with what's in
   the file to determine if they are the same file. */

static const int FILE_CONTENT_COMPARE_BYTES = 4096;


static char fileContent[FILE_CONTENT_COMPARE_BYTES];
static char indexFileContent[FILE_CONTENT_COMPARE_BYTES];
static char *   srcFile = __FILE__;

TDataAscii::TDataAscii(char *name, char *type, char *param, int recSize)
: TData(name, type, param, recSize)
{
#if DEBUGLVL >= 1
    printf("TDataAscii::TDataAscii(0x%p)(%s, %s, %s, %d)\n",
           this, name, type, param, recSize);
    printf("_data = 0x%p\n", _data);
#endif

    _fileOpen = true;
    if (_data->Open("r") != StatusOk)
      _fileOpen = false;
    
    DataSeg::Set(NULL, NULL, 0, 0);
    
    _bytesFetched = 0;
    
    _lastPos = 0;
    _currPos = 0;
    _lastIncompleteLen = 0;

    _totalRecs = 0;

    float estNumRecs = 0;

    if (_fileOpen) {
#ifdef CONCURRENT_IO
      if (_data->InitializeProc() < 0)
        fprintf(stderr, "Cannot use concurrent I/O to access data stream\n");
#endif

      /* Read first 10 records from data source and estimate record size. */

      float recSizeSum = 0;
      int i;
      for(i = 0; i < 10; i++) {
          char buf[LINESIZE];
          if (_data->Fgets(buf, LINESIZE) == NULL)
              break;
          recSizeSum += strlen(buf);
      }

      _data->Seek(0, SEEK_SET);
    
      if (i > 0)
        estNumRecs = 1.2 * _data->DataSize() / (recSizeSum / i);
    }

    _indexP = new FileIndex((unsigned long)estNumRecs);

#if DEBUGLVL >= 3
    printf("Allocated %lu index entries\n", (unsigned long)estNumRecs);
#endif

    Dispatcher::Current()->Register(this, 10, AllState, false, -1);
}

TDataAscii::~TDataAscii()
{
#if DEBUGLVL >= 1
  printf("TDataAscii destructor\n");
#endif

  if (_fileOpen) {
      if (_data->SupportsAsyncIO() &&_data->TerminateProc() < 0)
          fprintf(stderr, "Could not terminate data source process\n");
    _data->Close();
  }

  Dispatcher::Current()->Unregister(this);

  delete _indexP;
  delete _indexFileName;
}

Boolean TDataAscii::CheckFileStatus()
{
  CheckDataSource();

  /* See if file is no longer okay */
  if (!_data->IsOk()) {
    /* If file used to be okay, close it */
    if (_fileOpen) {
      if (_data->SupportsAsyncIO() &&_data->TerminateProc() < 0)
          fprintf(stderr, "Could not terminate data source process\n");
      Dispatcher::Current()->Unregister(this);
//      printf("Data stream %s is no longer available\n", _name);
      _data->Close();
      TData::InvalidateTData();
      _fileOpen = false;
    }
    Boolean old = DevError::SetEnabled(false);
    if (_data->Open("r") != StatusOk) {
      /* File access failure, get rid of index */
      _indexP->Clear();
      _initTotalRecs = _totalRecs = 0;
      _initLastPos = _lastPos = 0;
      _lastIncompleteLen = 0;
      (void)DevError::SetEnabled(old);
      return false;
    }
    (void)DevError::SetEnabled(old);
//    printf("Data stream %s has become available\n", _name);
    _fileOpen = true;
#ifdef CONCURRENT_IO
    if (_data->InitializeProc() < 0)
      fprintf(stderr, "Cannot use concurrent I/O to access data stream\n");
#endif
    Dispatcher::Current()->Register(this, 10, AllState, false, -1);
  }

  return true;
}

int TDataAscii::Dimensions(int *sizeDimension)
{
  sizeDimension[0] = _totalRecs;
  return 1;
}

Boolean TDataAscii::HeadID(RecId &recId)
{
  recId = 0;
  return (_totalRecs > 0);
}

Boolean TDataAscii::LastID(RecId &recId)
{
  if (!CheckFileStatus()) {
    recId = _totalRecs - 1;
    return false;
  }

  if (!_data->isTape()) {
    /* See if file has shrunk or grown */
    _currPos = _data->gotoEnd();
#if DEBUGLVL >= 5
//    printf("TDataAscii::LastID: currpos: %ld, lastpos: %ld\n", 
	   _currPos, _lastPos);
#endif
    if (_currPos < _lastPos) {
      /* File has shrunk, rebuild index from scratch */
      InvalidateTData();
    } else if (_currPos > _lastPos) {
      /* File has grown, build index for new records */
#if DEBUGLVL >= 3
//      printf("Extending index...\n");
#endif
      BuildIndex();
#ifndef ATTRPROJ
      QueryProc::Instance()->RefreshTData(this);
#endif
    }
  }
  
  recId = _totalRecs - 1;
  return (_totalRecs > 0);
}

TData::TDHandle TDataAscii::InitGetRecs(RecId lowId, RecId highId,
                                        Boolean asyncAllowed,
                                        ReleaseMemoryCallback *callback)
{
#if DEBUGLVL >= 3
  cout << " RecID lowID  = " << lowId << " highId " << highId << endl;
#endif

  DOASSERT((long)lowId < _totalRecs && (long)highId < _totalRecs
	   && highId >= lowId, "Invalid record parameters");

  TDataRequest *req = new TDataRequest;
  DOASSERT(req, "Out of memory");

  req->nextId = lowId;
  req->endId = highId;
  req->relcb = callback;

  /* Compute location and number of bytes to retrieve */
  streampos_t offset = _indexP->Get(req->nextId);
  iosize_t bytes = _indexP->Get(req->endId) + 1024 - offset;
  if ((long)req->endId < _totalRecs - 1) {
      /* Read up to the beginning of next record */
      bytes = _indexP->Get(req->endId + 1) - offset;
  }

  if (!asyncAllowed || !_data->SupportsAsyncIO()) {
#if DEBUGLVL >= 3
      //printf("Retrieving %llu:%lu bytes from TData 0x%p with direct I/O\n",
             offset, bytes, this);
#endif
      /* Zero handle indicates direct I/O */
      req->iohandle = 0;
  } else {
      /* Submit I/O request to the data source process */
      req->iohandle = _data->ReadProc(offset, bytes);
      DOASSERT(req->iohandle >= 0, "Cannot submit I/O request");
#if DEBUGLVL >= 3
      //printf("Retrieving %llu:%lu bytes from TData 0x%p with I/O handle %d\n",
             offset, bytes, this, req->iohandle);
#endif
      _currPos = offset;
  }

  req->lastChunk = req->lastOrigChunk = NULL;
  req->lastChunkBytes = 0;

  return req;
}

Boolean TDataAscii::GetRecs(TDHandle req, void *buf, int bufSize,
                            RecId &startRid, int &numRecs, int &dataSize)
{
  DOASSERT(req, "Invalid request handle");

#if DEBUGLVL >= 3
  //printf("TDataAscii::GetRecs: handle %d, buf = 0x%p\n", req->iohandle, buf);
#endif

  DOASSERT(req->iohandle >= 0, "I/O request not initialized properly");

  numRecs = bufSize / _recSize;
  DOASSERT(numRecs > 0, "Not enough record buffer space");

  if (req->nextId > req->endId)
    return false;
  
  int num = req->endId - req->nextId + 1;
  if (num < numRecs)
    numRecs = num;
  
  if (req->iohandle == 0)
    ReadRec(req->nextId, numRecs, buf);
  else
    ReadRecAsync(req, req->nextId, numRecs, buf);
  
  startRid = req->nextId;
  dataSize = numRecs * _recSize;
  req->nextId += numRecs;
  
  _bytesFetched += dataSize;
  
  if (req->nextId > req->endId)
    req->iohandle = -1;

  return true;
}

void TDataAscii::DoneGetRecs(TDHandle req)
{
  DOASSERT(req, "Invalid request handle");

  /*
     Release chunk of memory cached from pipe.
  */
  if (req->relcb && req->lastOrigChunk)
      req->relcb->ReleaseMemory(MemMgr::Buffer, req->lastOrigChunk, 1);

  if (req->iohandle > 0) {
    /*
       Flush data from pipe. We would also like to tell the DataSource
       (which is at the other end of the pipe) to stop, but we can't
       do that yet.
    */
    while (1) {
      char *chunk;
      streampos_t offset;
      iosize_t bytes;
      int status = _data->Consume(chunk, offset, bytes);
      DOASSERT(status >= 0, "Cannot consume data");
      if (bytes <= 0)
        break;
      /*
         Release chunk so buffer manager (or whoever gets the following
         call) can make use of it.
      */
      if (req->relcb)
        req->relcb->ReleaseMemory(MemMgr::Buffer, chunk, 1);
    }
  }

  delete req;
}

void TDataAscii::GetIndex(RecId id, int *&indices)
{
  static int index[1];
  index[0] = id;
  indices = index;
}

int TDataAscii::GetModTime()
{
  if (!CheckFileStatus())
    return -1;

  return _data->GetModTime();
}

char *TDataAscii::MakeIndexFileName(char *name, char *type)
{
  char *fname = StripPath(name);
  int nameLen = strlen(Init::WorkDir()) + 1 + strlen(fname) + 1;
  char *fn = new char [nameLen];
  sprintf(fn, "%s/%s", Init::WorkDir(), fname);
  return fn;
}

void TDataAscii::Initialize()
{
  _indexFileName = MakeIndexFileName(_name, _type);

  if (!CheckFileStatus())
    return;

  if (_data->isBuf()) {
      BuildIndex();
      return;
  }

  if (!_indexP->Initialize(_indexFileName, _data, this, _lastPos,
                           _totalRecs).IsComplete()) goto error;

  _initTotalRecs = _totalRecs;
  _initLastPos  = _lastPos;

  /* continue to build index */
  BuildIndex();
  return;

 error:
  /* recover from error by building index from scratch  */
  RebuildIndex();
}

void TDataAscii::Checkpoint()
{
  if (!CheckFileStatus()) {
    printf("Cannot checkpoint %s\n", _name);
    return;
  }

  if (_data->isBuf())
      return;

  //printf("Checkpointing %s: %ld total records, %ld new\n", _name,
//	 _totalRecs, _totalRecs - _initTotalRecs);
  
  if (_lastPos == _initLastPos && _totalRecs == _initTotalRecs)
    /* no need to checkpoint */
    return;
  
  if (!_indexP->Checkpoint(_indexFileName, _data, this, _lastPos,
    _totalRecs).IsComplete()) goto error;

  _currPos = _data->Tell();

  return;
  
 error:
  _currPos = _data->Tell();
}


void TDataAscii::InvalidateTData()
{
  if (_data->IsOk()) {
    RebuildIndex();
    TData::InvalidateTData();
  }
}


/* Build index for the file. This code should work when file size
   is extended dynamically. Before calling this function, position
   should be at the last place where file was scanned. */

void TDataAscii::BuildIndex()
{
  char buf[LINESIZE];
  char recBuf[_recSize];
  int oldTotal = _totalRecs;
  
  _currPos = _lastPos - _lastIncompleteLen;

  /* First go to last valid position of file */
  if (_data->Seek(_currPos, SEEK_SET) < 0) {
    reportErrSys("fseek");
    return;
  }

  _lastIncompleteLen = 0;

  while(1) {

    int len = 0;

    if (_data->Fgets(buf, LINESIZE) == NULL)
      break;

    len = strlen(buf);

    if (len > 0 && buf[len - 1] == '\n') {
      buf[len - 1] = 0;
      if (Decode(recBuf, _currPos, buf)) {
	_indexP->Set(_totalRecs++, _currPos);
      } else {
#if DEBUGLVL >= 7
	//printf("Ignoring invalid record: \"%s\"\n", buf);
#endif
      }
      _lastIncompleteLen = 0;
    } else {
#if DEBUGLVL >= 7
      //printf("Ignoring incomplete record: \"%s\"\n", buf);
#endif
      _lastIncompleteLen = len;
    }

    _currPos += len;
  }

  /*
     Last position is > current position because TapeDrive advances
     bufferOffset to the next block, past the EOF, when tape file ends.
  */
  _lastPos = _data->Tell();
  DOASSERT(_lastPos >= _currPos, "Incorrect file position");

#if DEBUGLVL >= 3
  //printf("Index for %s: %ld total records, %ld new\n", _name,
//	 _totalRecs, _totalRecs - oldTotal);
#endif

  if (_totalRecs <= 0)
      fprintf(stderr, "No valid records for data stream %s\n"
              "    (check schema/data correspondence)\n", _name);
}

/* Rebuild index */

void TDataAscii::RebuildIndex()
{
#if DEBUGLVL >= 3
 // printf("Rebuilding index...\n");
#endif

  InvalidateIndex();

  _indexP->Clear();
  _initTotalRecs = _totalRecs = 0;
  _initLastPos = _lastPos = 0;
  _lastIncompleteLen = 0;

  BuildIndex();
}

TD_Status TDataAscii::ReadRec(RecId id, int numRecs, void *buf)
{
#if DEBUGLVL >= 3
  //printf("TDataAscii::ReadRec %ld,%d,0x%p\n", id, numRecs, buf);
#endif

  char line[LINESIZE];
  
  char *ptr = (char *)buf;

  for(int i = 0; i < numRecs; i++) {

    int len;
    if (_currPos != (long) _indexP->Get(id + i)) {
      if (_data->Seek(_indexP->Get(id + i), SEEK_SET) < 0) {
        perror("fseek");
        DOASSERT(0, "Cannot perform file seek");
      }
      _currPos = _indexP->Get(id + i);
    }
    if (_data->Fgets(line, LINESIZE) == NULL) {
      reportErrSys("fgets");
      DOASSERT(0, "Cannot read from file");
    }
    len = strlen(line);

    if (len > 0 ) {
      DOASSERT(line[len - 1] == '\n', "Data record too long");
      line[len - 1] = '\0';
    }

    Boolean valid = Decode(ptr, _currPos, line);
    DOASSERT(valid, "Inconsistent validity flag");
    ptr += _recSize;

    _currPos += len;
  }

  return TD_OK;
}

TD_Status TDataAscii::ReadRecAsync(TDataRequest *req, RecId id,
                                   int numRecs, void *buf)
{
#if DEBUGLVL >= 3
  //printf("TDataAscii::ReadRecAsync %ld,%d,0x%p\n", id, numRecs, buf);
#endif

  int recs = 0;
  char *ptr = (char *)buf;
  char line[LINESIZE];
  int partialRecSize = 0;

  while (recs < numRecs) {
    /* Take chunk from cached values if present */
    char *chunk = req->lastChunk;
    char *origChunk = req->lastOrigChunk;
    iosize_t bytes = req->lastChunkBytes;

    /* No chunk in cache, get next chunk from data source */
    if (!chunk) {
        streampos_t offset;
        int status = _data->Consume(chunk, offset, bytes);
        DOASSERT(status >= 0, "Cannot consume data");
        DOASSERT((off_t)offset == _currPos, "Invalid data chunk consumed");
        _currPos += bytes;
        origChunk = chunk;
    } else {
        req->lastChunk = req->lastOrigChunk = NULL;
        req->lastChunkBytes = 0;
    }

    DOASSERT(chunk && origChunk, "Inconsistent state");

    if (bytes <= 0)
        break;

    while (recs < numRecs && bytes > 0) {
      char *eol = (char *)memchr(chunk, '\n', bytes);
      if (!eol) {
        DOASSERT(partialRecSize == 0, "Two consecutive partial records");
        /* Store fraction of record for next loop */
        memcpy(line, chunk, bytes);
        line[bytes] = 0;
        partialRecSize = bytes;
#if DEBUGLVL >= 3
        //printf("Caching remainder of chunk (%d bytes): \"%s\"\n",
         //      partialRecSize, line);
#endif
        break;
      }

      /*
         Append record to existing record from previous iteration of
         outer loop if there is a fragment of it left. Terminating
         newline is first replaced with a null, then appended, and
         then the newline is put back.
      */

      char *record = chunk;
      int recSize = eol - record;
      int fullRecSize = recSize;

      char oldch = *eol;
      *eol = 0;

      if (partialRecSize > 0) {
          fullRecSize += partialRecSize;
          memcpy(&line[partialRecSize], record, recSize + 1);
#if DEBUGLVL >= 5
          //printf("Got %d-byte record (%d partial): \"%s\"\n", fullRecSize,
                 partialRecSize, line);
#endif
          partialRecSize = 0;
          record = line;
      } else {
#if DEBUGLVL >= 5
          //printf("Got %d-byte full record: \"%s\"\n", fullRecSize, record);
#endif
      }

      Boolean valid = Decode(ptr, _currPos, record);
      if (valid) {
        ptr += _recSize;
        recs++;
      }

      *eol = oldch;
      chunk = eol + 1;
      bytes -= recSize + 1;
    }

    if (recs == numRecs && bytes > 0) {
      /* Save unused piece of chunk for next call to this function */
      req->lastChunk = chunk;
      req->lastOrigChunk = origChunk;
      req->lastChunkBytes = bytes;
#if DEBUGLVL >= 3
      //printf("Saving %ld bytes of chunk 0x%p for next function call\n",
     //        bytes, origChunk);
#endif
    } else {
      /*
         Release chunk so buffer manager (or whoever gets the following
         call) can make use of it.
      */
      if (req->relcb)
          req->relcb->ReleaseMemory(MemMgr::Buffer, origChunk, 1);
    }
  }

  if (recs != numRecs)
    fprintf(stderr, "Data source produced %d records, not %d\n",
            recs, numRecs);
  DOASSERT(recs == numRecs, "Incomplete data transfer");

  
  return TD_OK;
}

void TDataAscii::WriteRecs(RecId startRid, int numRecs, void *buf)
{
  DOASSERT(!_data->isTape(), "Writing to tape not supported yet");

  _totalRecs += numRecs;

  _indexP->Set(_totalRecs - 1, _lastPos);
  int len = strlen((char *)buf);

  if (_data->append(buf, len) != len) {
    reportErrSys("append");
    DOASSERT(0, "Cannot append to file");
  }

  _lastPos = _data->Tell();
  _currPos = _lastPos;
}

void TDataAscii::WriteLine(void *line)
{
  DOASSERT(!_data->isTape(), "Writing to tape not supported yet");

  int len = strlen((char *)line);

  if (_data->append(line, len) != len) {
    reportErrSys("append");
    DOASSERT(0, "Cannot append to file");
  }

  _lastPos = _data->Tell();
  _currPos = _lastPos;
}

void TDataAscii::Cleanup()
{
  Checkpoint();

  if (_data->isTape())
    _data->printStats();
}

void TDataAscii::PrintIndices()
{
  int cnt = 0;
/*  for(long i = 0; i < _totalRecs; i++) {
    printf("%ld ", _indexP->Get(i));
    if (cnt++ == 10) {
      printf("\n");
      cnt = 0;
    }
  }
  printf("\n");
*/
}
