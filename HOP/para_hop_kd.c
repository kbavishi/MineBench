/* KD.C */
/* This was written by Joachim Stadel and the NASA HPCC ESS at
the University of Washington Department of Astronomy as part of
the SMOOTH program, v2.0.1.
URL: http://www-hpcc.astro.washington.edu/tools/SMOOTH */

/* DJE--I have removed all the subroutines not used by HOP, notably
the input and output routines. */

/* HOP Version 1.0 (12/15/97) -- Original Release */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <assert.h>
#include "para_kd.h"
#include <omp.h> 
/* #include "tipsydefs.h" */ /* Don't need this, since I removed kdReadTipsy()*/


#define MAX_ROOT_ITTR	32
#define COMM_NUM_STAGES 7
extern double comm_timing[COMM_NUM_STAGES];
extern double comp_timing[16];
extern int nthreads;
extern int *local_num_p;
extern int *start_index;

void kdTime(KD kd,int *puSecond,int *puMicro)
{
	struct rusage ru;

	getrusage(0,&ru);
	*puMicro = ru.ru_utime.tv_usec - kd->uMicro;
	*puSecond = ru.ru_utime.tv_sec - kd->uSecond;
	if (*puMicro < 0) {
		*puMicro += 1000000;
		*puSecond -= 1;
		}
	kd->uSecond = ru.ru_utime.tv_sec;
	kd->uMicro = ru.ru_utime.tv_usec;
	}


int kdInit(KD *pkd,int nBucket)
{
	KD kd;

	kd = (KD)malloc(sizeof(struct kdContext));
	assert(kd != NULL);
	kd->nBucket = nBucket;
	*pkd = kd;
	return(1);
	}

/*
 ** JST's Median Algorithm
 */
int kdMedianJst(KD kd,int d,int l,int u)
{
	float fm;
    int i,k,m;
    PARTICLE *p,t;

	p = kd->p;
    k = (l+u)/2;
	m = k;
    while (l < u) {
		m = (l+u)/2;
		fm = p[m].r[d];
		t = p[m];
		p[m] = p[u];
		p[u] = t;
		i = u-1;
		m = l;
		while (p[m].r[d] < fm) ++m;
		while (m < i) {
			while (p[i].r[d] >= fm) if (--i == m) break;
			/*
			 ** Swap
			 */
			t = p[m];
			p[m] = p[i];
			p[i] = t;
			--i;
			while (p[m].r[d] < fm) ++m;
			}
		t = p[m];
		p[m] = p[u];
		p[u] = t;
        if (k <= m) u = m-1;
        if (k >= m) l = m+1;
        }
    return(m);
    }


void kdCombine(KDN *p1,KDN *p2,KDN *pOut)
{
	int j;

	/*
	 ** Combine the bounds.
	 */
	for (j=0;j<3;++j) {
		if (p2->bnd.fMin[j] < p1->bnd.fMin[j])
			pOut->bnd.fMin[j] = p2->bnd.fMin[j];
		else
			pOut->bnd.fMin[j] = p1->bnd.fMin[j];
		if (p2->bnd.fMax[j] > p1->bnd.fMax[j])
			pOut->bnd.fMax[j] = p2->bnd.fMax[j];
		else
			pOut->bnd.fMax[j] = p1->bnd.fMax[j];
		}
	}


void kdUpPass(KD kd,int iCell, KDN *c)
{
        int l,u,pj,j;

        if (c[iCell].iDim != -1) {
                l = LOWER(iCell);
                u = UPPER(iCell);
                kdUpPass(kd,l,c);
                kdUpPass(kd,u,c);
                kdCombine(&c[l],&c[u],&c[iCell]);
                }
        else {
                l = c[iCell].pLower;
                u = c[iCell].pUpper;
                for (j=0;j<3;++j) {
                        c[iCell].bnd.fMin[j] = kd->p[u].r[j];
                        c[iCell].bnd.fMax[j] = kd->p[u].r[j];
                        }
                for (pj=l;pj<u;++pj) {
                        for (j=0;j<3;++j) {
                                if (kd->p[pj].r[j] < c[iCell].bnd.fMin[j])
                                        c[iCell].bnd.fMin[j] = kd->p[pj].r[j];
                                if (kd->p[pj].r[j] > c[iCell].bnd.fMax[j])
                                        c[iCell].bnd.fMax[j] = kd->p[pj].r[j];
                                }
                        }
                }
        }


int para_kdBuildTree(KD kd)
{
	int id, num_groups,pid,c_index,member_id, t,loops,l,k,n,i,d,m,j,ct, color,local_nNodes, local_index, group_size, group_id;
	KDN *c; 
        float *median_array; 
        PARTICLE **temp_left, **temp_right;
        int *left_count, *right_count;
        int *group_start_index, *num_particles, *total_left_count, *total_right_count,**left_index, **right_index;

        left_count = (int *) calloc(nthreads, sizeof(int));
        right_count = (int *) calloc(nthreads, sizeof(int));

        left_index = (int **) calloc(nthreads, sizeof(int *));
        right_index = (int **) calloc(nthreads, sizeof(int *));
        for (i=0; i<nthreads; i++) {
          left_index[i]=(int *) calloc(nthreads, sizeof(int));
          right_index[i]=(int *) calloc(nthreads, sizeof(int));
        }
 
	n = kd->nActive;
	kd->nLevels = 1;
	l = 1;
	while (n > kd->nBucket) {
        	n = n>>1;
		l = l<<1;
		++kd->nLevels;
		}
	kd->nSplit = l;
	kd->nNodes = l<<1;
        kd->kdNodes = (KDN *)malloc(kd->nNodes*sizeof(KDN));
        assert(kd->kdNodes != NULL);
	/*
	 ** Set up ROOT node
	 */
	c = kd->kdNodes;
        for (j=1; j<kd->nNodes; j++)
            c[j].fSplit=0;
        c[ROOT].pLower = 0;
        c[ROOT].pUpper = kd->nActive-1;

        group_start_index=(int *)calloc(nthreads, sizeof(int));
        num_particles = (int *)calloc(nthreads, sizeof(int));

        c[ROOT].bnd = kd->bnd;
       
        loops = 0;
        i = nthreads;
        while ((i = i >> 1) > 0) loops++;

        local_nNodes= (int)kd->nNodes/nthreads;
        median_array = (float *)calloc(nthreads, sizeof(float));
        temp_left = (PARTICLE **) malloc (nthreads* sizeof(PARTICLE *));
        temp_right = (PARTICLE **) malloc (nthreads* sizeof(PARTICLE *));
        
#pragma omp parallel private(i, j, group_size, color, num_groups, d, pid, t, m, member_id,ct, c_index) 
{
        pid = omp_get_thread_num();
/* start recursive bi-section here --------------------------------------*/
       
        for (i=0; i<loops ; i++)
        {
             int pow2_i = POW2(i);
             group_size = nthreads/pow2_i;
             num_groups = nthreads/group_size;
             color = pid /POW2(loops-i); 
             d = 0;

             for (j=1;j<3;++j) 
             {
               if (c[pow2_i+color].bnd.fMax[j]-c[pow2_i+color].bnd.fMin[j] >c[pow2_i+color].bnd.fMax[d]-c[pow2_i+color].bnd.fMin[d]) d = j; 
             }

             c[pow2_i+color].iDim = d;
            
             if (pid==0) {
               for (j=0; j<nthreads; j++)
                  num_particles[j/group_size] +=local_num_p[j];
               group_start_index[0] = 0;
               for (j=1; j<num_groups; j++)
                  group_start_index[j] +=group_start_index[j-1] + num_particles[j-1];
             }
#pragma omp barrier
             member_id=pid-color*group_size;
             if (member_id==0) {
                 c[pow2_i+color].pLower = group_start_index[color];
                 c[pow2_i+color].pUpper = group_start_index[color] + num_particles[color] - 1;
            }
#pragma omp barrier
             median_main(kd,pid,color, d, group_size,median_array); 
               
             left_count[pid]=0;
             right_count[pid]=0;
          
             temp_left[pid]=(PARTICLE *) malloc (local_num_p[pid] *sizeof(PARTICLE));
             temp_right[pid] = (PARTICLE *) malloc (local_num_p[pid] *sizeof(PARTICLE));
   
             for (t=start_index[pid]; t<start_index[pid]+local_num_p[pid];t++) {
                 if (kd->p[t].r[d]<=median_array[color])
                     temp_left[pid][left_count[pid]++]=kd->p[t];
                 else 
                    temp_right[pid][right_count[pid]++]=kd->p[t];
             }
#pragma omp barrier

             if (pid==0) {
               num_groups = nthreads/group_size;
               for (group_id =0; group_id<num_groups; group_id++) {
                 for (j=0; j<group_size; j++) {
                   if ((group_id==0)&&(j==0)) 
                     left_index[0][0]=0;
                   else if (j==0) 
                     left_index[group_id][0]=right_index[group_id-1][group_size-1]+right_count[group_id*group_size-1];
                   else
                     left_index[group_id][j]=left_index[group_id][j-1]+left_count[group_id*group_size+j-1];
                 }
             
                 right_index[group_id][0]=left_index[group_id][group_size-1]+left_count[(group_id+1)*group_size-1];
                 for (j=1; j<group_size; j++)
                   right_index[group_id][j]=right_index[group_id][j-1]+right_count[group_size*group_id+j-1];
               }
 
               total_left_count = (int *)calloc(num_groups, sizeof(int));
               total_right_count = (int *)calloc(num_groups, sizeof(int));
               
               for (group_id =0; group_id<num_groups; group_id++) {
                 for (j=0; j<group_size; j++) {  
                   total_left_count[group_id] += left_count[group_id*group_size+j];  
                   total_right_count[group_id] += right_count[group_id*group_size+j];           
                 }
               }

               for (group_id =0; group_id<num_groups; group_id++) 
                 for (j=0; j<group_size; j++) {  
                   id = group_id*group_size+j;
                   if (j==0)
                     start_index[id] = left_index[group_id][0];
                   else if (j<group_size/2)
                     start_index[id] = start_index[id-1] + total_left_count[group_id]*2/group_size;
                   else if (j==group_size/2)
                     start_index[id] = right_index[group_id][0];
                   else 
                     start_index[id] = start_index[id-1] + total_right_count[group_id]*2/group_size;
                 }
              
               for (j=0; j<nthreads-1; j++)
                 local_num_p[j] = start_index[j+1]-start_index[j];
               
               local_num_p[nthreads-1] = kd->nActive - start_index[nthreads-1];  
               for (j=0; j<nthreads; j++) {
                 num_particles[j]=0;
                 group_start_index[j]=0;
               }
             }
#pragma omp barrier       
             member_id=pid-color*group_size;

             j=left_index[color][member_id];
             for(t=0; t<left_count[pid]; t++) 
               kd->p[j++]=temp_left[pid][t];
             j=right_index[color][member_id];
             for(t=0; t<right_count[pid]; t++)
               kd->p[j++]=temp_right[pid][t];

             free(temp_left[pid]);
             free(temp_right[pid]);
#pragma omp barrier

             if (member_id==0) {    
               c[pow2_i+color].fSplit= median_array[color];
               c[LOWER(pow2_i+color)].bnd = c[pow2_i+color].bnd;
               c[LOWER(pow2_i+color)].bnd.fMax[d] = c[pow2_i+color].fSplit;
               c[UPPER(pow2_i+color)].bnd = c[pow2_i+color].bnd;
               c[UPPER(pow2_i+color)].bnd.fMin[d] = c[pow2_i+color].fSplit;
               
             }
#pragma omp barrier
        }
        /* calculate local KD tree ---------------------------------------------*/
       
        c_index = nthreads + pid;
 
        c[c_index].pLower=start_index[pid];
        c[c_index].pUpper=start_index[pid] + local_num_p[pid]-1;
        i=1;
        ct=1;
        SETNEXT(ct);
        while(1)
        {
           if (i<(int)(kd->nSplit/nthreads))
           {
               d=0;
               for (j=1;j<3;++j) 
               {
                  if (c[c_index].bnd.fMax[j]-c[c_index].bnd.fMin[j] >
                      c[c_index].bnd.fMax[d]-c[c_index].bnd.fMin[d]) d = j;
               }
               c[c_index].iDim = d;
               m = kdMedianJst(kd,d,c[c_index].pLower,c[c_index].pUpper);
               c[c_index].fSplit = kd->p[m].r[d]; 
               c[LOWER(c_index)].bnd = c[c_index].bnd;
	       c[LOWER(c_index)].bnd.fMax[d] = c[c_index].fSplit;
	       c[LOWER(c_index)].pLower = c[c_index].pLower;
               c[LOWER(c_index)].pUpper = m-1;
	       c[UPPER(c_index)].bnd = c[c_index].bnd;
	       c[UPPER(c_index)].bnd.fMin[d] = c[c_index].fSplit;
	       c[UPPER(c_index)].pLower = m;
	       c[UPPER(c_index)].pUpper = c[c_index].pUpper;
	       c_index = LOWER(c_index);
               i = LOWER(i);
	   }
           else 
           {
	       c[c_index].iDim = -1;
               SETNEXT(c_index);
	       SETNEXT(i);
	       if (i == ct) break;
	   }
	}
#pragma omp barrier
}/*end of paralleization */

        kdUpPass(kd,1,c);
        free(left_count);
        free(right_count);
        for (i=0; i<nthreads; i++) {
          free(left_index[i]);
          free(right_index[i]);
        }
        free(temp_left);
        free(temp_right);
        free(left_index);
        free(right_index);
	
        return(1);
}


int cmpParticles(const void *v1,const void *v2)
{
	PARTICLE *p1=(PARTICLE *)v1,*p2=(PARTICLE *)v2;
	
	return(p1->iOrder - p2->iOrder);
	}


void kdOrder(KD kd)
{
	qsort(kd->p,kd->nActive,sizeof(PARTICLE),cmpParticles);
	}

void kdFinish(KD kd)
{
	free(kd->p);
	free(kd->kdNodes);
	free(kd);
	}

