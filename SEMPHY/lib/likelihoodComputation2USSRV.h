// 	$Id: likelihoodComputation2USSRV.h 2399 2014-03-13 22:43:51Z wkliao $	
#ifndef ___LIKELIHOOD_COMPUTATION_2_USSRV
#define ___LIKELIHOOD_COMPUTATION_2_USSRV

#include "definitions.h"
#include "computePijComponent.h"
#include "sequenceContainer.h"
#include "suffStatComponent.h"
#include "ussrvModel.h"
#include "tree.h"
#include "computeUpAlg.h"
#include "likelihoodComputation.h"
#include <cmath>
#include <cassert>


namespace likelihoodComputation2USSRV {

	MDOUBLE getTreeLikelihoodAllPosAlphTheSame(const tree& et,
							const sequenceContainer& sc,const sequenceContainer& baseSc,
							const ussrvModel& model,const Vdouble * const weights=0);

	MDOUBLE getTreeLikelihoodFromUp2(const tree& et,
						const sequenceContainer& sc,
						const sequenceContainer& baseSc,
						const ussrvModel & model,
						const suffStatGlobalGam& cupBase,
						const suffStatGlobalHom& cupSSRV,
						VdoubleRep& posLike, // fill this vector with each position likelihood but without the weights.
						const Vdouble * weights=0);
	
};



#endif // ___LIKELIHOOD_COMPUTATION_2_USSRV
