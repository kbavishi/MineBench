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
  $Id: TDataRangeList.c 2396 2014-03-13 21:13:27Z wkliao $

  $Log: TDataRangeList.c,v $
  Revision 1.3  1996/11/23 21:15:20  jussi
  Minor improvements.

  Revision 1.2  1995/09/05 22:15:59  jussi
  Added CVS header.
*/

#include "TDataRangeList.h"
#include "RangeList.h"

TDataRangeList::TDataRangeList()
{
    for(int i = 0; i < TDataRangeListHashSize; i++)
        _hashTbl[i] = NULL;
}

TDataRangeListEntry *TDataRangeList::Find(TData *tdata)
{
    int bucket = Hash(tdata);
    TDataRangeListEntry *cur;
    for(cur = _hashTbl[bucket]; cur != NULL; cur = cur->next) {
        if (cur->tdata == tdata)
            break;
    }
    return cur;
}

RangeList *TDataRangeList::Get(TData *tdata)
{
    TDataRangeListEntry *cur = Find(tdata);
    if (cur == NULL) {
        /* create a new one */
        cur = new TDataRangeListEntry;
        cur->tdata = tdata;
        cur->rangeList = new RangeList;
        int bucket = Hash(tdata);
        cur->next = _hashTbl[bucket];
        _hashTbl[bucket] = cur;
    }
    return cur->rangeList;
}

void TDataRangeList::Clear()
{
    for(int i = 0; i < TDataRangeListHashSize; i++) {
        while(_hashTbl[i] != NULL) {
            TDataRangeListEntry *entry = _hashTbl[i];
            _hashTbl[i] = _hashTbl[i]->next;
            delete entry->rangeList;
            delete entry;
        }
    }
}
