// $Id: likelihoodComputationFactors.h 2399 2014-03-13 22:43:51Z wkliao $

#ifndef ___LIKELIHOOD_COMPUTATION_FACTORS
#define ___LIKELIHOOD_COMPUTATION_FACTORS

#include "definitions.h"
#include "tree.h"
#include "computePijComponent.h"
#include "sequenceContainer.h"
#include "suffStatComponent.h"

namespace likelihoodComputation {

	MDOUBLE getLOG_LofPos(const int pos, // with a site specific rate.
					  const tree& et,
					  const sequenceContainer& sc,
					  const stochasticProcess& sp,
					  const MDOUBLE gRate);

	// add all the other functions to use factors...


};



#endif

