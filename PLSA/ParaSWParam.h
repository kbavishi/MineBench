// ParaSWParam.h: interface for the ParaSWParam class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PARASWPARAM_H__A8D19220_B205_4136_A044_A3DF624B0558__INCLUDED_)
#define AFX_PARASWPARAM_H__A8D19220_B205_4136_A044_A3DF624B0558__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//--------------------------------------------------------

//define for parameter input:
#define	FT_FILE1	0x00
#define	FT_FILE2	0x01


//--------------------------------------------------------
//define for trace type:
#define TRACE_HERE 0
#define TRACE_UPLEFT 1
#define TRACE_LEFT 2
#define TRACE_UP 3
#define TRACE_UPLEFTPOS 5

#define NODE_UNREACHABLE	-1000000
#define TRACE_s_AFF_HERE	TRACE_HERE
#define TRACE_s_AFF_H		TRACE_UPLEFT
#define TRACE_s_AFF_E		TRACE_LEFT
#define TRACE_s_AFF_F		TRACE_UP

#define	ALIGN_NW_LIN	1
#define ALIGN_TEST_LOCAL	2
#define ALIGN_NW_AFF	3

#define ALIGNMENT_TYPE_single	0
#define ALIGNMENT_TYPE_forLSA	1

#define FASTLSA_TYPE_SEQUENTIAL	0
#define FASTLSA_TYPE_PARALLEL	1
//--------------------------------------------------------
//define for alignment type:
#define ALIGNMENT_ALGORITHM_MASK	0x03
#define ALIGNMENT_ALGORITHM_nw		0x00
#define ALIGNMENT_ALGORITHM_sw		0x01
#define ALIGNMENT_SCORE_MASK	0x0C
#define ALIGNMENT_SCORE_lin		0x00
#define ALIGNMENT_SCORE_aff		0x04
#define ALIGNMENT_SCORE_s_aff	0x08
#define ALIGNMENT_SUBMATRIX_MASK	0x30
#define ALIGNMENT_SUBMATRIX_yes		0x00
#define ALIGNMENT_SUBMATRIX_no		0x10
#define ALIGNMENT_PROCESS_MASK			0xC0
#define ALIGNMENT_PROCESS_sequential	0x00
#define ALIGNMENT_PROCESS_parallel		0x40
#define ALIGNMENT_STRATEGY_MASK				0x300
#define ALIGNMENT_STRATEGY_normal			0x000
#define ALIGNMENT_STRATEGY_fastLSA			0x100
#define ALIGNMENT_STRATEGY_onelevelfastLSA	0x200
#define ALIGNMENT_STRATEGY_parallelSW		0x300
//--------------------------------------------------------

/*****************************************************************************************************
* Class Sequence
*	contains one sequence of original data, for output
*	contains another sequence of data index in substitution matrix, for score computing
******************************************************************************************************/
class SubstitutionMatrix;
//define for sequence type:
#define	FT_PROTEIN					0x00
#define	FT_DNA						0x02
#define FT_UNKNOWN_SEQWUENCE_TYPE	-1
class Sequence{
public:
	Sequence();
	~Sequence();

public:
	//load data from file and index set consensus to substitution matrix:
	bool LoadSequenceFromFile(char fileName[],int sequenceType,SubstitutionMatrix *subMatrix);

public:
	int type;		//sequence is DNA/RNA or protein
	int length;
	char *data;
	int	*index;

	int *indexMem;
	char *dataMem;
};

/*****************************************************************************************************
* Class SubstitutionMatrix
******************************************************************************************************/
//define substitution matrix type:
#define FT_FULL_SUBMATRIX	0x04

class SubstitutionMatrix{
public:
	SubstitutionMatrix();
	~SubstitutionMatrix();

public:
	//load data from file and index set consensus to the file:
	bool LoadMatrixFromFile(char fileName[],int matrix_type);

public:
	char name[10];
	int index[26];
	int index_X,index_end;
	int **score;
	int dimension;     //real dimension for DNA or Protein score matix
	int max_dimension; //max dimension

	int gap_open;
	int gap_extend;

	int unpair;
	int pair;
};

/*****************************************************************************************************
* Struct Bound
******************************************************************************************************/
struct Bound{
	int start;
	int end;
};

/*****************************************************************************************************
* Class ParaSWGridBlockDivision
*	//the Grid and Block Division of one problem if it is not solvable
*	//Grid is the saved value during the FillCache step
*	//Block is the actual size assign to one processer during the FillCache step
*	//Once dicide the division, never change it in the following process.
******************************************************************************************************/
class ParaSWProcessorList;
struct Position;
class ParaSWGridBlockDivision{
public:
	ParaSWGridBlockDivision();
	virtual ~ParaSWGridBlockDivision();

public:
	//Because the Grid and Block Division of one problem is only needed when it is not solvable,
	//memory control of the Division must be done explicitly by call the functions below:
	//All the given "int style" heights and widths are with the value of (Bound.end-Bound.start) !!!!!!!!!!!!!!!!!!!
	/* first: divide problem into blocks, each block in h-direction assign to a certain processor */
	/* second: divide blocks into grids, which is to be saved */
	bool InitGridBlockDivision(
		int seq1_length, int seq2_length,		//length of two sequences, logical matrix is (len1+1)*(len2+1)
		int gridH_count, int gridW_count,		//number of grid to be saved
		ParaSWProcessorList *pList				//list of processors for this problem
 		);
	bool InitGridBlockDivisionSimple(			//simpler case: blockHeight all equal
		int seq1_length, int seq2_length,		//length of two sequences, logical matrix is (len1+1)*(len2+1)
		int gridH_count, int gridW_count,		//number of grid to be saved
		ParaSWProcessorList *pList				//list of processors for this problem
 		);

	void ReleaseGridBlockDivision();

	bool SearchPositionGrid(Position &pos,bool &isOnRows,int &i,int &j);		//search a position(i,j) in grid, return value in isOnRow & i & j

public:
	//first: divide problem into blocks, each block in h-direction assign to a certain processor
	Bound *blockHeights;	//bound of each block in h-direction, pos in global-logical-matrix, start at 0
	Bound *blockWidths;		//bound of each block in w-direction, pos in global-logical-matrix, start at 0
	int blockHeights_count;	//number of grids in h-direction
	int blockWidths_count;	//number of grids in w-direction
//	int *processorIDs;		//ID of processors in h-direction

	int *blockHeightOnGrid;	//if block's first row is on grid, ...OnGrid = 0x100 | i;
	int *blockWidthOnGrid;	//if block's last row is on grid, ...OnGrid = 0x200 | i;
							//if two rows are both on grid, ...OnGrid = 0x300 | i;
							//else ...OnGrid = 0x000 | i;

public:
	//second: divide blocks into grids, which is to be saved
	Bound *gridHeights;		//bound of each grid in h-direction, pos in global-logical-matrix, start at 0
	Bound *gridWidths;		//bound of each grid in w-direction, pos in global-logical-matrix, start at 0
	int gridHeights_count;	//number of grids in h-direction
	int gridWidths_count;	//number of grids in w-direction
};

/*****************************************************************************************************
* Class ParaSWParam
*	// Parameters for main problem.
******************************************************************************************************/
class ParaSWParam  
{
public:
	ParaSWParam();
	virtual ~ParaSWParam();

	void InitParameters(int ali_type,
						 Sequence *s1, Sequence *s2,
						 SubstitutionMatrix * sm,
						 int gap_o, int gap_e,
						 int proc_count, int solvable_size,
						 int *block_h, int block_w,
						 int grid_h_count,int grid_w_count,
						 int k_path);

	int		 align_type; //FASTLSA_DNA|ParaSW_PROTEIN; ParaSW_NW|ParaSW_SW; ParaSW_AFFINE|ParaSW_CONST;
	//type need : NW|SW Aff

	Sequence *seq1;       //File path or Data buff of seq1
	Sequence *seq2;       //File path or Data buff of seq2

	SubstitutionMatrix * pssm;	//position specific score matrix
	
	int		gap_open;   //gap open penalty
	int     gap_extend; //gap extension penalty
  
	int     processor_count; //def = 1
	int     block_width; //solvable block width of each processor, equal for all processors
	int     *block_height;//block height of each processor, or the capability comparison of the processors

	int		grid_height_count;	//one problem could have how many grid rows
	int		grid_width_count;	//one problem could have how many grid cols

	int		k;					// k nearest optimal paths
	
	int		solvableMemSize;	//size of score matrix which could be solved directly, in trace step
};

#endif // !defined(AFX_PARASWPARAM_H__A8D19220_B205_4136_A044_A3DF624B0558__INCLUDED_)
