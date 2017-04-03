/*----------------------------------------------------------------------
  File    : mvnorm.c
  Contents: Multivariate normal distribution estimation and management
  Author  : Christian Borgelt
  History : 10.11.2000 file created from files correl.c and cluster.c
            26.11.2000 first version completed
            24.05.2001 possibilistic parameter added
            26.05.2001 handling of roundoff errors improved
            15.07.2001 some assertions added
----------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#ifdef MVN_PARSE
#include <string.h>
#endif
#include <math.h>
#include <assert.h>
#include "mvnorm.h"
#ifdef STORAGE
#include "storage.h"
#endif

/*----------------------------------------------------------------------
  Preprocessor Definitions
----------------------------------------------------------------------*/
#define	M_PI  3.14159265358979323846  /* \pi */

#ifdef MVN_PARSE

/* --- error codes --- */
#define E_CHREXP    (-16)       /* character expected */
#define E_NUMEXP    (-17)       /* number expected */
#define E_ILLNUM    (-18)       /* illegal number */

/* --- functions for parser --- */
#define ERROR(c)    return _paerr(scan, c,       -1)
#define ERR_CHR(c)  return _paerr(scan, E_CHREXP, c)
#define GET_TOK()   if (sc_next(scan) < 0) \
                      return sc_error(scan, sc_token(scan))
#define GET_CHR(c)  if (sc_token(scan) != (c)) ERR_CHR(c); \
                    else GET_TOK();

#endif  /* #ifdef MVN_PARSE */
/*----------------------------------------------------------------------
  Constants
----------------------------------------------------------------------*/
#ifdef MVN_PARSE
#ifdef GERMAN                   /* deutsche Texte */
static const char *errmsgs[] = {/* Fehlermeldungen */
  /* E_CHREXP  -16 */  "\"%c\" erwartet statt %s",
  /* E_NUMEXP  -17 */  "Zahl erwartet statt %s",
  /* E_ILLNUM  -18 */  "ungültige Zahl %s",
};
#else                           /* English texts */
static const char *errmsgs[] = {/* error messages */
  /* E_CHREXP  -16 */  "\"%c\" expected instead of %s",
  /* E_NUMEXP  -17 */  "number expected instead of %s",
  /* E_ILLNUM  -18 */  "illegal number %s",
};
#endif  /* #ifdef GERMAN .. #else .. */
#endif  /* #ifdef MVN_PARSE */

/*----------------------------------------------------------------------
  Auxiliary Functions
----------------------------------------------------------------------*/

static double _normd (double drand (void))
{                               /* --- compute N(0,1) distrib. number */
  static double b;              /* buffer for random number */
  double x, y, r;               /* coordinates and radius */

  if (b != 0.0) {               /* if the buffer is full, */
    x = b; b = 0; return x; }   /* return the buffered number */
  do {                          /* pick a random point */
    x = 2.0*drand()-1.0;        /* in the unit square [-1,1]^2 */
    y = 2.0*drand()-1.0;        /* and check whether it lies */
    r = x*x +y*y;               /* inside the unit circle */
  } while ((r > 1) || (r == 0));
  r = sqrt(-2*log(r)/r);        /* factor for Box-Muller transform */
  b = x *r;                     /* save one of the random numbers */
  return y *r;                  /* and return the other */
}  /* _normd() */

/*--------------------------------------------------------------------*/

static int _decom (MVNORM *mvn)
{                               /* --- Cholesky decomposition */
  int    i, j, k;               /* loop variables */
  double *s, *r;                /* to traverse the matrix rows */
  double t, q;                  /* temporary buffers */

  assert(mvn);                  /* check the function argument */
  for (i = 0; i < mvn->size; i++) {
    s = mvn->decom[i];          /* traverse the matrix rows */
    for (t = mvn->covs[i][k = i]; --k >= 0; )
      t -= s[k] *s[k];          /* compute square of diag. element */
    if (t <= 0) return -1;      /* matrix is not positive definite */
    s[i] = t = sqrt(t);         /* store the diagonal element */
    if (t <= 0) return -1;      /* matrix is not positive definite */
    mvn->buf[i] = q = 1/t;      /* compute the corresponding factor */
    for (j = i; ++j < mvn->size; ) {
      r = mvn->decom[j];        /* traverse the row's other columns */
      for (t = mvn->covs[k = j][i]; --k >= 0; )
        t -= s[k] *r[k];        /* compute the off-diagonal elements */
      r[i] = t *q;              /* column by column */
    }                           /* (-> lower triangular matrix) */
  }
  for (t = mvn->decom[0][0], i = mvn->size; --i > 0; )
    t *= mvn->decom[i][i];      /* multiply the diagonal elements */
  mvn->det = t*t;               /* to compute the determinant */
  if (mvn->det <= 0) return -1; /* check for a positive determinant */
  return 0;                     /* return 'ok' */
}  /* _decom() */

/*--------------------------------------------------------------------*/

static int _inv (MVNORM *mvn)
{                               /* --- invert matrix from Cholesky d. */
  int    i, k, n;               /* loop variables */
  double *d;                    /* reciprocals of diagonal elements */
  double *s, *r;                /* to traverse the matrix elements */
  double t;                     /* temporary buffer */

  assert(mvn);                  /* check the function argument */
  d = mvn->buf;                 /* get reciprocals of diag. elements */
  for (n = mvn->size; --n >= 0; ) {
    r = mvn->diff;              /* traverse the unit vectors */
    r[n] = d[n];                /* and compute their originals */
    for (i = n; ++i < mvn->size; ) {
      s = mvn->decom[i];        /* traverse the matrix rows */
      for (t = 0, k = i; --k >= n; )
        t -= r[k] *s[k];        /* do forward substitution with */
      r[i] = t *d[i];           /* the lower triangular matrix L */
    }                           /* (store result in r = mvn->diff) */
    for (i = mvn->size; --i >= n; ) {
      for (t = r[i], k = mvn->size; --k > i; )
        t -= mvn->decom[k][i] *mvn->inv[k][n];
      mvn->inv[i][n] = t*d[i];  /* do backward substitution with */
    }                           /* the upper triangular matrix L^T */
  }                             /* (using only the lower triangle) */
  mvn->norm = 1/sqrt(mvn->det *pow(2*M_PI, mvn->size));
  return 0;                     /* compute normalization factor */
}  /* _inv() */                 /* and return 'ok' */

/*----------------------------------------------------------------------
  Main Functions
----------------------------------------------------------------------*/

static MVNORM* _create (int size)
{                               /* --- create a normal distribution */
  int    i;                     /* loop variable */
  MVNORM *mvn;                  /* created normal distribution */
  MVNROW **row;                 /* to traverse the matrix rows */
  double *p;                    /* to traverse the matrix rows */

  if (size <= 0) size = 1;      /* check and adapt the matrix size */
  mvn = (MVNORM*)calloc(1, sizeof(MVNORM) +(size-1) *sizeof(MVNROW*));
  if (!mvn) return NULL;        /* create the base structure */
  mvn->size  = size;            /* and store the matrix size */
  mvn->covs  = (double**)malloc(4*size *sizeof(double*));
  if (!mvn->covs) { free(mvn); return NULL; }
  mvn->corrs = mvn->covs  +size;   /* create vectors of pointers */
  mvn->decom = mvn->corrs +size;   /* for matrix rows */
  mvn->inv   = mvn->decom +size;
  mvn->exps  = (double*)malloc((2*size*size +5*size) *sizeof(double));
  if (!mvn->exps) { mvn_delete(mvn); return NULL; }
  p = mvn->exps +size;          /* create and organize data vectors */
  for (i = size; --i >= 0; ) { mvn->covs[i]  = p; p += i+1; }
  for (i = size; --i >= 0; ) { mvn->corrs[i] = p; p += i+1; }
  for (i = size; --i >= 0; ) { mvn->decom[i] = p; p += i+1; }
  for (i = size; --i >= 0; ) { mvn->inv[i]   = p; p += i+1; }
  mvn->diff = p;                /* set buffers for difference from */
  mvn->buf  = p +size;          /* center and intermediate results */
  for (row = mvn->rows +size; --size >= 0; ) {
    i = (size > 0) ? size-1 : 0;
    *--row = (MVNROW*)malloc(sizeof(MVNROW) +i *sizeof(MVNELEM));
    if (!*row) { mvn_delete(mvn); return NULL; }
  }                             /* create the statistics rows */
  return mvn;                   /* and return it */
}  /* _create() */

/*--------------------------------------------------------------------*/

MVNORM* mvn_create (int size)
{                                /* --- create a normal distribution */
  MVNORM *mvn;                   /* created normal distribution */

  mvn = _create(size);           /* create a normal distribution */
  if (!mvn) return NULL;
  mvn_clear(mvn);                /* clear the created normal dist. */
  return mvn;                    /* and return it */
}  /* mvn_create() */

/*--------------------------------------------------------------------*/

MVNORM* mvn_dup (const MVNORM *mvn)
{                               /* --- duplicate a normal distrib. */
  int     i, k;                 /* loop variables */
  MVNORM  *dup;                 /* created duplicate */
  MVNROW  *dr; const MVNROW  *sr;  /* to traverse the matrix rows */
  MVNELEM *de; const MVNELEM *se;  /* to traverse the matrix elements */
  double  *d;  const double  *s;   /* to traverse the parameters */

  assert(mvn);                  /* check the function argument */
  dup = _create(mvn->size);     /* create a normal distribution */
  if (!dup) return NULL;        /* of the same size */
  dup->det  = mvn->det;         /* copy the determinant */
  dup->norm = mvn->norm;        /* and the normalization factor */
  for (i = mvn->size; --i >= 0; ) {
    sr = mvn->rows[i];          /* traverse the matrix rows */
    dr = dup->rows[i];          /* of source and destination */
    *dr = *sr;                  /* and copy them */
    se = sr->elems +i;          /* traverse the row elements */
    de = dr->elems +i;          /* and copy them */
    for (k = i; --k >= 0; ) *--de = *--se;
  }
  i = 2*mvn->size*mvn->size +5*mvn->size;
  s = mvn->exps +i;             /* traverse and copy the vectors of */
  d = dup->exps +i;             /* estimated parameters, the Cholesky */
  while (--i >= 0) *--d = *--s; /* decomposition, and the inverse */
  return dup;                   /* return the created duplicate */
}  /* mvn_dup() */

/*--------------------------------------------------------------------*/

void mvn_delete (MVNORM *mvn)
{                               /* --- delete a normal distribution */
  int    i;                     /* loop variable */
  MVNROW **row;                 /* to traverse the matrix rows */

  assert(mvn);                  /* check the function argument */
  for (row = mvn->rows +(i = mvn->size); --i >= 0; )
    if (*--row) free(*row);     /* delete all statistics rows, */
  if (mvn->exps) free(mvn->exps);    /* the parameter vectors, */
  if (mvn->covs) free(mvn->covs);    /* the pointer vectors, */
  free(mvn);                         /* and the base structure */
}  /* mvn_delete() */

/*--------------------------------------------------------------------*/

void mvn_clear (MVNORM *mvn)
{                               /* --- clear a normal distribution */
  int     i, k;                 /* loop variables */
  MVNROW  *row;                 /* to traverse the matrix rows */
  MVNELEM *e;                   /* to traverse the matrix elements */

  assert(mvn);                  /* check the function argument */
  for (i = mvn->size; --i >= 0; ) {
    row = mvn->rows[i];         /* traverse the matrix rows and */
    row->cnt = row->sv = row->sv2 = 0;        /* clear the sums */
    for (e = row->elems +(k = i); --k >= 0; ) {
      (--e)->cnt = 0; e->sr = e->sr2 = e->sc = e->sc2 = e->src = 0; }
  }                             /* clear all sums of the */
}  /* mvn_clear() */            /* matrix elements */

/*--------------------------------------------------------------------*/

void mvn_add (MVNORM *mvn, const double vals[], double cnt)
{                               /* --- add a value vector */
  int     i, k;                 /* loop variables */
  MVNROW  *r1, *r2;             /* to traverse the matrix rows */
  MVNELEM *e;                   /* to traverse the matrix elements */

  assert(mvn && vals && (cnt >= 0)); /* check the function arguments */
  for (i = 0; i < mvn->size; i++) {
    if (vals[i] <= MVN_UNKNOWN) /* traverse the matrix rows, */
      continue;                 /* but skip those rows, for */
    r1 = mvn->rows[i];          /* which the value is unknown */
    r1->cv   = cnt    *vals[i]; /* precompute number of cases * value */
    r1->cv2  = r1->cv *vals[i]; /* and number of cases *value^2 */
    r1->cnt += cnt;             /* update the terms that are needed */
    r1->sv  += r1->cv;          /* to compute the expected value */
    r1->sv2 += r1->cv2;         /* and the variance */
    for (e = r1->elems +(k = i); --k >= 0; ) {
      if (vals[k] <= MVN_UNKNOWN)  /* traverse the preceding rows */
        continue;               /* but skip those rows, for */
      r2 = mvn->rows[k];        /* which the value is unknown */
      (--e)->cnt += cnt;
      e->sr  += r1->cv; e->sr2 += r1->cv2;
      e->sc  += r2->cv; e->sc2 += r2->cv2;
      e->src += r1->cv *vals[k];
    }                           /* update the terms needed */
  }                             /* to compute the covariance */
}  /* mvn_add() */

/*--------------------------------------------------------------------*/

int mvn_calc (MVNORM *mvn, int mode)
{                               /* --- calc. parameters from data */
  int     i, k;                 /* loop variables */
  MVNROW  *row;                 /* to traverse the matrix rows */
  MVNELEM *e;                   /* to traverse the matrix elements */
  double  *x, *v, *c;           /* to traverse the estim. parameters */
  double  cnt, t;               /* number of cases (-1), buffer */

  assert(mvn);                  /* check the function argument */
  if (mode & MVN_EXPVAR) {      /* expected values and variances */
    for (x = mvn->exps +(i = mvn->size); --i >= 0; ) {
      row  = mvn->rows[i];      /* traverse the statistics rows */
      *--x = (row->cnt > 0)     /* compute the expected value */
           ?  row->sv /row->cnt : 0;
      t    = row->sv2 -*x *row->sv;
      cnt  = (mode & MVN_MAXLLH) ? row->cnt : (row->cnt -1);
      mvn->covs[i][i] = ((t > 0) && (cnt > 0)) ? t /cnt : 0;
    }                           /* compute the variance */
  }
  if (mode & MVN_COVAR) {       /* if to compute covariances */
    for (x = mvn->exps, i = 0; i < mvn->size; i++) {
      row = mvn->rows[i];       /* traverse the statistics rows */
      v   = mvn->covs[i] +i;    /* and the covariance rows */
      if (*v <= 0) {            /* if the variance is zero */
        for (k = i; --k >= 0; ) *--v = 0;
        continue;               /* set all covariances to zero */
      }                         /* (to avoid inconsistencies) */
      for (e = row->elems +(k = i); --k >= 0; ) {
        cnt  = (mode & MVN_MAXLLH) ? (--e)->cnt : ((--e)->cnt -1);
        *--v = (cnt > 0)        /* compute the covariance */
             ? (e->src -x[k] *e->sr -x[i] *e->sc
               +e->cnt *x[i] *x[k]) /cnt : 0;
      }                         /* (this somewhat complicated formula */
    }                           /* takes missing values into account) */
  }
  if (mode & MVN_CORREL) {      /* correlation coefficients */
    for (i = 0; i < mvn->size; i++) {
      v  = mvn->covs[i];        /* traverse the covariances and */
      c  = mvn->corrs[i] +i;    /* the correlation coefficients */
      *c = 1;                   /* the correlation coefficient of */
      for (k = i; --k >= 0; ) { /* a variable with itself is 1 */
        t    = v[i] *mvn->covs[k][k];
        t    = (t > 0) ? sqrt(t) : 0;
        *--c = (t > 0) ? v[k] /t : 0;
      }                         /* compute the correlation coeffs. */
    }                           /* (set correl. coeff. to 'unknown' */
  }                             /* if the covariance is unknown) */
  if (mode & MVN_DECOM)         /* do Cholesky decomposition */
    if (_decom(mvn) != 0) return -1;
  if (mode & MVN_INVERSE)       /* compute inverse from Cholesky d. */
    if (_inv(mvn)   != 0) return -1;
  return 0;                     /* return 'ok' */
}  /* mvn_calc() */

/*--------------------------------------------------------------------*/

double mvn_eval (MVNORM *mvn, const double vals[])
{                               /* --- evaluate a normal distribution */
  int    i, k;                  /* loop variables */
  double *d, *x, *p;            /* to traverse vectors and matrices */
  double t;                     /* temporary buffer for exponent */

  assert(mvn && vals);          /* check the function arguments */
  d = mvn->diff +mvn->size;     /* traverse the data values and the */
  x = mvn->exps +mvn->size;     /* corresponding expected values */
  for (vals += (i = mvn->size); --i >= 0; )
    *--d = *--vals - *--x;      /* compute the difference vector d */
  p = mvn->buf +mvn->size;      /* get buffer for intermediate result */
  for (i = mvn->size; --i >= 0; ) {
    *--p = 0;                   /* traverse the matrix columns */
    for (d += k = mvn->size; --k > i; )
      *p += *--d * mvn->inv[k][i];
    for (x = mvn->inv[i] +(k = i+1); --k >= 0; )
      *p += *--d * *--x;        /* calc. product of the diff. vector */
  }                             /* with a column of the inverse */
  d += mvn->size;               /* traverse the diff. vector d again */
  for (t = 0, p += i = mvn->size; --i >= 0; )
    t += *--d * *--p;           /* calc. product with d^T * \Sigma^-1 */
  return mvn->norm *exp(-0.5 *t);      /* return the value */
}  /* mvn_eval() */             /* of the density function */

/*--------------------------------------------------------------------*/

double* mvn_rand (MVNORM *mvn, double drand (void))
{                               /* --- generate random sample point */
  int    i, k;                  /* loop variables */
  double *b, *r;                /* to access the buffer/matrix rows */

  for (b = mvn->buf +(i = mvn->size); --i >= 0; )
    *--b = _normd(drand);       /* generate points from N(0,1)^n */
  for (i = mvn->size; --i >= 0; ) {
    r = mvn->decom[i] +i;       /* traverse the matrix rows */
    b[i] *= *r;                 /* multiply the random vector */
    for (k = i; --k >= 0; )     /* with the Cholesky decomposed */
      b[i] += *--r *b[k];       /* covariance matrix (transposed) */
  }                             /* -> hyperellipsoidal norm. dist. */
  for (b += (i = mvn->size); --i >= 0; )
    *--b += mvn->exps[i];       /* add the expected value vector */
  return b;                     /* return the created sample point */
}  /* mvn_rand() */

/*--------------------------------------------------------------------*/

int mvn_desc (MVNORM *mvn, FILE *file, int offs, int maxlen)
{                               /* --- describe a normal distribution */
  int    i, k;                  /* loop variables */
  double *p;                    /* to traverse the matrix rows */

  assert(mvn && file);          /* check the function arguments */ 

  /* --- expected values --- */
  if (offs > 0) {               /* if the offset is positive, */
    for (k = offs; --k >= 0; )  /* indent the first line */
      fputc(' ', file);         /* to the given offset */
  }                             /* (indent all but the first line */
  offs = abs(offs);             /* if the offset is negative) */
  fputc('[', file);             /* start the expected values vector */
  for (p = mvn->exps, i = 1; i < mvn->size; i++)
    fprintf(file, "%g, ", *p++);/* print the expected values and */
  fprintf(file, "%g],\n", *p);  /* then terminate the vector */

  /* --- (co)variances --- */
  for (i = 0; i < mvn->size; i++) {
    if (i > 0)                  /* if this is not the first row, */
      fputs(",\n", file);       /* print a comma and start new line */
    for (k = offs; --k >= 0; )  /* indent the line */
      fputc(' ', file);         /* to the given offset */
    fputc('[', file);           /* start a new matrix row */
    for (p = mvn->covs[i], k = 0; k < i; k++)
      fprintf(file, "%g, ", *p++);      /* print the covariances */
    fprintf(file, "%g]", *p);   /* and then terminate the vector */
  }                             /* by printing the variance */

  return ferror(file) ? -1 : 0; /* return file write status */
}  /* mvn_desc() */

/*--------------------------------------------------------------------*/
#ifdef MVN_PARSE

static int _paerr (SCAN *scan, int code, int c)
{                               /* --- report a parse error */
  char src[256], dst[1024];     /* buffers for formatting */

  strncpy(src, sc_value(scan), 255); src[255] = '\0';
  sc_format(dst, src, 1);       /* copy and format the token value */
  if (code == E_CHREXP) return sc_error(scan, code, c, dst);
  else                  return sc_error(scan, code,    dst);
}  /* _paerr() */               /* print an error message */

/*--------------------------------------------------------------------*/

static int _get_exps (MVNORM *mvn, SCAN *scan)
{                               /* --- parse expected values */
  int    i;                     /* loop variable */
  MVNROW *row;                  /* to traverse the matrix rows */
  double t;                     /* temporary buffer */

  assert(mvn && scan);          /* check the function arguments */
  GET_CHR('[');                 /* consume '(' */
  for (i = 0; i < mvn->size; i++) {
    if (i > 0) {GET_CHR(',');}  /* check for a comma and consume it */
    if (sc_token(scan) != T_NUM) ERROR(E_NUMEXP);
    mvn->exps[i] = t = atof(sc_value(scan));
    GET_TOK();                  /* get and consume expected value */
    row = mvn->rows[i];         /* recompute the cum of values */
    row->sv = t *row->cnt;      /* from the expected value */
  }                             /* and the number of cases */
  GET_CHR(']');                 /* consume ')' */
  GET_CHR(',');                 /* consume ',' */
  return 0;                     /* return 'ok' */
}  /* _get_exps() */

/*--------------------------------------------------------------------*/

static int _get_covs (MVNORM *mvn, SCAN *scan)
{                               /* --- parse (co)variances */
  int     i, k;                 /* loop variables */
  MVNROW  *row;                 /* to traverse the matrix rows */
  MVNELEM *e;                   /* to traverse the matrix elements */
  double  *v;                   /* to traverse the (co)variances */

  assert(mvn && scan);          /* check the function arguments */
  for (i = 0; i < mvn->size; i++) {
    row = mvn->rows[i];         /* traverse the matrix rows */
    if (i > 0) {GET_CHR(',');}  /* check for a comma between rows */
    GET_CHR('[');               /* consume '(' */
    e = row->elems;             /* traverse the statistics and */
    v = mvn->covs[i];           /* the covariances of each row */
    for (k = 0; k < i; e++, v++, k++) {
      if (sc_token(scan) != T_NUM) ERROR(E_NUMEXP);
      *v = atof(sc_value(scan));/* get the covariance */
      e->src = *v *(row->cnt -1) +row->sv *mvn->exps[k];
      GET_TOK();                /* recompute mixed sum, */
      GET_CHR(',');             /* consume covariance, */
    }                           /* and consume ',' */
    if (sc_token(scan) != T_NUM) ERROR(E_NUMEXP);
    *v = atof(sc_value(scan));  /* get and check the variance */
    if (*v < 0)                  ERROR(E_ILLNUM);
    GET_TOK();                  /* consume variance */
    GET_CHR(']');               /* consume ')' */
    row->sv2 = *v *(row->cnt -1) +row->sv *mvn->exps[i];
    if (row->sv2 < 0) row->sv2 = 0;
    for (k = i; --k >= 0; ) {   /* recompute sum of squares and */
      --e;                      /* traverse the elements again */
      e->cnt = row->cnt;        /* set the number of cases */
      e->sr  = row->sv;  e->sc  = mvn->rows[k]->sv;
      e->sr2 = row->sv2; e->sc2 = mvn->rows[k]->sv2;
    }                           /* get the sums of values and */
  }                             /* the sums of squared values */
  return 0;                     /* return 'ok' */
}  /* _get_covs() */

/*--------------------------------------------------------------------*/

static int _get_poss (MVNORM *mvn, SCAN *scan)
{                               /* --- get possibilistic parameter */
  assert(mvn && scan);          /* check the function arguments */
  if (sc_token(scan) != ',') {  /* if no comma follows, */
    mvn->poss = 1; return 0; }  /* set a default value */
  GET_TOK();                    /* consume ',' */
  if (sc_token(scan) != T_NUM) ERROR(E_NUMEXP);
  mvn->poss = atof(sc_value(scan));
  if (mvn->poss <= 0)          ERROR(E_ILLNUM);
  GET_TOK();                    /* get and consume the parameter */
  return 0;                     /* return 'ok' */
}  /* _get_poss() */

/*--------------------------------------------------------------------*/

int mvn_parse (MVNORM *mvn, SCAN *scan, double cnt)
{                               /* --- parse a normal distribution */
  int    i;                     /* loop variable, function result */
  MVNROW **row;                 /* to traverse the matrix rows */

  assert(mvn && scan);          /* check the function arguments */
  if (cnt < 2) cnt = 2;         /* adapt the number of cases */
  for (row = mvn->rows +(i = mvn->size); --i >= 0; )
    (*--row)->cnt = cnt;        /* set number of cases */
  sc_errmsgs(scan, errmsgs, (int)(sizeof(errmsgs)/sizeof(const char*)));
  i = _get_exps(mvn, scan);     /* read the expected values */
  if (i < 0) return i;
  i = _get_covs(mvn, scan);     /* read the (co)variances */
  if (i < 0) return i;
  i = _get_poss(mvn, scan);     /* read the possibilistic parameter */
  if (i < 0) return i;
  return 0;                     /* return 'ok' */
}  /* mvn_parse() */

#endif
