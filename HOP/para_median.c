#include <stdio.h>
#include <math.h>
#include <omp.h> 
#include "para_kd.h"
#include <stdlib.h>

#define RANDOM_MULTIPLER 32
extern int nthreads;
extern int *local_num_p;
extern int *start_index;
extern float **samples_set;
extern float **median_seg;
extern float **local_median_seg;
extern int **global_acc_segments;
extern int **acc_segments;
extern int *size;
extern int *num_samples;
extern int *acc_num;
extern int *prev_num;
extern int *median_index_array;

int cmp_values(const float *a, const float *b)
{
    if ( *a<*b) return -1;
    else if (*a>*b) return 1;
    else return 0;
}


int bin_search(float  comparator,
               int    num,
               float *comp_base /* array of [num] */)
{
    int i, log2=0 , cur;
    int upper, lower;

    if (comparator <  comp_base[0])     return 0;
    if (comparator >= comp_base[num-1]) return num;

    cur   = num/2;
    upper = num;
    lower = 0;
    while ((num = num >> 1) > 0) log2++;

    for (i=0; i<log2+1; i++) {
        cur = (upper+lower)/2;
        if (comp_base[cur] <= comparator)
            lower = cur;
        else
            upper = cur;
    }

    return upper;
}


int median_main(KD             kd,
                int            pid,
                int            color,
		int            dim,
                int            group_size,
		float         *median_array)

{
    int num_groups,lb, ub,t, i, j, k, rand_index[RANDOM_MULTIPLER];

    int (*fcnt)(const void*, const void*);

    /* randomly pick RANDOM_MULTIPLER particle's indices */
    srand(1);

    rand_index[0]=(int)(((float)rand()/RAND_MAX)*local_num_p[pid] + start_index[pid]);

    rand_index[0]=5+start_index[pid];
    for (i=1;i<RANDOM_MULTIPLER; i++)
      rand_index[i]=(rand_index[i-1]+ local_num_p[pid]/RANDOM_MULTIPLER) % local_num_p[pid] + start_index[pid]; 

    num_groups = nthreads/group_size;
    if (pid==0) {
      samples_set = (float **)calloc(num_groups, sizeof(float *));
      num_samples = (int *) calloc(num_groups, sizeof(int));
      for (i=0; i<num_groups; i++) {
        num_samples[i] = group_size*RANDOM_MULTIPLER;
        samples_set[i] = (float *) calloc(num_samples[i], sizeof(float));
      }
    }
#pragma omp barrier

    for (j=0; j<RANDOM_MULTIPLER; j++) {
      samples_set[color][(pid%group_size)*RANDOM_MULTIPLER+j] = kd->p[rand_index[j]].r[dim];
    }
#pragma omp barrier

    if (pid==0) {
      fcnt = (int (*)(const void *, const void *))cmp_values; 
      for (i=0; i<num_groups; i++)
        qsort(samples_set[i], num_samples[i], sizeof(float), fcnt);

      acc_segments = (int **)calloc(nthreads, sizeof(int *));
      for (i=0; i<nthreads; i++)
        acc_segments[i]=(int *) calloc(num_samples[i/group_size]+1, sizeof(int));
      global_acc_segments = (int **) calloc(num_groups, sizeof(int *));
      for (i=0; i<num_groups; i++)
        global_acc_segments[i]=(int *)calloc(num_samples[i]+1, sizeof(int));
      
      size = (int *) calloc(num_groups, sizeof(int));
      for (i=0; i<nthreads; i++)
        size[i/group_size] +=local_num_p[i];
    }
#pragma omp barrier

    lb=start_index[pid];
    ub=lb+local_num_p[pid];
    
    for (i=lb; i<ub; i++)
      acc_segments[pid][bin_search(kd->p[i].r[dim], num_samples[color], samples_set[color])]++;

#pragma omp barrier

    if (pid==0) {
      for (i=0; i<nthreads; i++)
        for(j=0; j<num_samples[i/group_size]+1; j++)
          global_acc_segments[i/group_size][j] +=acc_segments[i][j];

      median_index_array = (int *)calloc(num_groups, sizeof(int));
      acc_num = (int *)calloc(num_groups, sizeof(int));
      prev_num = (int *)calloc(num_groups, sizeof(int));

      for (i=0; i<num_groups; i++) {
        acc_num[i]=global_acc_segments[i][0];
        median_index_array[i]=0;
        while (acc_num[i] <= size[i]/2){
          acc_num[i] +=global_acc_segments[i][++(median_index_array[i])];
        }
        prev_num[i]=acc_num[i] - global_acc_segments[i][median_index_array[i]];
      }

      median_seg = (float **) calloc(num_groups, sizeof(float *));
      for (i=0; i<num_groups; i++)
        median_seg[i] = (float *)calloc(global_acc_segments[i][median_index_array[i]], sizeof(float));
      local_median_seg = (float **)calloc(nthreads, sizeof(float *));
      for (i=0; i<nthreads; i++)
        local_median_seg[i] = (float *) calloc(acc_segments[i][median_index_array[i/group_size]], sizeof(float));
      
      free(acc_num);
      acc_num = (int *) calloc(nthreads, sizeof(int));
    }
  
#pragma omp barrier
    if (median_index_array[color]==0) {
      for (i=lb; i<ub; i++)
        if (kd->p[i].r[dim] < samples_set[color][median_index_array[color]])
          local_median_seg[pid][acc_num[pid]++] = kd->p[i].r[dim];
    }
    else if (median_index_array[color]==num_samples[color]) {
      for (i=lb; i<ub; i++)
        if (samples_set[color][median_index_array[color]-1] <= kd->p[i].r[dim])
          local_median_seg[pid][acc_num[pid]++] = kd->p[i].r[dim];
    }
    else {
      for (i=lb; i<ub; i++)
      if ((samples_set[color][median_index_array[color]-1] <= kd->p[i].r[dim]) && (kd->p[i].r[dim] < samples_set[color][median_index_array[color]]))
        local_median_seg[pid][acc_num[pid]++] = kd->p[i].r[dim];
    }
#pragma omp barrier
    
    if (pid==0) {
      for (i=0; i<num_groups; i++) {
        t=0;
        for (j=0; j<nthreads; j++) {
          if ((j/group_size) ==i)
            for (k=0; k<acc_num[j]; k++)
              median_seg[i][t++] = local_median_seg[j][k];
        }
      }
      for (i=0; i<num_groups; i++) {
        qsort(median_seg[i], global_acc_segments[i][median_index_array[i]], sizeof(float), fcnt);
        median_array[i] = median_seg[i][size[i]/2-prev_num[i]];
      }
       
      free(acc_num);
      free(prev_num);
      free(median_index_array);
      free(num_samples);
      free(size);
      for (i=0; i<num_groups; i++) {
        free(samples_set[i]);
        free(global_acc_segments[i]);
        free(median_seg[i]);
      }
      for (i=0; i<nthreads; i++){
        free(acc_segments[i]);
        free(local_median_seg[i]);
      }

      free(samples_set);
      free(global_acc_segments);
      free(median_seg);
      free(acc_segments);
      free(local_median_seg);
    }
#pragma omp barrier
}
       
