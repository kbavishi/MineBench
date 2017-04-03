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
  Implementation of ProjectionList class.
 */

/*
  $Id: ProjectionList.c 2396 2014-03-13 21:13:27Z wkliao $

  $Log: ProjectionList.c,v $
  Revision 1.1  1996/06/17 19:14:56  wenger
  Somehow the ProjectionList stuff never got committed before -- committing
  it now.

 */

#define _ProjectionList_c_

#include <stdio.h>

#include "ProjectionList.h"


#if !defined(lint) && defined(RCSID)
static char		rcsid[] = "$RCSfile: ProjectionList.c,v $ $Revision: 2396 $ $State: Exp $";
#endif

static char *	srcFile = __FILE__;

/*------------------------------------------------------------------------------
 * function: ProjectionList::ProjectionList
 * ProjectionList constructor.
 */
ProjectionList::ProjectionList()
{
	_projCount = 0;
	_projList.proj.attrCount = 0;
	_projList.proj.attrList = NULL;
	_projList.next = NULL;
	_currentNodeP = NULL;
	_lastNodeP = &_projList;
}

/*------------------------------------------------------------------------------
 * function: ProjectionList::~ProjectionList
 * ProjectionList destructor.
 */
ProjectionList::~ProjectionList()
{
	ProjListNode *	nodeP = _projList.next;
	ProjListNode *	nextP;

	while (nodeP != NULL)
	{
		nextP = nodeP->next;
		delete nodeP->proj.attrList;
		delete nodeP;
		nodeP = nextP;
	}
}

/*------------------------------------------------------------------------------
 * function: ProjectionList::AddProjection
 * Adds a new projection to the projection list.
 */
DevStatus
ProjectionList::AddProjection(Projection &proj)
{
	DevStatus	result = StatusOk;
	ProjListNode *	nodeP = new ProjListNode;

	nodeP->proj.attrCount = proj.attrCount;
	nodeP->proj.attrList = proj.attrList;

	nodeP->next = NULL;
	_lastNodeP->next = nodeP;
	_lastNodeP = nodeP;

	_projCount++;

	return result;
}

/*------------------------------------------------------------------------------
 * function: ProjectionList::GetFirstProj
 * Gets the first projection in the projection list.
 */
Projection *
ProjectionList::GetFirstProj()
{
	_currentNodeP = _projList.next;

	return &_currentNodeP->proj;
}

/*------------------------------------------------------------------------------
 * function: ProjectionList::GetNextProj
 * Gets the next projection in the projection list (call GetFirstProj()
 * before calling GetNextProj()).
 */
Projection *
ProjectionList::GetNextProj()
{
	if (_currentNodeP != NULL)
	{
		_currentNodeP = _currentNodeP->next;
	}

	return &_currentNodeP->proj;
}

/*------------------------------------------------------------------------------
 * function: ProjectionList::GetProjCount
 * Gets the number of projections in the list.
 */
int
ProjectionList::GetProjCount()
{
	return _projCount;
}

/*============================================================================*/
