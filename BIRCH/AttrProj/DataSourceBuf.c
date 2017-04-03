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
   Implementation of the DataSourceBuf class.
   */

/*
   $Id: DataSourceBuf.c 2396 2014-03-13 21:13:27Z wkliao $

   $Log: DataSourceBuf.c,v $
   Revision 1.7  1996/10/08 21:49:07  wenger
   ClassDir now checks for duplicate instance names; fixed bug 047
   (problem with FileIndex class); fixed various other bugs.

   Revision 1.6  1996/10/07 22:53:57  wenger
   Added more error checking and better error messages in response to
   some of the problems uncovered by CS 737 students.

   Revision 1.5  1996/08/05 19:48:56  wenger
   Fixed compile errors caused by some of Kevin's recent changes; changed
   the attrproj stuff to make a .a file instead of a .o; added some more
   TData file writing stuff; misc. cleanup.

   Revision 1.4  1996/08/04 21:23:23  beyer
   DataSource's are now reference counted.
   Added Version() which TData now check to see if the DataSource has changed,
     and if it has, it rebuilds its index and invalidates the cache.
   DataSourceFixedBuf is a DataSourceBuf that allocates and destoyes its
     own buffer.
   DerivedDataSource functionality is now in the TData constructor.
   Added some defaults for virtual methods.

   Revision 1.3  1996/07/12 18:24:42  wenger
   Fixed bugs with handling file headers in schemas; added DataSourceBuf
   to TDataAscii.

   Revision 1.2  1996/07/01 19:31:32  jussi
   Added an asynchronous I/O interface to the data source classes.
   Added a third parameter (char *param) to data sources because
   the DataSegment template requires that all data sources have the
   same constructor (DataSourceWeb requires the third parameter).

   Revision 1.1  1996/05/22 17:52:00  wenger
   Extended DataSource subclasses to handle tape data; changed TDataAscii
   and TDataBinary classes to use new DataSource subclasses to hide the
   differences between tape and disk files.

   */

#define _DataSourceBuf_c_

//#define DEBUG

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/param.h>
#include <unistd.h>

#include "DataSourceBuf.h"
#include "Util.h"
#include "DevError.h"
#include "tapedrive.h"


#if !defined(lint) && defined(RCSID)
static char		rcsid[] = "$RCSfile: DataSourceBuf.c,v $ $Revision: 2396 $ $State: Exp $";
#endif

static char *	srcFile = __FILE__;

/*------------------------------------------------------------------------------
 * function: DataSourceBuf::DataSourceBuf
 * DataSourceBuf constructor.
 */
DataSourceBuf::DataSourceBuf(char *buffer, int buffer_size,
			     int data_size, char *label)
: DataSource(label)
{
    DO_DEBUG(printf("DataSourceBuf::DataSourceBuf(%s)\n",
		    (label != NULL) ? label : "<null>"));
    DOASSERT(buffer != NULL, "DataSourceBuf::DataSourceBuf: null buffer");
    DOASSERT(buffer_size > 0, 
	     "DataSourceBuf::DataSourceBuf: buffer too small");

    _sourceBuf = buffer;
    _end_buffer = _sourceBuf + buffer_size -1;
    _end_data = _sourceBuf + data_size -1;
    DOASSERT(_end_data <= _end_buffer, "more data than buffer space");
    _currentLoc = NULL;
}

/*------------------------------------------------------------------------------
 * function: DataSourceBuf::~DataSourceBuf
 * DataSourceBuf destructor.
 */
DataSourceBuf::~DataSourceBuf()
{
    DO_DEBUG(printf("DataSourceBuf::~DataSourceBuf(%s)\n",
		    (_label != NULL) ? _label : "<null>"));
}

/*------------------------------------------------------------------------------
 * function: DataSourceBuf::Open
 * Simulate an fopen() on the DataSourceBuf object.
 */
DevStatus
DataSourceBuf::Open(char *)
{
    DO_DEBUG(printf("DataSourceBuf::Open(%s)\n",
		    (_label != NULL) ? _label : "<null>"));

    DevStatus	result = StatusOk;

    _currentLoc = _sourceBuf;

    return result;
}

/*------------------------------------------------------------------------------
 * function: DataSourceBuf::Close
 * Simulate an fclose() on the DataSourceBuf object.
 */
DevStatus
DataSourceBuf::Close()
{
    DO_DEBUG(printf("DataSourceBuf::Close(%s)\n",
		    (_label != NULL) ? _label : "<null>"));

    DevStatus	result = StatusOk;

    _currentLoc = NULL;

    return result;
}

/*------------------------------------------------------------------------------
 * function: DataSourceBuf::Fgets
 * Simulate an fgets() on the DataSourceBuf object.
 */
char *
DataSourceBuf::Fgets(char *buffer, int bufSize)
{
    DO_DEBUG(printf("DataSourceBuf::FGets(%s)\n",
		    (_label != NULL) ? _label : "<null>"));
    DOASSERT(buffer != NULL, "DataSourceBuf::Fgets null buffer");
    DOASSERT(bufSize > 0, "DataSourceBuf::Fgets: zero sized buffer");

    char *		endOfBuf = buffer + bufSize - 1;
    Boolean		endOfLine = false;
    char *		outputP = buffer;

    DO_DEBUG(printf("sourceBuf=0x%p, end_buffer=0x%p, end_data=0x%p, currentLoc=0x%p\n", 
		    _sourceBuf, _end_buffer, _end_data, _currentLoc));
    DO_DEBUG(printf("buffer=0x%p, endOfBuf=0x%p\n", buffer, endOfBuf));

    if (_currentLoc == NULL ) {
	reportError("DataSourceBuf: not open", devNoSyserr);
    } else if( _currentLoc > _end_data || *_currentLoc == '\0' ) {
#if 0 // This happens normally when building an index.
	if (_currentLoc > _end_data) {
	  reportErrNosys( "Reading past end of data");
	}
#endif
	return NULL;		// Signal "EOF" - don't modify buffer
    } else {
	while ((outputP < endOfBuf) && !endOfLine) {
	    // End of string in the buffer is equivalent to EOF in real
	    // fgets().
	    if (_currentLoc > _end_data || *_currentLoc == '\0') {
		endOfLine = true;
	    } else {
		if( *_currentLoc == '\n' ) {
		    endOfLine = true;
		}
		*outputP = *_currentLoc;
		_currentLoc++;
		outputP++;
	    }
	}
	*outputP = '\0';
    }
    DO_DEBUG(printf("sourceBuf=0x%p, end_buffer=0x%p, end_data=0x%p, currentLoc=0x%p\n", 
		    _sourceBuf, _end_buffer, _end_data, _currentLoc));
    return buffer;
}

/*------------------------------------------------------------------------------
 * function: DataSourceBuf::Fread
 * Simulate fread() on the buffer associated with this object.
 */
size_t
DataSourceBuf::Fread(char *buf, size_t size, size_t itemCount)
{
    DO_DEBUG(printf("DataSourceBuf::Fread(%s)\n",
		    (_label != NULL) ? _label : "<null>"));
    DO_DEBUG(printf("DataSourceBuf::Fread()\n"));

    return Read(buf, size * itemCount) / size;
}

/*------------------------------------------------------------------------------
 * function: DataSourceBuf::Read
 * Simulate read() on the buffer associated with this object.
 */
size_t
DataSourceBuf::Read(char *buf, int byteCount)
{
    DO_DEBUG(printf("DataSourceBuf::Read(%s)\n",
		    (_label != NULL) ? _label : "<null>"));
    
    int		result = -1;
    
    if (_currentLoc == NULL) {
	reportError("DataSourceBuf: not open", devNoSyserr);
    } else if( _currentLoc > _end_data ) {
	reportError("DataSourceBuf: read beyond eof", devNoSyserr);
    } else {
	int bytes_left = _end_data - _currentLoc + 1;
	if( byteCount > bytes_left ) byteCount = bytes_left;
	memcpy(buf, _currentLoc, byteCount);
	_currentLoc += byteCount;
	result = byteCount;
    }
    
    return result;
}

/*------------------------------------------------------------------------------
 * function: DataSourceBuf::Fwrite
 * Simulate fwrite() on the buffer associated with this object.
 * Note: no checking for going past end of buffer.
 */
size_t
DataSourceBuf::Fwrite(const char *buf, size_t size, size_t itemCount)
{
    DO_DEBUG(printf("DataSourceBuf::Fwrite(%s)\n",
		    (_label != NULL) ? _label : "<null>"));

    return Write(buf, size * itemCount) / size;
}

/*------------------------------------------------------------------------------
 * function: DataSourceBuf::Write
 * Simulate write() on the buffer associated with this object.
 */
size_t
DataSourceBuf::Write(const char *buf, size_t byteCount)
{
    DO_DEBUG(printf("DataSourceBuf::Write(%s)\n",
		    (_label != NULL) ? _label : "<null>"));
    
    int		result = 0;

    if (_currentLoc == NULL) {
	reportError("DataSourceBuf: not open", devNoSyserr);
	result = -1;
    } else {
	int bytes_left = _end_buffer - _currentLoc + 1;
	if( bytes_left > 0 ) {
	    if( (int) byteCount > bytes_left ) byteCount = bytes_left;
	    memcpy(_currentLoc, buf, byteCount);
	    _currentLoc += byteCount;
	    if( _currentLoc > _end_data ) {
		_end_data = _currentLoc - 1;
	    }
	    result = byteCount;
	}
    }
    
    return result;
}

/*------------------------------------------------------------------------------
 * function: DataSourceBuf::Seek
 * Simulate seek() on the buffer associated with this object.
 */
int
DataSourceBuf::Seek(long offset, int from)
{
    DO_DEBUG(printf("DataSourceBuf::Seek(%s)\n",
		    (_label != NULL) ? _label : "<null>"));

    switch (from)
      {
	case SEEK_SET:
	  _currentLoc = _sourceBuf + offset;
	  break;

	case SEEK_CUR:
	  _currentLoc += offset;
	  break;

	case SEEK_END:
	  (void) gotoEnd();
	  _currentLoc += offset;
	  break;

	default:
	  reportError("Illegal 'seek from' value", devNoSyserr);
	  return -1;
      }

    if( _currentLoc > _end_data ) {
	_currentLoc = _end_data + 1;
    }

    return _currentLoc - _sourceBuf;
}

/*------------------------------------------------------------------------------
 * function: DataSourceBuf::Tell
 * Simulate tell() on the buffer associated with this object.
 */
long
DataSourceBuf::Tell()
{
    DO_DEBUG(printf("DataSourceBuf::Tell(%s)\n",
		    (_label != NULL) ? _label : "<null>"));

    return _currentLoc - _sourceBuf;
}

/*------------------------------------------------------------------------------
 * function: DataSourceBuf::gotoEnd
 * Go to the end of the buffer associated with this object, and return the
 * offset of the end of the buffer.
 */
int
DataSourceBuf::gotoEnd()
{
    DO_DEBUG(printf("DataSourceBuf::gotoEnd(%s)\n",
		    (_label != NULL) ? _label : "<null>"));
    _currentLoc = _end_data + 1;
    return _currentLoc - _sourceBuf;
}


/*----------------------------------------------------------------------------*/
void DataSourceBuf::Clear()
{
    DO_DEBUG(printf("DataSourceBuf::Clear(%s)\n",
		    (_label != NULL) ? _label : "<null>"));
    _version++;
    *_sourceBuf = '\0';		// not really needed...
    _currentLoc = _sourceBuf;
    _end_data = _sourceBuf - 1;
}


/*============================================================================*/
