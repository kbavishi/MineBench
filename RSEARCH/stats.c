/*
 * stats.c
 *
 * Routines for calculating statistics in rsearch.  
 *
 * Robert J. Klein
 * Separate file started May 17, 2002
 */

#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <ctype.h>
#include <string.h>

#include "structs.h"		/* data structures, macros, #define's   */
#include "funcs.h"		/* external functions                   */
#include "squid.h"		/* general sequence analysis library    */
#include "msa.h"                /* squid's multiple alignment i/o       */
#include "histogram.h"          /* EVD histogram routines from HMMER    */
#include "stats.h"
#include "mpifuncs.h"

#ifdef _OPENMP
#include "omp.h"
#endif
#define MIN_CHUNK_D_MULTIPLIER 10
#define MAX_CHUNK_SIZE 1000000

/*
 * Function: serial_make_histogram()
 * Date:     Mon Apr 1 2002 [St. Louis]
 * Purpose:  Makes a histogram using random sequences.  Returns mu and lambda.
 *           Makes random sequences of length dblen, finds best hit at 
 *           arbitrary j's every D nucleotides along database.
 *           Also returns K (how much to scale N in calculating E-value)
 *
 * Inputs:   gc_comp     %GC of random seq
 *           cm          the model
 *           D           maximum value for D
 *           num_samples number of samples to take
 *           sample_length  length of each sample
 *           mu_p        pointer to final mu
 *           lambda_p    pointer to final lambda
 */  
#ifdef _OPENMP
  char *randseq, *dsq;
  float *nt_p;                /* Distribution for random sequences */
#pragma omp threadprivate(randseq, dsq, nt_p)
#endif


void serial_make_histogram (int *gc_count, int *partitions, int num_partitions,                            CM_t *cm, int D, int num_samples,
			    int sample_length, float *lambda, float *K) {
  int i;
#ifndef _OPENMP
  char *randseq;
  char *dsq;
  float *nt_p;                /* Distribution for random sequences */
#endif
  struct histogram_s *h;
  float score;
  int cur_partition;
  float cur_gc_freq[GC_SEGMENTS];
  float gc_comp;

#ifdef _OPENMP
  float *nt_p_buffer;
  char  *randseq_buffer;
  char  *dsp_buffer;
  int  thread_nums, thread_id;
#endif

  if (num_samples == 0) {
    for (i=0; i<GC_SEGMENTS; i++) {
      K[i] = 0.;
      lambda[i] = 0.;
    }
    return;        /* No sequence to analyse */
  }

#ifdef _OPENMP
  thread_nums = omp_get_max_threads();
  nt_p_buffer = MallocOrDie(sizeof(float)*Alphabet_size*thread_nums);
  randseq_buffer = (char *) MallocOrDie (sizeof(char) * (sample_length+1)*thread_nums);
  dsp_buffer = MallocOrDie (sizeof(char) * (sample_length+2)*thread_nums);
#pragma omp parallel private(thread_id)
{
   thread_id    = omp_get_thread_num();
   nt_p    =    nt_p_buffer + thread_id * Alphabet_size;
   randseq = randseq_buffer + thread_id * (sample_length+1);
   dsq     =     dsp_buffer + thread_id * (sample_length+2);
}
#else
  /* Allocate for random distribution */
  nt_p = MallocOrDie(sizeof(float)*Alphabet_size); 
#endif

  /* For each partition */
  for (cur_partition = 0; cur_partition < num_partitions; cur_partition++) {
   
    /* Initialize histogram; these numbers are guesses */
    h = AllocHistogram (0, 100, 100);

    /* Set up cur_gc_freq */
    for (i=0; i<GC_SEGMENTS; i++) {
      if (partitions[i] == cur_partition) {
	cur_gc_freq[i] = (float)gc_count[i];
      } else {
	cur_gc_freq[i] = 0.;
      }
    }
    FNorm(cur_gc_freq, GC_SEGMENTS);

    /* Take num_samples samples */
#pragma omp parallel for private(i, gc_comp)
    for (i=0; i<num_samples; i++) {

      /* Get random GC content */
      gc_comp = 0.01*FChoose (cur_gc_freq, GC_SEGMENTS);
      nt_p[1] = nt_p[2] = 0.5*gc_comp;
      nt_p[0] = nt_p[3] = 0.5*(1. - gc_comp);

      /* Get random sequence */
#ifdef _OPENMP
      Random_Sequence (Alphabet, nt_p, Alphabet_size, sample_length, randseq);
#else
      randseq = RandomSequence (Alphabet, nt_p, Alphabet_size, sample_length);
#endif
    
      /* Digitize the sequence, parse it, and add to histogram */
#ifdef _OPENMP
      Digitize_Sequence (randseq, sample_length, dsq);
#else
      dsq = DigitizeSequence (randseq, sample_length);
#endif

      /* Do the scan */
      score = CYKScan (cm, dsq, sample_length, IMPOSSIBLE, D, NULL);

      /* Add best score to histogram */
#pragma omp critical(ADD_TO_HISTOGRAM)
      AddToHistogram (h, score);

      /* Free stuff */
#ifndef _OPENMP
      free (randseq);
      free (dsq);
#endif
    }

    /* Fit the histogram.  */
    ExtremeValueFitHistogram (h, TRUE, 9999);
    for (i=0; i<GC_SEGMENTS; i++) {
      if (partitions[i] == cur_partition) {
	lambda[i] = h->param[EVD_LAMBDA];
	K[i] = exp(h->param[EVD_MU]*h->param[EVD_LAMBDA])/sample_length;
      }
    }

    FreeHistogram(h);
  }
#ifdef _OPENMP
  free(nt_p_buffer);
  free(randseq_buffer);
  free(dsp_buffer);
#else
  free(nt_p);
#endif
}

#ifdef USE_MPI
void parallel_make_histogram (int *gc_count, int *partitions, int num_partitions, 
			      CM_t *cm, int D, int num_samples,
			      int sample_length,float *lambda, float *K, 
			      int mpi_my_rank, int mpi_num_procs, 
			      int mpi_master_rank) {
  struct histogram_s **h;
  db_seq_t **randseqs;          /* The random sequences */
  int randseq_index;
  int num_seqs_made;
  float *nt_p;                  /* Distribution for random sequences */
  int proc_index;
  job_t **process_status;
  job_t *job_queue = NULL;
  char *seq;
  int seqlen;
  char job_type;
  float score;
  int dummy;              /* To hold bestr, which isn't used in these jobs */
  int i = 0;
  int cur_partition;
  float **cur_gc_freqs;
  float gc_comp;

  if (num_samples == 0) {
    for (i = 0; i<GC_SEGMENTS; i++) {
      K[i] = 0.;
      lambda[i] = 0.;
    }
  }

  if (mpi_my_rank == mpi_master_rank) {

    /* Allocate random distribution */
    nt_p = MallocOrDie(sizeof(float)*Alphabet_size); 

    /* Allocate histograms and set up cur_gc_freq */
    h = MallocOrDie(sizeof (struct histogram_s *)*num_partitions);
    cur_gc_freqs = MallocOrDie(sizeof(float *)*num_partitions);
    for (cur_partition = 0; cur_partition<num_partitions; cur_partition++) {
      /* Initialize histogram; these numbers are guesses */
      h[cur_partition] = AllocHistogram (0, 100, 100);
 
      /* Set up cur_gc_freq */
      cur_gc_freqs[cur_partition] = MallocOrDie(sizeof(float)*GC_SEGMENTS);
      for (i=0; i<GC_SEGMENTS; i++) {
	if (partitions[i] == cur_partition) {
	  cur_gc_freqs[cur_partition][i] = (float)gc_count[i];
	} else {
	  cur_gc_freqs[cur_partition][i] = 0.;
	}
      }
      FNorm (cur_gc_freqs[cur_partition], GC_SEGMENTS);
    }

    /* Set up arrays to hold pointers to active seqs and jobs on
       processes */
    randseqs = MallocOrDie(sizeof(db_seq_t *) * mpi_num_procs);
    process_status = MallocOrDie(sizeof(job_t *)*mpi_num_procs);
    for (randseq_index=0; randseq_index<mpi_num_procs; randseq_index++)
      randseqs[randseq_index] = NULL;
    for (proc_index = 0; proc_index < mpi_num_procs; proc_index++)
      process_status[proc_index] = NULL;
   
    num_seqs_made = 0;
    cur_partition = 0;

    do {
      /* Check for idle processes.  Send jobs */
      for (proc_index=0; proc_index<mpi_num_procs; proc_index++) {
	if (proc_index == mpi_master_rank) continue; /* Skip master process */
	if (process_status[proc_index] == NULL) {         
	  /* I'm idle -- need a job */
	  if (job_queue == NULL) {
	    for (randseq_index = 0; randseq_index <mpi_num_procs; 
		 randseq_index++) {
	      if (randseqs[randseq_index] == NULL) break;
	    }
	    if (randseq_index == mpi_num_procs)
	      Die ("Tried to read more than %d seqs at once\n", mpi_num_procs);
	    /* Get random sequence, digitize it */
	    if (num_seqs_made == num_samples * num_partitions)
	      /* We've got all we need.  Stop */
	      break;
	    cur_partition = num_seqs_made / num_samples;
	    randseqs[randseq_index] = MallocOrDie(sizeof(db_seq_t));
	    randseqs[randseq_index]->sqinfo.len = sample_length;
	    /* Get random GC content */
	    gc_comp = 0.01*FChoose(cur_gc_freqs[cur_partition], GC_SEGMENTS);
	    nt_p[1] = nt_p[2] = 0.5*gc_comp;
	    nt_p[0] = nt_p[3] = 0.5*(1. - gc_comp);
	    randseqs[randseq_index]->seq[0] = 
	      RandomSequence (Alphabet, nt_p, Alphabet_size, 
			      randseqs[randseq_index]->sqinfo.len);
	    randseqs[randseq_index]->dsq[0] = 
	      DigitizeSequence (randseqs[randseq_index]->seq[0], 
				randseqs[randseq_index]->sqinfo.len);
	    randseqs[randseq_index]->best_score = 0;
	    randseqs[randseq_index]->partition = cur_partition;
	    job_queue = enqueue (randseqs[randseq_index], randseq_index, D,
				 FALSE, HIST_SCAN_WORK);
	    num_seqs_made++;
	  }
	  if (job_queue != NULL)
	    send_next_job (&job_queue, process_status + proc_index, 
			   proc_index);
	} 
      } 
      /* Wait for next reply */
      if (procs_working(process_status, mpi_num_procs, mpi_master_rank)) {
	randseq_index = check_hist_results (randseqs, process_status, D);
	/* If the sequence is done */
	if (randseqs[randseq_index]->chunks_sent == 0) {
	  /* Get best score at D and add */
	  AddToHistogram (h[randseqs[randseq_index]->partition], 
			  randseqs[randseq_index]->best_score);
	  free(randseqs[randseq_index]->dsq[0]);
	  free(randseqs[randseq_index]->seq[0]);
	  free(randseqs[randseq_index]);
	  randseqs[randseq_index] = NULL;
	}
      }
    } while (num_seqs_made < num_samples*num_partitions || job_queue != NULL ||
	     procs_working(process_status, mpi_num_procs, mpi_master_rank));
      
    /* Terminate the processes */
    for (proc_index=0; proc_index<mpi_num_procs; proc_index++) {
      if (proc_index != mpi_master_rank) {
	send_terminate (proc_index);
      }
    }
    
    /* Fit the histogram.  */
    for (cur_partition=0; cur_partition<num_partitions; cur_partition++) {
      ExtremeValueFitHistogram (h[cur_partition], TRUE, 9999);
      for (i=0; i<GC_SEGMENTS; i++) {
	if (partitions[i] == cur_partition) {
	  lambda[i] = h[cur_partition]->param[EVD_LAMBDA];
	  K[i] = exp(h[cur_partition]->param[EVD_MU]*h[cur_partition]->param[EVD_LAMBDA])/sample_length;
	}
      }
    }
    free(randseqs);
    free(process_status);
    free(nt_p);
    for (i=0; i<cur_partition; i++) {
      FreeHistogram(h[i]);
      free(cur_gc_freqs[i]);
    }
    free(cur_gc_freqs);
    free(h);
  } else {
    seq = NULL;
    do {
      job_type = recieve_job (&seqlen, &seq, &dummy, mpi_master_rank);
      if (job_type == HIST_SCAN_WORK) {
	score = CYKScan (cm, seq, seqlen, IMPOSSIBLE, D, NULL);
	send_hist_scan_results (score, mpi_master_rank);
      } 
      if (seq != NULL)
	free(seq);
      seq = NULL;
    } while (job_type != TERMINATE_WORK);
  }
  MPI_Barrier(MPI_COMM_WORLD);
}
#endif

/*
 * Function: random_from_string
 * Date:     September, 1998 (approx.) -- from hmmgcc
 * This function returns a character randomly chosen from the string.
 * Used in conjunction with the function below that resolves degenerate code
 * nucleotides.
 */
char random_from_string (char *s) {
  int i;
  do {
    i = (int) ((float)(strlen(s)-1)*sre_random()/(RAND_MAX+1.0));
  } while (i<0 || i>=strlen(s));
  return(s[i]);
}

/*
 * Function: resolve_degenerate
 * Date:     September, 1998 (from hmmgcc)
 * This function resolves "degnerate" nucleotides by selecting a random 
 * A, C, G, or T as appropriate by the code present there.  Returns
 * the character passed in if that character does not represent a
 * non-degnerate nucleotide (either A, C, G, or T or not representative
 * at all of a nucleotide.
 *
 * The degenerate code used here is:
 * (taken from http://www.neb.com/neb/products/REs/RE_code.html
 *
 *                         R = G or A
 *                         K = G or T
 *                         B = not A (C or G or T)
 *                         V = not T (A or C or G)
 *                         Y = C or T
 *                         S = G or C
 *                         D = not C (A or G or T)
 *                         N = A or C or G or T
 *                         M = A or C
 *                         W = A or T
 *                         H = not G (A or C or T)
 *
 * This function assumes all letters are already uppercased via toupper
 * before calling.  In other words, it will return a "n" if passed an "n"
 * because it will assume that the symbol for all nucleotides will be passed
 * in as "N".
 */
char resolve_degenerate (char c) {
  c = toupper(c);
  switch (c) {
    case 'R' : return(random_from_string("GA"));
    case 'K' : return(random_from_string("GT"));
    case 'B' : return(random_from_string("CGT"));
    case 'V' : return(random_from_string("ACG"));
    case 'Y' : return(random_from_string("CT"));
    case 'S' : return(random_from_string("GC"));
    case 'D' : return(random_from_string("AGT"));
    case 'N' : return(random_from_string("ACGT"));
    case 'M' : return(random_from_string("AC"));
    case 'W' : return(random_from_string("AT"));
    case 'H' : return(random_from_string("ACT"));
  }
  return(c);
}

/*
 * Function: get_dbinfo()
 * Date:     RJK, Thu Apr 11, 2002 [St. Louis]
 * Purpose:  Given a pointer to an open but as yet unread sequence database 
 *           file, place to put the size, partition info, and place to put
 *           GC content, calculates total size of database and GC info 
 */
void get_dbinfo (SQFILE *dbfp, long *N, int *gc_count) {
  long counter = 0;
  char *seq;
  char c;
  int i,j;
  int gc;
  SQINFO sqinfo;

  for (i=0; i<GC_SEGMENTS; i++)
    gc_count[i] = 0;

  while (ReadSeq (dbfp, dbfp->format, &seq, &sqinfo)) {
    counter += sqinfo.len;
    for (i = 0; i<sqinfo.len; i+=100) {
      gc = 0;
      for (j=0; j<100; j++) {
	c = resolve_degenerate(seq[i+j]);
	if (c == 'G' || c == 'C')
	  gc++;
      }
      gc_count[gc]++;
    }
    FreeSequence (seq, &sqinfo);
  }
#ifdef PRINT_GC_COUNTS
  for (i=0; i<GC_SEGMENTS; i++) 
    printf ("%d\t%d\n", i, gc_count[i]);
#endif
  *N = counter;
  SeqfileRewind(dbfp);
}

/*
 * Function: e_to_score()
 * Date:     RJK, Mon April 15 2002 [St. Louis]
 * Purpose:  Given an E-value and mu, lambda, and N, returns a value for S
 *           that will give such an E-value.  Basically the inverse of 
 *           ExtremeValueE
 */
float e_to_score (float E, float *mu, float *lambda) {
  float lowest_score, result;
  int i;

  lowest_score = mu[0] - (log(E)/lambda[0]);
  for (i=1; i<GC_SEGMENTS; i++) {
    result = mu[i] - (log(E)/lambda[i]);
    if (result < lowest_score)
      lowest_score = result;
  }
  return (lowest_score);
}


/*
 * Function: RJK_ExtremeValueE
 * Date:     RJK, Mon Sep 30, 2002 [St. Louis]
 * Purpose:  Given a score (x), mu, and lambda, calculates 
 *           E=exp(-1*lambda(x-mu)) using first part of code from Sean's
 *           ExtremeValueP
 */
double RJK_ExtremeValueE (float x, float mu, float lambda) {
                        /* avoid underflow fp exceptions near P=0.0*/
  if ((lambda * (x - mu)) >= 2.3 * (double) DBL_MAX_10_EXP) {
    return 0.0;
  } else {
    return(exp(-1. * lambda * (x - mu)));
  }
}

void get_chunk_distribution(db_seq_t * cur_seq, int D, int num_procs, int do_complement, int* chunk_size, int *chunk_offset, int * n_procs)
{
	int chunksize;
	int chunkoffset;
	int nprocs;

	chunksize = ((cur_seq->sqinfo.len+D*(num_procs-1))/(num_procs))+1;
	chunksize = ((chunksize/D)+1)*D;
	if (do_complement)
	chunksize *= 2;
	if (chunksize < MIN_CHUNK_D_MULTIPLIER * D)
		chunksize = MIN_CHUNK_D_MULTIPLIER * D;
	if (chunksize > MAX_CHUNK_SIZE)
		chunksize = MAX_CHUNK_SIZE;
		
    *chunk_size   = chunksize;
	chunkoffset   = chunksize - D;
	*chunk_offset = chunkoffset;
	nprocs = cur_seq->sqinfo.len / chunkoffset;
	if(cur_seq->sqinfo.len % chunkoffset != 0)
		nprocs += 1;
	if(do_complement)
		nprocs *= 2;
		
	if(nprocs > num_procs)
		nprocs = num_procs;
	*n_procs = nprocs;
}

int get_my_chunk(db_seq_t * cur_seq, int thread_id, int num_procs, int chunksize, int chunkoffset, int *i, int *j, int * incomplement, int *start_proc)
{
	int curpos;	
	int k, kstop;

	if(*j==0)	//first time to deal with this sequence
	{
		curpos = 1;
		*incomplement = 0;
		
		if(thread_id < *start_proc)
			kstop = thread_id + num_procs;
		else
			kstop = thread_id;	
		
			
		for(k= *start_proc; k < kstop; k++)
		{
			curpos += chunkoffset;
			if(curpos > cur_seq->sqinfo.len)
			{
				if( *incomplement==0 )	//move to the inversed sequence
				{
					*incomplement = 1;					
					curpos = 1;					
				}else			// no chunk to get
				{
					*start_proc = (k+1)%num_procs;
					(*incomplement)=0;					
					return 0;
				}								
			}
		}
		
		*i = curpos;
		*j = curpos+chunksize-1;
		if(*j > cur_seq->sqinfo.len)
			*j = cur_seq->sqinfo.len;
				
	}else
	{
		curpos = *i;
		for(k = thread_id; k < thread_id + num_procs; k++)
		{
			curpos += chunkoffset;
			if(curpos > cur_seq->sqinfo.len)
			{
				if( *incomplement==0 )	//move to the inversed sequence
				{
					*incomplement = 1;
					curpos = 1;
				}else			// no chunk to get
				{
					*start_proc = (k+1) % num_procs;
					*incomplement=0;
					return 0;
				}								
			}
		}
		*i = curpos;
		*j = curpos+chunksize-1;
		if(*j > cur_seq->sqinfo.len)
			*j = cur_seq->sqinfo.len;
	}

	return 1;
}

void CombineResults(scan_results_t **results, scan_results_t **results_array, int n_procs, int do_complement)
{
  int i, j, k;
  for (i = 0; i < do_complement; i ++)
  {
     results[i] = results_array[i];
	 for(j = 1; j < n_procs; j ++)
	 {
	   scan_results_t * tmp_result  = results_array[2 * j + i];
       int num_results =  tmp_result->num_results;
	   for (k = 0; k < num_results; k ++)
	   {
        report_hit(tmp_result->data[k].start, tmp_result->data[k].stop, tmp_result->data[k].bestr,
			       tmp_result->data[k].score, results[i]);
		results[i]->data[results[i]->num_results-1].tr = tmp_result->data[k].tr;
	   }
	 }
  }
}
