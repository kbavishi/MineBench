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
  $Id: TDataAsciiInterp.c 2396 2014-03-13 21:13:27Z wkliao $

  $Log: TDataAsciiInterp.c,v $
  Revision 1.24  1996/11/01 19:28:23  kmurli
  Added DQL sources to include access to TDataDQL. This is equivalent to
  TDataAscii/TDataBinary. The DQL type in the Tcl/Tk corresponds to this
  class.

  Revision 1.23  1996/10/07 22:54:00  wenger
  Added more error checking and better error messages in response to
  some of the problems uncovered by CS 737 students.

  Revision 1.22  1996/10/02 15:23:51  wenger
  Improved error handling (modified a number of places in the code to use
  the DevError class).

  Revision 1.21  1996/07/21 02:23:23  jussi
  Added code that deletes allocated string space if string not
  inserted into string hash table.

  Revision 1.20  1996/07/15 17:02:06  jussi
  Added support for string attributes in GData.

  Revision 1.19  1996/07/01 19:28:07  jussi
  Added support for typed data sources (WWW and UNIXFILE). Renamed
  'cache' references to 'index' (cache file is really an index).
  Added support for asynchronous interface to data sources.

  Revision 1.18  1996/06/27 18:12:40  wenger
  Re-integrated most of the attribute projection code (most importantly,
  all of the TData code) into the main code base (reduced the number of
  modules used only in attribute projection).

  Revision 1.17  1996/06/27 15:49:32  jussi
  TDataAscii and TDataBinary now recognize when a file has been deleted,
  shrunk, or has increased in size. The query processor is asked to
  re-issue relevant queries when such events occur.

  Revision 1.16  1996/05/11 03:14:40  jussi
  Made this code independent of some control panel variables like
  _fileAlias and _fileName.

  Revision 1.15  1996/05/07 16:46:00  jussi
  This class now makes a copy of the attribute list so that attribute
  hi/lo values can be maintained per data stream, not per schema.
  Hi/lo values are now computed after composite parser is executed.

  Revision 1.14  1996/05/05 03:07:33  jussi
  Removed array of pointers to attribute info for matching values.

  Revision 1.13  1996/04/23 15:40:45  jussi
  Added debugging statements and allowed decoder to accept integer
  values with +/- prefixes.

  Revision 1.12  1996/04/19 17:21:21  wenger
  Put the GenClassInfo code back in -- this is needed for tape data;
  started adding the tape-related code back in (it was previously
  deleted for some reason; I'm not yet done adding it back); added
  the 'DEVise parseSchema' command and the first parts of the code
  related to it.

  Revision 1.11  1996/04/16 20:38:51  jussi
  Replaced assert() calls with DOASSERT macro.

  Revision 1.10  1996/03/29 18:14:20  wenger
  Got testWindowRep to compile and run, added drawing in
  windows; fixed a few more compile warnings, etc.

  Revision 1.9  1996/03/26 21:33:00  jussi
  Added computation of max/min attribute values.

  Revision 1.8  1996/02/01 18:28:59  jussi
  Improved handling of case where data file has more attributes
  than schema defined.

  Revision 1.7  1996/01/26 19:45:57  jussi
  Improved checking of data record validity.

  Revision 1.6  1995/12/28 19:48:13  jussi
  Small fixes to remove compiler warnings.

  Revision 1.5  1995/11/22 17:51:01  jussi
  Added missing DoubleAttr case in Decode().

  Revision 1.4  1995/11/22 17:04:14  jussi
  Added IsValid() method and cleaned up the code.

  Revision 1.3  1995/11/21 23:20:49  jussi
  Added copyright notice and cleaned up code.

  Revision 1.2  1995/09/05 22:15:51  jussi
  Added CVS header.
*/

#include <string.h>
#include <ctype.h>
#include <unistd.h>

#include "TDataAsciiInterp.h"
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

//#define DEBUG

#ifndef ATTRPROJ
TDataAsciiInterpClassInfo::TDataAsciiInterpClassInfo(char *className,
						     AttrList *attrList,
						     int recSize,
						     char *separators,
						     int numSeparators,
						     Boolean isSeparator,
						     char *commentString)
{
/* Note that this only saves a pointer to the attrList; it doesn't copy it. */
  _className = className;
  _attrList = attrList;
  _separators = separators;
  _numSeparators = numSeparators;
  _commentString = commentString;
  _recSize = recSize;
  _isSeparator = isSeparator;
  _tdata = NULL;
}

TDataAsciiInterpClassInfo::TDataAsciiInterpClassInfo(char *className,
						     char *name, char *type, char *param,
						     TData *tdata)
{
  _className = className;
  _name = name;
  _type = type;
  _param = param;
  _tdata = tdata;
}

TDataAsciiInterpClassInfo::~TDataAsciiInterpClassInfo()
{
  if (_tdata)
    delete _tdata;
}

char *TDataAsciiInterpClassInfo::ClassName()
{
  return _className;
}

static char buf[3][256];
static char *args[3];

void TDataAsciiInterpClassInfo::ParamNames(int &argc, char **&argv)
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

ClassInfo *TDataAsciiInterpClassInfo::CreateWithParams(int argc, char **argv)
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

  TDataAsciiInterp *tdata = new TDataAsciiInterp(name, type,
                                                 param, _recSize,
                                                 _attrList, _separators,
                                                 _numSeparators,
                                                 _isSeparator,
                                                 _commentString);
  return new TDataAsciiInterpClassInfo(_className, name, type, param, tdata);
}

char *TDataAsciiInterpClassInfo::InstanceName()
{
  return _name;
}

void *TDataAsciiInterpClassInfo::GetInstance()
{
  return _tdata;
}

/* Get parameters that can be used to re-create this instance */
void TDataAsciiInterpClassInfo::CreateParams(int &argc, char **&argv)
{
  argc = 3;
  argv = args;
  args[0] = _name;
  args[1] = _type;
  args[2] = _param;
}
#endif

TDataAsciiInterp::TDataAsciiInterp(char *name, char *type,
                                   char *param, int recSize,
				   AttrList *attrs, char *separators,
				   int numSeparators, Boolean isSeparator,
				   char *commentString) :
     TDataAscii(name, type, param, recSize), _attrList(*attrs)
{
#ifdef DEBUG
 // printf("TDataAsciiInterp %s, recSize %d\n", name, recSize);
#endif

  _recInterp = new RecInterp();
  _recInterp->SetAttrs(attrs);
  
  _recSize = recSize;
  _separators = separators;
  _numSeparators = numSeparators;
  _isSeparator = isSeparator;

  _commentString = commentString;
  if (_commentString)
    _commentStringLength = strlen(_commentString);
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

TDataAsciiInterp::~TDataAsciiInterp()
{
}

void TDataAsciiInterp::InvalidateIndex()
{
  for(int i = 0; i < _attrList.NumAttrs(); i++) {
      AttrInfo *info = _attrList.Get(i);
      info->hasHiVal = false;
      info->hasLoVal = false;
  }
}

Boolean TDataAsciiInterp::WriteIndex(int fd)
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

Boolean TDataAsciiInterp::ReadIndex(int fd)
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

Boolean TDataAsciiInterp::Decode(void *recordBuf, int recPos, char *line)
{
  /* set buffer for interpreted record */
  _recInterp->SetBuf(recordBuf);
  _recInterp->SetRecPos(recPos);

  int numArgs;
  char **args;

  Parse(line, numArgs, args, _separators, _numSeparators, _isSeparator);
  
  if (numArgs < _numPhysAttrs || 
      (_commentString != NULL &&
       strncmp(args[0], _commentString, _commentStringLength) == 0)) {
#ifdef DEBUG
    //printf("Too few arguments (%d < %d) or commented line\n",
	   numArgs, _numPhysAttrs);
    //printf("  %s\n", line);
#endif
    return false;
  }
	
  int i;
  for(i = 0; i < _numPhysAttrs; i++) {
    AttrInfo *info = _attrList.Get(i);
    if (info->type == IntAttr || info->type == DateAttr) {
      if (!isdigit(args[i][0]) && args[i][0] != '-' && args[i][0] != '+') {
#ifdef DEBUG
	//printf("Invalid integer/date value: %s\n", args[i]);
#endif
	return false;
      }
    } else if (info->type == FloatAttr || info->type == DoubleAttr) {
      if (!isdigit(args[i][0]) && args[i][0] != '.'
	  && args[i][0] != '-' && args[i][0] != '+') {
#ifdef DEBUG
	//printf("Invalid float/double value: %s\n", args[i]);
#endif
	return false;
      }
    }
  }

  DOASSERT(numArgs >= _numPhysAttrs, "Invalid number of arguments");

  for(i = 0; i < _numPhysAttrs; i++) {

    AttrInfo *info = _attrList.Get(i);
    char *ptr = (char *)recordBuf + info->offset;

    char *string = 0;
    int code = 0;
    int key = 0;

    switch(info->type) {
    case IntAttr:
      *(int *)ptr = atoi(args[i]);
      break;

    case FloatAttr:
      *(float *)ptr = UtilAtof(args[i]);
      break;

    case DoubleAttr:
      *(double *)ptr = UtilAtof(args[i]);
      break;

    case StringAttr:
      strncpy(ptr, args[i], info->length);
      ptr[info->length - 1] = '\0';
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
      break;

    case DateAttr:
      *(time_t *)ptr = atoi(args[i]);
      break;

    default:
      DOASSERT(0, "Unknown attribute type");
    }
  }
  
  /* decode composite attributes */
  if (hasComposite)
    CompositeParser::Decode(_attrList.GetName(), _recInterp);

  for(i = 0; i < _numAttrs; i++) {
    AttrInfo *info = _attrList.Get(i);

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
