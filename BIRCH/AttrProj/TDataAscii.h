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
  $Id: TDataAscii.h 2396 2014-03-13 21:13:27Z wkliao $

  $Log: TDataAscii.h,v $
  Revision 1.20  1996/12/03 20:32:58  jussi
  Updated to reflect new TData interface. Added support for concurrent I/O.

  Revision 1.19  1996/11/23 21:14:23  jussi
  Removed failing support for variable-sized records.

  Revision 1.18  1996/11/22 20:41:09  flisakow
  Made variants of the TDataAscii classes for sequential access,
  which build no indexes.

  ReadRec() method now returns a status instead of void for every
  class that has the method.

  Revision 1.17  1996/10/04 17:24:16  wenger
  Moved handling of indices from TDataAscii and TDataBinary to new
  FileIndex class.

  Revision 1.16  1996/08/04 21:59:53  beyer
  Added UpdateLinks that allow one view to be told to update by another view.
  Changed TData so that all TData's have a DataSource (for UpdateLinks).
  Changed all of the subclasses of TData to conform.
  A RecFile is now a DataSource.
  Changed the stats buffers in ViewGraph to be DataSources.

  Revision 1.15  1996/07/12 19:39:43  jussi
  Removed _file member variable as it's not needed.

  Revision 1.14  1996/07/03 23:13:51  jussi
  Added call to _data->Close() in destructor. Renamed
  _fileOkay to _fileOpen which is more accurate.

  Revision 1.13  1996/07/01 19:28:06  jussi
  Added support for typed data sources (WWW and UNIXFILE). Renamed
  'cache' references to 'index' (cache file is really an index).
  Added support for asynchronous interface to data sources.

  Revision 1.12  1996/06/27 18:12:39  wenger
  Re-integrated most of the attribute projection code (most importantly,
  all of the TData code) into the main code base (reduced the number of
  modules used only in attribute projection).

  Revision 1.11  1996/06/27 15:49:32  jussi
  TDataAscii and TDataBinary now recognize when a file has been deleted,
  shrunk, or has increased in size. The query processor is asked to
  re-issue relevant queries when such events occur.

  Revision 1.10  1996/06/12 14:56:33  wenger
  Added GUI and some code for saving data to templates; added preliminary
  graphical display of TDatas; you now have the option of closing a session
  in template mode without merging the template into the main data catalog;
  removed some unnecessary interdependencies among include files; updated
  the dependencies for Sun, Solaris, and HP; removed never-accessed code in
  ParseAPI.C.

  Revision 1.9  1996/05/22 17:52:15  wenger
  Extended DataSource subclasses to handle tape data; changed TDataAscii
  and TDataBinary classes to use new DataSource subclasses to hide the
  differences between tape and disk files.

  Revision 1.8  1996/05/07 16:43:01  jussi
  Cache file name now based on file alias (TData name). Added parameter
  to the Decode() function call.

  Revision 1.7  1996/03/26 21:18:44  jussi
  Merged with TDataTape. Added magic number to cache file.

  Revision 1.6  1996/01/25 20:22:58  jussi
  Improved support for data files that grow while visualization
  is being performed.

  Revision 1.5  1995/12/28 19:59:35  jussi
  Small fixes to remove compiler warnings.

  Revision 1.4  1995/11/24 21:35:24  jussi
  Added _currPos member.

  Revision 1.3  1995/11/22 17:52:02  jussi
  Added copyright notice and cleaned up the code. Added some
  optimizations a la TDataTape.h.

  Revision 1.2  1995/09/05 22:15:50  jussi
  Added CVS header.
*/

/* Textual data virtual base class */

#ifndef TDataAscii_h
#define TDataAscii_h

#include <stdio.h>
#include <sys/types.h>

#include "DeviseTypes.h"
#ifdef ATTRPROJ
#   include "ApDispatcher.h"
#else
#   include "Dispatcher.h"
#endif
#include "TData.h"
#include "RecId.h"
#include "DataSource.h"
#include "FileIndex.h"

class TDataAscii: public TData, private DispatcherCallback {
public:
	TDataAscii(char *name, char *type, char *param, int recSize);

	virtual ~TDataAscii();

	/**** MetaData about TDataAscii ****/

	// Get list of attributes
	virtual AttrList *GetAttrList() { return NULL; }

	// Return # of dimensions and the size of each dimension,
	// or -1 if unknown
	virtual int Dimensions(int *sizeDimension);

	// Return true if TDataTape appends records
	virtual Boolean HasAppend() { return true; }

	/* Convert RecId into index */
	virtual void GetIndex(RecId id, int *&indices);

	/**** Getting record Id's ****/

	/* Get RecId of 1st available record, return true if available */
	virtual Boolean HeadID(RecId &recId);

	/* Get RecId of last record, return true if available */
	virtual Boolean LastID(RecId &recId);

	/**** Getting Records ****/

	/**************************************************************
	Init getting records.
	***************************************************************/
	virtual TDHandle InitGetRecs(RecId lowId, RecId highId,
                                     Boolean asyncAllowed,
                                     ReleaseMemoryCallback *callback);

	/**************************************************************
	Get next batch of records, as much as fits into buffer. 
	Return false if no more.
	input:
		buf: where to start putting records.
		bufSize: size of buffer being supplied.
	output:
		startRid : starting record ID of this batch 
		numRecs: number of records.
		dataSize: # of bytes taken up by data.
		recPtrs: pointer to records for variable size records.
	**************************************************************/
	virtual Boolean GetRecs(TDHandle handle, void *buf, int bufSize,
                                RecId &startRid, int &numRecs, int &dataSize);

	virtual void DoneGetRecs(TDHandle handle);

	/* get the time file is modified. We only require that
	files modified later has time > files modified earlier. */
	virtual int GetModTime();

	/* Do a checkpoint */
	virtual void Checkpoint();

	// rebuild index, et. al
	virtual void InvalidateTData();

	/* writing a record. For TDataAscii, the new record
	is appended to the file (startRid not examined), numRecs ==1, 
	and buf points to a string to be written to disk. */
	virtual void WriteRecs(RecId startId, int numRecs, void *buf);

	/* Write a line into the file, but don't make it into a record */
	void WriteLine(void *line);

protected:
	/* For derived class */
	/* should be called by the constructors of derived classes */
	void Initialize();

	/* Decode a record and put data into buffer. Return false if
	this line is not valid. */
	virtual Boolean Decode(void *recordBuf, int recPos, char *line) = 0;

	/* Read/Write specific to each subclass index. The cached information
           is to be read during file start up, and written when file closes,
           so as to get the TData back to the state at the last
           file close. Return false and the index will be rebuilt
           from scratch every time. Return true and the base class
           will reuse the index it has cached. */
	virtual Boolean WriteIndex(int fd) { return false; }
	virtual Boolean ReadIndex(int fd) { return false; }

        /* This function is called by this class to ask the derived
           class to invalidate all indexed information */
        virtual void InvalidateIndex() {}

	static char *MakeIndexFileName(char *name, char *type);

private:
	/* From DispatcherCallback */
	char *DispatchedName() { return "TDataAscii"; }
	virtual void Cleanup();

	Boolean CheckFileStatus();

	/* Build or rebuild index */
	void BuildIndex();
	void RebuildIndex();

	TD_Status ReadRec(RecId id, int numRecs, void *buf);
	TD_Status ReadRecAsync(TDataRequest *req, RecId id,
                               int numRecs, void *buf);

	/* Print indices */
	void PrintIndices();

	char *_indexFileName;           // name of index file
	FileIndex *_indexP;

	long _totalRecs;                // total number of records
	long _lastPos;                  // position of last record in file
	long _currPos;                  // current file position
        long _lastIncompleteLen;        // length of last incomplete record

        Boolean _fileOpen;              // true if file is okay

	long _initTotalRecs;            // initial # of records in cache
	int _initLastPos;               // initial last position in file

	int _bytesFetched;              // total # of bytes fetched
};

#endif
