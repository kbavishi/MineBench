#ifndef __Parallel_SW_Algorithm__
#define __Parallel_SW_Algorithm__

#include "Common.h"
#include "ParaSWParam.h"
class ParaSWProblem;
class ParaSWPath;
class ParaSWPathGrid;
class ParaSWProcessor;
class ParaSWProcessorSet;
class ParaSWReadyTask;
class ParaSWReadyTaskArray;
class ParaSWGlobalMemory;

//-------------------------------------------
//abstract functions:
void initClusters(ParaSWParam &param);
//-------------------------------------------

//-------------------------------------------
//actrual functions:
ParaSWPath* ParaSW(ParaSWParam &param);		//main algorithm
//-------------------------------------------

//-------------------------------------------
//-------------------------------------------
//core functions: solve full matrix
ParaSWPathGrid** SolveFullMatrix_sw(ParaSWProblem *problem);	//result is saving in the path_grid
ParaSWPathGrid** SolveFullMatrix_nw(ParaSWProblem *problem);
ParaSWPathGrid** SolveFullMatrix(ParaSWProblem *problem,bool isSW);

//-------------------------------------------
//core functions: fill grid cache
bool FillGridCache_sw(ParaSWProblem *problem);
bool FillGridCache_nw(ParaSWProblem *problem);
bool FillGridCache(ParaSWProblem *problem,bool isSW);
bool FillGridCache_single_processor(ParaSWProblem *problem,bool isSW);
bool FillGridCache_each_processor(ParaSWProblem *sub_problem,ParaSWProblem *problem,int sub_problem_count,bool isSW,bool &isFilled);
bool FillGridCache_implement(ParaSWProcessor *proc,bool isWidthOnGrid,bool isHeightOnGrid,bool isSW);

//-------------------------------------------
//init first row and column functions:
bool InitTopLeftData_sw(ParaSWProblem *problem);	//init top left data(sw global)
bool InitTopLeftData_nw(ParaSWProblem *problem);	//init top left data(nw local), according to path_grid start and end

//-------------------------------------------
//allocate grid cache functions:
//bool AllocateGridCache(ParaSWProblem *problem);		//allocate grid cache for problem

//-------------------------------------------
//save the new path grids functions:
bool getPathGrids_sw(ParaSWProblem *problem, ParaSWPath *paths);	//save the new path grids for problem to paths
bool getPathGrids_nw(ParaSWProblem *problem, ParaSWPath *path);	//save the new path grids for problem to path
bool getPathGrids(ParaSWProblem *problem, ParaSWPath *paths, bool isSW);

//-------------------------------------------
//dispatch available processors function:
//bool DispatchProcessors(ParaSWPath *path, ParaSWProcessorSet *pSet, ParaSWReadyTaskArray *rtArray);

//-------------------------------------------
//do within a set of processors / a sub problem
//bool ProcessSubProblem(ParaSWParam &param,ParaSWReadyTask *pRT,ParaSWPath *path,ParaSWProcessorSet *pSet,bool isSW);

//-------------------------------------------
//get k-paths from ResultQueue:
bool GetPathsFromResultQueue(ParaSWPath *paths);


void ForwardFillCache(ParaSWParam &param,ParaSWPathGrid* path_grid,ParaSWProcessorSet *pSet,ParaSWPath *path);
void BackwardFindPath(ParaSWPath* paths,ParaSWParam& param);
void BackwardFindPathsForHugeBlock(ParaSWPath* paths, ParaSWProcessorSet *pSet, ParaSWParam& param);
bool IsTaskPartition(ParaSWPath *paths);
void AssignPathGridToProcessor(ParaSWPath *paths,int** group_proc,int group_proc_count[],int** group_task,int group_task_count[],int& gCount,int& path_grid_count);
int FindMyGroup(int* group_proc[],int group_proc_count[],int gCount,int my_rank);
ParaSWPathGrid* FindPathGrid(ParaSWPath* paths,int k);
ParaSWPathGrid* GetUnSolvablePathGrid(ParaSWPath* paths,ParaSWParam* param);
#endif