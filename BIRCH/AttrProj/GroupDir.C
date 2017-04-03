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
  $Id: GroupDir.C,v 1.6 1996/05/11 17:29:44 jussi Exp $

  $Log: GroupDir.C,v $
  Revision 1.6  1996/05/11 17:29:44  jussi
  Removed subitems() function that used Tcl_Interp arguments.

  Revision 1.5  1996/05/09 18:14:32  kmurli
  Modified Group.C and GroupDir.C to include an oiverloaded functions for
  get_items, subitems to take in a char * instead of Tcp_interp *. This
  is for use in the ServerAPI.c

  Revision 1.4  1996/03/26 21:08:57  jussi
  Minor improvements.

  Revision 1.3  1995/11/18 01:53:11  ravim
  Schemalist created.

  Revision 1.2  1995/09/27 23:59:47  ravim
  Fixed some bugs. Added some new functions for handling groups.

  Revision 1.1  1995/09/22 20:09:27  ravim
  Group structure for viewing schema
*/

using namespace std;

#include <stdio.h>
#include <string.h>
#include <iostream>

#include "GroupDir.h"

GroupDir::GroupDir()
{
  list = NULL;
}

GroupDir::~GroupDir()
{
  SchemaList *next;
  while (list) {
    next = list->next;
    delete list->sname;
    delete list->topgrps;
    delete list;
    list = next;
  }
}

void GroupDir::add_entry(char *schema)
{
  SchemaList *elem = new SchemaList;
  elem->sname = new char[strlen(schema) + 1];
  strcpy(elem->sname, schema);
  elem->topgrps = new ItemList;
  elem->next = list;
  list = elem;
}

// Searches for the given schema in the directory.
// If found, returns 1, else returns 0.

int GroupDir::find_entry(char *schema)
{
  SchemaList *curr = list;

  while (curr && (strcmp(curr->sname, schema)))
    curr = curr->next;

  if (!curr)
    return 0;

  return 1;
}

void GroupDir::add_topgrp(char *schema, Group *gp)
{
  SchemaList *curr = list;
  
  while (curr && (strcmp(curr->sname, schema)))
    curr = curr->next;

  if (!curr) {
    cout << "Could not find schema " << schema 
         << " in the directory...group not added" <<endl;
    return;
  }

  curr->topgrps->add_entry(gp);
}

// Find all top level groups in the given schema.

void GroupDir::top_level_groups(char *result, char *schema)
{
  Group *currgrp = 0;
  SchemaList *curr = list;

  result[0] = 0;

  while (curr && strcmp(curr->sname, schema))
    curr = curr->next;

  if (!curr) {
    cout << "Could not find schema " << schema
         << " in the schema directory " << endl;
    return;
  }

  currgrp = curr->topgrps->first_item();
  while (currgrp) {
    strcat(result,"{");
    strcat(result, currgrp->name);
    strcat(result,"} ");
    currgrp = curr->topgrps->next_item();
  }
}

void GroupDir::get_items(char *result, char *schema,
			 char *topgname, char *gname)
{
  SchemaList *curr = list;
  Group *currgrp, *retgrp;
   
  result[0] = 0;

  while (curr && strcmp(curr->sname, schema))
    curr = curr->next;

  if (!curr) {
    cout << "Could not find schema "<< schema << endl;
    return;
  }

  currgrp = curr->topgrps->first_item();

  while (currgrp && strcmp(currgrp->name, topgname))
    currgrp = curr->topgrps->next_item();

  if (!currgrp) {
    cout << "Could not find top group " << topgname
         << " in schema " << schema << endl;
    return;
  }

  retgrp = find_grp(currgrp, gname);
  if (!retgrp) {
    cout << "Could not find group with name " << gname << endl;
    return;
  }

  retgrp->subitems(result);
}
   

Group *GroupDir::find_grp(Group *gp, char *gname)
{
  Group *ret;
  Group *curr;
  if (!(strcmp(gp->name, gname)))
    return gp;

  ret = NULL;
  curr = gp->subgrps->first_item();
  while (curr && (!ret)) {
    if (curr->type != ITEM)
      ret = find_grp(curr, gname);
    curr = gp->subgrps->next_item();
  }

  return ret;
}

int GroupDir::num_topgrp(char *schema)
{
  SchemaList *curr = list;
  Group *currgrp;
  int num = 0;

  while (curr && (strcmp(curr->sname, schema)))
    curr = curr->next;

  if (!curr) {
    cout << "Could not find schema " << schema << endl;
    return 0;
  }
  
  currgrp = curr->topgrps->first_item();
  while (currgrp) {
    num++;
    currgrp = curr->topgrps->next_item();
  }

  return num;
}
