// 	$Id: rateShiftProbs4branch.h 2399 2014-03-13 22:43:51Z wkliao $	
#ifndef RATE_SHIFT_PROBS_4_BRANCH
#define RATE_SHIFT_PROBS_4_BRANCH

#include "tree.h"
#include "sequenceContainer.h"
#include "stochasticProcessSSRV.h"
#include "computePijComponent.h"
#include "suffStatComponent.h"
#include "ussrvModel.h"
#include "computeUpAlg.h"
#include "computeDownAlg.h"
#include "branchData.h"
#include "treeIt.h"
#include "mulAlphabet.h"
#include "alphabet.h"
#include "likelihoodComputation.h"
#include "likelihoodComputation2USSRV.h"
#include "normalDist.h"

class rateShiftProbs4branch
{
public:
	rateShiftProbs4branch(const tree& et,
				const sequenceContainer& scSSRV,const sequenceContainer& baseSc,
				const ussrvModel& model);
	~rateShiftProbs4branch();
	void runSSRV();
	void runUSSRV();
	void calculateBinomialPvalues();
	void calculatePoissonPvaluesTheoreticalLamda();
	void calculatePoissonPvaluesAmpiricLamda();
//	void debug();
	void printBranchesData(ostream& out) const;
	void printPositions4EachBranch(ostream& out) const;
	void printBinomialSignificantBranches(ostream& out,MDOUBLE alpha=0.1) const;
	void printPoissonSignificantBranches(ostream& out,MDOUBLE alpha=0.1) const;

	branchData* operator[](int i) {return _branchesData[i];}
	const branchData* operator[](int i) const {	return _branchesData[i];}

private:
// for the SSRV	
	void init();
	void computeUpSSRV();
	void computeDownSSRV();
	void allocatePlaceSSRV();
	void fillBranchesDataSSRV();
	void fillBranchesDataSSRVWithDebug(); // also calculate the likelihood of every position seperatly and check that it matches the branches results
	doubleRep calcVal(int id, int pos, int letter, int fatherLetter,const stochasticProcess& sp);

// for the USSRV calculation (this use the methods of the SSRV calculation)
	void fillBranchesDataUSSRV();
	void fillBranchesDataUSSRVWithDebug(); // also calculate the likelihood of every position seperatly and check that it matches the branches results

private:
	MDOUBLE _treeLogLikelihood;
	const tree& _et;
	const sequenceContainer& _scSSRV;
	const sequenceContainer& _baseSc;
	const ussrvModel& _model;

	computePijHom _pijSSRV; // maybe I can call computeDownAlg with the _sp, without filiing thie _pij. (this may cause problems in computeUpAlg. why ?)
	computePijGam _pijBase;
	suffStatGlobalHom _cupSSRV;  
	suffStatGlobalHom _cdownSSRV;

	vector<branchData*> _branchesData;
};
// @@@@ more efficient - get cup,cdown and pij from the bblEM2USSRV class.

#endif // RATE_SHIFT_PROBS_4_BRANCH
