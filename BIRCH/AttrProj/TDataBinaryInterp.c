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
  $Id: TDataBinaryInterp.c 2396 2014-03-13 21:13:27Z wkliao $

  $Log: TDataBinaryInterp.c,v $
  Revision 1.11  1996/10/02 15:23:52  wenger
  Improved error handling (modified a number of places in the code to use
  the DevError class).

  Revision 1.10  1996/08/29 18:24:42  wenger
  A number of Dali-related improvements: ShapeAttr1 now specifies image
  type when shape is 'image'; added new '-bytes' flag to Dali commands
  when sending images; TDataBinaryInterp now uses StringStorage so GData
  can access strings; fixed hash function for StringStorage so having the
  high bit set in a byte in the string doesn't crash the hash table;
  improved the error checking in some of the Dali code.

  Revision 1.9  1996/07/01 19:28:10  jussi
  Added support for typed data sources (WWW and UNIXFILE). Renamed
  'cache' references to 'index' (cache file is really an index).
  Added support for asynchronous interface to data sources.

  Revision 1.8  1996/06/27 18:12:43  wenger
  Re-integrated most of the attribute projection code (most importantly,
  all of the TData code) into the main code base (reduced the number of
  modules used only in attribute projection).

  Revision 1.7  1996/06/27 15:49:35  jussi
  TDataAscii and TDataBinary now recognize when a file has been deleted,
  shrunk, or has increased in size. The query processor is asked to
  re-issue relevant queries when such events occur.

  Revision 1.6  1996/05/11 03:14:52  jussi
  Made this code independent of some control panel variables like
  _fileAlias and _fileName.

  Revision 1.5  1996/05/07 16:46:20  jussi
  This class now makes a copy of the attribute list so that attribute
  hi/lo values can be maintained per data stream, not per schema.
  Hi/lo values are now computed after composite parser is executed.

  Revision 1.4  1996/05/05 03:08:23  jussi
  Added support for composite attributes. Also added tape drive
  support.

  Revision 1.3  1996/04/16 20:38:52  jussi
  Replaced assert() calls with DOASSERT macro.

  Revision 1.2  1996/02/01 18:28:55  jussi
  Improved handling of case where data file has more attributes
  than schema defined.

  Revision 1.1  1996/01/23 20:54:51  jussi
  Initial revision.
*/

#include <string.h>
#include <unistd.h>

#include "TDataBinaryInterp.h"
#include "AttrList.h"
#include "RecInterp.h"
#include "CompositeParser.h"
#include "Parse.h"
#include "Control.h"
#include "Util.h"
#include "DevError.h"
#ifndef ATTRPROJ
#  include "StringStorage.h"
#endif

#ifndef ATTRPROJ
TDataBinaryInterpClassInfo::TDataBinaryInterpClassInfo(char *className,
						       AttrList *attrList,
						       int recSize)
{
  _className = className;
  _attrList = attrList;
  _recSize = recSize;
  _tdata = NULL;

  // compute size of physical record (excluding composite attributes)

  _physRecSize = 0;
  _attrList->InitIterator();
  while(_attrList->More()) {
    AttrInfo *info = _attrList->Next();
    if (!info->isComposite)
      _physRecSize += info->length;
  }
  _attrList->DoneIterator();

  DOASSERT(_physRecSize > 0 && _physRecSize <= _recSize,
	   "Invalid physical record size");
}

TDataBinaryInterpClassInfo::TDataBinaryInterpClassInfo(char *className,
						       char *name,
						       char *type,
                                                       char *param,
						       TData *tdata)
{
  _className = className;
  _name = name;
  _type = type;
  _param = param;
  _tdata = tdata;
}

TDataBinaryInterpClassInfo::~TDataBinaryInterpClassInfo()
{
  if (_tdata)
    delete _tdata;
}

char *TDataBinaryInterpClassInfo::ClassName()
{
  return _className;
}

static char buf[3][256];
static char *args[3];

void TDataBinaryInterpClassInfo::ParamNames(int &argc, char **&argv)
{
  argc = 3;
  argv = args;
  args[0] = buf[0];
  args[1] = buf[1];
  args[2] = buf[2];
  
  strcpy(buf[0], "Name {foobar}");
  strcpy(buf[1], "Type {foobar}");
  strcpy(buf[2], "Param {foobar}");
}

ClassInfo *TDataBinaryInterpClassInfo::CreateWithParams(int argc, char **argv)
{
  if (argc != 2 && argc != 3)
    return (ClassInfo *)NULL;

  char *name, *type, *param;

  if (argc == 2) {
    name = CopyString(argv[1]);
    type = CopyString("UNIXFILE");
    param = CopyString(argv[0]);
  } else {
    name = CopyString(argv[0]);
    type = CopyString(argv[1]);
    param = CopyString(argv[2]);
  }

  TDataBinaryInterp *tdata = new TDataBinaryInterp(name, type,
                                                   param, _recSize,
                                                   _physRecSize,
                                                   _attrList);
  return new TDataBinaryInterpClassInfo(_className, name, type, param, tdata);
}

char *TDataBinaryInterpClassInfo::InstanceName()
{
  return _name;
}

void *TDataBinaryInterpClassInfo::GetInstance()
{
  return _tdata;
}

void TDataBinaryInterpClassInfo::CreateParams(int &argc, char **&argv)
{
  argc = 3;
  argv = args;
  args[0] = _name;
  args[1] = _type;
  args[2] = _param;
}
#endif

TDataBinaryInterp::TDataBinaryInterp(char *name, char *type,
                                     char *param, int recSize,
				     int physRecSize, AttrList *attrs) :
     TDataBinary(name, type, param, recSize, physRecSize), _attrList(*attrs)
{
#ifdef DEBUG
  //printf("TDataBinaryInterp %s, recSize %d, physRecSize %d\n",
	 name, recSize, physRecSize);
#endif

  _recInterp = new RecInterp();
  _recInterp->SetAttrs(attrs);
  
  _recSize = recSize;
  _physRecSize = physRecSize;
  _numAttrs = _numPhysAttrs = _attrList.NumAttrs();
  
  hasComposite = false;

  _attrList.InitIterator();
  while(_attrList.More()) {
    AttrInfo *info = _attrList.Next();
    if (info->isComposite) {
      hasComposite = true;
      _numPhysAttrs--;
    }
  }
  _attrList.DoneIterator();

  Initialize();
}

TDataBinaryInterp::~TDataBinaryInterp()
{
}

void TDataBinaryInterp::InvalidateIndex()
{
  for(int i = 0; i < _attrList.NumAttrs(); i++) {
      AttrInfo *info = _attrList.Get(i);
      info->hasHiVal = false;
      info->hasLoVal = false;
  }
}

Boolean TDataBinaryInterp::WriteIndex(int fd)
{
  int numAttrs = _attrList.NumAttrs();
  if (write(fd, &numAttrs, sizeof numAttrs) != sizeof numAttrs) {
    reportErrSys("write");
    return false;
  }

  for(int i = 0; i < _attrList.NumAttrs(); i++) {
    AttrInfo *info = _attrList.Get(i);
    if (info->type == StringAttr)
      continue;
    if (write(fd, &info->hasHiVal, sizeof info->hasHiVal)
	!= sizeof info->hasHiVal) {
      reportErrSys("write");
      return false;
    }
    if (write(fd, &info->hiVal, sizeof info->hiVal) != sizeof info->hiVal) {
      reportErrSys("write");
      return false;
    }
    if (write(fd, &info->hasLoVal, sizeof info->hasLoVal)
	!= sizeof info->hasLoVal) {
      reportErrSys("write");
      return false;
    }
    if (write(fd, &info->loVal, sizeof info->loVal) != sizeof info->loVal) {
      reportErrSys("write");
      return false;
    }
  }

  return true;
}

Boolean TDataBinaryInterp::ReadIndex(int fd)
{
  int numAttrs;
  if (read(fd, &numAttrs, sizeof numAttrs) != sizeof numAttrs) {
    reportErrSys("read");
    return false;
  }
  if (numAttrs != _attrList.NumAttrs()) {
    //printf("Index has inconsistent schema; rebuilding\n");
    return false;
  }

  for(int i = 0; i < _attrList.NumAttrs(); i++) {
    AttrInfo *info = _attrList.Get(i);
    if (info->type == StringAttr)
      continue;
    if (read(fd, &info->hasHiVal, sizeof info->hasHiVal)
	!= sizeof info->hasHiVal) {
      reportErrSys("read");
      return false;
    }
    if (read(fd, &info->hiVal, sizeof info->hiVal) != sizeof info->hiVal) {
      reportErrSys("read");
      return false;
    }
    if (read(fd, &info->hasLoVal, sizeof info->hasLoVal)
	!= sizeof info->hasLoVal) {
      reportErrSys("read");
      return false;
    }
    if (read(fd, &info->loVal, sizeof info->loVal) != sizeof info->loVal) {
      reportErrSys("read");
      return false;
    }
  }

  return true;
}

Boolean TDataBinaryInterp::Decode(void *recordBuf, int recPos, char *line)
{
  /* set buffer for interpreted record */
  _recInterp->SetBuf(recordBuf);
  _recInterp->SetRecPos(recPos);

  if (recordBuf != line)
    memcpy(recordBuf, line, _physRecSize);

  /* decode composite attributes */
  if (hasComposite)
    CompositeParser::Decode(_attrList.GetName(), _recInterp);

  for(int i = 0; i < _numAttrs; i++) {
    AttrInfo *info = _attrList.Get(i);

    char *string = NULL;
    int code = 0;
    int key = 0;

    char *ptr = (char *)recordBuf + info->offset;
    int intVal;
    float floatVal;
    double doubleVal;
    time_t dateVal;

    switch(info->type) {
    case IntAttr:
      intVal = *(int *)ptr;
      if (info->hasMatchVal && intVal != info->matchVal.intVal)
	return false;
      if (!info->hasHiVal || intVal > info->hiVal.intVal) {
	info->hiVal.intVal = intVal;
	info->hasHiVal = true;
      }
      if (!info->hasLoVal || intVal < info->loVal.intVal) {
	info->loVal.intVal = intVal;
	info->hasLoVal = true;
      }
#ifdef DEBUG
      //printf("int %d, hi %d, lo %d\n", intVal, info->hiVal.intVal,
	     info->loVal.intVal);
#endif
      break;

    case FloatAttr:
      floatVal = *(float *)ptr;
      if (info->hasMatchVal && floatVal != info->matchVal.floatVal)
	return false;
      if (!info->hasHiVal || floatVal > info->hiVal.floatVal) {
	info->hiVal.floatVal = floatVal;
	info->hasHiVal = true;
      }
      if (!info->hasLoVal || floatVal < info->loVal.floatVal) {
	info->loVal.floatVal = floatVal;
	info->hasLoVal = true;
      }
#ifdef DEBUG
      //printf("float %.2f, hi %.2f, lo %.2f\n", floatVal,
	     info->hiVal.floatVal, info->loVal.floatVal);
#endif
      break;

    case DoubleAttr:
      doubleVal = *(double *)ptr;
      if (info->hasMatchVal && doubleVal != info->matchVal.doubleVal)
	return false;
      if (!info->hasHiVal || doubleVal > info->hiVal.doubleVal) {
	info->hiVal.doubleVal = doubleVal;
	info->hasHiVal = true;
      }
      if (!info->hasLoVal || doubleVal < info->loVal.doubleVal) {
	info->loVal.doubleVal = doubleVal;
	info->hasLoVal = true;
      }
#ifdef DEBUG
      //printf("double %.2f, hi %.2f, lo %.2f\n", doubleVal,
	     info->hiVal.doubleVal, info->loVal.doubleVal);
#endif
      break;

    case StringAttr:
#ifndef ATTRPROJ
      string = CopyString(ptr);
      code = StringStorage::Insert(string, key);
#ifdef DEBUG
      //printf("Inserted \"%s\" with key %d, code %d\n", ptr, key, code);
#endif
      DOASSERT(code >= 0, "Cannot insert string");
      if (!code)
        delete string;
#endif
      if (info->hasMatchVal && strcmp(ptr, info->matchVal.strVal))
	return false;
      break;

    case DateAttr:
      dateVal = *(time_t *)ptr;
      if (info->hasMatchVal && dateVal != info->matchVal.dateVal)
	return false;
      if (!info->hasHiVal || dateVal > info->hiVal.dateVal) {
	info->hiVal.dateVal = dateVal;
	info->hasHiVal = true;
      }
      if (!info->hasLoVal || dateVal < info->loVal.dateVal) {
	info->loVal.dateVal = dateVal;
	info->hasLoVal = true;
      }
#ifdef DEBUG
      //printf("date %ld, hi %ld, lo %ld\n", dateVal, info->hiVal.dateVal,
	     info->loVal.dateVal);
#endif
      break;

    default:
      DOASSERT(0, "Unknown attribute type");
    }
  }
  
  return true;
}
