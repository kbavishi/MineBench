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
  Header file for DataSourceTape class.
 */

/*
  $Id: DataSourceTape.h 2396 2014-03-13 21:13:27Z wkliao $

  $Log: DataSourceTape.h,v $
  Revision 1.3  1996/07/01 19:31:35  jussi
  Added an asynchronous I/O interface to the data source classes.
  Added a third parameter (char *param) to data sources because
  the DataSegment template requires that all data sources have the
  same constructor (DataSourceWeb requires the third parameter).

  Revision 1.2  1996/06/27 15:51:00  jussi
  Added IsOk() method which is used by TDataAscii and TDataBinary
  to determine if a file is still accessible. Also moved GetModTime()
  functionality from TDataAscii/TDataBinary to the DataSource
  classes.

  Revision 1.1  1996/05/22 17:52:07  wenger
  Extended DataSource subclasses to handle tape data; changed TDataAscii
  and TDataBinary classes to use new DataSource subclasses to hide the
  differences between tape and disk files.

 */

#ifndef _DataSourceTape_h_
#define _DataSourceTape_h_


#include "DataSource.h"


class TapeDrive;

class DataSourceTape : public DataSource
{
public:
	DataSourceTape(char *name, char *label, char *param = 0);
	virtual ~DataSourceTape();

	virtual char *objectType() {return "DataSourceTape";};

	virtual DevStatus Open(char *mode);
	virtual Boolean IsOk();
	virtual DevStatus Close();

	virtual char *Fgets(char *buffer, int size);
	virtual size_t Fread(char *buf, size_t size, size_t itemCount);
	virtual size_t Read(char *buf, int byteCount);

	virtual int Seek(long offset, int from);
	virtual long Tell();

	virtual int gotoEnd();

	virtual int append(void *buf, int recSize);

	virtual int GetModTime();

	virtual void printStats();

	virtual Boolean isFile() {return false;};

	virtual Boolean isBuf() {return false;};

	virtual Boolean isTape() {return true;};

private:
	char *		_filename;
	TapeDrive *	_tapeP;
};


#endif /* _DataSourceTape_h_ */

/*============================================================================*/
