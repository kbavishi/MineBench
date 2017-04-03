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
  Header file for DataSourceSegment classes (template).
 */

/*
  $Id: DataSourceSegment.h 2396 2014-03-13 21:13:27Z wkliao $

  $Log: DataSourceSegment.h,v $
  Revision 1.7  1996/12/03 20:36:28  jussi
  Removed unnecessary AsyncFd() ans AsyncIO().

  Revision 1.6  1996/08/06 19:23:03  beyer
  Made functional again after the last changes

  Revision 1.5  1996/08/04 21:23:25  beyer
  DataSource's are now reference counted.
  Added Version() which TData now check to see if the DataSource has changed,
    and if it has, it rebuilds its index and invalidates the cache.
  DataSourceFixedBuf is a DataSourceBuf that allocates and destoyes its
    own buffer.
  DerivedDataSource functionality is now in the TData constructor.
  Added some defaults for virtual methods.

  Revision 1.4  1996/07/11 17:25:33  wenger
  Devise now writes headers to some of the files it writes;
  DataSourceSegment class allows non-fixed data length with non-zero
  offset; GUI for editing schema files can deal with comment lines;
  added targets to top-level makefiles to allow more flexibility.

  Revision 1.3  1996/07/01 19:31:34  jussi
  Added an asynchronous I/O interface to the data source classes.
  Added a third parameter (char *param) to data sources because
  the DataSegment template requires that all data sources have the
  same constructor (DataSourceWeb requires the third parameter).

  Revision 1.2  1996/06/27 15:50:59  jussi
  Added IsOk() method which is used by TDataAscii and TDataBinary
  to determine if a file is still accessible. Also moved GetModTime()
  functionality from TDataAscii/TDataBinary to the DataSource
  classes.

  Revision 1.1  1996/06/04 14:21:41  wenger
  Ascii data can now be read from session files (or other files
  where the data is only part of the file); added some assertions
  to check for pointer alignment in functions that rely on this;
  Makefile changes to make compiling with debugging easier.

 */

#ifndef _DataSourceSegment_
#define _DataSourceSegment_


#ifdef __GNUG__
#pragma interface
#endif


#include "DeviseTypes.h"

#define DATA_LENGTH_UNDEFINED (-1)


class DataSourceSegment : public DataSource
{
public:
    DataSourceSegment(DataSource* dataSource, 
		      long dataOffset, long dataLength);
    ~DataSourceSegment();

    virtual char *objectType();

    virtual DevStatus Open(char *mode);
    virtual char IsOk();
    virtual DevStatus Close();

    virtual char *Fgets(char *buffer, int size);
    virtual size_t Fread(char *buf, size_t size, size_t itemCount);
    virtual size_t Read(char *buf, int byteCount);
    
    virtual size_t Fwrite(const char *buf, size_t size, size_t itemCount);
    virtual size_t Write(const char *buf, size_t byteCount);

    virtual int Seek(long offset, int from);
    virtual long Tell();

    virtual int gotoEnd();

    virtual int append(void *buf, int recSize);

    virtual int GetModTime();

    // virtual void printStats();

    virtual Boolean isFile();
    virtual Boolean isBuf();
    virtual Boolean isTape();
    

protected:

    DataSource* _dataSource;
    long _dataOffset;
    long _dataLength;
};


#endif /* _DataSourceSegment_ */

/*============================================================================*/
