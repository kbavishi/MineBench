#ifndef _H_FUZZY_KMEANS
#define _H_FUZZY_KMEANS

#ifndef FLT_MAX
#define FLT_MAX 3.40282347e+38
#endif

/* fuzzy_kmeans.c */
float   multid_feuclid_dist  (float*, float*, int);
float   euclid_dist_2        (float*, float*, int);
int     find_nearest_point   (float* , int, float**, int);
void    sum_fuzzy_members    (float**, int, float**, int, int, float, float*);
float **fuzzy_kmeans_cluster (int, float**, int, int, int, float, float);
float   fuzzy_validity       (float**, int, int, float**, int, float);

float  *extract_moments(float*, int, int);
void    zscore_transform(float**, int, int);
int     cluster(int, int, int, int, int, int, float**, int, int, int, float,
                float, int*, float***, int*, float*, double*, double*);

float **kmeans_clustering(int, float**, int, int, int, float, int*);

int     _debug;

#endif
