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
  Header file for DataSeg (data segment) class.  This class holds info
  for data that occupies only part of a file (or another data source).
 */

/*
  $Id: DataSeg.h 2396 2014-03-13 21:13:27Z wkliao $

  $Log: DataSeg.h,v $
  Revision 1.1  1996/06/04 14:21:40  wenger
  Ascii data can now be read from session files (or other files
  where the data is only part of the file); added some assertions
  to check for pointer alignment in functions that rely on this;
  Makefile changes to make compiling with debugging easier.

 */

#ifndef _DataSeg_h_
#define _DataSeg_h_


class DataSeg
{
public:
	static void Set(char *label, char *filename, long offset, long length);
	static void Get(char *&label, char *&filename, long &offset, long &length);

private:
	static char *	_label;
	static char *	_filename;
	static long		_offset;
	static long		_length;
};


#endif /* _DataSeg_h_ */

/*============================================================================*/
