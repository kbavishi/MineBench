/*************************************************************************/
/**   File:         kmeans_clustering.c                                 **/
/**   Description:  Implementation of regular k-means clustering        **/
/**                 algorithm                                           **/
/**   Author:  Wei-keng Liao                                            **/
/**            ECE Department, Northwestern University                  **/
/**            email: wkliao@ece.northwestern.edu                       **/
/**                                                                     **/
/**   Edited by: Jay Pisharath                                          **/
/**              Northwestern University.                               **/
/**                                                                     **/
/*************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <math.h>
#include "fuzzy_kmeans.h"
#include <omp.h>

#define RANDOM_MAX 2147483647

#ifndef FLT_MAX
#define FLT_MAX 3.40282347e+38
#endif

extern double wtime(void);

/*----< euclid_dist_2() >----------------------------------------------------*/
/* multi-dimensional spatial Euclid distance square */
__inline
float euclid_dist_2(float *pt1,
                    float *pt2,
                    int    numdims)
{
    int i;
    float ans=0.0;

    for (i=0; i<numdims; i++)
        ans += (pt1[i]-pt2[i]) * (pt1[i]-pt2[i]);

    return(ans);
}


/*----< kmeans_clustering() >---------------------------------------------*/
float** kmeans_clustering(int     is_perform_atomic, /* in: */
                          float **feature,    /* in: [npoints][nfeatures] */
                          int     nfeatures,
                          int     npoints,
                          int     nclusters,
                          float   threshold,
                          int    *membership) /* out: [npoints] */
{

    int      i, j, k, index, loop=0;
    int     *new_centers_len; /* [nclusters]: no. of points in each cluster */
    float    delta;
    float  **clusters;   /* out: [nclusters][nfeatures] */
    float  **new_centers;     /* [nclusters][nfeatures] */
    double   timing;

    int      nthreads;
    int    **partial_new_centers_len;
    float ***partial_new_centers;

    nthreads = omp_get_max_threads();

    /* allocate space for returning variable clusters[] */
    clusters    = (float**) malloc(nclusters *             sizeof(float*));
    clusters[0] = (float*)  malloc(nclusters * nfeatures * sizeof(float));
    for (i=1; i<nclusters; i++)
        clusters[i] = clusters[i-1] + nfeatures;

    /* randomly pick cluster centers */
    for (i=0; i<nclusters; i++) {
        int n = (int)random() % npoints;
        for (j=0; j<nfeatures; j++)
            clusters[i][j] = feature[n][j];
    }

    for (i=0; i<npoints; i++)
	membership[i] = -1;

    /* need to initialize new_centers_len and new_centers[0] to all 0 */
    new_centers_len = (int*) calloc(nclusters, sizeof(int));

    new_centers    = (float**) malloc(nclusters *            sizeof(float*));
    new_centers[0] = (float*)  calloc(nclusters * nfeatures, sizeof(float));
    for (i=1; i<nclusters; i++)
        new_centers[i] = new_centers[i-1] + nfeatures;

    if (!is_perform_atomic) {
        partial_new_centers_len    = (int**) malloc(nthreads *
	                                            sizeof(int*));
        partial_new_centers_len[0] = (int*)  calloc(nthreads*nclusters,
	                                            sizeof(int));
        for (i=1; i<nthreads; i++)
            partial_new_centers_len[i] = partial_new_centers_len[i-1]+nclusters;

        partial_new_centers    =(float***)malloc(nthreads *
	                                         sizeof(float**));
        partial_new_centers[0] =(float**) malloc(nthreads*nclusters *
	                                         sizeof(float*));
        for (i=1; i<nthreads; i++)
            partial_new_centers[i] = partial_new_centers[i-1] + nclusters;
        for (i=0; i<nthreads; i++)
            for (j=0; j<nclusters; j++)
                partial_new_centers[i][j] = (float*)calloc(nfeatures,
		                                           sizeof(float));
    }

    if (_debug) timing = omp_get_wtime();
    do {
        delta = 0.0;

        if (is_perform_atomic) {
            #pragma omp parallel for \
                    private(i,j,index) \
                    firstprivate(npoints,nclusters,nfeatures) \
                    shared(feature,clusters,membership,new_centers,new_centers_len) \
                    schedule(static) \
                    reduction(+:delta)
            for (i=0; i<npoints; i++) {
	        /* find the index of nestest cluster centers */
	        index = find_nearest_point(feature[i],
				           nfeatures,
				           clusters,
				           nclusters);
	        /* if membership changes, increase delta by 1 */
	        if (membership[i] != index) delta += 1.0;

	        /* assign the membership to object i */
	        membership[i] = index;

	        /* update new cluster centers : sum of objects located within */
                #pragma omp atomic
	        new_centers_len[index]++;
	        for (j=0; j<nfeatures; j++)
                    #pragma omp atomic
		    new_centers[index][j] += feature[i][j];
            }
	}
        else {
            #pragma omp parallel \
                    shared(feature,clusters,membership,partial_new_centers,partial_new_centers_len)
            {
                int tid = omp_get_thread_num();
                #pragma omp for \
                            private(i,j,index) \
                            firstprivate(npoints,nclusters,nfeatures) \
                            schedule(static) \
                            reduction(+:delta)
                for (i=0; i<npoints; i++) {
	            /* find the index of nestest cluster centers */
	            index = find_nearest_point(feature[i],
				               nfeatures,
				               clusters,
				               nclusters);
	            /* if membership changes, increase delta by 1 */
	            if (membership[i] != index) delta += 1.0;

	            /* assign the membership to object i */
	            membership[i] = index;

	            /* update new cluster centers : sum of all objects located
		       within */
	            partial_new_centers_len[tid][index]++;
	            for (j=0; j<nfeatures; j++)
		        partial_new_centers[tid][index][j] += feature[i][j];
                }
            } /* end of #pragma omp parallel */

            /* let the main thread perform the array reduction */
            for (i=0; i<nclusters; i++) {
                for (j=0; j<nthreads; j++) {
                    new_centers_len[i] += partial_new_centers_len[j][i];
                    partial_new_centers_len[j][i] = 0.0;
                    for (k=0; k<nfeatures; k++) {
                        new_centers[i][k] += partial_new_centers[j][i][k];
                        partial_new_centers[j][i][k] = 0.0;
                    }
                }
            }
        }

	/* replace old cluster centers with new_centers */
        for (i=0; i<nclusters; i++) {
            for (j=0; j<nfeatures; j++) {
                if (new_centers_len[i] > 0)
		    clusters[i][j] = new_centers[i][j] / new_centers_len[i];
		new_centers[i][j] = 0.0;   /* set back to 0 */
	    }
	    new_centers_len[i] = 0;   /* set back to 0 */
	}
            
        delta /= npoints;
    } while (delta > threshold && loop++ < 500);

    if (_debug) {
	timing = omp_get_wtime() - timing;
        printf("nloops = %2d (T = %7.4f)",loop,timing);
    }
    free(new_centers[0]);
    free(new_centers);
    free(new_centers_len);

    return clusters;
}

