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
  $Id: RecInterp.h 2396 2014-03-13 21:13:27Z wkliao $

  $Log: RecInterp.h,v $
  Revision 1.5  1996/05/07 16:37:15  jussi
  Added _recPos variable and methods for it.

  Revision 1.4  1996/01/19 20:03:19  jussi
  Remove redundant Print().

  Revision 1.3  1995/12/28 20:47:00  jussi
  Added copyright notice and cleaned up the code a bit.

  Revision 1.2  1995/09/05 22:15:31  jussi
  Added CVS header.
*/

#ifndef RecInterp_h
#define RecInterp_h

#include <sys/types.h>
#include <sys/time.h>

#include "DeviseTypes.h"

class AttrList;
class AttrInfo;

class RecInterp {
public:
  RecInterp();

  void SetBuf(void *buf);
  void *GetBuf() { return _buf; }

  void SetRecPos(int pos) { _recPos = pos; }
  int  GetRecPos() { return _recPos; }

  void SetAttrs(AttrList *attrs);

  /* Get the address of attributes, or NULL if not found */
  char *GetString(char *attrName);
  double *GetFloat(char *attrName);
  int *GetInt(char *attrName);
  time_t *GetDate(char *attrName);
  
  /* Get attribute info, or NULL */
  AttrInfo *GetAttrInfo(char *attrName);

  /* print Attr names as heading */
  void PrintAttrHeading(); 
  
  /* Print info about ith attribute into buffer. */
  void PrintAttr(char *buf, int attrNum, Boolean printAttrName = false);

private:
  AttrList *_attrs;                     /* attributes in record */
  void     *_buf;                       /* record buffer */
  int      _recPos;                     /* position of record */
};

#endif
