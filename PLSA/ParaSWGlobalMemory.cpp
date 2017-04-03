// ParaSWGlobalMemory.cpp: implementation of the ParaSWGlobalMemory class.
//
//////////////////////////////////////////////////////////////////////

#include "Common.h"
#include "ParaSWGlobalMemory.h"

#include "ParaSWParam.h"
#include "ParaSWProblem.h"

#include "Debug.h"

extern CDebug debugf;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

ParaSWGlobalMemory::ParaSWGlobalMemory()
{
	gridRows = NULL;
	gridCols = NULL;
	gridRowsMem = NULL;
	gridColsMem = NULL;
	beachLine = NULL;
	beachLineMem = NULL;

	length_h = 0;
	length_w = 0;

	grid_h_count = 0;
	grid_w_count = 0;
}

ParaSWGlobalMemory::~ParaSWGlobalMemory()
{
	Release_Mem2D_1(gridCols,grid_w_count,length_h,gridColsMem);
	Release_Mem2D_1(gridRows,grid_h_count,length_w,gridRowsMem);
	Release_Mem_1(beachLine,length_w,beachLineMem);
	length_h = 0;
	length_w = 0;
	grid_h_count = 0;
	grid_w_count = 0;
}

bool ParaSWGlobalMemory::InitGlobalMemory(ParaSWParam *param, ParaSWPathGrid* path_grid)
{
	if(gridRows!=NULL || gridCols!=NULL || beachLine!=NULL)
	{
		return false;	//reallocate not allowed
	}

	grid_h_count = param->grid_height_count;
	grid_w_count = param->grid_width_count;


	// Eric, 2004/11/16
	if (path_grid != NULL)
	{
		length_h = path_grid->end.i - path_grid->start.i;
		length_w = path_grid->end.j - path_grid->start.j;
		
		Allocate_Mem2D_1(gridRows,Node,grid_h_count,length_w,gridRowsMem);
		Allocate_Mem2D_1(gridCols,Node,grid_w_count,length_h,gridColsMem);
		Allocate_Mem_1(beachLine,Node,length_w,beachLineMem);
	}
	return true;
}
