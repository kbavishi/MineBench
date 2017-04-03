/*----------------------------------------------------------------------
  File    : radfn.c
  Contents: radial function management
            (for clustering, learning vector quantization etc.)
  Author  : Christian Borgelt
  History : 15.08.2003 file created from file cluster1.c
----------------------------------------------------------------------*/
#include <math.h>
#include "gamma.h"
#include "radfn.h"

/*----------------------------------------------------------------------
  Preprocessor Definitions
----------------------------------------------------------------------*/
#define	M_PI         3.14159265358979323846  /* \pi */
#define MINDENOM     1e-12      /* minimal value of denominator */

/*----------------------------------------------------------------------
  Functions
----------------------------------------------------------------------*/

double rf_cauchy (double d2, double *params)
{                               /* --- (generalized) Cauchy function */
  double t;                     /* temporary buffer */

  if (d2 < 0) {                 /* if to the get normalization factor */
    if ((params[0] <= -d2) || (params[1] <= 0) || (params[2] <= 0))
      return 0;                 /* check whether the integral exists */
    t = -d2 /params[0]; d2 *= -0.5;
    return (gamma(d2) *params[0] *params[2] *sin(t *M_PI))
         / (2 *pow(M_PI, d2+1) *pow(params[2] /params[0], t));
  }                             /* return the normalization factor */
  if (params[0] != 2)           /* raise to the given power */
    d2 = pow(d2, 0.5*params[0]);/* (note that d2 is already squared) */
  d2 = d2 *params[1] +params[2];/* apply linear transformation */
  return (d2 > MINDENOM) ? 1/d2 : 1/MINDENOM;
}  /* rf_cauchy() */            /* compute Cauchy function */

/*----------------------------------------------------------------------
f_cauchy(d) = 1/(a_1 *d^{a_0} +a_2), default: 1/d^2
For retrieving the normalization factor, it must be
d2 = -number of dimensions of the data space.
----------------------------------------------------------------------*/

double rf_gauss (double d2, double *params)
{                               /* --- (general.) Gaussian function */
  double t;                     /* temporary buffer */

  if (d2 < 0) {                 /* if to the get normalization factor */
    if ((params[0] <= 0) || (params[1] <= 0))
      return 0;                 /* check whether the integral exists */
    t = -d2 /params[0]; d2 *= -0.5;
    return (gamma(d2) *params[0] *pow(0.5 *params[1], t))
         / (2 *pow(M_PI, d2) *exp(-0.5 *params[2]) *gamma(t));
  }                             /* return the normalization factor */
  if (params[0] != 2)           /* raise to the given power */
    d2 = pow(d2, 0.5*params[0]);/* (note that d2 is already squared) */
  d2 = d2 *params[1] +params[2];/* apply linear transformation */
  return exp(-0.5 *d2);         /* compute Gaussian function */
}  /* rf_gauss() */

/*----------------------------------------------------------------------
f_gauss(d) = exp(-0.5 *(a_1 *d^{a_0} +a_2)), default: exp(-0.5 *d^2)
For retrieving the normalization factor, it must be
d2 = -number of dimensions of the data space.
----------------------------------------------------------------------*/
