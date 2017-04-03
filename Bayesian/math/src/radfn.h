/*----------------------------------------------------------------------
  File    : radfn.h
  Contents: radial function management
            (for clustering, learning vector quantization etc.)
  Author  : Christian Borgelt
  History : 15.08.2003 file created from file cluster1.c
----------------------------------------------------------------------*/
#ifndef __RADFN__
#define __RADFN__

/*----------------------------------------------------------------------
  Type Definitions
----------------------------------------------------------------------*/
typedef double RADFN (double d2, double *params);

/*----------------------------------------------------------------------
  Functions
----------------------------------------------------------------------*/
extern double rf_cauchy (double d2, double *params);
extern double rf_gauss  (double d2, double *params);

#endif
