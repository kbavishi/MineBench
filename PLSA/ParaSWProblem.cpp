// ParaSWProblem.cpp: implementation of the ParaSWProblem class.
//
//////////////////////////////////////////////////////////////////////

#include "Common.h"
#include "ParaSWProblem.h"
#include "Debug.h"

extern CDebug debugf;
int bBackSeparate;

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
ParaSWProblem::ParaSWProblem()
{
	InitProblem();
}

ParaSWProblem::~ParaSWProblem()
{
	ReleaseProblem();
}

bool ParaSWProblem::IsMainProblem()
{
	return b_isMainProblem;
}
bool ParaSWProblem::IsSolvable()
{
	return b_isSolvable;
}
bool ParaSWProblem::IsBlockProblem()
{
	return b_isBlockProblem;
}

bool ParaSWProblem::InitProblem()
{
	b_isMainProblem = true;
	b_isSolvable = false;
	b_isBlockProblem = false;

	global_param = NULL;		//point to outside memory
	local_index_seq1 = NULL;	//point to outside memory
	local_index_seq2 = NULL;	//point to outside memory

	path_grid = NULL;			//point to outside memory

	block_division = NULL;
	block_Beach = NULL;
//	block_Beach_Admin = NULL;

	processors = NULL;
	return true;
}

bool ParaSWProblem::InitMainProblem(ParaSWParam *param,ParaSWPathGrid *pg)
{
	b_isMainProblem = true;
	//get global parameter:
	global_param = param;
	//get local problem:
	path_grid = pg;			//decide outside
	
	//set local sequence:	// sequences contain the first row/column of the local matrix!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	local_index_seq1 = global_param->seq1->index + path_grid->start.i - 1;
	local_index_seq2 = global_param->seq2->index + path_grid->start.j - 1;

	//if is SW problem, should have node count:
	big_nodes.InitNodeArray(param->k);
	
	//don't know solvable, must wait for the processor assign to this problem

	return true;
}

bool ParaSWProblem::InitLocalProblem(ParaSWParam *param,ParaSWPathGrid *pg)
{
	b_isMainProblem = false;
	//get global parameter:
	global_param = param;
	//get local problem:
	path_grid = pg;			//decide outside
	
	//set local sequence:	// sequences contain the first row/column of the local matrix!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	local_index_seq1 = global_param->seq1->index + path_grid->start.i - 1;
	local_index_seq2 = global_param->seq2->index + path_grid->start.j - 1;

	//if is SW problem, should have node count:
	big_nodes.InitNodeArray(param->k);
	
	//don't know solvable, must wait for the processor assign to this problem

	return true;
}

bool ParaSWProblem::InitMemory_Solvable()
{//solvable problem:		full local matrix
	ParaSWProcessorListNode *p;
	p = processor_list.first;
	scoreMatrix = p->p_proc->ScoreMatrix;

	pHozScore = p->p_proc->pHozScore;		// eric 2004.10.14
	pVerScore = p->p_proc->pVerScore;		// eric 2004.10.14
	
	b_isBlockProblem = true;
	return true;
}

bool ParaSWProblem::ResetBlockLocalCol(bool isSW)		//for first column on grid, reset local_start for first column, before fill block
{
	int i;
	int local_len1 = path_grid->end.i-path_grid->start.i;
	int local_start1 = path_grid->start.i;
	int local_start2 = path_grid->start.j;
	int t;

	t = pVerScore[0].trace[0] - 1;
	t = t>0 ? t:0;
	pHozScore[0].SetOneStartPosition(0,false,local_start1,local_start2,t);
	pHozScore[0].SetOneStartPosition(1,false,local_start1,local_start2,1);
	pHozScore[0].SetOneStartPosition(2,false,local_start1,local_start2,2);
	pVerScore[0].SetOneStartPosition(0,false,local_start1,local_start2,t);
	pVerScore[0].SetOneStartPosition(1,false,local_start1,local_start2,1);
	pVerScore[0].SetOneStartPosition(2,false,local_start1,local_start2,2);

	for(i=1;i<=local_len1;i++)
	{
		//E:
		pVerScore[i].SetOneStartPosition(1,false,local_start1+i,local_start2,1);
		//F:
		t = pVerScore[i].trace[2]-1;
		t = t>0 ? t:0;
		pVerScore[i].SetOneStartPosition(2,false,
			pVerScore[i-1].local_start[t].i,
			pVerScore[i-1].local_start[t].j,
			pVerScore[i-1].trace_start[t]);
		//H:
		t = pVerScore[i].trace[0]-1;
		t = t>0 ? t:0;
		if(t==2)
		{
			pVerScore[i].SetOneStartPosition(0,false,
				pVerScore[i].local_start[t].i,
				pVerScore[i].local_start[t].j,
				pVerScore[i].trace_start[t]);
		}else
		{
			pVerScore[i].SetOneStartPosition(0,false,local_start1+i,local_start2,t);
		}
	}
	return true;
}

bool ParaSWProblem::ResetBlockLocalRow(bool isSW)		//for first row on grid, reset local_start for first row, before fill block
{
	int j;
	int local_len2 = path_grid->end.j-path_grid->start.j;
	int local_start1 = path_grid->start.i;
	int local_start2 = path_grid->start.j;
	int t;

	//first point:
	t = pVerScore[0].trace[0] - 1;
	t = t>0 ? t:0;
	pHozScore[0].SetOneStartPosition(0,false,local_start1,local_start2,t);
	pHozScore[0].SetOneStartPosition(1,false,local_start1,local_start2,1);
	pHozScore[0].SetOneStartPosition(2,false,local_start1,local_start2,2);
	pVerScore[0].SetOneStartPosition(0,false,local_start1,local_start2,t);
	pVerScore[0].SetOneStartPosition(1,false,local_start1,local_start2,1);
	pVerScore[0].SetOneStartPosition(2,false,local_start1,local_start2,2);
	
	for(j=1;j<=local_len2;j++)
	{
		//E:
		t = pHozScore[j].trace[1]-1;
		t = t>0 ? t:0;
		pHozScore[j].SetOneStartPosition(1,false,
			pHozScore[j-1].local_start[t].i,
			pHozScore[j-1].local_start[t].j,
			pHozScore[j-1].trace_start[t]);
		//F:
		pHozScore[j].SetOneStartPosition(2,false,local_start1,local_start2+j,2);
		//H:
		t = pHozScore[j].trace[0]-1;
		t = t>0 ? t:0;
		if(t==1)
		{
			pHozScore[j].SetOneStartPosition(0,false,
				pHozScore[j].local_start[t].i,
				pHozScore[j].local_start[t].j,
				pHozScore[j].trace_start[t]);
		}else
		{
			pHozScore[j].SetOneStartPosition(0,false,local_start1,local_start2+j,t);
		}
	}
	return true;
}

bool ParaSWProblem::FillBlock_sw()
{
	int i;
	int j;
	int local_len1 = path_grid->end.i-path_grid->start.i;
	int local_len2 = path_grid->end.j-path_grid->start.j;
	int global_i;
	int global_j;
	int sc;
	
	int t0;
	int t1;
	int t2;

	//fill interior:
	Node upleft, upleft_prev, up, left;
	for(i=1;i<=local_len1;i++)
	{
		upleft_prev = pVerScore[i-1];
		pVerScore[i-1] = pHozScore[local_len2];
		left = pVerScore[i];
		
		for(j=1;j<=local_len2;j++)
		{
			global_i = path_grid->start.i + i;
			global_j = path_grid->start.j + j;

			sc = global_param->pssm->score[local_index_seq1[i]][local_index_seq2[j]];
			
			upleft = upleft_prev;
			upleft_prev = pHozScore[j];
			up = pHozScore[j];
			
			//E:
			t0 = left.score[0] + global_param->gap_open;	//H[i][j-1]
			t1 = left.score[1] + global_param->gap_extend;	//E[i][j-1]
			if(t0>=t1)
			{
				pHozScore[j].SetOneData(1,t0,TRACE_s_AFF_H);
				pHozScore[j].SetOneStartPosition(1,left,0);
			}
			else
			{
				pHozScore[j].SetOneData(1,t1,TRACE_s_AFF_E);
				pHozScore[j].SetOneStartPosition(1,left,1);
			}
			//F:
			t0 = up.score[0] + global_param->gap_open;
			t2 = up.score[2] + global_param->gap_extend;
			if(t0>=t2)
			{
				pHozScore[j].SetOneData(2,t0,TRACE_s_AFF_H);
				pHozScore[j].SetOneStartPosition(2,up,0);
			}
			else
			{	
				pHozScore[j].SetOneData(2,t2,TRACE_s_AFF_F);
				pHozScore[j].SetOneStartPosition(2,up,2);
			}
			//H:
			t0 = upleft.score[0] + sc;
			t1 = pHozScore[j].score[1];
			t2 = pHozScore[j].score[2];
			if(0>=t0 && 0>=t1 && 0>=t2)
			{
				pHozScore[j].SetOneData(0,0,TRACE_s_AFF_HERE);
				pHozScore[j].SetOneStartPosition(0,true,global_i,global_j);
				pHozScore[j].SetOneStartPosition(0,false,global_i,global_j,0);
			}else if(t0>=t1 && t0>=t2)
			{
				pHozScore[j].SetOneData(0,t0,TRACE_s_AFF_H);
				pHozScore[j].SetOneStartPosition(0,upleft,0);
			}
			else if(t1>=t2)
			{
				pHozScore[j].SetOneData(0,t1,TRACE_s_AFF_E);
				pHozScore[j].SetOneStartPosition(0,pHozScore[j],1);
			}
			else
			{
				pHozScore[j].SetOneData(0,t2,TRACE_s_AFF_F);
				pHozScore[j].SetOneStartPosition(0,pHozScore[j],2);
			}
			
			//update maximum score and position:
			big_nodes.TryAddBigNode(pHozScore[j],global_i,global_j);

			left = pHozScore[j];
		}//end of for j
	}//end of for i

	pVerScore[local_len1] = pHozScore[local_len2];

	return true;
}

bool ParaSWProblem::FillBlock_nw()
{
	int i;
	int j;
	int local_len1 = path_grid->end.i-path_grid->start.i;
	int local_len2 = path_grid->end.j-path_grid->start.j;
	int sc;
	
	int t0;
	int t1;
	int t2;
	
	//fill interior:
	Node upleft, upleft_prev, up, left;
	for(i=1;i<=local_len1;i++)
	{
		upleft_prev = pVerScore[i-1];
		pVerScore[i-1] = pHozScore[local_len2];
		left = pVerScore[i];
		
		for(j=1;j<=local_len2;j++)
		{
			upleft = upleft_prev;
			upleft_prev = pHozScore[j];
			up = pHozScore[j];
			
			sc = global_param->pssm->score[local_index_seq1[i]][local_index_seq2[j]];
			
			//E:
			t0 = left.score[0] + global_param->gap_open;	//H[i][j-1]
			t1 = left.score[1] + global_param->gap_extend;	//E[i][j-1]
			
			if(t0>=t1)
			{
				pHozScore[j].SetOneData(1,t0,TRACE_s_AFF_H);
				pHozScore[j].SetOneStartPosition(1,left,0);
			}
			else
			{
				pHozScore[j].SetOneData(1,t1,TRACE_s_AFF_E);
				pHozScore[j].SetOneStartPosition(1,left,1);
			}
			//F:
			t0 = up.score[0] + global_param->gap_open;
			t2 = up.score[2] + global_param->gap_extend;
			if(t0>=t2)
			{
				pHozScore[j].SetOneData(2,t0,TRACE_s_AFF_H);
				pHozScore[j].SetOneStartPosition(2,up,0);
			}
			else
			{	
				pHozScore[j].SetOneData(2,t2,TRACE_s_AFF_F);
				pHozScore[j].SetOneStartPosition(2,up,2);
			}
			//H:
			t0 = upleft.score[0] + sc;
			t1 = pHozScore[j].score[1];
			t2 = pHozScore[j].score[2];
			if(t0>=t1 && t0>=t2)
			{
				pHozScore[j].SetOneData(0,t0,TRACE_s_AFF_H);
				pHozScore[j].SetOneStartPosition(0,upleft,0);
			}
			else if(t1>=t2)
			{
				pHozScore[j].SetOneData(0,t1,TRACE_s_AFF_E);
				pHozScore[j].SetOneStartPosition(0,pHozScore[j],1);
			}
			else
			{
				pHozScore[j].SetOneData(0,t2,TRACE_s_AFF_F);
				pHozScore[j].SetOneStartPosition(0,pHozScore[j],2);
			}

			left = pHozScore[j];
		}//end of for j

	}//end of for i

	pVerScore[local_len1] = pHozScore[local_len2];
	return true;
}

bool ParaSWProblem::FillBlock(bool isSW)
{
	if(isSW) return FillBlock_sw();
	else return FillBlock_nw();
}

bool ParaSWProblem::AssignGridCache(ParaSWGlobalMemory &gMem)
{
	int i;
	int start1 = path_grid->start.i;
	int start2 = path_grid->start.j;
	int end1 = path_grid->end.i;
	int end2 = path_grid->end.j;
	SubstitutionMatrix *pssm = global_param->pssm;
	int grid_h_count = global_param->grid_height_count;
	int grid_w_count = global_param->grid_width_count;

	//1. allocate memory used for block_division:
	//if problem unSolved:
	block_division = new ParaSWGridBlockDivision;	//the division of the blocks, for unSolved problems

	//2. divide the problem
	block_division->InitGridBlockDivision(end1-start1,end2-start2,
							grid_h_count,grid_w_count,&processor_list);

	//3. allocate memory for grid cache:
	//get memory from global memory:
	if (bBackSeparate == true)
	{
		for(i=0;i<block_division->gridHeights_count;i++)
		{
			gridRows[i] = &(gMem.gridRows[i][0])-1;	//only use from 1
		}
		for(i=0;i<block_division->gridWidths_count;i++)
		{													//FIX.BUG.06.01: start1/start2 turnover
			gridCols[i] = &(gMem.gridCols[i][0])-1;	//only use from 1
		}
	}
	else
	{
		for(i=0;i<block_division->gridHeights_count;i++)
		{
			gridRows[i] = &(gMem.gridRows[i][start2])-1;	//only use from 1
		}
		for(i=0;i<block_division->gridWidths_count;i++)
		{													//FIX.BUG.06.01: start1/start2 turnover
			gridCols[i] = &(gMem.gridCols[i][start1])-1;	//only use from 1
		}
	}

	//4. allocate memory for beach line:
//	block_Beach = new BeachLine;			//beach line of blocks
//	block_Beach->InitBeachLine(this,&gMem);
//	block_Beach_Admin = new BeachLine;			//beach line of blocks
//	block_Beach_Admin->InitBeachLineFront(this,&gMem);

	//have grid cache, is not solvable or blockProblem
	b_isBlockProblem = false;
	
	return true;
}

bool ParaSWProblem::TryGetProcessors(ParaSWProcessorSet &pSet)
{
	if(pSet.AssignProcessors(processor_list,this))
	{
		b_isSolvable = (processor_list.GetCount() == 1 &&
						processor_list.first->p_proc->blockMaxWidth > path_grid->end.j-path_grid->start.j &&
						processor_list.first->p_proc->blockMaxHeight > path_grid->end.i-path_grid->start.i);

		ParaSWProcessorListNode *p;
		p = processor_list.first;
		int l = processor_list.GetCount();
		Allocate_Mem(processors,ParaSWProcessor*,l);
		int i=0;
		while(p!=NULL)
		{
			processors[i] = p->p_proc;
			i++;
			p = p->next;
		}
		return true;
	}
	return false;
}

bool ParaSWProblem::GetProcessors(ParaSWProcessorList &pList)
{
	if(pList.IsEmpty()) return false;			//no avalible processor
	ParaSWProcessorListNode *p;
	while(!pList.IsEmpty())
	{
		p = pList.GetHead();
		processor_list.AddHead(p);
	}
	
	b_isSolvable = (processor_list.first->p_proc->blockMaxWidth > path_grid->end.j-path_grid->start.j &&
					processor_list.first->p_proc->blockMaxHeight > path_grid->end.i-path_grid->start.i);
	
	p = processor_list.first;
	int l = processor_list.GetCount();
	Allocate_Mem(processors,ParaSWProcessor*,l);
	int i=0;
	while(p!=NULL)
	{
		processors[i] = p->p_proc;
		i++;
		p = p->next;
	}
	return true;
}

bool ParaSWProblem::SendBackProcessors(ParaSWProcessorSet &pSet)
{
	int l = processor_list.GetCount();
	Release_Mem(processors,l);
	return pSet.RecycleProcessors(processor_list);
}

bool ParaSWProblem::ReleaseProblem()
{
	ReleaseGridCache();
	Release_Object(block_division);
	return true;
}

bool ParaSWProblem::ReleaseGridCache()
{
	if(IsSolvable() ||				//no need for release grid cache
		path_grid == NULL ||		//no local problem
		block_division == NULL)		//no grid cache
	{
		return false;	
	}

	Release_Object(block_Beach);
//	Release_Object(block_Beach_Admin);
	
	return true;
}

/*****************************************************************************************************
* Class NodeArray
*	// big node array for the main problem
******************************************************************************************************/
#include "memory.h"
NodeArray::NodeArray()
{
	nodes = NULL;
	positions = NULL;
	node_count = 0;

	big_count = 0;
}

NodeArray::~NodeArray()
{
	Release_Mem_2(nodes,node_count);
	Release_Mem_2(positions,node_count);
	node_count = 0;

	big_count = 0;
}

bool NodeArray::InitNodeArray(int k)
{
	int i;
	if(k<=node_count) return false;
	if(nodes!=NULL && node_count==k)
	{
		node_count = k;
		big_count = 0;
		memset(nodes,0,k*sizeof(Node));
		for(i=0;i<k;i++)
		{
			nodes[i].SetStartPositionHere(-1,-1);
		}
		positions[i].i = -1;
		positions[i].j = -1;
		return false;
	}
	if(nodes!=NULL) return false;
	if(positions!=NULL) return false;

	nodes = new Node[k];
	positions = new Position[k];
	node_count = k;
	big_count = 0;
	
	memset(nodes,0,k*sizeof(Node));
	for(i=0;i<k;i++)
	{
		nodes[i].SetStartPositionHere(-1,-1);
		positions[i].i = -1;
		positions[i].j = -1;
	}
	
	return true;
}

bool NodeArray::TryAddBigNode(Node &n,int h,int w)
{
	int i;
	for(i=0;i<node_count;i++)
	{//if exist same start position
		if((nodes[i].global_start[0].i == n.global_start[0].i) &&
			(nodes[i].global_start[0].j == n.global_start[0].j))
		{//same start position
			if(nodes[i].score[0] < n.score[0])
			{
				nodes[i] = n;
				positions[i].i = h;
				positions[i].j = w;
				return true;
			}else
			{
				return false;
			}
		}
	}
	for(i=0;i<node_count;i++)
	{//if exist unfilled start position
		if(positions[i].i == -1 && positions[i].j == -1)
		{
			nodes[i] = n;
			positions[i].i = h;
			positions[i].j = w;
			if(big_count<node_count) big_count++;
			return true;
		}
	}
	/*
	for(i=0;i<node_count;i++)
	{//if exist different bigger score start position
		if(nodes[i].score[0] < n.score[0])
		{
			nodes[i] = n;
			positions[i].i = h;
			positions[i].j = w;
			if(big_count<node_count) big_count++;
			return true;
		}
	}
	*/
	int minnode = 0;
	int minscore = nodes[minnode].score[0];
	for(i=1;i<node_count;i++)
	{
		if(nodes[i].score[0]<minscore)
		{
			minnode = i;
			minscore = nodes[i].score[0];
		}
	}
	if(minscore < n.score[0])
	{
		nodes[minnode] = n;
		positions[minnode].i = h;
		positions[minnode].j = w;
	}

	return false;
}


/*****************************************************************************************************
* Class Node
*	// element in the score matrix
******************************************************************************************************/
Node::Node()
{
}

Node::~Node()
{
}

bool Node::operator =(Node& n1)
{
	for(int i=0;i<Node_length;i++)
	{
		this->score[i] = n1.score[i];
		this->trace[i] = n1.trace[i];
		this->global_start[i].i = n1.global_start[i].i;
		this->global_start[i].j = n1.global_start[i].j;
		this->local_start[i].i = n1.local_start[i].i;
		this->local_start[i].j = n1.local_start[i].j;
		this->trace_start[i] = n1.trace_start[i];
	}
	return true;
}

bool Node::operator !=(Node& n1)
{
	int sum = 1;
	for(int i=0;i<Node_length;i++)
	{
		sum &= (this->score[i] == n1.score[i]);
		sum &= (this->trace[i] == n1.trace[i]);
		sum &= (this->global_start[i].i == n1.global_start[i].i);
		sum &= (this->global_start[i].j == n1.global_start[i].j);
		sum &= (this->local_start[i].i == n1.local_start[i].i);
		sum &= (this->local_start[i].j == n1.local_start[i].j);
		sum &= (this->trace_start[i] == n1.trace_start[i]);
	}
	return (1-sum);
}

bool Node::SetData(int k, ... )
{
	if(k<-1 || k>Node_length) return false;

	va_list marker;
	va_start(marker,k);
	int i;
	for(i=0;i<k;i++)
	{
		score[i] = va_arg(marker,int);
		trace[i] = (char) va_arg(marker,int);
	}
	return true;
}


bool Node::SetStartPositionHere(int h,int w)
{
	int i;
	for(i=0;i<Node_length;i++)
	{
		this->global_start[i].i = h;
		this->global_start[i].j = w;
		this->local_start[i].i = h;
		this->local_start[i].j = w;
		this->trace_start[i] = i;	//the local trace status is just this status
	}
	return true;
}

bool Node::SetStartPosition(bool isGlobal, ... )
{
	va_list marker;
	va_start(marker,isGlobal);
	int i;
	if(isGlobal)
	{
		for(i=0;i<Node_length;i++)
		{
			global_start[i].i = va_arg(marker,int);
			global_start[i].j = va_arg(marker,int);
		}
	}else
	{
		for(i=0;i<Node_length;i++)
		{
			local_start[i].i = va_arg(marker,int);
			local_start[i].j = va_arg(marker,int);
			trace_start[i] = (char) va_arg(marker,int);
		}
	}
	return true;
}

bool Node::SetOneStartPosition(int n,bool isGlobal,int i,int j,char k)
{
	if(isGlobal)
	{		
		global_start[n].i = i;
		global_start[n].j = j;
	}else
	{
		local_start[n].i = i;
		local_start[n].j = j;
		trace_start[n] = k;
	}
	return true;
}

bool Node::SetOneStartPosition(int n,Node &preNode,int m)
{
	this->global_start[n].i = preNode.global_start[m].i;
	this->global_start[n].j = preNode.global_start[m].j;
	this->local_start[n].i = preNode.local_start[m].i;
	this->local_start[n].j = preNode.local_start[m].j;
	this->trace_start[n] = preNode.trace_start[m];
	return true;
}

bool Node::SetOneData(int n,int s,char t)
{
#if 0
	if(n<0 || n>=Node_length) return false;
#endif
	score[n] = s;
	trace[n] = t;
	return true;
}

void Node::Init_nw_10(int s0,char t2,int st0h,int st0w,char ts0)
{
	this->score[0] = s0;			this->score[1] = NODE_UNREACHABLE;	this->score[2] = s0;
	this->trace[0] = TRACE_s_AFF_F;	this->trace[1] = -1;				this->trace[2] = t2;

	this->global_start[0].i = st0h;	this->global_start[1].i = -1;	this->global_start[2].i = st0h;
	this->global_start[0].j = st0w;	this->global_start[1].j = -1;	this->global_start[2].j = st0w;

	this->local_start[0].i = st0h;	this->local_start[1].i = -1;	this->local_start[2].i = st0h;
	this->local_start[0].j = st0w;	this->local_start[1].j = -1;	this->local_start[2].j = st0w;
	this->trace_start[0] = ts0;		this->trace_start[1] = -1;		this->trace_start[2] = ts0;
}

void Node::Init_nw_20(int s0,int st0h,int st0w,char ts0)
{
	this->score[0] = s0;			this->score[1] = NODE_UNREACHABLE;	this->score[2] = s0;
	this->trace[0] = TRACE_s_AFF_F;	this->trace[1] = -1;				this->trace[2] = TRACE_s_AFF_F;

	this->global_start[0].i = st0h;	this->global_start[1].i = -1;	this->global_start[2].i = st0h;
	this->global_start[0].j = st0w;	this->global_start[1].j = -1;	this->global_start[2].j = st0w;

	this->local_start[0].i = st0h;	this->local_start[1].i = -1;	this->local_start[2].i = st0h;
	this->local_start[0].j = st0w;	this->local_start[1].j = -1;	this->local_start[2].j = st0w;
	this->trace_start[0] = ts0;		this->trace_start[1] = -1;		this->trace_start[2] = ts0;
}

void Node::Init_nw_01(int s0,char t1,int st0h,int st0w,char ts0)
{
	this->score[0] = s0;			this->score[1] = s0;	this->score[2] = NODE_UNREACHABLE;
	this->trace[0] = TRACE_s_AFF_E;	this->trace[1] = t1;	this->trace[2] = -1;

	this->global_start[0].i = st0h;	this->global_start[1].i = st0h;	this->global_start[2].i = -1;
	this->global_start[0].j = st0w;	this->global_start[1].j = st0w;	this->global_start[2].j = -1;

	this->local_start[0].i = st0h;	this->local_start[1].i = st0h;	this->local_start[2].i = -1;
	this->local_start[0].j = st0w;	this->local_start[1].j = st0w;	this->local_start[2].j = -1;
	this->trace_start[0] = ts0;		this->trace_start[1] = ts0;		this->trace_start[2] = -1;
}

void Node::Init_nw_02(int s0,int st0h,int st0w,char ts0)
{
	this->score[0] = s0;			this->score[1] = s0;			this->score[2] = NODE_UNREACHABLE;
	this->trace[0] = TRACE_s_AFF_E;	this->trace[1] = TRACE_s_AFF_E;	this->trace[2] = -1;

	this->global_start[0].i = st0h;	this->global_start[1].i = st0h;	this->global_start[2].i = -1;
	this->global_start[0].j = st0w;	this->global_start[1].j = st0w;	this->global_start[2].j = -1;

	this->local_start[0].i = st0h;	this->local_start[1].i = st0h;	this->local_start[2].i = -1;
	this->local_start[0].j = st0w;	this->local_start[1].j = st0w;	this->local_start[2].j = -1;
	this->trace_start[0] = ts0;		this->trace_start[1] = ts0;		this->trace_start[2] = -1;
}

/*****************************************************************************************************
* Class BeachLine
*	// beachline for blocks
*	// only save one row, that is enough
******************************************************************************************************/
#include "ParaSWGlobalMemory.h"
BeachLine::BeachLine()
{
	beach = NULL;
	beach_front = NULL;
	block_width_count = 0;
	block_width = 0;
}

BeachLine::~BeachLine()
{
	Release_Mem_2(beach,block_width_count);
	Release_Mem_2(beach_front,block_width_count);
	block_width_count = 0;
	block_width = 0;
}

bool BeachLine::InitBeachLine(ParaSWProblem *problem,ParaSWGlobalMemory *gMem)
{
	return InitBeachLineNodes(problem,gMem) && InitBeachLineFront(problem,gMem);
}

bool BeachLine::InitBeachLineNodes(ParaSWProblem *problem,ParaSWGlobalMemory *gMem)
{
//	if(beach!=NULL) return false;
	Release_Mem_2(beach,block_width_count);
	block_width_count = problem->block_division->blockWidths_count;
	block_width = problem->global_param->block_width;
	Allocate_Mem(beach,Node*,block_width_count);
	int i;
	if (bBackSeparate == true)
	{
		for(i=0;i<block_width_count;i++)
		{
			beach[i] = &(gMem->beachLine[problem->block_division->blockWidths[i].start]);
		}
	}
	else
	{
		for(i=0;i<block_width_count;i++)
		{
			beach[i] = &(gMem->beachLine[problem->path_grid->start.j + problem->block_division->blockWidths[i].start]);
		}
	}
	return true;
}

bool BeachLine::InitBeachLineFront(ParaSWProblem *problem,ParaSWGlobalMemory *gMem)
{
//	if(beach_front!=NULL) return false;
	Release_Mem_2(beach_front,block_width_count);
	block_width_count = problem->block_division->blockWidths_count;
	block_width = problem->global_param->block_width;
	Allocate_Mem(beach_front,int,block_width_count);
	return true;
}

/*****************************************************************************************************
* Class ParaSWReadyTaskArray
*	// task array with assigned processors
******************************************************************************************************/
ParaSWReadyTaskArray::ParaSWReadyTaskArray()
{
	readyTasks = NULL;
	readyTasks_count = 0;
	total_number = 0;
}

ParaSWReadyTaskArray::~ParaSWReadyTaskArray()
{
	Release_Mem_2(readyTasks,total_number);
	total_number = 0;
}

bool ParaSWReadyTaskArray::InitReadyTaksArray(int p_count)
{
	if(readyTasks!=NULL) return false;
	total_number = p_count;
	Allocate_Mem(readyTasks,ParaSWReadyTask,total_number);
	return true;
}

ParaSWReadyTask * ParaSWReadyTaskArray::GetOneReadyTask()
{
	if(readyTasks==NULL) return NULL;
	if(readyTasks_count<=0) return NULL;
	readyTasks_count--;
	return &(readyTasks[readyTasks_count]);
}
