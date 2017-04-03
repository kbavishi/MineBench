// ParaSWProblem.h: interface for the ParaSWProblem class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PARASWPROBLEM_H__A96EAE84_71F1_4BBC_95AB_86AA978C1B11__INCLUDED_)
#define AFX_PARASWPROBLEM_H__A96EAE84_71F1_4BBC_95AB_86AA978C1B11__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/*****************************************************************************************************
* struct Position
*	// Position in the score matrix
******************************************************************************************************/
struct Position
{
public:
	int i;
	int j;
};

/*****************************************************************************************************
* Class Node
*	// element in the score matrix
******************************************************************************************************/
#include <stdarg.h>
//define for node length:
#define	Node_length	3
class Node
{
public:
	Node();
 	~Node();

	bool operator = (Node& n1);
	bool operator != (Node& n1);		// eric, 2004.10.14
	
	//set score and trace:
	bool SetData(int k, ... );	//set first k scores and traces
	bool SetOneData(int n,int s,char t);

	//set 
	bool SetStartPosition(bool isGlobal, ... );							//for lin & aff
	bool SetStartPositionHere(int h,int w);								//for lin & aff
	bool SetOneStartPosition(int n,bool isGlobal,int i,int j,char k=0);	//for aff
	bool SetOneStartPosition(int n,Node &preNode,int m);				//for aff

	//for initialization:
	void Init_nw_10(int s0,char t2,int st0h,int st0w,char ts0);
	void Init_nw_20(int s0,int st0h,int st0w,char ts0);
	void Init_nw_01(int s0,char t1,int st0h,int st0w,char ts0);
	void Init_nw_02(int s0,int st0h,int st0w,char ts0);

public:
	int score[Node_length];			//Value: H(max),E(left),F(top)
	Position global_start[Node_length];	//Start position: Sh,Se,Sf in global matrix, for computing the k-optimal paths //all the positions are position in score matrix, 0 based
	Position local_start[Node_length];	//Start position: Sh,Se,Sf in local block
	char trace_start[4];		//Local start tracing direction: local_start[i] come from which status
	char trace[4];
	// char trace[Node_length];			//Path tracing direction: score[i] come from which status: H,E,F
	// char trace_start[Node_length];		//Local start tracing direction: local_start[i] come from which status
};

/*****************************************************************************************************
* Class NodeArray
*	// big node array for the main problem
******************************************************************************************************/
class NodeArray
{
public:
	NodeArray();
	~NodeArray();

	bool InitNodeArray(int k);		//allocate memory for the node array
	bool TryAddBigNode(Node &n,int h,int w);	//try to add a big node to the array

	Node * nodes;
	Position * positions;			//position for each node
	int node_count;

	int big_count;
};

/*****************************************************************************************************
* Class BeachLine
*	// beachline for blocks
*	// only save one row, that is enough
*	// the total length of beach line is length of seq2
*	// every part of beach line is start at +1 position of score matrix
******************************************************************************************************/
class ParaSWProblem;
class ParaSWGlobalMemory;
class BeachLine
{
public:
	BeachLine();
	~BeachLine();

	bool InitBeachLine(ParaSWProblem *problem,ParaSWGlobalMemory *gMem);	//init the beach line for a problem, memory get from a global memory
	bool InitBeachLineNodes(ParaSWProblem *problem,ParaSWGlobalMemory *gMem);
	bool InitBeachLineFront(ParaSWProblem *problem,ParaSWGlobalMemory *gMem);

public:
	Node **beach;			// the beach line of row
	int *beach_front;		// the beach front of each beach row, for the first line is 0, number of block
	int block_width_count;
	int block_width;
};
/*****************************************************************************************************
* Class ParaSWReadyTask
*	// task with assigned processors
******************************************************************************************************/
class ParaSWPathGrid;

#include "ParaSWProcessorSet.h"

class ParaSWReadyTask
{
public:
	ParaSWPathGrid *path_grid;	//local start and end points for this problem, point to outside memory
	ParaSWProcessorList processor_list;	//list of processors, by the processor set
};
/*****************************************************************************************************
* Class ParaSWReadyTaskArray
*	// task array with assigned processors
******************************************************************************************************/
class ParaSWReadyTaskArray
{
public:
	ParaSWReadyTaskArray();
	~ParaSWReadyTaskArray();
	bool InitReadyTaksArray(int p_count);
	ParaSWReadyTask *GetOneReadyTask();

public:
	ParaSWReadyTask *readyTasks;
	int readyTasks_count;
	int total_number;
};
/*****************************************************************************************************
* Class ParaSWProblem
*	// problem from top to bottom level
*	// algorithm:
*	//		main problem:	SW
*	//		sub problem:	NW
*	// storage before fill grid cache step:
*	//		solvable problem:		cacheRow & cacheCol
*	//		un-solvable problem:	gridRows & gridCols & block_division & block_Beach
*	// storage in fill grid cache step:
*	//		solvable problem:		no need fill grid cache
*	//		un-solvable problem:	gridRows & gridCols & block_division & block_Beach
*	// storage in solve path step:
*	//		solvable problem:		full local matrix
*	//		un-solvable problem:	no need solve path
******************************************************************************************************/
class ParaSWParam;
class Sequence;
class ParaSWPathGrid;

class ParaSWGridBlockDivision;
class BeachLine;
class ParaSWGlobalMemory;

#define MAX_GRID_ROWS	128
#define MAX_GRID_COLS	128

#include "ParaSWParam.h"
#include "ParaSWPath.h"
#include "ParaSWProcessorSet.h"
#include "ParaSWGlobalMemory.h"

class ParaSWProblem  
{
public:
	ParaSWProblem();
	virtual ~ParaSWProblem();

public:
	bool InitProblem();
	bool InitMainProblem(ParaSWParam *param,ParaSWPathGrid *pg);	//init problem to local problem, PathGird gives the offset
	bool InitLocalProblem(ParaSWParam *param,ParaSWPathGrid *pg);	//init problem to local problem, PathGird gives the offset

	bool TryGetProcessors(ParaSWProcessorSet &pSet);	//if successful, then do this problem
	bool GetProcessors(ParaSWProcessorList &pList);		//get processors assign to this problem. if successful, then do this problem
	bool SendBackProcessors(ParaSWProcessorSet &pSet);	//after fill cache or solve matrix, return processors
	bool ReleaseProblem();
	bool ReleaseGridCache();

	bool InitMemory_Solvable();	//init memory for solvable main problem
	bool AssignGridCache(ParaSWGlobalMemory &gMem);	//assign grid cache for problem

	bool FillBlock(bool isSW);				//fill block after scoreMatrix has assigned and initialized
	bool FillBlock_sw();		//fill block using sw, for global matrix, after scoreMatrix has assigned and initialized
	bool FillBlock_nw();		//fill block using nw, for local matrix, after scoreMatrix has assigned and initialized


	bool ResetBlockLocalCol(bool isSW);		//for first column on grid, reset local_start for first column, before fill block
	bool ResetBlockLocalRow(bool isSW);		//for first row on grid, reset local_start for first row, before fill block

public:
	bool IsMainProblem();
	bool IsSolvable();
	bool IsBlockProblem();

public:
	bool b_isMainProblem;	//if is main problem, us SW; else us NW
	bool b_isSolvable;		//if this problem solvable
	bool b_isBlockProblem;	//if is block problem, just use in the fill grid cache step

	NodeArray big_nodes;	//save the biggest nodes, for main problem only
	
	ParaSWParam *global_param;	//global parameters, point to outside memory
	int *local_index_seq1;	//local sequence 1 index pointer, point to outside memory
	int *local_index_seq2;	//local sequence 2 index pointer, point to outside memory

	ParaSWPathGrid *path_grid;	//local start and end points for this problem, point to outside memory

	ParaSWProcessorList processor_list;	//list of processors, by the processor set
	ParaSWProcessor **processors;	//pointer of each processor

public:
	//if problem unSolved:
	ParaSWGridBlockDivision *block_division;	//the division of the blocks, for unSolved problems
	BeachLine *block_Beach;			//beach line of blocks
//	BeachLine *block_Beach_Admin;	//beach line of blocks in local admin processor, only need beach front
	Node *gridRows[MAX_GRID_ROWS];
	Node *gridCols[MAX_GRID_COLS];
	Node cacheNode;				//left up point

public:
	//if problem solvable:
	Node **scoreMatrix;			//whole score matrix
	Node *pHozScore;				// eric, 2004.10.14
	Node *pVerScore;				// eric, 2004.10.14
};

#endif // !defined(AFX_PARASWPROBLEM_H__A96EAE84_71F1_4BBC_95AB_86AA978C1B11__INCLUDED_)
