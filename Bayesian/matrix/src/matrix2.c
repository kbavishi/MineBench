/*----------------------------------------------------------------------
  File    : matrix2.c
  Contents: general vector and matrix management
            functions for matrix inversion and equation system solving
  Author  : Christian Borgelt
  History : 30.04.1999 file created
            10.05.1999 functions mat_gjinv and mat_gjdet completed
            11.05.1999 LU decomposition functions completed
            12.05.1999 Cholesky decomposition functions completed
            13.05.1999 vector and matrix read functions completed
            17.05.1999 parameter 'det' added to function mat_gjinv
            17.08.2001 VECTOR data type replaced by double*
            13.10.2001 covariance funcs. redesigned, mat_regress added
            23.10.2001 functions mat_addsv and mat_var added
            26.10.2001 equation system solution removed from mat_regress
            14.11.2003 bug in mat_covar (non-negative diagonal) fixed
----------------------------------------------------------------------*/
#include <stdio.h>
#include <limits.h>
#include <float.h>
#include <assert.h>
#include "matrix.h"
#ifdef STORAGE
#include "storage.h"
#endif

/*----------------------------------------------------------------------
  Preprocessor Definitions
----------------------------------------------------------------------*/
/* --- matrix flags --- */
#define ODDPERM    0x0002       /* odd row permutation (LU decomp.) */

/* --- functions --- */
#ifdef NDEBUG                   /* if to compile the release version */
#define DBGMSG(s)               /* suppress all debug messages */
#else                           /* if to compile the debug version */
#define DBGMSG(s)  fprintf(stderr, s)
#endif                          /* print messages to stderr */

/*----------------------------------------------------------------------
  Triangular Matrix Functions
----------------------------------------------------------------------*/

MATRIX* mat_tr2sym (MATRIX *res, const MATRIX *mat, int src)
{                               /* --- convert triang. to sym. matrix */
  int    row, col;              /* loop variables */
  double *d; const double *s;   /* to traverse the matrix rows */

  assert(res && mat             /* check the function arguments */
     && (mat->rowcnt == mat->colcnt)
     && (res->rowcnt == mat->rowcnt)
     && (res->colcnt == mat->colcnt));
  if (src == MAT_LOWER) {       /* if to use the lower triangle */
    if (res != mat) {           /* if result and source differ */
      for (row = res->rowcnt; --row >= 0; ) {
        s = mat->els[row] +row +1;
        d = res->els[row] +row +1;
        for (col = row+1; --col >= 0; ) *--d = *--s;
      }                         /* copy the lower triangle of the */
    }                           /* source to the destination */
    for (row = res->rowcnt-1; --row >= 0; ) {
      for (d = res->els[row] +(col = res->colcnt); --col > row; )
        *--d = res->els[col][row];
    } }                         /* copy lower to upper triangle */
  else {                        /* if to use the upper triangle */
    if (res != mat) {           /* if result and source differ */
      for (row = res->rowcnt; --row >= 0; ) {
        s = mat->els[row] +res->rowcnt;
        d = res->els[row] +res->rowcnt;
        for (col = res->rowcnt; --col >= row; ) *--d = *--s;
      }                         /* copy the upper triangle of the */
    }                           /* source to the destination */
    for (row = res->rowcnt; --row > 0; ) {
      for (d = res->els[row] +(col = row); --col >= 0; )
        *--d = res->els[col][row];
    }                           /* copy upper to lower triangle */
  }
  return res;                   /* return the result matrix */
}  /* mat_tr2sym() */

/*--------------------------------------------------------------------*/

MATRIX* mat_trmuls (MATRIX *res, const MATRIX *mat, int loc, double k)
{                               /* --- multiply matrix with scalar */
  int    row, col;              /* loop variables */
  int    beg, end;              /* range of column indices */
  double *d; const double *s;   /* to traverse the matrix rows */

  assert(res && mat             /* check the function arguments */
      && (mat->rowcnt == mat->colcnt)
      && (res->rowcnt == mat->rowcnt)
      && (res->colcnt == mat->colcnt));
  beg = res->colcnt; end = 0;   /* initialize the column range */
  for (row = res->rowcnt; --row >= 0; ) {
    d = res->els[row]; s = mat->els[row];
    if (loc == MAT_UPPER) end = row;
    else                  beg = row+1;
    for (col = beg; --col >= end; )
      d[col] = s[col] *k;       /* multiply the matrix rows */
  }                             /* column by column with the factor */
  return res;                   /* return the result matrix */
}  /* mat_trmuls() */

/*--------------------------------------------------------------------*/

double* mat_trsubst (double *res, const MATRIX *mat, int loc,
                     const double *vec)
{                               /* --- forward/backward substitution */
  int          row, col;        /* loop variables */
  const double *s;              /* to traverse the matrix rows */
  double       t;               /* temporary buffer */

  assert(res && mat             /* check the function arguments */
     && (mat->rowcnt == mat->colcnt));
  if (loc == MAT_LOWER) {       /* if matrix is lower triangular */
    res[0] = vec[0] /mat->els[0][0];
    for (row = 0; ++row < mat->rowcnt; ) {
      s = mat->els[row]; t = vec[row];
      for (col = row; --col >= 0; )
        t -= s[col] *res[col];  /* traverse the matrix rows and */
      res[row] = t/s[row];      /* do the backward substitution */
    } }
  else {                        /* if matrix is upper triangular */
    for (row = mat->rowcnt; --row >= 0; ) {
      s = mat->els[row]; t = vec[row];
      for (col = mat->colcnt; --col > row; )
        t -= s[col] *res[col];  /* traverse the matrix rows */
      res[row] = t/s[row];      /* do the forward substitution */
    }
  }
  return res;                   /* return the result vector */
}  /* mat_trsubst() */

/*--------------------------------------------------------------------*/

MATRIX* mat_trinv (MATRIX *res, const MATRIX *mat, int loc)
{                               /* --- invert a triangular matrix */
  int    row, col, i;           /* loop variables */
  double *d; const double *s;   /* to traverse the matrix rows */
  double t;                     /* temporary buffer */

  assert(res && mat             /* check the function arguments */
     && (mat->rowcnt == mat->colcnt)
     && (res->rowcnt == mat->rowcnt)
     && (res->colcnt == mat->colcnt));
  if (loc == MAT_LOWER) {       /* if matrix is lower triangular */
    for (i = res->colcnt; --i >= 0; ) {
      d    = res->els[i];       /* traverse the unit vectors */
      d[i] = 1 /mat->els[i][i]; /* process the diagonal element */
      for (row = i; ++row < res->rowcnt; ) {
        s = mat->els[row]; t = 0;
        for (col = row; --col >= i; ) t -= d[col] *s[col];
        d[row] = t *res->els[row][row];
      }                         /* do forward substitution */
    }                           /* with lower triangular matrix */
    for (row = res->rowcnt; --row >= 0; ) {
      for (d = res->els[row] +(col = row); --col >= 0; )
        *--d = res->els[col][row];
      for (d = res->els[row] +(col = mat->colcnt); --col > row; )
        *--d = 0;               /* copy upper to lower triangle */
    } }                         /* and clear the upper triangle */
  else {                        /* if matrix is upper triangular */
    for (i = 0; i < res->colcnt; i++) {
      d    = res->els[i];       /* traverse the unit vectors */
      d[i] = 1 /mat->els[i][i]; /* process the diagonal element */
      for (row = i; --row >= 0; ) {
        s = mat->els[row]; t = 0;
        for (col = row; ++col <= i; ) t -= d[col] *s[col];
        d[row] = t *res->els[row][row];
      }                         /* do backward substitution */
    }                           /* with upper triangular matrix */
    for (row = 0; row < res->rowcnt; row++) {
      for (d = res->els[row] +(col = res->colcnt); --col > row; )
        *--d = res->els[col][row];
      for (--d; --col >= 0; ) *--d = 0;
    }                           /* copy lower to upper triangle */
  }                             /* and clear the lower triangle */
  return res;                   /* return the inverted matrix */
}  /* mat_trinv() */

/*----------------------------------------------------------------------
  Gauss-Jordan Elimination Functions

  Reference:
  W.H. Press, S.A. Teukolsky, W.T. Vetterling, and B.P. Flannery.
  Numerical Recipes in C --- The Art of Scientific Computing
  (2nd edition), pp. 36--41.
  Cambridge University Press, Cambridge, United Kingdom 1992
----------------------------------------------------------------------*/

double mat_gjdet (MATRIX *mat, int mode)
{                               /* --- Gauss-Jordan determinant */
  MATRIX *dup = NULL;           /* duplicate of input matrix */
  int    row, col, i;           /* loop variables, index buffers */
  int    pivrow, pivcol = 0;    /* row and column of the pivot elem. */
  double pivot;                 /* pivot element */
  double *p, *r;                /* pivot element row and current row */
  int    *flags;                /* matrix row flags */
  double det = 1;               /* determinant of the matrix */
  double t;                     /* temporary buffer */

  assert(mat                    /* check the function arguments */
     && (mat->rowcnt == mat->colcnt));
  if (mat->rowcnt <= 1)         /* if it is a 1x1 matrix, the only */
    return mat->els[0][0];      /* matrix element is the determinant */
  if (!(mode & MAT_NOCOPY)) {   /* if not to work in place, */
    mat = dup = mat_dup(mat,0); /* duplicate the matrix */
    if (!dup) return -DBL_MAX;  /* and check for success */
  }
  for (flags = mat->map +(row = mat->rowcnt); --row >= 0; )
    *--flags = 0;               /* clear the row/column flags */
  for (col = mat->colcnt; --col >= 0; ) {
    if (mode & MAT_PARTPIV) {   /* if to do only partial pivoting, */
      pivot = fabs(mat->els[col][col]); /* init. the pivot element */
      for (row = pivrow = pivcol = col; --row >= 0; ) {
        t = fabs(mat->els[row][col]);  /* traverse the matrix rows */
        if (t > pivot) { pivot = t; pivrow = row; }
      } }                       /* find the column pivot element */
    else {                      /* if to do full pivoting, */
      pivot = 0; pivrow = 0;    /* initialize the pivot element */
      for (row = mat->rowcnt; --row >= 0; ) {
        if (flags[row]) continue;       /* traverse the matrix rows */
        r = mat->els[row];      /* that have not been processed yet */
        for (i = mat->colcnt; --i >= 0; ) {
          if (flags[i]) continue;    /* traverse the matrix columns */
          t = fabs(r[i]);       /* that have not been processed yet */
          if (t > pivot) { pivot = t; pivrow = row; pivcol = i; }
        }                       /* find the pivot element and */
      }                         /* note its row and column */
    }
    if (pivot == 0)             /* if the matrix is singular, */
      return 0;                 /* its determinant is zero */
    flags[pivcol] = -1;         /* mark the pivot row/column */
    p = mat->els[pivrow];       /* and get the pivot row */
    if (pivrow != pivcol) {     /* if the pivot is not on the diag. */
      mat->els[pivrow] = mat->els[pivcol];
      mat->els[pivcol] = p;     /* swap the pivot row into place */
      det = -det;               /* and negate the determinant */
    }                           /* to take the exchange into account */
    det *= pivot = p[pivcol];   /* multiply in the pivot element */
    for (row = mat->rowcnt; --row >= 0; ) {
      if (flags[row]) continue; /* traverse the matrix rows, */
      r = mat->els[row];        /* but skip the processed ones */
      t = r[pivcol] /pivot;     /* compute the reduction factor */
      for (i = mat->colcnt; --i >= 0; )
        if (!flags[i]) r[i] -= p[i] *t;
    }                           /* traverse and reduce the matrix row */
  }                             /* by sub. a mult. of the pivot row */
  if (dup) mat_delete(dup);     /* delete the created duplicate */
  return det;                   /* return the computed determinant */
}  /* mat_gjdet() */

/*--------------------------------------------------------------------*/

int mat_gjinv (MATRIX *res, const MATRIX *mat, int mode, double *pdet)
{                               /* --- Gauss-Jordan matrix inversion */
  int    row, col, i, k;        /* loop variables, index buffers */
  int    *map1, *map2;          /* exchange maps for bookkeeping */
  int    pivrow, pivcol = 0;    /* row and column of the pivot elem. */
  double pivot;                 /* pivot element */
  double *p, *r;                /* pivot element row and current row */
  double det;                   /* (reciprocal of) determinant */
  double t;                     /* temporary buffer */

  assert(mat && res && res->map /* check the function arguments */
     && (mat->rowcnt == mat->colcnt)
     && (res->rowcnt == mat->rowcnt)
     && (res->colcnt == mat->colcnt));
  if (mat->rowcnt <= 1) {       /* if this is a 1x1 matrix */
    t = mat->els[0][0];         /* get the top left matrix element */
    if (t == 0) {               /* if the matrix is singular, abort */
      DBGMSG("mat_gjinv: singular matrix\n"); return -1; }
    res->els[0][0] = 1/t;       /* invert the only matrix element */
    return 0;                   /* and abort the function */
  }
  if (res != mat)               /* if the result differs from the */
    mat_copy(res, mat, 0);      /* source matrix, copy the source */
  map1 = res->map;              /* get the exchange maps */
  map2 = map1 +res->rowcnt;     /* for bookkeeping */
  for (row = res->rowcnt; --row >= 0; )
    map1[row] = 0;              /* clear the row/column flags */
  det = 1;                      /* initialize the determinant */
  for (col = res->colcnt; --col >= 0; ) {
    if (mode & MAT_PARTPIV) {   /* if to do only partial pivoting, */
      pivot = fabs(res->els[col][col]); /* init. the pivot element */
      for (row = pivrow = pivcol = col; --row >= 0; ) {
        t = fabs(res->els[row][col]);  /* traverse the matrix rows */
        if (t > pivot) { pivot = t; pivrow = row; }
      } }                       /* find the column pivot element */
    else {                      /* if to do full pivoting, */
      pivot = 0; pivrow = 0;    /* initialize the pivot element */
      for (row = res->rowcnt; --row >= 0; ) {
        if (map1[row] < 0) continue;    /* traverse the matrix rows */
        r = res->els[row];      /* that have not been processed yet */
        for (i = res->colcnt; --i >= 0; ) {
          if (map1[i] < 0) continue; /* traverse the matrix columns */
          t = fabs(r[i]);       /* that have not been processed yet */
          if (t > pivot) { pivot = t; pivrow = row; pivcol = i; }
        }                       /* find the pivot element and */
      }                         /* note its row and column */
    }
    if (pivot == 0) {           /* if the matrix is singular, abort */
      DBGMSG("mat_gjinv: singular matrix\n"); return -1; }
    map1[pivcol] |= INT_MIN;    /* mark the pivot row/column */
    map1[col]    |= pivrow;     /* note the row and the column */
    map2[col]     = pivcol;     /* for later unscrambling */
    p = res->els[pivrow];       /* get the pivot row */
    if (pivrow != pivcol) {     /* if rows have to be exchanged */
      res->els[pivrow] = res->els[pivcol];
      res->els[pivcol] = p;     /* swap the pivot row into place and */
      det = -det;               /* update the sign of the determinant */
    }                          
    det *= pivot = 1/p[pivcol]; /* update the determinant */
    p[pivcol] = 1;              /* set the diagonal element */
    for (i = res->colcnt; --i >= 0; )
      p[i] *= pivot;            /* divide the pivot row by the pivot */
    for (row = res->rowcnt; --row >= 0; ) {
      if (row == pivcol)        /* traverse the matrix rows, */
        continue;               /* but skip the current (pivot) row */
      r = res->els[row];        /* get the element in the pivot */
      t = r[pivcol]; r[pivcol] = 0;      /* column and clear it */
      for (i = res->colcnt; --i >= 0; )
        r[i] -= p[i] *t;        /* reduce the matrix row by subtract. */
    }                           /* a multiple of the pivot row */
  }
  for (col = 0; col < res->colcnt; col++) {
    i = map1[col] & ~INT_MIN;   /* traverse the exchange maps */
    k = map2[col];              /* if no exchange is necessary, */
    if (i == k) continue;       /* skip the map entries */
    for (row = res->rowcnt; --row >= 0; ) {
      r = res->els[row];        /* traverse the matrix rows */
      t = r[i]; r[i] = r[k]; r[k] = t;
    }                           /* swap the columns into place */
  }                             /* reversing the exchange order */
  if (pdet) *pdet = det;        /* set the determinant of the inverse */
  return 0;                     /* return 'ok' */
}  /* mat_gjinv() */

/*--------------------------------------------------------------------*/

int mat_gjsol (double **res, MATRIX *mat,
               double *const*vec, int cnt, int mode, double *pdet)
{                               /* --- solve linear equation systems */
  MATRIX *dup = NULL;           /* duplicate of input matrix */
  int    row, col, i, k;        /* loop variables, index buffers */
  int    *map1, *map2;          /* exchange maps for bookkeeping */
  int    pivrow, pivcol = 0;    /* row and column of the pivot elem. */
  double pivot;                 /* pivot element */
  double *p, *r;                /* pivot element row and current row */
  double *v;                    /* to traverse the rhs vectors */
  double det;                   /* (reciprocal of) determinant */
  double t;                     /* temporary buffer */

  assert(mat && vec && res      /* check the function arguments */
     && (mat->rowcnt == mat->colcnt) && (cnt >= 0));
  if (mat->rowcnt <= 1) {       /* if the matrix has size 1x1 */
    t = mat->els[0][0];         /* get the top left matrix element */
    if (t == 0) {               /* if the matrix is singular, abort */
      DBGMSG("mat_gjsol: singular matrix\n"); return -1; }
    for (i = cnt; --i >= 0;)    /* otherwise divide all numbers */
      res[i][0] /= t;           /* by the only matrix element */
    if (mode & MAT_INVERSE)     /* if to compute the inverse matrix, */
      mat->els[0][0] = 1/t;     /* invert the only matrix element */
    return 0;                   /* and abort the function */
  }                             /* if this is a 2x2 matrix */
  if (!(mode & MAT_NOCOPY)) {   /* if not to work in place, */
    mat = dup = mat_dup(mat,0); /* duplicate the matrix */
    if (!dup) return -1;        /* and check for success */
  }
  if ((double *const*)res != vec) {  /* if source and dest. differ, */
    for (i = cnt; --i >= 0; )   /* copy the right hand side vectors */
      vec_copy(res[i], vec[i], mat->rowcnt);
  }
  map1 = mat->map;              /* get the exchange maps */
  map2 = map1 +mat->rowcnt;     /* for bookkeeping */
  for (row = mat->rowcnt; --row >= 0; )
    map1[row] = 0;              /* clear the row/column flags */
  det = 1;                      /* initialize the determinant */
  for (col = mat->colcnt; --col >= 0; ) {
    if (mode & MAT_PARTPIV) {   /* if to do only partial pivoting, */
      pivot = fabs(mat->els[col][col]); /* init. the pivot element */
      for (row = pivrow = pivcol = col; --row >= 0; ) {
        t = fabs(mat->els[row][col]);  /* traverse the matrix rows */
        if (t > pivot) { pivot = t; pivrow = row; }
      } }                       /* find the column pivot element */
    else {                      /* if to do full pivoting, */
      pivot = 0; pivrow = 0;    /* initialize the pivot element */
      for (row = mat->rowcnt; --row >= 0; ) {
        if (map1[row] & INT_MIN)
          continue;             /* traverse the matrix rows */
        r = mat->els[row];      /* that have not been processed yet */
        for (i = mat->colcnt; --i >= 0; ) {
          if (map1[i] & INT_MIN)
            continue;           /* traverse the matrix columns */
          t = fabs(r[i]);       /* that have not been processed yet */
          if (t > pivot) { pivot = t; pivrow = row; pivcol = i; }
        }                       /* find the pivot element and */
      }                         /* note its row and column */
    }
    if (pivot == 0) {           /* if the matrix is singular, abort */
      DBGMSG("mat_gjsol: singular matrix\n"); return -1; }
    map1[pivcol] |= INT_MIN;    /* mark the pivot row/column */
    map1[col]    |= pivrow;     /* note the row and the column */
    map2[col]     = pivcol;     /* for later unscrambling */
    p = mat->els[pivrow];       /* get the pivot row */
    if (pivrow != pivcol) {     /* if rows have to be exchanged */
      mat->els[pivrow] = mat->els[pivcol];
      mat->els[pivcol] = p;     /* swap the pivot row into place and */
      det = -det;               /* update the sign of the determinant */
      for (i = cnt; --i >= 0; ) {
        v = res[i]; t = v[pivrow];
        v[pivrow] = v[pivcol]; v[pivcol] = t;
      }                         /* swap vector elements in the */
    }                           /* same way as the matrix rows */
    det *= pivot = 1/p[pivcol]; /* update the determinant */
    p[pivcol] = 1;              /* set the diagonal element */
    for (i = mat->colcnt; --i >= 0; )
      p[i] *= pivot;            /* divide the pivot row by the pivot */
    for (i = cnt; --i >= 0; )   /* do the same with the rhs vectors */
      res[i][pivcol] *= pivot;
    for (row = mat->rowcnt; --row >= 0; ) {
      if (row == pivcol)        /* traverse the matrix rows, */
        continue;               /* but skip the current (pivot) row */
      r = mat->els[row];        /* get the element in the pivot */
      t = r[pivcol]; r[pivcol] = 0;      /* column and clear it */
      for (i = mat->colcnt; --i >= 0; )
        r[i] -= p[i] *t;        /* reduce the matrix row */
      for (i = cnt; --i >= 0;){ /* and the rhs vectors */
        v = res[i]; v[row] -= v[pivcol] *t; }
    }                           /* i.e., subtract from the row */
  }                             /* a multiple of the pivot row */
  if (mode & MAT_INVERSE) {     /* if to compute the inverse matrix */
    for (col = 0; col < mat->colcnt; col++) {
      i = map1[col] & ~INT_MIN; /* traverse the exchange maps */
      k = map2[col];            /* if no exchange is necessary, */
      if (i == k) continue;     /* skip the map entries */
      for (row = mat->rowcnt; --row >= 0; ) {
        r  = mat->els[row];     /* traverse the matrix rows */
        t = r[i]; r[i] = r[k]; r[k] = t;
      }                         /* swap the columns into place */
    }                           /* reversing the exchange order */
  }                             /* (unscramble the matrix columns) */
  if (pdet) *pdet = det;        /* set the determinant of the inverse */
  if (dup) mat_delete(dup);     /* delete the created duplicate */
  return 0;                     /* return 'ok' */
}  /* mat_gjsol() */

/*----------------------------------------------------------------------
  LU Decomposition Functions

  Reference:
  W.H. Press, S.A. Teukolsky, W.T. Vetterling, and B.P. Flannery.
  Numerical Recipes in C --- The Art of Scientific Computing
  (2nd edition), pp. 43--49.
  Cambridge University Press, Cambridge, United Kingdom 1992
----------------------------------------------------------------------*/

int mat_ludecom (MATRIX *res, const MATRIX *mat)
{                               /* --- LU decompose a matrix */
  int    row, col, i;           /* loop variables, index buffers */
  int    pivrow;                /* row of the pivot element */
  double pivot;                 /* pivot element/row maximum */
  double *r, *c;                /* matrix row/column buffer */
  double t;                     /* temporary buffer */

  assert(mat && res && res->map /* check the function arguments */
     && (res->rowcnt == mat->rowcnt)
     && (res->colcnt == mat->colcnt));
  if (res != mat)               /* if the result differs from the */
    mat_copy(res, mat, 0);      /* source matrix, copy the source */
  for (row = res->rowcnt; --row >= 0; )
    res->map[row] = row;        /* initialize the matrix row map and */
  res->flags &= ~ODDPERM;       /* clear the permutation parity flag */
  c = res->buf;                 /* get the column buffer */
  for (col = 0; col < res->colcnt; col++) {
    for (row = 0; row <= col; row++) {
      r = res->els[row]; t = r[col];
      for (i = row; --i >= 0; ) t -= c[i] *r[i];
      c[row] = r[col] = t;      /* compute a column of the */
    }                           /* upper triangular matrix U */
    pivot  = fabs(c[col]);      /* initialize the pivot element */
    pivrow = col;               /* and its row index */
    for (row = res->rowcnt; --row > col; ) {
      r = res->els[row]; t = r[col];
      for (i = col; --i >= 0; ) t -= c[i] *r[i];       
      r[col] = t;               /* compute a column of the */
      t = fabs(t);              /* lower triangular matrix L */
      if (t > pivot) { pivot = t; pivrow = row; }
    }                           /* update the pivot element */
    if (pivot == 0) {           /* if the matrix is singular, abort */
      DBGMSG("mat_ludecom: singular matrix\n"); return -1; }
    r = res->els[pivrow];       /* get the row of the pivot element */
    if (pivrow != col) {        /* if rows need to be exchanged */
      res->els[pivrow] = res->els[col];
      res->els[col]    = r;     /* exchange current and pivot row */
      i                = res->map[pivrow];
      res->map[pivrow] = res->map[col];
      res->map[col]    = i;     /* exchange the row map entries */
      res->flags ^= ODDPERM;    /* and toggle the parity flag */
    }                           /* of the row permutation */
    t = 1/r[col];               /* compute the pivot factor */
    for (row = res->rowcnt; --row > col; )
      res->els[row][col] *= t;  /* divide the column of the lower */
  }                             /* triangular matrix L by the pivot */
  return 0;                     /* return 'ok' */
}  /* mat_ludecom() */

/*--------------------------------------------------------------------*/

double* mat_lusubst (double *res, const MATRIX *mat, const double *vec)
{                               /* --- fw/bw subst. for LU decomp. */
  int          row, col, i;     /* loop variables, first nonzero el. */
  const double *r;              /* current matrix row */
  double       *v;              /* copied and mapped vector */
  double       t;               /* temporary buffer */

  assert(res && vec && mat      /* check the function arguments */
      && mat->map && (mat->rowcnt == mat->colcnt));
  for (v = mat->buf +(row = i = mat->rowcnt); --row >= 0; ) {
    *--v = vec[mat->map[row]];
    if (*v != 0) i = row;       /* copy and map the input vector and */
  }                             /* find the first nonzero element */
  for (row = i; ++row < mat->rowcnt; ) {
    r = mat->els[row]; t = v[row];
    for (col = row; --col >= i; ) t -= v[col] *r[col];
    v[row] = t;                 /* do forward substitution with */
  }                             /* the lower triangular matrix L */
  for (row = mat->rowcnt; --row >= 0; ) {
    r = mat->els[row]; t = v[row];
    for (col = mat->colcnt; --col > row; ) t -= res[col] *r[col];
    res[row] = t /r[row];       /* do backward substitution with */
  }                             /* the upper triangular matrix U */
  return res;                   /* return the result vector */
}  /* mat_lusubst() */

/*--------------------------------------------------------------------*/

double mat_ludet (const MATRIX *mat)
{                               /* --- compute det. from LU decomp. */
  int    i;                     /* loop variable */
  double det;                   /* determinant of the matrix */

  assert(mat && mat->map        /* check the functions argument */
     && (mat->rowcnt == mat->colcnt));
  det = (mat->flags & ODDPERM)  /* get the initial value depending on */
      ? -1 : +1;                /* the parity of the row permutation */
  for (i = mat->rowcnt; --i >= 0; )
    det *= mat->els[i][i];      /* multiply the diagonal elements */
  return det;                   /* and return the result */
}  /* mat_ludet() */

/*--------------------------------------------------------------------*/

int mat_luinv (MATRIX *res, const MATRIX *mat)
{                               /* --- invert matrix from LU decomp. */
  MATRIX       *dup = NULL;     /* duplicate of LU decomposition */
  int          row, col, i, k;  /* loop variables, index buffer */
  const double *p;              /* current matrix row */
  double       *v;              /* copied and mapped vector */
  double       t;               /* temporary buffer */

  assert(mat && res && mat->map /* check the function arguments */
     && (mat->rowcnt == mat->colcnt)
     && (res->rowcnt == mat->rowcnt)
     && (res->colcnt == mat->colcnt));
  if (res == mat) {             /* if the result matrix is identical */
    mat = dup = mat_dup(mat,0); /* to the LU decomposition matrix, */
    if (!dup) return -1;        /* duplicate the LU decomp. matrix */
  }                             /* to make space for the inverse */
  v = res->buf;                 /* traverse the matrix columns */
  for (i = mat->colcnt; --i >= 0; ) {
    for (v += (row = i), *v = 1; --row >= 0; )
      *--v = 0;                 /* build the next unit vector */
    for (row = i; ++row < mat->rowcnt; ) {
      p = mat->els[row]; t = 0;
      for (col = row; --col >= i; ) t -= v[col] *p[col];
      v[row] = t;               /* traverse the rows of the matrix L */
    }                           /* and do the forward substitution */
    k = mat->map[i];            /* get the destination column */
    for (row = mat->rowcnt; --row >= 0; ) {
      p = mat->els[row]; t = v[row];
      for (col = mat->colcnt; --col > row; ) t -= v[col] *p[col];
      res->els[row][k] = v[row] = t /p[row];
    }                           /* traverse the rows of the matrix U */
  }                             /* and do the backward substitution */
  if (dup) mat_delete(dup);     /* delete the created duplicate */
  return 0;                     /* return 'ok' */
}  /* mat_luinv() */

/*----------------------------------------------------------------------
  Cholesky Decomposition Functions
  (for symmetric positive definite matrices only)

  Reference:
  W.H. Press, S.A. Teukolsky, W.T. Vetterling, and B.P. Flannery.
  Numerical Recipes in C --- The Art of Scientific Computing
  (2nd edition), pp. 96--98.
  Cambridge University Press, Cambridge, United Kingdom 1992
----------------------------------------------------------------------*/

int mat_chdecom (MATRIX *res, const MATRIX *mat)
{                               /* --- Cholesky decompose a matrix */
  int          row, col, i;     /* loop variables */
  const double *s;              /* to traverse the source matrix rows */
  double       *r1, *r2;        /* to traverse the result matrix rows */
  double       t, q;            /* temporary buffers */

  assert(mat && res             /* check the function arguments */
     && (res->rowcnt == mat->rowcnt)
     && (res->colcnt == mat->colcnt));
  for (row = 0; row < mat->rowcnt; row++) {
    s  = mat->els[row]; t = s[row];    /* traverse the matrix rows */
    r1 = res->els[row];         /* compute square of diag. element */
    for (i = row; --i >= 0; ) t -= r1[i] *r1[i];
    if (t <= 0) {               /* if not positive definite, abort */
      DBGMSG("mat_chdecom: matrix not positive definite\n"); return -1;}
    r1[row] = t = sqrt(t);      /* set the diagonal element */
    res->buf[row] = q = (t > 0) ? 1/t : DBL_MAX;
    for (col = row; ++col < mat->colcnt; ) {
      r2 = res->els[col]; t = s[col];
      for (i = row; --i >= 0; ) t -= r1[i] *r2[i];
      r2[row] = t*q;            /* compute the off-diagonal elements */
    }                           /* (the lower triangular matrix) and */
  }                             /* store them in the result matrix */
  return 0;                     /* return 'ok' */
}  /* mat_chdecom() */          /* (only lower triangle is computed) */

/*--------------------------------------------------------------------*/

double* mat_chsubst (double *res, const MATRIX *mat, const double *vec)
{                               /* --- fw/bw subst. for Cholesky dec. */
  int    row, col;              /* loop variables */
  double *b;                    /* reciprocals of diagonal elements */
  double *s;                    /* to traverse the matrix rows */
  double t;                     /* temporary buffer */

  assert(res && vec && mat      /* check the function arguments */
     && (mat->rowcnt == mat->colcnt));
  b = mat->buf;                 /* get reciprocals of diag. elements */
  for (row = 0; row < mat->rowcnt; row++) {
    s = mat->els[row]; t = vec[row];
    for (col = row; --col >= 0; )
      t -= res[col] *s[col];    /* do forward substitution with */
    res[row] = t *b[row];       /* the lower triangular matrix L */
  }
  for (row = mat->rowcnt; --row >= 0; ) {
    s = mat->els[row]; t = res[row];
    for (col = row; ++col < mat->colcnt; )
      t -= res[col] *mat->els[col][row];
    res[row] = t *b[row];       /* do backward substitution with */
  }                             /* the upper triangular matrix L^T */
  return res;                   /* (but access only lower triangle) */
}  /* mat_chsubst() */          /* and return the result vector */

/*--------------------------------------------------------------------*/

double mat_chdet (const MATRIX *mat)
{                               /* --- compute det. from Cholesky d. */
  int    i;                     /* loop variable */
  double det;                   /* determinant of the matrix */

  assert(mat                    /* check the function argument */
     && (mat->rowcnt == mat->colcnt));
  det = mat->els[0][0];         /* multiply diagonal elements */
  for (i = mat->rowcnt; --i > 0; ) det *= mat->els[i][i];
  return det *det;              /* return the square of the product, */
}  /* mat_chdet() */            /* which is the determinant */

/*--------------------------------------------------------------------*/

int mat_chinv (MATRIX *res, const MATRIX *mat)
{                               /* --- invert matrix from Cholesky d. */
  int          row, col, i;     /* loop variables */
  const double *b;              /* reciprocals of diagonal elements */
  const double *s;              /* to traverse the source matrix rows */
  double       *d;              /* to traverse the dest.  matrix rows */
  double       t;               /* temporary buffer */

  assert(res && mat             /* check the function arguments */
     && (mat->rowcnt == mat->colcnt)
     && (mat->rowcnt == res->rowcnt)
     && (mat->colcnt == res->colcnt));
  b = mat->buf;                 /* get reciprocals of diag. elements */
  for (i = mat->colcnt; --i >= 0; ) {
    d = res->els[i];            /* traverse the unit vectors */
    d[i] = b[i];                /* and compute their originals */
    for (row = i; ++row < mat->rowcnt; ) {
      s = mat->els[row];        /* traverse the matrix rows */
      for (t = 0, col = row; --col >= i; )
        t -= d[col] *s[col];    /* do forward substitution with */
      d[row] = t *b[row];       /* the lower triangular matrix L */
    }
    for (row = mat->rowcnt; --row >= i; ) {
      t = d[row];               /* traverse the matrix columns */
      for (col = mat->colcnt; --col > row; )
        t -= d[col] *mat->els[col][row];
      d[row] = t *b[row];       /* do backward substitution with */
    }                           /* the upper triangular matrix L^T */
  }                             /* (but access only lower triangle) */
  return 0;                     /* return 'ok' */
}  /* mat_chinv() */            /* (only upper triangle is computed) */

/*----------------------------------------------------------------------
  (Co)Variance/Correlation Matrix Functions
----------------------------------------------------------------------*/

MATRIX* mat_addvec (MATRIX *mat, const double *vec, double wgt)
{                               /* --- add a vector to a matrix */
  int    row;                   /* loop variable */
  double *b;                    /* to traverse the buffer */
  double t;                     /* temporary buffer */

  assert(mat && vec             /* check the function arguments */
     && (mat->rowcnt == mat->colcnt) && (wgt >= 0));
  mat->weight += wgt;           /* sum the vector weight */
  for (b = mat->buf +(row = mat->rowcnt); --row >= 0; )
    *--b += t = vec[row] *wgt;  /* sum the weighted vector */
  return mat;                   /* return the matrix worked on */
}  /* mat_addvec() */

/*--------------------------------------------------------------------*/

MATRIX* mat_addsv (MATRIX *mat, const double *vec, double wgt)
{                               /* --- add squared vector to a matrix */
  int    row;                   /* loop variable */
  double *b;                    /* to traverse the buffer */
  double t;                     /* temporary buffer */

  assert(mat && vec             /* check the function arguments */
     && (mat->rowcnt == mat->colcnt) && (wgt >= 0));
  mat->weight += wgt;           /* sum the vector weight */
  for (b = mat->buf +(row = mat->rowcnt); --row >= 0; ) {
    *--b += t = vec[row] *wgt;  /* sum the weighted vector */
    mat->els[row][row] += t *vec[row];
  }                             /* of the vector with itself */
  return mat;                   /* return the matrix worked on */
}  /* mat_addsv() */

/*--------------------------------------------------------------------*/

MATRIX* mat_addmp (MATRIX *mat, const double *vec, double wgt)
{                               /* --- add matrix product to a matrix */
  int    row, col;              /* loop variables */
  double *b, *d;                /* to traverse the buffer/matrix rows */
  double t;                     /* temporary buffer */

  assert(mat && vec             /* check the function arguments */
     && (mat->rowcnt == mat->colcnt) && (wgt >= 0));
  mat->weight += wgt;           /* sum the vector weight */
  for (b = mat->buf +(row = mat->rowcnt); --row >= 0; ) {
    *--b += t = vec[row] *wgt;  /* sum the weighted vector */
    for (d = mat->els[row] +(col = mat->colcnt); --col >= row; )
      *--d += vec[col] *t;      /* sum the weighted matrix product */
  }                             /* of the vector with itself */
  return mat;                   /* return the matrix worked on */
}  /* mat_addmp() */

/*--------------------------------------------------------------------*/

MATRIX* mat_mean (MATRIX *res, const MATRIX *mat)
{                               /* --- mean value computation */
  int    i;                     /* loop variable */
  double *b;                    /* to traverse the buffer */
  const double *s;              /* dito, for the source matrix */
  double t, w;                  /* temporary buffers */

  assert(mat && res             /* check the function arguments */
     && (mat->rowcnt == mat->colcnt)
     && (res->rowcnt == res->colcnt)
     && (mat->rowcnt == res->colcnt));
  w = mat->weight;              /* get the weight sum */
  t = (w > 0) ? 1/w : 0;        /* and its reciprocal */
  b = res->buf +res->rowcnt;    /* traverse the sum of the weighted */
  s = mat->buf +res->rowcnt;    /* vectors (stored in the buffer) */
  for (i = mat->rowcnt; --i >= 0; )
    *--b = *--s *t;             /* compute average/expected values */
  return res;                   /* and return the result matrix */
}  /* mat_mean() */

/*--------------------------------------------------------------------*/

MATRIX* mat_var (MATRIX *res, const MATRIX *mat, int mle)
{                               /* --- variance computation */
  int    row;                   /* loop variable */
  double *b;                    /* to traverse the buffer */
  const double *s;              /* dito, for the source matrix */
  double t, w;                  /* temporary buffers */

  assert(mat && res             /* check the function arguments */
     && (mat->rowcnt == mat->colcnt)
     && (res->rowcnt == res->colcnt)
     && (mat->rowcnt == res->colcnt));
  w = mat->weight;              /* get the weight sum */
  t = (w > 0) ? 1/w : 0;        /* and its reciprocal */
  b = res->buf +res->rowcnt;    /* traverse the sum of the weighted */
  s = mat->buf +res->rowcnt;    /* vectors (stored in the buffer) */
  for (row = mat->rowcnt; --row >= 0; )
    *--b = *--s *t;             /* compute average/expected values */
  t = (mle)   ? w : w-1;        /* compute the factor for */
  t = (t > 0) ? 1/t : 0;        /* the variance estimation */
  for (row = res->rowcnt; --row >= 0; )
    res->els[row][row] = t *(mat->els[row][row] -b[row] *b[row] *w);
  return res;                   /* compute the variances and */
}  /* mat_var() */              /* return the result matrix */

/*--------------------------------------------------------------------*/

MATRIX* mat_covar (MATRIX *res, const MATRIX *mat, int mle)
{                               /* --- covariance matrix computation */
  int    row, col;              /* loop variables */
  double *b, *d;                /* to traverse the buffer/matrix rows */
  const double *s;              /* dito, for the source matrix */
  double r, t, w;               /* temporary buffers */

  assert(mat && res             /* check the function arguments */
     && (mat->rowcnt == mat->colcnt)
     && (res->rowcnt == res->colcnt)
     && (mat->rowcnt == res->colcnt));
  w = mat->weight;              /* get the weight sum */
  t = (w > 0) ? 1/w : 0;        /* and its reciprocal */
  b = res->buf +res->rowcnt;    /* traverse the sum of the weighted */
  s = mat->buf +res->rowcnt;    /* vectors (stored in the buffer) */
  for (row = res->rowcnt; --row >= 0; )
    *--b = *--s *t;             /* compute average/expected values */
  t = (mle)   ? w : w-1;        /* compute the factor for */
  t = (t > 0) ? 1/t : 0;        /* the (co)variance estimation */
  for (row = res->rowcnt; --row >= 0; ) {
    s = mat->els[row];          /* traverse the element product sums */
    d = res->els[row];          /* in the lower matrix triangle */
    for (r = b[row], col = mat->colcnt; --col >= row; )
      d[col] = t *(s[col] -b[col] *r *w);
    if (d[row] < 0) d[row] = 0; /* compute the (co)variances and */
  }                             /* ensure non-negative diagonal */
  return res;                   /* return the (covariance) matrix */
}  /* mat_covar() */

/*--------------------------------------------------------------------*/

MATRIX* mat_correl (MATRIX *res, const MATRIX *mat)
{                               /* --- correl. from covar. matrix */
  int    row, col;              /* loop variables */
  double *d; const double *s;   /* to traverse the matrix rows */
  double t;                     /* temporary buffer */

  assert(res && mat             /* check the function arguments */
     && (res->rowcnt == res->colcnt)
     && (mat->rowcnt == mat->colcnt)
     && (res->rowcnt == mat->rowcnt));
  for (row = res->rowcnt; --row >= 0; ) {
    t = mat->els[row][row];     /* traverse the variances (diagonal) */
    t = (t > 0) ? sqrt(t) : 0;  /* and compute their square roots */
    res->buf[row] = (t > 0) ? 1/t : 0;
  }                             /* note the standard deviations */
  for (row = res->rowcnt; --row >= 0; ) {
    t = res->buf[row];          /* get next needed std. deviation */
    d = res->els[row];          /* traverse the matrix rows */
    s = mat->els[row];          /* of source and destination */
    for (col = res->colcnt; --col > row; )
      d[col] = s[col] *res->buf[col] *t;
  }                             /* compute the correlation coeff. */
  for (row = res->rowcnt; --row >= 0; )
    res->els[row][row] = 1;     /* set the diagonal to 1 */
  return res;                   /* return the correlation matrix */
}  /* mat_correl() */           /* (only upper triangle is computed) */

/*--------------------------------------------------------------------*/

double mat_regress (MATRIX *res, double *rhs, const MATRIX *mat)
{                               /* --- set up regression eq. system */
  int    row, col, n;           /* loop variables, number of columns */
  double *d; const double *s;   /* to traverse the matrix rows */
  double *b;                    /* to traverse the buffer */
  double so2;                   /* sum of squared outputs */

  assert(rhs && res && mat      /* check the function arguments */
     && (res->rowcnt == res->colcnt)
     && (mat->rowcnt == mat->colcnt)
     && (res->rowcnt == mat->rowcnt));
  rhs += n = mat->colcnt;       /* copy the sum of output values */
  *--rhs = mat->buf[--n];       /* to the right hand side vector */
  for (b = res->buf +(row = n); --row >= 0; ) {
    *--rhs = mat->els[row][n];  /* move the last column to the rhs */
    res->els[row][n] = *--b;    /* and the buffer to the last column */
  }                             /* (build system of normal equations) */
  so2 = mat->els[n][n];         /* note the sum of squared outputs */
  res->els[n][n] = mat->weight; /* store the sum of vector weights */
  if (res != mat) {             /* if not to work in place, */
    res->weight = mat->weight;  /* copy the sum of vector weights */
    for (row = n; --row >= 0; ) {
      s = mat->els[row] +n;     /* copy the upper triangle */
      d = res->els[row] +n;     /* of the aggregation matrix */
      for (col = n; --col >= row; ) *--d = *--s;
    }                           /* (only the upper triangle is needed */
  }                             /*  for the Cholesky decomposition) */
  return so2;                   /* return the sum of squared outputs */
}  /* mat_regress() */
