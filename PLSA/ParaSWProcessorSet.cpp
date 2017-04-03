// ParaSWProcessorSet.cpp: implementation of the ParaSWProcessorSet class.
//
//////////////////////////////////////////////////////////////////////

#include "Common.h"
#include "ParaSWProcessorSet.h"

/*****************************************************************************************************
* Class ParaSWProcessorSet
*	//Main set of all processors
******************************************************************************************************/
#include "ParaSWProblem.h"
ParaSWProcessorSet::ParaSWProcessorSet()
{
	processors = NULL;
	count = 0;
}

ParaSWProcessorSet::~ParaSWProcessorSet()
{
	Release_Mem(processors,count);
}

bool ParaSWProcessorSet::IsRunning()
{
	return (count>processorList.GetCount());
}

bool ParaSWProcessorSet::InitProcessorSet(ParaSWParam *param, ParaSWPathGrid* path_grid)
{
	if(processors!=NULL) return false;		//reallocate not allowed
	int num = param->processor_count;
	int* b_h = param->block_height;
	int b_w = param->block_width;

	count = num;
	processors = new ParaSWProcessor[num];
	for(int i=0;i<count;i++)
	{
		processors[i].InitProcessor(b_h[i],b_w);
		/*
		NOTE:
		 The following regions should be shared among all the processors:
		   (*) Grid Cache  (only stored in processor 0)
		 Whereas, those regions should be distributed separately in all processors:
		   (*) Score Matrix 
		   (*) Beach-line
		*/
		if( i == 0)
			processors[i].gMem->InitGlobalMemory(param, path_grid);
		else
		{
			processors[i].gMem->length_h = param->seq1->length;
			processors[i].gMem->length_w = param->seq2->length;
			processors[i].gMem->grid_h_count = param->grid_height_count;
			processors[i].gMem->grid_w_count = param->grid_width_count;

			processors[i].gMem->gridRows = NULL;
			processors[i].gMem->gridCols = NULL;
			Allocate_Mem_1(processors[i].gMem->beachLine,Node,processors[i].gMem->length_w,processors[i].gMem->beachLineMem);
			//processors[i].gMem->beachLine = NULL;
		}
		processorList.AddHead(&processors[i]);			
	}
	return true;
}

bool ParaSWProcessorSet::AssignProcessors(ParaSWProcessorList &targetList,ParaSWProblem *problem)
{
	if(processors == NULL)	return false;		//no processor
	if(processorList.IsEmpty()) return false;	//no avalible processor
	if(!targetList.IsEmpty()) return false;		//problem has processor

	int height = problem->path_grid->end.i - problem->path_grid->start.i;
	ParaSWProcessorListNode *p;

	while(height>0 && !processorList.IsEmpty())	//greedy assignment
	{
		p = processorList.GetHead();
		targetList.AddHead(p);
		height = height - ( p->p_proc->blockMaxHeight - 1 );
	}

	return true;
}

bool ParaSWProcessorSet::RecycleProcessors(ParaSWProcessorList &targetList)
{
	ParaSWProcessorListNode *p;
	while(!targetList.IsEmpty())
	{
		p = targetList.GetHead();
		processorList.AddHead(p);
	}
	return true;
}

/*****************************************************************************************************
* Class ParaSWProcessor
*	//Each processor's ability.
*	//allocate memory at the initialize cluster step.
******************************************************************************************************/
#include "ParaSWProblem.h"

ParaSWProcessor::ParaSWProcessor()
{
	blockMaxHeight = 0;
	blockMaxWidth = 0;

	processorID = processorCount;
	processorCount++;

	ScoreMatrix = NULL;
	pVerScore = NULL;	// eric, 2004.10.14
	pHozScore = NULL;	// eric, 2004.10.14

	gMem = new ParaSWGlobalMemory;

	sub_problem = new ParaSWProblem;			//once
	sub_problem->block_Beach = new BeachLine;	//once
	path_grid = new ParaSWPathGrid;				//once
	sub_problem_h = -1;
	sub_problem_w = -1;		//the current block number (h,w) of this sub problem
	pre_local_len2 = 0;
}

ParaSWProcessor::~ParaSWProcessor()
{
	Release_Mem (pHozScore, blockMaxWidth);	// eric, 2004.10.14
	Release_Mem (pVerScore, blockMaxHeight);	// eric, 2004.10.14
	// alignFree (pHozScore);
	// alignFree (pVerScore);

	Release_Mem2D_0(ScoreMatrix,blockMaxHeight,blockMaxWidth);
	Release_Object(gMem);
	Release_Object(sub_problem);
	Release_Object(path_grid);
}

bool ParaSWProcessor::InitProcessor(int h,int w)
{
	if(ScoreMatrix!=NULL) return false;	//reallocate not allowed
	if (pHozScore != NULL || pVerScore != NULL) return false;	// eric, 2004.10.14

	blockMaxHeight = h;
	blockMaxWidth = w;

	Allocate_Mem2D(ScoreMatrix,Node,h,w);

	Allocate_Mem(pHozScore, Node, w);	// eric, 2004.10.14
	Allocate_Mem(pVerScore, Node, h);	// eric, 2004.10.14
	// pHozScore = (Node *)alignMalloc(sizeof(Node) * w);
	// pVerScore = (Node *)alignMalloc(sizeof(Node) * h);

	return true;
}

bool ParaSWProcessor::IsSolvable(int h,int w)
{
	return (h<=blockMaxHeight)&&(w<=blockMaxWidth);
}

/*****************************************************************************************************
* Class ParaSWProcessorList
*	// List of ParaSWProcessor
*	// Contains:
*	//		get head method
*	//		add tail method
******************************************************************************************************/
ParaSWProcessorList::ParaSWProcessorList()
{
	first = NULL;
}

ParaSWProcessorList::~ParaSWProcessorList()
{
	Empty();
}

bool ParaSWProcessorList::IsEmpty()
{
	return (first == NULL);
}

void ParaSWProcessorList::Empty()
{
	ParaSWProcessorListNode *p;
	p = first;
	while(p!=NULL)
	{
		p = first->next;
		delete first;
		first = p;
	}
}

ParaSWProcessorListNode* ParaSWProcessorList::GetHead()
{
	if(first==NULL) return NULL;
	ParaSWProcessorListNode *p;
	p = first;
	first = first->next;
	p->next = NULL;

	return p;
}

ParaSWProcessorListNode* ParaSWProcessorList::AddHead(ParaSWProcessorListNode *pn)
{
	pn->next = first;
	first = pn;

	return first;
}

ParaSWProcessorListNode* ParaSWProcessorList::AddHead(ParaSWProcessor *pp)
{
	ParaSWProcessorListNode *p = new ParaSWProcessorListNode;
	p->p_proc = pp;
	p->next = first;
	first = p;

	return p;
}

int ParaSWProcessorList::GetCount()
{
	int count=0;
	ParaSWProcessorListNode *p = first;
	while(p!=NULL)
	{
		count++;
		p=p->next;
	}
	return count;
}




