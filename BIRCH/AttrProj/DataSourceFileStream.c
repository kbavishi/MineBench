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
  Implementation of DataSourceFileStream class.
 */

/*
  $Id: DataSourceFileStream.c 2396 2014-03-13 21:13:27Z wkliao $

  $Log: DataSourceFileStream.c,v $
  Revision 1.9  1996/12/03 20:36:07  jussi
  Added support for concurrent I/O.

  Revision 1.8  1996/11/18 22:29:01  jussi
  Added DataSize() method.

  Revision 1.7  1996/10/07 22:53:57  wenger
  Added more error checking and better error messages in response to
  some of the problems uncovered by CS 737 students.

  Revision 1.6  1996/08/04 21:23:23  beyer
  DataSource's are now reference counted.
  Added Version() which TData now check to see if the DataSource has changed,
    and if it has, it rebuilds its index and invalidates the cache.
  DataSourceFixedBuf is a DataSourceBuf that allocates and destoyes its
    own buffer.
  DerivedDataSource functionality is now in the TData constructor.
  Added some defaults for virtual methods.

  Revision 1.5  1996/07/01 19:31:33  jussi
  Added an asynchronous I/O interface to the data source classes.
  Added a third parameter (char *param) to data sources because
  the DataSegment template requires that all data sources have the
  same constructor (DataSourceWeb requires the third parameter).

  Revision 1.4  1996/06/27 15:50:58  jussi
  Added IsOk() method which is used by TDataAscii and TDataBinary
  to determine if a file is still accessible. Also moved GetModTime()
  functionality from TDataAscii/TDataBinary to the DataSource
  classes.

  Revision 1.3  1996/06/04 19:58:44  wenger
  Added the data segment option to TDataBinary; various minor cleanups.

  Revision 1.2  1996/06/04 14:20:23  wenger
  Ascii data can now be read from session files (or other files
  where the data is only part of the file); added some assertions
  to check for pointer alignment in functions that rely on this;
  Makefile changes to make compiling with debugging easier.

  Revision 1.1  1996/05/22 17:52:03  wenger
  Extended DataSource subclasses to handle tape data; changed TDataAscii
  and TDataBinary classes to use new DataSource subclasses to hide the
  differences between tape and disk files.

 */

#define _DataSourceFileStream_c_

//#define DEBUG

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "DataSourceFileStream.h"
#include "Util.h"
#include "DevError.h"


#if !defined(lint) && defined(RCSID)
static char		rcsid[] = "$RCSfile: DataSourceFileStream.c,v $ $Revision: 2396 $ $State: Exp $";
#endif

static char *	srcFile = __FILE__;

/*------------------------------------------------------------------------------
 * function: DataSourceFileStream::DataSourceFileStream
 * DataSourceFileStream constructor.
 */
DataSourceFileStream::DataSourceFileStream(char *filename, char *label)
: DataSource(label)
{
	DO_DEBUG(printf("DataSourceFileStream::DataSourceFileStream(%s, %s)\n",
		filename, (label != NULL) ? label : "null"));

	_filename = strdup(filename);
	_file = NULL;
}

/*------------------------------------------------------------------------------
 * function: DataSourceFileStream::~DataSourceFileStream
 * DataSourceFileStream destructor.
 */
DataSourceFileStream::~DataSourceFileStream()
{
	DO_DEBUG(printf("DataSourceFileStream::~DataSourceFileStream()\n"));

	delete [] _filename;
	if (_file != NULL) fclose(_file);
}

/*------------------------------------------------------------------------------
 * function: DataSourceFileStream::Open
 * Do an fopen() on the file corresponding to the DataSourceFileStream object.
 */
DevStatus
DataSourceFileStream::Open(char *mode)
{
	DO_DEBUG(printf("DataSourceFileStream::Open()\n"));

	DevStatus	result = StatusOk;

	_file = fopen(_filename, mode);
	if (_file == NULL)
	{
		char	errBuf[MAXPATHLEN+100];
		sprintf(errBuf, "unable to open file %s", _filename);
		reportError(errBuf, errno);
		result = StatusFailed;
	}

	return result;
}

/*------------------------------------------------------------------------------
 * function: DataSourceFileStream::IsOk
 * Return true if file is open and in good status.
 */
Boolean
DataSourceFileStream::IsOk()
{
    if (!_file)
        return false;

    struct stat sbuf;
    if (stat(_filename, &sbuf) < 0)
        return false;

    // All the following checks succeed even if the file has been deleted!
    // Stat is almost the only function that recognizes that a file
    // has been removed. Doing an fopen/fclose would also work.

#if 0
    if (GetModTime() < 0)
        return false;
    if (fseek(_file, 0, SEEK_CUR) < 0)
        return false;
    if (Tell() < 0)
        return false;
#endif

    return true;
}

/*------------------------------------------------------------------------------
 * function: DataSourceFileStream::Close
 * Do an fclose() on the file corresponding to the DataSourceFileStream object.
 */
DevStatus
DataSourceFileStream::Close()
{
	DO_DEBUG(printf("DataSourceFileStream::Close()\n"));
        DOASSERT(_file != NULL, "Invalid file pointer");

	DevStatus	result = StatusOk;

	if (fclose(_file) != 0)
	{
		reportError("error closing file", errno);
		result = StatusFailed;
	}
	_file = NULL;	// So destructor doesn't try to close it again.

	return result;
}

/*------------------------------------------------------------------------------
 * function: DataSourceFileStream::Fgets
 * Do an fgets() on the file corresponding to the DataSourceFileStream object.
 */
char *
DataSourceFileStream::Fgets(char *buffer, int bufSize)
{
	DO_DEBUG(printf("DataSourceFileStream::Fgets()\n"));
        DOASSERT(_file != NULL, "Invalid file pointer");

	errno = 0;
	char *result = fgets(buffer, bufSize, _file);
	if ((result == NULL) && (errno != 0)) reportErrSys("fgets");

	return result;
}

/*------------------------------------------------------------------------------
 * function: DataSourceFileStream::Fread
 * Do fread() on the stream associated with this object.
 */
size_t
DataSourceFileStream::Fread(char *buf, size_t size, size_t itemCount)
{
	DO_DEBUG(printf("DataSourceFileStream::Fread()\n"));
        DOASSERT(_file != NULL, "Invalid file pointer");

	return fread(buf, size, itemCount, _file);
}

/*------------------------------------------------------------------------------
 * function: DataSourceFileStream::Read
 * Do read() on the stream associated with this object.
 */
size_t
DataSourceFileStream::Read(char *buf, int byteCount)
{
	DO_DEBUG(printf("DataSourceFileStream::Read()\n"));
        DOASSERT(_file != NULL, "Invalid file pointer");

	return read(fileno(_file), buf, byteCount);
}

/*------------------------------------------------------------------------------
 * function: DataSourceFileStream::Fwrite
 * Do fwrite() on the stream associated with this object.
 */
size_t
DataSourceFileStream::Fwrite(const char *buf, size_t size, size_t itemCount)
{
	DO_DEBUG(printf("DataSourceFileStream::Fwrite()\n"));
        DOASSERT(_file != NULL, "Invalid file pointer");

	return fwrite(buf, size, itemCount, _file);
}

/*------------------------------------------------------------------------------
 * function: DataSourceFileStream::Write
 * Do write() on the stream associated with this object.
 */
size_t
DataSourceFileStream::Write(const char *buf, size_t byteCount)
{
	DO_DEBUG(printf("DataSourceFileStream::Write()\n"));
        DOASSERT(_file != NULL, "Invalid file pointer");

	return write(fileno(_file), buf, byteCount);
}

/*------------------------------------------------------------------------------
 * function: DataSourceFileStream::Seek
 * Do fseek() on the stream associated with this object.
 */
int
DataSourceFileStream::Seek(long offset, int from)
{
	DO_DEBUG(printf("DataSourceFileStream::Seek()\n"));
        DOASSERT(_file != NULL, "Invalid file pointer");

	return fseek(_file, offset, from);
}

/*------------------------------------------------------------------------------
 * function: DataSourceFileStream::Tell
 * Do ftell() on the stream associated with this object.
 */
long
DataSourceFileStream::Tell()
{
	DO_DEBUG(printf("DataSourceFileStream::Tell()\n"));
        DOASSERT(_file != NULL, "Invalid file pointer");

	return ftell(_file);
}

/*------------------------------------------------------------------------------
 * function: DataSourceFileStream::gotoEnd
 * Go to the end of the file associated with this object, and return the
 * offset of the end of the file.
 */
int
DataSourceFileStream::gotoEnd()
{
	//DO_DEBUG(printf("DataSourceFileStream::gotoEnd()\n"));
        DOASSERT(_file != NULL, "Invalid file pointer");

	int		result = 0;

	if (fseek(_file, 0, SEEK_END) < 0)
	{
		reportError("Cannot seek to end of file", errno);
		result = -1;
		DOASSERT(false, "");
	}
	else
	{
		result = ftell(_file);
		if (result < 0)
		{
			reportError("Cannot get current file position", errno);
			result = -1;
			DOASSERT(false, "");
		}
	}

	return result;
}

/*------------------------------------------------------------------------------
 * function: DataSourceFileStream::append
 * Append the given record to the end of the file associated with this
 * object.
 */
int
DataSourceFileStream::append(void *buf, int recSize)
{
	DO_DEBUG(printf("DataSourceFileStream::append()\n"));
        DOASSERT(_file != NULL, "Invalid file pointer");

	int		result = 0;

	if (gotoEnd() < 0)
	{
		reportError("Cannot go to end of file", devNoSyserr);
		result = -1;
	}
	else
	{
		result = write(fileno(_file), buf, recSize);
	}

	return result;
}

/*------------------------------------------------------------------------------
 * function: DataSourceFileStream::GetModTime
 * Returns the last modification time of the object.
 */
int
DataSourceFileStream::GetModTime()
{
    DO_DEBUG(printf("DataSourceFileStream::GetModTime()\n"));
    DOASSERT(_file != NULL, "Invalid file pointer");

    struct stat sbuf;
    int status = fstat(fileno(_file), &sbuf);
    if (status < 0) {
        reportError("Cannot get modification time for file", devNoSyserr);
        return -1;
    }
    int result = (long)sbuf.st_mtime;
    
    return result;
}

/*============================================================================*/

/*------------------------------------------------------------------------------
 * function: DataSourceFileStream::DataSize
 * Return file size.
 */
bytecount_t
DataSourceFileStream::DataSize()
{
    if (!_file)
        return 0;

    struct stat sbuf;
    if (stat(_filename, &sbuf) < 0)
        return 0;

    return (bytecount_t)sbuf.st_size;
}
