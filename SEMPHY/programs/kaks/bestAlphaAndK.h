// 	$Id: bestAlphaAndK.h 2399 2014-03-13 22:43:51Z wkliao $	
#ifndef ___BEST_ALPHA_AND_K
#define ___BEST_ALPHA_AND_K

#include "definitions.h"
#include "tree.h"
#include "likelihoodComputation.h"
#include "likelihoodComputation2Codon.h"
#include "sequenceContainer.h"
#include "stochasticProcess.h"
#include "gammaDistribution.h"
#include "generalGammaDistribution.h"
#include "logFile.h"
#include "JTTcodonModel.h"
#include "wYangModel.h"
#include "bblEM2codon.h"
#include "computeUpAlg.h"
#include "empirSelectionModel.h"


#define VERBOS

//evaluate best Alpha Ks and BBL for the data using JTT model.
class bestAlphaAndKsAndBBL {
public:
	explicit bestAlphaAndKsAndBBL(tree& et, 
					   const sequenceContainer& sc,
					   empiriSelectionModel & empiriSelectionModel,
					   const MDOUBLE upperBoundOnAlpha = 50.0,
					   const MDOUBLE upperBoundOnBeta = 50.0,
					   const MDOUBLE epsilonFOptimization = 0.01,
					   const MDOUBLE epsilon= 0.05,
					   const int maxIterations=20);
	const MDOUBLE getBestAlpha() const{return _bestAlpha;}
	const MDOUBLE getBestBeta() const{return _bestBeta;}
	const MDOUBLE getBestL() const {return _bestL;}
	const MDOUBLE getBestTr() const {return _bestTr;}
	const MDOUBLE getBestTv() const {return _bestTv;}
	const MDOUBLE getBestTrTr() const {return _bestTrTr;}
	const MDOUBLE getBestTrTv() const {return _bestTrTv;}
	const MDOUBLE getBestTvTv() const {return _bestTvTv;}
	const MDOUBLE getBestThreeSub() const {return _bestThreeSub;}
	const MDOUBLE getBestOmega() const {return _bestOmega;}
	const MDOUBLE getBestBetaProb() const {return _bestBetaProb;}

	const MDOUBLE getBestF() const {return _bestF;}
private:
	MDOUBLE _bestAlpha;
	MDOUBLE _bestL;
	MDOUBLE _bestTr;
	MDOUBLE _bestTv;
	MDOUBLE _bestTrTr;
	MDOUBLE _bestTrTv;
	MDOUBLE _bestTvTv;
	MDOUBLE _bestThreeSub;
	MDOUBLE _bestBeta;
	MDOUBLE _bestF;
	MDOUBLE _bestOmega; //be used for beta distribution
	MDOUBLE _bestBetaProb;//be used for beta distribution
};


//The fanctor to eval alpha or Ks using the alphaOrKs flag.
class evalAlphaOrKs{
public:
	explicit evalAlphaOrKs(const tree& et,
				const sequenceContainer& sc,
			       const  empiriSelectionModel &model,
				int alphaOrKs)
				: _et(et),_sc(sc),_model(model),_alphaOrKs(alphaOrKs){};

	MDOUBLE operator()(MDOUBLE param);	
	

private:
	const tree& _et;
	const sequenceContainer& _sc;	
	empiriSelectionModel _model;
	int _alphaOrKs; //flag to eval different parametrs (alpha or ks)

};

#endif


