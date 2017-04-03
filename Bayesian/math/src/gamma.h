/*----------------------------------------------------------------------
  File    : gamma.h
  Contents: computation of the gamma function
  Author  : Christian Borgelt
  History : 04.07.2002 file created
            02.08.2002 function gamma added
            19.05.2003 incomplete gamma function added
----------------------------------------------------------------------*/
#ifndef __GAMMA__
#define __GAMMA__
#include <math.h>

/*----------------------------------------------------------------------
  Functions
----------------------------------------------------------------------*/
extern double logGa (double n);
extern double gamma (double n);
extern double incGa (double n, double x);

/*----------------------------------------------------------------------
  Preprocessor Definitions
----------------------------------------------------------------------*/
#define gamma(x)    exp(logGa(x))

#endif
