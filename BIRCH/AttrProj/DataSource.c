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
  Implementation of DataSource class.
 */

/*
  $Id: DataSource.c 2396 2014-03-13 21:13:27Z wkliao $

  $Log: DataSource.c,v $
  Revision 1.8  1996/12/13 21:34:20  jussi
  Added checking of available semaphores.

  Revision 1.7  1996/12/03 20:36:02  jussi
  Added support for concurrent I/O.

  Revision 1.6  1996/08/04 21:23:22  beyer
  DataSource's are now reference counted.
  Added Version() which TData now check to see if the DataSource has changed,
    and if it has, it rebuilds its index and invalidates the cache.
  DataSourceFixedBuf is a DataSourceBuf that allocates and destoyes its
    own buffer.
  DerivedDataSource functionality is now in the TData constructor.
  Added some defaults for virtual methods.

  Revision 1.5  1996/07/01 19:31:30  jussi
  Added an asynchronous I/O interface to the data source classes.
  Added a third parameter (char *param) to data sources because
  the DataSegment template requires that all data sources have the
  same constructor (DataSourceWeb requires the third parameter).

  Revision 1.4  1996/05/22 17:51:55  wenger
  Extended DataSource subclasses to handle tape data; changed TDataAscii
  and TDataBinary classes to use new DataSource subclasses to hide the
  differences between tape and disk files.

  Revision 1.3  1996/05/07 22:28:14  jussi
  Reverted the changes made in the previous check-in because I
  found a better way to fix the problem where only the tail
  part of a schema file name is returned when a session is saved.

  Revision 1.2  1996/05/07 22:13:44  jussi
  Added virtual method getName() to DataSourceFile which returns
  the filename, not the regular name or alias. The filename is
  needed when a session file is saved.

  Revision 1.1  1996/05/07 16:04:17  wenger
  Added final version of code for reading schemas from session files;
  added an error-reporting class to improve error info.
 */

#define _DataSource_c_

//#define DEBUG

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/param.h>
#include <unistd.h>

#include "DataSource.h"
#include "Util.h"
#include "DevError.h"
#include "tapedrive.h"


#if !defined(lint) && defined(RCSID)
static char	rcsid[] = "$RCSfile: DataSource.c,v $ $Revision: 2396 $ $State: Exp $";
#endif

static char *	srcFile = __FILE__;

/*------------------------------------------------------------------------------
 * function: DataSource::DataSource
 * DataSource constructor.
 */
DataSource::DataSource(char *label, ViewGraph* controlling_view)
{
    DO_DEBUG(printf("DataSource::DataSource(%s)\n",
		    (label != NULL) ? label : "<null>"));

    _ref_count = 0;
    _label = NULL;
    if (label != NULL)
	_label = strdup(label);

    _version = 0;
    _controlling_view = NULL;

    _child = -1;
}

/*------------------------------------------------------------------------------
 * function: DataSource::~DataSource
 * DataSource destructor.
 */
DataSource::~DataSource()
{
    DO_DEBUG(printf("DataSource::~DataSource(%s)\n",
		    (_label != NULL) ? _label : "<null>"));

    DOASSERT(_ref_count == 0, 
	     "Deleting datasource with dangling references");
    
    if (_label != NULL) {
	delete [] _label;
    }
}


//---------------------------------------------------------------------------
void DataSource::AddRef()
{
    DO_DEBUG(printf("DataSource::AddRef(%s) refs=%d\n",
		    (_label != NULL) ? _label : "<null>", _ref_count));
    _ref_count++;
}


//---------------------------------------------------------------------------
// returns true if the caller should delete this object
bool DataSource::DeleteRef()
{
    DO_DEBUG(printf("DataSource::DeleteRef(%s) refs=%d\n",
		    (_label != NULL) ? _label : "<null>", _ref_count));
    _ref_count--;
    DOASSERT(_ref_count >= 0, "invalid datasource ref_count");
    // this could delete itself, but other old code still does that...
    // also, this could be statically allocated...
    return _ref_count == 0;
}



/*------------------------------------------------------------------------------
 * function: DataSource::getLabel
 * Get the label of a DataSource object.
 */
char *
DataSource::getLabel()
{
	DO_DEBUG(printf("DataSource::getLabel()\n"));

	return _label;
}

/*------------------------------------------------------------------------------
 * function: DataSource::Fgets
 * This is a dummy function to catch calls to Fgets() for derived classes
 * that don't implement it.
 */
char *
DataSource::Fgets(char *buffer, int size)
{
	DO_DEBUG(printf("DataSource::Fgets()\n"));

	fprintf(stderr, "Fgets method not implemented for class %s\n",
		objectType());
	DOASSERT(false, "");

	return NULL;
}

/*------------------------------------------------------------------------------
 * function: DataSource::Fread
 * This is a dummy function to catch calls to Fread() for derived classes
 * that don't implement it.
 */
size_t
DataSource::Fread(char *buf, size_t size, size_t itemCount)
{
	DO_DEBUG(printf("DataSource::Fread()\n"));

	fprintf(stderr, "Fread method not implemented for class %s\n",
		objectType());
	DOASSERT(false, "");

	return 0;
}

/*------------------------------------------------------------------------------
 * function: DataSource::Read
 * This is a dummy function to catch calls to Read() for derived classes
 * that don't implement it.
 */
size_t
DataSource::Read(char *buf, int byteCount)
{
	DO_DEBUG(printf("DataSource::Read()\n"));

	fprintf(stderr, "Read method not implemented for class %s\n",
		objectType());
	DOASSERT(false, "");

	return 0;
}

/*------------------------------------------------------------------------------
 * function: DataSource::Fwrite
 * This is a dummy function to catch calls to Fwrite() for derived classes
 * that don't implement it.
 */
size_t
DataSource::Fwrite(const char *buf, size_t size, size_t itemCount)
{
	DO_DEBUG(printf("DataSource::Fwrite()\n"));

	fprintf(stderr, "Fwrite method not implemented for class %s\n",
		objectType());
	DOASSERT(false, "");

	return 0;
}

/*------------------------------------------------------------------------------
 * function: DataSource::Write
 * This is a dummy function to catch calls to Write() for derived classes
 * that don't implement it.
 */
size_t
DataSource::Write(const char *buf, size_t byteCount)
{
	DO_DEBUG(printf("DataSource::Write()\n"));

	fprintf(stderr, "Write method not implemented for class %s\n",
		objectType());
	DOASSERT(false, "");

	return 0;
}

/*------------------------------------------------------------------------------
 * function: DataSource::append
 * This is a dummy function to catch calls to append() for derived classes
 * that don't implement it.
 */
int
DataSource::append(void *buf, int recSize)
{
	DO_DEBUG(printf("DataSource::append()\n"));

	fprintf(stderr, "append method not implemented for class %s\n",
		objectType());
	DOASSERT(false, "");

	return -1;
}

/*--------------------------------------------------------------------------*/
int DataSource::GetModTime()
{
  return -1;
}


/*--------------------------------------------------------------------------*/
Boolean DataSource::isFile()
{
  return false;
}


/*--------------------------------------------------------------------------*/
Boolean DataSource::isBuf()
{
  return false;
}


/*--------------------------------------------------------------------------*/
Boolean DataSource::isTape()
{
  return false;
}


/*------------------------------------------------------------------------------
 * function: DataSource::printStats
 * This is a dummy function to catch calls to printStats() for derived classes
 * that don't implement it.
 */
void
DataSource::printStats()
{
	DO_DEBUG(printf("DataSource::printStats()\n"));

	fprintf(stderr, "printStats method not implemented for class %s\n",
		objectType());
	DOASSERT(false, "");

	return;
}

/*============================================================================*/

//
// Kick off data source as a separate process
//

int DataSource::InitializeProc()
{
    if (_child >= 0) {
        fprintf(stderr, "Child process/thread exists already\n");
        return 0;
    }

    if (SemaphoreV::numAvailable() < 4) {
        // One semaphore needed for DataSource, and 3 more for DataPipe.
        return -1;
    }

    int status;
    _mutex = new SemaphoreV(Semaphore::newKey(), status, 1);
    if (!_mutex || status < 0) {
        fprintf(stderr, "Cannot create semaphore\n");
        return -1;
    }
    _mutex->setValue(1);

    if (pipe(_reqFd) < 0) {
        perror("pipe");
        return -1;
    }
#ifdef SOLARIS
    // Pipes are bi-directional in Solaris
    _replyFd[0] = _reqFd[1];
    _replyFd[1] = _reqFd[0];
#else
    if (pipe(_replyFd) < 0) {
        perror("pipe");
        return -1;
    }
#endif

    _totalCount = 0;
    _readBytes = 0;
    _writeBytes = 0;
    _seekBytes = 0;

    _handle = 1;

    _dpipe = new DataPipe(10, status);
    if (!_dpipe || status < 0) {
        fprintf(stderr, "Cannot create data pipe\n");
        return -1;
    }

    DO_DEBUG(printf("Creating data source process/thread...\n"));

#ifdef DS_PROCESS
    fflush(stdout);
    _child = fork();
    if (_child < 0) {
        perror("fork");
        return -1;
    }

    if (!_child) {
        (void)ProcessReq(this);
        close(_reqFd[0]);
        close(_reqFd[1]);
#ifndef SOLARIS
        close(_replyFd[0]);
        close(_replyFd[1]);
#endif
        exit(1);
    }
#endif

#ifdef DS_THREAD
    if (pthread_create(&_child, 0, ProcessReq, this)) {
        perror("pthread_create");
        return -1;
    }
#endif

    DO_DEBUG(printf("Created data source process/thread %ld\n", (long)_child));

    return 0;
}

/*============================================================================*/

//
// Terminate data source process
//

int DataSource::TerminateProc()
{
    if (_child < 0) {
      fprintf(stderr, "Child process/thread no longer exists\n");
      return 0;
    }

    DO_DEBUG(printf("Terminating child process/thread %ld...\n",
                    (long)_child));

    Request req = { TerminateReq, 0, 0 };
    if (writen(_reqFd[1], (char *)&req, sizeof req) < (int)sizeof req) {
        perror("write");
    } else {
#ifdef DS_PROCESS
        while(1) {
            int status;
            pid_t child = wait(&status);
            if (child < 0) {
                if (errno == EINTR)
                    continue;
                if (errno != ECHILD) {
                    perror("wait");
                    break;
                }
            } else
                break;
        }
#endif
#ifdef DS_THREAD
        (void)pthread_join(_child, 0);
#endif
    }

    close(_reqFd[0]);
    close(_reqFd[1]);
#ifndef SOLARIS
    close(_replyFd[0]);
    close(_replyFd[1]);
#endif

    delete _dpipe;

    _mutex->destroy();
    delete _mutex;

    _child = -1;

    DO_DEBUG(printf("Child process/thread terminated\n"));

    return 0;
}
    
/*============================================================================*/

//
// Submit read request to data source process
//

int DataSource::ReadProc(streampos_t offset, iosize_t bytes)
{
    AcquireMutex();

    DO_DEBUG(printf("Submitting read request %llu:%lu to data source 0x%p\n",
                    offset, bytes, this));

    Request req = { ReadReq, offset, bytes };
    if (writen(_reqFd[1], (char *)&req, sizeof req) < (int)sizeof req) {
        perror("write");
        ReleaseMutex();
        return -1;
    }

    Reply reply;
    if (readn(_replyFd[0], (char *)&reply, sizeof reply) < (int)sizeof reply) {
        perror("write");
        ReleaseMutex();
        return -1;
    }

    ReleaseMutex();

    return reply.handle;
}

/*============================================================================*/

//
// Submit write request to data source process
//

int DataSource::WriteProc(streampos_t offset, iosize_t bytes)
{
    AcquireMutex();

    DO_DEBUG(printf("Submitting write request %llu:%lu to data source 0x%p\n",
                    offset, bytes, this));

    Request req = { WriteReq, offset, bytes };
    if (writen(_reqFd[1], (char *)&req, sizeof req) < (int)sizeof req) {
        perror("write");
        ReleaseMutex();
        return -1;
    }

    Reply reply;
    if (readn(_replyFd[0], (char *)&reply, sizeof reply) < (int)sizeof reply) {
        perror("write");
        ReleaseMutex();
        return -1;
    }

    ReleaseMutex();

    return reply.handle;
}

/*============================================================================*/

//
// Jacket routine for interfacing ProcessReq() to POSIX threads.
//

void *DataSource::ProcessReq(void *arg)
{
    DataSource &me = *(DataSource *)arg;
    return me.ProcessReq();
}

/*============================================================================*/

//
// Read requests from pipe and execute them.
//

void *DataSource::ProcessReq()
{
#if defined(DS_PROCESS) && defined(DEBUG) && 0
    char fname[64];
    sprintf(fname, "source.fd.%d", _reqFd[0]);
    printf("Source 0x%p log file is %s\n", this, fname);
    FILE *out = freopen(fname, "w", stdout);
    if (!out)
        perror("freopen");
#endif

    while (1) {
        Request req;
        int status = readn(_reqFd[0], (char *)&req, sizeof req);
        if (status < (int)sizeof req) {
            perror("read");
            break;
        }

        if (req.type == TerminateReq) {
            DO_DEBUG(printf("\nSource 0x%p received quit command\n", this));
            printf("Data source: %llu requests, read: %.2f MB, write: %.2f MB, seek: %.2f MB\n",
                   _totalCount, _readBytes / 1048576.0,
                   _writeBytes / 1048576.0, _seekBytes / 1048576.0);
            break;
        }

        DO_DEBUG(printf("\nSource 0x%p received request: %d, %llu, %lu\n",
                        this, req.type, req.offset, req.bytes));

        _totalCount++;

        Reply reply = { _handle };
        _handle++;

        status = writen(_replyFd[1], (char *)&reply, sizeof reply);
        if (status < (int)sizeof reply) {
            perror("write");
            break;
        }

        DO_DEBUG(printf("Source 0x%p request acknowledged\n", this));

        if (req.type == ReadReq)
            ReadStream(req.offset, req.bytes);
        else
            WriteStream(req.offset, req.bytes);

        DO_DEBUG(printf("Source 0x%p request completed\n", this));
    }

    DO_DEBUG(printf("Source 0x%p terminates\n", this));

    return 0;
}

/*============================================================================*/

//
// Read data from device and pipe it to the consumer.
//

void DataSource::ReadStream(streampos_t offset, iosize_t totbytes)
{
    DO_DEBUG(printf("Reading bytes %llu:%lu from data source 0x%p\n",
                    offset, totbytes, this));

    streampos_t curpos = Tell();
    int status = Seek(offset, SEEK_SET);
    DOASSERT(status >= 0, "Cannot seek data source");

    // Be careful not to overflow/underflow (unsigned numbers)
    _seekBytes += (curpos > offset ? curpos - offset : offset - curpos);

    iosize_t bytes = 0;

    while (!totbytes || bytes < totbytes) {
        char *page;
        status = MemMgr::Instance()->Allocate(MemMgr::Buffer, page);
        DOASSERT(status >= 0 && page, "Failed to allocate buffer space\n");
        int pageSize = MemMgr::Instance()->PageSize();
        iosize_t reqsize = pageSize;
        if (totbytes > 0 && totbytes - bytes < reqsize)
            reqsize = totbytes - bytes;
        DO_DEBUG(printf("Data source 0x%p reads %lu bytes\n", this, reqsize));
        size_t b = Fread(page, 1, reqsize);
        DO_DEBUG(printf("Data source 0x%p produces bytes %llu:%u\n",
                        this, offset, b));
        status = Produce(page, offset, b);
        DOASSERT(status >= 0, "Cannot produce data");
        offset += b;
        bytes += b;
        _readBytes += b;
        if ((int)b < pageSize)
            break;
    }

    status = Produce(0, offset, 0);
    DOASSERT(status >= 0, "Cannot produce data");
}

/*============================================================================*/

//
// Consume data from pipe and write it to device.
//

void DataSource::WriteStream(streampos_t offset, iosize_t totbytes)
{
    DO_DEBUG(printf("Writing bytes %llu:%lu to data source 0x%p\n",
                    offset, totbytes, this));

    streampos_t curpos = Tell();
    int status = Seek(offset, SEEK_SET);
    DOASSERT(status >= 0, "Cannot seek data source");

    // Be careful not to overflow/underflow (unsigned numbers)
    _seekBytes += (curpos > offset ? curpos - offset : offset - curpos);

    iosize_t bytes = 0;

    while (!totbytes || bytes < totbytes) {
        char *page;
        streampos_t off;
        iosize_t b;
        int status = Consume(page, off, b);
        DOASSERT(status >= 0, "Cannot consume data");
        DO_DEBUG(printf("Data source 0x%p consumes bytes %llu:%lu\n",
                        this, offset, b));
        DOASSERT(off == offset, "Invalid data chunk consumed");
        if (b <= 0)
            continue;
        size_t s = Fwrite(page, 1, b);
        if (s != (size_t)b)
            break;
        status = MemMgr::Instance()->Deallocate(MemMgr::Buffer, page);
        DOASSERT(status >= 0 && page, "Failed to deallocate buffer space\n");
        offset += b;
        bytes += b;
        _writeBytes += b;
    }
}
