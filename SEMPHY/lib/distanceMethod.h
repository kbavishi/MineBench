// $Id: distanceMethod.h 2399 2014-03-13 22:43:51Z wkliao $

#ifndef ___DISTANCE_METHOD
#define ___DISTANCE_METHOD
#include "definitions.h"
#include "sequence.h"

/*********************************************************
Distance method is a class for computing pairwise distance 
between 2 different sequences
*******************************************************/
class distanceMethod {
public:
  virtual const MDOUBLE giveDistance(const sequence& s1,
				     const sequence& s2,
				     const vector<MDOUBLE> * weights=NULL,
				     MDOUBLE* score=NULL) const=0;
  virtual distanceMethod* clone(void) const=0;
  virtual ~distanceMethod() {}
};


#endif

