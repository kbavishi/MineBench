// ParaSWPath.cpp: implementation of the ParaSWPath class.
//
//////////////////////////////////////////////////////////////////////

#include "Common.h"
#include "ParaSWParam.h"
#include "ParaSWPath.h"
#include "Debug.h"
extern CDebug debugf;

/*****************************************************************************************************
* Class ParaSWPath
*	// Result of the alignment, one of the k-opimal paths.
*	// Contains:
*	//		One List of UnSolved path grids
*	//		A solution of joint the solved paths
******************************************************************************************************/
ParaSWPath::ParaSWPath()
{
	InitPath();
}

ParaSWPath::~ParaSWPath()
{
	Release_Mem_2(seq1,length+1);
	Release_Mem_2(seq2,length+1);
	Release_Mem_2(ali,length+1);
}

bool ParaSWPath::IsSolved()
{
	return (unSolvedPathGrids.IsEmpty());
}

bool ParaSWPath::IsTaskQueueEmpty()
{
	return IsSolved();
}

ParaSWPathGrid* ParaSWPath::GetUnSolved()
{
	return unSolvedPathGrids.GetHead();
}

ParaSWPathGrid* ParaSWPath::PopFromTaskQueue()
{
	return GetUnSolved();
}

void ParaSWPath::AddUnSolvedGrid(ParaSWPathGrid* pg)
{
	unSolvedPathGrids.AddHead(pg);
}

void ParaSWPath::PushIntoTaskQueue(ParaSWPathGrid* pg)
{
	AddUnSolvedGrid(pg);
}

void ParaSWPath::AddSolvedGrids(ParaSWPathGrid* pg)
{
	solvedPathGrids.AddHead(pg);
}

void ParaSWPath::PushIntoResultQueue(ParaSWPathGrid* pg)
{
	AddSolvedGrids(pg);
}

bool ParaSWPath::SortSolvedPath()
{
	//sort solved path in a global end->start order
	if(!IsSolved()) return false;

	ParaSWPathGrid *p;		//p for the current first grid in solved list
	ParaSWPathGrid *p_pre;	//p_pre for the current first grid's previous grid
	ParaSWPathGrid *p_it,*p_it_pre;	//iterator of the process

	ParaSWPathGrid *p_new;	//p_new for the first pointer of the new list
	p_new = NULL;
	while(!solvedPathGrids.IsEmpty())
	{
		//find minimum grid in the solved path:
		p = solvedPathGrids.first;
		p_pre = NULL;
		p_it = solvedPathGrids.first;
		p_it_pre = NULL;
		while(p_it!=NULL)
		{
			if(p_it->start < p->start)	//minimum grid
			{
				p_pre = p_it_pre;
				p = p_it;
			}
			p_it_pre = p_it;
			p_it = p_it->next;
		}
		//save the minimum grid:
		if(p_pre!=NULL) p_pre->next = p->next;	//remove from old list
		else solvedPathGrids.first = solvedPathGrids.first->next;
		p->next = p_new;	//add to new list
		p_new = p;			//add to new list
	}
	solvedPathGrids.first = p_new;
	return true;
}

bool ParaSWPath::InitPath()
{
	//set seq1,seq2,ali to 0
	seq1 = NULL;
	seq2 = NULL;
	ali = NULL;
	length = -1;

	seq1_start = 0;
	seq1_end = 0;
	seq2_start = 0;
	seq2_end = 0;
	return true;
}

bool ParaSWPath::JointAllPath(Sequence *s1,Sequence *s2)
{
	//get seq1,seq2,ali for output
	if(!IsSolved()) return false;
	
	if(seq1!=NULL || seq2!=NULL || ali!=NULL)
	{//reallocated:
		return false;
	}

	ParaSWPathGrid *p;		//p for the current first grid in solved list
	int cur_len;			//current length of the alignment
	int i;
	char *d1 = s1->data;	//pointer to the sequence's char data
	char *d2 = s2->data;

	//caculate the length of the alignment:
	p = solvedPathGrids.first;
	cur_len = 0;
	while(p!=NULL)
	{
		//this grid:
		cur_len += p->points_len - 1;	//there must one point superposed
		p = p->next;
	}

	//allocate alignment:
	length = cur_len;
	seq1 = new char[length+1];		//+1 for the last charactor '\0'
	seq2 = new char[length+1];
	ali = new char[length+1];
	
	//fill alignment:
	p = solvedPathGrids.first;
	seq1[length] = '\0';		//first one in SW is not paired, score is 0
	seq2[length] = '\0';
	ali[length] = '\0';
	cur_len = 0;
	int i_1,i_2;	//pos in to sequences
	seq1_end = p->end.i;
	seq2_end = p->end.j;
	while(p!=NULL)
	{
		i_1 = p->end.i-1;
		i_2 = p->end.j-1;
		for(i=0;i<p->points_len-1;i++)
		{
			switch(p->direction[i])
			{
			case UPLEFT:
				seq1[cur_len+i] = d1[i_1]; i_1--;
				seq2[cur_len+i] = d2[i_2]; i_2--;
				if(seq1[cur_len+i] == seq2[cur_len+i]) ali[cur_len+i] = seq1[cur_len+i];
				else ali[cur_len+i] = CHAR_UNPAIR;		//unpair
				break;
			case LEFT:
				seq1[cur_len+i] = CHAR_GAP;	//gap 1
				seq2[cur_len+i] = d2[i_2]; i_2--;
				ali[cur_len+i] = CHAR_UNPAIR;
				break;
			case UP:
				seq1[cur_len+i] = d1[i_1]; i_1--;
				seq2[cur_len+i] = CHAR_GAP;	//gap 2
				ali[cur_len+i] = CHAR_UNPAIR;
				break;
			default:
				return false;	//something wrong in the direction
				break;
			}
		}
		cur_len += p->points_len-1;

		if(p->next==NULL)
		{
			seq1_start = p->start.i + 1;
			seq2_start = p->start.j + 1;
		}
		//next:
		p = p->next;
	}
	
	char tmp;
	for (i=0; i<length/2; i++)
	{
		tmp=seq1[i]; seq1[i]=seq1[length-1-i]; seq1[length-1-i]=tmp;
		tmp=seq2[i]; seq2[i]=seq2[length-1-i]; seq2[length-1-i]=tmp;
		tmp= ali[i];  ali[i]= ali[length-1-i];  ali[length-1-i]=tmp;
	}
	
	return true;
}
/*****************************************************************************************************
* Class ParaSWPathGrid
*	// Path for each grid, the unit of result path
*	// if Solved:
*	//		save the direction of the path, from end to start point
*	// if UnSolved:
*	//		don't save the direction of the path
******************************************************************************************************/
ParaSWPathGrid::ParaSWPathGrid()
{
	IsSolved = false;		//solve status for this grid

	start.i = -1;			//local start point (h,w) of global path
	start.j = -1;
	end.i = -1;				//local end point (h,w) of global path
	end.j = -1;

	trace_num = -1;			//unknown trace number
	
	direction = NULL;		//direction_len = points_len-1, from end to start point
	points_len = 1;			//point length of the local path

	delta_score = 0;

	pathID = -1;

	next = NULL;
}

ParaSWPathGrid::~ParaSWPathGrid()
{
	Release_Mem_2(direction,(end.i-start.i+end.j-start.j));
}

bool ParaSWPathGrid::InitPathGrid(int start_i,int start_j,
								  int end_i,int end_j,
								  Node *s_node,Node *e_node)
{
	if(start_i>end_i || start_j>end_j)
	{
		return false;
	}
	IsSolved = false;
	Release_Mem_2(direction,(end.i-start.i+end.j-start.j));
	
	points_len = 1;

	start.i = start_i;
	start.j = start_j;
	end.i = end_i;
	end.j = end_j;
	if(s_node!=NULL)
		start_node = (*s_node);
	if(e_node!=NULL)
		end_node = (*e_node);

	next = NULL;
	return true;
}

bool ParaSWPathGrid::InitPathGrid(int path_id,int start_i,int start_j,
								  int end_i,int end_j,
								  Node *s_node,Node *e_node)
{
	if(start_i>end_i || start_j>end_j)
	{
		return false;
	}
	pathID = path_id;
	IsSolved = false;
	Release_Mem_2(direction,(end.i-start.i+end.j-start.j));
	
	points_len = 1;

	start.i = start_i;
	start.j = start_j;
	end.i = end_i;
	end.j = end_j;
	if(s_node!=NULL)
		start_node = (*s_node);
	if(e_node!=NULL)
		end_node = (*e_node);

	next = NULL;
	return true;
}

bool ParaSWPathGrid::TrySimpleSolvePathGrid()
{
	int i;
	if(start.i == end.i && start.j == end.j)
	{
		//some error
		return true;
	}
	if(start.i == end.i)
	{
		AllocatePath();
		for(i=0;i<end.j-start.j;i++)
		{
			direction[i] = TRACE_s_AFF_E;
		}
		return true;
	}
	if(start.j == end.j)
	{
		AllocatePath();
		for(i=0;i<end.i-start.i;i++)
		{
			direction[i] = TRACE_s_AFF_F;
		}
		return true;
	}
	return false;
}

bool ParaSWPathGrid::AllocatePath()
{
	if(direction!=NULL)
	{
		return false;
	}
	direction = new char[(end.i-start.i+end.j-start.j)];
	return true;
}

/*****************************************************************************************************
* Class ParaSWPathGridList
*	// List of ParaSWPathGrid
*	// Contains:
*	//		get head method
*	//		add head method
******************************************************************************************************/
ParaSWPathGridList::ParaSWPathGridList()
{
	first = NULL;
}

ParaSWPathGridList::~ParaSWPathGridList()
{
	ParaSWPathGrid *p;
	p = first;
	while(p!=NULL)
	{
		p = first->next;
		delete first;
		first = p;
	}
}

bool ParaSWPathGridList::IsEmpty()
{
	return (first == NULL);
}

ParaSWPathGrid* ParaSWPathGridList::GetHead()
{
	if(first==NULL) return NULL;
	ParaSWPathGrid *p;
	p = first;
	first = first->next;
	p->next = NULL;

	return p;
}

ParaSWPathGrid* ParaSWPathGridList::AddHead(ParaSWPathGrid *pg)
{
	if(pg==NULL) return NULL;
	pg->next = first;
	first = pg;

	return first;
}

int ParaSWPathGridList::GetCount()
{
	int count=0;
	ParaSWPathGrid *p = first;
	while(p!=NULL)
	{
		count++;
		p=p->next;
	}
	return count;
}

ParaSWPathGrid* ParaSWPathGridList::AddHead(ParaSWPathGridList *pl)
{
	ParaSWPathGrid *p,*p_pre;
	p = pl->first; p_pre = NULL;
	while(p!=NULL)
	{
		p_pre = p;
		p = p->next;
	}
	p_pre->next = this->first;
	this->first = pl->first;
	pl->first = NULL;			//move all the path grid from pl to this

	return this->first;
}


