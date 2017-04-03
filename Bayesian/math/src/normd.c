/*----------------------------------------------------------------------
  File    : normd.c
  Contents: computation of normally distributed random numbers
  Author  : Christian Borgelt
  History : 04.11.2002 file created
----------------------------------------------------------------------*/
#include <math.h>
#include "normd.h"

/*----------------------------------------------------------------------
  Functions
----------------------------------------------------------------------*/

double normd (double randfn (void))
{                               /* --- compute N(0,1) distrib. number */
  static double b;              /* buffer for random number */
  double x, y, r;               /* coordinates and radius */

  if (b != 0.0) {               /* if the buffer is full, */
    x = b; b = 0; return x; }   /* return the buffered number */
  do {                          /* pick a random point */
    x = 2.0*randfn()-1.0;       /* in the unit square [-1,1]^2 */
    y = 2.0*randfn()-1.0;       /* and check whether it lies */
    r = x*x +y*y;               /* inside the unit circle */
  } while ((r > 1) || (r == 0));
  r = sqrt(-2*log(r)/r);        /* factor for Box-Muller transform */
  b = x *r;                     /* save one of the random numbers */
  return y *r;                  /* and return the other */
}  /* normd() */

/*----------------------------------------------------------------------
Source for the polar method to generate normally distributed numbers:
D.E. Knuth.
The Art of Computer Programming, Vol. 2: Seminumerical Algorithms
Addison-Wesley, Reading, MA, USA 1998
pp. 122-123
----------------------------------------------------------------------*/
