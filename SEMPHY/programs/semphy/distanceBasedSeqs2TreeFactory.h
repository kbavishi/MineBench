// 	$Id: distanceBasedSeqs2TreeFactory.h 2399 2014-03-13 22:43:51Z wkliao $	
#include "distanceBasedSeqs2Tree.h"

#include "pairwiseGammaDistance.h"

#include "nj.h"

//using namespace std;

#ifndef __DISTANCEBASEDSEQS2TREEFACTORY_H
#define __DISTANCEBASEDSEQS2TREEFACTORY_H

typedef enum {homogeneousRatesDTME,
			  pairwiseGammaDTME,
			  commonAlphaDTME,
			  rate4siteDTME,
			  posteriorDTME
} distanceBasedMethod_t;



distanceBasedSeqs2Tree* distanceBasedSeqs2TreeFactory(const distanceBasedMethod_t distanceBasedMethod, 
						      stochasticProcess& sp, // may change sp (alpha)
						      const bool   useJcDistance,
						      const bool   optimizeAlpha,
						      const double epsilonLikelihoodImprovement4iterNJ,
						      const double epsilonLikelihoodImprovement4pairwiseDistance,
						      const double epsilonLikelihoodImprovement4alphaOptimiz,
						      const double epsilonLikelihoodImprovement4BBL);



#endif





