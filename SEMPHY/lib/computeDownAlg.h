// $Id: computeDownAlg.h 2399 2014-03-13 22:43:51Z wkliao $

#ifndef ___COMPUTE_DOWN_ALG
#define ___COMPUTE_DOWN_ALG

#include "definitions.h"
#include "tree.h"
#include "suffStatComponent.h"
#include "sequenceContainer.h"
#include "computePijComponent.h"


class computeDownAlg {
public: 
	void fillComputeDown(const tree& et,
					   const sequenceContainer& sc,
					   const int pos,
					   const computePijHom& pi,
					   suffStatGlobalHomPos& ssc,
					   const suffStatGlobalHomPos& cup);

	void fillComputeDown(const tree& et,
					   const sequenceContainer& sc,
					   const int pos,
					   const stochasticProcess& sp,
					   suffStatGlobalHomPos& ssc,
					   const suffStatGlobalHomPos& cup);

	void fillComputeDownSpecificRate(const tree& et,
					   const sequenceContainer& sc,
					   const int pos,
					   const stochasticProcess& sp,
					   suffStatGlobalHomPos& ssc,
					   const suffStatGlobalHomPos& cup,
					   const MDOUBLE gRate);

};
#endif
