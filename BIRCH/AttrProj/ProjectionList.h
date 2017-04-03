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
  Header file for ProjectionList class.
 */

/*
  $Id: ProjectionList.h 2396 2014-03-13 21:13:27Z wkliao $

  $Log: ProjectionList.h,v $
  Revision 1.1  1996/06/17 19:14:58  wenger
  Somehow the ProjectionList stuff never got committed before -- committing
  it now.

 */

#ifndef _ProjectionList_h_
#define _ProjectionList_h_


#include "DeviseTypes.h"


/* Holds the list of attributes for a particular projection. */
struct Projection
{
	int		attrCount;
	int *	attrList;
};

const int	illegalAttr = -1;

class ProjectionList
{
public:
	ProjectionList();
	~ProjectionList();

	/* Add a new projection to the list (note that the _pointer_ to the
	 * attrList array is copied; attrList will be deleted in the
	 * destructor). */
	DevStatus AddProjection(Projection &proj);

	/* Get the first projection in the list. */
	Projection *GetFirstProj();

	/* Get the next projection in the list (call GetFirstProj() before
	 * calling GetNextProj()). */
	Projection *GetNextProj();

	/* Get the number of projections in the list. */
	int GetProjCount();

private:
	struct ProjListNode
	{
		Projection		proj;
		ProjListNode *	next;
	};

	int				_projCount;
	ProjListNode	_projList;		// Dummy node.
	ProjListNode *	_currentNodeP;	// For GetNextProj().
	ProjListNode *	_lastNodeP;		// For AddProjection().
};


#endif /* _ProjectionList_h_ */

/*============================================================================*/
