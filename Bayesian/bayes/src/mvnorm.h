/*----------------------------------------------------------------------
  File    : mvnorm.h
  Contents: Multivariate normal distribution estimation and management
  Author  : Christian Borgelt
  History : 10.11.2000 file created
            26.11.2000 first version completed
            24.05.2001 possibilistic parameter added
----------------------------------------------------------------------*/
#ifndef __MVNORM__
#define __MVNORM__
#include <stdio.h>
#include <float.h>
#ifdef MVN_PARSE
#ifndef SC_SCAN
#define SC_SCAN
#endif
#include "scan.h"
#endif

/*----------------------------------------------------------------------
  Preprocessor Definitions
----------------------------------------------------------------------*/
#define MVN_UNKNOWN (-DBL_MAX)  /* an unknown value */

/* --- evaluation flags --- */
#define MVN_EXPVAR   0x0001     /* compute exp. values and variances */
#define MVN_COVAR    0x0002     /* compute covariances */
#define MVN_CORREL   0x0004     /* compute correlation coefficients */
#define MVN_DECOM    0x0008     /* compute Cholesky decomposition */
#define MVN_INVERSE  0x0010     /* compute inverse matrix */
#define MVN_ALL      0x001f     /* compute all parameters */
#define MVN_MAXLLH   0x0080     /* use max. likelihood estimates */
                                /* default: use unbiased estimates */

/*----------------------------------------------------------------------
  Type Definitions
----------------------------------------------------------------------*/
typedef struct {                /* --- matrix element --- */
  double  cnt;                  /* sum of number of cases */
  double  sr;                   /* sum of row    var. values */
  double  sr2;                  /* sum of squared row    var. values */
  double  sc;                   /* sum of column var. values */
  double  sc2;                  /* sum of squared column var. values */
  double  src;                  /* sum of product of values */
} MVNELEM;                      /* (matrix element) */

typedef struct {                /* --- matrix row --- */
  double  cv;                   /* number of cases * current value */
  double  cv2;                  /* number of cases * current value^2 */
  double  cnt;                  /* sum of number of cases */
  double  sv;                   /* sum of values */
  double  sv2;                  /* sum of squared values */
  MVNELEM elems[1];             /* matrix elements */
} MVNROW;                       /* (matrix row) */

typedef struct {                /* --- multivariate normal dist. --- */
  int     size;                 /* number of rows / columns */
  double  *exps;                /* expected values */
  double  **covs;               /* covariance matrix */
  double  **corrs;              /* correlation coefficients */
  double  **decom;              /* Cholesky decomposition */
  double  **inv;                /* inverse of covariance matrix */
  double  det;                  /* determinant of inverse matrix */
  double  norm;                 /* normalization factor */
  double  poss;                 /* param. for possibilistic interpr. */
  double  *diff;                /* difference vector from center */
  double  *buf;                 /* buffer for intermediate results */
  MVNROW  *rows[1];             /* matrix rows for statistics */
} MVNORM;                       /* (multivariate normal distribution) */

/*----------------------------------------------------------------------
  Functions
----------------------------------------------------------------------*/
extern MVNORM* mvn_create (int size);
extern MVNORM* mvn_dup    (const MVNORM *mvn);
extern void    mvn_delete (MVNORM *mvn);
extern int     mvn_size   (const MVNORM *mvn);

extern void    mvn_clear  (MVNORM *mvn);
extern void    mvn_add    (MVNORM *mvn, const double vals[],
                           double cnt);
extern int     mvn_calc   (MVNORM *mvn, int flags);
extern double  mvn_eval   (MVNORM *mvn, const double vals[]);
extern double* mvn_rand   (MVNORM *mvn, double drand (void));

extern double  mvn_exp    (MVNORM *mvn, int index);
extern double  mvn_var    (MVNORM *mvn, int index);
extern double  mvn_cov    (MVNORM *mvn, int index1, int index2);
extern double  mvn_corr   (MVNORM *mvn, int index1, int index2);
extern double  mvn_decom  (MVNORM *mvn, int index1, int index2);
extern double  mvn_inv    (MVNORM *mvn, int index1, int index2);
extern double  mvn_det    (MVNORM *mvn);
extern double  mvn_poss   (MVNORM *mvn);

extern int     mvn_desc   (MVNORM *mvn, FILE *file,
                           int offs, int maxlen);
#ifdef MVN_PARSE
extern int     mvn_parse  (MVNORM *mvn, SCAN *scan, double cnt);
#endif

/*----------------------------------------------------------------------
  Preprocessor Definitions
----------------------------------------------------------------------*/
#define mvn_size(d)       ((d)->size)

#define mvn_exp(d,i)      ((d)->exps[i])
#define mvn_var(d,i)      ((d)->covs[i][i])
#define mvn_cov(d,i,k)    (((i) > (k)) ? (d)->covs [i][k] \
                                       : (d)->covs [k][i])
#define mvn_corr(d,i,k)   (((i) > (k)) ? (d)->corrs[i][k] \
                                       : (d)->corrs[k][i])
#define mvn_decom(d,i,k)  (((i) > (k)) ? (d)->decom[i][k] \
                                       : (d)->decom[k][i])
#define mvn_inv(d,i,k)    (((i) > (k)) ? (d)->inv  [i][k] \
                                       : (d)->inv  [k][i])
#define mvn_det(d)        ((d)->det)
#define mvn_poss(d)       ((d)->poss)

#endif
