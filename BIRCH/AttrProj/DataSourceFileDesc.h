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
  Header file for DataSourceFileDesc class.
 */

/*
  $Id: DataSourceFileDesc.h 2396 2014-03-13 21:13:27Z wkliao $

  $Log: DataSourceFileDesc.h,v $
  Revision 1.1  1996/07/01 19:21:26  jussi
  Initial revision.
*/

#ifndef _DataSourceFileDesc_h_
#define _DataSourceFileDesc_h_

#include "DataSourceFileStream.h"

class DataSourceFileDesc : public DataSourceFileStream
{
public:
    DataSourceFileDesc(int fd, char *label);
    virtual ~DataSourceFileDesc();

    virtual char *objectType() {return "DataSourceFileDesc";};

    virtual DevStatus Open(char *mode);
    virtual Boolean IsOk();
    virtual DevStatus Close();

protected:
    int _fd;
};


#endif /* _DataSourceFileDesc_h_ */

/*============================================================================*/
