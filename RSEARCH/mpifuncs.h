/*
 * mpifuncs.h
 *
 * Basic functions for using MPI in rsearch 
 *
 * Robert J. Klein
 * May 28, 2002
 */

#ifndef _MPIFUNCS_H
#define _MPIFUNCS_H

#ifdef USE_MPI

#include "mpi.h"
#include "structs.h"

/* Work types */
#define TERMINATE_WORK 0
#define STD_SCAN_WORK 1
#define ALIGN_WORK 2
#define HIST_SCAN_WORK 3

/* Results types */
#define STD_SCAN_RESULTS 0
#define ALIGN_RESULTS 1
#define HIST_SCAN_RESULTS 3

/* Communication tags */
#define JOB_PACKET_TAG 1
#define SEQ_TAG 2
#define STD_SCAN_RESULTS_SIZE_TAG 3
#define STD_SCAN_RESULTS_TAG 4
#define HIST_RESULTS_TAG 5

typedef struct _job_t {
  char job_type;
  int seqlen;
  char *dsq;
  int db_seq_index;
  char in_complement;
  int index;            /* result index for alignment, seq index for scan */
  int bestr;            /* Best root state -- only used for alignment jobs,
			   0 for scanning jobs */
  struct _job_t *next;
} job_t;

/* Get rank of master process (lowest ranked one that can do I/O */
/* Also checks the version string */
int get_master_rank (MPI_Comm comm, int mpi_my_rank);

/* First broadcast of information */
void first_broadcast (int *num_samples, int *querylen, float *D_scale,
		      CM_t **cm, int mpi_my_rank, int mpi_master_rank);

/* Second broadcast of information */
void second_broadcast (float *cutoff, int *cutoff_type, int *do_complement, 
		       int *do_align,
		       float *mu, float *lambda, float *K, long *N,
		       int mpi_my_rank, int mpi_master_rank);

/* Get job from master process */
char recieve_job (int *seqlen_p, char **seq_p, int *bestr_p, int mpi_master_rank);

/* Send results of a scan (scan_results_t) */
void send_scan_results (scan_results_t *results, int mpi_master_node);

/* Send results of an alignment (Parsetree_t *) */
void send_align_results (Parsetree_t *tr, int mpi_master_node);

/* TRUE if a slave is still working, FALSE if all done */
int procs_working (job_t **process_status, int mpi_num_procs, int mpi_master_rank);

/* Chunk the seq */
job_t *enqueue (db_seq_t *active_seq, int db_seq_index, int D,
		int do_complement, int job_type);

/* Put alignments into the queue */
void enqueue_alignments (job_t **queue, db_seq_t *active_seq, int db_seq_index,
			 int do_complement, int job_type);

/* Send the next job from the queue */
void send_next_job (job_t **queue, job_t **process_status, int rank_to_send_to); 

/* Do a blocking call to MPI_Recv until a process finishes, then process results */
int check_results (db_seq_t **active_seqs, job_t **process_status, int D);

/* Send the termination code to rank i */
void send_terminate (int i);

/* Check for results from a histogram scan */
int check_hist_results (db_seq_t **seq, job_t **process_status, int D);

/* Send histogram scan results */
void send_hist_scan_results (float score, int mpi_master_node);

#endif

#endif
