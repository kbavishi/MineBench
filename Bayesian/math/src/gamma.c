/*----------------------------------------------------------------------
  File    : gamma.c
  Contents: computation of the gamma function
  Author  : Christian Borgelt
  History : 04.07.2002 file created
----------------------------------------------------------------------*/
#include <math.h>
#include <assert.h>
#include "gamma.h"

/*----------------------------------------------------------------------
  Preprocessor Definitions
----------------------------------------------------------------------*/
#define LG_C0        1.000000000190015       /* coefficients for */
#define LG_C1       76.18009172947146        /* the computation of */
#define LG_C2     (-86.50532032941677)       /* ln(\Gamma(n)) */
#define LG_C3       24.01409824083091
#define LG_C4     ( -1.231739572450155)
#define LG_C5        0.1208650972866179e-2
#define LG_C6     ( -0.5395239384953e-5)
#define SQRT_2PI     2.5066282746310005      /* \sqrt(2\pi) */

#define IG_MAXITER  100
#define IG_TINY     1e-30
#define IG_EPSILON  1e-8

/*----------------------------------------------------------------------
  Functions
----------------------------------------------------------------------*/

double logGa (double n)
{                               /* --- compute ln(Gamma(n))         */
  double s;                     /*           = ln((n-1)!), n \in IN */

  assert(n > 0);                /* check the function argument */
  s = LG_C0 +LG_C1/(n+1) +LG_C2/(n+2) +LG_C3/(n+3)
            +LG_C4/(n+4) +LG_C5/(n+5) +LG_C6/(n+6);
  return (n+0.5) *log(n+5.5) -(n+5.5) +log(SQRT_2PI *s/n);
}  /* logGa() */

/*----------------------------------------------------------------------
Use Lanczos' approximation
\Gamma(n+1) = (n+\gamma+0.5)^(n+0.5)
            * e^{-(n+\gamma+0.5)}
            * \sqrt{2\pi}
            * (c_0 +c_1/(n+1) +c_2/(n+2) +...+c_n/(n+k) +\epsilon)
and exploit the recursion \Gamma(n+1) = n *\Gamma(n) once,
i.e., compute \Gamma(n) as \Gamma(n+1) /n.

For the choices \gamma = 5, k = 6, and c_0 to c_6 as defined above
it is |\epsilon| < 2e-10 for all n > 0.

Source: W.H. Press, S.A. Teukolsky, W.T. Vetterling, and B.P. Flannery
        Numerical Recipes in C - The Art of Scientific Computing
        Cambridge University Press, Cambridge, United Kingdom 1992
        pp. 213-214
----------------------------------------------------------------------*/

double incGa (double n, double x)
{                               /* --- compute incomplete Gamma func. */
  int    i;                     /* loop variable */
  double a, b, c, d, e, f;      /* buffers */

  assert((n > 0) && (x >= 0));  /* check the function arguments */
  if (x < n+1) {                /* series representation */
    if (x <= 0) return 0;       /* treat x = 0 as a special case */
    a = n; f = d = 1/n;         /* compute initial values */
    for (i = IG_MAXITER; --i >= 0; ) {
      f += d *= x/++a;          /* add one term of the series */
      if (fabs(d) < fabs(f) *IG_EPSILON) break;
    }                           /* if term is small enough, abort */
    return f *exp(-x +n *log(x) -logGa(n)); }
  else {                        /* continued fraction representation */
    b = x+1-n; c = 1/IG_TINY; f = d = 1/b;
    for (i = 1; i < IG_MAXITER; i++) {
      a = i*(n-i);              /* use Lentz's algorithm to compute */
      d = a *d +(b += 2);       /* consecutive approximations */
      if (fabs(d) < IG_TINY) d = IG_TINY;
      c = b +a/c;
      if (fabs(c) < IG_TINY) c = IG_TINY;
      d = 1/d; f *= e = d *c;
      if (fabs(e-1) < IG_EPSILON) break;
    }                           /* if factor is small enough, abort */
    return 1 -f *exp(-x +n *log(x) -logGa(n));
  }
}  /* incGa() */                           

/*----------------------------------------------------------------------
series approximation:
P(a,x) =    \gamma(a,x)/\Gamma(a)
\gamma(a,x) = e^-x x^a \sum_{n=0}^\infty (\Gamma(a)/\Gamma(a+1+n)) x^n

continued fraction approximation:
P(a,x) = 1 -\Gamma(a,x)/\Gamma(a)
\Gamma(a,x) = e^-x x^a (1/(x+1-a- 1(1-a)/(x+3-a- 2*(2-a)/(x+5-a- ...))))

Source: W.H. Press, S.A. Teukolsky, W.T. Vetterling, and B.P. Flannery
        Numerical Recipes in C - The Art of Scientific Computing
        Cambridge University Press, Cambridge, United Kingdom 1992
        formulae:          pp. 216-219
        Lentz's algorithm: p.  171
----------------------------------------------------------------------*/
