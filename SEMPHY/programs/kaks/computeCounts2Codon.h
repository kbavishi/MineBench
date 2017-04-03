// $Id: computeCounts2Codon.h 2399 2014-03-13 22:43:51Z wkliao $

// version 1.00
// last modified 3 Nov 2002

#ifndef ___COMPUTE_COUNTS_2_CODON
#define ___COMPUTE_COUNTS_2_CODON

#include "definitions.h"
#include "countTableComponent.h"
#include "sequenceContainer.h"
#include "computePijComponent.h"
#include "suffStatComponent.h"
#include "empirSelectionModel.h"


// things included for the function "fillCountTableComponentGam"
#include "sequenceContainer.h"

class computeCounts2Codon {
public:
	explicit computeCounts2Codon() {};
	void computeCountsNodeFatherNodeSonHomPos(const sequenceContainer& sc,
												   const computePijHom& pi,
												   const empiriSelectionModel& model,
												   const suffStatGlobalHomPos& cup,
												   const suffStatGlobalHomPos& cdown,
												   const MDOUBLE weight,
												   const MDOUBLE posProb,
												   const tree::nodeP nodeSon,
												   countTableComponentHom& _ctc,
												   const MDOUBLE rateCategorProb=1.0);
};

#endif
