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
  $Id: ApParseCat.h 2396 2014-03-13 21:13:27Z wkliao $

  $Log: ApParseCat.h,v $
  Revision 1.4  1996/10/10 16:45:18  wenger
  Changed function names, etc., in ApParseCat.c to get rid of name clashes
  when Donko puts transformation engine code into DEVise.

  Revision 1.3  1996/04/30 15:31:39  wenger
  Attrproj code now reads records via TData object; interface to Birch
  code now in place (but not fully functional).

  Revision 1.2  1996/04/25 19:25:12  wenger
  Attribute projection code can now parse a schema, and create the
  corresponding TData object.

  Revision 1.1  1996/04/22 18:01:48  wenger
  First version of "attribute projection" code.  The parser (with
  the exception of instantiating any TData) compiles and runs.

*/

#ifndef ApParseCat_h
#define ApParseCat_h

#include "DeviseTypes.h"
#include "TData.h"
#if 0
#include "ClassDir.h"
#include "AttrList.h"
#endif

#define NO_GEN_CLASS_INFO

/* Parse a catalog file and register new file type with the system.
   Return name of new file type if successful, else return NULL */
extern char *ApParseCat(char *catFile, char *dataFile, TData *&tDataP);

/* Parse schema(s) from buffer(s) and register new "file type" with
   the system.  Return the name of the new "file type" if successful,
   otherwise return NULL. */
extern char *ApParseSchema(char *schemaName, char *physSchema, char *logSchema);

#ifndef NO_GEN_CLASS_INFO
/* Register a new constructor for class. The
   constructor is called depending on the "source" statment stored
   in the catalog file. For example, "source tape" will call
   the function that generates a TData class that reads from tape */

class AttrList;
class GenClassInfo {
public:
  /* Generate a new TData interpreter */
  virtual ClassInfo *Gen(char *source, Boolean isAscii, char *className,
			 AttrList *attrList, int recSize, char *separators,
			 int numSeparators, Boolean isSeparator,
			 char *commentString) = 0;
};

extern void ApRegisterGenClassInfo(char *source, GenClassInfo *gen);
#endif

#endif
