/*----------------------------------------------------------------------
  File    : quantile.c
  Contents: compute quantiles of normal and chi^2 distribution
  Author  : Christian Borgelt
  History : 19.05.2003 file created
----------------------------------------------------------------------*/
#include <math.h>
#include <assert.h>
#include "gamma.h"
#include "quantile.h"

/*----------------------------------------------------------------------
  Preprocessor Definitions
----------------------------------------------------------------------*/
#define NQ_A0    ( -0.322232431088)      /* coefficients of */
#define NQ_A1    ( -1)                   /* the polynomials */
#define NQ_A2    ( -0.342242088547)      /* used to compute */
#define NQ_A3    ( -0.0204231210245)     /* an approximation of */
#define NQ_A4    ( -0.453642210148e-4)   /* the quantiles of the */
#define NQ_B0       0.0993484626060      /* normal distribution */
#define NQ_B1       0.588581570495
#define NQ_B2       0.531103462366
#define NQ_B3       0.103537752850
#define NQ_B4       0.0038560700634
#define NQ_PMIN     1e-20                /* minimal probability */

#define LN_2        0.69314718055994530942  /* ln(2) */

/*----------------------------------------------------------------------
  Functions
----------------------------------------------------------------------*/

double ndqtl (double prob)
{                               /* --- comp. quantile of normal dist. */
  double p, x, z;               /*     with mean 0 and variance 1 */

  assert((prob >= 0)            /* check the function argument */
      && (prob <= 1));          /* (a probability must be in [0,1]) */
  if (prob == 0.5) return 0;    /* treat prob = 0.5 as a special case */
  p = (prob > 0.5) ? 1-prob : prob; /* transform to left tail if nec. */
  if (p < NQ_PMIN) return (prob < 0.5) ? -10 : 10;
  x = sqrt(log(1/(p*p)));       /* compute quotient of polynomials */
  z = x + ((((NQ_A4 *x +NQ_A3) *x +NQ_A2) *x +NQ_A1) *x +NQ_A0)
        / ((((NQ_B4 *x +NQ_B3) *x +NQ_B2) *x +NQ_B1) *x +NQ_B0);
  return (prob < 0.5) ? -z : z; /* retransform to right tail if nec. */
}  /* ndqtl() */

/*----------------------------------------------------------------------
References: - R.E. Odeh and J.O. Evans.
              The percentage points of the normal distribution.
              Applied Statistics 22:96--97, 1974
            - J.D. Beasley and S.G. Springer.
              Algorithm AS 111:
              The percentage points of the normal distribution.
              Applied Statistics 26:118--121, 1977
            - M.J. Wichura
              Algorithm AS 241:
              The percentage points of the normal distribution.
              Applied Statistics 37:477--484, 1988
----------------------------------------------------------------------*/

double c2qtl (double prob, double df)
{                               /* --- compute quantile of chi2 dist. */
  double e = 0.5e-6, p = prob, g;
  double y, c, ch, a = 0, q = 0, p1 = 0, p2 = 0, t = 0, x = 0, b = 0;
  double s1, s2, s3, s4, s5, s6;
	
  assert(df > 0);               /* check the function arguments */
  if (prob <   2e-6) prob =   2e-6;
  if (prob > 1-2e-6) prob = 1-2e-6;

  y = df/2; c = y-1;
  g = logGa(y);
  if      (df < -1.24 *log(p)) {
    ch = pow(p *y *exp(g +y *LN_2), 1.0/y);
    if (ch -e < 0) return ch; }
  else if (df > 0.32) {
    x  = ndqtl(p);
    p1 = 0.222222/df;
    t  = x *sqrt(p1) +1 -p1;
    ch = df *t*t*t;
    if (ch > 2.2*df +6) ch = -2 *(log(1-p) -c *log(0.5*ch) +g); }
  else {
    ch = 0.4;
    a  = log(1-p);
    do {
      q  = ch;
      p1 = 1 +ch *(4.67 +ch);
      p2 =    ch *(6.73 +ch *(6.66 +ch));
      t  = -0.5  +    ( 4.67 +2 *ch) /p1
         - (6.73 +ch *(13.32 +3 *ch))/p2;
      ch -= (1 -exp(a +g +0.5 *ch +c *LN_2) *p2/p1) /t;
    } while (fabs(q/ch-1)-0.01 > 0);
  }
  do {
    q  = ch;
    p1 = 0.5*ch;
    p2 = p -(t = incGa(y, p1));
    t  = p2 *exp(y *LN_2 +g +p1 -c *log(ch));   
    b  = t/ch; a = 0.5*t -b*c;
    s1 = (210 +a *(140 +a *(105 +a *(  84 +a *( 70 +a *  60))))) /420;
    s2 = (420 +a *(735 +a *(966 +a *(1141 +a *1278))))          /2520;
    s3 = (210 +a *(462 +a *(707 +a *  932)))                    /2520;
    s4 = (252 +a *(672 +a *1182)+c *( 294 +a *(889 +a *1740)))  /5040;
    s5 = ( 84 +a * 264          +c *( 175 +a * 606))            /2520;
    s6 = (120                   +c *( 346 +c * 127))            /5040;
    ch += t *(1 +0.5 *t *s1
        - c *b *(s1 -b *(s2 -b *(s3 -b *(s4 -b *(s5 -b *s6))))));
  } while (fabs(q/ch-1) > e);
  return ch;
}  /* c2qtl() */

/*--------------------------------------------------------------------
Reference: D.J. Best and D.E. Roberts.
           The percentage points of the chi^2 distribution.
           Applied Statistics 24:385--388, 1975
--------------------------------------------------------------------*/
