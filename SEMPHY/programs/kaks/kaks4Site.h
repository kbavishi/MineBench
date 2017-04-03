// 	$Id: kaks4Site.h 2399 2014-03-13 22:43:51Z wkliao $	
#ifndef ___KAKS4SITE____H
#define ___KAKS4SITE____H


#include "kaksOptions.h"
#include "sequenceContainer.h"
#include "tree.h"
#include "stochasticProcess.h"
#include "goldmanYangModel.h"
#include "likelihoodComputation.h"
#include "numRec.h"
#include "nucleotide.h"
#include "codon.h"
#include "amino.h"
#include "logFile.h"
#include "fastaFormat.h"
#include "clustalFormat.h"
#include "recognizeFormat.h"
#include "someUtil.h"
#include "nj.h"
#include "evaluateCharacterFreq.h"
#include "uniDistribution.h"
#include "gammaDistribution.h"
#include "generalGammaDistribution.h"
#include "betaOmegaDistribution.h"
#include "trivialAccelerator.h"
#include "bestAlphaAndK.h"
#include "siteSpecificForce.h"
#include "JTTcodonModel.h"
#include "bblEM.h"
#include "wYangModel.h"
#include "likeDist.h"
#include "definitions.h"
#include "evaluateCharacterFreq.h"
#include "params.h"
#include "utils.h"
#include <algorithm>


class kaks4Site {

public:
	explicit kaks4Site(int argc, char* argv[]);
	virtual ~kaks4Site();
//private:
	void printInfo(ostream& out);
	void fillOptionsParameters(int argc, char* argv[]);
	void printOptionParameters();
	void initCodonScAndTree();
	void createTree();
	void createModel();	
	void computeAlphaAndKsAndBBL();
	void computeEB_EXPKaKs4Site(); //using BAYESIAN estimation
	void computeMLKaKs4Site();  //using ML estimation for goldman and yang model
	void computeGlobalParams(MDOUBLE initV,MDOUBLE initK);
	
	MDOUBLE nonSyn2Syn(const stochasticProcess & sp);
	MDOUBLE computeKaKs();
	Vdouble copmuteFreq();

	MDOUBLE findBestParamManyStarts( Vint pointsNum,  Vint iterNum,  Vdouble tols);
	void printGlobalRes();
private:
	kaksOptions* _pOptions;
	sequenceContainer _codonSc;
	tree _t;
	empiriSelectionModel _model;
	codon * _codonAlph;
};


#endif
