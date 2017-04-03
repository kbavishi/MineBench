// ParallelSW.cpp : Defines the entry point for the console application.
//

#include <iostream>
#include "Common.h"
#include "Debug.h"


///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////

#include "ParaSWParam.h"

#include "ParseFile.h"

#include "ParaSWPath.h"

#include "ParaSWProcessorSet.h"
int ParaSWProcessor::processorCount = 0;

#include "ParaSWProblem.h"

#include "ParaSWGlobalMemory.h"

CDebug debugf;

#include "ParallelSWAlgorithm.h"

#ifndef WIN32
#include "wtime.h"
#else
#include "time.h"
#endif

int _num_procs;

int main(int argc, char *argv[])
{
#ifdef WIN32
	clock_t begin = clock();
#endif
	int *block_h, block_w, tmp_block_w, tmp_block_h;
	int grid_h_count;		//one problem could have how many grid rows
	int grid_w_count;		//one problem could have how many grid cols
	int k_path;				//output k-optimal paths
	
	CParseFile pf;				//for input
	
	int arg_s = 0;

	strcpy (pf.seq1, argv[arg_s+1]);	// sequence 1
	strcpy (pf.seq2, argv[arg_s+2]);	// sequence 2
	strcpy (pf.matrix, argv[arg_s+3]);	// matrix file
	
	tmp_block_h = atoi(argv[arg_s+4]);	// block height
	tmp_block_w = atoi(argv[arg_s+5]);	// block length
	
	grid_h_count = atoi(argv[arg_s+6]);
	grid_w_count = atoi(argv[arg_s+7]);
	k_path = atoi(argv[arg_s+8]);

	_num_procs = atoi(argv[arg_s+9]);
#ifdef _OPENMP
	omp_set_num_threads(_num_procs);
#endif
	
	printf("Parellel Smith Waterman Algorithm implementation for OpenMP\n");
	printf("Threads Number=%d\n",_num_procs);

	// Load sequence and matrix

	if (!pf.LoadFile(argc,argv))
	{
		printf ("Error reading files.\n\n");
		return -1;
	}
	int i;

	// Automaticly adjust block height and width according to the number of processors
	int size_h = pf.data1->length;
	int size_w = pf.data2->length;
	int tmp;

	tmp = size_h / (_num_procs*tmp_block_h);
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
	printf ("Seqence(1) Length=%d, Sequence Length(2)=%d\n Adjust block height=%d, block width=%d\n", size_h, size_w, tmp_block_h, tmp_block_w);

	block_h = new int[_num_procs];
	for(i=0;i<_num_procs;i++)
		block_h[i] = tmp_block_h;
	block_w = tmp_block_w;			//solvable block width of each processor, equal for all processors

	//-------------------------------------------
	//main objects of the algorithm:
	ParaSWParam			param;
	//-------------------------------------------

	//-------------------------------------------
	//basic settings:
	int ali_type = 0;
	Sequence *s1 = pf.data1;
	Sequence *s2 = pf.data2;
	SubstitutionMatrix * sm = pf.subMatrix;
	int gap_o = -12;			//gap open penalty
	int gap_e = -4;				//gap extension penalty
	int solvable_size = 10000;	//currently NOT used. Size of score matrix which could be solved directly, in trace step

	//-------------------------------------------
	//init parameters:
	param.InitParameters(ali_type,
						s1,s2,
						sm,
						gap_o,gap_e,
						_num_procs,solvable_size,
						block_h,block_w,
						grid_h_count,grid_w_count,
						k_path);
	debugf.PrintParaSWParam(&param,"main()");
	//-------------------------------------------
	//do the ParaSW algorithm:
	ParaSWPath * paths;
#ifndef WIN32
	timer_start(0);//this function call takes much time
	paths = ParaSW(param);
	timer_stop(0);
#else
	clock_t start, end;
	start = clock();
	paths = ParaSW(param);
	end = clock();
#endif

	//-------------------------------------------
	//output the results:
#ifndef WIN32
	printf ("\nSuccess!\nTotal time: %.2fs\n", timer_read(0));
#else
	printf ("\nSuccess!\nParaSW time: %.2fs\n", (float)(end-start)/CLOCKS_PER_SEC);
#endif

	//get k-paths from ResultQueue, save to each paths[i]
	GetPathsFromResultQueue(paths);

	char c[100];
	//sprintf(c,"ParaSW() time = %u milliseconds",time1);

	debugf.PrintTitle("End of ParaSW()",c);

	while(paths[0].PopFromTaskQueue() != NULL){}

	for(i=0;i<param.k;i++)
	{
		sprintf(c,"path[%d] :\n",i);
		paths[i].SortSolvedPath();
		debugf.PrintPath(&(paths[i]),c);
		paths[i].JointAllPath(s1,s2);
		//debugf.PrintAlignment(paths[i].seq1,paths[i].seq2,paths[i].ali,paths[i].length,c);
		debugf.PrintAlignment(&(paths[i]),c);
	}

	//-------------------------------------------
	//release memory:
	delete [] block_h;
	delete [] paths;
#ifdef WIN32
	end = clock();
	printf ("Total time: %.2fs\n", (float)(end-begin)/CLOCKS_PER_SEC);
#endif
	return 0;
	// exit (0);
}




