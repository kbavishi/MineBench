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
  $Id: RecInterp.c 2396 2014-03-13 21:13:27Z wkliao $

  $Log: RecInterp.c,v $
  Revision 1.6  1996/05/07 16:37:14  jussi
  Added _recPos variable and methods for it.

  Revision 1.5  1996/01/19 20:03:17  jussi
  Remove redundant Print().

  Revision 1.4  1995/12/28 19:46:47  jussi
  Small fixes to remove compiler warnings.

  Revision 1.3  1995/12/14 17:53:12  jussi
  Small fixes to get rid of g++ -Wall warnings.

  Revision 1.2  1995/09/05 22:15:30  jussi
  Added CVS header.
*/

#include <stdio.h>
#include <time.h>

#include "AttrList.h"
#include "RecInterp.h"
#include "Util.h"

RecInterp::RecInterp()
{
  _attrs = NULL;
  _buf = NULL;
  _recPos = 0;
}

void RecInterp::SetBuf(void *buf)
{
  _buf = buf;
}

void RecInterp::SetAttrs(AttrList *attrs)
{
  _attrs = attrs;
}

char *RecInterp::GetString(char *attrName)
{
  AttrInfo *info;
  if (_attrs == NULL || _buf == NULL)
    return NULL;
  if ((info = _attrs->Find(attrName)) == NULL)
    return NULL;
  return ((char *)_buf) + info->offset;
}

double *RecInterp::GetFloat(char *attrName)
{
  AttrInfo *info;
  if (_attrs == NULL || _buf == NULL)
    return NULL;
  if ((info = _attrs->Find(attrName)) == NULL)
    return NULL;
  return (double *)(((char *)_buf) + info->offset);
}

int *RecInterp::GetInt(char *attrName)
{
  AttrInfo *info;
  if (_attrs == NULL || _buf == NULL)
    return NULL;
  if ((info = _attrs->Find(attrName)) == NULL)
    return NULL;
  return (int *)(((char *)_buf) + info->offset);
}

time_t *RecInterp::GetDate(char *attrName)
{
  AttrInfo *info;
  if (_attrs == NULL || _buf == NULL)
    return NULL;
  if ((info = _attrs->Find(attrName)) == NULL)
    return NULL;
  return (time_t *)(((char *)_buf) + info->offset);
}

AttrInfo *RecInterp::GetAttrInfo(char *attrName)
{
  if (_attrs == NULL )
    return NULL;
  return _attrs->Find(attrName);
}

void RecInterp::PrintAttrHeading()
{
  if (_attrs == NULL )
    return;
  
  int num = _attrs->NumAttrs();
  for(int i = 0; i < num; i++) {
    AttrInfo *info = _attrs->Get(i);
//    printf("%s: ", info->name);
  }
}

void RecInterp::PrintAttr(char *buf, int attrNum, Boolean printAttrName)
{
  if (_attrs == NULL || _buf == NULL) {
    buf[0] = '\0';
    return;
  }
  
  AttrInfo *info = _attrs->Get(attrNum);
  int *intVal;
  float *floatVal;
  double *doubleVal;
  char *strVal;
  time_t *tm;

  switch(info->type) {
  case IntAttr:
    intVal = (int *)(((char *)_buf) + info->offset);
    if (printAttrName)
      sprintf(buf, "%s: %d", info->name, *intVal);
    else
      sprintf(buf, "%d ", *intVal);
    break;

  case FloatAttr:
    floatVal = (float *)(((char *)_buf) + info->offset);
    if (printAttrName)
      sprintf(buf, "%s: %.2f", info->name, *floatVal);
    else
      sprintf(buf, "%f", *floatVal);
    break;

  case DoubleAttr:
    doubleVal = (double *)(((char *)_buf) + info->offset);
    if (printAttrName)
      sprintf(buf, "%s: %.2f", info->name, *doubleVal);
    else
      sprintf(buf, "%f", *doubleVal);
    break;

  case StringAttr:
    strVal = ((char *)_buf) + info->offset;
    if (printAttrName)
      sprintf(buf, "%s: %s", info->name, strVal);
    else
      sprintf(buf, "%s", strVal);
    break;

  case DateAttr:
    tm = (time_t *)(((char *)_buf) + info->offset);
    if (printAttrName)
      sprintf(buf, "%s: '%s'", info->name, DateString(*tm));
    else
      sprintf(buf, "'%s'", DateString(*tm));
    break;
  }
}
