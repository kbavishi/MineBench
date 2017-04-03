/*************************************************************************/
/**   File:         fuzzy_kmeans.c                                      **/
/**   Description:  Implementation of fuzzy c-means clustering          **/
/**                 algorithm and cluster validity function.            **/
/**   Author:  Brendan McCane                                           **/
/**            James Cook University of North Queensland.               **/
/**            Australia. email: mccane@cs.jcu.edu.au                   **/
/**                                                                     **/
/**   Edited by: Jay Pisharath, Wei-keng Liao                           **/
/**              Northwestern University.                               **/
/**                                                                     **/
/*************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <math.h>
#include "fuzzy_kmeans.h"
#include <omp.h>

#define SQR(x) ((x)*(x))

extern double wtime(void);

/*----< multid_feuclid_dist() >----------------------------------------------*/
/* multi-dimensional spatial Euclid distance */
__inline
float multid_feuclid_dist(float *pt1,
                          float *pt2,
                          int    numdims)
{
    float ans=0.0;
    int i;

    for (i=0; i<numdims; i++)
        ans += SQR(pt1[i]-pt2[i]);

    ans = sqrt((double)ans);
    return(ans);
}

/*----< find_nearest_point() >-----------------------------------------------*/
__inline
int find_nearest_point(float  *pt,          /* [nfeatures] */
                       int     nfeatures,
                       float **pts,         /* [npts][nfeatures] */
                       int     npts)
{
    int index, i;
    float max_dist=FLT_MAX;

    /* find the cluster center id with min distance to pt */
    for (i=0; i<npts; i++) {
        float dist;
        dist = euclid_dist_2(pt, pts[i], nfeatures);  /* no need square root */
        if (dist < max_dist) {
            max_dist = dist;
            index    = i;
        }
    }
    return(index);
}

/* --------------------------------------------------------------------
 * Procedure: fuzzy_kmeans_cluster
 * Comments:  cluster elements into nclusters clusters.
 * -------------------------------------------------------------------- */

/*----< sum_fuzzy_members() >------------------------------------------------*/
__inline
void sum_fuzzy_members(float **feature,         /* [npoints][nfeatures] */
                       int     npoints,
                       float **cluster_centres, /* [nclusters] */
                       int     nclusters,
                       int     nfeatures,
                       float   fuzzyq,
                       float  *sum_points)      /* [npoints] */
{
    int   i, j;
    float distance;

    #pragma omp parallel for \
                shared(sum_points,feature,cluster_centres) \
                firstprivate(npoints,nclusters,nfeatures,fuzzyq) \
                private(i, j, distance) \
                schedule (static)
    for (i=0; i<npoints; i++) { /* for every object */
        sum_points[i] = 0.0;
        for (j=0; j<nclusters; j++) { /* for each cluster center */
            /* square of Euclid distance */
            distance = euclid_dist_2(feature[i], cluster_centres[j], nfeatures);

            /* ??? */
            /* sum of all fuzzy distance to the cluster centers */
            sum_points[i] += pow(1.0/(distance+0.00001), 1.0/(fuzzyq-1));
        }
    }
}

#define RANDOM_MAX 2147483647

/*----< fuzzy_kmeans_cluster() >---------------------------------------------*/
float **fuzzy_kmeans_cluster(int     is_perform_atomic,
                             float **feature,    /* [npoints][nfeatures] */
                             int     nfeatures,
                             int     npoints,
                             int     nclusters,
                             float   fuzzyq,
                             float   delta_centres_threshold)
                                     /* is this the tolerate error ? */
{
    float **cluster_centres;
    float max_delta_centres=0.0;
    float *sum_points;
    int i, j, k, index, loop=0;
    double timing;
    float sqd_dist, membership;

    int     nthreads;
    float **new_centre, *sum;
    float **partial_sum, ***partial_new_centre;

    nthreads = omp_get_max_threads();

    /* need sum and new_centre[0] initial 0 */
    sum = (float*) calloc(nclusters, sizeof(float));
    new_centre    = (float**) malloc(nclusters *            sizeof(float*));
    new_centre[0] = (float*)  calloc(nclusters * nfeatures, sizeof(float));
    for (i=1; i<nclusters; i++)
        new_centre[i] = new_centre[i-1] + nfeatures;

    if (!is_perform_atomic) {
        partial_sum    = (float**) malloc(nthreads          * sizeof(float*));
        partial_sum[0] = (float*)  calloc(nthreads*nclusters, sizeof(float));
        for (i=1; i<nthreads; i++)
            partial_sum[i] = partial_sum[i-1] + nclusters;

        partial_new_centre    = (float***)malloc(nthreads         *
	                                         sizeof(float**));
        partial_new_centre[0] = (float**) malloc(nthreads*nclusters*
	                                         sizeof(float*));
        for (i=1; i<nthreads; i++)
            partial_new_centre[i] = partial_new_centre[i-1] + nclusters;
        for (i=0; i<nthreads; i++)
            for (j=0; j<nclusters; j++)
                partial_new_centre[i][j] = (float*)calloc(nfeatures,
		                                          sizeof(float));
    }

    cluster_centres   = (float**) malloc(nclusters * sizeof(float*));
    cluster_centres[0]= (float*)  malloc(nclusters * nfeatures * sizeof(float));
    for (i=1; i<nclusters; i++)
        cluster_centres[i] = cluster_centres[i-1] + nfeatures;

    /* cluster vector: coords of each cluster centre */
    for (i=0; i<nclusters; i++) {
        /* randomly pick a cluster centre */
        int n = (int)random() % npoints;
        for (j=0; j<nfeatures; j++)
            cluster_centres[i][j] = feature[n][j];
/*
            cluster_centres[i][j] = feature[i*(int)(npoints/nclusters)][j]
                                  + random()/(float)RANDOM_MAX;
*/
    }

    /* sum_points: vector for sums of dists of each pt to all clusters */
    sum_points = (float*) malloc(npoints * sizeof(float));

    if (_debug) timing = omp_get_wtime();
    do {
        max_delta_centres = 0.0;
        /* to improve efficiency without using bazillions of memory */
        /* calc the sum of distances from each pt to all clusters */
        sum_fuzzy_members(feature,         /* [npoints][nfeatures] */
                          npoints,
                          cluster_centres, /* [nclusters] */
                          nclusters,
                          nfeatures,
                          fuzzyq,
                          sum_points);     /* out: [npoints] */

        if (is_perform_atomic) {
            #pragma omp parallel for \
                        firstprivate(npoints,nclusters,nfeatures,fuzzyq) \
                        shared(feature,cluster_centres,sum,new_centre) \
                        private(i,j,k,sqd_dist,membership) \
                        schedule(static)
            for (j=0; j<npoints; j++) {
                /* calculate fuzzy memberships of each point j to cluster i */
                for (i=0; i<nclusters; i++) {
                    sqd_dist = euclid_dist_2(feature[j],
                                             cluster_centres[i],
                                             nfeatures);
                    /* the possibility of object j belong to clustr i */
                    membership = pow(1.0/(sqd_dist+0.00001), 1.0/(fuzzyq-1)) /
                                 (sum_points[j]+0.00001);
                    membership = pow(membership, fuzzyq);

                    #pragma omp atomic 
                    sum[i] += membership;
                    for (k=0; k<nfeatures; k++) {
                         #pragma omp atomic 
                         new_centre[i][k] += membership * feature[j][k];
                    }
                }
            }
        }
	else {
            #pragma omp parallel \
                        shared(feature,cluster_centres,partial_sum,new_centre)
            {
                int tid = omp_get_thread_num();
                #pragma omp for \
                            firstprivate(npoints,nclusters,nfeatures,fuzzyq) \
                            private(i,j,k,sqd_dist,membership) \
                            schedule(static)
                for (j=0; j<npoints; j++) {
                    /* calculate fuzzy membership of point j to cluster i */
                    for (i=0; i<nclusters; i++) {
                        sqd_dist = euclid_dist_2(feature[j],
                                                 cluster_centres[i],
                                                 nfeatures);
                        /* the possibility of object j belong to clustr i */
                        membership = pow(1.0/(sqd_dist+0.00001), 1.0/(fuzzyq-1))
			             / (sum_points[j]+0.00001);
                        membership = pow(membership, fuzzyq);

                        partial_sum[tid][i] += membership;
                        for (k=0; k<nfeatures; k++)
                            partial_new_centre[tid][i][k] += membership *
			                                     feature[j][k];
                    }
                }
            } /* end of #pragma omp parallel */

            /* let the main thread perform the array reduction */
            for (i=0; i<nclusters; i++) {
                sum[i] = 0.0;
                for (k=0; k<nfeatures; k++) new_centre[i][k] = 0.0;
                for (j=0; j<nthreads; j++) {
                    sum[i] += partial_sum[j][i];
                    partial_sum[j][i] = 0.0;
                    for (k=0; k<nfeatures; k++) {
                        new_centre[i][k] += partial_new_centre[j][i][k];
                        partial_new_centre[j][i][k] = 0.0;
                    }
                }
            }
        }

        for (i=0; i<nclusters; i++) {
            float delta_centre;
            for (k=0; k<nfeatures; k++)
                new_centre[i][k] /= sum[i];
            sum[i] = 0.0;
            
            /* distance between new and old centers */
            delta_centre = multid_feuclid_dist(cluster_centres[i],
                                               new_centre[i],
                                               nfeatures);
            for (j=0; j<nfeatures; j++) {
                cluster_centres[i][j] = new_centre[i][j];
                new_centre[i][j] = 0.0; /* reset new_centre[] to 0 */
            }

            /* find max of center changes among k centers */
            if (delta_centre>max_delta_centres)
                max_delta_centres = delta_centre;
        }
    } while (max_delta_centres > delta_centres_threshold && loop++ < 500);

    if (_debug) {
        timing = omp_get_wtime() - timing;
        printf("nloops = %2d (T = %7.4f)",loop,timing);
    }                     

    free(sum);
    free(new_centre[0]);
    free(new_centre);
    free(sum_points);

    return(cluster_centres);
}

/* --------------------------------------------------------------------
 * Procedure: fuzzy_validity
 * Comments:  Calculates a fuzzy validity criterion based on compactness
 *            and separation of clusters.
 *              From Xie and Beni, vol PAMI-13, no8, pp.841-847, August, 91.
 * -------------------------------------------------------------------- */

/*----< fuzzy_validity() >---------------------------------------------------*/
float fuzzy_validity(float **feature,         /* [npoints][nfeatures] */
                     int     nfeatures,
                     int     npoints,
                     float **cluster_centres, /* [nclusters][nfeatures] */
                     int     nclusters, 
                     float   fuzzyq)
{
    int    i, j, k;
    int    nearest_cluster;
    int   *numpoints_in_clusters;
    float *sum_points;
    float  valid_sum=0.0, min_dist=FLT_MAX;
    float  ret, penalty;
    float  sqd_dist, memb_fuzzyq, membership;

    if (nclusters<2) return(FLT_MAX);

    numpoints_in_clusters = (int*)calloc(nclusters, sizeof(int));
    /* calculate number of points in each cluster */
    #pragma omp parallel for \
                shared(feature,cluster_centres,numpoints_in_clusters) \
                firstprivate(npoints,nfeatures,nclusters) \
                private(i, nearest_cluster) \
                schedule (static)
    for (i=0; i<npoints; i++) {
        nearest_cluster = find_nearest_point(feature[i], 
                                             nfeatures, 
                                             cluster_centres, 
                                             nclusters);
        #pragma omp atomic 
        numpoints_in_clusters[nearest_cluster]++;
    }

    /* calculate function which penalises validity if the clusters have
       a small number of points in them. */
    penalty=0.0;
    #pragma omp parallel for \
                shared(numpoints_in_clusters) \
                firstprivate(nclusters) \
                private(i) \
                schedule (static) \
                reduction(+:penalty)
    for (i=0; i<nclusters; i++)
        penalty += 1.0/(1.0+((float)numpoints_in_clusters[i]));
    penalty /= (float)nclusters;
    free(numpoints_in_clusters);
/*
    feature = (float**)calloc(npoints, sizeof(float*));
    for (i=0; i<npoints; i++)
        feature[i] = (float*)calloc(nfeatures, sizeof(float));
*/

    /* sum_points: vector for sums of dists of each pt to all clusters */
    sum_points = (float*) malloc(npoints * sizeof(float));

    /* to improve efficiency without using bazillions of memory */
    /* calc the sum of distances from each pt to all clusters */
    sum_fuzzy_members(feature,
                      npoints,
                      cluster_centres,
                      nclusters,
                      nfeatures,
                      fuzzyq,
                      sum_points);   /* out: [npoints] */

    #pragma omp parallel for \
                shared(feature,cluster_centres) \
                firstprivate(npoints,nclusters,nfeatures,fuzzyq) \
                private(i,j,sqd_dist,memb_fuzzyq,membership) \
                schedule(static) \
                reduction(+:valid_sum)
    for (j=0; j<npoints; j++) {
        /* calculate fuzzy memberships of each point */
        for (i=0; i<nclusters; i++) {
            sqd_dist = euclid_dist_2(feature[j],
                                     cluster_centres[i],
                                     nfeatures);
            membership = pow(1.0/(sqd_dist+0.00001), 1.0/(fuzzyq-1)) /
                         (sum_points[j]+0.00001);
            memb_fuzzyq = pow(membership, fuzzyq);

            valid_sum += sqd_dist*memb_fuzzyq;
        }
    }
    free(sum_points);

    /* find minimum distance between any two cluster centres */
    /*                               ^^^^^^^  ???            */
    min_dist = 0.0;
    for (i=0; i<nclusters-1; i++) {
        float this_cluster_min_dist=FLT_MAX;
        for (j=i+1; j<nclusters; j++) {
            /* ??? these 2 loops are not for ANY TWO ??? */
            float tmp_dist;
            tmp_dist = multid_feuclid_dist(cluster_centres[i], 
                                           cluster_centres[j],
                                           nfeatures);
            if (tmp_dist<this_cluster_min_dist)
                this_cluster_min_dist = tmp_dist;
            /* find the min distance between i and centers > i */
        }
        min_dist += this_cluster_min_dist;
    }
    min_dist = SQR((min_dist/(float)(nclusters-1)));

    ret = (float)(valid_sum/(npoints*min_dist)+penalty);
    return(ret);
}
