/*
  ========================================================================
  DEVise Data Visualization Software
  (c) Copyright 1992-1995
  By the DEVise Development Group
  Madison, Wisconsin
  All Rights Reserved.
  ========================================================================

  Under no circumstances is this software to be copied, distributed,
  or altered in any way without prior permission from the DEVise
  Development Group.
*/

/*
  $Id: ItemList.C,v 1.3 1995/12/22 01:20:40 ravim Exp $

  $Log: ItemList.C,v $
  Revision 1.3  1995/12/22 01:20:40  ravim
  Made insert an append rather than prepend because the order of attrs and
  groups needs to be maintained.

  Revision 1.2  1995/09/27 23:59:50  ravim
  Fixed some bugs. Added some new functions for handling groups.

  Revision 1.1  1995/09/22 20:09:29  ravim
  Group structure for viewing schema
*/

#include <stdio.h>
#include "ItemList.h"

ItemList::ItemList()
{
  list = NULL;
  curr = NULL;
}

ItemList::~ItemList()
{
  GroupItem *nptr;
  GroupItem *ptr = list;

  while (ptr)
  {
    nptr = ptr->nxt;
    delete(ptr);
    ptr = nptr;
  }
  list = NULL;
}

void ItemList::add_entry(Group *itmp)
{
  // Find end of list
  GroupItem *ptr = list;
  while (ptr && (ptr->nxt))
    ptr = ptr->nxt;

  GroupItem *newitem = new(GroupItem);
  newitem->itm = itmp;
  newitem->nxt = NULL;

  if (ptr)
    ptr->nxt = newitem;
  else
    list = newitem;
}

void ItemList::remove_entry(Group *itmp)
{
  GroupItem *tmptr;
  GroupItem *ptr = list;

  if (!list) return;
  if (list->itm == itmp)
  {
    tmptr = list;
    list = list->nxt;
    delete tmptr;
    return;
  }

  while ((ptr->nxt) && (ptr->nxt->itm != itmp))
    ptr = ptr->nxt;

  if (!ptr->nxt) return;

  tmptr = ptr->nxt;
  ptr->nxt = ptr->nxt->nxt;
  delete tmptr;
}

Group *ItemList::first_item()
{
  curr = list;
  
  if (!curr)
    return NULL;
  return curr->itm;
}

Group *ItemList::next_item()
{
  curr = curr->nxt;

  if (!curr)
    return NULL;
  return curr->itm;
}  
