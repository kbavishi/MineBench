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
  Implementation of DataSourceTape class.
 */

/*
  $Id: DataSourceTape.c 2396 2014-03-13 21:13:27Z wkliao $

  $Log: DataSourceTape.c,v $
  Revision 1.6  1996/11/23 21:07:21  jussi
  Fixed interface to TapeDrive object.

  Revision 1.5  1996/10/07 22:53:58  wenger
  Added more error checking and better error messages in response to
  some of the problems uncovered by CS 737 students.

  Revision 1.4  1996/07/01 19:31:34  jussi
  Added an asynchronous I/O interface to the data source classes.
  Added a third parameter (char *param) to data sources because
  the DataSegment template requires that all data sources have the
  same constructor (DataSourceWeb requires the third parameter).

  Revision 1.3  1996/06/27 15:50:59  jussi
  Added IsOk() method which is used by TDataAscii and TDataBinary
  to determine if a file is still accessible. Also moved GetModTime()
  functionality from TDataAscii/TDataBinary to the DataSource
  classes.

  Revision 1.2  1996/06/04 19:58:46  wenger
  Added the data segment option to TDataBinary; various minor cleanups.

  Revision 1.1  1996/05/22 17:52:06  wenger
  Extended DataSource subclasses to handle tape data; changed TDataAscii
  and TDataBinary classes to use new DataSource subclasses to hide the
  differences between tape and disk files.

 */

#define _DataSourceTape_c_

//#define DEBUG

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/param.h>
#include <unistd.h>

#include "DataSourceTape.h"
#include "Util.h"
#include "DevError.h"
#include "tapedrive.h"


#if !defined(lint) && defined(RCSID)
static char		rcsid[] = "$RCSfile: DataSourceTape.c,v $ $Revision: 2396 $ $State: Exp $";
#endif

static char *	srcFile = __FILE__;

/*------------------------------------------------------------------------------
 * function: DataSourceTape::DataSourceTape
 * DataSourceTape constructor.
 */
DataSourceTape::DataSourceTape(char *filename, char *label, char *param) :
     DataSource(label)
{
	DO_DEBUG(printf("DataSourceTape::DataSourceTape(%s, %s)\n", filename,
		label));

	_filename = strdup(filename);
	_tapeP = NULL;
}

/*------------------------------------------------------------------------------
 * function: DataSourceTape::~DataSourceTape
 * DataSourceTape destructor.
 */
DataSourceTape::~DataSourceTape()
{
	DO_DEBUG(printf("DataSourceTape::~DataSourceTape()\n"));

	delete [] _filename;
	if (_tapeP != NULL) delete _tapeP;
}

/*------------------------------------------------------------------------------
 * function: DataSourceTape::Open
 * Create a TapeDrive object corresponding to the file named in this object.
 */
DevStatus
DataSourceTape::Open(char *mode)
{
	DO_DEBUG(printf("DataSourceTape::Open()\n"));

	DevStatus	result = StatusOk;

	_tapeP = new TapeDrive(_filename, mode);
	if (_tapeP == NULL)
	{
		reportError("Error instatiating TapeDrive object", devNoSyserr);
		result = StatusFailed;
	}

	return result;
}

/*------------------------------------------------------------------------------
 * function: DataSourceTape::IsOk
 * Return true if tape file is open and in good status.
 */
Boolean
DataSourceTape::IsOk()
{
  if (!_tapeP) return false;
  if (Tell() < 0) return false;
  return true;
}

/*------------------------------------------------------------------------------
 * function: DataSourceTape::Close
 * Delete the TapeDrive object associated with this object.
 */
DevStatus
DataSourceTape::Close()
{
	DO_DEBUG(printf("DataSourceTape::Close()\n"));

	if (_tapeP != NULL) delete _tapeP;
        _tapeP = NULL;

	return StatusOk;
}

/*------------------------------------------------------------------------------
 * function: DataSourceTape::Fgets
 * Simulate an fgets() on the TapeDrive object associated with this object.
 */
char *
DataSourceTape::Fgets(char *buffer, int size)
{
	DO_DEBUG(printf("DataSourceTape::Fgets()\n"));

	char *result = buffer;

        int bytes = _tapeP->gets(buffer, size);
        if (bytes < 0) {
	  reportErrSys("Tape::gets()");
	  result = NULL;
	} else if (!bytes)
          result = NULL;
        else if (bytes < size)
          *(result + bytes) = 0;

	return result;
}

/*------------------------------------------------------------------------------
 * function: DataSourceTape::Fread
 * Simulate an fread() on the TapeDrive object associated with this object.
 */
size_t
DataSourceTape::Fread(char *buf, size_t size, size_t itemCount)
{
	DO_DEBUG(printf("DataSourceTape::Fread()\n"));

	return _tapeP->read((void *) buf, size * itemCount) / size;
}

/*------------------------------------------------------------------------------
 * function: DataSourceTape::Read
 * Simulate a read() on the TapeDrive object associated with this object.
 */
size_t
DataSourceTape::Read(char *buf, int byteCount)
{
	DO_DEBUG(printf("DataSourceTape::Read()\n"));

	return _tapeP->read((void *) buf, byteCount);
}

/*------------------------------------------------------------------------------
 * function: DataSourceTape::Seek
 * Simulate a seek() on the TapeDrive object associated with this object.
 */
int
DataSourceTape::Seek(long offset, int from)
{
	DO_DEBUG(printf("DataSourceTape::Seek()\n"));
	int		result = 0;

	if (from != SEEK_SET)
	{
		reportError(
			"'Seek from' value must be SEEK_SET for DataSourceTape object",
			devNoSyserr);
		result = -1;
	}
	else
	{
		result = _tapeP->seek(offset);
	}

	return result;
}

/*------------------------------------------------------------------------------
 * function: DataSourceTape::Tell
 * Simulate a tell() on the TapeDrive object associated with this object.
 */
long
DataSourceTape::Tell()
{
	DO_DEBUG(printf("DataSourceTape::Tell()\n"));

	return _tapeP->tell();
}

/*------------------------------------------------------------------------------
 * function: DataSourceTape::gotoEnd
 * Go to the end of the tape file associated with this object, and return the
 * offset of the end of the file.
 */
int
DataSourceTape::gotoEnd()
{
	//DO_DEBUG(printf("DataSourceTape::gotoEnd()\n"));
	int		result = 0;

	long end = 1024 * 1024 * 1024;
	if (_tapeP->seek(end) >= end)
	{
		reportError("Could not seek to end of tape file", devNoSyserr);
		result = -1;
	}
	else
	{
		result = _tapeP->tell();
	}

	return result;
}

/*------------------------------------------------------------------------------
 * function: DataSourceTape::append
 * Append the given record to the end of the tape file associated with this
 * object.
 */
int
DataSourceTape::append(void *buf, int recSize)
{
	DO_DEBUG(printf("DataSourceTape::append()\n"));

	DOASSERT(0, "DataSourceTape::append not yet implemented");

	return _tapeP->append(buf, recSize);
}

/*------------------------------------------------------------------------------
 * function: DataSourceFileStream::GetModTime
 * Returns the last modification time of the object.
 */
int
DataSourceTape::GetModTime()
{
    DO_DEBUG(printf("DataSourceTape::GetModTime()\n"));

    reportError("Cannot get modification time for tape data", devNoSyserr);
    return -1;
}

/*------------------------------------------------------------------------------
 * function: DataSourceTape::printStats
 * Print usage statistics.
 */
void
DataSourceTape::printStats()
{
	DO_DEBUG(printf("DataSourceTape::printStats()\n"));

	_tapeP->printStats();

	return;
}

/*============================================================================*/
