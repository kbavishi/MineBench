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
  Implementation of DataSourceFileDesc class.

  Note that the file descriptor is fdopen()'d, giving a file pointer
  _file, but when this is class returns a file descriptor to any
  requestor, fileno(_file) is returned instead of _fd. This is because
  a derived class may have substituted some other file pointer in
  place of _file, so fileno(_fileno) is no longer the same as _fd.
  DataSourceWeb is one example of this behavior.
 */

/*
  $Id: DataSourceFileDesc.c 2396 2014-03-13 21:13:27Z wkliao $

  $Log: DataSourceFileDesc.c,v $
  Revision 1.1  1996/07/01 19:21:25  jussi
  Initial revision.
*/

#define _DataSourceFileDesc_c_

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "DataSourceFileDesc.h"
#include "Util.h"
#include "DevError.h"

//#define DEBUG

#if !defined(lint) && defined(RCSID)
static char		rcsid[] = "$RCSfile: DataSourceFileDesc.c,v $ $Revision: 2396 $ $State: Exp $";
#endif

static char *	srcFile = __FILE__;

/*------------------------------------------------------------------------------
 * function: DataSourceFileDesc::DataSourceFileDesc
 * DataSourceFileDesc constructor.
 */
DataSourceFileDesc::DataSourceFileDesc(int fd, char *label) :
     DataSourceFileStream("no file", label)
{
    DO_DEBUG(printf("DataSourceFileDesc::DataSourceFileDesc(%d,%s)\n",
                    fd, (label != NULL) ? label : "null"));

    _fd = fd;
}

/*------------------------------------------------------------------------------
 * function: DataSourceFileDesc::~DataSourceFileDesc
 * DataSourceFileDesc destructor.
 */
DataSourceFileDesc::~DataSourceFileDesc()
{
    DO_DEBUG(printf("DataSourceFileDesc::~DataSourceFileDesc()\n"));

    if (_fd >= 0) close(_fd);
}

/*------------------------------------------------------------------------------
 * function: DataSourceFileDesc::Open
 * Do an fdopen() on the file descriptor.
 */
DevStatus
DataSourceFileDesc::Open(char *mode)
{
    DO_DEBUG(printf("DataSourceFileDesc::Open()\n"));

    _file = fdopen(_fd, mode);
    if (_file == NULL)
    {
        char	errBuf[MAXPATHLEN+100];
        sprintf(errBuf, "unable to open file descriptor %d", _fd);
        reportError(errBuf, errno);
        return StatusFailed;
    }

    return StatusOk;
}

/*------------------------------------------------------------------------------
 * function: DataSourceFileDesc::IsOk
 * Return true if file descriptor is valid.
 */
Boolean
DataSourceFileDesc::IsOk()
{
    if (_fd < 0 || _file == NULL)
        return false;

    return true;
}

/*------------------------------------------------------------------------------
 * function: DataSourceFileDesc::Close
 * Do a close() on the file descriptor.
 */
DevStatus
DataSourceFileDesc::Close()
{
    DO_DEBUG(printf("DataSourceFileDesc::Close()\n"));

    if (close(_fd) < 0)
    {
        reportError("error closing file", errno);
        _fd = -1;
        return StatusFailed;
    }

    _fd = -1;	// So destructor doesn't try to close it again.

    return DataSourceFileStream::Close();
}

/*============================================================================*/
