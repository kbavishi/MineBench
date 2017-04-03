// ParaSWPath.h: interface for the ParaSWPath class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PARASWPATH_H__6E9522BB_E22B_4CAB_A637_5CBC9986F72D__INCLUDED_)
#define AFX_PARASWPATH_H__6E9522BB_E22B_4CAB_A637_5CBC9986F72D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/*****************************************************************************************************
* Struct Point
******************************************************************************************************/
struct Point{
	int i;
	int j;
	bool operator < (Point &p) { return (i<=p.i && j<=p.j && !(i==p.i && j==p.j)); };
};

/*****************************************************************************************************
* Class ParaSWPathGrid
*	// Path for each grid, the unit of result path
*	// if Solved:
*	//		save the direction of the path, from end to start point
*	// if UnSolved:
*	//		don't save the direction of the path
******************************************************************************************************/
//define for direction:
#define UPLEFT	1
#define LEFT	2
#define UP		3
//define for output alignment:
#define CHAR_GAP	'-'
#define CHAR_UNPAIR	' '
#define CHAR_END	'='
class Node;
#include "ParaSWProblem.h"
class ParaSWPathGrid
{
public:
	ParaSWPathGrid();
	~ParaSWPathGrid();

	bool InitPathGrid(int start_i,int start_j,
					int end_i,int end_j,
					Node *s_node,Node *e_node);
	bool InitPathGrid(int path_id,int start_i,int start_j,
					int end_i,int end_j,
					Node *s_node,Node *e_node);
	bool AllocatePath();

	bool TrySimpleSolvePathGrid();	//deal with the degenerate path grid, only use in nw

public:
	bool IsSolved;		//solve status for this grid

	Point start;		//local start point (h,w) of global path, (h,w) is the position in the logical score matrix, 0 based
	Point end;			//local end point (h,w) of global path
	Node start_node;	//start Node
	Node end_node;		//end Node
	int trace_num;		//trace from which status of the end node
	
	char *direction;	//direction_len = points_len-1, from end to start point
		//length of the memory of direction is the max length: (end.i-start.i+end.j-start.j)
	int points_len;		//point length of the local path

	int delta_score;	//score_end - score_start

	int pathID;			//this path grid belongs to which path

	ParaSWPathGrid *next;	//for list
};

/*****************************************************************************************************
* Class ParaSWPathGridList
*	// List of ParaSWPathGrid
*	// Contains:
*	//		get head method
*	//		add head method
******************************************************************************************************/
class ParaSWPathGridList
{
public:
	ParaSWPathGridList();
	~ParaSWPathGridList();

public:
	bool IsEmpty();				//test the list
	ParaSWPathGrid* GetHead();	//remove head of the list and return the pointer
	ParaSWPathGrid* AddHead(ParaSWPathGrid *pg);	//add an element to the list
	ParaSWPathGrid* AddHead(ParaSWPathGridList *pl);	//add a list to the list, not used

	int GetCount();

public:
	ParaSWPathGrid *first;	//first pointer of the list
};

/*****************************************************************************************************
* Class ParaSWPath
*	// Result of the alignment, one of the k-opimal paths.
*	// Contains:
*	//		One List of UnSolved path grids
*	//		A solution of joint the solved paths
******************************************************************************************************/
class Sequence;
class ParaSWPath  
{
public:
	ParaSWPath();
	~ParaSWPath();

public:
	bool IsSolved();				//test if no tasks
	bool IsTaskQueueEmpty();		//test if no tasks

	ParaSWPathGrid* GetUnSolved();		//get task from un-solvable tasks
	ParaSWPathGrid* PopFromTaskQueue();	//get task from un-solvable tasks
	void AddUnSolvedGrid(ParaSWPathGrid* pg);
	void PushIntoTaskQueue(ParaSWPathGrid* pg);
	void AddSolvedGrids(ParaSWPathGrid* pg);
	void PushIntoResultQueue(ParaSWPathGrid* pg);

public:
	ParaSWPathGridList unSolvedPathGrids;		//tasks for fill grid cache
	ParaSWPathGridList solvedPathGrids;			//result

public:
	//this part could be done as a global function
	bool SortSolvedPath();	//sort solved path in a global end->start order
	bool InitPath();		//set seq1,seq2,ali to 0
	bool JointAllPath(Sequence *s1,Sequence *s2);	//get seq1,seq2,ali for output
public:
	char* seq1;		//the final result for out put
	char* seq2;
	char* ali;
	int length;		//length of seq1[], seq2[], and ali[]
	int seq1_start;	//start of seq1, 1 base
	int seq2_start;
	int seq1_end;
	int seq2_end;
};

#endif // !defined(AFX_PARASWPATH_H__6E9522BB_E22B_4CAB_A637_5CBC9986F72D__INCLUDED_)
