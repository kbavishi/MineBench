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
  Implementation of DataSourceDQL class.

  It derives from the DataSourceFileDesc class and makes _file
  be the file pointer of the cache file instead of the fdopen()'d
  pointer to the file descriptor of the socket which is the default
  behavior of the DataSourceFileDesc class.
 */

#define _DataSourceDQL_c_

//#define DEBUG

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "DataSourceDQL.h"
#include "Util.h"
#include "DevError.h"

/*------------------------------------------------------------------------------
 * function: DataSourceDQL::DataSourceDQL
 * DataSourceDQL constructor.
 */
DataSourceDQL::DataSourceDQL(char *query, char *name) : DataSource(name)
{
    DO_DEBUG(printf("DataSourceDQL::DataSourceDQL(%s,%s,%s)\n",
                    url, cache, (label != NULL) ? label : "null"));
	
	_query = _name = 0;
    if (query)
		_query = strdup(query);
	if (name)
		_name = strdup(name);
}

/*------------------------------------------------------------------------------
 * function: DataSourceDQL::~DataSourceDQL
 * DataSourceDQL destructor.
 */
DataSourceDQL::~DataSourceDQL()
{
  delete _query;
  delete _name;
}

/*------------------------------------------------------------------------------
 * function: DataSourceDQL::Open
 * Open DQL data source.
 */
DevStatus
DataSourceDQL::Open(char *mode)
{
    return StatusOk;
}

/*------------------------------------------------------------------------------
 * function: DataSourceDQL::Close
 * Do a close() on the file descriptor.
 */
DevStatus
DataSourceDQL::Close()
{
    DO_DEBUG(printf("DataSourceDQL::Close()\n"));
	return StatusOk;
}

/*------------------------------------------------------------------------------
 * function: DataSourceDQL::ChildProc
 * Child process for fetching data from DQL source.
 */
DevStatus
DataSourceDQL::ChildProc()
{
	return StatusOk;
}

/*------------------------------------------------------------------------------
 * function: DataSourceDQL::Fwrite
 * Do fwrite() on the stream associated with this object.
 */
size_t
DataSourceDQL::Fwrite(const char *buf, size_t size, size_t itemCount)
{
    DO_DEBUG(printf("DataSourceDQL::Fwrite()\n"));

    reportError("writing to DQL data source not supported", EINVAL);
    return 0;
}

/*------------------------------------------------------------------------------
 * function: DataSourceDQL::Write
 * Do write() on the stream associated with this object.
 */
size_t
DataSourceDQL::Write(const char *buf, size_t byteCount)
{
    DO_DEBUG(printf("DataSourceDQL::Write()\n"));

    reportError("writing to DQL data source not supported", EINVAL);
    return 0;
}

/*------------------------------------------------------------------------------
 * function: DataSourceDQL::append
 * Append the given record to the end of the file associated with this
 * object.
 */
int
DataSourceDQL::append(void *buf, int recSize)
{
    DO_DEBUG(printf("DataSourceDQL::append()\n"));
    int result = 0;

    reportError("writing to DQL data source not supported", EINVAL);
    return -1;
}

/*============================================================================*/
