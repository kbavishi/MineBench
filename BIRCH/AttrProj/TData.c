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
  $Id: TData.c 2396 2014-03-13 21:13:27Z wkliao $

  $Log: TData.c,v $
  Revision 1.14  1996/12/03 20:32:09  jussi
  Improved Init/Get/Done interface.

  Revision 1.13  1996/11/23 21:16:01  jussi
  Fixed detection of tape drive device.

  Revision 1.12  1996/11/18 18:10:53  donjerko
  New files and changes to make DTE work with Devise

  Revision 1.11  1996/11/01 19:28:23  kmurli
  Added DQL sources to include access to TDataDQL. This is equivalent to
  TDataAscii/TDataBinary. The DQL type in the Tcl/Tk corresponds to this
  class.

  Revision 1.10  1996/10/07 22:53:59  wenger
  Added more error checking and better error messages in response to
  some of the problems uncovered by CS 737 students.

  Revision 1.9  1996/08/23 16:56:20  wenger
  First version that allows the use of Dali to display images (more work
  needs to be done on this); changed DevStatus to a class to make it work
  better; various minor bug fixes.

  Revision 1.8  1996/08/15 19:54:31  wenger
  Added 'pure' targets for attrproj and devread; fixed some dynamic
  memory problems.  Found some bugs while demo'ing for soils science
  people.

  Revision 1.7  1996/08/12 20:49:58  jussi
  Added missing statement that creates a new DataSourceWeb for
  WWW data sources.

  Revision 1.6  1996/08/05 19:48:57  wenger
  Fixed compile errors caused by some of Kevin's recent changes; changed
  the attrproj stuff to make a .a file instead of a .o; added some more
  TData file writing stuff; misc. cleanup.

  Revision 1.5  1996/08/04 21:59:51  beyer
  Added UpdateLinks that allow one view to be told to update by another view.
  Changed TData so that all TData's have a DataSource (for UpdateLinks).
  Changed all of the subclasses of TData to conform.
  A RecFile is now a DataSource.
  Changed the stats buffers in ViewGraph to be DataSources.

  Revision 1.4  1996/07/23 20:13:05  wenger
  Preliminary version of code to save TData (schema(s) and data) to a file.

  Revision 1.3  1996/01/27 00:17:27  jussi
  Added copyright notice and cleaned up a bit.

  Revision 1.2  1995/09/05 22:15:44  jussi
  Added CVS header.
*/

//#define DEBUG

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "TData.h"
#include "Exit.h"
#include "Util.h"
#include "DevFileHeader.h"
#include "DevError.h"
#include "AttrList.h"

#include "DataSourceFileStream.h"
#include "DataSourceSegment.h"
#include "DataSourceTape.h"
#include "DataSourceBuf.h"
#include "DataSourceDQL.h"
#include "DataSeg.h"
#ifndef ATTRPROJ
#include "ViewGraph.h"
#include "QueryProc.h"
#endif

#ifndef ATTRPROJ
#include "DataSourceWeb.h"
#endif

#ifdef ATTRPROJ
#   include "ApInit.h"
#else
#   include "Init.h"
#endif

static DevStatus WriteString(int fd, char *string);

/*---------------------------------------------------------------------------*/
TData::TData(char* name, char* type, char* param, int recSize)
{
    DO_DEBUG(printf("TData::TData(%s, %s, %s, %d)\n",
		    name, type, param, recSize));

    _name = name;
    _type = type;
    _param = param;
    _recSize = recSize;
    _data = NULL;
    _version = 0;

    // Find out whether the data occupies an entire data source or only
    // a segment of it
    char *	segLabel;
    char *	segFile;
    long	segOffset;
    long	segLength;

    DataSeg::Get(segLabel, segFile, segOffset, segLength);

    // Check that data segment label matches TData name

    if (strcmp(_name, segLabel)) {
	cout << (void*) segLabel << " " << segLabel[0] << " " << segLabel[1] << endl; 
	cout << _name << " != " << segLabel << endl;
	DOASSERT(false, "Data segment does not match tdata");
    }

    // Now instantiate the appropriate type of source, according to
    // whether this is a tape, disk file, or Web resource, and whether
    // or not the data occupies the entire file.

    if (!strcmp(_type, "UNIXFILE")) {
	char *file = _param;
	if (strcmp(file, segFile)) {
	    DOASSERT(false, "Data segment does not match tdata");
	}
	if (   !strncmp(file, "/dev/rmt", 8)
	    || !strncmp(file, "/dev/nrmt", 9)
	    || !strncmp(file, "/dev/rst", 8)
	    || !strncmp(file, "/dev/nrst", 9)) {

	    _data = new DataSourceTape(file, NULL);
	} else {
	    _data = new DataSourceFileStream(file, NULL);
	}
    }
	else if (!strcmp(_type,"DQL")){
	  // nothin here..
	  // Instantiate to a data stream..
	    _data = new DataSourceDQL(param, _name);
	}

#if 0
    // buffer stuff not working or used
    else if (!strcmp(_type, "BUFFER")) {
	// For BUFFER data sources, _param is a pointer to the buffer
	DOASSERT(_param, "Invalid buffer data source");
	// need buffer & data sizes
	_data = new DataSourceBuf(_param, buffer_size, data_size, NULL);
    }
#endif

#ifndef ATTRPROJ
	//--------------------------------------------------------------------
	// statistics datasources are stored in their corresponding viewgraph.
	// the viewgraph will delete the source.
    else if (!strcmp(_type, "BASICSTAT")) {
	DOASSERT( strncmp(name, "Stat: ", 6) == 0, "invalid basicstat prefix");
	ViewGraph* v = (ViewGraph *)ControlPanel::FindInstance(name+6);
	DOASSERT(v, "BASICSTAT view not found");
	_data = v->GetViewStatistics();
    } else if (!strcmp(_type, "HISTOGRAM")) {
	DOASSERT( strncmp(name, "Hist: ", 6) == 0, "invalid histogram prefix");
	ViewGraph* v = (ViewGraph *)ControlPanel::FindInstance(name+6);
	DOASSERT(v, "HISTOGRAM view not found");
	_data = v->GetViewHistogram();
	DO_DEBUG(printf("found histogram data source 0x%p from view 0x%p\n",
			_data, v));
    } else if (!strcmp(_type, "GDATASTAT")) {
	DOASSERT( strncmp(name, "Gstat: ", 7) == 0, "invalid gdatastat prefix");
	ViewGraph* v = (ViewGraph *)ControlPanel::FindInstance(name+7);
	DOASSERT(v, "GDATASTAT view not found");
	_data = v->GetGdataStatistics();
    }
    else if (!strcmp(_type, "WWW")) {
	char *file = MakeCacheFileName(_name, _type);
        _data = new DataSourceWeb(param, _name, file);
	delete file;
    }
#endif
    else {
	fprintf(stderr, "Invalid TData type: %s\n", _type);
	DOASSERT(0, "Invalid TData type");
    }

    //TEMPTEMP -- make sure this works right!!
    if ((segOffset != 0) || (segLength != 0)) {
	  _data = new DataSourceSegment(_data, segOffset, segLength);
    }
	
    DOASSERT(_data, "Couldn't find/create data source");
    _data->AddRef();
}

/*---------------------------------------------------------------------------*/
TData::TData(DataSource* data_source)
{
    _name = NULL;
    _type = NULL;
    _param = NULL;
    _recSize = 0;
    _data = data_source;
    _version = 0;
}

/*------------------------------------------------------------------------------
 * function: TData::~TData
 * TData destructor.
 */
TData::~TData()
{
    if( _data && _data->DeleteRef() )
        delete _data;
    delete _param;
    delete _type;
    delete _name;
}

/*------------------------------------------------------------------------------
 * function: TData::WriteRecs
 * For writing records. Default: not implemented.
 */

void TData::WriteRecs(RecId startId, int numRecs, void *buf)
{
  DOASSERT(0, "TData::WriteRecs not implemented");
}

/*------------------------------------------------------------------------------
 * function: TData::Save
 * Save this TData (schema(s) and actual data) to the given file.
 */
DevStatus
TData::Save(char *filename)
{
  DO_DEBUG(printf("TData::Save(%s)\n", filename));

  DevStatus result = StatusOk;

  int fd = open(filename, O_CREAT | O_WRONLY | O_TRUNC, 0644);
  if (fd == -1)
  {
    reportError("Can't open file", errno);
    result = StatusFailed;
  }
  else
  {
    if (WriteHeader(fd) == StatusFailed) result = StatusFailed;
    if (WriteLogSchema(fd) == StatusFailed) result = StatusFailed;
    if (WritePhysSchema(fd) == StatusFailed) result = StatusFailed;
    if (WriteData(fd) == StatusFailed) result = StatusFailed;

    if (close(fd) == -1)
    {
      reportError("Can't close file", errno);
      result = StatusFailed;
    }
  }

  return result;
}

/*------------------------------------------------------------------------------
 * function: TData::WriteHeader
 * Write the appropriate file header to the given file descriptor.
 */
DevStatus
TData::WriteHeader(int fd)
{
  DevStatus result = StatusOk;

  result += WriteString(fd, DevFileHeader::Get(FILE_TYPE_TDATA));

  return result;
}

/*------------------------------------------------------------------------------
 * function: TData::WriteLogSchema
 * Write the logical schema, if any, to the given file descriptor.
 */
DevStatus
TData::WriteLogSchema(int fd)
{
  DevStatus result = StatusOk;

  (void)/*TEMPTEMP*/WriteString(fd, "\nstartSchema logical\n");





  (void)/*TEMPTEMP*/WriteString(fd, "endSchema\n");

  return result;
}

/*------------------------------------------------------------------------------
 * function: TData::WritePhysSchema
 * Write the physical schema to the given file descriptor.
 */
DevStatus
TData::WritePhysSchema(int fd)
{
  DevStatus result = StatusOk;

  (void)/*TEMPTEMP*/WriteString(fd, "\nstartSchema physical\n");

  (void)/*TEMPTEMP*/ WriteString(fd, "type <name> ascii|binary\n"/*TEMPTEMP*/);

  (void)/*TEMPTEMP*/ WriteString(fd, "comment //\nseparator ','\n"/*TEMPTEMP*/);

  AttrList *attrListP = GetAttrList();
  if (attrListP == NULL)
  {
    reportError("Error writing schema", errno);
    result = StatusFailed;
  }
  else
  {
    attrListP->Write(fd);
  }

  (void)/*TEMPTEMP*/WriteString(fd, "endSchema\n");

  return result;
}

/*------------------------------------------------------------------------------
 * function: TData::WriteData
 * Write the actual data to the given file descriptor.
 */
DevStatus
TData::WriteData(int fd)
{
  DevStatus result = StatusOk;

  (void)/*TEMPTEMP*/ WriteString(fd, "startData\n");

  return result;
}

/*------------------------------------------------------------------------------
 * function: WriteString
 * TEMPTEMP
 */
static DevStatus
WriteString(int fd, char *string)
{
  DevStatus result = StatusOk;

  int stringLen = strlen(string);
  if (write(fd, string, stringLen) != stringLen)
  {
    reportError("Error writing to file", errno);
    result = StatusFailed;
  }

  return result;
}

//---------------------------------------------------------------------------
void TData::InvalidateTData()
{
    DO_DEBUG(printf("invaliding tdata version %d for %d\n",
		    _version, _data->Version()));
#ifndef ATTRPROJ
    QueryProc::Instance()->ClearTData(this);
#endif
    _version = _data->Version();
}

//---------------------------------------------------------------------------

char* TData::MakeCacheFileName(char *name, char *type)
{
  char *fname = StripPath(name);
  char *cacheDir = Init::CacheDir();
  int nameLen = strlen(cacheDir) + 1 + strlen(fname) + 1 + strlen(type) + 1;
  char *fn = new char [nameLen];
  sprintf(fn, "%s/%s.%s", cacheDir, fname, type);
  return fn;
}

//===========================================================================
