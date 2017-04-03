/*----------------------------------------------------------------------
  File    : zeta.c
  Contents: compute Riemann's zeta-function
  Author  : Christian Borgelt
  History : 20.10.1998 file created
----------------------------------------------------------------------*/
#include <math.h>
#include "zeta.h"

/*----------------------------------------------------------------------
  Functions
----------------------------------------------------------------------*/

double zeta (double x)
{                               /* --- compute Riemann's zeta-function */
  double t = 1, z = 0;          /* term to add and result */
  double base = 2;              /* base for terms */

  do {                          /* compute the sum */
    z += t;                     /* zeta(x) = \sum_{n=1}^\infty n^{-x} */
    t = pow(base++, -x);        /* by successively adding terms */
  } while (z +t > z);           /* until a terms gets zero */
  return z;                     /* return the function value */
}  /* zeta() */
