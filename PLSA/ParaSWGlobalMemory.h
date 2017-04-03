// ParaSWGlobalMemory.h: interface for the ParaSWGlobalMemory class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PARASWGLOBALMEMORY_H__A7303D9F_42CF_499D_A0A9_AEBD70834691__INCLUDED_)
#define AFX_PARASWGLOBALMEMORY_H__A7303D9F_42CF_499D_A0A9_AEBD70834691__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/*****************************************************************************************************
* Class ParaSWGlobalMemory
*	// global memory for grid caches and beach line
*	// every segment of a line cache is left-open-right-close
*	// thus: the length is not the score matrix dimension length but the sequence length
******************************************************************************************************/
class Node;
#include "ParaSWProblem.h"
class ParaSWParam;
class ParaSWGlobalMemory  
{
public:
	ParaSWGlobalMemory();
	virtual ~ParaSWGlobalMemory();

	bool InitGlobalMemory(ParaSWParam *param, ParaSWPathGrid* path_grid);

public:
	Node **gridRows;
	Node **gridCols;
	Node *gridRowsMem;
	Node *gridColsMem;

	Node *beachLine;
	Node *beachLineMem;

	int length_h;
	int length_w;;

	int grid_h_count;
	int grid_w_count;
};

#endif // !defined(AFX_PARASWGLOBALMEMORY_H__A7303D9F_42CF_499D_A0A9_AEBD70834691__INCLUDED_)
