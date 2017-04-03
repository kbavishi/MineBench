/* rsearch.c
 *
 * Given an RNA sequence, a structure, and a matrix, constructs a 
 * covariance model and searches with it.
 *
 * Based on cmbuild and cmscore from INFERNAL.
 *
 * Robert J. Klein
 * Started March 8, 2002
 * 
 *****************************************************************
 * @LICENSE@
 ***************************************************************** 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "wtime.h"
#include <time.h>
// #include <unistd.h>
#include <ctype.h>

#ifdef _OPENMP
#include <omp.h>
#endif

#ifdef USE_MPI
#include "mpi.h"
#include "mpifuncs.h"
#endif

#include "structs.h"		/* data structures, macros, #define's   */
#include "funcs.h"		/* external functions                   */
#include "squid.h"		/* general sequence analysis library    */
#include "msa.h"                /* squid's multiple alignment i/o       */
#include "histogram.h"          /* EVD histogram routines from HMMER    */
#include "stats.h"

/* coordinate -- macro that checks if it's reverse complement and if so 
   returns coordinate in original strand
   a = true if revcomp, false if not
   b = the position in current seq
   c = length of the seq
*/
#define coordinate(a,b,c) ( a ? -1*b+c+1 : b)

/* Various defaults defined here */
#define DEFAULT_MATRIX "RIBOSUM85-60"
#define DEFAULT_ALPHA 10.
#define DEFAULT_BETA 5.
#define DEFAULT_ALPHAP 0.
#define DEFAULT_BETAP 15.
#define DEFAULT_BEGINSC 0.
#define DEFAULT_ENDSC -15.
#define DEFAULT_NUM_SAMPLES 0
#define DEFAULT_CUTOFF 0.0
#define DEFAULT_CUTOFF_TYPE SCORE_CUTOFF

static int in_mpi;

static char banner[] = "rsearch -- searches a database with an RNA sequence/structure\n";

static char usage[]  = "\
Usage: rsearch [-options] <query sequence file> <database file>\n\
The sequence file is expected to be in FASTA format.\n\
  Available options are:\n\
   -h     : help; print brief help on version and usage\n\
   -c     : complement: also search reverse complement strand\n\
   -m <s> : matrix: use matrix <s>, either full path or in $RNAMAT\n\
   -n <n> : Determine EVD with N samples (0 = no P-values reported)\n\
   -L <n> : Use length L for determing EVD (default = 2*D)\n\
   -s <n> : Use cutoff score of n (default 0)\n\
   -E <n> : Use cutoff E-value of n (default ignored; overrides -s\n\
   -B     : Babelfish; autodetect alternative sequence file format\n\
";

static char experts[] = "\
   --queryformat <s>: specify that input alignment is in format <s>, not STOCKHOLM\n\
   --dbformat <s>   : specify that database is in format <s>, not FASTA\n\
   --alpha <n>      : set alpha to -n (gap open penalty)\n\
   --beta <n>       : set beta to -n (gap position penalty)\n\
   --alphap <n>     : set alpha' to -n (stem gap open penalty)\n\
   --betap <n>      : set beta' to -n (stem gap position penalty)\n\
   --noalign        : Don't print out alignments; only print score\n\
   --seed <n>       : Set random seed to n\n\
   --beginsc <n>    : Penalty for entering model locally\n\
   --endsc <n>      : Penalty for leaving model locally\n\
   --dscale <n>     : Factor to scale query length by to get D (default=2)\n\
   --partition <n>[,<n>]... : Parition points for different GC content EVDs\n\
   --cmquery        : Query file is INFERNAL (v0.54) model built with cmbuild\n\
";

static struct opt_s OPTIONS[] = {
  { "-h", TRUE, sqdARG_NONE }, 
  { "-c", TRUE, sqdARG_NONE },
  { "-m", TRUE, sqdARG_STRING },
  { "-n", TRUE, sqdARG_INT },
  { "-L", TRUE, sqdARG_INT },
  { "-s", TRUE, sqdARG_FLOAT },
  { "-B", TRUE, sqdARG_NONE },
  { "-E", TRUE, sqdARG_FLOAT },
  { "--queryformat",  FALSE, sqdARG_STRING },
  { "--dbformat", FALSE, sqdARG_STRING },
  { "--alpha", FALSE, sqdARG_FLOAT },
  { "--beta", FALSE, sqdARG_FLOAT },
  { "--alphap", FALSE, sqdARG_FLOAT },
  { "--betap", FALSE, sqdARG_FLOAT },
  { "--noalign", FALSE, sqdARG_NONE },
  { "--seed", FALSE, sqdARG_INT},
  { "--beginsc", FALSE, sqdARG_FLOAT},
  { "--endsc", FALSE, sqdARG_FLOAT},
  { "--dscale", FALSE, sqdARG_FLOAT},
  { "--partition", FALSE, sqdARG_STRING},
  { "--cmquery", FALSE, sqdARG_NONE }
};
#define NOPTIONS (sizeof(OPTIONS) / sizeof(struct opt_s))

/* Function: set_partitions
 * Date:     RJK, Mon, Oct 7, 2002 [St. Louis]
 * Purpose:  Given a partion array (int [100]) which contains what parttion
 *           number each %GC is in, sets a new partition by dividing the one
 *           each partition point is in.
 */
int set_partitions (int *partition, int *num_partitions, char *list) {
  int cur_point = 0;
  int old_partition;

  while (*list != '\0') {
    if (isdigit(*list)) {
      cur_point = (cur_point * 10) + (*list - '0');
    } else if (*list == ',') {
      if (cur_point < 0 || cur_point >= GC_SEGMENTS) 
	return(1);
      if (partition[cur_point] == partition[cur_point-1]) {
	old_partition = partition[cur_point];
	while (partition[cur_point] == old_partition) {
	  partition[cur_point] = *num_partitions;
	  cur_point++;
	}
      }
      (*num_partitions)++;
      cur_point = 0;
    } else {
      return(1);
    }
    list++;
  }
  if (cur_point < 0 || cur_point >= GC_SEGMENTS)
    return(1);

  if (partition[cur_point] == partition[cur_point-1]) {
    old_partition = partition[cur_point];
    while (partition[cur_point] == old_partition) {
      partition[cur_point] = *num_partitions;
      cur_point++;
    }
  }
  (*num_partitions)++;
  return(0);
}



/*
 * Function: read_next_seq
 * Date:     RJK, Wed May 29, 2002 [St. Louis]
 * Purpose:  Given a dbfp and whether or not to take the reverse complement,
 *           reads in the sequence and prepares reverse complement.
 */
db_seq_t *read_next_seq (SQFILE *dbfp, int do_complement) {
  db_seq_t *retval;
  int readseq_return;

  retval = MallocOrDie(sizeof(db_seq_t));

  do {
    readseq_return = ReadSeq(dbfp, dbfp->format, &(retval->seq[0]), &(retval->sqinfo));
  } while (retval->sqinfo.len == 0 && readseq_return != 0);
  if (readseq_return == 0) return(NULL);

  s2upper (retval->seq[0]);                      /* Makes uppercase */

  /* Digitize the sequence */
  retval->dsq[0] = DigitizeSequence (retval->seq[0], retval->sqinfo.len);

  if (do_complement) {
    retval->seq[1] = MallocOrDie(sizeof(char)*(retval->sqinfo.len+1));
    revcomp (retval->seq[1], retval->seq[0]);
    retval->dsq[1] = DigitizeSequence (retval->seq[1], retval->sqinfo.len);
  }

  retval->results[0] = NULL;
  retval->results[1] = NULL;

  return(retval);
}

/*
 * print_alignment
 *
 * Given a model and the parse tree and digitized sequence for the target,
 * prints the annotated alignment of the sequence.
 * | = MATL or MATR, >< are used for MATP
 *
 * i and j are the start and stop coordinates to print
 *
 * Based on ParsetreeDump from parsetree.c
 */
void print_alignment (CM_t *cm, Parsetree_t *tr, char *dsq, int i, int j) {
  int   x;
  int   v;
  int size;
  char *seqbuf, *alibuf;
  char outbuf[61];

  size = j-i+1;

  seqbuf = MallocOrDie (sizeof(char)*(size+1));
  alibuf = MallocOrDie (sizeof(char)*(size+1));
  seqbuf[size] = '\0';
  alibuf[size] = '\0';
  for (x=0; x<size; x++) {
    seqbuf[x] = '-';
    alibuf[x] = ' ';
  }

  for (x = 0; x < tr->n; x++) {
    v = tr->state[x];
    if (cm->sttype[v] == MP_st) {
      seqbuf[tr->emitl[x]-i] = Alphabet[(int)dsq[tr->emitl[x]]]; 
      seqbuf[tr->emitr[x]-i] = Alphabet[(int)dsq[tr->emitr[x]]];
      alibuf[tr->emitl[x]-i] = '>';
      alibuf[tr->emitr[x]-i] = '<';
    } else if (cm->sttype[v] == IL_st || cm->sttype[v] == ML_st) {
	seqbuf[tr->emitl[x]-i] = Alphabet[(int)dsq[tr->emitl[x]]];
        if (cm->sttype[v] == ML_st) {
	  alibuf[tr->emitl[x]-i] = '|';
	}
    } else if (cm->sttype[v] == IR_st || cm->sttype[v] == MR_st) {
	seqbuf[tr->emitr[x]-i] = Alphabet[(int)dsq[tr->emitr[x]]];
	if (cm->sttype[v] == MR_st) {
	  alibuf[tr->emitr[x]-i] = '|';
	}
    }
  }

  outbuf[60] = '\0';
  for (x=0; x<size; x+=60) {
    strncpy (outbuf, seqbuf+x, 60);
    printf ("     %s\n", outbuf);
    strncpy (outbuf, alibuf+x, 60);
    printf ("     %s\n\n", outbuf);
  }
}

/* Function: get_gc_comp
 * Date:     RJK, Mon Oct 7, 2002 [St. Louis]
 * Purpose:  Given a sequence and start and stop coordinates, returns 
 *           integer GC composition of the region 
 */
int get_gc_comp(char *seq, int start, int stop) {
  int i;
  int gc_count;
  char c;

  if (start > stop) {
    i = start;
    start = stop;
    stop = i;
  }

  gc_count = 0;
  for (i=start; i<=stop; i++) {
    c = resolve_degenerate(seq[i]);
    if (c=='G' || c == 'C')
      gc_count++;
  }
  return ((int)(100.*gc_count/(stop-start+1)));
}


/*
 * Function: print_results
 * Date:     RJK, Wed May 29, 2002 [St. Louis]
 * Purpose:  Given the needed information, prints the results.
 *
 *           cm                  the model
 *           cons                consensus seq for model (query seq)
 *           dbseq               the database seq
 *           name                sequence name
 *           len                 length of the sequence
 *           in_complement       are we doing the minus strand
 *           do_stats            should we calculate stats?
 *           mu, lambda          for statistics
 */
void print_results (CM_t *cm, CMConsensus_t *cons, db_seq_t *dbseq,
		    int do_complement, int do_stats, float *mu, 
		    float *lambda) {
  int i;
  char *name;
  int len;
  scan_results_t *results;
  Fancyali_t *ali;
  int in_complement;
  int header_printed = 0;
  int gc_comp;

  name = dbseq->sqinfo.name;
  len = dbseq->sqinfo.len;

  for (in_complement = 0; in_complement <= do_complement; in_complement++) {
    results = dbseq->results[in_complement];
    if (results == NULL || results->num_results == 0) continue;

    if (!header_printed) {
      header_printed = 1;
      printf (">%s\n\n", name);
    }
    printf ("  %s strand results:\n\n", in_complement ? "Minus" : "Plus");

    for (i=0; i<results->num_results; i++) {
      printf (" Query = %d - %d, Target = %d - %d\n", 
	      cons->lpos[cm->ndidx[results->data[i].bestr]]+1,
	      cons->rpos[cm->ndidx[results->data[i].bestr]]+1,
	      coordinate(in_complement, results->data[i].start, len), 
	      coordinate(in_complement, results->data[i].stop, len));
      if (do_stats) {
	gc_comp = get_gc_comp (dbseq->seq[in_complement], 
			       results->data[i].start, results->data[i].stop);
	printf (" Score = %.2f, E = %.4g, P = %.4g\n", results->data[i].score,
		RJK_ExtremeValueE(results->data[i].score, mu[gc_comp], 
				  lambda[gc_comp]),
		ExtremeValueP(results->data[i].score, mu[gc_comp], 
			      lambda[gc_comp]));
      } else {
	printf (" Score = %.2f\n", results->data[i].score);
      }
      printf ("\n");
      if (results->data[i].tr != NULL) {
	ali = CreateFancyAli (results->data[i].tr, cm, cons, 
			      dbseq->dsq[in_complement] +
			      (results->data[i].start-1));
	PrintFancyAli(stdout, ali);
	printf ("\n");
      }
    }
  }
  fflush(stdout);
}

/*
 * Function: remove_hits_over_e_cutoff
 * Date:     RJK, Tue Oct 8, 2002 [St. Louis]
 * Purpose:  Given an E-value cutoff, lambdas, mus, a sequence, and
 *           a list of results, calculates GC content for each hit, 
 *           calculates E-value, and decides wheter to keep hit or not.
 */
void remove_hits_over_e_cutoff (scan_results_t *results, char *seq,
				float cutoff, float *lambda, float *mu) {
  int gc_comp;
  int i, x;
  scan_result_node_t swap;

  if (results == NULL)
    return;

  for (i=0; i<results->num_results; i++) {
    gc_comp = get_gc_comp (seq, results->data[i].start, results->data[i].stop);
    if (RJK_ExtremeValueE(results->data[i].score, 
			  mu[gc_comp], lambda[gc_comp])> cutoff) {
      results->data[i].start = -1;
    }
  }

  
  for (x=0; x<results->num_results; x++) {
    while (results->num_results > 0 && 
	   results->data[results->num_results-1].start == -1)
      results->num_results--;
    if (x<results->num_results && results->data[x].start == -1) {
      swap = results->data[x];
      results->data[x] = results->data[results->num_results-1];
      results->data[results->num_results-1] = swap;
      results->num_results--;
    }
  }
  while (results->num_results > 0 &&
	 results->data[results->num_results-1].start == -1)
    results->num_results--;

  sort_results(results);
}  


/*
 * Function: serial_search_databas
 * Date:     RJK, Tue May 28, 2002 [St. Louis]
 * Purpose:  Given an open database file, a model, and various parameters, does
 * the search using CYKScan and then determines and prints out the alignments.
 *
 * Parameters:        dbfp         the database
 *                    cm           the model
 *                    D            maximum size of hit
 *                    score_cutoff min. score to report
 *                    do_complement search complementary strand
 *                    do_align     calculate and do alignment
 *                    do_stats     calculate statistics
 *                    mu           for stats
 *                    lambda        "
 */
void serial_search_database (SQFILE *dbfp, CM_t *cm, CMConsensus_t *cons,
			     int D, float cutoff, int cutoff_type, 
			     int do_complement, int do_align, int do_stats,
			     float *mu, float *lambda) {

  int in_complement;            /* Am I currently doing reverse complement? */
  int i,a;
  db_seq_t *dbseq;
  float min_cutoff;

  if (cutoff_type == SCORE_CUTOFF) {
    min_cutoff = cutoff;
  } else {
    min_cutoff = e_to_score (cutoff, mu, lambda);
  }


  while ((dbseq = read_next_seq (dbfp, do_complement))) {
    for (in_complement = 0; in_complement <= do_complement; in_complement++) {
      /* Scan */
      dbseq->results[in_complement] = CreateResults(INIT_RESULTS);
      CYKScan (cm, dbseq->dsq[in_complement], dbseq->sqinfo.len, min_cutoff, 
	       D, dbseq->results[in_complement]);
      remove_overlapping_hits (dbseq->results[in_complement],
			       dbseq->sqinfo.len);
      if (cutoff_type == E_CUTOFF) {
	remove_hits_over_e_cutoff (dbseq->results[in_complement],
				   dbseq->seq[in_complement],
				   cutoff, lambda, mu);
      }
      /* Align results */
      if (do_align) {
	for (i=0; i<dbseq->results[in_complement]->num_results; i++) {
	  CYKDivideAndConquer 
	    (cm, dbseq->dsq[in_complement], dbseq->sqinfo.len, 
	     dbseq->results[in_complement]->data[i].bestr,
	     dbseq->results[in_complement]->data[i].start, 
	     dbseq->results[in_complement]->data[i].stop, 
	     &(dbseq->results[in_complement]->data[i].tr));
	  /* Now, subtract out the starting point of the result so 
	     that it can be added in later.  This makes the print_alignment
	     routine compatible with the parallel version, while not needing
	     to send the entire database seq over for each alignment. */
	  for (a=0; a<dbseq->results[in_complement]->data[i].tr->n; a++) {
	    dbseq->results[in_complement]->data[i].tr->emitl[a] -= 
	      (dbseq->results[in_complement]->data[i].start - 1);
	    dbseq->results[in_complement]->data[i].tr->emitr[a] -= 
	      (dbseq->results[in_complement]->data[i].start - 1);
	  }
	}
      }
    }
    /* Print results */
    print_results (cm, cons, dbseq, do_complement, do_stats,
		   mu, lambda);
    fflush (stdout);

    FreeResults(dbseq->results[0]);
    free(dbseq->dsq[0]);
    if (do_complement) {
      FreeResults(dbseq->results[1]);
      free(dbseq->dsq[1]);
      free(dbseq->seq[1]);
    }
    FreeSequence(dbseq->seq[0], &(dbseq->sqinfo));
  }
}


#ifdef _OPENMP
int start_proc;
#pragma omp threadprivate(start_proc)

void my_search_database (SQFILE *dbfp, CM_t *cm, CMConsensus_t *cons, 
			       int D, float cutoff, int cutoff_type,
			       int do_complement,
			       int do_align, int do_stats,
			       float *mu, float *lambda)
{
	db_seq_t * cur_seq;

	scan_results_t ** results_array;
	scan_results_t ** results;
	int *incomplement_array;

	int seq_index;
	float min_cutoff;
	int left, right;
	Parsetree_t * tr=NULL;
	int num;

    int thread_nums = omp_get_max_threads();
    results_array = (scan_results_t **)malloc(sizeof(scan_results_t *) * 2 * thread_nums);

	if (cutoff_type == SCORE_CUTOFF) {
    		min_cutoff = cutoff;
    	}else
    	{
    		min_cutoff = e_to_score (cutoff, mu, lambda);
    	}
    		
	seq_index=0;
	cur_seq = read_next_seq(dbfp, do_complement);

#pragma omp parallel
{
	start_proc = 0;
}

	
	while(cur_seq!=NULL)
	{		
		int chunksize, chunkoffset, n_procs, l;
		seq_index++;		
	    get_chunk_distribution(cur_seq, D, thread_nums, do_complement, &chunksize, &chunkoffset, &n_procs);

#pragma omp parallel private(results, tr)
{
	    char * seq;		
	    int i, j, k, incomplement, seqlen, old_start, flag;
	    int thread_id    = omp_get_thread_num();
        results = results_array + 2 * thread_id;
		for(incomplement=0; incomplement<=do_complement; incomplement++)
		{
			results[incomplement] = CreateResults(INIT_RESULTS);		
		}
		
		i=0; j=0;
		old_start = -1;
		incomplement = 0;

		flag = get_my_chunk(cur_seq, thread_id, thread_nums, chunksize, chunkoffset, &i, &j, &incomplement, &start_proc);
		while(flag)
		{	
			//scan chunk
	        scan_results_t *tmp_results = CreateResults(INIT_RESULTS);
			old_start = start_proc;
			seqlen = j-i+1;
			seq = MallocOrDie(sizeof(char)*(seqlen+2));
			seq[0] = seq[seqlen+1] = DIGITAL_SENTINEL;
			memcpy(seq+1, cur_seq->dsq[incomplement]+i, seqlen);			
			
			CYKScan (cm, seq, seqlen, min_cutoff, D, tmp_results);
			//report really position
			for(k=0; k<tmp_results->num_results; k++)
			{
				if(i==1||tmp_results->data[k].start>D)
				{
					report_hit(i+tmp_results->data[k].start-1, i+tmp_results->data[k].stop-1, tmp_results->data[k].bestr, tmp_results->data[k].score, results[incomplement]);
				}				
			}						
			FreeResults(tmp_results);
			free(seq);		
			flag = get_my_chunk(cur_seq, thread_id, thread_nums, chunksize, chunkoffset, &i, &j, &incomplement, &start_proc);
		}		
				
		for(incomplement=0; incomplement <= do_complement; incomplement ++)
		{			
			if(old_start >= 0)
			{
				remove_overlapping_hits(results[incomplement], cur_seq->sqinfo.len);
				if (cutoff_type == E_CUTOFF)
					remove_hits_over_e_cutoff(results[incomplement], cur_seq->seq[incomplement], cutoff, lambda, mu);
				
				if(do_align)
				{					
					for(k=0; k < results[incomplement]->num_results; k++)
					{
						seqlen = results[incomplement]->data[k].stop - results[incomplement]->data[k].start+1;
						seq = MallocOrDie(sizeof(char)*(seqlen+2));
						seq[0] = seq[seqlen+1] = DIGITAL_SENTINEL;
						memcpy(seq+1, cur_seq->dsq[incomplement]+results[incomplement]->data[k].start, seqlen);
						
						CYKDivideAndConquer(cm, seq, seqlen, results[incomplement]->data[k].bestr, 1, seqlen, &tr);
						results[incomplement]->data[k].tr = tr;
						free(seq);
					}
				}
			}
		}
}

        CombineResults(cur_seq->results, results_array, n_procs, do_complement);
		for(l = 0; l <= do_complement; l ++)
				remove_overlapping_hits(cur_seq->results[l], cur_seq->sqinfo.len);
		print_results(cm, cons, cur_seq, do_complement, do_stats, mu, lambda);

        for (l = 0; l < thread_nums; l ++) {
			SFree_Results(results_array[l * 2]);
			if(do_complement)
				SFree_Results(results_array[l * 2 + 1]);
		}

		if(do_complement)
		{
			free(cur_seq->dsq[1]);
			free(cur_seq->seq[1]);
		}
		free(cur_seq->dsq[0]);
		FreeSequence(cur_seq->seq[0], &(cur_seq->sqinfo));
		
		cur_seq = read_next_seq(dbfp, do_complement);
	}

    free(results_array);
}

#endif

#ifdef USE_MPI
/*
 * Function: parallel_search_database
 * Date:     RJK, Tue May 28, 2002 [St. Louis]
 * Purpose:  Given the same parameters as serial_search_database, does
 *           the database search with alignments and printing results, but
 *           in a parallel fashion.
 *     
 *           It the master node performs the following tasks:
 *           while (!eof(dbfp) && slaves_working) {
 *             For each empty process, send next job
 *             Wait for a result
 *             For each empty process, send next job
 *             Process result
 *           }
 *            
 *
 *           The slave processes do the following:
 *           while (1) {
 *             recieve_job
 *             if (terminate code) return;
 *             do the job
 *             send the results
 *           }
 */
void parallel_search_database (SQFILE *dbfp, CM_t *cm, CMConsensus_t *cons, 
							   int D, float cutoff, int cutoff_type,
							   int do_complement,
							   int do_align, int do_stats,
							   float *mu, float *lambda,
							   int mpi_my_rank, int mpi_master_rank, 
							   int mpi_num_procs) {
	char job_type;
	int seqlen;
	char *seq;
	scan_results_t *results;
	db_seq_t **active_seqs;
	job_t **process_status;
	int eof = FALSE;
	job_t *job_queue = NULL;
	int proc_index, active_seq_index;
	int bestr;       /* Best root state -- for alignments */
	Parsetree_t *tr;
	float min_cutoff;
	
	if (cutoff_type == SCORE_CUTOFF) {
		min_cutoff = cutoff;
	} else {
		min_cutoff = e_to_score (cutoff, mu, lambda);
	}
	
	if (mpi_my_rank == mpi_master_rank) {
    /* Set up arrays to hold pointers to active seqs and jobs on
		processes */
		active_seqs = MallocOrDie(sizeof(db_seq_t *) * mpi_num_procs);
		process_status = MallocOrDie(sizeof(job_t *)*mpi_num_procs);
		for (active_seq_index=0; active_seq_index<mpi_num_procs; 
		active_seq_index++) 
			active_seqs[active_seq_index] = NULL;
		for (proc_index = 0; proc_index < mpi_num_procs; proc_index++)
			process_status[proc_index] = NULL;
		
		
		do {
			/* Check for idle processes.  Send jobs */
			for (proc_index=0; proc_index<mpi_num_procs; proc_index++) {
				if (proc_index == mpi_master_rank) continue;  /* Skip master process */
				if (process_status[proc_index] == NULL) {         
					/* I'm idle -- need a job */
					if (job_queue == NULL) {           /* Queue is empty */
						/* Find next non-master open process */
						for (active_seq_index=0; active_seq_index<mpi_num_procs; 
						active_seq_index++) {
							if (active_seqs[active_seq_index] == NULL) break;
						}
						if (active_seq_index == mpi_num_procs) {
							Die ("Tried to read more than %d seqs at once\n", mpi_num_procs);
						}
						active_seqs[active_seq_index] = read_next_seq(dbfp, do_complement);
						if (active_seqs[active_seq_index] == NULL) {
							eof = TRUE;
							break;            /* Queue is empty and no more seqs */
						}
						else
							job_queue = enqueue (active_seqs[active_seq_index], 
							active_seq_index, D, do_complement, 
							STD_SCAN_WORK);
					}
					if (job_queue != NULL)
						send_next_job (&job_queue, process_status + proc_index, 
						proc_index);
				} 
			} 
			/* Wait for next reply */
			if (procs_working(process_status, mpi_num_procs, mpi_master_rank)) {
				active_seq_index = check_results (active_seqs, process_status, D);
				if (active_seqs[active_seq_index]->chunks_sent == 0) {
					remove_overlapping_hits
						(active_seqs[active_seq_index]->results[0], 
						active_seqs[active_seq_index]->sqinfo.len);
					if (cutoff_type == E_CUTOFF)
						remove_hits_over_e_cutoff 
						(active_seqs[active_seq_index]->results[0],
						active_seqs[active_seq_index]->seq[0],
						cutoff, lambda, mu);
					if (do_complement) {
						remove_overlapping_hits 
							(active_seqs[active_seq_index]->results[1],
							active_seqs[active_seq_index]->sqinfo.len);
						if (cutoff_type == E_CUTOFF)
							remove_hits_over_e_cutoff 
							(active_seqs[active_seq_index]->results[1],
							active_seqs[active_seq_index]->seq[1],
							cutoff, lambda, mu);
					}
					/* Check here if doing alignments and queue them or check if 
					all done */
					if (do_align && 
						active_seqs[active_seq_index]->alignments_sent == -1) {
						enqueue_alignments (&job_queue, active_seqs[active_seq_index],
							active_seq_index, do_complement, ALIGN_WORK);
					}
					if (!do_align || 
						active_seqs[active_seq_index]->alignments_sent == 0) {
						print_results (cm, cons, active_seqs[active_seq_index], 
							do_complement, do_stats, mu, lambda);
						if (do_complement) {
							FreeResults(active_seqs[active_seq_index]->results[1]);
							free(active_seqs[active_seq_index]->dsq[1]);
							free(active_seqs[active_seq_index]->seq[1]);
						}
						FreeResults(active_seqs[active_seq_index]->results[0]);
						free(active_seqs[active_seq_index]->dsq[0]);
						FreeSequence(active_seqs[active_seq_index]->seq[0], 
							&(active_seqs[active_seq_index]->sqinfo));
						active_seqs[active_seq_index] = NULL;
					}
				}
			}
		} while (!eof || job_queue != NULL || 
			(procs_working(process_status, mpi_num_procs, mpi_master_rank)));
		for (proc_index=0; proc_index<mpi_num_procs; proc_index++) {
			if (proc_index != mpi_master_rank) {
				send_terminate (proc_index);
			}
		}
		free(active_seqs);
		free(process_status);
	} else {
		seq = NULL;
		do {
			job_type = recieve_job (&seqlen, &seq, &bestr, mpi_master_rank);
			if (job_type == STD_SCAN_WORK) {
				results = CreateResults(INIT_RESULTS);
				CYKScan (cm, seq, seqlen, min_cutoff, D, results);
				send_scan_results (results, mpi_master_rank);
				FreeResults(results);
			} else if (job_type == ALIGN_WORK && do_align) {
				CYKDivideAndConquer(cm, seq, seqlen, bestr, 1, seqlen, &tr);
				send_align_results (tr, mpi_master_rank);
				FreeParsetree(tr);
			}
			if (seq != NULL)
				free(seq);
			seq = NULL;
		} while (job_type != TERMINATE_WORK);
	}
	MPI_Barrier(MPI_COMM_WORLD);
}

/*
 * Function: exit_from_mpi
 * Date:     RJK, Thu Jun 6, 2002 [St. Louis]
 * Purpose:  Calls MPI_Abort on exit if in_mpi flag is 1, otherwise
 *           returns
 */
void exit_from_mpi () {
  if (in_mpi)
    MPI_Abort (MPI_COMM_WORLD, -1);
}

#endif

int
main(int argc, char **argv)
{
  char            *queryfile;    /* file to read query sequence from */
  char            *dbfile;      /* Database file */
  int              queryformat;      /* format of sequence file */
  int              dbformat;         
  FILE            *matfp;       /* open matrix file for reading */
  MSAFILE	  *queryfp;     /* open query seqfile for reading */
  int             querylen;     /* length of the query file */
  SQFILE          *dbfp;        /* Database file for reading */
  CM_t            *cm;          /* a covariance model       */
  CMConsensus_t   *cons;        /* Sequence information for printing
				   the query in the alignment */
  fullmat_t       *fullmat;     /* The full matrix */
  float           alpha = DEFAULT_ALPHA; 
  float           beta = DEFAULT_BETA;
  float           alphap = DEFAULT_ALPHAP;
  float           betap = DEFAULT_BETAP;
  float           min_alpha_beta_sum;

  char matrixname[256];         /* Name of the matrix, from -m */
  int num_samples = DEFAULT_NUM_SAMPLES;
  int sample_length = -1;       /* Flag for set to 2*D */
  int do_complement = 0;        /* Shall I do reverse complement? */

  float cutoff = DEFAULT_CUTOFF;
  int cutoff_type = DEFAULT_CUTOFF_TYPE;

  int do_align = TRUE;
  float lambda[GC_SEGMENTS];
  float K[GC_SEGMENTS];
  float mu[GC_SEGMENTS];        /* Set from lambda, K, N */
  int seed;                     /* Random seed */

  long N;                        /* Effective number of sequences for this search */
  int defined_N = FALSE;
  float beginsc = DEFAULT_BEGINSC;
  float endsc = DEFAULT_ENDSC;
  float D_scale = 2.0;          /* Scale for querylen to get D */

  int partitions[GC_SEGMENTS];       /* What partition each percentage point goes to */
  int num_partitions = 1;
  int gc_count[GC_SEGMENTS];

  int cmquery = 0;              /* Set to true with --cmquery */

  int i;

  char *optname;                /* name of option found by Getopt()        */
  char *optarg;                 /* argument found by Getopt()              */
  int   optind;                 /* index in argv[]                         */

  float dura_his, dura_fin;
  double start_clock, end_clock;

#ifdef USE_MPI
  int mpi_my_rank;              /* My rank in MPI */
  int mpi_num_procs;            /* Total number of processes */
  int mpi_master_rank;          /* Rank of master process */

  /* Initailize MPI, get values for rank and num procs */
  MPI_Init (&argc, &argv);

  atexit (exit_from_mpi);
  in_mpi = 1;                /* Flag for exit_from_mpi() */

  MPI_Comm_rank (MPI_COMM_WORLD, &mpi_my_rank);
  MPI_Comm_size (MPI_COMM_WORLD, &mpi_num_procs);

  /*
   * Determine master process.  This is the lowest ranking one that can do I/O
   */
  mpi_master_rank = get_master_rank (MPI_COMM_WORLD, mpi_my_rank);

  /* If I'm the master, do the following set up code -- parse arguments, read
     in matrix and query, build model */
  if (mpi_my_rank == mpi_master_rank) {
#endif

  /**********************************************
   * Print header here 
   *********************************************/
  // printf ("RSEARCH version %s\n", VERSION);
#ifdef USE_MPI
  printf ("Running in parallel with %d processes\n", mpi_num_procs);
#endif
  // printf ("%s\n%s\n\n", COPYRIGHT, LICENSE);


  /* Seed the random number generator with the time */
  /* This is the seed used in the HMMER code */
  seed = (time ((time_t *) NULL));  
  //seed = 10000;

  /* Initialize partition array */
  for (i = 0; i < GC_SEGMENTS; i++) 
    partitions[i] = 0;

  /*********************************************** 
   * Parse command line
   ***********************************************/

  queryformat            = MSAFILE_STOCKHOLM;
  dbformat               = SQFILE_FASTA;
  matrixname[0]          = '\0';

  while (Getopt(argc, argv, OPTIONS, NOPTIONS, usage,
                &optind, &optname, &optarg))  {
    if      (strcmp(optname, "-B") == 0) {
      queryformat            = SQFILE_UNKNOWN;
      dbformat               = SQFILE_UNKNOWN;
    }
    else if (strcmp(optname, "-c") == 0) {
      do_complement = 1;
    }
    else if (strcmp(optname, "-m") == 0) {
      if (strlen(optarg) > 255)
	Die ("Matrix name can't exceed 255 characters\n");
      strncpy (matrixname, optarg, 255);
      matrixname[255] = '\0';
    }
    else if (strcmp (optname, "-n") == 0) {
      num_samples = atoi(optarg);
    }
    else if (strcmp (optname, "-L") == 0) {
      sample_length = atoi(optarg);
    }
    else if (strcmp (optname, "-s") == 0) {
      cutoff = atof(optarg);
      cutoff_type = SCORE_CUTOFF;
    }
    else if (strcmp(optname, "-E") == 0) {
      cutoff = atof(optarg);
      cutoff_type = E_CUTOFF;
    }
    else if (strcmp(optname, "--queryformat") == 0) {
      queryformat = String2SeqfileFormat(optarg);
      if (queryformat == SQFILE_UNKNOWN) 
	Die("unrecognized sequence file format \"%s\"", optarg);
    } 
    else if (strcmp (optname, "--dbformat") == 0) {
      dbformat = String2SeqfileFormat(optarg);
      if (dbformat == SQFILE_UNKNOWN)
	Die("unrecognized sequence file format \"%s\"", optarg);
    }
    else if (strcmp (optname, "--alphap") == 0) {
      alphap = atof(optarg);
    }
    else if (strcmp (optname, "--betap") == 0) {
      betap = atof(optarg);
    }
    else if (strcmp (optname, "--alpha") == 0) {
      alpha = atof(optarg);
    } 
    else if (strcmp (optname, "--beta") == 0) {
      beta = atof(optarg);
    } else if (strcmp (optname, "--noalign") == 0) {
      do_align = FALSE;
    } else if (strcmp (optname, "--seed") == 0) {
      seed = atoi(optarg);
    } else if (strcmp (optname, "--beginsc") == 0) {
      beginsc = -1*atof(optarg);
    } else if (strcmp (optname, "--endsc") == 0) {
      endsc = -1*atof(optarg);
    } else if (strcmp (optname, "--dscale") == 0) {
      D_scale = atof(optarg);
    } else if (strcmp (optname, "--partition") == 0) {
      if (set_partitions (partitions, &num_partitions, optarg)) {
	Die("Specifiy partitions separated by commas, no spaces, range 0-100, integers only\n");
      }
    } else if (strcmp (optname, "--cmquery") == 0) {
      cmquery = 1;
    }
    else if (strcmp(optname, "-h") == 0) {
      puts(usage);
      puts(experts);
      exit(EXIT_SUCCESS);
    }
  }

  if (argc - optind != 2) 
    Die("Incorrect number of arguments.\n%s\n", usage);
  queryfile = argv[optind++]; 
  dbfile = argv[optind++];


  /**********************************************
   * Seed random number generator
   **********************************************/
  sre_srandom (seed);
  printf ("Random seed: %d\n", seed);

  printf ("D scale of %.1f\n", D_scale);

  /*********************************************** 
   * Preliminaries: open our files for i/o
   ***********************************************/

  /* Query file.  Use MSAFileOpen if cmquery is FALSE, otherwise wait until
     later */
  if (cmquery == 0) {
    if ((queryfp = MSAFileOpen(queryfile, queryformat, NULL)) == NULL)
      Die("Failed to open sequence database file %s\n%s\n", queryfile, usage);
  } 
  /* Database and matrix */
  if ((dbfp = SeqfileOpen (dbfile, dbformat, NULL)) == NULL)
    Die ("Failed to open sequence database file %s\n%s\n", dbfile, usage);
  //if ((matfp = MatFileOpen (DEFAULT_MATRIX, getenv("RNAMAT"), matrixname)) == NULL) 
  //if ((matfp = MatFileOpen (DEFAULT_MATRIX, "C:\\rsearch-1.1\\matrices", matrixname)) == NULL) 
  if ((matfp = MatFileOpen (DEFAULT_MATRIX, "C:\\", matrixname)) == NULL) 
    Die ("Failed to open matrix file\n%s\n", usage);

  /**************************************************
   *   Set up matrix
   **************************************************/
  if (! (fullmat = ReadMatrix(matfp)))
    Die ("Failed to read matrix file \n%s\n", usage);
  printf ("Matrix: %s\n", fullmat->name);

  /*************************************************
   *    Print alphas and betas
   *************************************************/
  printf ("Alpha: %.2f\n", alpha);
  printf ("Beta: %.2f\n", beta);
  printf ("Alpha': %.2f\n", alphap);
  printf ("Beta': %.2f\n", betap);

  /********************************************
   *    Check if alpha+beta sum too low 
   ********************************************/
  min_alpha_beta_sum = get_min_alpha_beta_sum(fullmat);
  if (alpha + beta < min_alpha_beta_sum) {
    Die ("alpha + beta must sum to less than %.2f\n", min_alpha_beta_sum);
  }
  if (alphap + betap < min_alpha_beta_sum) {
    Die ("alpha' + beta' must sum to less than %.2f\n", min_alpha_beta_sum);
  }

  /**************************************************
   *    Get cm and consensus
   **************************************************/
  if (cmquery == 0) {
    cm = build_cm (queryfp, fullmat, &querylen, alpha, beta, alphap, betap,
		   beginsc, endsc);
    cons = CreateCMConsensus (cm, 0.0, 0.0);
    printf ("Query file: %s\n", queryfile);
  } else {
    querylen = 1;                 /* Dscale is actually D */
    cm = read_cm (queryfile);
    cons = CreateCMConsensus(cm, 3.0, 1.0);
    printf ("Query CM: %s\n", queryfile);
    printf ("ATTENTION: Using INFERNAL covariance file.  Though matrix, alpha,\n");
    printf ("beta, gap, and locality parameters are printed, they are not used in \n");
    printf ("practice.  Please ignore them.\n");
    printf ("D scale parameter is actually D -- query length set at 1\n\n");
  }
  printf ("Database file: %s\n\n", dbfile);

  printf ("\nbeginsc = %f\nendsc = %f\n", beginsc, endsc);

#ifdef USE_MPI
  }   /* End of first block that is only done by master process */
  /* Barrier for debugging */
  MPI_Barrier(MPI_COMM_WORLD);

  /* Here we need to broadcast the following parameters:
     num_samples, querylen, D_scale, cm */
  first_broadcast(&num_samples, &querylen, &D_scale, &cm, 
		  mpi_my_rank, mpi_master_rank);
#endif
  /**************************************************
   * Make the histogram
   *************************************************/

#ifdef USE_MPI
  if (mpi_my_rank == mpi_master_rank) {
#endif
  get_dbinfo (dbfp, &N, gc_count);
  if (do_complement) N*=2;
#ifdef USE_MPI
  }
#endif

  /* Set sample_length to 2*D if < 0 */
  if (sample_length < 0) {
    sample_length = 2*(int)(D_scale*querylen);
  }

  wtime(&start_clock);
  
  if (num_samples > 0) {
#ifdef USE_MPI
    if (mpi_num_procs > 1)
      parallel_make_histogram(gc_count, partitions, num_partitions,
			      cm, (int)(D_scale*querylen), 
			      num_samples, sample_length, lambda, K, 
			      mpi_my_rank, mpi_num_procs, mpi_master_rank);
    else 
#endif
      serial_make_histogram (gc_count, partitions, num_partitions,
			     cm, (int)(D_scale * querylen), 
			     num_samples, sample_length, lambda, K);
  } 

  wtime(&end_clock);
  dura_his = (float)(end_clock - start_clock);
  printf( "Making histogram cost %2.1f seconds\n", dura_his );

#ifdef USE_MPI
  if (mpi_my_rank == mpi_master_rank) {
#endif

  /* Set mu from K, lambda, N */
  if (num_samples > 0)
    for (i=0; i<GC_SEGMENTS; i++) {
      mu[i] = log(K[i]*N)/lambda[i];
    }

  if (cutoff_type == E_CUTOFF) {
    if (num_samples < 1) 
      Die ("Cannot use -E option without defined lambda and K\n");
  }

  if (num_samples > 0) {
    printf ("Statistics calculated with simulation of %d samples of length %d\n", num_samples, sample_length);
    if (num_partitions == 1) {
      printf ("No partition points\n");
    } else {
      printf ("Partition points are: ");
      for (i=1; i<GC_SEGMENTS; i++) {
	if (partitions[i] != partitions[i-1]) {
	  printf ("%d ", i);
	}
      }
      printf ("\n");
    }
    for (i=0; i<GC_SEGMENTS; i++) {
      printf ("GC = %d\tlambda = %.4f\tmu = %.4f\n", i, lambda[i], mu[i]);
    }
    printf ("N = %ld\n", N);
    if (cutoff_type == SCORE_CUTOFF) {
      printf ("Using score cutoff of %.2f\n", cutoff);
    } else {
      printf ("Using E cutoff of %.2f\n", cutoff);
    }
  } else {
    printf ("lambda and K undefined -- no statistics\n");
    printf ("Using score cutoff of %.2f\n", cutoff);
  }
  fflush(stdout);

  /*************************************************
   *    Do the search
   *************************************************/
#ifdef USE_MPI
  }                   /* Done with second master-only block */
  
  /* Now I need to broadcast the following parameters:
     cutoff, cutoff_type, do_complement, do_align, defined_mu, defined_lambda, 
     defined_K, mu, lambda, K, N */
  second_broadcast(&cutoff, &cutoff_type, &do_complement, &do_align, mu, lambda, K, &N, mpi_my_rank, mpi_master_rank);
#endif

#ifdef USE_MPI
    if (mpi_num_procs > 1)
      parallel_search_database (dbfp, cm, cons, (int)(D_scale*querylen), 
				cutoff, cutoff_type, do_complement, do_align,
				num_samples > 0, mu, lambda,
				mpi_my_rank, mpi_master_rank, mpi_num_procs);
    else
#endif
#ifdef _OPENMP
      my_search_database       (dbfp, cm, cons, (int)(D_scale*querylen), 
				cutoff, cutoff_type, do_complement, do_align,
				num_samples > 0, mu, lambda);
#else
      serial_search_database (dbfp, cm, cons, (int)(D_scale*querylen), 
			      cutoff, cutoff_type, do_complement, do_align,
			      num_samples > 0, mu, lambda);
#endif


  wtime(&end_clock);
  dura_fin = (float)(end_clock - start_clock);
  printf( "we cost %2.1f seconds totally, %2.1f for making histogram\n", dura_fin, dura_his );
  fflush(stdout);

  FreeCM(cm);

#ifdef USE_MPI
  MPI_Barrier(MPI_COMM_WORLD);
  MPI_Finalize();
  in_mpi = 0;
  if (mpi_my_rank == mpi_master_rank) {
#endif
    printf ("Fin\n");
    fflush(stdout);

    SeqfileClose(dbfp);
    SqdClean();
#ifdef USE_MPI
  }
#endif


  return EXIT_SUCCESS;
}
