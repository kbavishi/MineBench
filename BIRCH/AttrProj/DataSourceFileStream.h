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
  Header file for DataSourceFileStream class.
 */

/*
  $Id: DataSourceFileStream.h 2396 2014-03-13 21:13:27Z wkliao $

  $Log: DataSourceFileStream.h,v $
  Revision 1.6  1996/12/03 20:36:07  jussi
  Added support for concurrent I/O.

  Revision 1.5  1996/11/18 22:29:02  jussi
  Added DataSize() method.

  Revision 1.4  1996/08/04 21:23:24  beyer
  DataSource's are now reference counted.
  Added Version() which TData now check to see if the DataSource has changed,
    and if it has, it rebuilds its index and invalidates the cache.
  DataSourceFixedBuf is a DataSourceBuf that allocates and destoyes its
    own buffer.
  DerivedDataSource functionality is now in the TData constructor.
  Added some defaults for virtual methods.

  Revision 1.3  1996/07/01 19:31:33  jussi
  Added an asynchronous I/O interface to the data source classes.
  Added a third parameter (char *param) to data sources because
  the DataSegment template requires that all data sources have the
  same constructor (DataSourceWeb requires the third parameter).

  Revision 1.2  1996/06/27 15:50:59  jussi
  Added IsOk() method which is used by TDataAscii and TDataBinary
  to determine if a file is still accessible. Also moved GetModTime()
  functionality from TDataAscii/TDataBinary to the DataSource
  classes.

  Revision 1.1  1996/05/22 17:52:04  wenger
  Extended DataSource subclasses to handle tape data; changed TDataAscii
  and TDataBinary classes to use new DataSource subclasses to hide the
  differences between tape and disk files.

 */

#ifndef _DataSourceFileStream_h_
#define _DataSourceFileStream_h_


#include "DataSource.h"


class DataSourceFileStream : public DataSource
{
public:

	DataSourceFileStream(char *filename, char *label);

	virtual ~DataSourceFileStream();

	virtual char *objectType() {return "DataSourceFileStream";};

	virtual DevStatus Open(char *mode);
	virtual Boolean IsOk();
	virtual DevStatus Close();

	// WARNING: don't mix the F* functions with the plain ones
	// eg, Fread (or Fgets) with Read.  The F* functions use 
	// stdio routines which are buffered while the other ones don't.
	virtual char *Fgets(char *buffer, int size);
	virtual size_t Fread(char *buf, size_t size, size_t itemCount);
	virtual size_t Read(char *buf, int byteCount);

	virtual size_t Fwrite(const char *buf, size_t size, size_t itemCount);
	virtual size_t Write(const char *buf, size_t byteCount);

	virtual int Seek(long offset, int from);
	virtual long Tell();

	virtual int gotoEnd();

	virtual int append(void *buf, int recSize);

	virtual Boolean isFile() { return true; }

	char* GetName() { return _filename; }

	int GetModTime();

        virtual bytecount_t DataSize();

      protected:

	DataSourceFileStream() : DataSource() {
	    _filename = NULL;
	    _file = NULL;
	}

	char *		_filename;
	FILE *		_file;
};


#endif /* _DataSourceFileStream_h_ */

/*============================================================================*/
