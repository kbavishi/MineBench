/*************************************************************************/
/**   File:         cluster.c                                           **/
/**   Description:  Takes as input a file, containing 1 data point per  **/
/**                 per line, and performs a fuzzy c-means clustering   **/
/**                 on the data. Fuzzy clustering is performed using    **/
/**                 min to max clusters and the clustering that gets    **/
/**                 the best score according to a compactness and       **/
/**                 separation criterion are returned.                  **/
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
#include <string.h>
#include <limits.h>
#include <math.h>
#include <float.h>
#include <omp.h>

#include "fuzzy_kmeans.h"
extern double wtime(void);

/*---< extract_moments() >---------------------------------------------------*/
float *extract_moments(float *data,
                       int    num_elts,
                       int    num_moments)
{
    int i, j;
    float *moments;

    moments = (float *)calloc(num_moments, sizeof(float));
    for (i=0; i<num_elts; i++)
        moments[0] += data[i];

    moments[0] = moments[0] / num_elts;
    for (j=1; j<num_moments; j++) {
        moments[j] = 0;
        for (i=0; i<num_elts; i++)
            moments[j] += pow((data[i]-moments[0]), j+1);
        moments[j] = moments[j] / (num_elts);
    }
    return(moments);
}

/*---< zscore_transform() >--------------------------------------------------*/
void zscore_transform(float **data, /* in & out: [numObjects][numAttributes] */
                      int     numObjects,
                      int     numAttributes)
{
    float *single_variable, *moments;
    int i, j;

    single_variable = (float*)calloc(numObjects, sizeof(float));
    for (i=0; i<numAttributes; i++) {
        for (j=0; j<numObjects; j++)
            single_variable[j] = data[j][i];
        moments = extract_moments(single_variable, numObjects, 2);
        moments[1] = (float)sqrt((double)moments[1]);
        for (j=0; j<numObjects; j++)
            data[j][i] = (data[j][i]-moments[0])/moments[1];
        free(moments);
    }
    free(single_variable);
}

/*---< cluster() >-----------------------------------------------------------*/
int cluster(int      perform_fuzzy_kmeans, /* in: */
            int      is_perform_valid,     /* in: */
            int      is_perform_atomic,    /* in: */
            int      is_perform_assign,    /* in: */
            int      numObjects,      /* number of input objects */
            int      numAttributes,   /* size of attribute of each object */
            float  **attributes,      /* [numObjects][numAttributes] */
            int      use_zscore_transform,
            int      min_nclusters,   /* testing k range from min to max */
            int      max_nclusters,
            float    fuzzyq,          /* fuzziness factor  1 ... infinity */
            float    threshold,       /* in:   */
            int     *best_nclusters,  /* out: number between min and max */
            float ***cluster_centres, /* out: [best_nclusters][numAttributes] */
            int     *cluster_assign,  /* out: [numObjects] */
            float   *validity,        /* out: [max_nclusters-min_nclusters+1] */
            double  *cluster_timing,  /* out: [max_nclusters-min_nclusters+1] */
            double  *valid_timing)    /* out: [max_nclusters-min_nclusters+1] */
{
    int     i, j, itime;
    int     nclusters;
    int    *membership;
    float **tmp_cluster_centres;
    float   min_valid=FLT_MAX;
    double  assign_timing;

    if (!perform_fuzzy_kmeans)
        membership = (int*) malloc(numObjects * sizeof(int));

    if (use_zscore_transform)
        zscore_transform(attributes, numObjects, numAttributes);

    if (_debug)
        printf("Initial min_nclusters = %d max_nclusters = %d\n",
               min_nclusters,max_nclusters);

    itime = 0;
    /* from min_nclusters to max_nclusters, find best_nclusters */
    for (nclusters=min_nclusters; nclusters<=max_nclusters; nclusters++) {
        /* set the seed for random() called in subroutines later */
        srandom(7);

        cluster_timing[itime] = omp_get_wtime();
        if (perform_fuzzy_kmeans) {
            tmp_cluster_centres = fuzzy_kmeans_cluster(is_perform_atomic,
                                                       attributes,
                                                       numAttributes,
                                                       numObjects,
                                                       nclusters,
                                                       fuzzyq,
                                                       threshold);
        }
        else {
            tmp_cluster_centres = kmeans_clustering(is_perform_atomic,
                                                    attributes,
                                                    numAttributes,
                                                    numObjects,
                                                    nclusters,
                                                    threshold,
                                                    membership);
        }
        cluster_timing[itime] = omp_get_wtime() - cluster_timing[itime];

        /* ---------------------------------------------------------------
          Calculates a fuzzy validity criterion based on compactness
          and separation of clusters.
         * ------------------------------------------------------------- */
        if (is_perform_valid) {
            valid_timing[itime] = omp_get_wtime();
            validity[itime] =
                fuzzy_validity(attributes,   /* [numObjects][numAttributes] */
                               numAttributes,
                               numObjects,
                               tmp_cluster_centres,
                               nclusters,
                               fuzzyq);
            if (_debug) printf("K = %2d  validity = %6.4f\n", nclusters,validity[itime]);

            if (validity[itime] < min_valid) { /* replace the cluster results
	                                          with a smaller validity */
                if (*cluster_centres) {
                    free((*cluster_centres)[0]);
                    free(*cluster_centres);
                }
                *cluster_centres = tmp_cluster_centres;
                min_valid = validity[itime];
                *best_nclusters = nclusters;
                if (!perform_fuzzy_kmeans)
                    memcpy(cluster_assign, membership, numObjects *sizeof(int));
            }
            else {
                free(tmp_cluster_centres[0]);
                free(tmp_cluster_centres);
            }
            valid_timing[itime] = omp_get_wtime() - valid_timing[itime];
	}
        else {
            if (*cluster_centres) {
                free((*cluster_centres)[0]);
                free(*cluster_centres);
            }
            *cluster_centres = tmp_cluster_centres;
            *best_nclusters = nclusters;
        }

        if (_debug) {
            printf("K = %2d T_cluster = %7.4f", nclusters,cluster_timing[itime]);
            if (is_perform_valid)
                printf(" T_valid = %7.4f", valid_timing[itime]);
            printf("\n");
        }
        itime++;
    }

    if (perform_fuzzy_kmeans) {
        if (is_perform_assign) {
        /* the closest cluster centre to each of the data points ------------*/
        if (_debug) assign_timing = omp_get_wtime();
            #pragma omp parallel for \
                  shared(cluster_assign,attributes,cluster_centres,best_nclusters) \
                  firstprivate(numObjects,numAttributes) \
                  private(i) \
                  schedule (static)
            for (i=0; i<numObjects; i++)
                cluster_assign[i] = find_nearest_point(attributes[i],
                                                       numAttributes,
                                                       *cluster_centres,
                                                       *best_nclusters);
            if (_debug) {
                assign_timing = omp_get_wtime() - assign_timing;
                printf("cluster assign timing = %8.4f sec\n", assign_timing);
            }
        }
    }

    if (!perform_fuzzy_kmeans)
        free(membership);

    return 0;
}

