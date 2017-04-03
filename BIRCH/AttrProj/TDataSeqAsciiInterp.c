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
  $Id: TDataSeqAsciiInterp.c 2396 2014-03-13 21:13:27Z wkliao $

  $Log: TDataSeqAsciiInterp.c,v $
  Revision 1.2  1996/11/22 20:41:16  flisakow
  Made variants of the TDataAscii classes for sequential access,
  which build no indexes.

  ReadRec() method now returns a status instead of void for every
  class that has the method.

*/

#include <string.h>
#include <ctype.h>
#include <unistd.h>

#include "TDataSeqAsciiInterp.h"
#include "AttrList.h"
#include "RecInterp.h"
#include "CompositeParser.h"
#include "Parse.h"
#include "Control.h"
#include "Util.h"
#include "DevError.h"

//#define DEBUG

TDataSeqAsciiInterp::TDataSeqAsciiInterp(char *name, char *type,
                                   char *param, int recSize,
				   AttrList *attrs, char *separators,
				   int numSeparators, Boolean isSeparator,
				   char *commentString) :
     TDataSeqAscii(name, type, param, recSize), _attrList(*attrs)
{
#ifdef DEBUG
//  printf("TDataSeqAsciiInterp %s, recSize %d\n", name, recSize);
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

TDataSeqAsciiInterp::~TDataSeqAsciiInterp()
{
}

void TDataSeqAsciiInterp::InvalidateIndex()
{
  for(int i = 0; i < _attrList.NumAttrs(); i++) {
      AttrInfo *info = _attrList.Get(i);
      info->hasHiVal = false;
      info->hasLoVal = false;
  }
}

Boolean TDataSeqAsciiInterp::WriteIndex(int fd)
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

Boolean TDataSeqAsciiInterp::ReadIndex(int fd)
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

Boolean TDataSeqAsciiInterp::Decode(void *recordBuf, int recPos, char *line)
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
//	   numArgs, _numPhysAttrs);
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
//	     info->loVal.intVal);
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
	//     info->hiVal.floatVal, info->loVal.floatVal);
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
//	     info->hiVal.doubleVal, info->loVal.doubleVal);
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
	//     info->loVal.dateVal);
#endif
      break;

    default:
      DOASSERT(0, "Unknown attribute type");
    }
  }
  
  return true;
}
