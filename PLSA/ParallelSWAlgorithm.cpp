#include "Common.h"
#include "ParallelSWAlgorithm.h"
#include "ParaSWParam.h"
#include "ParseFile.h"
#include "ParaSWPath.h"
#include "ParaSWProcessorSet.h"
#include "ParaSWProblem.h"

#include <math.h>
#include <memory.h>

#ifdef WIN32
#include <windows.h>
#include "time.h"
#include "xmmintrin.h"
#else
#include <unistd.h>
#include <sched.h>
#include <time.h>
#include "wtime.h"
#include "emmintrin.h"
#endif

#ifdef _VTUNE
#include <vtuneapi.h>
#endif


extern CDebug debugf;
extern int _num_procs;
extern int bBackSeparate;

// static unsigned __int64 timer_count[32];

/*
unsigned __int64 GetCycleCount ()
{
#ifdef WIN32
	__asm RDTSC;
#else
	asm ("rdtsc");
#endif
}
*/

/******************************************************************************/
/******************************************************************************/
// implementations:

ParaSWPath* ParaSW(ParaSWParam &param)
{
	//2. allocate output paths
	//for global main processor:
	//	allocate output paths / TaskQueue(unSolvedPathGrids) / ResultQueue(solvedPathGrids)
	//	only one paths while processing, but every path grid have a pathID
	ParaSWPath *paths = new ParaSWPath[param.k];// contains several output paths
												// only use paths[0] during the process
												// paths[i(>0)] only use for the output
	//-------------------------------------------
	//3. deal with main problem first because of the SW:
	//for global main processor:
	//	initiate the first path grid for global problem, and push into TaskQueue:
	ParaSWPathGrid *path_grid = new ParaSWPathGrid;
	path_grid->InitPathGrid(-1,0,0,param.seq1->length,param.seq2->length,NULL,NULL);

	bool bBackward = false;
	bBackSeparate = false;

	//-------------------------------------------
	//	Allocate grid cache
	//	Allocate beach line
	//	Allocate local block cache in each processor
	//	Copy sequence to each processor
	//-------------------------------------------
	ParaSWProcessorSet processorSet;
	processorSet.InitProcessorSet(&param, path_grid);

#ifndef WIN32
	timer_start(1);
	// Forward path
	ForwardFillCache(param,path_grid,&processorSet,&paths[0]);
	timer_stop(1);
#else
	clock_t start, end;
	start = clock();
	// Forward path
	ForwardFillCache(param,path_grid,&processorSet,&paths[0]);
	end = clock();
#endif
	
#ifndef WIN32
	printf ("Forward time: %.2fs\n", timer_read(1));
#else
	printf ("Forward time: %.2fs\n", (float)(end-start)/CLOCKS_PER_SEC);
#endif

#ifdef _VTUNE
	// VTResume();
#endif

	bBackward = true;
	if (bBackward)
	{
		/*First step:
		 *    Find those blocks which is still bigger than the number of THRESHOLD
		 *   And try to solve it until there are smaller ones in the path_grids lists.
		 */
#ifndef WIN32
		timer_start(2);
#else
		clock_t time = clock();
#endif
		BackwardFindPathsForHugeBlock(paths, &processorSet, param);
#ifndef WIN32
		timer_stop(2);
		printf ("BackwardFindPathsForHugeBlock Time: %.2f\n", timer_read(2));
		timer_start(3);
#else
		time = clock()-time;
		printf ("BackwardFindPathsForHugeBlock Time: %.2f\n", (float)time/CLOCKS_PER_SEC);
		time = clock();
#endif
		/*Second step:
		 *    Now the blocks in the unsolved polls of path_grids list are smaller ones.
		 *    so we group into N groups(N is the number of processors), that is 
		 *    each processor forms a group and assign the task to these groups.
		 *    each groups solve those matrix parallelly
		 */

		bBackSeparate = true;

		int path_grid_count = paths[0].unSolvedPathGrids.GetCount();
		int** group_proc = new int*[_num_procs];
		int* group_proc_count = new int[_num_procs];
		int** group_task = new int*[_num_procs];
		int* group_task_count = new int[_num_procs];
		int gCount;

		AssignPathGridToProcessor(paths, group_proc,  group_proc_count, group_task, group_task_count, gCount,path_grid_count);
		param.processor_count = 1;
		//param.grid_height_count = 2;
		//param.grid_width_count = 2;
#ifdef _OPENMP
#pragma omp parallel
#endif
		{
#ifdef _OPENMP
			int _my_rank = omp_get_thread_num();
			int _num = omp_get_num_threads();
#else
			int _my_rank = 0;
			int _num = 1;
#endif
			int _mygroup_id = FindMyGroup(group_proc,group_proc_count,gCount,_my_rank);
			
			ParaSWPath *new_paths = new ParaSWPath[param.k];// contains several output paths
			new_paths->length = paths->length;
			new_paths->seq1 = paths->seq1;
			new_paths->seq2 = paths->seq2;
			new_paths->seq1_start = paths->seq1_start;
			new_paths->seq2_start = paths->seq2_start;
			new_paths->seq1_end = paths->seq1_end;
			new_paths->seq2_end = paths->seq2_end;
			int i;
			for(i = 0;i < group_task_count[_mygroup_id]; i++)
			{
				ParaSWPathGrid* pg = FindPathGrid(paths,group_task[_mygroup_id][i]);
				ParaSWPathGrid* newpg = new ParaSWPathGrid;
				memcpy(newpg,pg,sizeof(ParaSWPathGrid));
				new_paths->AddUnSolvedGrid(newpg);
			}

			/*
			 find paths in each groups individually and gather all the solved paths
			*/

			ParaSWParam param1 = param;
			param1.processor_count = 1;
			param1.block_height = new int[1];
			param1.block_height[0] = param.block_height[0];
			
			BackwardFindPath(new_paths,param1);

			ParaSWPathGrid* pg = new_paths->solvedPathGrids.first;
			while(pg != NULL)
			{
				ParaSWPathGrid* npg = new ParaSWPathGrid;
				memcpy(npg,pg,sizeof(ParaSWPathGrid));
				memcpy(npg->direction,pg->direction,sizeof(char)*pg->points_len);
#ifdef _OPENMP
#pragma omp critical 
#endif
				paths[0].AddSolvedGrids(npg);
				pg = pg->next;
			}
		}

#ifndef WIN32
		timer_stop(3);
		printf ("Second phase in backward period Time: %.2f\n", timer_read(3));
#else
		time = clock()-time;
		printf ("Second phase in backward period Time: %.2f\n", (float)time/CLOCKS_PER_SEC);
#endif
	}

#ifdef _VTUNE
	// VTPause();
#endif

/*	float timer, aver_timer = 0.0;
	for (int i = 0; i < _num_procs; i++)
	{
		timer = (double)timer_count[i]/3000000000;
		aver_timer += timer;
		printf ("The sync cost is: %.2f s\n", timer);
	}
	printf ("The average sync cost is: %.2f s\n", aver_timer/_num_procs);
*/
	//-------------------------------------------
	//8. return the unsorted paths:
	return paths;
}

//Get the first huge path grid from the paths we are working on
ParaSWPathGrid* GetHugePathGrid(ParaSWPath* paths)
{
	if(paths[0].unSolvedPathGrids.IsEmpty())
		return NULL;

	ParaSWPathGrid* pg_max = paths[0].unSolvedPathGrids.first;
	ParaSWPathGrid* pre_pg_max = NULL;
	double maxArea = (double)(pg_max->end.j - pg_max->start.j)*(pg_max->end.i - pg_max->start.i);

	ParaSWPathGrid* pg = paths[0].unSolvedPathGrids.first;
	ParaSWPathGrid* pre_pg = NULL;
	pre_pg = pg;
	pg = pg->next;

	while(pg != NULL)
	{
		if((double)(pg->end.j - pg->start.j)*(pg->end.i - pg->start.i) > maxArea)
		{
			maxArea = (double)(pg->end.j - pg->start.j)*(pg->end.i - pg->start.i);
			pg_max = pg;
			pre_pg_max = pre_pg;
		}
		pre_pg = pg;
		pg = pg->next;
	}
	
	//if(pg_max == paths[0].unSolvedPathGrids.first)
	if(pre_pg_max == NULL)
	{
		paths[0].unSolvedPathGrids.first = pg_max->next;
	}
	else
	{
		pre_pg_max->next = pg_max->next;
	}
	return pg_max;	
}


void ForwardFillCache(ParaSWParam &param,
					  ParaSWPathGrid* path_grid,
					  ParaSWProcessorSet *pSet,
					  ParaSWPath *path
					  )
{
	ParaSWProblem problem1;		//each admin processor new such a problem
	ParaSWProblem *problem = &problem1;
	problem->InitMainProblem(&param,path_grid);	

	ParaSWProcessor *localAdminProc;
	localAdminProc = &(pSet->processors[0]);
	problem->GetProcessors(pSet->processorList);

	ParaSWPathGrid **pg;

	if(problem->IsSolvable())
	{
		problem->InitMemory_Solvable();	//problem need init memory
		InitTopLeftData_sw(problem);				//init top left data(sw global)
		pg = SolveFullMatrix_sw(problem);	//release previous path_grid, new pathgrid[k] and return
		for(int i=0;i<problem->big_nodes.big_count;i++)	//for the ith optimal path_grid[i]
		{
			path->AddSolvedGrids(pg[i]);	//may need assign pathID
		}
		delete [] pg;
	}
	//-------------------------------------------
	//6. unsolvable problem: fill grid cache
	else
	{
		problem->AssignGridCache(*(localAdminProc->gMem));	//Get current GridCache of the problem from the local processor's memory
		InitTopLeftData_sw(problem);
		FillGridCache_sw(problem);                  //fill grid cache and save partial results to global grid cache
		
		getPathGrids_sw(problem, path);				//get k path grids from problem and release previous path_grid															//paths[i].SaveUnSolved(new path grid)
		problem->ReleaseGridCache();
	}

	problem->SendBackProcessors(*pSet);
}

void BackwardFindPathsForHugeBlock(ParaSWPath* paths, ParaSWProcessorSet *pMainSet, ParaSWParam& param)
{

	int block_height = param.block_height[0];
	int block_width = param.block_width;


	if(IsTaskPartition(paths))
	{
		return;
	}
	ParaSWPathGrid* path_grid = NULL;
	while((path_grid = GetHugePathGrid(paths)) != NULL)
	{

		int tmp_block_h = block_height;
		int tmp_block_w = block_width;
		int size_h = path_grid->end.j - path_grid->start.j +1;
		int size_w = path_grid->end.i - path_grid->start.i +1;
		int tmp;
		
/*		tmp = size_h / (_num_procs*tmp_block_h);
		if (tmp != 0)
		{
			tmp_block_h  = size_h/(tmp*_num_procs)+1;
			tmp_block_h += (size_h%(tmp*_num_procs))==0?0:1;
		}
		else
			tmp_block_h  = size_h/_num_procs + ((size_h%_num_procs)==0?0:1) +1;
		
		tmp = size_w / (_num_procs*tmp_block_w);
		if (tmp != 0)
		{
			tmp_block_w  = size_w/(tmp*_num_procs)+1;
			tmp_block_w += (size_w%(tmp*_num_procs))==0?0:1;
		}
		else
			tmp_block_w  = size_w/_num_procs + ((size_w%_num_procs)==0?0:1) +1;

		for(int i=0;i<_num_procs;i++)
		{
			param.block_height[i] = tmp_block_h;
		}
		param.block_width = tmp_block_w;
*/		

		ParaSWProcessorSet* pSet = pMainSet;

		ParaSWProcessor *localAdminProc;		
		localAdminProc = &(pSet->processors[0]);
		
		ParaSWProblem problem1;	
		ParaSWProblem *problem = &problem1;
		problem->InitLocalProblem(&param,path_grid);

		problem->GetProcessors(pSet->processorList);

		if(problem->IsSolvable())
		{
			problem->ReleaseGridCache();
			paths->AddUnSolvedGrid(path_grid);
			break;
		}

		problem->AssignGridCache(*(localAdminProc->gMem));	//Get current GridCache of the problem from the local processor's memory

		InitTopLeftData_nw(problem);

		FillGridCache_nw(problem);                  //fill grid cache and save partial results to global grid cache

		ParaSWPath* new_paths = new ParaSWPath[param.k];

		getPathGrids_nw(problem, new_paths);				//get k path grids from problem and release previous path_grid																	

		ParaSWPathGrid* pg;// = new_paths[0].unSolvedPathGrids.first;;
		while((pg = new_paths[0].GetUnSolved())!=NULL)
		{
			paths->AddUnSolvedGrid(pg);
		}
		problem->ReleaseGridCache();
	
		problem->SendBackProcessors(*pSet);	// Eric

		if(IsTaskPartition(paths))
		{
			break;
		}		
	}

	//Restore the original block size before adjustment
	for(int i=0;i<_num_procs;i++)
	{
		param.block_height[i] = block_height;
	}
	param.block_width = block_width;

}

int FindMinimalLoad(int* group_proc[],int group_proc_count[],double used_area[],int gCount)
{
	double M;
	int   Mindex;
	double k;
	M=(double)used_area[0] / group_proc_count[0];
	Mindex = 0 ;
	for(int i = 1;i < gCount; i++)
	{
		if((k=(double)used_area[i]/group_proc_count[i])<M)
		{
			M = k;
			Mindex = i;
		}
	}
	return Mindex;
}

bool IsTaskPartition(ParaSWPath *paths)
{
	int** group_proc = new int*[_num_procs];
	int* group_proc_count = new int[_num_procs];
	int** group_task = new int*[_num_procs];
	int* group_task_count = new int[_num_procs];
	int gCount;
	int path_grid_count;

	ParaSWPathGrid* pg = paths[0].unSolvedPathGrids.first;
	path_grid_count=0;

	while(pg != NULL)
	{
		path_grid_count++;
		pg = pg->next;
	}

	double* area=new double[path_grid_count];
	double sum_area = 0;

	path_grid_count = 0;
	pg = paths[0].unSolvedPathGrids.first;
	while(pg != NULL)
	{
		path_grid_count++;
		area[path_grid_count-1] = (double)(pg->end.j-pg->start.j)*(pg->end.i-pg->start.i);
		pg = pg->next;
		sum_area += area[path_grid_count-1];
	}


	int proc=0;
	int* mygroup;
	int* task_flag;

	int i;
	int j;


	//area_index is the index array for the path_grid in sort
	int* area_index = new int[path_grid_count];
	for(int u=0;u<path_grid_count;u++)
		area_index[u] = u;

	//sort the path_grid by the area 
	double tmp;
	int tmp_index;
	for(i=0;i<path_grid_count;i++)
	{
		for(j=i+1;j<path_grid_count;j++)
		{
			if(area[i]<area[j])
			{
				tmp = area[i];
				area[i] = area[j];
				area[j] = tmp;
				tmp_index = area_index[i];
				area_index[i] = area_index[j];
				area_index[j] = tmp_index;
			}
		}
	}
	//used_area is the area occupied in each processors
	double* used_area=new double[_num_procs];

	int N = _num_procs;

	int** tmp_task;
	int* tmp_task_count;
	tmp_task = new int*[_num_procs];
	tmp_task_count = new int[_num_procs];
	for(i = 0;i<_num_procs;i++)
	{
		tmp_task[i] = new int[path_grid_count];
		tmp_task_count[i] = 0;
	}

	task_flag = new int[path_grid_count];
	for(i=0;i<path_grid_count;i++)
		task_flag[i]=0;
	for(i=0;i<_num_procs;i++)
		used_area[i] = 0;


	gCount = 0;
	for(i=0;i<path_grid_count;i++)
	{
		int p = (int) (ceil((float)area[i]/sum_area*N));
	}
	for(;proc<_num_procs;proc++)
	{
		int p = 1;
		mygroup = new int[p];
		mygroup[0] = proc;
		group_proc[gCount] = mygroup;
		group_proc_count[gCount] = p;
		used_area[gCount]= 0;
		gCount++;
	}
	for(i = 0;i < path_grid_count;i++)
	{
		if(task_flag[i] == 0)
		{
			j = FindMinimalLoad(group_proc,group_proc_count,used_area,gCount);
			tmp_task[j][tmp_task_count[j]] = area_index[i];
			used_area[j] = used_area[j] + area[i];
			tmp_task_count[j] ++;
			task_flag[i] = 1;
		}
	}

	
	for(i=0;i<gCount;i++)
	{
		group_task_count[i] = tmp_task_count[i];
		group_task[i] = new int[group_task_count[i]];
		for(j=0;j<tmp_task_count[i];j++)
		{
			group_task[i][j] = tmp_task[i][j];
		}
	}



	//Evaluate whehter this strategy for partition is fair enough ( the differnce of load between each processor is below 10%)
	double THRESHOLD = 0.1;
	bool bRet = true;
	for(i = 0;i < gCount; i++)
	{
		for(j = i+1;j < gCount; j++)
		{
			if(fabs(used_area[i] - used_area[j]) > THRESHOLD * used_area[i])
			{
				bRet = false;
				break;
			}
		}
		if(bRet == false)break;
	}
	

	//finalize all the allocated variables 
	for(i = 0;i<_num_procs;i++)
	{
		delete tmp_task[i];
	}
	delete[] tmp_task_count;
	delete	task_flag;
	delete	area;
	delete  area_index;
	delete  used_area;
	
	return bRet;
}


//-------------------------------------------
//init first row and column functions:
bool InitTopLeftData_sw(ParaSWProblem *problem)	//init top left data(sw global)
{
	int i,j;
	int &start1 = problem->path_grid->start.i;
	int &start2 = problem->path_grid->start.j;
	int &end1 = problem->path_grid->end.i;
	int &end2 = problem->path_grid->end.j;
	int &go = problem->global_param->gap_open;
	int &ge = problem->global_param->gap_extend;

	if(problem->IsBlockProblem())	//is block problem, fill first row & column in the matrix
	{
		Node **&SM = problem->scoreMatrix;

		SM[0][0].SetData(Node_length,0,TRACE_s_AFF_HERE,0,TRACE_s_AFF_HERE,0,TRACE_s_AFF_HERE);
		SM[0][0].SetStartPosition(true,0,0,0,0,0,0);
		SM[0][0].SetStartPosition(false,0,0,0,0,0,0,0,0,0);
		for (i=1; i<=end1-start1; i++)
		{
			SM[i][0].SetData(Node_length,	0,TRACE_s_AFF_HERE,		NODE_UNREACHABLE,TRACE_s_AFF_E,		ge,TRACE_s_AFF_F);
			SM[i][0].SetStartPosition(true,	i,0,		-1,-1,		i-1,0);
			SM[i][0].SetStartPosition(false,i,0,0,		i,0,1,		i-1,0,2);
		}
		for (j=1; j<=end2-start2; j++)
		{
			SM[0][j].SetData(Node_length,	0,TRACE_s_AFF_HERE,		ge,TRACE_s_AFF_E,		NODE_UNREACHABLE,TRACE_s_AFF_F);
			SM[0][j].SetStartPosition(true,	0,j,		0,j-1,		-1,-1);
			SM[0][j].SetStartPosition(false,0,j,0,		0,j-1,1,	0,j,2);
		}
	}
	else	//is not block problem, fill first grid row & column
	{
		Node **gridRows = problem->gridRows;
		Node **gridCols = problem->gridCols;
		Node *cacheNode = &(problem->cacheNode);
		int grid_h_count = problem->block_division->gridHeights_count;
		int grid_w_count = problem->block_division->gridWidths_count;
		ParaSWGridBlockDivision *gbd =problem->block_division;
		cacheNode[0].SetData(Node_length,0,TRACE_s_AFF_HERE,0,TRACE_s_AFF_HERE,0,TRACE_s_AFF_HERE);
		cacheNode[0].SetStartPosition(true,0,0,0,0,0,0);
		cacheNode[0].SetStartPosition(false,0,0,0,0,0,0,0,0,0);
		for (i=1; i<=end1-start1; i++)
		{
			gridCols[0][i].SetData(Node_length,	0,TRACE_s_AFF_HERE,	NODE_UNREACHABLE,TRACE_s_AFF_E,	ge,TRACE_s_AFF_F);
			gridCols[0][i].SetStartPosition(true,	i,0,		-1,-1,		i-1,0);
			gridCols[0][i].SetStartPosition(false,	i,0,0,		i,0,1,		i-1,0,2);
		}
		for (j=1; j<=end2-start2; j++)
		{
			gridRows[0][j].SetData(Node_length,	0,TRACE_s_AFF_HERE,	ge,TRACE_s_AFF_E,	NODE_UNREACHABLE,TRACE_s_AFF_F);
			gridRows[0][j].SetStartPosition(true,	0,j,		0,j-1,		-1,-1);
			gridRows[0][j].SetStartPosition(false,	0,j,0,		0,j-1,1,	0,j,2);
		}
	}
	
	return true;
}

bool InitTopLeftData_nw(ParaSWProblem *problem)	//init top left data(nw local), according to path_grid start and end
{
	int i,j;
	int start1 = problem->path_grid->start.i;
	int start2 = problem->path_grid->start.j;
	int l1 = problem->path_grid->end.i - problem->path_grid->start.i;
	int l2 = problem->path_grid->end.j - problem->path_grid->start.j;
	Node **SM = problem->scoreMatrix;
	int go = problem->global_param->gap_open;
	int ge = problem->global_param->gap_extend;
	int trace_number = problem->path_grid->trace_num;
	char *trace = problem->path_grid->start_node.trace;
	char *trace_start = problem->path_grid->end_node.trace_start;
	int trace_current;
	
	//don't need problem->IsSolvable()
	//problem->IsBlockProblem() means memory has been allocated
	if(problem->IsBlockProblem())	//is block problem, fill first row & column in the matrix
	{//for block problem, no need to init the start positions, no need to pass start positions
		//trace_current = trace[trace_start[trace_number]];	//start point trace direction
		if(trace_start[trace_number] == 0)		//fix bug 2004.07.28: trace which status of the start point
		{
			trace_current = trace[trace_start[trace_number]];
		}else
		{
			trace_current = trace_start[trace_number]+1;
		}
		if((trace_current == TRACE_s_AFF_H) || (trace_current == TRACE_s_AFF_HERE))
		{
			SM[0][0].SetData(Node_length,	0,TRACE_s_AFF_HERE,		NODE_UNREACHABLE,-1,	NODE_UNREACHABLE,-1);
			SM[0][0].SetStartPosition(true,	start1,start2,		-1,-1,		-1,-1);
			SM[0][0].SetStartPosition(false,start1,start2,0,	-1,-1,-1,	-1,-1,-1);
			//i=1 and j=1 need special handling:
			//i=1:
			SM[1][0].Init_nw_10(go,TRACE_s_AFF_H,start1,start2,0);
			//j=1:
			SM[0][1].Init_nw_01(go,TRACE_s_AFF_H,start1,start2,0);
			//i>=2 and j>=2 case:
			for (i=2; i<=l1; i++)
			{	SM[i][0].Init_nw_20((i-1)*ge + go,start1,start2,0);	}
			for (j=2; j<=l2; j++)
			{	SM[0][j].Init_nw_02((j-1)*ge + go,start1,start2,0);	}
		}else if(trace_current == TRACE_s_AFF_E)
		{
			SM[0][0].SetData(Node_length,	0,TRACE_s_AFF_E,	0,TRACE_s_AFF_HERE,		NODE_UNREACHABLE,-1);
			SM[0][0].SetStartPosition(true,	start1,start2,		start1,start2,			-1,-1);
			SM[0][0].SetStartPosition(false,start1,start2,1,	start1,start2,1,		-1,-1,0);
			//i=1 and j=1 need special handling:
			//i=1:
			SM[1][0].Init_nw_10(go,TRACE_s_AFF_H,start1,start2,1);
			//j=1:
			SM[0][1].Init_nw_01(ge,TRACE_s_AFF_E,start1,start2,1);
			//i>=2 and j>=2 case:
			for (i=2; i<=l1; i++)
			{	SM[i][0].Init_nw_20((i-1)*ge + go,start1,start2,1);	}
			for (j=2; j<=l2; j++)
			{	SM[0][j].Init_nw_02(j*ge,start1,start2,1);	}
		}else if(trace_current == TRACE_s_AFF_F)
		{
			SM[0][0].SetData(Node_length,	0,TRACE_s_AFF_F,	NODE_UNREACHABLE,-1,	0,TRACE_s_AFF_HERE);
			SM[0][0].SetStartPosition(true,	start1,start2,		-1,-1,		start1,start2);
			SM[0][0].SetStartPosition(false,start1,start2,2,	-1,-1,-1,	start1,start2,2);
			//i=1 and j=1 need special handling:
			//i=1:
			SM[1][0].Init_nw_10(ge,TRACE_s_AFF_F,start1,start2,2);
			//j=1:
			SM[0][1].Init_nw_01(go,TRACE_s_AFF_H,start1,start2,2);
			//i>=2 and j>=2 case:
			for (i=2; i<=l1; i++)
			{	SM[i][0].Init_nw_20(i*ge,start1,start2,2);	}
			for (j=2; j<=l2; j++)
			{	SM[0][j].Init_nw_02((j-1)*ge + go,start1,start2,2);	}
		}
	}
	else	//is not block problem, fill first grid row & column
	{
		Node **gridRows = problem->gridRows;
		Node **gridCols = problem->gridCols;
		Node *cacheNode = &(problem->cacheNode);
		if(trace_start[trace_number] == 0)		//fix bug 2004.07.28: trace which status of the start point
		{
			trace_current = trace[trace_start[trace_number]];
		}else
		{
			trace_current = trace_start[trace_number]+1;
		}
		//trace_current = trace[trace_start[trace_number]];	//start point trace direction
		if((trace_current == TRACE_s_AFF_H) || (trace_current == TRACE_s_AFF_HERE))
		{
			cacheNode[0].SetData(Node_length,	0,TRACE_s_AFF_HERE,		NODE_UNREACHABLE,-1,	NODE_UNREACHABLE,-1);
			cacheNode[0].SetStartPosition(true,	start1,start2,		-1,-1,		-1,-1);
			cacheNode[0].SetStartPosition(false,start1,start2,0,	-1,-1,-1,	-1,-1,-1);
			//i=1 and j=1 need special handling:
			//i=1:
			gridCols[0][1].Init_nw_10(go,TRACE_s_AFF_H,start1,start2,0);
			//j=1:
			gridRows[0][1].Init_nw_01(go,TRACE_s_AFF_H,start1,start2,0);
			//i>=2 and j>=2 case:
			for (i=2; i<=l1; i++)
			{	gridCols[0][i].Init_nw_20((i-1)*ge + go,start1,start2,0);	}
			for (j=2; j<=l2; j++)
			{	gridRows[0][j].Init_nw_02((j-1)*ge + go,start1,start2,0);	}
		}else if(trace_current == TRACE_s_AFF_E)
		{
			//cacheNode[0].SetData(Node_length,	0,TRACE_s_AFF_E,	0,TRACE_s_AFF_HERE,		NODE_UNREACHABLE,-1);	//status E's trace number should be E, not HERE, because not end
			cacheNode[0].SetData(Node_length,	0,TRACE_s_AFF_E,	0,TRACE_s_AFF_E,		NODE_UNREACHABLE,-1);	//fix bug 2004.07.28: set TRACE_s_AFF_E for following divide
			cacheNode[0].SetStartPosition(true,	start1,start2,		start1,start2,			-1,-1);
			cacheNode[0].SetStartPosition(false,start1,start2,1,	start1,start2,1,		-1,-1,0);
			//i=1 and j=1 need special handling:
			//i=1:
			gridCols[0][1].Init_nw_10(go,TRACE_s_AFF_H,start1,start2,1);
			//j=1:
			gridRows[0][1].Init_nw_01(ge,TRACE_s_AFF_E,start1,start2,1);
			//i>=2 and j>=2 case:
			for (i=2; i<=l1; i++)
			{	gridCols[0][i].Init_nw_20((i-1)*ge + go,start1,start2,1);	}
			for (j=2; j<=l2; j++)
			{	gridRows[0][j].Init_nw_02(j*ge,start1,start2,1);	}
		}else if(trace_current == TRACE_s_AFF_F)
		{
			//cacheNode[0].SetData(Node_length,	0,TRACE_s_AFF_F,	NODE_UNREACHABLE,-1,	0,TRACE_s_AFF_HERE);	//status F's trace number should be F, not HERE, because not end
			cacheNode[0].SetData(Node_length,	0,TRACE_s_AFF_F,	NODE_UNREACHABLE,-1,	0,TRACE_s_AFF_F);		//fix bug 2004.07.28: set TRACE_s_AFF_F for following divide
			cacheNode[0].SetStartPosition(true,	start1,start2,		-1,-1,		start1,start2);
			cacheNode[0].SetStartPosition(false,start1,start2,2,	-1,-1,-1,	start1,start2,2);
			//i=1 and j=1 need special handling:
			//i=1:
			gridCols[0][1].Init_nw_10(ge,TRACE_s_AFF_F,start1,start2,2);
			//j=1:
			gridRows[0][1].Init_nw_01(go,TRACE_s_AFF_H,start1,start2,2);
			//i>=2 and j>=2 case:
			for (i=2; i<=l1; i++)
			{	gridCols[0][i].Init_nw_20(i*ge,start1,start2,2);	}
			for (j=2; j<=l2; j++)
			{	gridRows[0][j].Init_nw_02((j-1)*ge + go,start1,start2,2);	}
		}
	}
	
	return true;
}

//-------------------------------------------
//core functions: solve full matrix
ParaSWPathGrid** SolveFullMatrix_sw(ParaSWProblem *problem)	//result is saving in the path_grid
{
	return SolveFullMatrix(problem,true);
}

ParaSWPathGrid** SolveFullMatrix_nw(ParaSWProblem *problem)	//result is saving in the path_grid
{
	return SolveFullMatrix(problem,false);
}

ParaSWPathGrid** SolveFullMatrix(ParaSWProblem *problem,bool isSW)
{
	int i,j;
	int start1 = problem->path_grid->start.i;
	int start2 = problem->path_grid->start.j;
	int end1 = problem->path_grid->end.i;
	int end2 = problem->path_grid->end.j;
	Node **SM = problem->scoreMatrix;
	int go = problem->global_param->gap_open;
	int ge = problem->global_param->gap_extend;
	int *seq1 = problem->local_index_seq1;
	int *seq2 = problem->local_index_seq2;
	SubstitutionMatrix *pssm = problem->global_param->pssm;

	int l1 = end1-start1;
	int l2 = end2-start2;

	//fill matrix:
	int t0,t1,t2;
	int t_score;
	int sc;
	if(isSW)	//for sw full matrix:
	{
		for (i=1; i<=l1; i++)
		{
			for (j=1; j<=l2; j++)
			{
				sc = pssm->score[seq1[i]][seq2[j]];
				//E:
				t0 = SM[i][j-1].score[0] + go;
				t1 = SM[i][j-1].score[1] + ge;
				if(t0>=t1)
				{
					SM[i][j].SetOneData(1,t0,TRACE_s_AFF_H);
					SM[i][j].SetOneStartPosition(1,SM[i][j-1],0);
				}
				else
				{
					SM[i][j].SetOneData(1,t1,TRACE_s_AFF_E);
					SM[i][j].SetOneStartPosition(1,SM[i][j-1],1);
				}
				//F:
				t0 = SM[i-1][j].score[0] + go;
				t2 = SM[i-1][j].score[2] + ge;
				if(t0>=t2)
				{
					SM[i][j].SetOneData(2,t0,TRACE_s_AFF_H);
					SM[i][j].SetOneStartPosition(2,SM[i-1][j],0);
				}
				else
				{	
					SM[i][j].SetOneData(2,t2,TRACE_s_AFF_F);
					SM[i][j].SetOneStartPosition(2,SM[i-1][j],2);
				}
				//H:
				t0 = SM[i-1][j-1].score[0] + sc;
				t1 = SM[i][j].score[1];
				t2 = SM[i][j].score[2];
				if(0>=t0 && 0>=t1 && 0>=t2)
				{
					SM[i][j].SetOneData(0,0,TRACE_s_AFF_HERE);
					SM[i][j].SetOneStartPosition(0,true,i,j);
					SM[i][j].SetOneStartPosition(0,false,i,j,0);
					t_score = 0;
				}else if(t0>=t1 && t0>=t2)
				{
					SM[i][j].SetOneData(0,t0,TRACE_s_AFF_H);
					SM[i][j].SetOneStartPosition(0,SM[i-1][j-1],0);
					t_score = t0;
				}
				else if(t1>=t2)
				{
					SM[i][j].SetOneData(0,t1,TRACE_s_AFF_E);
					SM[i][j].SetOneStartPosition(0,SM[i][j],1);
					t_score = t1;
				}
				else
				{
					SM[i][j].SetOneData(0,t2,TRACE_s_AFF_F);
					SM[i][j].SetOneStartPosition(0,SM[i][j],2);
					t_score = t2;
				}
				
				//update maximum score and position:
				//UpdateMaxScorePos(t_score,i,j,(*this)(i,j));
				problem->big_nodes.TryAddBigNode(SM[i][j],i+start1,j+start2);
			}
		}
	}//end of SW
	else	//for NW
	{
		for (i=1; i<=l1; i++)
		{
			for (j=1; j<=l2; j++)
			{
				sc = pssm->score[seq1[i]][seq2[j]];
				//E:
				t0 = SM[i][j-1].score[0] + go;
				t1 = SM[i][j-1].score[1] + ge;
				if(t0>=t1)
				{
					SM[i][j].SetOneData(1,t0,TRACE_s_AFF_H);
				}
				else
				{
					SM[i][j].SetOneData(1,t1,TRACE_s_AFF_E);
				}
				//F:
				t0 = SM[i-1][j].score[0] + go;
				t2 = SM[i-1][j].score[2] + ge;
				if(t0>=t2)
				{
					SM[i][j].SetOneData(2,t0,TRACE_s_AFF_H);
				}
				else
				{	
					SM[i][j].SetOneData(2,t2,TRACE_s_AFF_F);
				}
				//H:
				t0 = SM[i-1][j-1].score[0] + sc;
				t1 = SM[i][j].score[1];
				t2 = SM[i][j].score[2];
				if(t0>=t1 && t0>=t2)
				{
					SM[i][j].SetOneData(0,t0,TRACE_s_AFF_H);
					t_score = t0;
				}
				else if(t1>=t2)
				{
					SM[i][j].SetOneData(0,t1,TRACE_s_AFF_E);
					t_score = t1;
				}
				else
				{
					SM[i][j].SetOneData(0,t2,TRACE_s_AFF_F);
					t_score = t2;
				}
			}
		}
	}

	//new path grids:
	ParaSWPathGrid **pg;
	int pg_count;
	if(isSW)	//for SW
	{
		if(problem->big_nodes.big_count>1)
		{
			Release_Object(problem->path_grid);	//release previous path_grid
			pg = new ParaSWPathGrid*[problem->big_nodes.big_count];	//fix bug 2004.07.28: allocate
			for(i=0;i<problem->big_nodes.big_count;i++)
			{
				pg[i] = new ParaSWPathGrid;
			}
			//pg = new ParaSWPathGrid[problem->big_nodes.big_count];	//the error is : allocate as an array, but delete separately
		}else
		{
			pg = new ParaSWPathGrid*[1];
			pg[0] = problem->path_grid;
		}
		pg_count = problem->big_nodes.big_count;
	}else		//for NW
	{
		pg = new ParaSWPathGrid*[1];
		pg[0] = problem->path_grid;
		pg_count = 1;
	}

	//find optimal alignments by tracing:
	//Alignfromtrace_core(type,s1,s2,s1_bound,s2_bound,ETM,ali,LeftUpPoint);
	int k;
	for(k=0;k<pg_count;k++)
	{
		int ali_start1;
		int ali_start2;
		int ali_end1;
		int ali_end2;
		if(isSW)	//for SW
		{
			ali_start1 = problem->big_nodes.nodes[k].global_start[0].i;
			ali_start2 = problem->big_nodes.nodes[k].global_start[0].j;
			ali_end1 = problem->big_nodes.positions[k].i;
			ali_end2 = problem->big_nodes.positions[k].j;
			//init path grid:
			pg[k]->InitPathGrid(k,ali_start1,
						ali_start2,
						ali_end1,
						ali_end2,
						&(SM[ali_start1][ali_start2]),
						&(SM[ali_end1][ali_end2]));
			pg[k]->trace_num = 0;	//bug fixed 2004.07.27: SW and solve full matrix, last status is 0
			pg[k]->delta_score = pg[k]->end_node.score[0] - pg[k]->start_node.score[0];	//for total score
			//pg[k].InitPathGrid(ali_start1,ali_start2,ali_end1,ali_end2,&(SM[ali_start1][ali_start2]),&(SM[ali_end1][ali_end2]));
		}else
		{
			ali_start1 = problem->path_grid->start.i;
			ali_start2 = problem->path_grid->start.j;
			ali_end1 = problem->path_grid->end.i;
			ali_end2 = problem->path_grid->end.j;
		}
		pg[k]->AllocatePath();
		char *dir;
		dir = pg[k]->direction;
		
		//find the best alignment:
		int current = pg[k]->trace_num;	//the current trace number
		int t;	//in previous point, which status is the status link the current status
		int t1;	//inside this point, which status is the status link its previous point in the alignment
				//IN: current: inside this point, which status is the status link its posterior point in the alignment
				//note: the trace procedure is from the last point to the first point in the alignment
		i = ali_end1 - start1;
		j = ali_end2 - start2;
		bool end_flag=false;
		if(current == 0)
		{
			t1 = SM[i][j].trace[0];	//H trace
			if(t1 == TRACE_s_AFF_HERE)
			{
				//return NULL;
				end_flag = true;
			}else if(t1 == TRACE_s_AFF_H)
			{
				t = t1;
			}else
			{
				t = SM[i][j].trace[t1-1];	//E or F trace
			}
		}else
		{
			t1 = current + 1;
			t = SM[i][j].trace[current];	//E or F trace
		}
		pg[k]->points_len = 1;
		while(t!=TRACE_s_AFF_HERE && !end_flag)
		{
			//the current direction:
			dir[pg[k]->points_len-1] = t1;
			pg[k]->points_len++;
			//go to next point:
			i -= (t1!=TRACE_s_AFF_E);
			j -= (t1!=TRACE_s_AFF_F);
			current = t-1;
			//next node status:
			if(current == 0)
			{
				t1 = SM[i][j].trace[current];		//H trace
				if(t1 == TRACE_s_AFF_HERE)
				{
					//return NULL;
					end_flag = true;
				}else if(t1 == TRACE_s_AFF_H)
				{
					t = t1;
				}else
				{
					t = SM[i][j].trace[t1-1];	//E or F trace
				}
			}else
			{
				t1 = current + 1;
				t = SM[i][j].trace[current];	//E or F trace
			}
		}
		pg[k]->IsSolved = true;
	}

	//return the paths:
	return pg;
}

//-------------------------------------------
//core functions: fill grid cache
bool FillGridCache_sw(ParaSWProblem *problem)
{
	return FillGridCache(problem,true);
}

bool FillGridCache_nw(ParaSWProblem *problem)
{
	return FillGridCache(problem,false);
}

bool FillGridCache_nw_single_processor(ParaSWProblem *problem)
{
	return FillGridCache_single_processor(problem,false);
}

bool FillGridCache_implement(ParaSWProcessor *proc,bool isWidthOnGrid,bool isHeightOnGrid,bool isSW)
{
	//4. reset local start point for first column or row of a block if needed:
	if(isWidthOnGrid)	//if grid start, local start set to here
	{
		proc->sub_problem->ResetBlockLocalCol(isSW);
	}
	if(isHeightOnGrid)	//if grid start, local start set to here
	{
		proc->sub_problem->ResetBlockLocalRow(isSW);
	}
	
	////////////////////////////////////////////////////////////////
	//5. fill block
	proc->sub_problem->FillBlock(isSW);
	return true;
}

bool FillGridCache_each_processor(ParaSWProblem *sub_problem,
								  ParaSWProblem *problem,		//for the path grid and the division
								  int sub_problem_count,
								  bool isSW,
								  bool &isFilled)
{
	int i,j;
	int start1 = problem->path_grid->start.i;
	int start2 = problem->path_grid->start.j;
	
	int go = problem->global_param->gap_open;
	int ge = problem->global_param->gap_extend;
	int block_h_count = problem->block_division->blockHeights_count;
	int block_w_count = problem->block_division->blockWidths_count;
	Node **gridRows = problem->gridRows;
	Node **gridCols = problem->gridCols;
	Node *cacheNode = &(problem->cacheNode);
	ParaSWGridBlockDivision *gbd =problem->block_division;
	//BeachLine *beach_line = problem->block_Beach;
	BeachLine *beach_line = sub_problem->block_Beach;
	BeachLine *next_beach_line = problem->processors[sub_problem->processors[0]->tmp_next_proc]->sub_problem->block_Beach;
//	BeachLine *admin_beach_line = problem->block_Beach_Admin;
	
	ParaSWProcessor *proc = sub_problem->processor_list.first->p_proc;
	int local_start1;
	int local_start2;
	int local_len1;
	int local_len2;
	Node tmpNode;

	////////////////////////////////////////////////////////////////
	//1. test if this processor need not work:
	if(proc->sub_problem_h>=gbd->blockHeights_count)	//no need work
	{
		return true;
	}else											//need work
	{
		isFilled = false;
	}
	
	local_start1 = sub_problem->path_grid->start.i;
	local_start2 = sub_problem->path_grid->start.j;
	local_len1 = sub_problem->path_grid->end.i - local_start1;
	local_len2 = sub_problem->path_grid->end.j - local_start2;
	
	Node *pHozScore = sub_problem->pHozScore;	// eric, 2004.10.14
	Node *pVerScore = sub_problem->pVerScore;	// eric, 2004.10.14

	////////////////////////////////////////////////////////////////
	//2. init processors' left column, from grid cache in global memory OR in local memory
	if(proc->sub_problem_w == 0)	//first column, get from grid cache
	{
		if(local_start1!=start1)
		{
			tmpNode = gridCols[0][local_start1 - start1];
		}else
		{
			tmpNode = cacheNode[0];
		}
		pHozScore[0] = tmpNode;	// eric, 2004.10.14
		pVerScore[0] = tmpNode;	// eric, 2004.10.14

		for(j=1;j<=local_len1;j++)	//first element must recalculate
			pVerScore[j] = gridCols[0][local_start1 - start1 + j];	// eric, 2004.10.14
	}
	else						//not first column get from self
	{
		pHozScore[0] = pVerScore[0];
	}
	
	////////////////////////////////////////////////////////////////
	//3. get top row from beach line of local memory
	//wait until (beach_line->beach_front[sub_problem_w[i]] == sub_problem_h[i])

	// unsigned __int64 t = GetCycleCount();
	while (beach_line->beach_front[proc->sub_problem_w] != proc->sub_problem_h)
	{
#ifdef WIN32
		__asm PAUSE;
		// Sleep(0);
		// _mm_pause();
#else 
		// usleep(1);
		// sched_yield();
		_mm_pause();
#endif
	}

#ifdef _OPENMP
	// timer_count[omp_get_thread_num()] += GetCycleCount() - t;
#endif
	
	// printf ("block[%d, %d] ready, count = %d\n", proc->sub_problem_w, proc->sub_problem_h, count);
	
	for(j=1;j<=local_len2;j++)
		pHozScore[j] = beach_line->beach[proc->sub_problem_w][j-1];	// eric, 2004.10.14
	
	////////////////////////////////////////////////////////////////
	//STEP 4 & 5 could parallel.
	////////////////////////////////////////////////////////////////
	FillGridCache_implement(proc,
		(gbd->blockWidthOnGrid[proc->sub_problem_w] & 0x100) ==0x100,	//0x100 means first column on grid, need reset the local start point
		(gbd->blockHeightOnGrid[proc->sub_problem_h] & 0x100) ==0x100,	//0x100 means first row on grid, need reset the local start point
		isSW);
	
	////////////////////////////////////////////////////////////////
	//6. save to the beach line in global memory
	//use block-to-block data transfer
	//could be done like this:
	//	1st. send data from SM to next beach_line
	//	2nd. read data from local beach_line to SM for initialize
	//<transfer>
	for(j=0;j<local_len2;j++)
		next_beach_line->beach[proc->sub_problem_w][j] = pHozScore[j+1];	// eric, 2004.10.14
	
#ifdef _OPENMP
// #pragma omp critical (beach_line)
#endif
	next_beach_line->beach_front[proc->sub_problem_w] = proc->sub_problem_h+1;
	
	////////////////////////////////////////////////////////////////
	//7. save to grid cache if needed
	//<transfer>
	int i_grid;
	if((gbd->blockWidthOnGrid[proc->sub_problem_w] & 0x200) == 0x200)	//end is on grid
	{
		i_grid = (gbd->blockWidthOnGrid[proc->sub_problem_w] & 0x0ff) + 1;	//number of grid
		for(i=1;i<=local_len1;i++)
			gridCols[i_grid][local_start1 - start1 + i] = pVerScore[i];	// eric, 2004.10.14
	}
	if((gbd->blockHeightOnGrid[proc->sub_problem_h] & 0x200) == 0x200)	//end is on grid
	{
		i_grid = (gbd->blockHeightOnGrid[proc->sub_problem_h] & 0x0ff) + 1;	//number of grid
		for(i=1;i<=local_len2;i++)
			gridRows[i_grid][local_start2 - start2 + i] = pHozScore[i];	// eric, 2004.10.14
	}

	////////////////////////////////////////////////////////////////
	//8. reset path grid and problem front for a sub problem
	proc->pre_local_len2 = local_len2;
	proc->sub_problem_h += ((proc->sub_problem_w+1) / gbd->blockWidths_count) * sub_problem_count;
	if(proc->sub_problem_h>=gbd->blockHeights_count)	//no need work any more, don't erase the path_grid structure
	{
		return false;
	}
	proc->sub_problem_w = (proc->sub_problem_w+1) % gbd->blockWidths_count;
	sub_problem->path_grid->start.i = start1 + gbd->blockHeights[proc->sub_problem_h].start;
	sub_problem->path_grid->start.j = start2 + gbd->blockWidths[proc->sub_problem_w].start;
	sub_problem->path_grid->end.i = start1 + gbd->blockHeights[proc->sub_problem_h].end;
	sub_problem->path_grid->end.j = start2 + gbd->blockWidths[proc->sub_problem_w].end;
	sub_problem->local_index_seq1 = sub_problem->global_param->seq1->index + sub_problem->path_grid->start.i - 1;
	sub_problem->local_index_seq2 = sub_problem->global_param->seq2->index + sub_problem->path_grid->start.j - 1;
	
	return false;
}

bool FillGridCache_single_processor(ParaSWProblem *problem,bool isSW)
{
	int i;
	int j;
	int start1 = problem->path_grid->start.i;
	int start2 = problem->path_grid->start.j;
	int block_h_count = problem->block_division->blockHeights_count;
	int block_w_count = problem->block_division->blockWidths_count;
	Node **gridRows = problem->gridRows;
	ParaSWGridBlockDivision *gbd =problem->block_division;
//	BeachLine *beach_line = problem->block_Beach;
//	BeachLine *admin_beach_line = problem->block_Beach_Admin;

	////////////////////////////////////////////////////////////////
	//1. the division is done in ParaSWProblem->TryGetProcessors()
	////////////////////////////////////////////////////////////////
	//2. init beach line from grid cache
//	for(i=0;i<block_w_count;i++)
//	{
//		for(j=0;j<gbd->blockWidths[i].end-gbd->blockWidths[i].start;j++)
//		{
//			beach_line->beach[i][j] = gridRows[0][gbd->blockWidths[i].start+j+1];
//		}
//		admin_beach_line->beach_front[i] = 0;
//	}

	////////////////////////////////////////////////////////////////
	//3. init each processors' sub_problem:
	int sub_problem_count = problem->processor_list.GetCount();
	ParaSWProcessor **processors = problem->processors;
	for(i=0;i<sub_problem_count;i++)
	{
		processors[i]->path_grid->InitPathGrid(start1 + gbd->blockHeights[i].start,
			start2,
			start1 + gbd->blockHeights[i].end,
			start2 + gbd->blockWidths[0].end,
			NULL,NULL);
		processors[i]->sub_problem->InitLocalProblem(problem->global_param,processors[i]->path_grid);
		processors[i]->sub_problem->processor_list.AddHead(processors[i]);	//give processors to sub problem
		Allocate_Mem(processors[i]->sub_problem->processors,ParaSWProcessor*,1);
		processors[i]->sub_problem->processors[0] = processors[i];			//give processors to sub problem
		processors[i]->sub_problem->scoreMatrix = processors[i]->ScoreMatrix;

		processors[i]->sub_problem->pHozScore = processors[i]->pHozScore;	// eric 2004.10.14
		processors[i]->sub_problem->pVerScore = processors[i]->pVerScore;	// eric 2004.10.14

		processors[i]->sub_problem_w = 0;
		processors[i]->sub_problem_h = i;
		processors[i]->pre_local_len2 = -1;

		//init beach line on each processor:
		processors[i]->sub_problem->block_Beach->InitBeachLine(problem,processors[i]->gMem);	//use admin's problem to init block beach
		for(int k=0;k<block_w_count;k++)
		{
			for(j=0;j<gbd->blockWidths[k].end-gbd->blockWidths[k].start;j++)
			{
				processors[i]->sub_problem->block_Beach->beach[k][j] = gridRows[0][gbd->blockWidths[k].start+j+1];
			}
			processors[i]->sub_problem->block_Beach->beach_front[k] = 0;
		}
		processors[i]->tmp_next_proc = (i+1)%sub_problem_count;
	}

	////////////////////////////////////////////////////////////////
	//4. fill grid cache using each processor:
	int k;
	bool isFilled;
	while(1)
	{
		isFilled = true;
		for(k=0;k<sub_problem_count;k++)			//parallel can be done here!!!!!!!!!!!!!!!!!
		{
			FillGridCache_each_processor(processors[k]->sub_problem,problem,
				sub_problem_count,isSW,isFilled);
		}
		//test if end:
		if(isFilled) break;
	}//end while(1)

	////////////////////////////////////////////////////////////////
	//5. get maximum positions from each sub_problem:
	if(isSW)
	{//<transfer>
		for(k=0;k<sub_problem_count;k++)
		{
			for(i=0;i<processors[k]->sub_problem->big_nodes.big_count;i++)
			{
				
				problem->big_nodes.TryAddBigNode(processors[k]->sub_problem->big_nodes.nodes[i],
					processors[k]->sub_problem->big_nodes.positions[i].i,
					processors[k]->sub_problem->big_nodes.positions[i].j);
			}
		}
		for(int pk=0;pk<problem->global_param->k;pk++)
		{
			printf("score=%d  x=%d y=%d globalstart.i=%d globalstart.j=%d\n",
				problem->big_nodes.nodes[pk].score[0],
				problem->big_nodes.positions[pk].i,
				problem->big_nodes.positions[pk].j,
				problem->big_nodes.nodes[pk].global_start->i,
				problem->big_nodes.nodes[pk].global_start->j
				);
		}//end for pk

	}else
	{//<transfer>
		//the last block is computed on the processor: (block_h_count-1)%sub_problem_count
		int p_n = (sub_problem_count+block_h_count-1)%sub_problem_count;
		//the last node position in the unique processor is saved in path_grid
		int p_j = processors[p_n]->sub_problem->path_grid->end.j - processors[p_n]->sub_problem->path_grid->start.j;
		int p_i = processors[p_n]->sub_problem->path_grid->end.i - processors[p_n]->sub_problem->path_grid->start.i;
		problem->path_grid->end_node = processors[p_n]->sub_problem->pHozScore[p_j];
	}

	return true;
}


bool FillGridCache(ParaSWProblem *problem,bool isSW)
{
	int i;
	int j;
	int start1 = problem->path_grid->start.i;
	int start2 = problem->path_grid->start.j;
	int block_h_count = problem->block_division->blockHeights_count;
	int block_w_count = problem->block_division->blockWidths_count;
	Node **gridRows = problem->gridRows;
	ParaSWGridBlockDivision *gbd =problem->block_division;
//	BeachLine *beach_line = problem->block_Beach;
//	BeachLine *admin_beach_line = problem->block_Beach_Admin;

	////////////////////////////////////////////////////////////////
	//1. the division is done in ParaSWProblem->TryGetProcessors()
	////////////////////////////////////////////////////////////////
	//2. init beach line from grid cache
//	for(i=0;i<block_w_count;i++)
//	{
//		for(j=0;j<gbd->blockWidths[i].end-gbd->blockWidths[i].start;j++)
//		{
//			beach_line->beach[i][j] = gridRows[0][gbd->blockWidths[i].start+j+1];
//		}
//		admin_beach_line->beach_front[i] = 0;
//	}

	////////////////////////////////////////////////////////////////
	//3. init each processors' sub_problem:
	int sub_problem_count = problem->processor_list.GetCount();
	ParaSWProcessor **processors = problem->processors;
	for(i=0;i<sub_problem_count;i++)
	{
		processors[i]->path_grid->InitPathGrid(start1 + gbd->blockHeights[i].start,
			start2,
			start1 + gbd->blockHeights[i].end,
			start2 + gbd->blockWidths[0].end,
			NULL,NULL);
		processors[i]->sub_problem->InitLocalProblem(problem->global_param,processors[i]->path_grid);
		processors[i]->sub_problem->processor_list.AddHead(processors[i]);	//give processors to sub problem
		Allocate_Mem(processors[i]->sub_problem->processors,ParaSWProcessor*,1);
		processors[i]->sub_problem->processors[0] = processors[i];			//give processors to sub problem
		processors[i]->sub_problem->scoreMatrix = processors[i]->ScoreMatrix;

		processors[i]->sub_problem->pHozScore = processors[i]->pHozScore;	// eric 2004.10.14
		processors[i]->sub_problem->pVerScore = processors[i]->pVerScore;	// eric 2004.10.14

		processors[i]->sub_problem_w = 0;
		processors[i]->sub_problem_h = i;
		processors[i]->pre_local_len2 = -1;

		//init beach line on each processor:
		processors[i]->sub_problem->block_Beach->InitBeachLine(problem,processors[i]->gMem);	//use admin's problem to init block beach
		for(int k=0;k<block_w_count;k++)
		{
			for(j=0;j<gbd->blockWidths[k].end-gbd->blockWidths[k].start;j++)
			{
				processors[i]->sub_problem->block_Beach->beach[k][j] = gridRows[0][gbd->blockWidths[k].start+j+1];
			}
			processors[i]->sub_problem->block_Beach->beach_front[k] = 0;
		}
		processors[i]->tmp_next_proc = (i+1)%sub_problem_count;
	}

	////////////////////////////////////////////////////////////////
	//4. fill grid cache using each processor:
#ifdef _OPENMP
#pragma omp parallel
#endif
	{
		int k;
		bool isFilled;
#ifndef _OPENMP
		k = 0;
		int nss = 1;
#else		
		k =  omp_get_thread_num();
		int nss = omp_get_num_threads();
#endif
		while(1)
		{
			isFilled = true;
			FillGridCache_each_processor(processors[k]->sub_problem,problem,
					sub_problem_count,isSW,isFilled);
			if(isFilled) break;
		}
	}

	int k;

	////////////////////////////////////////////////////////////////
	//5. get maximum positions from each sub_problem:
	if(isSW)
	{//<transfer>
		for(k=0;k<sub_problem_count;k++)
		{
			for(i=0;i<processors[k]->sub_problem->big_nodes.big_count;i++)
			{
				problem->big_nodes.TryAddBigNode(processors[k]->sub_problem->big_nodes.nodes[i],
					processors[k]->sub_problem->big_nodes.positions[i].i,
					processors[k]->sub_problem->big_nodes.positions[i].j);
			}
		}
		for(int pk=0;pk<problem->global_param->k;pk++)
		{
			printf("score=%d  x=%d y=%d globalstart.i=%d globalstart.j=%d\n",
				problem->big_nodes.nodes[pk].score[0],
				problem->big_nodes.positions[pk].i,
				problem->big_nodes.positions[pk].j,
				problem->big_nodes.nodes[pk].global_start->i,
				problem->big_nodes.nodes[pk].global_start->j
				);
		}//end for pk

	}else
	{//<transfer>
		//the last block is computed on the processor: (block_h_count-1)%sub_problem_count
		int p_n = (sub_problem_count+block_h_count-1)%sub_problem_count;
		//the last node position in the unique processor is saved in path_grid
		int p_j = processors[p_n]->sub_problem->path_grid->end.j - processors[p_n]->sub_problem->path_grid->start.j;
		int p_i = processors[p_n]->sub_problem->path_grid->end.i - processors[p_n]->sub_problem->path_grid->start.i;
		problem->path_grid->end_node = processors[p_n]->sub_problem->pHozScore[p_j];
	}

	return true;
}

//-------------------------------------------
//save the new path grids functions:
bool getPathGrids_sw(ParaSWProblem *problem, ParaSWPath *paths)	//save the new path grids for problem to paths
{
	return getPathGrids(problem,paths,true);
}

bool getPathGrids_nw(ParaSWProblem *problem, ParaSWPath *path)	//save the new path grids for problem to path
{
	return getPathGrids(problem,path,false);
}

bool getPathGrids(ParaSWProblem *problem, ParaSWPath *paths, bool isSW)
{
	int i;
	int start1 = problem->path_grid->start.i;
	int start2 = problem->path_grid->start.j;
	int end1 = problem->path_grid->end.i;
	int end2 = problem->path_grid->end.j;
	Node t_node = problem->path_grid->end_node;	//for nw

	int go = problem->global_param->gap_open;
	int ge = problem->global_param->gap_extend;
	int *seq1 = problem->local_index_seq1;
	int *seq2 = problem->local_index_seq2;
	SubstitutionMatrix *pssm = problem->global_param->pssm;
	int grid_h_count = problem->global_param->grid_height_count;
	int grid_w_count = problem->global_param->grid_width_count;
	int block_h_count = problem->block_division->blockHeights_count;
	int block_w_count = problem->block_division->blockWidths_count;
	Node **gridRows = problem->gridRows;
	Node **gridCols = problem->gridCols;
	Node *cacheNode = &(problem->cacheNode);
	ParaSWGridBlockDivision *gbd =problem->block_division;
	int k_paths = problem->big_nodes.big_count;

	ParaSWPathGrid *pg;
	Node *start_node;
	Node *end_node;
	Position pos;			//current grid's RB position
	Position pre_pos;		//current grid's LU position
	Position global_start;
	bool isOnRows=false;	//for search on grids
	int i_grid=0;
	int j_grid=0;
	int trace_number;		//current trace number
	if(!isSW)	//for NW
	{
		k_paths = 1;
	}
	for(i=0;i<k_paths;i++)
	{
		if(isSW)	//for SW
		{
			trace_number = 0;
			global_start.i = problem->big_nodes.nodes[i].global_start[trace_number].i;
			global_start.j = problem->big_nodes.nodes[i].global_start[trace_number].j;
			pos.i = problem->big_nodes.positions[i].i;
			pos.j = problem->big_nodes.positions[i].j;
			end_node = &(problem->big_nodes.nodes[i]);
		}else		//for NW
		{
			trace_number = problem->path_grid->trace_num;
			global_start.i = start1;//problem->path_grid->start.i;
			global_start.j = start2;//problem->path_grid->start.j;
			pos.i = end1;//problem->path_grid->end.i;
			pos.j = end2;//problem->path_grid->end.j;
			end_node = &t_node;//&(problem->path_grid->end_node);
		}

		//while(pos.i != global_start.i && pos.j != global_start.j)
		while(pos.i != global_start.i || pos.j != global_start.j)		//fix bug 2004.07.28: must use "||"
		{
			//start and end positions and nodes:
			pre_pos.i = end_node->local_start[trace_number].i;
			pre_pos.j = end_node->local_start[trace_number].j;
			Position t_pos;
			if(isSW)	//global position needed!!!!
			{
				t_pos.i = pre_pos.i;
				t_pos.j = pre_pos.j;
			}else		//local position needed!!!!
			{
				t_pos.i = pre_pos.i - global_start.i;
				t_pos.j = pre_pos.j - global_start.j;
			}
			if(pre_pos.i == global_start.i && pre_pos.j == global_start.j)
			{
				start_node = cacheNode;
			}else if(gbd->SearchPositionGrid(t_pos,isOnRows,i_grid,j_grid))
			{
				if(isOnRows)
				{
					start_node = &(gridRows[i_grid][j_grid]);
				}else
				{
					start_node = &(gridCols[j_grid][i_grid]);
				}

			}else
			{
				return false;	//error
			}
			//new path grid:
			pg = new ParaSWPathGrid;
			if(isSW)
				pg->InitPathGrid(i,pre_pos.i,pre_pos.j,pos.i,pos.j,start_node,end_node);	//pg->InitPathGrid(pre_pos.i,pre_pos.j,pos.i,pos.j,start_node,end_node);
			else
				pg->InitPathGrid(problem->path_grid->pathID,pre_pos.i,pre_pos.j,pos.i,pos.j,start_node,end_node);
			pg->trace_num = trace_number;
			pg->delta_score = end_node->score[trace_number] - start_node->score[end_node->trace_start[trace_number]];
			//insert into paths://<transfer>
			paths->AddUnSolvedGrid(pg);	//paths[i].AddUnSolvedGrid(pg);
			//new position:
			trace_number = end_node->trace_start[trace_number];
			pos.i=pre_pos.i;
			pos.j=pre_pos.j;
			end_node = start_node;
		}//end of while
	}//end of for each paths[i]

	//delete previous path grid:
	//need change!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	Release_Object(problem->path_grid);
	return true;
}

//-------------------------------------------
//dispatch available processors function:
/*
bool DispatchProcessors(ParaSWPath *path, ParaSWProcessorSet *pSet, ParaSWReadyTaskArray *rtArray)
{
	if(rtArray->readyTasks_count!=0) return false;	//previous ready tasks must started before re-dispatch
	int task_count = path->unSolvedPathGrids.GetCount();
	int p_count = pSet->processorList.GetCount();
	int count;
	count = task_count<p_count ? task_count:p_count;
	int i;
	for(i=0;i<count;i++)
	{
		rtArray->readyTasks[i].path_grid = path->GetUnSolved();
		rtArray->readyTasks[i].processor_list.Empty();
	}
	for(i=0;i<p_count;i++)
	{
		rtArray->readyTasks[i%count].processor_list.AddHead(pSet->processorList.GetHead());
	}
	rtArray->readyTasks_count = count;
	return true;
}
*/


void AssignPathGridToProcessor(ParaSWPath *paths,
							   int** group_proc,
							   int group_proc_count[],
							   int** group_task,
							   int group_task_count[],
							   int& gCount,
							   int& path_grid_count
							   )
{
	ParaSWPathGrid* pg = paths[0].unSolvedPathGrids.first;
	path_grid_count=0;

	while(pg != NULL)
	{
		path_grid_count++;
		pg = pg->next;
	}

	double* area=new double[path_grid_count];
	double sum_area = 0;

	path_grid_count = 0;
	pg = paths[0].unSolvedPathGrids.first;
	while(pg != NULL)
	{
		path_grid_count++;
		area[path_grid_count-1] = (double)(pg->end.j-pg->start.j)*(pg->end.i-pg->start.i);
		pg = pg->next;
		sum_area += area[path_grid_count-1];
	}

	int proc=0;
	int* mygroup;
	int* task_flag;

	int i;
	int j;


	//area_index is the index array for the path_grid in sort
	int* area_index = new int[path_grid_count];
	for(int u=0;u<path_grid_count;u++)
		area_index[u] = u;

	//sort the path_grid by the area 
	double tmp;
	int tmp_index;
	for(i=0;i<path_grid_count;i++)
	{
		for(j=i+1;j<path_grid_count;j++)
		{
			if(area[i]<area[j])
			{
				tmp = area[i];
				area[i] = area[j];
				area[j] = tmp;
				tmp_index = area_index[i];
				area_index[i] = area_index[j];
				area_index[j] = tmp_index;
			}
		}
	}
	//used_area is the area occupied in each processors
	double* used_area=new double[_num_procs];

	int N = _num_procs;

	int** tmp_task;
	int* tmp_task_count;
	tmp_task = new int*[_num_procs];
	tmp_task_count = new int[_num_procs];
	for(i = 0;i<_num_procs;i++)
	{
		tmp_task[i] = new int[path_grid_count];
		tmp_task_count[i] = 0;
	}

	task_flag = new int[path_grid_count];
	for(i=0;i<path_grid_count;i++)
		task_flag[i]=0;
	for(i=0;i<_num_procs;i++)
		used_area[i] = 0;


	gCount = 0;
	for(i=0;i<path_grid_count;i++)
	{
		int p = (int) (ceil((float)area[i]/sum_area*N));
/*
		if(p>1)
		{
			if((proc+p)>_num_procs)
				p = _num_procs - proc;
			mygroup = new int[p];
			for(j=0;j<p;j++)
			{
				mygroup[j]=proc;
				proc++;
			}
			group_proc[gCount] = mygroup;
			group_proc_count[gCount] = p;
			tmp_task[gCount][0] = area_index[i];
			tmp_task_count[gCount] = 1;
			task_flag[i] = 1;
			used_area[gCount] = area[i];
			N = N-p;
			sum_area = sum_area - area[i];
			gCount ++;
		}
*/
	}
	for(;proc<_num_procs;proc++)
	{
		int p = 1;
		mygroup = new int[p];
		mygroup[0] = proc;
		group_proc[gCount] = mygroup;
		group_proc_count[gCount] = p;
		used_area[gCount]= 0;
		gCount++;
	}
	for(i = 0;i < path_grid_count;i++)
	{
		if(task_flag[i] == 0)
		{
			j = FindMinimalLoad(group_proc,group_proc_count,used_area,gCount);
			tmp_task[j][tmp_task_count[j]] = area_index[i];
			used_area[j] = used_area[j] + area[i];
			tmp_task_count[j] ++;
			task_flag[i] = 1;
		}
	}


	//only for debug
	/*
	fprintf(debugf.debugFile, "GroupCount=%d\n",gCount);
	for(i=0;i<gCount;i++)
	{
		fprintf(debugf.debugFile, "Group %d: used area:%f\n    **",i,used_area[i]);
		for(j=0;j<group_proc_count[i];j++)
		{
			fprintf(debugf.debugFile, "(%d)",group_proc[i][j]);
		}
		fprintf(debugf.debugFile, "Assign Task:",tmp_task_count[i]);
		for(j=0;j<tmp_task_count[i];j++)
		{
			fprintf(debugf.debugFile, "(%d)",tmp_task[i][j]);
		}
		fprintf(debugf.debugFile, "\n");
	}
	*/

	
	for(i=0;i<gCount;i++)
	{
		group_task_count[i] = tmp_task_count[i];
		group_task[i] = new int[group_task_count[i]];
		for(j=0;j<tmp_task_count[i];j++)
		{
			group_task[i][j] = tmp_task[i][j];
		}
	}

	//finalize all the allocated variables 
	for(i = 0;i<_num_procs;i++)
	{
		delete tmp_task[i];
	}
	delete[] tmp_task_count;
	delete	task_flag;
	delete	area;
	delete  area_index;
	delete  used_area;
	return;
}




//-------------------------------------------
//get k-paths from ResultQueue:
bool GetPathsFromResultQueue(ParaSWPath *paths)
{
	ParaSWPathGrid *pg,*pg_pre;
	pg = paths[0].solvedPathGrids.first;
	pg_pre = NULL;
	while(pg!=NULL)
	{
		if(pg->pathID!=0)
		{
			if(pg_pre!=NULL)
			{
				pg_pre->next = pg->next;
			}else
			{
				paths[0].solvedPathGrids.first = pg->next;
			}
			paths[pg->pathID].solvedPathGrids.AddHead(pg);
		}else
		{
			pg_pre = pg;
		}
		if(pg_pre!=NULL)	// fix bug 2004.07.27: each path has just one path_grid
		{
			pg = pg_pre->next;
		}else
		{
			pg = paths[0].solvedPathGrids.first;
		}
	}
	return true;
}

ParaSWPathGrid* FindPathGrid(ParaSWPath* paths,int k)
{
	ParaSWPathGrid* pg = paths[0].unSolvedPathGrids.first;
	int i=0;
	for(i=0;i<k;i++)
	{
		pg = pg->next;
	}
	return pg;
}

//Get the first unsolvable path grid from the paths we are working on
ParaSWPathGrid* GetUnSolvablePathGrid(ParaSWPath* paths,ParaSWParam* param)
{
	if(paths[0].unSolvedPathGrids.IsEmpty())
		return NULL;

	ParaSWPathGrid* pg = paths[0].unSolvedPathGrids.first;
	if(((pg->end.j - pg->start.j) >= param->block_width)
		||((pg->end.i - pg->start.i) >= param->block_height[0]))
	{
		paths[0].unSolvedPathGrids.first = pg->next;
		return pg;
	}
	ParaSWPathGrid* pre_pg = pg;
	pg = pg->next;
	while(pg != NULL)
	{
		if(((pg->end.j - pg->start.j) >= param->block_width)
			||((pg->end.i - pg->start.i) >= param->block_height[0]))
		{
			//ParaSWPathGrid* npg = new ParaSWPathGrid;
			//memcpy(npg,pg,sizeof(ParaSWPathGrid));
			pre_pg->next = pg->next;
			return pg;
		}
		pre_pg = pg;
		pg = pg->next;
	}
	return NULL;
}


void BackwardFindPath(ParaSWPath* paths,ParaSWParam& param)
{

#ifdef _OPENMP
	int _my_rank = omp_get_thread_num();
#else
	int _my_rank = 0;
#endif

	ParaSWPathGrid* path_grid = NULL;
	
	//param.grid_height_count = 2;
	//param.grid_width_count = 2;
	
	int grid_h = param.grid_height_count;
	int grid_w = param.grid_width_count;

	while((path_grid = GetUnSolvablePathGrid(paths,&param)) != NULL)
	{
		param.grid_height_count = grid_h;
		param.grid_width_count = grid_w;

		// printf("Now Processor(%d) are working on the path_grid(%d,%d)->(%d,%d)\n",_my_rank,path_grid->start.i,path_grid->start.j,path_grid->end.i,path_grid->end.j);
		// fflush (stdout);
		// Invoidance of path grid size is smaller block size
		if(((path_grid->end.j - path_grid->start.j) < param.block_width)||
		   ((path_grid->end.i - path_grid->start.i) < param.block_height[0]))
		{
			param.grid_height_count = 2;
			param.grid_width_count = 2;
			// printf("Now Processor(%d) are working on the path_grid(%d,%d)->(%d,%d)\n",_my_rank,path_grid->start.i,path_grid->start.j,path_grid->end.i,path_grid->end.j);
			// printf("Change grid from %d*%d to %d*%d\n",grid_h,grid_w,param.grid_height_count,param.grid_width_count);
		}

		ParaSWProcessorSet processorSet;
		processorSet.InitProcessorSet(&param, path_grid);
		
		ParaSWProcessorSet* pSet = &processorSet;
		
		ParaSWProcessor *localAdminProc;		
		localAdminProc = &(pSet->processors[0]);
		
		ParaSWProblem problem1;	
		ParaSWProblem *problem = &problem1;
		problem->InitLocalProblem(&param,path_grid);

		problem->GetProcessors(pSet->processorList);

		if(problem->IsSolvable())
		{
			printf("********************ERROR!!!!*************\n");
			exit(-1);

			continue;
		}

		problem->AssignGridCache(*(localAdminProc->gMem));	//Get current GridCache of the problem from the local processor's memory
		InitTopLeftData_nw(problem);
		FillGridCache_nw_single_processor(problem);                  //fill grid cache and save partial results to global grid cache

		ParaSWPath* new_paths = new ParaSWPath[param.k];
		getPathGrids_nw(problem, new_paths);				//get k path grids from problem and release previous path_grid	
		int nPath;
		nPath = new_paths[0].unSolvedPathGrids.GetCount();
		ParaSWPathGrid* pg;
		for(int i=0;i<nPath;i++)
		{
			pg = new_paths[0].GetUnSolved();
			paths->AddUnSolvedGrid(pg);
		}

		problem->ReleaseGridCache();
	}

	path_grid = NULL;

	param.grid_height_count = grid_h;
	param.grid_width_count = grid_w;

	ParaSWPathGrid* pg;

	pg = paths[0].unSolvedPathGrids.first;

	int path_grid_count = paths[0].unSolvedPathGrids.GetCount();
	ParaSWPathGrid ** solve_pg;
	for(int i=0;i<path_grid_count;i++)
	{
		int _num_group_procs = 1;

	    //Solve this problem
	    ParaSWPathGrid* mypg = FindPathGrid(paths,i);

		ParaSWPathGrid* prog = new ParaSWPathGrid;
		memcpy(prog,mypg,sizeof(ParaSWPathGrid));

		//param.processor_count = _num_group_procs;
		ParaSWProcessorSet processorSet;
		processorSet.InitProcessorSet(&param, path_grid);
		
		ParaSWProcessorSet* pSet = &processorSet;
		
		ParaSWProcessor *localAdminProc;		
		localAdminProc = &(pSet->processors[0]);

		ParaSWProblem problem1;	
		ParaSWProblem *problem = &problem1;
		problem->InitLocalProblem(&param,prog);
	
		problem->GetProcessors(pSet->processorList);	
	
		if(!problem->IsSolvable())
		{
			printf("****************************Error********************************\n");
		}

		problem->InitMemory_Solvable();	//problem need init memory

		InitTopLeftData_nw(problem);				//init top left data(nw local), according to path_grid start and end

		solve_pg = SolveFullMatrix_nw(problem);	//just fill previous path_grid
		paths->AddSolvedGrids(solve_pg[0]);

		delete[] solve_pg;
	}
}


int FindMyGroup(int* group_proc[],int group_proc_count[],int gCount,int my_rank)
{
	for(int i = 0;i < gCount; i++)
	{
		for(int j = 0;j < group_proc_count[i]; j++)
		{
			if(group_proc[i][j] == my_rank)
				return i;
		}
	}
	return -1;
}
