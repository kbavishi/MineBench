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
  Implementation of DataSourceWeb class.

  It derives from the DataSourceFileDesc class and makes _file
  be the file pointer of the cache file instead of the fdopen()'d
  pointer to the file descriptor of the socket which is the default
  behavior of the DataSourceFileDesc class.
 */

/*
  $Id: DataSourceWeb.c 2396 2014-03-13 21:13:27Z wkliao $

  $Log: DataSourceWeb.c,v $
  Revision 1.7  1996/10/04 17:10:43  wenger
  Fixed error in open_ftp() call that prevented compile.

  Revision 1.6  1996/10/02 19:47:15  jussi
  Added support for opening an HTTP connection for writing (posting)
  Web data.

  Revision 1.5  1996/10/02 15:23:48  wenger
  Improved error handling (modified a number of places in the code to use
  the DevError class).

  Revision 1.4  1996/07/14 20:34:09  jussi
  Rewrote class to fork a process that does all data transfers
  from the Web site.

  Revision 1.3  1996/07/12 19:37:21  jussi
  Added timeout processing for network connections.

  Revision 1.2  1996/07/02 22:48:14  jussi
  Fixed small bug in AsyncIO. Close() no longer uses the base
  class Close() routine.

  Revision 1.1  1996/07/01 19:21:24  jussi
  Initial revision.
*/

#define _DataSourceWeb_c_

//#define DEBUG

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "DataSourceWeb.h"
#include "Timer.h"
#include "Util.h"
#include "Web.h"
#include "DevError.h"

#if !defined(lint) && defined(RCSID)
static char rcsid[] = "$RCSfile: DataSourceWeb.c,v $ $Revision: 2396 $ $State: Exp $";
#endif

static char * srcFile = __FILE__;

/*------------------------------------------------------------------------------
 * function: DataSourceWeb::DataSourceWeb
 * DataSourceWeb constructor.
 */
DataSourceWeb::DataSourceWeb(char *url, char *label, char *cache) :
	DataSourceFileStream(cache, label)
{
    DO_DEBUG(printf("DataSourceWeb::DataSourceWeb(%s,%s,%s)\n",
                    url, cache, (label != NULL) ? label : "null"));

    _url = strdup(url);
    _childPid = -1;
}

/*------------------------------------------------------------------------------
 * function: DataSourceWeb::~DataSourceWeb
 * DataSourceWeb destructor.
 */
DataSourceWeb::~DataSourceWeb()
{
    DO_DEBUG(printf("DataSourceWeb::~DataSourceWeb()\n"));

    if (_childPid >= 0)
        (void)kill(_childPid, SIGKILL);

    delete _url;
}

/*------------------------------------------------------------------------------
 * function: DataSourceWeb::Open
 * Open Web data source.
 */
DevStatus
DataSourceWeb::Open(char *mode)
{
    DO_DEBUG(printf("DataSourceWeb::Open()\n"));

    DOASSERT(!_file && _childPid < 0, "Invalid file or process id");

    if (strcmp(mode, "r")) {
        reportError("cannot write to a Web data source", EINVAL);
        return StatusFailed;
    }

    // if file can be opened for reading, assume that the cache file
    // is complete and there is no need to fetch more data from the URL
    _file = fopen(_filename, "r");
    if (_file != NULL) {
        return StatusOk;
    }

    // create cache file and close it immediately
    _file = fopen(_filename, "w");
    if (!_file) {
        char	errBuf[MAXPATHLEN+100];
        sprintf(errBuf, "cannot create cache file %s", _filename);
        reportError(errBuf, errno);
        return StatusFailed;
    }
    fclose(_file);

    // reopen cache file for reading
    _file = fopen(_filename, "r");
    DOASSERT(_file, "Inconsistent file activity");

    _childPid = fork();
    if (_childPid < 0) {
        reportError("cannot fork process", errno);
        return StatusFailed;
    }

    if (!_childPid) {
        DevStatus status = ChildProc();
        if (status != StatusOk)
            reportError("Web data transfer failed", errno);
        _exit(1);
    }

    return StatusOk;
}

/*------------------------------------------------------------------------------
 * function: DataSourceWeb::Close
 * Do a close() on the file descriptor.
 */
DevStatus
DataSourceWeb::Close()
{
    DO_DEBUG(printf("DataSourceWeb::Close()\n"));

    if (_childPid >= 0)
        (void)kill(_childPid, SIGKILL);

    _childPid = -1;

    return DataSourceFileStream::Close();
}

/*------------------------------------------------------------------------------
 * function: DataSourceWeb::ChildProc
 * Child process for fetching data from Web source.
 */
DevStatus
DataSourceWeb::ChildProc()
{
    DO_DEBUG(printf("DataSourceWeb::ChildProc()\n"));

    Timer::StopTimer();

    printf("Initiating data transfer from %s\n", _url);

    int fd;
    size_t totlen = 0;

    if (!strncmp(_url, "ftp://", 6))
        fd = open_ftp(_url, 1);
    else
        fd = open_http(_url, 1, &totlen);

    if (fd < 0) {
        fprintf(stderr, "Unable to open URL %s", _url);
        return StatusFailed;
    }

    // attempt to create an empty cache file
    FILE *cfile = fopen(_filename, "w");
    if (!cfile) {
        fprintf(stderr, "Unable to create cache file %s", _filename);
        return StatusFailed;
    }

    char buffer[1024];

    while (1) {
        int len = read(fd, buffer, sizeof buffer);
        if (len == 0) {
            printf("Data transfer from %s complete\n", _url);
            close(fd);
            fclose(cfile);
            return StatusOk;
        }
        if (len < 0) {
            reportErrSys("Cannot read from network");
            break;
        }
        if (fseek(cfile, 0, SEEK_END) < 0) {
            reportErrSys("Cannot seek to end of file");
            break;
        }
        if (fwrite(buffer, len, 1, cfile) != 1) {
            reportErrSys("Cannot write to cache file");
            break;
        }
    }
    
    close(fd);
    fclose(cfile);

    return StatusFailed;
}

/*------------------------------------------------------------------------------
 * function: DataSourceWeb::Fwrite
 * Do fwrite() on the stream associated with this object.
 */
size_t
DataSourceWeb::Fwrite(const char *buf, size_t size, size_t itemCount)
{
    DO_DEBUG(printf("DataSourceWeb::Fwrite()\n"));

    reportError("writing to Web data source not supported", EINVAL);
    return 0;
}

/*------------------------------------------------------------------------------
 * function: DataSourceWeb::Write
 * Do write() on the stream associated with this object.
 */
size_t
DataSourceWeb::Write(const char *buf, size_t byteCount)
{
    DO_DEBUG(printf("DataSourceWeb::Write()\n"));

    reportError("writing to Web data source not supported", EINVAL);
    return 0;
}

/*------------------------------------------------------------------------------
 * function: DataSourceWeb::append
 * Append the given record to the end of the file associated with this
 * object.
 */
int
DataSourceWeb::append(void *buf, int recSize)
{
    DO_DEBUG(printf("DataSourceWeb::append()\n"));
    int result = 0;

    reportError("writing to Web data source not supported", EINVAL);
    return -1;
}

/*============================================================================*/
