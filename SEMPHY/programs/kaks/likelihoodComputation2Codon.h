// $Id: likelihoodComputation2Codon.h 2399 2014-03-13 22:43:51Z wkliao $

#ifndef ___LIKELIHOOD_COMPUTATION_2_CODON
#define ___LIKELIHOOD_COMPUTATION_2_CODON

#include "definitions.h"
#include "computePijComponent.h"
#include "sequenceContainer.h"
#include "suffStatComponent.h"
#include "empirSelectionModel.h"


namespace likelihoodComputation2Codon {

	MDOUBLE getTreeLikelihoodAllPosAlphTheSame(const tree& et,
							const sequenceContainer& sc,
							const empiriSelectionModel & model);

	MDOUBLE getProbOfPosUpIsFilledSelectionGam(const int pos,const tree& et, //used for gamma model
						const sequenceContainer& sc,
						const stochasticProcess& sp,
						const suffStatGlobalGamPos& cup,
						const distribution * distr);

	MDOUBLE getTreeLikelihoodFromUp2(const tree& et,
						const sequenceContainer& sc,
						const empiriSelectionModel & model,
						const suffStatGlobalGam& cup,
						Vdouble& posLike, // fill this vector with each position likelihood but without the weights.
						const Vdouble * weights=0);
};



#endif
