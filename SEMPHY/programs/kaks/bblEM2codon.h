// 	$Id: bblEM2codon.h 2399 2014-03-13 22:43:51Z wkliao $	
//copy of bblEM of the lib + changing to codon model 
#ifndef ___BBL_EM_2_CODON_H
#define ___BBL_EM_2_CODON_H

#include "definitions.h"
#include "tree.h"
#include "stochasticProcess.h"
#include "sequenceContainer.h"
#include "countTableComponent.h"
#include "computePijComponent.h"
#include "suffStatComponent.h"
#include "empirSelectionModel.h"
#include <vector>
using namespace std;

class bblEM2codon {
public:
	explicit bblEM2codon(tree& et,
				const sequenceContainer& sc,
				const empiriSelectionModel &model,
				const Vdouble * weights = NULL,
				const int maxIterations=50,
				const MDOUBLE epsilon=0.05,
				const MDOUBLE tollForPairwiseDist=0.01);
	MDOUBLE getTreeLikelihood() const {return _treeLikelihood;}

private:
	MDOUBLE compute_bblEM(const int maxIterations,
					const MDOUBLE epsilon,
					const MDOUBLE tollForPairwiseDist);
	void bblEM_it(const MDOUBLE tollForPairwiseDist);
	void computeDown(const int pos);
	void computeUp();
	void addCounts(const int pos);
	void addCounts(const int pos, tree::nodeP mynode, const MDOUBLE posProb, const MDOUBLE weig);
	void optimizeBranches(const MDOUBLE tollForPairwiseDist);
	void allocatePlace();


	MDOUBLE _treeLikelihood;
	tree& _et;
	const sequenceContainer& _sc;
	const empiriSelectionModel& _model;
	vector<countTableComponentGam> _computeCountsV; // for each node - a table of rate*alph*alph
	computePijGam _pij;
	suffStatGlobalGam _cup;
	suffStatGlobalGamPos _cdown;
	const Vdouble * _weights;
	Vdouble _posLike;
	
};

#endif
