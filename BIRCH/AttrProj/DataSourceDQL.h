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
  Header file for DataSourceDQL class.
 */

/*
  $Id: DataSourceDQL.h 2396 2014-03-13 21:13:27Z wkliao $

  $Log: DataSourceDQL.h,v $
  Revision 1.2  1996/11/13 16:49:14  wenger
  Fixed error in Seek declaration.

  Revision 1.1  1996/11/01 19:28:18  kmurli
  Added DQL sources to include access to TDataDQL. This is equivalent to
  TDataAscii/TDataBinary. The DQL type in the Tcl/Tk corresponds to this
  class.

  Revision 1.3  1996/07/14 20:34:16  jussi
  Rewrote class to fork a process that does all data transfers
  from the DQL site.

  Revision 1.2  1996/07/12 19:39:02  jussi
  DQL data source uses Timer services.

  Revision 1.1  1996/07/01 19:21:25  jussi
  Initial revision.
*/

#ifndef _DataSourceDQL_h_
#define _DataSourceDQL_h_

#include <sys/types.h>

#include "DataSourceFileStream.h"

class DataSourceDQL : public DataSource
{
public:
    DataSourceDQL(char *url, char *label);
    virtual ~DataSourceDQL();

    virtual char *objectType() { return "DataSourceDQL"; }
    
    virtual DevStatus Open(char *mode);
    virtual DevStatus Close();

    virtual size_t Fwrite(const char *buf, size_t size, size_t itemCount);
    virtual size_t Write(const char *buf, size_t byteCount);

    virtual int append(void *buf, int recSize);

	virtual Boolean IsOk(){
		return false;
	}
	virtual int Seek(long offset, int from){
		return 0;
	}
	virtual long Tell(){
		return 0;
	}
	virtual int gotoEnd(){
		return 0;	
	}
    
protected:
    virtual DevStatus ChildProc();

    char * _query;              // URL of data source
    char * _name;              // URL of data source
};

#endif /* _DataSourceDQL_h_ */

/*============================================================================*/
