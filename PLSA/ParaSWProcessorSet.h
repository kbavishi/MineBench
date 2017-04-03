// ParaSWProcessorSet.h: interface for the ParaSWProcessorSet class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PARASWPROCESSORSET_H__D208B8CF_855C_43F7_BBA3_5BCFDF32E5FD__INCLUDED_)
#define AFX_PARASWPROCESSORSET_H__D208B8CF_855C_43F7_BBA3_5BCFDF32E5FD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/*****************************************************************************************************
* Class ParaSWProcessor
*	//Each processor's ability.
*	//allocate memory at the initialize cluster step.
******************************************************************************************************/
class Node;
class ParaSWProblem;
class ParaSWPathGrid;
class ParaSWGlobalMemory;

class ParaSWProcessor
{
public:
	ParaSWProcessor();
	~ParaSWProcessor();

public:
	bool InitProcessor(int h,int w);

public:
	bool IsSolvable(int h,int w);		//test if the size (h,w) is solvable

public:
	int blockMaxHeight;		//max height of block, according to the speed of the processor
	int blockMaxWidth;		//max width of block, according to the memory of the processor

	int processorID;		//ID of this processor
	static int processorCount;		//global count of processors

public:
	Node *pHozScore;				// eric, 2004.10.14
	Node *pVerScore;				// eric, 2004.10.14

	Node **ScoreMatrix;			//temporary storage: blockMaxHeight * blockMaxWidth
	ParaSWGlobalMemory *gMem;	//temporary storage for grid cache and beach line on each processor

public:
	ParaSWProblem *sub_problem;
	ParaSWPathGrid *path_grid;
	int sub_problem_h;
	int sub_problem_w;		//the current block number (h,w) of this sub problem
	int pre_local_len2;		//store the previous local length for fill block

public:
	int tmp_next_proc;		//pos of next processor in a local processor list
};

/*****************************************************************************************************
* struct ParaSWProcessorListNode
*	// List node of ParaSWProcessor
*	// Contains:
*	//		pointer to a processor
*	//		pointer to next node
******************************************************************************************************/
struct ParaSWProcessorListNode
{
	ParaSWProcessor *p_proc;
	ParaSWProcessorListNode *next;
};

/*****************************************************************************************************
* Class ParaSWProcessorList
*	// List of ParaSWProcessor
*	// Contains:
*	//		get head method
*	//		add tail method
******************************************************************************************************/
class ParaSWProcessorList
{
public:
	ParaSWProcessorList();
	~ParaSWProcessorList();

public:
	bool IsEmpty();		//test the list
	void Empty();		//empty the list, only release memory of node, not memory of processor
	ParaSWProcessorListNode* GetHead();								//remove head of the list and return the pointer
	ParaSWProcessorListNode* AddHead(ParaSWProcessorListNode *pn);	//add an element to the list
	ParaSWProcessorListNode* AddHead(ParaSWProcessor *pp);			//add an element to the list
	int GetCount();		//count the processors in the list

public:
	ParaSWProcessorListNode *first;	//first pointer of the list
};

/*****************************************************************************************************
* Class ParaSWProcessorSet
*	//Main set of all processors
******************************************************************************************************/
class ParaSWProblem;
class ParaSWParam;
class ParaSWProcessorSet
{
public:
	ParaSWProcessorSet();
	~ParaSWProcessorSet();

public:
	bool InitProcessorSet(ParaSWParam *param, ParaSWPathGrid* path_grid);	//allocate processors

	bool AssignProcessors(ParaSWProcessorList &targetList,ParaSWProblem *problem);	//assign processors to a problem
	bool RecycleProcessors(ParaSWProcessorList &targetList);						//recycle processors

	bool IsRunning();		//test if any processor is running

public:
	ParaSWProcessor *processors;
	int count;

	ParaSWProcessorList processorList;
};

#endif // !defined(AFX_PARASWPROCESSORSET_H__D208B8CF_855C_43F7_BBA3_5BCFDF32E5FD__INCLUDED_)
