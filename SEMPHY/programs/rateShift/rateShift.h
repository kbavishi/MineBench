// 	$Id: rateShift.h 2399 2014-03-13 22:43:51Z wkliao $	
#ifndef ___RATE_SHIFT____H
#define ___RATE_SHIFT____H

#include "definitions.h"
#include "alphabet.h"
#include "sequenceContainer.h"
#include "stochasticProcess.h"
#include "tree.h"
#include "likelihoodComputation.h"
#include "nucJC.h"
#include "aaJC.h"
#include "nucleotide.h"
#include "mulAlphabet.h"
#include "replacementModelSSRV.h"
#include "stochasticProcessSSRV.h"
#include "bestAlphaAndNu.h"
#include "someUtil.h"
#include "recognizeFormat.h"
#include "amino.h"
#include "gammaDistribution.h"
#include "uniDistribution.h"

#include "readDatMatrix.h"
#include "chebyshevAccelerator.h"
#include "likeDist.h"
#include "bestAlpha.h"
#include "distanceTable.h"
#include "nj.h"
#include "treeUtil.h"
#include "trivialAccelerator.h"
#include "rateShiftOptions.h"
#include "bestParamUSSRV.h"
#include "computePijComponent.h"
#include "doubleRep.h"
#include "posData.h"
#include "rateShiftProbs4branch.h"
#include <algorithm>
#include <iostream>
#include <fstream>
#include <ctime> 
#include <iomanip>
using namespace std;


class rateShift {

public:
	explicit rateShift() : _options(NULL),_baseSc(NULL), _scSSRV(NULL),_baseAlph(NULL),_alph(NULL),_model(NULL),_rateShiftProbs4branch(NULL) {};
	virtual ~rateShift();
	void run(int argc, char** argv);

private:
	void printrateShiftInfo(ostream& out);
	void fillOptionsParameters(int argc, char** argv);
	void printOptionParameters();

	void getStartingSequenceData();
	void getStartingUSSRVmodel();
	
	void calcLikelihood();
	void calcLikelihoodWithoutOptimization();
	void calcLikelihoodWithOptimization(bool AlphaOptimization, bool NuOptimization,
											   bool FOptimization, bool bblOptimization);
	void setOriginalAlphaAndNuAndF();
	void calcSSRV4site();
	void calcShifts4branch();
	void sortSSRV4site() {std::sort(_SSRV4site.begin(),_SSRV4site.end(),cmpPosDataSSRVPointers());}
	void printSSRV4site(ofstream& out) const;
	void printBranchesData(ostream& out,ostream& outTreeWithBranchesNames,ostream& outPositions4EachBranch) const;
	void printGapsPositionsInRefSeq(ofstream& out) const;
	void fillReferenceSequence();
	void printOutputTree();

	void printTreeWithNodeIdBPStyle(ostream &out) const;
	void recursivePrintTree(ostream &out,const tree::nodeP &myNode) const;

private:
	rateShiftOptions* _options;
	sequenceContainer* _baseSc; // for the base model
	sequenceContainer* _scSSRV; // for the SSRV model
	tree _et;	
	alphabet* _baseAlph; // for the base model
	mulAlphabet* _alph; // for the SSRV model
	ussrvModel* _model;
	vector<posDataSSRV*> _SSRV4site;
	sequence* _refSeq; // the reference sequence
	rateShiftProbs4branch* _rateShiftProbs4branch;
};


#endif
