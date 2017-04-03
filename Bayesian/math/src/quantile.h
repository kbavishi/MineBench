/*----------------------------------------------------------------------
  File    : quantile.c
  Contents: compute quantiles of normal and chi^2 distribution
  Author  : Christian Borgelt
  History : 19.05.2003 file created
----------------------------------------------------------------------*/
#ifndef __QUANTILE__
#define __QUANTILE__

/*----------------------------------------------------------------------
  Functions
----------------------------------------------------------------------*/
extern double ndqtl (double prob);
extern double c2qtl (double prob, double df);

#endif
