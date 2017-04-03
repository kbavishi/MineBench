#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#define MAIN_FILE
#include "pclass.h"

#define FILE_NAME_LEN 	256
#define LINE_LEN	2048

#define itag		1

int VRCompare(const void *,const void *);
struct timeval tp;
  
int main(int argc, char *argv[])
{
  FILE *fp_data;
  char datafile[FILE_NAME_LEN], file_name[FILE_NAME_LEN];
  int i,j,k;
//  int elemspp;
//  int *inp_array;
  int (*fcnt)(const void*, const void*);
  int mm,nn,nthreads, pid, lb, ub, local_natr;
  double ts,te, io_t_1, io_t_2, *io_time;

  if(argc < 6) {
      printf(" Usage: %s <datafile> <#recoreds> <#attributes> <#classes> <#threads>\n", argv[0]);
      printf("   <datafile>, is used to read data file\n");
      printf("   <#records>, number of records\n");
      printf("   <#attributes>, number of attributes\n");
      printf("   <#classes>, number of classes\n");
      printf("   <#threads>, numner of threads\n");
  }

  strcpy(datafile,argv[1]);
  nrec = atoi(argv[2]);
  natr = atoi(argv[3]);
  nclass = atoi(argv[4]);
  nthreads = atoi(argv[5]);

  seconds(ts); 
  catr = (Continuous *)calloc(natr,sizeof(Continuous));

  for(i=0;i<natr;i++) {
    if(!(catr[i].valsrids = (VR *)calloc(nrec, sizeof(VR)))) {
      printf("Memory Crunch @ catr[%d].valsrids\n",i);
    }
  }

  fcnt = (int (*)(const void *, const void *))VRCompare;
  strcat(datafile, ".att");

  io_time = (double *)malloc(sizeof(double)*nthreads);
  omp_set_num_threads(nthreads);
#pragma omp parallel private(io_t_1, io_t_2,mm, nn, fp_data,file_name,pid, local_natr, lb, ub,i,j)
{
  pid = omp_get_thread_num();
  local_natr = (natr + nthreads - 1)/nthreads;
  lb = pid*local_natr;
  ub = min((pid+1)*local_natr, natr);

  seconds(io_t_1);
  for(i=lb;i<ub;i++) {
    sprintf(file_name, "%s.%d", datafile, i);
    fp_data = fopen(file_name, "r");
    for (j=0; j<nrec; j++) {
      fscanf(fp_data, "%d\n", &mm);
      fscanf(fp_data, "%d\n", &nn);
      catr[i].valsrids[j].val=mm;
      catr[i].valsrids[j].cid = nn;
      catr[i].valsrids[j].rid = j;
    }
    fclose(fp_data);
  }

  seconds(io_t_2);
  for(i=lb;i<ub;i++) 
    qsort(catr[i].valsrids,nrec,sizeof(VR),VRCompare);

  io_time[pid] = io_t_2 - io_t_1;
  
}

  printf("Starting Classify\n");
  
  ParClassify(nthreads);
  seconds(te);
 
  double max;
  max=io_time[0];
  for (i=1; i<nthreads; i++) 
    if (io_time[i]>max)
      max=io_time[i]; 
  printf("Total execution time with %d processoers: %f\n",nthreads, te-ts); 
  printf("I/O time = %f\n", max);
  printf("Computation time =%f\n", te-ts - max);
}
