/*----------------------------------------------------------------------
  File    : matrix.h
  Contents: general vector and matrix management
  Author  : Christian Borgelt
  History : 30.04.1999 file created
            15.05.1999 diagonal element functions added
            17.05.1999 parameter 'det' added to function mat_gjinv
            18.05.1999 function 'mat_tri2sym' added
            14.07.2001 variable mat_err removed (replaced by tfs_err)
            17.08.2001 VECTOR data type replaced by double*
            13.10.2001 covariance funcs. redesigned, mat_regress added
            15.10.2001 vec_show, mat_show made macros, mat_sub added
            23.10.2001 functions mat_addsv and mat_var added
            25.10.2001 function mat_subx added
            26.10.2001 interface of function mat_regress changed
            07.11.2001 argument loc added to mat_dup, mat_copy, mat_cmp
            09.11.2001 functions mat_diasum, mat_diaprod, mat_track add.
            10.11.2001 functions vec_sqrlen, vec_sqrdist, vec_dist added
            11.11.2001 function mat_mulvdv added
            09.09.2002 initialization mode MAT_VALUE added
----------------------------------------------------------------------*/
#ifndef __MATRIX__
#define __MATRIX__
#include <stdlib.h>
#include <math.h>
#ifdef MAT_READ
#include "tfscan.h"
#endif

/*----------------------------------------------------------------------
  Preprocessor Definitions
----------------------------------------------------------------------*/
/* --- initialization modes --- */
#define MAT_ZERO      0x0000    /* set matrix to zero */
#define MAT_UNIT      0x0001    /* set a unit matrix */
#define MAT_LOWER     0x0002    /* set lower triangular matrix */
#define MAT_LEFT      0x0002    /* set left  triangular matrix */
#define MAT_UPPER     0x0004    /* set upper triangular matrix */
#define MAT_RIGHT     0x0004    /* set right triangular matrix */
#define MAT_DIAG      0x0006    /* set diagonal matrix */
#define MAT_SET       0x0008    /* set to given values */
#define MAT_RESET     0x0008    /* reset to normal state */
#define MAT_VALUE     0x0010    /* set to a single given value */
#define MAT_FULL      0x0000    /* compare full matrix */

/* --- mode flags --- */
#define MAT_FULLPIV   0x0000    /* do full pivoting */
#define MAT_PARTPIV   0x0001    /* do partial pivoting */
#define MAT_NOCOPY    0x8000    /* do not copy the input matrix */
#define MAT_INVERSE   0x4000    /* compute inverse matrix */

/* --- error codes --- */
#ifndef OK
#define OK            0         /* no error */
#define E_NONE        0         /* no error */
#define E_NOMEM     (-1)        /* not enough memory */
#define E_FOPEN     (-2)        /* file open failed */
#define E_FREAD     (-3)        /* file read failed */
#define E_FWRITE    (-4)        /* file write failed */
#endif
#ifndef E_VALUE
#define E_VALUE    (-16)        /* illegal field value */
#define E_FLDCNT   (-17)        /* wrong number of fields */
#define E_EMPFLD   (-18)        /* empty field name */
#define E_DUPFLD   (-19)        /* duplicate field name */
#define E_MISFLD   (-20)        /* missing field name */
#define E_RECCNT   (-21)        /* wrong number of records */
#endif

/*----------------------------------------------------------------------
  Type Definitions
----------------------------------------------------------------------*/
typedef struct {                /* --- a matrix --- */
  int    rowcnt;                /* number of rows */
  int    colcnt;                /* number of columns */
  int    flags;                 /* flags, e.g. ROWBLKS, ODDPERM */
  int    *map;                  /* row/column permutation map */
  double weight;                /* sum of vector weights */
  double *buf;                  /* row/column buffer, mean values */
  double *els[1];               /* matrix elements (in rows) */
} MATRIX;                       /* (matrix) */

/*----------------------------------------------------------------------
  Vector Functions
----------------------------------------------------------------------*/
/* --- basic functions --- */
extern double* vec_init    (double *vec, int n, int unit);
extern double* vec_copy    (double *dst, const double *src,   int n);
extern double  vec_cmp     (const double *a, const double *b, int n);

/* --- vector operations --- */
extern double  vec_sqrlen  (const double *vec, int n);
extern double  vec_len     (const double *vec, int n);
extern double  vec_sqrdist (const double *a, const double *b, int n);
extern double  vec_dist    (const double *a, const double *b, int n);
extern double* vec_add     (double *res, int n,
                            const double *a, double k, const double *b);
extern double* vec_muls    (double *res, int n,
                            const double *vec, double k);
extern double  vec_mul     (const double *a, const double *b, int n);
extern double  vec_sprod   (const double *a, const double *b, int n);
extern double* vec_vprod   (double *res, const double *a,
                                         const double *b);
extern MATRIX* vec_mprod   (MATRIX *res, const double *a,
                                         const double *b);

/* --- input/output functions --- */
extern void    vec_show    (const double *vec, int n);
extern int     vec_write   (const double *vec, int n, FILE *file,
                            const char *fmt, const char *sep);
#ifdef MAT_READ
extern int     vec_read    (double *vec, int n,
                            TFSCAN *tfscan, FILE *file);
extern double* vec_readx   (TFSCAN *tfscan, FILE *file, int *n);
#endif

/*----------------------------------------------------------------------
  Matrix Functions
----------------------------------------------------------------------*/
/* --- basic functions --- */
extern MATRIX* mat_create  (int rowcnt, int colcnt);
extern void    mat_delete  (MATRIX *mat);
extern MATRIX* mat_dup     (const MATRIX *mat, int loc);
extern MATRIX* mat_copy    (MATRIX *dst, const MATRIX *src,   int loc);
extern double  mat_cmp     (const MATRIX *A, const MATRIX *B, int loc);
extern int     mat_issqr   (const MATRIX *mat);
extern double* mat_buf     (MATRIX *mat);

/* --- initialization and access functions --- */
extern void    mat_init    (MATRIX *mat, int mode, const double *vals);
extern void    mat_crop    (MATRIX *mat, int loc);
extern double  mat_get     (const MATRIX *mat, int row, int col);
extern double  mat_set     (MATRIX *mat, int row, int col, double val);

/* --- row operations --- */
extern int     mat_rowcnt  (const MATRIX *mat);
extern void    mat_rowinit (MATRIX *mat, int row, int unit);
extern double* mat_row     (MATRIX *mat, int row);
extern double* mat_rowget  (const MATRIX *mat, int row, double *vec);
extern void    mat_rowset  (MATRIX *mat, int row, const double *vec);
extern double  mat_rowlen  (const MATRIX *mat, int row);
extern void    mat_rowadd  (MATRIX *mat, int row1, double k, int row2);
extern void    mat_rowaddv (MATRIX *mat, int row,
                            double k, const double *vec);
extern void    mat_rowmuls (MATRIX *mat, int row,  double k);
extern double  mat_rowmulv (const MATRIX *mat, int row,
                            const double *vec);
extern double  mat_rowmul  (const MATRIX *mat, int row1, int row2);
extern void    mat_rowexg  (MATRIX *mat, int row1, int row2);
extern void    mat_shuffle (MATRIX *mat, double randfn(void));

/* --- column operations --- */
extern int     mat_colcnt  (const MATRIX *mat);
extern void    mat_colinit (MATRIX *mat, int col, int unit);
extern double* mat_colget  (const MATRIX *mat, int col, double *vec);
extern void    mat_colset  (MATRIX *mat, int col, const double *vec);
extern double  mat_collen  (const MATRIX *mat, int col);
extern void    mat_coladd  (MATRIX *mat, int col1, double k, int col2);
extern void    mat_coladdv (MATRIX *mat, int col,
                            double k, const double *vec);
extern void    mat_colmuls (MATRIX *mat, int col,  double k);
extern double  mat_colmulv (const MATRIX *mat, int col,
                            const double *vec);
extern double  mat_colmul  (const MATRIX *mat, int col1, int col2);
extern void    mat_colexg  (MATRIX *mat, int col1, int col2);

/* --- diagonal operations --- */
extern void    mat_diainit (MATRIX *mat, int unit);
extern double* mat_diaget  (const MATRIX *mat, double *vec);
extern void    mat_diaset  (MATRIX *mat, const double *vec);
extern void    mat_diaaddv (MATRIX *mat, double k, const double *vec);
extern void    mat_diamuls (MATRIX *mat, double k);
extern double  mat_diasum  (MATRIX *mat);
extern double  mat_diaprod (MATRIX *mat);
extern double  mat_track   (MATRIX *mat);

/* --- matrix/vector operations --- */
extern double* mat_mulmv   (double *res, MATRIX *mat,const double *vec);
extern double* mat_mulvm   (double *res, const double *vec,MATRIX *mat);
extern double  mat_mulvmv  (const MATRIX *mat, const double *vec);
extern double  mat_mulvdv  (const MATRIX *mat, const double *vec);

/* --- general matrix operations --- */
extern MATRIX* mat_transp  (MATRIX *res, const MATRIX *mat);
extern MATRIX* mat_muls    (MATRIX *res, const MATRIX *mat, double k);
extern MATRIX* mat_add     (MATRIX *res, const MATRIX *A,
                            double k,    const MATRIX *B);
extern MATRIX* mat_mul     (MATRIX *res, const MATRIX *A,
                                         const MATRIX *B);
extern MATRIX* mat_sub     (MATRIX *res, const MATRIX *mat,
                            int row, int col);
extern MATRIX* mat_subx    (MATRIX *res, const MATRIX *mat,
                            int *rowids, int *colids);

/* --- triangular matrix operations --- */
extern MATRIX* mat_tr2sym  (MATRIX *res, const MATRIX *mat, int src);
extern MATRIX* mat_trmuls  (MATRIX *res, const MATRIX *mat, int loc,
                            double k);
extern double* mat_trsubst (double *res, const MATRIX *mat, int loc,
                            const double *vec);
extern MATRIX* mat_trinv   (MATRIX *res, const MATRIX *mat, int loc);

/* --- Gauss-Jordan elimination functions --- */
extern double  mat_gjdet   (MATRIX *mat, int mode);
extern int     mat_gjinv   (MATRIX *res, const MATRIX *mat,
                            int mode, double *pdet);
extern int     mat_gjsol   (double **res, MATRIX *mat,
                            double *const*vec, int cnt,
                            int mode, double *pdet);

/* --- LU decomposition functions --- */
extern int     mat_ludecom (MATRIX *res, const MATRIX *mat);
extern double* mat_lusubst (double *res, const MATRIX *mat,
                            const double *vec);
extern double  mat_ludet   (const MATRIX *mat);
extern int     mat_luinv   (MATRIX *res, const MATRIX *mat);

/* --- Cholesky decomposition functions --- */
extern int     mat_chdecom (MATRIX *res, const MATRIX *mat);
extern double* mat_chsubst (double *res, const MATRIX *mat,
                            const double *vec);
extern double  mat_chdet   (const MATRIX *mat);
extern int     mat_chinv   (MATRIX *res, const MATRIX *mat);

/* --- eigenvalue and eigenvector functions --- */
extern int     mat_jacobi  (MATRIX *mat, int mode,
                            double *eival, double **eivec);
extern int     mat_3dred   (const MATRIX *mat, MATRIX *otm,
                            double *diag, double *subdiag);
extern int     mat_3dqli   (const double *diag, const double *subdiag,
                            int n, double *eival, double **eivec);
extern void    mat_bal     (MATRIX *res, const MATRIX *mat);
extern void    mat_heselm  (MATRIX *res, const MATRIX *mat);
extern void    mat_hesqr   (MATRIX *mat, int mode,
                            double *real, double *img);

/* --- (co)variance/correlation/regression operations --- */
extern MATRIX* mat_addvec  (MATRIX *mat, const double *vec, double wgt);
extern MATRIX* mat_addsv   (MATRIX *mat, const double *vec, double wgt);
extern MATRIX* mat_addmp   (MATRIX *mat, const double *vec, double wgt);
extern double  mat_weight  (const MATRIX *mat);
extern MATRIX* mat_mean    (MATRIX *res, const MATRIX *mat);
extern MATRIX* mat_var     (MATRIX *res, const MATRIX *mat, int mle);
extern MATRIX* mat_covar   (MATRIX *res, const MATRIX *mat, int mle);
extern MATRIX* mat_correl  (MATRIX *res, const MATRIX *mat);
extern double  mat_regress (MATRIX *res, double *rhs,const MATRIX *mat);

/* --- input/output functions --- */
extern void    mat_show    (const MATRIX *mat);
extern int     mat_write   (const MATRIX *mat, FILE *file,
                            const char *fmt, const char *sep);
#ifdef MAT_READ
extern int     mat_read    (MATRIX *mat, TFSCAN *tfscan, FILE *file);
extern MATRIX* mat_readx   (TFSCAN *tfscan, FILE *file,
                            int rowcnt, int colcnt);
#endif

/*----------------------------------------------------------------------
  Preprocessor Definitions
----------------------------------------------------------------------*/
#define vec_len(a,n)       sqrt(vec_sqrlen(a,n))
#define vec_dist(a,b,n)    sqrt(vec_sqrdist(a,b,n))
#define vec_mul(a,b,n)     vec_sprod(a,b,n)
#define vec_show(v,n)      vec_write(v, n, stdout, "% 9g", " \n")

/*--------------------------------------------------------------------*/
#define mat_issqr(m)       ((m)->rowcnt == (m)->colcnt)
#define mat_buf(m)         ((m)->buf)
#define mat_get(m,r,c)     ((m)->els[r][c])
#define mat_set(m,r,c,x)   ((m)->els[r][c] = (x))
#define mat_colcnt(m)      ((m)->colcnt)   
#define mat_rowcnt(m)      ((m)->rowcnt)   
#define mat_row(m,r)       ((m)->els[r])
#define mat_track(m)       mat_diasum(m)
#define mat_weight(m)      ((m)->weight)
#define mat_show(m)        mat_write(m, stdout, "% 9g", " \n")

#endif
