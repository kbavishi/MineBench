/*************************************************************************/
/**   File:         example.c                                           **/
/**   Description:  Takes as input a file:                              **/
/**                 ascii  file: containing 1 data point per line       **/
/**                 binary file: first int is the number of objects     **/
/**                              2nd int is the no. of features of each **/
/**                              object                                 **/
/**                 This example performs a fuzzy c-means clustering    **/
/**                 on the data. Fuzzy clustering is performed using    **/
/**                 min to max clusters and the clustering that gets    **/
/**                 the best score according to a compactness and       **/
/**                 separation criterion are returned.                  **/
/**   Author:  Wei-keng Liao                                            **/
/**            ECE Department Northwestern University                   **/
/**            email: wkliao@ece.northwestern.edu                       **/
/**                                                                     **/
/**   Edited by: Jay Pisharath                                          **/
/**              Northwestern University.                               **/
/**                                                                     **/
/*************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
#include <math.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <fcntl.h>
#include <omp.h>

#include "fuzzy_kmeans.h"

extern double wtime(void);

/*---< usage() >------------------------------------------------------------*/
void usage(char *argv0) {
    char *help =
        "Usage: %s [switches] -i filename\n"
        "       -i filename:     file containing data to be clustered\n"
        "       -b               input file is in binary format\n"
        "       -q fuzziness:    fuzziness factor for fuzzy clustering\n"
        "                        1.0 is non-fuzzy up to infinity\n"
        "       -f               to perform fuzzy kmeans clustering\n"
        "                        default is regular kmeans clustering\n"
        "       -m max_clusters: maximum number of clusters allowed\n"
        "       -n min_clusters: minimum number of clusters allowed\n"
        "       -z             : don't zscore transform data\n"
        "       -t threshold   : threshold value\n"
        "       -p nproc       : number of threads\n"
        "       -v             : calculate the validity \n"
        "       -a             : perform atomic OpenMP pragma\n"
        "       -s             : perform object assignment if fuzzy performs\n"
        "       -o             : output timing results, default 0\n"
        "       -d             : enable debug mode\n";
    fprintf(stderr, help, argv0);
    exit(-1);
}

/*---< main() >-------------------------------------------------------------*/
int main(int argc, char **argv) {
           int     opt;
    extern char   *optarg;
    extern int     optind;
           float   fuzzyq=1.5;
           int     max_nclusters=13;
           int     min_nclusters=4;
           char   *filename = 0;
           FILE   *cluster_centre_file;
           FILE   *clustering_file;
           float  *buf;
           float **attributes;
           float **cluster_centres=NULL;
           float **tmp_cluster_centres;
           int     i, j;
           int     nclusters;
           int     best_nclusters;
           int    *cluster_assign;
           int     numAttributes;
           int     numObjects;
           int     use_zscore_transform=1;
           char    line[1024];
           char    outFileName[1024];
           int     isBinaryFile = 0;
           int     nloops, len, nthreads;
           int     perform_fuzzy_kmeans = 0;
           int     is_perform_valid = 0;
           int     is_perform_atomic = 0;
           int     is_perform_assign = 0;
           int     is_perform_output = 0;
           int     _timing_tables;
           float  *validity;  /* [max_nclusters-min_nclusters+1] */
           float   threshold = 0.001;
           double  sum, timing, min_timing = FLT_MAX, io_timing;
           double *clustering_timing;   /* [max_nclusters-min_nclusters+1] */
           double *valid_timing;        /* [max_nclusters-min_nclusters+1] */
           double *min_cluster_timing;  /* [max_nclusters-min_nclusters+1] */
           double *min_valid_timing;    /* [max_nclusters-min_nclusters+1] */

    _debug = 0;
    nthreads = 0;
    while ( (opt=getopt(argc,argv,"p:i:q:m:n:t:avbzdfso"))!= EOF) {
        switch (opt) {
            case 'i': filename=optarg;
                      break;
            case 'b': isBinaryFile = 1;
                      break;
            case 'q': fuzzyq=atof(optarg);
                      break;
            case 'f': perform_fuzzy_kmeans=1;
                      break;
            case 't': threshold=atof(optarg);
                      break;
            case 'm': max_nclusters = atoi(optarg);
                      break;
            case 'n': min_nclusters = atoi(optarg);
                      break;
            case 'z': use_zscore_transform = 0;
                      break;
            case 'p': nthreads = atoi(optarg);
                      break;
            case 'v': is_perform_valid = 1;
                      break;
            case 'a': is_perform_atomic = 1;
                      break;
            case 's': is_perform_assign = 1;
                      break;
            case 'o': is_perform_output = 1;
                      break;
            case 'd': _debug = 1;
                      break;
            case '?': usage(argv[0]);
                      break;
            default: usage(argv[0]);
                      break;
        }
    }

    if (filename == 0) usage(argv[0]);

/*
    if (perform_fuzzy_kmeans == 1)
        is_perform_valid = 1;
*/
    if (nthreads > 0)  /* if specified in command line */
        omp_set_num_threads(nthreads);

    numAttributes = numObjects = 0;

    /* from the input file, get the numAttributes and numObjects ------------*/
    io_timing = omp_get_wtime();
    if (isBinaryFile) {
        int infile;
        if ((infile = open(filename, O_RDONLY, "0600")) == -1) {
            fprintf(stderr, "Error: file %s (%s)\n", filename,strerror(errno));
            exit(1);
        }
        read(infile, &numObjects,    sizeof(int));
        read(infile, &numAttributes, sizeof(int));
        if (_debug) {
            printf("File %s contains numObjects = %d\n",filename,numObjects);
            printf("File %s, number of attributes in each point = %d\n",filename,numAttributes);
        }

        /* allocate space for attributes[] and read attributes of all objects */
        buf           = (float*) malloc(numObjects*numAttributes*sizeof(float));
        attributes    = (float**)malloc(numObjects*             sizeof(float*));
        attributes[0] = (float*) malloc(numObjects*numAttributes*sizeof(float));
        for (i=1; i<numObjects; i++)
            attributes[i] = attributes[i-1] + numAttributes;

        read(infile, buf, numObjects*numAttributes*sizeof(float));

        close(infile);
    }
    else {
        FILE *infile;
        if ((infile = fopen(filename, "r")) == NULL) {
            fprintf(stderr, "Error: file %s (%s)\n", filename,strerror(errno));
            exit(1);
        }
        while (fgets(line, 1024, infile) != NULL)
            if (strtok(line, " \t\n") != 0)
                numObjects++;
        rewind(infile);
        while (fgets(line, 1024, infile) != NULL) {
            if (strtok(line, " \t\n") != 0) {
                /* ignore the id (first attribute): numAttributes = 1; */
                while (strtok(NULL, " ,\t\n") != NULL) numAttributes++;
                break;
            }
        }
        if (_debug) {
            printf("File %s contains numObjects = %d\n",filename,numObjects);
            printf("File %s, number of attributes in each point = %d\n",filename,numAttributes);
        }

        /* allocate space for attributes[] and read attributes of all objects */
        buf           = (float*) malloc(numObjects*numAttributes*sizeof(float));
        attributes    = (float**)malloc(numObjects*             sizeof(float*));
        attributes[0] = (float*) malloc(numObjects*numAttributes*sizeof(float));
        for (i=1; i<numObjects; i++)
            attributes[i] = attributes[i-1] + numAttributes;
        rewind(infile);
        i = 0;
        while (fgets(line, 1024, infile) != NULL) {
            if (strtok(line, " \t\n") == NULL) continue;
            /* if (_debug) printf("[%2d] : ",i); */
            for (j=0; j<numAttributes; j++) {
                buf[i] = atof(strtok(NULL, " ,\t\n"));
                /* if (_debug) printf("%6.2f ", buf[i]); */
                i++;
            }
            /* if (_debug) printf("\n"); */
        }
        fclose(infile);
    }
    io_timing = omp_get_wtime() - io_timing;

#ifdef _PERFORM_IO_ONLY_
#define _PERFORM_IO_ONLY_
    printf("Number of threads = %d\n", omp_get_max_threads());
    printf("File %s contains  numObjects = %d, each of size %d\n",
               filename,numObjects,numAttributes);
    printf("I/O time = %8.4f\n", io_timing);

    exit(0);
#endif

    /* the core of the clustering ==========================================*/
    cluster_assign = (int*) malloc(numObjects * sizeof(int));

    nloops = 8;
    len    = max_nclusters - min_nclusters + 1;
    validity           = (float*)  calloc(len, sizeof(float));
     clustering_timing = (double*) calloc(len, sizeof(double));
          valid_timing = (double*) calloc(len, sizeof(double));
    min_cluster_timing = (double*) calloc(len, sizeof(double));
      min_valid_timing = (double*) calloc(len, sizeof(double));

    for (i=0; i<nloops; i++) {
        /* since zscore transform may perform in cluster() which modifies the
           contents of attributes[][], we need to re-store the originals */
        memcpy(attributes[0], buf, numObjects*numAttributes*sizeof(float));

        timing = omp_get_wtime();
        cluster_centres = NULL;
        cluster(perform_fuzzy_kmeans,
                is_perform_valid,
                is_perform_atomic,
                is_perform_assign,
                numObjects,
                numAttributes,
                attributes,           /* [numObjects][numAttributes] */
                use_zscore_transform, /* 0 or 1 */
                min_nclusters,        /* pre-define range from min to max */
                max_nclusters,
                fuzzyq,               /* fuzziness factor  1 ... infinity */
                threshold,
                &best_nclusters,      /* return: number between min and max */
                &cluster_centres,     /* return:
                                         [best_nclusters][numAttributes] */
                cluster_assign,       /* return: [numObjects] cluster id for
                                         each object */
                validity,             /* return: [len] */
                clustering_timing,    /* return: [len] */
                valid_timing);        /* return: [len] */

        timing = omp_get_wtime() - timing;
        if (_debug) printf("nloop = %d cluster() time = %.4f\n", i, timing);
        if (timing < min_timing) {
            min_timing = timing;
            for (j=0; j<len; j++) {
                min_cluster_timing[j] = clustering_timing[j];
                  min_valid_timing[j] =      valid_timing[j];
            }
        }
    }

    if (is_perform_output) {
        printf("Number of threads = %d\n", omp_get_max_threads());

        printf("File %s contains  numObjects = %d, each of size %d\n",
               filename,numObjects,numAttributes);
        if (perform_fuzzy_kmeans) {
	    if (is_perform_assign)
                printf("**** Fuzzy Kmeans (Loop N first) with assign ****");
	    else
                printf("**** Fuzzy Kmeans (Loop N first) without assign ****");
        }
        else
            printf("Performing **** Regular Kmeans (Loop N first) ****");

        if (is_perform_atomic)
            printf(" use atomic pragma ******\n");
        else
            printf(" use array reduction ******\n");

        _timing_tables = 1;
        if (_timing_tables) printf(" K, Tcluster,   Tvalid,   Tsum\n");

        for (i=0; i<len; i++) {
            if (_timing_tables) {
                printf("%2d, %8.4f, %8.4f, %8.4f\n", min_nclusters+i,
                       min_cluster_timing[i], min_valid_timing[i], 
                       min_cluster_timing[i]+min_valid_timing[i]);
            }
            else {
                printf("for %2d clusters: ", min_nclusters+i);
                printf("validity = %6.4f", validity[i]);
                printf(" T_cluster = %8.4f", min_cluster_timing[i]);
                printf(" T_valid = %8.4f",  min_valid_timing[i]);
                printf(" T_sum = %8.4f\n",  min_cluster_timing[i]+min_valid_timing[i]);
            }
        }

        for (i=0; i<79; i++) printf("-"); printf("\n");
        sum = 0.0; for (i=0; i<len; i++) sum += min_cluster_timing[i];
        if (_timing_tables) printf("  , %8.4f,", sum);
        else printf("sum                                              %.4f", sum);
        sum = 0.0; for (i=0; i<len; i++) sum += min_valid_timing[i];
        if (_timing_tables) printf(" %8.4f,", sum);
        else printf("           %8.4f",sum);
        for (i=0; i<len; i++) sum += min_cluster_timing[i];
        if (_timing_tables) printf(" %8.4f\n", sum);
        else printf("           %.4f\n",sum);
        printf("I/O time = %8.4f\n", io_timing);
        if (!_timing_tables) {
            printf("Conclude : best no. of clusters found = %d\n", best_nclusters);
            printf("Total timing = %10.4f sec\n", min_timing);
        }
        for (i=0; i<79; i++) printf("-"); printf("\n");
    }

#ifdef _GNUPLOT_OUTPUT_
#define _GNUPLOT_OUTPUT_
    {
        FILE **fptr;
        fptr = (FILE**) malloc(best_nclusters * sizeof(FILE*));
        for (i=0; i<best_nclusters; i++) {
            sprintf(outFileName, "group.%d", i);
            fptr[i] = fopen(outFileName, "w");
        }
        for (i=0; i<numObjects; i++) {
            fprintf(fptr[cluster_assign[i]], "%6.4f %6.4f\n",
                    attributes[i][0], attributes[i][1]);
        }
        for (i=0; i<best_nclusters; i++)
            fclose(fptr[i]);
        free(fptr);
    }
#endif

#ifdef _OUTPUT_TO_FILE_
    /* output: the coordinates of the cluster centres ----------------------*/
    sprintf(outFileName, "%s.cluster_centres", filename);
    cluster_centre_file = fopen(outFileName, "w");
    for (i=0; i<best_nclusters; i++) {
        fprintf(cluster_centre_file, "%d ", i);
        for (j=0; j<numAttributes; j++)
            fprintf(cluster_centre_file, "%f ", cluster_centres[i][j]);
        fprintf(cluster_centre_file, "\n");
    }
    fclose(cluster_centre_file);

    if (_debug)
        printf("cluster_centre_file = %s\n", outFileName);

    /* output: the closest cluster centre to each of the data points --------*/
    sprintf(outFileName, "%s.cluster_assign", filename);
    clustering_file = fopen(outFileName, "w");
    for (i=0; i<numObjects; i++)
        fprintf(clustering_file, "%d %d\n", i, cluster_assign[i]);
    fclose(clustering_file);
    free(cluster_assign);

    if (_debug)
        printf("for each point, the found center is in clustering_file = %s\n",
               outFileName);
#endif

    free(min_valid_timing);
    free(min_cluster_timing);
    free(valid_timing);
    free(clustering_timing);
    free(validity);

    free(cluster_assign);
    free(attributes);
    free(cluster_centres[0]);
    free(cluster_centres);
    free(buf);

    return(0);
}

