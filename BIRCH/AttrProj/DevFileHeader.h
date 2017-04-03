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
  Declaration of DevFileHeader class.
 */

/*
  $Id: DevFileHeader.h 2396 2014-03-13 21:13:27Z wkliao $

  $Log: DevFileHeader.h,v $
  Revision 1.3  1996/11/03 02:41:37  kmurli
  Modified to include the query schema level. Also modified to include DQL
  processing

  Revision 1.2  1996/08/23 16:56:15  wenger
  First version that allows the use of Dali to display images (more work
  needs to be done on this); changed DevStatus to a class to make it work
  better; various minor bug fixes.

  Revision 1.1  1996/07/11 17:25:40  wenger
  Devise now writes headers to some of the files it writes;
  DataSourceSegment class allows non-fixed data length with non-zero
  offset; GUI for editing schema files can deal with comment lines;
  added targets to top-level makefiles to allow more flexibility.

 */

#ifndef _DevFileHeader_h_
#define _DevFileHeader_h_


#include "DeviseTypes.h"


// Note: file types are defined as strings to make Tcl interface easier.
#define FILE_TYPE_SESSION	"session"
#define FILE_TYPE_TDATA		"tdata"
#define FILE_TYPE_DATACAT	"dataCat"
#define FILE_TYPE_PSCHEMA	"physSchema"
#define FILE_TYPE_LSCHEMA	"logSchema"
#define FILE_TYPE_QSCHEMA	"querySchema"
#define FILE_TYPE_TEMP		"temp"
#define FILE_TYPE_WORK		"work"
#define FILE_TYPE_CACHE		"cache"
#define FILE_TYPE_CORAL		"coral"
#define FILE_TYPE_PIXMAP	"pixmap"
#define FILE_TYPE_SCHEMACAT	"schemaCat"


class DevFileHeader
{
public:
    static char *Get(char *fileType);
    //static DevStatus Write();
    //static DevStatus Read();
    //static DevStatus Parse();
    //static DevStatus Skip();
};


#endif /* _DevFileHeader_h_ */

/*============================================================================*/
