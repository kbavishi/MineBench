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
  $Id: TDataAsciiInterp.h 2396 2014-03-13 21:13:27Z wkliao $

  $Log: TDataAsciiInterp.h,v $
  Revision 1.10  1996/07/01 19:28:08  jussi
  Added support for typed data sources (WWW and UNIXFILE). Renamed
  'cache' references to 'index' (cache file is really an index).
  Added support for asynchronous interface to data sources.

  Revision 1.9  1996/06/27 18:12:40  wenger
  Re-integrated most of the attribute projection code (most importantly,
  all of the TData code) into the main code base (reduced the number of
  modules used only in attribute projection).

  Revision 1.8  1996/06/27 15:49:33  jussi
  TDataAscii and TDataBinary now recognize when a file has been deleted,
  shrunk, or has increased in size. The query processor is asked to
  re-issue relevant queries when such events occur.

  Revision 1.7  1996/05/07 16:46:01  jussi
  This class now makes a copy of the attribute list so that attribute
  hi/lo values can be maintained per data stream, not per schema.
  Hi/lo values are now computed after composite parser is executed.

  Revision 1.6  1996/05/05 03:07:46  jussi
  Removed array of pointers to attribute info for matching values.

  Revision 1.5  1996/03/26 21:33:01  jussi
  Added computation of max/min attribute values.

  Revision 1.4  1996/01/23 20:56:06  jussi
  Cleaned up the code a little bit.

  Revision 1.3  1995/11/22 17:05:00  jussi
  Added IsValid() method and cleaned up code. Added copyright notice.

  Revision 1.2  1995/09/05 22:15:52  jussi
  Added CVS header.
*/

/* interpreted TData using parsed information */

#ifndef TDataAsciiInterp_h
#define TDataAsciiInterp_h

#include "DeviseTypes.h"
#include "ClassDir.h"
#include "TDataAscii.h"
#include "AttrList.h"

#ifndef ATTRPROJ
class TDataAsciiInterpClassInfo: public ClassInfo {
public:
  TDataAsciiInterpClassInfo(char *className, AttrList *attrList, 
			    int recSize, char *separators, int numSeparators,
			    Boolean isSeparator, char *commentString);
	
  TDataAsciiInterpClassInfo(char *className, char *name, char *type,
                            char *param, TData *tdata);
  virtual ~TDataAsciiInterpClassInfo();

  /* Info for category */
  virtual char *CategoryName() { return "tdata"; } 

  /* Info for class */
  virtual char *ClassName();

  /* Get name of parameters and default/current values */
  virtual void ParamNames(int &argc, char **&argv);

  /* Create instance using the supplied parameters. Return
     the instance info if successful, otherwise return NULL. */
  virtual ClassInfo *CreateWithParams(int argc, char **argv);

  /**************************************************
    Instance Info.
  ***************************************************/
  virtual char *InstanceName();
  virtual void *GetInstance();

  /* Get parameters that can be used to re-create this instance */
  virtual void CreateParams(int &argc, char **&argv);

private:
  char *_className;
  char *_name;
  char *_type;
  char *_param;
  TData *_tdata;
  int _recSize;
  AttrList *_attrList;
  char *_separators;
  int _numSeparators;
  char *_commentString;
  Boolean _isSeparator;
};
#endif

class RecInterp;

class TDataAsciiInterp: public TDataAscii {
public:
  TDataAsciiInterp(char *name, char *type, char *param,
                   int recSize, AttrList *attrs,
		   char *separators, int numSeparators, 
		   Boolean isSeparator, char *commentString);
  virtual ~TDataAsciiInterp();

  AttrList *GetAttrList(){ return &_attrList; }

protected:
  /* Decode a record and put data into buffer. Return false if
     this line is not valid. */
  virtual Boolean Decode(void *recordBuf, int recPos, char *line);
  
  virtual void InvalidateIndex();
  virtual Boolean WriteIndex(int fd);
  virtual Boolean ReadIndex(int fd);

private:
  AttrList  _attrList;             /* list of attributes */
  Boolean   hasComposite;
  int       _recSize;
  char      *_separators;
  Boolean   _isSeparator;
  int       _numSeparators;
  char      *_commentString;       /* string for comment, or NULL */
  int       _commentStringLength;  /* length of comment string */
  int       _numAttrs;             /* # attributes (including composite) */
  int       _numPhysAttrs;         /* number of physical attributes */
  RecInterp *_recInterp;
};

#endif
