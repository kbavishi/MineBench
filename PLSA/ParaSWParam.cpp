// ParaSWParam.cpp: implementation of the ParaSWParam class.
//
//////////////////////////////////////////////////////////////////////

#include "Common.h"
#include "memory.h"
#include "ctype.h"

#include "ParaSWParam.h"
#include "ParaSWProblem.h"
#include "Debug.h"

extern CDebug debugf;
/*****************************************************************************************************
* Class ParaSWParam:	all parameters used in a ParaSWProblem
******************************************************************************************************/
ParaSWParam::ParaSWParam()
{
	align_type = ALIGNMENT_ALGORITHM_sw | 
				ALIGNMENT_SCORE_s_aff | 
				ALIGNMENT_SUBMATRIX_yes |
				ALIGNMENT_PROCESS_parallel |
				ALIGNMENT_STRATEGY_parallelSW; 
	//default type: sw s_aff yes par parSW

	block_height = NULL;
	int b_h[] = {6};

	InitParameters(
		align_type,		//int align_type;		//type need : NW|SW Aff
		NULL,			//Sequence *seq1;       //File path or Data buff of seq1
		NULL,			//Sequence *seq2;       //File path or Data buff of seq2
		NULL,			//SubstitutionMatrix * pssm;	//position specific score matrix
		-12,			//int gap_open;			//gap open penalty
		-4,				//int gap_extend;		//gap extension penalty
		1,				//int processor_count;	//def = 1
		10000,			//int solvableMemSize;	//size of score matrix which could be solved directly, in trace step
		b_h,			//int *block_height;		//block height of each processor, or the capability comparison of the processors
		4,				//int block_width;			//solvable block width of each processor, equal for all processors
		3,				//int grid_height_count;	//one problem could have how many grid rows
		3,				//int grid_width_count;		//one problem could have how many grid cols
		1);				//int k;					// k nearest optimal paths
}

void ParaSWParam::InitParameters(int ali_type,
								 Sequence *s1, Sequence *s2,
								 SubstitutionMatrix * sm,
								 int gap_o, int gap_e,
								 int proc_count, int solvable_size,
								 int *block_h, int block_w,
								 int grid_h_count,int grid_w_count,
								 int k_path)
{
	align_type = ali_type;	//type need : NW|SW Aff
	
	seq1 = s1;				//File path or Data buff of seq1
	seq2 = s2;				//File path or Data buff of seq2
	
	pssm = sm;				//position specific score matrix
	
	gap_open = gap_o;		//gap open penalty
	gap_extend = gap_e;		//gap extension penalty
	
	Release_Mem(block_height,processor_count);
	if(block_height!=NULL)
	{
		return;
	}
	processor_count = proc_count;	//def = 1
	block_width = block_w;			//block width of each processor, equal for all processors
	if(proc_count>0)		//block height of each processor, or the capability comparison of the processors
	{
		block_height = new int[proc_count];		//once
		for(int i=0;i<proc_count;i++)
		{
			block_height[i] = block_h[i];
		}
	}else
	{
		block_height = NULL;
	}
	
	grid_height_count = grid_h_count;	//one problem could have how many grid rows
	grid_width_count = grid_w_count;	//one problem could have how many grid cols
	
	k = k_path;							// k nearest optimal paths
	
	solvableMemSize = solvable_size;	//size of score matrix which could be solved directly, in trace step
}

ParaSWParam::~ParaSWParam()
{
	Release_Mem(block_height,processor_count);
}

/*****************************************************************************************************
* Class ParaSWGridBlockDivision
*	//the Grid and Block Division of one problem if it is not solvable
*	//Grid is the saved value during the FillCache step
*	//Block is the actual size assign to one processer during the FillCache step
*	//Once dicide the division, never change it in the following process.
******************************************************************************************************/
#include "ParaSWProcessorSet.h"
ParaSWGridBlockDivision::ParaSWGridBlockDivision()
{
	blockHeights = NULL;		//bound of each block in h-direction, pos in global-logical-matrix, start at 0
	blockWidths = NULL;			//bound of each block in w-direction, pos in global-logical-matrix, start at 0
	blockHeights_count = 0;		//number of grids in h-direction
	blockWidths_count = 0;		//number of grids in w-direction
//	processorIDs = NULL;		//ID of processors in h-direction
	
	blockHeightOnGrid = NULL;	//if block's first row is on grid, ...OnGrid = 1;
	blockWidthOnGrid = NULL;	//if block's last row is on grid, ...OnGrid = 2;
								//else ...OnGrid = 0;
	
	gridHeights = NULL;			//bound of each grid in h-direction, pos in global-logical-matrix, start at 0
	gridWidths = NULL;			//bound of each grid in w-direction, pos in global-logical-matrix, start at 0
	gridHeights_count = 0;		//number of grids in h-direction
	gridWidths_count = 0;		//number of grids in w-direction
}

ParaSWGridBlockDivision::~ParaSWGridBlockDivision()
{
	ReleaseGridBlockDivision();
}

bool ParaSWGridBlockDivision::InitGridBlockDivision(
		int seq1_length, int seq2_length,		//length of two sequences, logical matrix is (len1+1)*(len2+1)
		int gridH_count, int gridW_count,		//number of grid to be saved
		ParaSWProcessorList *pList				//list of processors for this problem
 		)
{
	if(pList==NULL) return false;		//no processors
	if(pList->IsEmpty()) return false;	//no processors
	ReleaseGridBlockDivision();

	/* Get information from pList */
	int proc_count = pList->GetCount();		//number of processors
	int *proc_block_heights; 				//block_heights for each processor
	proc_block_heights = new int[proc_count];
	int *proc_IDs;							//IDs for each processor
	proc_IDs = new int[proc_count];
	int block_heights_sum = 0;				//sum of block_heights
	int block_width;						//block_width for all blocks
	ParaSWProcessorListNode *p;
	p = pList->first;
	int i=0;
	block_width = p->p_proc->blockMaxWidth-1;
	while(p!=NULL)
	{
		proc_block_heights[i] = p->p_proc->blockMaxHeight-1;
		proc_IDs[i] = p->p_proc->processorID;
		block_heights_sum += proc_block_heights[i];
		
		p = p->next; i++;
	}
	
	/* first: divide problem into blocks, each block in h-direction assign to a certain processor */
	int pos;
	int t;
	// h-direction block count:
	blockHeights_count = seq1_length / block_heights_sum * proc_count;	// basic part
	t = seq1_length % block_heights_sum;	//last part
	if(t!=0)
	{
		pos = 0;
		for(i=0;i<proc_count;i++)
		{
			pos+=proc_block_heights[i];
			if(pos >= t) break;
		}
		blockHeights_count += i+1;	//last part
	}
	// w-direction block count:
	blockWidths_count = (seq2_length + block_width - 1) / block_width;	//upper integer
	// allocate:
	blockHeights = new Bound[blockHeights_count];
//	processorIDs = new int[blockHeights_count];
	blockWidths = new Bound[blockWidths_count];
	blockHeightOnGrid = new int[blockHeights_count];	//could be smaller
	blockWidthOnGrid = new int[blockWidths_count];		//could be smaller
	// fill value: blockHeights/processorIDs/blockWidths
	pos = 0;	//h-pos in the logical matrix
	for(i=0;i<blockHeights_count;i++)
	{
		blockHeights[i].start = pos;
//		processorIDs[i] = proc_IDs[i % proc_count];
		pos += proc_block_heights[i % proc_count];
		blockHeights[i].end = pos < seq1_length ? pos : seq1_length;
	}
	pos = 0;	//w-pos in the logical matrix
	for(i=0;i<blockWidths_count;i++)
	{
		blockWidths[i].start = pos;
		pos += block_width;
		blockWidths[i].end = pos < seq2_length ? pos : seq2_length;
	}

	/* second: divide blocks into grids, which is to be saved */
	//int *blockHeightOnGrid;	//if block's first row is on grid, ...OnGrid = 0x100 | i;
	//int *blockWidthOnGrid;	//if block's last row is on grid, ...OnGrid = 0x200 | i;
							//if two rows are both on grid, ...OnGrid = 0x300 | i;
							//else ...OnGrid = 0x000 | i;

	//gridH_count:
	if(gridH_count >= blockHeights_count)
	{//too few blocks in a grid
		gridHeights_count = blockHeights_count;
		// allocate:
		gridHeights = new Bound[gridHeights_count];
		// fill value: gridHeights/blockHeightOnGrid / gridWidths/blockWidthOnGrid
		for(i=0;i<gridHeights_count;i++)
		{
			gridHeights[i].start = blockHeights[i].start;
			gridHeights[i].end = blockHeights[i].end;
			blockHeightOnGrid[i] = 0x300 | i;
		}
	}else
	{//enough blocks in a grid
		gridHeights_count = gridH_count;
		// allocate:
		gridHeights = new Bound[gridHeights_count];
		// fill value: gridHeights/blockHeightOnGrid / gridWidths/blockWidthOnGrid
		int i_grid;
		memset(blockHeightOnGrid,0,blockHeights_count*sizeof(int));
		// h-fill:
		t = seq1_length / gridHeights_count;	//approximately height in one grid
		gridHeights[0].start = 0;	//first row of the matrix
		blockHeightOnGrid[0] |= 0x100;	//first row of a grid
		i_grid = 0; pos = 0;
		for(i=0;i<blockHeights_count;i++)
		{
			if(i_grid == gridHeights_count - 1)			//last grid
			{
				i = blockHeights_count-1;				//last block
				pos = seq1_length;						//last row
			}else
			{
				pos += proc_block_heights[i % proc_count];
				pos = pos < seq1_length ? pos : seq1_length;	//no more than seq1_length
			}
			if(pos >= (i_grid+1)*t)
			{
				gridHeights[i_grid].end = pos;
				blockHeightOnGrid[i] |= (0x200 | i_grid);		//last row of a grid
				i_grid++;
				if(i_grid == gridHeights_count) break;	//last grid
				gridHeights[i_grid].start = pos;
				blockHeightOnGrid[i+1] |= (0x100 | i_grid);	//first row of a grid
			}
		}// havn't consider the situations: 1. last several grids havn't assigned; 2. some grids start and end at the same row
	}
	//gridW_count:
	if(gridW_count >= blockWidths_count)
	{//too few blocks in a grid
		gridWidths_count = blockWidths_count;
		// allocate:
		gridWidths = new Bound[gridWidths_count];
		// fill value: gridHeights/blockHeightOnGrid / gridWidths/blockWidthOnGrid
		for(i=0;i<gridWidths_count;i++)
		{
			gridWidths[i].start = blockWidths[i].start;
			gridWidths[i].end = blockWidths[i].end;
			blockWidthOnGrid[i] = 0x300 | i;
		}
	}else
	{//enough blocks in a grid
		gridWidths_count = gridW_count;
		// allocate:
		gridWidths = new Bound[gridWidths_count];
		// fill value: gridHeights/blockHeightOnGrid / gridWidths/blockWidthOnGrid
		int i_grid;
		memset(blockWidthOnGrid,0,blockWidths_count*sizeof(int));
		// w-fill:
		t = seq2_length / gridWidths_count;	//approximately width in one grid
		gridWidths[0].start = 0;
		blockWidthOnGrid[0] |= 0x100;	//first col of a grid
		i_grid = 0; pos = 0;
		for(i=0;i<blockWidths_count;i++)
		{
			if(i_grid == gridWidths_count - 1)
			{
				i = blockWidths_count-1;
				pos = seq2_length;
			}else
			{
				pos += block_width;
				pos = pos < seq2_length ? pos : seq2_length;	//no more than seq2_length
			}
			if(pos >= (i_grid+1)*t)
			{
				gridWidths[i_grid].end = pos;
				blockWidthOnGrid[i] |= (0x200 | i_grid);		//last col of a grid
				i_grid++;
				if(i_grid == gridWidths_count) break;
				gridWidths[i_grid].start = pos;
				blockWidthOnGrid[i+1] |= (0x100 | i_grid);		//first col of a grid
			}
		}// havn't consider the situations: 1. last several grids havn't assigned; 2. some grids start and end at the same row
	}
	//for last row or column, no grid:
	blockHeightOnGrid[blockHeights_count-1] = 0x1ff & blockHeightOnGrid[blockHeights_count-1];
	blockWidthOnGrid[blockWidths_count-1] = 0x1ff & blockWidthOnGrid[blockWidths_count-1];

	return true;
}

bool ParaSWGridBlockDivision::InitGridBlockDivisionSimple(
		int seq1_length, int seq2_length,		//length of two sequences, logical matrix is (len1+1)*(len2+1)
		int gridH_count, int gridW_count,		//number of grid to be saved
		ParaSWProcessorList *pList				//list of processors for this problem
 		)
{
	if(pList==NULL) return false;		//no processors
	if(pList->IsEmpty()) return false;	//no processors
	ReleaseGridBlockDivision();

	/* Get information from pList */
	int proc_count = pList->GetCount();		//number of processors
	int block_heights = pList->first->p_proc->blockMaxHeight-1; //block_heights for each processor
	int block_width = pList->first->p_proc->blockMaxWidth-1;	//block_width for all blocks
	int *proc_IDs;							//IDs for each processor
	proc_IDs = new int[proc_count];
	int i=0;
//	ParaSWProcessorListNode *p;
//	p = pList->first;
//	while(p!=NULL)
//	{
//		proc_IDs[i] = p->p_proc->processorID;
//		p = p->next; i++;
//	}
	/* first: divide problem into blocks, each block in h-direction assign to a certain processor */
	int pos;
	int t;
	// h-direction block count: |~len1/bh~|
	blockHeights_count = (seq1_length + block_heights - 1) / block_heights;
	// w-direction block count: |~len2/bw~|
	blockWidths_count = (seq2_length + block_width - 1) / block_width;	//upper integer
	// allocate:
//	processorIDs = new int[blockHeights_count];
	blockHeights = new Bound[blockHeights_count];
	blockWidths = new Bound[blockWidths_count];
	blockHeightOnGrid = new int[blockHeights_count];
	blockWidthOnGrid = new int[blockWidths_count];
	// fill value: blockHeights/processorIDs/blockWidths
	pos = 0;	//h-pos in the logical matrix
	for(i=0;i<blockHeights_count;i++)
	{
		blockHeights[i].start = pos;
//		processorIDs[i] = proc_IDs[i % proc_count];
		pos += block_heights;
		blockHeights[i].end = pos < seq1_length ? pos : seq1_length;
	}
	pos = 0;	//w-pos in the logical matrix
	for(i=0;i<blockWidths_count;i++)
	{
		blockWidths[i].start = pos;
		pos += block_width;
		blockWidths[i].end = pos < seq2_length ? pos : seq2_length;
	}

	/* second: divide blocks into grids, which is to be saved */
	//gridH_count:
	if(gridH_count >= blockHeights_count)
	{//too few blocks in a grid
		gridHeights_count = blockHeights_count;
		// allocate:
		gridHeights = new Bound[gridHeights_count];
		// fill value: gridHeights/blockHeightOnGrid / gridWidths/blockWidthOnGrid
		for(i=0;i<gridHeights_count;i++)
		{
			gridHeights[i].start = blockHeights[i].start;
			gridHeights[i].end = blockHeights[i].end;
			blockHeightOnGrid[i] = 0x300 | i;
		}
	}else
	{//enough blocks in a grid
		gridHeights_count = gridH_count;
		// allocate:
		gridHeights = new Bound[gridHeights_count];
		// fill value: gridHeights/blockHeightOnGrid / gridWidths/blockWidthOnGrid
		int i_grid;
		memset(blockHeightOnGrid,0,blockHeights_count*sizeof(int));
		// h-fill:
		t = seq1_length / gridHeights_count;	//approximately height in one grid
		gridHeights[0].start = 0;	//first row of the matrix
		blockHeightOnGrid[0] |= 0x100;	//first row of a grid
		i_grid = 0; pos = 0;
		for(i=0;i<blockHeights_count;i++)
		{
			if(i_grid == gridHeights_count - 1)			//last grid
			{
				i = blockHeights_count-1;				//last block
				pos = seq1_length;						//last row
			}else
			{
				pos += block_heights;
				pos = pos < seq1_length ? pos : seq1_length;	//no more than seq1_length
			}
			if(pos >= (i_grid+1)*t)
			{
				gridHeights[i_grid].end = pos;
				blockHeightOnGrid[i] |= (0x200 | i_grid);		//last row of a grid
				i_grid++;
				if(i_grid == gridHeights_count) break;	//last grid
				gridHeights[i_grid].start = pos;
				blockHeightOnGrid[i+1] |= (0x100 | i_grid);	//first row of a grid
			}
		}// havn't consider the situations: 1. last several grids havn't assigned; 2. some grids start and end at the same row
	}
	//gridW_count:
	if(gridW_count >= blockWidths_count)
	{//too few blocks in a grid
		gridWidths_count = blockWidths_count;
		// allocate:
		gridWidths = new Bound[gridWidths_count];
		// fill value: gridHeights/blockHeightOnGrid / gridWidths/blockWidthOnGrid
		for(i=0;i<gridWidths_count;i++)
		{
			gridWidths[i].start = blockWidths[i].start;
			gridWidths[i].end = blockWidths[i].end;
			blockWidthOnGrid[i] = 0x300 | i;
		}
	}else
	{//enough blocks in a grid
		gridWidths_count = gridW_count;
		// allocate:
		gridWidths = new Bound[gridWidths_count];
		// fill value: gridHeights/blockHeightOnGrid / gridWidths/blockWidthOnGrid
		int i_grid;
		memset(blockWidthOnGrid,0,blockWidths_count*sizeof(int));
		// w-fill:
		t = seq2_length / gridWidths_count;	//approximately width in one grid
		gridWidths[0].start = 0;
		blockWidthOnGrid[0] |= 0x100;	//first col of a grid
		i_grid = 0; pos = 0;
		for(i=0;i<blockWidths_count;i++)
		{
			if(i_grid == gridWidths_count - 1)
			{
				i = blockWidths_count-1;
				pos = seq2_length;
			}else
			{
				pos += block_width;
				pos = pos < seq2_length ? pos : seq2_length;	//no more than seq2_length
			}
			if(pos >= (i_grid+1)*t)
			{
				gridWidths[i_grid].end = pos;
				blockWidthOnGrid[i] |= (0x200 | i_grid);		//last col of a grid
				i_grid++;
				if(i_grid == gridWidths_count) break;
				gridWidths[i_grid].start = pos;
				blockWidthOnGrid[i+1] |= (0x100 | i_grid);		//first col of a grid
			}
		}// havn't consider the situations: 1. last several grids havn't assigned; 2. some grids start and end at the same row
	}
	//for last row or column, no grid:
	blockHeightOnGrid[blockHeights_count-1] = 0x1ff & blockHeightOnGrid[blockHeights_count-1];
	blockWidthOnGrid[blockWidths_count-1] = 0x1ff & blockWidthOnGrid[blockWidths_count-1];

	return true;
}

bool ParaSWGridBlockDivision::SearchPositionGrid(Position &pos,bool &isOnRows,int &i,int &j)
{
	//if on rows, i=i_grid, j=pos.j
	//if on cols, i=pos.i, j=j_grid
	int i_grid;
	int j_grid;
	for(i_grid = 0;i_grid<gridHeights_count;i_grid++)
	{
		if(pos.i == gridHeights[i_grid].start)
		{
			isOnRows = true;
			i = i_grid;
			j = pos.j;
			return true;
		}
	}
	for(j_grid = 0;j_grid<gridWidths_count;j_grid++)
	{
		if(pos.j == gridWidths[j_grid].start)
		{
			isOnRows = false;
			i = pos.i;
			j = j_grid;
			return true;
		}
	}
	return false;
}

void ParaSWGridBlockDivision::ReleaseGridBlockDivision()
{
//	Release_Mem_2(processorIDs,blockHeights_count);
	Release_Mem_2(blockHeightOnGrid,blockHeights_count);
	Release_Mem_2(blockWidthOnGrid,blockWidths_count);
	Release_Mem(blockHeights,blockHeights_count);
	Release_Mem(blockWidths,blockWidths_count);

	Release_Mem(gridHeights,gridHeights_count);
	Release_Mem(gridWidths,gridWidths_count);
}

/*****************************************************************************************************
* Class Sequence
*	contains one sequence of original data, for output
*	contains another sequence of data index in substitution matrix, for score computing
******************************************************************************************************/
#include "string.h"

Sequence::Sequence()
{
	data = NULL;
	index = NULL;
	dataMem = NULL;
	indexMem = NULL;
	length = 0;
	type = FT_UNKNOWN_SEQWUENCE_TYPE;
}

Sequence::~Sequence()
{
	Release_Mem_1(data,length,dataMem);
	Release_Mem_1(index,length,indexMem);
	length = 0;
	type = FT_UNKNOWN_SEQWUENCE_TYPE;
}

bool Sequence::LoadSequenceFromFile(char fileName[],int sequenceType,SubstitutionMatrix *subMatrix)
{
	if(length!=0)
	{
		printf( "Error: Reload sequence data! -----Sequence::LoadSequenceFromFile()\n");
		return false;
	}
	
	FILE *f;
	if( (f  = fopen( fileName, "r" )) == NULL )
	{
		printf( "The file '%s' was not opened\n",fileName );
		return false;
	}
	else
		printf( "The file '%s' was opened\n",fileName );
	char line[1024];
	int count;
	int flag;
	int i,l,pos;

		//first scan:
	count = 0;
	flag = 0;
	while(!feof(f))
	{
		line[0]='\0';
		fgets(line,1024,f);
		l = strlen(line);
		if(line[l-1] == 10) l--;
		if(flag == 0)
		{
			if(line[0] == '>')
				flag = 2;
			continue;
		}else if(flag == 2)
		{
			for(i=0;i<l;i++)
			{
				if(!isspace(line[i]))
					count++;
			}
			continue;
		}
	}

	//second scan:
	fseek(f,0,SEEK_SET);
	length = count;
	Allocate_Mem_1(data,char,length,dataMem);
	pos = 0;
	flag = 0;
	while(!feof(f))
	{
		line[0]='\0';
		fgets(line,1024,f);
		l = strlen(line);
		if(line[l-1] == 10) l--;
		if(flag == 0)
		{
			if(line[0] == '>')
				flag = 2;
			continue;
		}else if(flag == 2)
		{
			for(i=0;i<l;i++)
			{
				if(!isspace(line[i]))
				{
					data[pos] = line[i];
					pos++;
				}
			}
			continue;
		}
	}

	fclose(f);

	//index:
	Allocate_Mem_1(index,int,length,indexMem);
	for(i=0;i<length;i++)
	{
		if(data[i]>='A' && data[i]<='Z')
		{
			index[i] = subMatrix->index[data[i] - 'A'];
		}else if(data[i]>='a' && data[i]<='z')
		{
			index[i] = subMatrix->index_X;
		}else if(data[i]=='*' || data[i]=='-')
		{
			index[i] = subMatrix->index_end;
		}
	}
	return true;
}

/*****************************************************************************************************
* Class SubstitutionMatrix
******************************************************************************************************/
SubstitutionMatrix::SubstitutionMatrix()
{
	name[0]='\0';
	index_X = -1;
	index_end = -1;
	score = NULL;
	dimension = max_dimension = 0;
	
	gap_open = 0;
	gap_extend = 0;

	unpair = 0;
	pair = 0;
}

SubstitutionMatrix::~SubstitutionMatrix()
{
	Release_Mem2D(score,max_dimension,max_dimension);
}

bool SubstitutionMatrix::LoadMatrixFromFile(char fileName[],int matrix_type)
{
	if(dimension!=0)
	{
		printf( "Error: Reload sequence data! -----SubstitutionMatrix::LoadMatrixFromFile()\n");
		return false;
	}
	index_end = 23;
	index_X   = 22;
	max_dimension = 24;
	
	score  = new int*[max_dimension];
	int *p = new int[max_dimension*max_dimension];
	int i;
	for(i=0;i<max_dimension;i++)
	{
		score[i] = &(p[i*max_dimension]);
	}
	
	gap_open = -12;		//for affine
	gap_extend = -4;	//for affine
	unpair = -2;	    //for no subMatrix
	pair = 2;		    //for no subMatrix

	FILE *f;
	if( (f  = fopen( fileName, "r" )) == NULL )
	{
		printf( "The file '%s' was not opened\n",fileName );
		return false;
	}
	else
		printf( "The file '%s' was opened\n",fileName );

	if(matrix_type != FT_FULL_SUBMATRIX)
	{
		printf( "Can not handle this!\n" );
		return false;
	}

	char c;
	char line[1024];
	int  l;
	char aa[26];
	int  j;
	int sta;

	memset(aa, 'A', 26); // avoid index[aa[i]-'A'] overflow
	
	while(!feof(f))
	{
		line[0] = '\0';
		fgets(line,1024,f);
		l = strlen(line);
		if(line[l-1] == 10) l--;
		if(line[0] == '#')
			continue;
		else if(line[0] == ' ')
		{
			dimension = 0;
			for(i=0;i<l;i++)
			{
				if(line[i]!=' ' && line[i] !='\r')
				{
					aa[dimension++] = line[i];
				}
			}
			
			for(i=0;i<26;i++) index[i] = -1;	//unknown aa, X
			for(i=0;i<dimension;i++)
			{
				if(aa[i]!='*')	index[aa[i]-'A'] = i;
				if(aa[i]=='X')	index_X = i;
				if(aa[i]=='*')	index_end = i;
			}
			for(i=0;i<26;i++) index[i] = (index[i] == -1) ? index_X:index[i];

			for(i=0;i<dimension;i++)
			{
				sta = fscanf(f,"%c",&c);
				for(j=0;j<dimension;j++)
				{
					sta = fscanf(f," %d",&score[i][j]);
				}
				if(!feof(f))
					sta = fscanf(f,"\n");
			}
			break;
		}else
			return false;
	}

	fclose(f);
	return true;
}



