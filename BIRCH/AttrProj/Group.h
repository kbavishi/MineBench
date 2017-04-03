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
  $Id: Group.h 2396 2014-03-13 21:13:27Z wkliao $

  $Log: Group.h,v $
  Revision 1.9  1996/06/12 14:56:06  wenger
  Added GUI and some code for saving data to templates; added preliminary
  graphical display of TDatas; you now have the option of closing a session
  in template mode without merging the template into the main data catalog;
  removed some unnecessary interdependencies among include files; updated
  the dependencies for Sun, Solaris, and HP; removed never-accessed code in
  ParseAPI.C.

  Revision 1.8  1996/06/07 19:41:06  wenger
  Integrated some of the special attribute projection sources back
  into the regular Devise sources.

  Revision 1.7  1996/05/11 17:29:55  jussi
  Removed subitems() function that used Tcl_Interp arguments.

  Revision 1.6  1996/05/09 18:14:31  kmurli
  Modified Group.C and GroupDir.C to include an oiverloaded functions for
  get_items, subitems to take in a char * instead of Tcp_interp *. This
  is for use in the ServerAPI.c

  Revision 1.5  1996/01/11 21:56:06  jussi
  Replaced libc.h with stdlib.h.

  Revision 1.4  1995/11/18 01:57:49  ravim
  Groups associated with schema.

  Revision 1.3  1995/11/15 07:04:08  ravim
  Minor changes.

  Revision 1.2  1995/09/27 23:59:46  ravim
  Fixed some bugs. Added some new functions for handling groups.

  Revision 1.1  1995/09/22 20:09:26  ravim
  Group structure for viewing schema
*/

#ifndef _GROUP_H_
#define _GROUP_H_

#include <stdlib.h>

#include "MapInterpClassInfo.h"

#define MAX_STR_LEN 200

#define TOPGRP 1
#define SUBGRP 2
#define ITEM 3

class Group;
class ItemList;

class Group
{
public:
  char *name;
  ItemList *subgrps;
  Group *parent;
  int type;

  Group(char *name, Group *par, int typ);
  ~Group();
  Group *insert_item(char *name);
  Group *insert_group(char *name);
  Group *parent_group();
  void subitems(char *);
};

#endif
