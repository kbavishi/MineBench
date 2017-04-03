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
  $Id: AttrList.h 2396 2014-03-13 21:13:27Z wkliao $

  $Log: AttrList.h,v $
  Revision 1.8  1996/11/23 21:22:46  jussi
  Removed Config.h. Includes Init.h or ApInit.h instead.

  Revision 1.7  1996/07/23 20:12:41  wenger
  Preliminary version of code to save TData (schema(s) and data) to a file.

  Revision 1.6  1996/05/07 16:14:19  jussi
  Added copy constructor and GetVal() method.

  Revision 1.5  1996/04/19 19:06:35  wenger
  Added DEVise copyright notice, etc.

  Revision 1.4  1995/12/20 07:04:28  ravim
  High and low values of attrs can be specified.

  Revision 1.3  1995/09/05 21:12:23  jussi
  Added/updated CVS header.

  Revision 1.2  1995/09/05 20:39:21  jussi
  Added CVS header.
*/

#ifndef AttrList_h
#define AttrList_h

#include <sys/time.h>

#include "DeviseTypes.h"
#include "DList.h"

#ifdef ATTRPROJ
#   include "ApInit.h"
#else
#   include "Init.h"
#endif

enum AttrType { IntAttr, FloatAttr, DoubleAttr, StringAttr, DateAttr };

union AttrVal {
	double doubleVal;          /* double value */
	float floatVal;            /* float value */
	int intVal;                /* integer value */
	char *strVal;              /* string value */
	time_t dateVal;            /* date value */
};

struct AttrInfo {
	char *name;                /* name of attribute */
	int attrNum;               /* attribute number, starting from 0 */
	int offset;                /* offset from beginning of record */
	int length;                /* max length of attribute */
	Boolean isComposite;       /* true if this attribute is a composite
				      requiring user defined parser. */
	Boolean isSorted;          /* true if this attirubte is the
				      sort attribute */
	AttrType type;             /* attribute type */
	Boolean hasMatchVal;       /* true if matching value specified */
	AttrVal matchVal;          /* matching value */
	Boolean hasHiVal;          /* true if high value specified */
	AttrVal hiVal;             /* high value */
	Boolean hasLoVal;          /* true if low value specified */
	AttrVal loVal;             /* low value */
};

const int MAX_ATTRLIST_SIZE = DEVISE_MAX_TDATA_ATTRS;

class AttrList {
public:
  AttrList(char *name);
  ~AttrList();

  /* Copy constructor */
  AttrList(AttrList &attrs);

  /* Insert attribute into list of attributes */
  void InsertAttr(int attrNum,
		  char *name, int offset, int length, AttrType type,
		  Boolean hasMatchVal = false,
		  AttrVal *matchVal = (AttrVal *)NULL,
		  Boolean isComposite = false,
		  Boolean isSorted = false,
		  Boolean hasHiVal = false, 
		  AttrVal *hiVal = (AttrVal *)NULL, 
		  Boolean hasLoVal = false, 
		  AttrVal *loVal = (AttrVal *)NULL);

  char *GetName() { return _name; }

  /* Find an attribute, or NULL if not found */
  AttrInfo *Find(char *name);

  /* Get ith attribute info */
  AttrInfo *Get(int index);

  /* Get # of attributes */
  int NumAttrs() { return _size;}

  /* iterator for list of attributes */
  void InitIterator();
  Boolean More();
  AttrInfo *Next();
  void DoneIterator();

  void Print();
  void Write(int fd);

  static double GetVal(AttrVal *aval, AttrType atype);

private:
  AttrInfo *_attrs[MAX_ATTRLIST_SIZE];
  int   _size;
  int   _index;
  char *_name;

  static void PrintVal(AttrVal *aval, AttrType atype);
};

#endif
