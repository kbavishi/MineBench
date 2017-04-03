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
  $Id: ItemList.h 2396 2014-03-13 21:13:27Z wkliao $

  $Log: ItemList.h,v $
  Revision 1.2  1995/09/27 23:59:51  ravim
  Fixed some bugs. Added some new functions for handling groups.

  Revision 1.1  1995/09/22 20:09:30  ravim
  Group structure for viewing schema
*/

#ifndef _ITEMLIST_H_
#define _ITEMLIST_H_

#include "Group.h"

struct GroupItem
{
  Group *itm;
  GroupItem *nxt;
};
typedef GroupItem GroupItem;
  
class ItemList
{
public:
  GroupItem *list;
  GroupItem *curr;

  ItemList();
  ~ItemList();
  
  void add_entry(Group *itm);
  void remove_entry(Group *itm);
  Group *first_item();
  Group *next_item();
};

#endif
