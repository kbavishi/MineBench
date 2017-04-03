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
  $Id: DataSourceFixedBuf.h 2396 2014-03-13 21:13:27Z wkliao $

  $Log: DataSourceFixedBuf.h,v $
  Revision 1.1  1996/08/04 21:23:24  beyer
  DataSource's are now reference counted.
  Added Version() which TData now check to see if the DataSource has changed,
    and if it has, it rebuilds its index and invalidates the cache.
  DataSourceFixedBuf is a DataSourceBuf that allocates and destoyes its
    own buffer.
  DerivedDataSource functionality is now in the TData constructor.
  Added some defaults for virtual methods.

*/

#ifndef _DATASOURCEFIXEDBUF_H_
#define _DATASOURCEFIXEDBUF_H_

#include "DataSourceBuf.h"


class DataSourceFixedBuf
: public DataSourceBuf
{
  public:

    DataSourceFixedBuf(int buffer_size, char* label);

    ~DataSourceFixedBuf();

  protected:

  private:

    // N/A
    DataSourceFixedBuf(const DataSourceFixedBuf& other);
    DataSourceFixedBuf& operator=(const DataSourceFixedBuf& other);
};



inline
DataSourceFixedBuf::DataSourceFixedBuf(int buffer_size, char* label)
: DataSourceBuf(new char[buffer_size], buffer_size, 0, label)
{
}


inline
DataSourceFixedBuf::~DataSourceFixedBuf()
{
    delete [] _sourceBuf;
}


#endif // _DATASOURCEFIXEDBUF_H_
