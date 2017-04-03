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
  $Id: CompositeParser.c 2396 2014-03-13 21:13:27Z wkliao $

  $Log: CompositeParser.c,v $
  Revision 1.4  1996/09/05 23:14:16  kmurli
  Added a destructor to free the fileType char pointer after use.
  CVS ----------------------------------------------------------------------

  Revision 1.3  1996/03/26 20:22:01  jussi
  Added copyright notice and cleaned up the code a bit.

  Revision 1.2  1995/09/05 22:14:35  jussi
  Added CVS header.
*/

#include <stdio.h>

#include "RecInterp.h"
#include "CompositeParser.h"
#include "Exit.h"
#include <malloc.h>
#include <string.h>

CompositeEntry CompositeParser::_entries[MAX_COMPOSITE_ENTRIES];
int CompositeParser::_numEntries = 0;
int CompositeParser::_hintIndex = -1;

CompositeParser::~CompositeParser()
{
  for(int i = 0; i < _numEntries; i++) 
	free(_entries[i].fileType);
}
	
void CompositeParser::Register(char *fileType, UserComposite *userComposite){
  if (_numEntries >= MAX_COMPOSITE_ENTRIES) {
    fprintf(stderr,"CompositeParser:: too many entries\n");
    Exit::DoExit(2);
  }

  _entries[_numEntries].fileType = (char * )fileType;
  _entries[_numEntries].userComposite = userComposite;
  _numEntries++;
}

void CompositeParser::Decode(char *fileType, RecInterp *recInterp)
{
  if (_hintIndex >= 0 && !strcmp(fileType, _entries[_hintIndex].fileType)) {
    /* found it */
    _entries[_hintIndex].userComposite->Decode(recInterp);
    return;
  }

  /* search for a matching file type */

  for(int i = 0; i < _numEntries; i++) {
    if (!strcmp(fileType,_entries[i].fileType)) {
      /* found it */
      _hintIndex = i;
      _entries[_hintIndex].userComposite->Decode(recInterp);
      return;
    }
  }
  
  /* not found */
  fprintf(stderr, "Can't find user composite function for file type %s\n",
	  fileType);
  Exit::DoExit(2);
}
