/*----------------------------------------------------------------------
  File    : matrix3.c
  Contents: general vector and matrix management
            eigenvalue and eigenvector functions
  Author  : Christian Borgelt
  History : 18.05.1999 file created
----------------------------------------------------------------------*/
#include <stdio.h>
#include <limits.h>
#include <float.h>
#include <assert.h>
#include "matrix.h"

/*----------------------------------------------------------------------
  Auxiliary Functions
----------------------------------------------------------------------*/
#ifndef hypot

static double hypot (double a, double b)
{                               /* --- length of hypotenuse */
  double x, y, t;               /* absolute values and temp. buffer */

  x = fabs(a); y = fabs(b);     /* compute the absolute values */
  if      (x > y) { t = y/x; return x *sqrt(1+t*t); }
  else if (y > x) { t = x/y; return y *sqrt(1+t*t); }
  return 0;                     /* compute the hypotenuse length */
}  /* hypot() */                /* avoiding underflow or overflow */

#endif
/*----------------------------------------------------------------------
  Eigenvalue and Eigenvector Functions
----------------------------------------------------------------------*/

int mat_jacobi (MATRIX *mat, int mode, double *eival, double **eivec)
{
}  /* mat_jacobi() */

/*--------------------------------------------------------------------*/

int mat_3dred (const MATRIX *mat, MATRIX *otm,
               double *diag, double *subdiag)
{
}  /* mat_3dred() */

/*--------------------------------------------------------------------*/

int mat_3dqli (const double *diag, const double *subdiag, int n,
               double *eival, double **eivec)
{
}  /* mat_3dqli() */

/*--------------------------------------------------------------------*/

void mat_bal (MATRIX *res, const MATRIX *mat)
{
}  /* mat_bal() */

/*--------------------------------------------------------------------*/

void mat_heselm (MATRIX *res, const MATRIX *mat)
{
}  /* mat_heselm() */

/*--------------------------------------------------------------------*/

void mat_hesqr (MATRIX *mat, int mode, double *real, double *img)
{
}  /* mat_hesqr() */
