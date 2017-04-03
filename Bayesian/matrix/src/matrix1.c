/*----------------------------------------------------------------------
  File    : matrix1.c
  Contents: general vector and matrix management
            without matrix inversion and equation system solving and
            without eigenvalue and eigenvector functions
  Author  : Christian Borgelt
  History : 30.04.1999 file created
            13.05.1999 vector and matrix read functions completed
            14.07.2001 adapted to modified module tfscan
            17.08.2001 VECTOR data type replaced by double*
            13.10.2001 parameters vec and mat added to read functions
                       (original versions kept as vec_readx, mat_readx)
            15.10.2001 vec_show, mat_show made macros, mat_sub added
            25.10.2001 function mat_subx added
            07.11.2001 argument loc added to mat_dup, mat_copy, mat_cmp
            09.11.2001 functions mat_diasum and mat_diaprod added
            10.11.2001 functions vec_sqrlen and vec_sqrdist added
            11.11.2001 function mat_mulvdv added
            07.08.2002 bug in read error reporting fixed
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
#define ROWBLKS    0x0001       /* one memory block per matrix row */
#define ODDPERM    0x0002       /* odd row permutation (LU decomp.) */

/* --- sizes --- */
#define BLKSIZE    256          /* block size for matrices */

/*----------------------------------------------------------------------
  Basic Vector Functions
----------------------------------------------------------------------*/

double* vec_init (double *vec, int n, int unit)
{                               /* --- initialize a vector */
  assert(vec && (unit < n));    /* check the function arguments */
  while (--n >= 0) vec[n] = 0;  /* clear the vector and */
  if (unit >= 0) vec[unit] = 1; /* set a unit direction */
  return vec;                   /* return the initialized vector */
}  /* vec_init() */

/*--------------------------------------------------------------------*/

double* vec_copy (double *dst, const double *src, int n)
{                               /* --- copy a vector */
  assert(src && dst);           /* check the function arguments */
  while (--n >= 0) dst[n] = src[n];
  return dst;                   /* copy the vector elements */
}  /* vec_copy() */             /* and return the destination */

/*--------------------------------------------------------------------*/

double vec_cmp (const double *a, const double *b, int n)
{                               /* --- compare two vectors */
  double max = 0, t;            /* maximal difference, buffer */

  assert(a && b);               /* check the function arguments */
  while (--n >= 0) {            /* traverse the vector elements, */
    t = fabs(a[n] - b[n]);      /* compute their differences, */
    if (t > max) max = t;       /* and determine the maximum */
  }                             /* of these differences */
  return max;                   /* return the maximum difference */
}  /* vec_cmp() */

/*----------------------------------------------------------------------
  Vector Operations
----------------------------------------------------------------------*/

double vec_sqrlen (const double *vec, int n)
{                               /* --- compute squared vector length */
  double sum = 0;               /* sum of squared elements */

  assert(vec && (n > 0));       /* check the function argument */
  while (--n >= 0) sum += vec[n] *vec[n];
  return sum;                   /* compute the squared vector length */
}  /* vec_sqrlen() */

/*--------------------------------------------------------------------*/

double vec_sqrdist (const double *a, const double *b, int n)
{                               /* --- compute squared distance */
  double sum = 0;               /* sum of element products */
  double t;                     /* element of difference vector */

  assert(a && b && (n > 0));    /* check the function arguments */
  while (--n >= 0) { t = a[n] -b[n]; sum += t*t; }
  return sum;                   /* compute the squared distance */
}  /* vec_sqrdist() */

/*--------------------------------------------------------------------*/

double* vec_add (double *res, int n,
                 const double *a, double k, const double *b)
{                               /* --- add a vector to another */
  assert(res && a && b && (n > 0));  /* check the function arguments */
  if      (k ==  1)             /* if to add the vectors */
    while (--n >= 0) res[n] = a[n] +   b[n];
  else if (k == -1)             /* if to subtract the vectors */
    while (--n >= 0) res[n] = a[n] -   b[n];
  else                          /* if to add an arbitrary multiple */
    while (--n >= 0) res[n] = a[n] +k *b[n];
  return res;                   /* return the result vector */
}  /* vec_add() */

/*--------------------------------------------------------------------*/

double* vec_muls (double *res, int n, const double *vec, double k)
{                               /* --- multiply vector with scalar */
  assert(res && vec && (n > 0));/* check the function arguments */
  if (k == -1)                  /* if to negate the vector */
    while (--n >= 0) res[n] = -vec[n];
  else                          /* otherwise (arbitrary factor) */
    while (--n >= 0) res[n] = vec[n] *k;
  return res;                   /* multiply the vector elements */
}  /* vec_muls() */             /* and return the result vector */

/*--------------------------------------------------------------------*/

double vec_sprod (const double *a, const double *b, int n)
{                               /* --- compute scalar product */
  double sum = 0;               /* sum of element products */

  assert(a && b && (n > 0));    /* check the function arguments */
  while (--n >= 0) sum += a[n] *b[n];
  return sum;                   /* sum the products of the elements */
}  /* vec_sprod() */            /* and return the result */

/*--------------------------------------------------------------------*/

double* vec_vprod (double *res, const double *a, const double *b)
{                               /* --- compute vector product */
  double x, y;                  /* temporary buffers */

  assert(a && b && res);        /* check the function arguments */
  x      = a[1]*b[2] - a[2]*b[1];
  y      = a[2]*b[0] - a[0]*b[2];
  res[2] = a[0]*b[1] - a[1]*b[0];
  res[1] = y;                   /* assign results to buffers first */
  res[0] = x;                   /* to allow res = a or res = b, */
  return res;                   /* then copy buffers to result */
}  /* vec_vprod() */

/*--------------------------------------------------------------------*/

MATRIX* vec_mprod (MATRIX *res, const double *a, const double *b)
{                               /* --- compute matrix product */
  int    row, col;              /* loop variables */
  double *d;                    /* to traverse matrix rows */
  double t;                     /* temporary buffer */

  assert(a && b && res);        /* check the function arguments */
  for (row = res->rowcnt; --row >= 0; ) {
    d = res->els[row];          /* traverse the matrix rows */
    t = a[row];                 /* and the vector a */
    for (col = res->colcnt; --col >= 0; )
      d[col] = t *b[col];       /* multiply each element of the */
  }                             /* vector a with the vector b and */
  return res;                   /* store the result in the rows */
}  /* vec_mprod() */

/*--------------------------------------------------------------------*/

int vec_write (const double *vec, int n, FILE *file,
               const char *fmt, const char *sep)
{                               /* --- write a vector to a file */
  assert(vec                    /* check the function arguments */
      && file && sep && (n > 0));
  while (--n >= 0) {            /* traverse the vector elements */
    fprintf(file, fmt, *vec++); /* print a vector element and */
    if (n > 0) fputc(sep[0], file);   /* an element separator */
  }
  fputs(sep+1, file);           /* print a vector separator */
  return ferror(file) ? -1 : 0; /* check for an error */
}  /* vec_write() */

/*--------------------------------------------------------------------*/
#ifdef MAT_READ

static int _vecerr (TFSCAN *tfs, int code, int fld, int exp)
{                               /* --- clean up on a read error */
  TFSERR *err = tfs_err(tfs);   /* error information of scanner */

  err->fld = fld;               /* fill the error */
  err->exp = exp;               /* information structure */
  if (code == E_FLDCNT) err->s =  "";
  else                  err->s = tfs_buf(tfs);
  return err->code = code;      /* return error result of vec_read() */
}  /* _vecerr() */

/*--------------------------------------------------------------------*/

int vec_read (double *vec, int n, TFSCAN *tfscan, FILE *file)
{                               /* --- read a vector from a file */
  int  i = 0;                   /* field index */
  int  d;                       /* delimiter type */
  char *end;                    /* end pointer for conversion */
  char *buf;                    /* read buffer (of tfscan) */

  assert(vec                    /* check the function arguments */
      && tfscan && file && (n > 0));
  buf = tfs_buf(tfscan);        /* get the read buffer */
  do {                          /* vector element read loop */
    d = tfs_getfld(tfscan, file, NULL, 0);
    if (d <= TFS_REC) {         /* read the next vector element */
      if (d < 0)             return _vecerr(tfscan, E_FREAD, i+1, 0);
      if ((i <= 0) && !*buf) return _vecerr(tfscan, 1,         1, 0);
    }                           /* if the record is empty, abort */
    if (i >= n)                 /* if the vector is full, abort */
      return _vecerr(tfscan, E_FLDCNT, i+1, n);
    vec[i++] = strtod(buf, &end);
    if (*end || (end == buf))   /* convert and store the value read */
      return _vecerr(tfscan, E_VALUE, i, 0);
  } while (d > TFS_REC);        /* while not at end of record */
  if (i < n)                    /* check the number of fields */
    return _vecerr(tfscan, E_FLDCNT, i, n);
  return 0;                     /* return 'ok' */
}  /* vec_read() */

/*--------------------------------------------------------------------*/

static double* _vxerr (TFSCAN *tfs, int code, int fld, int exp,
                       double *vec)
{                               /* --- clean up on a read error */
  if (vec) free(vec);           /* delete an allocated vector */
  _vecerr(tfs, code, fld, exp); /* set error information */
  return NULL;                  /* return error result of vec_readx() */
}  /* _vxerr() */

/*--------------------------------------------------------------------*/

double* vec_readx (TFSCAN *tfscan, FILE *file, int *n)
{                               /* --- read a vector from a file */
  double *vec = NULL, *p;       /* created vector, realloc. buffer */
  int    i, vsz;                /* field index and vector size */
  int    d;                     /* delimiter type */
  char   *end;                  /* end pointer for conversion */
  char   *buf;                  /* read buffer (of tfscan) */

  assert(tfscan && file && n);  /* check the function arguments */
  buf = tfs_buf(tfscan);        /* get the read buffer */
  vsz = i = 0;                  /* initialize the index variables */
  do {                          /* vector element read loop */
    d = tfs_getfld(tfscan, file, NULL, 0);
    if (d <= TFS_REC) {         /* read the next vector element */
      if (d < 0)             return _vxerr(tfscan, E_FREAD, i+1,0, vec);
      if ((i <= 0) && !*buf) return _vxerr(tfscan, E_NONE,    1,0, vec);
    }                           /* if the record is empty, abort */
    if (i >= vsz) {             /* if the current vector is full */
      if (*n > vsz)             /* if vector empty and dim. given, */
        vsz = *n;               /* set the requested dimension */
      else if (*n <= 0)         /* if no dim. given, increase size */
        vsz += (vsz > BLKSIZE) ? vsz >> 1 : BLKSIZE;
      else                      /* otherwise abort the function */
        return _vxerr(tfscan, E_FLDCNT, i+1, *n, vec);
      p = (double*)realloc(vec, vsz *sizeof(double));
      if (!p) return _vxerr(tfscan, E_NOMEM, i+1, 0, vec);
      vec = p;                  /* allocate/enlarge the vector */
    }                           /* and set the new vector */
    vec[i++] = strtod(buf, &end);
    if (*end || (end == buf))   /* convert and store the value read */
      return _vxerr(tfscan, E_VALUE, i, 0, vec);
  } while (d > TFS_REC);        /* while not at end of record */
  if (i < vsz) {                /* check the number of fields */
    if (*n > 0) return _vxerr(tfscan, E_FLDCNT, i, *n, vec);
    vec = (double*)realloc(vec, i *sizeof(double));
  }                             /* try to shrink the vector */
  *n = i;                       /* note the number of elements */
  return vec;                   /* return the created vector */
}  /* vec_readx() */

#endif
/*----------------------------------------------------------------------
  Basic Matrix Functions
----------------------------------------------------------------------*/

MATRIX* mat_create (int rowcnt, int colcnt)
{                               /* --- create a matrix */
  MATRIX *mat;                  /* created matrix */
  double *p;                    /* to traverse the matrix elements */
  int    n, m;                  /* number of matrix/buffer elements */

  assert((rowcnt > 0) && (colcnt > 0));  /* check the arguments */
  mat = (MATRIX*)malloc(sizeof(MATRIX) +(rowcnt-1) *sizeof(double*));
  if (!mat) return NULL;        /* allocate the matrix body */
  mat->rowcnt = rowcnt;         /* note the number of rows */
  mat->colcnt = colcnt;         /* and the number of columns */
  mat->flags  = 0;              /* clear the matrix flags */
  mat->weight = 0;              /* and the vector weight sum */
  if (rowcnt != colcnt)         /* if the matrix is not square, */
    mat->map = NULL;            /* no permutation map is needed */
  else {                        /* if the matrix is square */
    mat->map = (int*)malloc(2 *rowcnt *sizeof(int));
    if (!mat->map) { free(mat); return NULL; }
  }                             /* create a permutation map */
  m = (rowcnt > colcnt) ? rowcnt : colcnt;
  n = rowcnt *colcnt;           /* compute the number of elements */
  p = (double*)malloc((n+m) *sizeof(double));
  if (!p) { if (mat->map) free(mat->map); free(mat); return NULL; }
  mat->buf = p;                 /* allocate the matrix elements */
  for (p += n+m; --rowcnt >= 0; ) {  /* organize the memory */
    mat->els[rowcnt] = p -= colcnt;  /* into matrix rows */
    mat->buf[rowcnt] = 0;            /* and clear the */
  }                                  /* vector buffer */
  return mat;                   /* return the created matrix */
}  /* mat_create() */

/*--------------------------------------------------------------------*/

void mat_delete (MATRIX *mat)
{                               /* --- delete a matrix */
  int row;                      /* loop variable */

  assert(mat);                  /* check the function argument */
  if (mat->flags & ROWBLKS) {   /* if one memory block per row */
    for (row = mat->rowcnt; --row >= 0; )
      if (mat->els[row]) free(mat->els[row]);
  }                             /* delete the matrix rows */
  if (mat->buf) free(mat->buf); /* delete the vector buffer, */
  if (mat->map) free(mat->map); /* the permutation map, */
  free(mat);                    /* and the matrix body */
}  /* mat_delete() */

/*--------------------------------------------------------------------*/

MATRIX* mat_dup (const MATRIX *mat, int loc)
{                               /* --- duplicate a matrix */
  MATRIX *dup;                  /* created duplicate */

  assert(mat);                  /* check the function argument */
  dup = mat_create(mat->rowcnt, mat->colcnt);
  if (!dup) return NULL;        /* create a new matrix and */
  return mat_copy(dup,mat,loc); /* copy the old matrix into it */
}  /* mat_dup() */

/*--------------------------------------------------------------------*/

MATRIX* mat_copy (MATRIX *dst, const MATRIX *src, int loc)
{                               /* --- copy a matrix */
  int    row, col;              /* loop variables */
  int    beg, end;              /* range of column indices */
  double *d; const double *s;   /* matrix rows of dest. and source */

  assert(src && dst             /* check the function arguments */
     && (src->colcnt == dst->colcnt)
     && (src->rowcnt == dst->rowcnt));
  dst->weight = src->weight;    /* copy the vector weight sum */
  if (src->map) {               /* copy the permutation map */
    for (row = src->rowcnt; --row >= 0; )
      dst->map[row] = src->map[row];
    dst->flags |= src->flags & ODDPERM;
  }                             /* copy the odd permutation flag */
  if (dst->buf) {               /* if there is a vector buffer, */
    d = dst->buf; s = src->buf; /* get pointers to the buffers */
    for (row = dst->rowcnt; --row >= 0; )
      d[row] = s[row];          /* copy the buffer contents */
  }
  if (loc == MAT_DIAG) {        /* if to copy the diagonal */
    assert(dst->rowcnt == dst->colcnt);
    for (row = dst->rowcnt; --row >= 0; )
      dst->els[row][row] = src->els[row][row]; }
  else {                        /* if to copy all or a triangle */
    assert(!(loc & (MAT_LOWER|MAT_UPPER))
         || (dst->rowcnt == dst->colcnt));
    beg = dst->colcnt; end = 0; /* initialize the column range */
    for (row = dst->rowcnt; --row >= 0; ) {
      d = dst->els[row]; s = src->els[row];
      if      (loc == MAT_UPPER) end = row;
      else if (loc == MAT_LOWER) beg = row+1;
      for (col = beg; --col >= end; )
        d[col] = s[col];        /* copy the matrix rows */
    }                           /* column by column */
  }
  return dst;                   /* return the destination matrix */
}  /* mat_copy() */

/*--------------------------------------------------------------------*/

double mat_cmp (const MATRIX *A, const MATRIX *B, int loc)
{                               /* --- compare two matrices */
  int          row, col;        /* loop variables */
  int          beg, end;        /* range of column indices */
  const double *sa, *sb;        /* to traverse the matrix rows */
  double       max = 0, t;      /* maximal difference, buffer */

  assert(A && B                 /* check the function arguments */
     && (A->rowcnt == B->rowcnt)
     && (A->colcnt == B->colcnt));
  if (loc == MAT_DIAG) {        /* if to compare the diagonal */
    assert(A->rowcnt == A->colcnt);
    for (row = A->rowcnt; --row >= 0; ) {
      t = fabs(A->els[row][row] - B->els[row][row]);
      if (t > max) max = t;     /* compute the element differences */
    } }                         /* and determine their maximum */
  else {                        /* if to compare all or a triangle */
    assert(!(loc & (MAT_LOWER|MAT_UPPER))
         || (A->rowcnt == A->colcnt));
    beg = A->colcnt; end = 0;   /* initialize the column range */
    for (row = A->rowcnt; --row >= 0; ) {
      sa = A->els[row]; sb = B->els[row];
      if      (loc == MAT_UPPER) end = row;
      else if (loc == MAT_LOWER) beg = row+1;
      for (col = beg; --col >= end; ) {
        t = fabs(sa[col] - sb[col]);
        if (t > max) max = t;   /* compute the element differences */
      }                         /* and determine their maximum */
    }
  }
  return max;                   /* return the maximal difference */
}  /* mat_cmp() */

/*----------------------------------------------------------------------
  Matrix Initialization Functions
----------------------------------------------------------------------*/

void mat_init (MATRIX *mat, int mode, const double *vals)
{                               /* --- initialize a matrix */
  int    row, col;              /* loop variable */
  double *p;                    /* to traverse the matrix rows */
  double t;                     /* initialization value */

  assert(mat);                  /* check the function argument */
  mat->flags &= ROWBLKS;        /* clear all special flags */
  mat->weight = 0;              /* and the vector weight sum */
  if (mat->buf) {               /* if there is a vector buffer */
    t = ((mode == MAT_VALUE) && vals) ? *vals : 0;
    for (p = mat->buf +(row = mat->rowcnt); --row >= 0; )
      *--p = t;                 /* clear the vector buffer or */
  }                             /* set it to the given value */
  switch (mode) {               /* evaluate the initialization mode */
    case MAT_ZERO:              /* set matrix to zero */
    case MAT_UNIT:              /* set a unit matrix */
      for (row = mat->rowcnt; --row >= 0; )
        for (p = mat->els[row] +(col = mat->colcnt); --col >= 0; )
          *--p = 0;             /* clear the matrix elements */
      if (mode == MAT_UNIT) {   /* if to set a unit matrix */
        assert(mat->rowcnt == mat->colcnt);
        for (row = mat->rowcnt; --row >= 0; )
          mat->els[row][row] = 1;
      } break;                  /* set the diagonal elements */
    case MAT_UPPER:             /* set upper triangular matrix */
      if (!vals) break;         /* if no values are given, abort */
      assert(mat->rowcnt == mat->colcnt);
      for (row = 0; row < mat->rowcnt; row++) {
        p = mat->els[row] +row; /* traverse the matrix rows */
        for (col = mat->colcnt; --col >= row; )
          *p++ = *vals++;       /* copy the given values */
      } break;                  /* to the upper triangular matrix */
    case MAT_LOWER:             /* set lower triangular matrix */
      if (!vals) break;         /* if no values are given, abort */
      assert(mat->rowcnt == mat->colcnt);
      for (row = 0; row < mat->rowcnt; row++) {
        p = mat->els[row];      /* traverse the matrix rows */
        for (col = 0; col <= row; col++)
          *p++ = *vals++;       /* copy the given values */
      } break;                  /* to the lower triangular matrix */
    case MAT_DIAG:              /* set matrix diagonal */
      if (!vals) break;         /* if no values are given, abort */
      assert(mat->rowcnt == mat->colcnt);
      for (row = 0; row < mat->rowcnt; row++)
        mat->els[row][row] = *vals++;
      break;                    /* copy the values to the diagonal */
    case MAT_VALUE:             /* set to single given value */
      t = (vals) ? *vals : 0;   /* get the initialization value */
      for (row = mat->rowcnt; --row >= 0; )
        for (p = mat->els[row] +(col = mat->colcnt); --col >= 0; )
          *--p = t;             /* set all matrix elements */
      break;                    /* to the single given value */
    default:                    /* set to given values */
      if (!vals) break;         /* if no values are given, abort */
      for (row = 0; row < mat->rowcnt; row++) {
        p = mat->els[row];      /* traverse the matrix rows */
        for (col = mat->colcnt; --col >= 0; )
          *p++ = *vals++;       /* copy the given values */
      } break;                  /* to the matrix rows */
  }  /* switch (mode) ... */
}  /* mat_init() */

/*--------------------------------------------------------------------*/

void mat_crop (MATRIX *mat, int loc)
{                               /* --- crop to triangular/diagonal */
  int    row, col;              /* loop variables */
  double *d;                    /* to traverse the matrix rows */

  assert(mat                    /* check the function arguments */
     && (mat->rowcnt == mat->colcnt));
  if (loc & MAT_UPPER)          /* if to crop to upper triangle */
    for (row = mat->rowcnt; --row > 0; )
      for (d = mat->els[row] +(col = row); --col >= 0; )
        *--d = 0;               /* clear the lower triangle */
  if (loc & MAT_LOWER)          /* if to crop to lower triangle */
    for (row = mat->rowcnt-1; --row >= 0; )
      for (d = mat->els[row] +(col = mat->colcnt); --col > row; )
        *--d = 0;               /* clear the upper triangle */
}  /* mat_crop() */

/*----------------------------------------------------------------------
  Matrix Row Operations
----------------------------------------------------------------------*/

void mat_rowinit (MATRIX *mat, int row, int unit)
{                               /* --- initialize a row vector */
  int    col;                   /* loop variable */
  double *d;                    /* matrix row to initialize */

  assert(mat                    /* check the function arguments */
     && (row >= 0) && (row < mat->rowcnt) && (unit < mat->colcnt));
  for (d = mat->els[row] +(col = mat->colcnt); --col >= 0; )
    *--d = 0;                   /* clear the row vector and */
  if (unit >= 0) d[unit] = 1;   /* set a null vector or a unit vector */
}  /* mat_rowinit() */

/*--------------------------------------------------------------------*/

double* mat_rowget (const MATRIX *mat, int row, double *vec)
{                               /* --- get a row vector */
  int          col;             /* loop variable */
  const double *s;              /* matrix row to get */

  assert(mat && vec             /* check the function arguments */
     && (row >= 0) && (row < mat->rowcnt));
  s = mat->els[row];            /* copy from the matrix row */
  for (col = mat->colcnt; --col >= 0; ) vec[col] = s[col];
  return vec;                   /* return the filled vector */
}  /* mat_rowget() */

/*--------------------------------------------------------------------*/

void mat_rowset (MATRIX *mat, int row, const double *vec)
{                               /* --- set a row vector */
  int    col;                   /* loop variable */
  double *d;                    /* matrix row to set */

  assert(mat && vec             /* check the function arguments */
     && (row >= 0) && (row < mat->rowcnt));
  d = mat->els[row];            /* copy to the matrix row */
  for (col = mat->colcnt; --col >= 0; ) d[col] = vec[col];     
}  /* mat_rowset() */           

/*--------------------------------------------------------------------*/

double mat_rowlen (const MATRIX *mat, int row)
{                               /* --- compute row vector length */
  int    col;                   /* loop variable */
  double sum = 0;               /* sum of squared elements */
  double *p;                    /* matrix row to process */

  assert(mat                    /* check the function arguments */
     && (row >= 0) && (row < mat->rowcnt));
  p = mat->els[row];            /* sum the row element squares */
  for (col = mat->colcnt; --col >= 0; ) sum += p[col] *p[col];
  return sqrt(sum);             /* compute and return the length */
}  /* mat_rowlen() */

/*--------------------------------------------------------------------*/

void mat_rowadd (MATRIX *mat, int row1, double k, int row2)
{                               /* --- add a row vector to another */
  int    col;                   /* loop variable */
  double *s, *d;                /* matrix rows to add */

  assert(mat                    /* check the function arguments */
     && (row1 >= 0) && (row1 < mat->rowcnt)
     && (row2 >= 0) && (row2 < mat->rowcnt));
  s = mat->els[row2];           /* get the source and */
  d = mat->els[row1];           /* the destination row */
  if      (k ==  1)             /* if to add the rows */
    for (col = mat->colcnt; --col >= 0; ) d[col] +=    s[col];
  else if (k == -1)             /* if to subtract the rows */
    for (col = mat->colcnt; --col >= 0; ) d[col] -=    s[col];
  else                          /* if to add an arbitrary multiple */
    for (col = mat->colcnt; --col >= 0; ) d[col] += k *s[col];
}  /* mat_rowadd() */

/*--------------------------------------------------------------------*/

void mat_rowaddv (MATRIX *mat, int row, double k, const double *vec)
{                               /* --- add a vector to a row */
  int    col;                   /* loop variable */
  double *d;                    /* matrix row to add to */

  assert(mat && vec             /* check the function arguments */
     && (row >= 0) && (row < mat->rowcnt));
  d = mat->els[row];            /* get the destination row */
  if      (k ==  1)             /* if to add to the row */
    for (col = mat->colcnt; --col >= 0; ) d[col] +=    vec[col];
  else if (k == -1)             /* if to subtract from the row */
    for (col = mat->colcnt; --col >= 0; ) d[col] -=    vec[col];
  else                          /* if to add an arbitrary multiple */
    for (col = mat->colcnt; --col >= 0; ) d[col] += k *vec[col];
}  /* mat_rowaddv() */

/*--------------------------------------------------------------------*/

void mat_rowmuls (MATRIX *mat, int row, double k)
{                               /* --- mul. matrix row with scalar */
  int    col;                   /* loop variable */
  double *d;                    /* matrix row to multiply */

  assert(mat                    /* check the function arguments */
     && (row >= 0) && (row < mat->rowcnt));
  d = mat->els[row];            /* multiply the row elements */
  for (col = mat->colcnt; --col >= 0; ) d[col] *= k;
}  /* mat_rowmuls() */

/*--------------------------------------------------------------------*/

double mat_rowmulv (const MATRIX *mat, int row, const double *vec)
{                               /* --- multiply a row with a vector */
  int    col;                   /* loop variable */
  double *s;                    /* matrix row to multiply with */
  double sum = 0;               /* sum of element products */

  assert(mat && vec             /* check the function arguments */
     && (row >= 0) && (row < mat->rowcnt));
  s = mat->els[row];            /* sum the element products */
  for (col = mat->colcnt; --col >= 0; ) sum += s[col] *vec[col];
  return sum;                   /* return the result */
}  /* mat_rowmulv() */

/*--------------------------------------------------------------------*/

double mat_rowmul (const MATRIX *mat, int row1, int row2)
{                               /* --- multiply two matrix rows */
  int    col;                   /* loop variable */
  double *s1, *s2;              /* matrix rows to process */
  double sum = 0;               /* sum of element products */

  assert(mat                    /* check the function argument */
     && (row1 >= 0) && (row1 < mat->rowcnt)
     && (row2 >= 0) && (row2 < mat->rowcnt));
  s1 = mat->els[row1];          /* get the matrix rows to multiply */
  s2 = mat->els[row2];          /* and sum the element products */
  for (col = mat->colcnt; --col >= 0; ) sum += s1[col] *s2[col];
  return sum;                   /* return the result */
}  /* mat_rowmul() */

/*--------------------------------------------------------------------*/

void mat_rowexg (MATRIX *mat, int row1, int row2)
{                               /* --- exchange two matrix rows */
  double *r;                    /* exchange buffer */

  assert(mat                    /* check the function arguments */
     && (row1 >= 0) && (row1 < mat->rowcnt)
     && (row2 >= 0) && (row2 < mat->rowcnt));
  r              = mat->els[row1];
  mat->els[row1] = mat->els[row2];
  mat->els[row2] = r;           /* exchange the matrix rows */
}  /* mat_rowexg() */

/*--------------------------------------------------------------------*/

void mat_shuffle (MATRIX *mat, double randfn(void))
{                               /* --- shuffle the rows of a matrix */
  int    i, n;                  /* vector index, number of vectors */
  double **p, *vec;             /* to traverse the vectors, buffer */

  assert(mat && randfn);        /* check the function arguments */
  for (p = mat->els, n = mat->rowcnt; --n > 0; ) {
    i = (int)((n+1) *randfn()); /* traverse the pattern vector */
    if      (i > n) i = n;      /* compute a random index in the */
    else if (i < 0) i = 0;      /* remaining vector section */
    vec = p[i]; p[i] = *p; *p++ = vec;
  }                             /* exchange first and i-th pattern */
}  /* mat_shuffle() */

/*----------------------------------------------------------------------
  Matrix Column Operations
----------------------------------------------------------------------*/

void mat_colinit (MATRIX *mat, int col, int unit)
{                               /* --- initialize a column vector */
  int row;                      /* loop variable */

  assert(mat                    /* check the function arguments */
     && (col >= 0) && (col < mat->colcnt) && (unit < mat->rowcnt));
  for (row = mat->rowcnt; --row >= 0; )
    mat->els[row][col] = 0;     /* clear the column vector */
  if (unit >= 0) mat->els[unit][col] = 1;
}  /* mat_colinit() */          /* set a null vector or a unit vector */

/*--------------------------------------------------------------------*/

double* mat_colget (const MATRIX *mat, int col, double *vec)
{                               /* --- get a column vector */
  int row;                      /* loop variable */

  assert(mat && vec             /* check the function arguments */
     && (col >= 0) && (col < mat->colcnt));
  for (row = mat->rowcnt; --row >= 0; )
    vec[row] = mat->els[row][col];
  return vec;                   /* copy from the matrix column */
}  /* mat_colget() */           /* and return the filled vector */

/*--------------------------------------------------------------------*/

void mat_colset (MATRIX *mat, int col, const double *vec)
{                               /* --- set a column vector */
  int row;                      /* loop variable */

  assert(mat && vec             /* check the function arguments */
     && (col >= 0) && (col < mat->colcnt));
  for (row = mat->rowcnt; --row >= 0; )
    mat->els[row][col] = vec[row];
}  /* mat_colset() */           /* copy to the matrix column */

/*--------------------------------------------------------------------*/

double mat_collen (const MATRIX *mat, int col)
{                               /* --- compute column vector length */
  int    row;                   /* loop variable */
  double sum = 0;               /* sum of squared elements */
  double *e;                    /* to traverse the matrix elements */

  assert(mat                    /* check the function arguments */
     && (col >= 0) && (col < mat->colcnt));
  for (row = mat->rowcnt; --row >= 0; ) {
    e = mat->els[row] +col; sum += *e * *e; }
  return sqrt(sum);             /* sum the element squares and */
}  /* mat_collen() */           /* compute and return the length */

/*--------------------------------------------------------------------*/

void mat_coladd (MATRIX *mat, int col1, double k, int col2)
{                               /* --- add a column to another */
  int row;                      /* loop variable */

  assert(mat                    /* check the function arguments */
     && (col1 >= 0) && (col1 < mat->rowcnt)
     && (col2 >= 0) && (col2 < mat->rowcnt));
  if      (k ==  1)             /* if to add to the column */
    for (row = mat->rowcnt; --row >= 0; )
      mat->els[row][col1] +=    mat->els[row][col2];
  else if (k == -1)             /* if to subtract from the column */
    for (row = mat->rowcnt; --row >= 0; )
      mat->els[row][col1] -=    mat->els[row][col2];
  else                          /* if to add an arbitrary multiple */
    for (row = mat->rowcnt; --row >= 0; )
      mat->els[row][col1] += k *mat->els[row][col2];
}  /* mat_coladd() */

/*--------------------------------------------------------------------*/

void mat_coladdv (MATRIX *mat, int col, double k, const double *vec)
{                               /* --- add a vector to a column */
  int row;                      /* loop variable */

  assert(mat && vec             /* check the function arguments */
     && (col >= 0) && (col < mat->colcnt));
  if      (k ==  1)             /* if to add to the column */
    for (row = mat->rowcnt; --row >= 0; )
      mat->els[row][col] +=    vec[row];
  else if (k == -1)             /* if to subtract from the column */
    for (row = mat->rowcnt; --row >= 0; )
      mat->els[row][col] -=    vec[row];
  else                          /* if to add an arbitrary multiple */
    for (row = mat->rowcnt; --row >= 0; )
      mat->els[row][col] += k *vec[row];
}  /* mat_coladdv() */

/*--------------------------------------------------------------------*/

void mat_colmuls (MATRIX *mat, int col, double k)
{                               /* --- mul. matrix column with scalar */
  int row;                      /* loop variable */

  assert(mat                    /* check the function arguments */
     && (col >= 0) && (col < mat->rowcnt));
  for (row = mat->rowcnt; --row >= 0; )
    mat->els[row][col] *= k;    /* multiply the column elements */
}  /* mat_colmuls() */

/*--------------------------------------------------------------------*/

double mat_colmulv (const MATRIX *mat, int col, const double *vec)
{                               /* --- mul. a column with a vector */
  int    row;                   /* loop variable */
  double sum = 0;               /* sum of element products */

  assert(mat && vec             /* check the function arguments */
     && (col >= 0) && (col < mat->colcnt));
  for (row = mat->colcnt; --row >= 0; )
    sum += mat->els[row][col] *vec[row];
  return sum;                   /* sum the element products */
}  /* mat_colmulv() */          /* and return the result */

/*--------------------------------------------------------------------*/

double mat_colmul (const MATRIX *mat, int col1, int col2)
{                               /* --- multiply two matrix columns */
  int    row;                   /* loop variable */
  double sum = 0;               /* sum of element products */

  assert(mat                    /* check the function arguments */
     && (col1 >= 0) && (col1 < mat->colcnt)
     && (col2 >= 0) && (col2 < mat->colcnt));
  for (row = mat->colcnt; --row >= 0; )
    sum += mat->els[row][col1] *mat->els[row][col2];
  return sum;                   /* sum the element products */
}  /* mat_colmul() */           /* and return the result */

/*--------------------------------------------------------------------*/

void mat_colexg (MATRIX *mat, int col1, int col2)
{                               /* --- exchange two matrix columns */
  int    row;                   /* loop variable */
  double t, *s1, *s2;           /* exchange buffer, matrix elements */

  assert(mat                    /* check the function arguments */
     && (col1 >= 0) && (col1 < mat->rowcnt)
     && (col2 >= 0) && (col2 < mat->rowcnt));
  for (row = mat->rowcnt; --row >= 0; ) {
    s1 = mat->els[row] +col1;   /* get pointers to */
    s2 = mat->els[row] +col2;   /* the column elements and */
    t = *s1; *s1 = *s2; *s2 = t;/* exchange the elements */
  }
}  /* mat_colexg() */

/*----------------------------------------------------------------------
  Diagonal Operations
----------------------------------------------------------------------*/

void mat_diainit (MATRIX *mat, int unit)
{                               /* --- initialize the diagonal */
  int    i;                     /* loop variable */
  double v = (unit < -1) ? 1:0; /* initialization value */

  assert(mat                    /* check the function argument */
     && (mat->rowcnt == mat->colcnt) && (unit < mat->rowcnt));
  for (i = mat->rowcnt; --i >= 0; )
    mat->els[i][i] = v;         /* initialize the diagonal */
  if (unit >= 0) mat->els[unit][unit] = 1;
}  /* mat_diainit() */

/*--------------------------------------------------------------------*/

double* mat_diaget (const MATRIX *mat, double *vec)
{                               /* --- get the diagonal of a matrix */
  int i;                        /* loop variable */

  assert(mat && vec             /* check the function arguments */
     && (mat->rowcnt == mat->colcnt));
  for (i = mat->rowcnt; --i >= 0; )
    vec[i] = mat->els[i][i];    /* copy from the matrix diagonal */
  return vec;                   /* and return the filled vector */
}  /* mat_diaget() */

/*--------------------------------------------------------------------*/

void mat_diaset (MATRIX *mat, const double *vec)
{                               /* --- set the diagonal of a matrix */
  int i;                        /* loop variable */

  assert(mat && vec             /* check the function arguments */
     && (mat->rowcnt == mat->colcnt));
  for (i = mat->rowcnt; --i >= 0; )
    mat->els[i][i] = vec[i];    /* copy vector to the matrix diagonal */
}  /* mat_diaset() */

/*--------------------------------------------------------------------*/

void mat_diaaddv (MATRIX *mat, double k, const double *vec)
{                               /* --- add a vector to a diagonal */
  int i;                        /* loop variable */

  assert(mat && vec             /* check the function arguments */
     && (mat->rowcnt == mat->colcnt));
  i = mat->rowcnt;              /* initialize the loop variable */
  if      (k ==  1)             /* if to add to the diagonal */
    while (--i >= 0) mat->els[i][i] +=    vec[i];
  else if (k == -1)             /* if to subtract from the diagonal */
    while (--i >= 0) mat->els[i][i] -=    vec[i];
  else                          /* if to add an arbitrary multiple */
    while (--i >= 0) mat->els[i][i] += k *vec[i];
}  /* mat_diaaddv() */

/*--------------------------------------------------------------------*/

void mat_diamuls (MATRIX *mat, double k)
{                               /* --- multiply diag. with a scalar */
  int i;                        /* loop variable */

  assert(mat                    /* check the function argument */
     && (mat->rowcnt == mat->colcnt));
  for (i = mat->rowcnt; --i >= 0; )
    mat->els[i][i] *= k;        /* multiply the diagonal with k */
}  /* mat_diamuls() */

/*--------------------------------------------------------------------*/

double mat_diasum (MATRIX *mat)
{                               /* --- sum of diagonal elements */
  int    i;                     /* loop variable */
  double sum;                   /* sum of diagonal elements (track) */

  assert(mat                    /* check the function argument */
     && (mat->rowcnt == mat->colcnt));
  sum = mat->els[0][0];         /* sum the diagonal elements */
  for (i = mat->rowcnt; --i > 0; ) sum += mat->els[i][i];
  return sum;                   /* and return the computed sum */
}  /* mat_diasum() */

/*--------------------------------------------------------------------*/

double mat_diaprod (MATRIX *mat)
{                               /* --- product of diagonal elements */
  int    i;                     /* loop variable */
  double prod;                  /* product of diagonal elements */

  assert(mat                    /* check the function argument */
     && (mat->rowcnt == mat->colcnt));
  prod = mat->els[0][0];        /* multiply the diagonal elements */
  for (i = mat->rowcnt; --i > 0; ) prod *= mat->els[i][i];
  return prod;                  /* return the computed product */
}  /* mat_diaprod() */

/*----------------------------------------------------------------------
  Matrix/Vector Operations
----------------------------------------------------------------------*/

double* mat_mulmv (double *res, MATRIX *mat, const double *vec)
{                               /* --- multiply matrix and vector */
  int          row, col;        /* loop variables */
  double       *s1;             /* to traverse the matrix rows */
  const double *s2;             /* to traverse the vector */

  assert(res && mat && vec);    /* check the function arguments */
  if (res != vec)               /* if the two vectors differ, */
    s2 = vec;                   /* use the source vector directly */
  else {                        /* if result and source are identical */
    s2 = s1 = mat->buf;         /* traverse the source vector */
    for (col = mat->colcnt; --col >= 0; )
      s1[col] = vec[col];       /* copy the source vector */
  }                             /* to a buffer in the matrix */
  for (row = mat->rowcnt; --row >= 0; ) {
    s1 = mat->els[row];         /* traverse the matrix rows */
    res[row] = 0;               /* initialize the result */
    for (col = mat->colcnt; --col >= 0; )
      res[row] += s1[col] *s2[col];
  }                             /* multiply matrix and vector elems. */
  return res;                   /* return the resulting vector */
}  /* mat_mulmv() */

/*--------------------------------------------------------------------*/

double* mat_mulvm (double *res, const double *vec, MATRIX *mat)
{                               /* --- multiply vector and matrix */
  int          row, col;        /* loop variables */
  double       *s1;             /* to traverse the matrix rows */
  const double *s2;             /* to traverse the vector */

  assert(res && mat && vec);    /* check the function arguments */
  if (res != vec)               /* if the two vectors differ, */
    s2 = vec;                   /* use the source vector directly */
  else {                        /* if result and source are identical */
    s2 = s1 = mat->buf;         /* traverse the source vector */
    for (row = mat->rowcnt; --row >= 0; )
      s1[row] = vec[row];       /* copy the source vector */
  }                             /* to a buffer in the matrix */
  for (col = mat->colcnt; --col >= 0; )
    res[col] = 0;               /* initialize the result */
  for (row = mat->rowcnt; --row >= 0; ) {
    s1 = mat->els[row];         /* traverse the matrix rows */
    for (col = mat->colcnt; --col >= 0; )
      res[col] += s1[col] *s2[row];
  }                             /* multiply matrix and vector elems. */
  return res;                   /* return the resulting vector */
}  /* mat_mulvm() */

/*--------------------------------------------------------------------*/

double mat_mulvmv (const MATRIX *mat, const double *vec)
{                               /* --- compute vec * mat * vec */
  int    row, col;              /* loop variables */
  double *p;                    /* to traverse the matrix rows */
  double t, res = 0;            /* result of multiplication */

  assert(mat && vec             /* check the function arguments */
     && (mat->rowcnt == mat->colcnt));
  for (row = mat->rowcnt; --row >= 0; ) {
    t = 0;                      /* traverse the matrix rows */
    p = mat->els[row];          /* and the columns of each row */
    for (col = mat->colcnt; --col >= 0; )
      t += vec[col] *p[col];    /* mult. matrix from left to vector, */
    res += vec[row] *t;         /* then compute the scalar product */
  }                             /* of the result and the vector */
  return res;                   /* return vec * mat * vec */
}  /* mat_mulvmv() */

/*--------------------------------------------------------------------*/

double mat_mulvdv (const MATRIX *mat, const double *vec)
{                               /* --- compute vec * diag * vec */
  int    i;                     /* loop variable */
  double res = 0;               /* result of multiplication */

  assert(mat && vec             /* check the function arguments */
     && (mat->rowcnt == mat->colcnt));
  for (i = mat->rowcnt; --i >= 0; )
    res += vec[i] *vec[i] *mat->els[i][i];
  return res;                   /* return vec * diag * vec */
}  /* mat_mulvdv() */

/*----------------------------------------------------------------------
  General Matrix Operations
----------------------------------------------------------------------*/

MATRIX* mat_transp (MATRIX *res, const MATRIX *mat)
{                               /* --- transpose a matrix */
  int    row, col;              /* loop variables */
  double *s, *d;                /* to traverse the destination rows */
  double t;                     /* exchange buffer */

  assert(res && mat             /* check the function arguments */
      && (res->rowcnt == mat->colcnt)
      && (res->colcnt == mat->rowcnt));
  if (res == mat) {             /* if the result is id. to the matrix */
    for (row = mat->rowcnt -1; --row >= 0; ) {
      for (s = res->els[row] +(col = mat->colcnt); --col > row; ) {
        d = res->els[col] +row; /* traverse the columns of the row */
        t = *--s; *s = *d; *d = t;
      }                         /* exchange elements so that they */
    } }                         /* are mirrored at the diagonal */
  else {                        /* if source and destination differ */
    for (col = mat->colcnt; --col >= 0; )
      for (s = res->els[col] +(row = mat->rowcnt); --row >= 0; )
        *--s = mat->els[row][col];
  }                             /* transpose the source columns */
  return res;                   /* return the transposed matrix */
}  /* mat_transp() */

/*--------------------------------------------------------------------*/

MATRIX* mat_muls (MATRIX *res, const MATRIX *mat, double k)
{                               /* --- multiply matrix with scalar */
  int    row, col;              /* loop variables */
  double *d; const double *s;   /* to traverse the matrix rows */

  assert(res && mat             /* check the function arguments */
      && (res->rowcnt == mat->rowcnt)
      && (res->colcnt == mat->colcnt));
  for (row = mat->rowcnt; --row >= 0; ) {
    d = res->els[row]; s = mat->els[row];
    for (col = mat->colcnt; --col >= 0; )
      d[col] = s[col] *k;       /* multiply the matrix rows */
  }                             /* column by column with the factor */
  return res;                   /* return the result matrix */
}  /* mat_muls() */

/*--------------------------------------------------------------------*/

MATRIX* mat_add (MATRIX *res, const MATRIX *A,
                 double k,    const MATRIX *B)
{                               /* --- add a multiple of a matrix */
  int          row, col;        /* loop variables */
  const double *sa, *sb;        /* to traverse the source matrix rows */
  double       *r;              /* to traverse the dest.  matrix rows */

  assert(res && A && B          /* check the function arguments */
     && (A->rowcnt == B->rowcnt)   && (A->colcnt == B->colcnt)
     && (A->rowcnt == res->rowcnt) && (A->colcnt == res->colcnt));
  if      (k ==  1) {           /* if to add the matrices */
    for (row = A->rowcnt; --row >= 0; ) {
      r  = res->els[row];       /* traverse the matrix rows */
      sa = A->els[row]; sb = B->els[row];
      for (col = A->colcnt; --col >= 0; )
        r[col] = sa[col] +sb[col];     /* traverse the columns */
    } }                         /* and add the matrix elements */
  else if (k == -1) {           /* if to subtract the matrices */
    for (row = A->rowcnt; --row >= 0; ) {
      r  = res->els[row];       /* traverse the matrix rows */
      sa = A->els[row]; sb = B->els[row];
      for (col = A->colcnt; --col >= 0; )
        r[col] = sa[col] -sb[col];     /* traverse the columns */
    } }                         /* and add the matrix elements */
  else {                        /* if to add an arbitrary multiple */
    for (row = A->rowcnt; --row >= 0; ) {
      r  = res->els[row];       /* traverse the matrix rows */
      sa = A->els[row]; sb = B->els[row];
      for (col = A->colcnt; --col >= 0; )
        r[col] = sa[col] +k *sb[col];
    }                           /* traverse the columns and */
  }                             /* add the matrix elements */
  return res;                   /* return the result matrix */
}  /* mat_add() */

/*--------------------------------------------------------------------*/

MATRIX* mat_mul (MATRIX *res, const MATRIX *A, const MATRIX *B)
{                               /* --- multiply two matrices */
  int          row, col, i;     /* loop variables */
  const double *sa, *sb;        /* to traverse the source      rows */
  double       *d;              /* to traverse the destination rows */
  double       *b;              /* to traverse the buffer */
  double       t;               /* temporary buffer */

  assert(res && A && B          /* check the function arguments */
      && (A->colcnt   == B->rowcnt)
      && (res->rowcnt == A->rowcnt)
      && (res->colcnt == B->colcnt));
  if      (res == A) {          /* if matrix A is id. to the result, */
    b = res->buf;               /* get the (row) buffer of matrix A */
    for (row = res->rowcnt; --row >= 0; ) {
      d = res->els[row];        /* traverse the rows of matrix A */
      for (col = A->colcnt; --col >= 0; ) {
        b[col] = d[col];        /* buffer the current row of matrix A */
        d[col] = 0;             /* (because it will be overwritten) */
      }                         /* and initialize a row of the result */
      for (i = B->rowcnt; --i >= 0; ) {
        t = b[i]; sb = B->els[i];
        for (col = B->colcnt; --col >= 0; )
          d[col] += t *sb[col]; /* multiply the row of matrix A */
      }                         /* with the matrix B and store the */
    } }                         /* result in the row of matrix A */
  else if (res == B) {          /* if matrix B is id. to the result, */
    b = res->buf;               /* get the (row) buffer of matrix B */
    for (col = B->colcnt; --col >= 0; ) {
      for (i = B->rowcnt; --i >= 0; )
        b[i] = B->els[i][col];  /* buffer the column of matrix B */
      for (row = A->rowcnt; --row >= 0; ) {
        sa = A->els[row];       /* traverse the rows of matrix A */
        d  = res->els[row]+col; /* get the element to compute */
        *d = 0;                 /* and initialize the result */
        for (i = A->colcnt; --i >= 0; )
          *d += sa[i] *b[i];    /* multiply the matrix A with the */
      }                         /* column of matrix B and store the */
    } }                         /* result in the column of matrix B */
  else {                        /* if the result differs from both */
    for (row = A->rowcnt; --row >= 0; ) {
      sa = A->els[row];         /* traverse the rows    of matrix A */
      d  = res->els[row];       /* traverse the columns of matrix B */
      for (col = B->colcnt; --col >= 0; )
        d[col] = 0;             /* initialize the result */
      for (i = B->rowcnt; --i >= 0; ) {
        t = sa[i]; sb = B->els[i];
        for (col = B->colcnt; --col >= 0; )
          d[col] += t *sb[col]; /* multiply the row of matrix A with */
      }                         /* the matrix B and store the result */
    }                           /* in the corr. row of the matrix res */
  }
  return res;                   /* return the resulting matrix */
}  /* mat_mul() */

/*--------------------------------------------------------------------*/

MATRIX* mat_sub (MATRIX *res, const MATRIX *mat, int row, int col)
{                               /* --- cut out a submatrix */
  int    i, k;                  /* loop variables */
  double *d; const double *s;   /* to traverse the matrix rows */

  assert(res && mat             /* check the function arguments */
     && (row >= 0) && (row < mat->rowcnt -res->rowcnt)
     && (col >= 0) && (row < mat->colcnt -res->colcnt));
  for (i = res->rowcnt; --i >= 0; ) {
    s = mat->els[i +row] +col; d = res->els[i];
    for (k = res->colcnt; --k >= 0; ) d[k] = s[k];
  }                             /* copy a rect. part of the matrix */
  return res;                   /* return the created submatrix */
}  /* mat_sub() */

/*--------------------------------------------------------------------*/

MATRIX* mat_subx (MATRIX *res, const MATRIX *mat,
                  int *rowids, int *colids)
{                               /* --- cut out a submatrix (extended) */
  int    i, k;                  /* loop variables */
  double *d; const double *s;   /* to traverse the matrix rows */

  assert(rowids && colids       /* check the function arguments */
      && res && mat && (res != mat)
      && (mat->rowcnt >= res->rowcnt)
      && (mat->colcnt >= res->colcnt));
  for (i = res->rowcnt; --i >= 0; ) {
    s = mat->els[rowids[i]]; d = res->els[i];
    for (k = res->colcnt; --k >= 0; ) d[k] = s[colids[k]];
  }                             /* copy a part of the matrix */
  return res;                   /* return the created submatrix */
}  /* mat_subx() */

/*----------------------------------------------------------------------
  Matrix Input/Output Functions
----------------------------------------------------------------------*/

int mat_write (const MATRIX *mat, FILE *file,
               const char *fmt, const char *sep)
{                               /* --- write a matrix to a file */
  int          row, col;        /* loop variables */
  const double *p;              /* to traverse the matrix rows */

  assert(mat && file && sep);   /* check the function arguments */
  for (row = 0; row < mat->rowcnt; row++) {
    p = mat->els[row];          /* traverse the matrix rows */
    for (col = 0; col < mat->colcnt; col++) {
      if (col > 0) fputc(sep[0], file);
      fprintf(file, fmt, p[col]);
    }                           /* print an element separator */
    fputs(sep +1, file);        /* and a matrix element; after */
  }                             /* each row print a row separator */
  return ferror(file) ? -1 : 0; /* check for an error */
}  /* mat_write() */

/*--------------------------------------------------------------------*/
#ifdef MAT_READ

int mat_read (MATRIX *mat, TFSCAN *tfscan, FILE *file)
{                               /* --- read a matrix from a file */
  TFSERR *err;                  /* error information of scanner */
  int    i, k;                  /* loop variable, buffer */

  assert(mat && tfscan && file);/* check the function arguments */
  for (i = k = 0; i < mat->rowcnt; i++) {
    k = vec_read(mat->els[i], mat->colcnt, tfscan, file);
    if (k != 0) break;          /* read the matrix rows */
  }
  if (k <= 0) return k;         /* return error code or 'ok' */
  err = tfs_err(tfscan);        /* get the error information */
  err->rec = i+1;               /* and fill it */
  return err->code = E_RECCNT;  /* return an error code */
}  /* mat_read() */

/*--------------------------------------------------------------------*/

static MATRIX* _materr (TFSCAN *tfs, int code, int rec, int exp,
                        MATRIX *mat)
{                               /* --- clean up on read error */
  TFSERR *err = tfs_err(tfs);   /* error information of scanner */

  if (mat) mat_delete(mat);     /* delete created matrix */
  if (code <= 0) err->code = code;
  if (rec  >= 0) err->rec  = rec;  /* fill the error */
  if (exp  >= 0) err->exp  = exp;  /* information structure */
  return NULL;                  /* return error result of mat_readx() */
}  /* _materr() */

/*--------------------------------------------------------------------*/

MATRIX* mat_readx (TFSCAN *tfscan, FILE *file, int rowcnt, int colcnt)
{                               /* --- read a matrix from a file */
  MATRIX *mat, *p;              /* created matrix, realloc. buffer */
  double *vec;                  /* a row vector */
  int    vsz;                   /* size of matrix vector */

  assert(tfscan && file);       /* check the function arguments */
  if (colcnt < 0) colcnt = 0;   /* eliminate negative values */
  vsz = (rowcnt > 0) ? rowcnt : BLKSIZE;
  mat = (MATRIX*)malloc(sizeof(MATRIX) +(vsz-1) *sizeof(double*));
  if (!mat) return _materr(tfscan, E_NOMEM, 1, 0, mat);
  mat->flags  = ROWBLKS;        /* create a matrix and */
  mat->rowcnt = 0;              /* initialize some variables */
  mat->buf    = NULL;           /* (for a proper cleanup on error) */
  mat->map    = NULL;
  while (1) {                   /* row vector read loop */
    vec = vec_readx(tfscan, file, &colcnt);
    if (!vec) {                 /* read the next vector */
      if (tfs_err(tfscan)->code == E_NONE) break;
      return _materr(tfscan, 1, mat->rowcnt+1, -1, mat);
    }                           /* check for error and end of matrix */
    if (mat->rowcnt >= vsz) {   /* if the row vector is full */
      if (rowcnt > 0)           /* if a fixed size was given, abort */
        return _materr(tfscan, E_RECCNT, mat->rowcnt+1, rowcnt, mat);
      vsz += (vsz > BLKSIZE) ? vsz >> 1 : BLKSIZE;
      p = (MATRIX*) realloc(mat, sizeof(MATRIX) +(vsz-1) *sizeof(double*));
      if (!p) return _materr(tfscan, E_NOMEM, mat->rowcnt+1, 0, mat);
      mat = (MATRIX*)p;         /* enlarge the matrix structure */
    }                           /* and set the new matrix */
    mat->els[mat->rowcnt++] = vec;
  }                             /* store the pattern read */
  mat->colcnt = colcnt;         /* set the number of columns */
  if (rowcnt > mat->rowcnt)     /* check the number of rows */
    return _materr(tfscan, E_RECCNT, mat->rowcnt, rowcnt, mat);
  if (mat->rowcnt == colcnt) {  /* if the matrix is square */
    mat->map = (int*)malloc(2 *colcnt *sizeof(int));
    if (!mat->map) return _materr(tfscan, E_NOMEM, colcnt, 0, mat);
  }                             /* create a permutation map */
  vsz = (colcnt > mat->rowcnt) ? colcnt : mat->rowcnt;
  mat->buf = (double*)malloc(vsz *sizeof(double));
  if (!mat->buf) return _materr(tfscan, E_NOMEM, mat->rowcnt, 0, mat);
  return mat;                   /* allocate a vector buffer and */
}  /* mat_readx() */            /* return the created matrix */

#endif
