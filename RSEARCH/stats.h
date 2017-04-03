/*
 * stats.h
 * 
 * Header file for stats.c
 */

#ifndef _stats_h
#define _stats_h

void serial_make_histogram (int *gc_count, int *partitions, int num_partitions,                            CM_t *cm, int D, int num_samples, 
			    int sample_length, float *lambda, float *K);

#ifdef USE_MPI
void parallel_make_histogram (int *gc_count, int *partitions, int num_partitions,
			      CM_t *cm, int D, int num_samples,
			      int sample_length, float *lambda, float *K, 
			      int mpi_my_rank, int num_procs, 
			      int mpi_master_rank);
#endif

void get_chunk_distribution(db_seq_t * cur_seq, int D, int num_procs, int do_complement, int* chunk_size, int *chunk_offset, int * n_procs);
int get_my_chunk(db_seq_t * cur_seq, int thread_id, int num_procs, int chunksize, int chunkoffset, int *i, int *j, int * incomplement, int *start_proc);
void CombineResults(scan_results_t **results, scan_results_t **results_array, int n_procs, int do_complement);

void get_dbinfo (SQFILE *dbfp, long *N, int *gc_count);

float e_to_score (float E, float *mu, float *lambda);

double RJK_ExtremeValueE (float x, float mu, float lambda);

char resolve_degenerate (char c);


#endif
