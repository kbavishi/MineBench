/*----------------------------------------------------------------------
  File    : choose.c
  Contents: compute n choose k
  Author  : Christian Borgelt
  History : 29.04.1996 file created
            17.05.2003 (n-i+1) replaced by n--
----------------------------------------------------------------------*/
#include <limits.h>
#include "choose.h"

/*----------------------------------------------------------------------
  Functions
----------------------------------------------------------------------*/

unsigned int choose (unsigned int n, unsigned int k)
{                               /* --- compute n choose k */
  unsigned int i, t;            /* loop variable, buffer */
  unsigned int r = 1;           /* result */

  if (k > n) return 0;          /* check range of k */
  for (i = 1; i <= k; i++) {    /* calculation loop */
    t = n--;                    /* calculate next factor in numerator */
    if (UINT_MAX /t < r)        /* if result of multiplication */
      return 0;                 /* is out of range, abort */
    r = (r *t) /i;              /* calculate \prod_{i=1}^k (n-i+1)/i */
  }
  return r;                     /* return result */
}  /* choose() */

/*--------------------------------------------------------------------*/

double dchoose (unsigned int n, unsigned int k)
{                               /* --- compute n choose k */
  unsigned int i;               /* loop variable, buffer */
  double r = 1.0;               /* result */

  if (k > n) return 0;          /* check range of k */
  for (i = 1; i <= k; i++)      /* calculation loop */
    r = (r *n--) /i;            /* calculate \prod_{i=1}^k (n-i+1)/i */
  return r;                     /* return result */
}  /* dchoose() */
