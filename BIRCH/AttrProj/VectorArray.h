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
  Header file for VectorArray class.
 */

/*
  $Id: VectorArray.h 2396 2014-03-13 21:13:27Z wkliao $

  $Log: VectorArray.h,v $
  Revision 1.2  1996/04/30 18:53:39  wenger
  Attrproj now generates a single projection of all attributes of the
  real data.

  Revision 1.1  1996/04/30 15:31:56  wenger
  Attrproj code now reads records via TData object; interface to Birch
  code now in place (but not fully functional).

 */

#ifndef _VectorArray_h_
#define _VectorArray_h_

#include "DeviseTypes.h"

using namespace std;

#include <iostream>	// Needed for vector.h.
#include <fstream>	// Needed for vector.h.
#include "vector.h"


class VectorArray
{
public:
	VectorArray(int vectorCount);
	~VectorArray();

	DevStatus Init(int vecNum, int vecDim);

	int GetVecCount();
	Vector *GetVector(int vecNum);

private:
	int			_vectorCount;
	Vector *	_vectors;
};


#endif /* _VectorArray_h_ */

/*============================================================================*/
