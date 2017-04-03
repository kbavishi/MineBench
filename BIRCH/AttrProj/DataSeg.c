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
  Implementation of DataSeg (data segment class).
 */

/*
  $Id: DataSeg.c 2396 2014-03-13 21:13:27Z wkliao $

  $Log: DataSeg.c,v $
  Revision 1.2  1996/08/05 19:48:53  wenger
  Fixed compile errors caused by some of Kevin's recent changes; changed
  the attrproj stuff to make a .a file instead of a .o; added some more
  TData file writing stuff; misc. cleanup.

  Revision 1.1  1996/06/04 14:21:40  wenger
  Ascii data can now be read from session files (or other files
  where the data is only part of the file); added some assertions
  to check for pointer alignment in functions that rely on this;
  Makefile changes to make compiling with debugging easier.

 */

#define _DataSeg_C_

//#define DEBUG

#include <string.h>

#include "DataSeg.h"
#include "Util.h"

/*
 * Static global variables.
 */
#if !defined(lint) && defined(RCSID)
static char		rcsid[] = "$RCSfile: DataSeg.c,v $ $Revision: 2396 $ $State: Exp $";
#endif

static char *	srcFile = __FILE__;

char *		DataSeg::_label = NULL;
char *		DataSeg::_filename = NULL;
long		DataSeg::_offset = 0;
long		DataSeg::_length = 0;

/*------------------------------------------------------------------------------
 * function: DataSeg::Set
 * Set the information about a data segment.
 */
void
DataSeg::Set(char *label, char *filename, long offset, long length)
{
	if (label == NULL) label = "";
	if (filename == NULL) filename = "";

	DO_DEBUG(printf("DataSeg::Set(%s, %s, %ld, %ld)\n", label, filename,
		offset, length));

	if (_label != NULL)
	{
		delete _label;
		_label = NULL;
	}
	if (label != NULL) _label = strdup(label);

	if (_filename != NULL)
	{
		delete _filename;
		_filename = NULL;
	}
	if (filename != NULL) _filename = strdup(filename);

	_offset = offset;
	_length = length;

	return;
}

/*------------------------------------------------------------------------------
 * function: DataSeg::Get
 * Get the information about a data segment.
 */
void
DataSeg::Get(char *&label, char *&filename, long &offset, long &length)
{
	DO_DEBUG(printf("DataSeg::Get()\n"));

	label = _label;
	filename = _filename;
	offset = _offset;
	length = _length;

	return;
}

/*============================================================================*/
