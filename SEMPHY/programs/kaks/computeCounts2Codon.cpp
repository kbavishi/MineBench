// 	$Id: computeCounts2Codon.cpp 2399 2014-03-13 22:43:51Z wkliao $	
#include "computeCounts2Codon.h"


void computeCounts2Codon::computeCountsNodeFatherNodeSonHomPos(const sequenceContainer& sc,
												   const computePijHom& pi,
												   const empiriSelectionModel& model,
												   const suffStatGlobalHomPos& cup,
												   const suffStatGlobalHomPos& cdown,
												   const MDOUBLE weight,
												   const MDOUBLE posProb,
												   const tree::nodeP nodeSon,
												   countTableComponentHom& _ctc,
												   const MDOUBLE rateCategorProb 
												   ) {
	
	assert(posProb>0.0);
	if (weight == 0) return;
	int alph1,alph2;
	for (alph1 =0; alph1< pi.alphabetSize(); ++alph1) {
		for (alph2 =0; alph2< pi.alphabetSize(); ++alph2) {
			MDOUBLE tmp = cup.get(nodeSon->id(),alph1) *
			cdown.get(nodeSon->id(),alph2) *
			pi.getPij(nodeSon->id(),alph1,alph2)*
			model.freq(alph1)
			* rateCategorProb
			/
			posProb;
			_ctc.addToCounts(alph1,alph2,tmp*weight);
		}
	}
}
												   
