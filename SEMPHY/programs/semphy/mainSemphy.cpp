// $Id: mainSemphy.cpp 2399 2014-03-13 22:43:51Z wkliao $

// DO: (1) ADD NUMBER OF DESCRETE CATEGORIES TO THE OPTIONS.
// DO: (2) ADD NUMBER OF RANDOM STARTS TO THE OPTIONS.
// DO: (3) CHENYSHEV PARAMETERS TO THE OPTIONS.
// DO: (4) ADD PARAMETER CONCERNING THE STARTING NJ TREE.
// DO: (5) ADD PARAMETER CONCERNING THE GAMMA PARAM OPTIMIZATION.

#include "definitions.h"
#include "mainSemphy.h"
#include "jcDistance.h" 
#include "distanceTable.h" 
#include "nj.h" 
#include "constraints.h"
#include "bblEM.h"
#include "semphySearchBestTree.h"
#include "logFile.h"
#include "readDatMatrix.h"
#include "datMatrixHolder.h"
#include "likelihoodComputation.h"
#include "likeDist.h"
#include "bestAlpha.h"
#include "findRateOfGene.h"
#include "talRandom.h"
#include "someUtil.h"
#include "getRandomWeights.h"
#include "codon.h"
#include "recognizeFormat.h"
#include "generalGammaDistributionLaguerre.h"
#include "correctToCanonialForm.h"
#include <iostream>
#include <fstream>
#include <cassert>
#include <string>
#include <cstdio>
using namespace std;

//*******************************************************************************
// constructors
//*******************************************************************************
mainSemphy::mainSemphy(const semphy_args_info& gn):	_args_info(gn),_evolObj(_args_info),_weights(NULL), _s2tPtr(NULL), _numberOfRandomStart(1){
	initializeFromArgsInfo();
}

mainSemphy::mainSemphy(int argc, char* argv[]):_weights(NULL), _s2tPtr(NULL) , _numberOfRandomStart(1){
	readCommandLineInformation(argc,argv);
	initializeFromArgsInfo();
	myLog::printArgv(1, argc, argv);

}

void mainSemphy::readCommandLineInformation(int argc, char* argv[]) {
	if (argc < 2) errorMsg::reportError("The program must get some parameters in the command line, use -h for help");
	if (semphy_cmdline_parser(argc, argv, &_args_info) != 0) {
	  errorMsg::reportError("error reading command line",1);
	}	
	cmdline2EvolObjs<semphy_args_info> _evolObj(_args_info);
}

void mainSemphy::initializeFromArgsInfo(){
//	AND WE CHECK TO SEE IF WE ARE ASKED FOR SOMETING WE CAN NOT DELIVER
//	if	(_args_info.min_improv_given)  errorMsg::reportError("minimum-improvement not yet implimented");
//	if	(_args_info.maxDistance_given) errorMsg::reportError("max distance not yet implimented");
//	if	(_args_info.exact_given)		  errorMsg::reportError("exact counts not yet implimented");
	argsConsistencyCheck();
	_evolObj.initializeRandomSeed();
	_evolObj.initializeLogFile();
	//   initializeAlphabet();
	_alphP=_evolObj.cmdline2Alphabet();
	//	initializeSequenceContainer();
	_sc = _evolObj.cmdline2SequenceContainer(_alphP);
	//	takeCareOfGaps();
	_evolObj.takeCareOfGaps(_sc);
	//	readTreeFile();
	_etPtr=_evolObj.cmdline2Tree();
	//  make sure tree is in canonical form
    semphyCorrectToCanonialForm();
	//	readConstraintTreeFile();
	_constraintTreePtr = _evolObj.cmdline2ConstraintTree();
	//	initializeStochaticProcess();
	_sp=_evolObj.cmdline2StochasticProcessSafe();
	//	initializeOutputStream(); //out().
	_outPtr = _evolObj.cmdline2OutputStream();
	printSemphyTitle(out());
	constraintTreeConsistencyCheck(); // check that the tree is compatible with the constraint tree if given.
}

void mainSemphy::printSemphyTitle(ostream & out) {
  if (_args_info.verbose_arg<0) return;	// just don't print if using -v-1
	out<<"#################################################################"<<endl;
	out<<"# SEMPHY: A Structural EM Algorithm for Phylogenetic Inference  #"<<endl;
	out<<"# for information, please send email to semphy@cs.huji.ac.il    #"<<endl;
	out<<"#################################################################"<<endl;
	out<<endl;
}

mainSemphy::~mainSemphy() {
	if (_alphP) delete _alphP;
	//	if (_spPtr) delete _spPtr;
	if (_etPtr) delete _etPtr;
	if (_constraintTreePtr) delete _constraintTreePtr;
	if (_weights) delete _weights;
	if (_s2tPtr) delete _s2tPtr;
	myLog::endLog();// close log file as late as posible
}
void mainSemphy::semphyCorrectToCanonialForm(void){
  if (_etPtr == NULL) return;

  int nNodes=_etPtr->getNodesNum();

  vector<char> isRealTaxa(nNodes,0);
  vector<tree::nodeP> all;
  _etPtr->getAllNodes(all,_etPtr->getRoot());
  for (vector<tree::nodeP>::iterator i=all.begin();i!=all.end();++i)
	isRealTaxa[(*i)->id()]=(*i)->isLeaf();

  VVdouble dummyDistanceTable(nNodes);
  for (int i=0;i<nNodes;++i) {
	dummyDistanceTable[i].resize(nNodes);
	mult(dummyDistanceTable[i], 0.0); 	// all values are 0 now;
  }
  correctToCanonialForm ctcf(_etPtr, dummyDistanceTable, isRealTaxa);
  ctcf.correctTree();
}

void mainSemphy::argsConsistencyCheck() const 
{
	// make sure that if the user askes for an action that requires a
	// tree, either he gives a tree or he askes SEMPHY to make one
	if (!(_args_info.tree_given || _args_info.DistanceTableEstimationMethod_group_counter!=0 || _args_info.SEMPHY_given))	// we neither have a tree of can make one
		errorMsg::reportError("Must ask to build a tree by some method or give a tree as input");
	
	// Check that tree is not given for non-iterative NJ
	if (_args_info.tree_given && (_args_info.NJ_given || _args_info.homogeneousRatesDTME_given || _args_info.pairwiseGammaDTME_given))
		errorMsg::reportError("Don't give initial tree with non-iterative NJ");
	
	// Check options related to a given tree and to bootstrap
	if (_args_info.BPonUserTree_given && !_args_info.tree_given)
		errorMsg::reportError("Must give --tree with --BPonUserTree");
    if (_args_info.BPonUserTree_given &&
		(_args_info.commonAlphaDTME_given || _args_info.rate4siteDTME_given || _args_info.posteriorDTME_given))
		errorMsg::reportError("Can't do --BPonUserTree with iterative NJ methods",1);

	// Check that the method chosen is compatible with the configuration of the other parameters and flags
	if (_args_info.commonAlphaDTME_given || _args_info.posteriorDTME_given) {
		if (!_args_info.optimizeAlpha_given)
			errorMsg::reportError("Must use --optimizeAlpha with --commonAlphaDTME or --posteriorDTME",1);
	}

	if (_args_info.homogeneous_flag && (_args_info.optimizeAlpha_given || _args_info.alpha_given || _args_info.categories_given || _args_info.laguerre_flag)) // problematic with homogeneous
		errorMsg::reportError("Must not use --optimizeAlpha or --alpha with --homogeneous",1);

	if (_args_info.rate4siteDTME_given || _args_info.homogeneousRatesDTME_given) {
		if (!_args_info.homogeneous_flag) {
			errorMsg::reportError("Must use --homogeneous with --rate4siteDTME or --homogeneousRatesDTME_given");
		}
	}
}

// check that the inputed tree, constraint tree
// are consistant, to prevent surprises and horrible problems.
void mainSemphy::constraintTreeConsistencyCheck() const {
	if (_etPtr!=NULL) {
		if (_constraintTreePtr !=NULL) {
			constraints c1(*_constraintTreePtr);
			c1.setTree(*_etPtr);
			if (!c1.fitsConstraints()) {
				LOG(1,<<"Input tree does not fit constraints!"<<endl);
			    LOGDO(1,c1.outputMissingClads(myLog::LogFile()));
				errorMsg::reportError("Please enter a starting tree that fits the constraints");
			}
		}
	}
}

void mainSemphy::optimizeBranchLengths() {
	if (_etPtr == NULL) {
		errorMsg::reportError("A tree must be given before optimizing branch length");
	}
	if(_args_info.optimizeAlpha_flag) {
		bestAlphaAndBBL bestAlpha2(*_etPtr, _sc, _sp, _weights ,1.5, 5.0,_args_info.epsilonLikelihoodImprovement4alphaOptimiz_arg,_args_info.epsilonLikelihoodImprovement4BBL_arg,_args_info.maxNumOfBBLIter_arg);
		_treeLogLikelihood = bestAlpha2.getBestL();
		out()<<"# Best alpha after branch length optimiziation"<<endl;
		out()<<bestAlpha2.getBestAlpha() <<endl;
		out()<<"# The likelihood of the tree"<<endl;
		out() <<_treeLogLikelihood<<endl;

		LOG(1,<<"# Best alpha after branch length optimiziation"<<endl);
		LOG(1,<<bestAlpha2.getBestAlpha() <<endl);
		LOG(1,<<"# The likelihood of the tree"<<endl);
		LOG(1,<<_treeLogLikelihood<<endl);
	} else {
	  bblEM bblEM1(*_etPtr,_sc,_sp,_weights,_args_info.maxNumOfBBLIter_arg,_args_info.epsilonLikelihoodImprovement4BBL_arg);//maxIterations=1000,epsilon=0.05
		_treeLogLikelihood=bblEM1.getTreeLikelihood();
		out()<<"# The likelihood of the tree"<<endl;
		out()<<_treeLogLikelihood<<endl;
		LOG(1,<<"# The likelihood of the tree"<<endl);
		LOG(1,<<_treeLogLikelihood<<endl);
	}
}

void mainSemphy::printTree(const int logLvl) const {
	int printMsg=max(3,logLvl);
	out()<<"# The tree"<<endl;
	_etPtr->output(out());
	LOG(printMsg,<<"# The tree"<<endl);
	LOGDO(logLvl,_etPtr->output(myLog::LogFile()));
}

void mainSemphy::printTreeToTreeFile() const {
	if (_args_info.treeoutputfile_given) {
		ofstream treeO(_args_info.treeoutputfile_arg);
		if (! treeO.is_open()) {
			errorMsg::reportError("can not open tree output file");
		}
		_etPtr->output(treeO);
		treeO.close();
	}
}

void mainSemphy::nullifyTree() {
	if (_etPtr) delete (_etPtr);
	_etPtr = NULL;
}

// This function is used, so that for example in bp, a new tree will be computed in NJ.
// The computeNJ for example will not compute the NJ tree if a tree is given.
void mainSemphy::setTree(const tree& inEtPtr) {
	if (_etPtr) delete (_etPtr);
	_etPtr = new tree(inEtPtr);
}

// void mainSemphy::getDistanceTableAndNames(VVdouble& disTable,
// 										  vector<string> & vNames,
// 										  const distanceMethod* cd) const {

// 	giveDistanceTable(cd,_sc,disTable,vNames,_weights);
// }

// void mainSemphy::computeNJtreeFromDisTableAndNames(const VVdouble& disTable,
// 										  const vector<string> & vNames) {
// 	NJalg nj1;
// 	if (_args_info.constraint_given) { // did we get a constraint tree
// 		setTree(nj1.computeTree(disTable,vNames,_constraintTreePtr));
// 	} else {
// 		setTree(nj1.computeTree(disTable,vNames));
// 	}
// }


void mainSemphy::computeNJtree(bool doBootstrapOneIteration) {

	distanceBasedMethod_t dtme = homogeneousRatesDTME; // default, vanila NJ
	if (_args_info.homogeneousRatesDTME_given || _args_info.NJ_given)
		dtme = homogeneousRatesDTME;
	else if (_args_info.pairwiseGammaDTME_given)
		dtme = pairwiseGammaDTME;
	else if (_args_info.commonAlphaDTME_given)
		dtme = commonAlphaDTME;
	else if (_args_info.rate4siteDTME_given)
		dtme = rate4siteDTME;
	else if (_args_info.posteriorDTME_given)
		dtme = posteriorDTME;
	else if (_args_info.SEMPHY_given)
		dtme = homogeneousRatesDTME;
	else errorMsg::reportError("mainSemphy::computeNJtree: An unsuppored DTME was specified");

	bool useJcDistance = (_args_info.nucjc_given || _args_info.aaJC_given);

	if (!doBootstrapOneIteration) {
		_s2tPtr = distanceBasedSeqs2TreeFactory(dtme, _sp, useJcDistance, _args_info.optimizeAlpha_flag, _args_info.epsilonLikelihoodImprovement4iterNJ_arg, _args_info.epsilonLikelihoodImprovement4pairwiseDistance_arg, _args_info.epsilonLikelihoodImprovement4alphaOptimiz_arg, _args_info.epsilonLikelihoodImprovement4BBL_arg);
		if (_etPtr == NULL) {
			if (_args_info.commonAlphaDTME_given) {
				commonAlphaDistanceSeqs2Tree *caS2tPtr = static_cast<commonAlphaDistanceSeqs2Tree*>(_s2tPtr);
				if (_args_info.alpha_given) { // use the given alpha
					setTree(caS2tPtr->seqs2TreeIterative(_sc, _args_info.alpha_arg, _weights, _constraintTreePtr));
				} else {		       // homogeneous rates in first iteration
					setTree(caS2tPtr->seqs2TreeIterative(_sc, _weights, _constraintTreePtr));
				}
			} else {  // all other methods
				setTree(_s2tPtr->seqs2Tree(_sc, _weights, _constraintTreePtr));
			}

			// If an initial tree was given then pass it to the iterative seqs2Tree method
			// NOTE: argsConsistencyCheck makes sure that non-interative NJ can't be run with --tree
		} else {
			iterativeDistanceSeqs2Tree *itS2tPtr = static_cast<iterativeDistanceSeqs2Tree*>(_s2tPtr);
			if (_args_info.alpha_given) {
				setTree(itS2tPtr->seqs2TreeIterative(_sc, *_etPtr, _args_info.alpha_arg, _weights, _constraintTreePtr));
			} else {
				setTree(itS2tPtr->seqs2TreeIterative(_sc, *_etPtr, _weights, _constraintTreePtr));
			}
		}

	// Do one bootstrap iteration
	} else 
		setTree(_s2tPtr->seqs2TreeBootstrap(_sc, _weights, _constraintTreePtr));
}

void mainSemphy::computeSemphyTree() {
	if (_etPtr == NULL) computeNJtree();
	bool Gamma = false;
	if (_args_info.optimizeAlpha_flag) Gamma = true;
	semphySearchBestTree(_sc,*_etPtr,_constraintTreePtr,_sp,out(),_numberOfRandomStart,Gamma);
}

void mainSemphy::optimizeGlobalRate() {
    out()<<"we are in void mainSemphy::optimizeGlobalRate()"<<endl;
	// THIS FUNCTION SHOULD BE FIXED IN THE FUTURE TO DO ITERATIONS OVER THE TWO COMPUTATIONS...
	if (_args_info.optimizeAlpha_flag) { // here we do alpha but NO bbl.
		optimizeAlphaOnly();
	}
	MDOUBLE rateOfGene=findTheBestFactorFor(*_etPtr,
											_sc,
											_sp, // changes _sp
											_weights,
											_treeLogLikelihood);
	out()<<"# The global rate of the tree"<<endl;
	out() << rateOfGene<<endl;
	out()<<"# The likelihood of the tree is "<<_treeLogLikelihood<<endl;
	LOG(1,<<"# The global rate of the tree"<<endl);
	LOG(1,<< rateOfGene<<endl);
	LOG(1,<<"# The likelihood of the tree is "<<_treeLogLikelihood<<endl);
}

void mainSemphy::optimizeAlphaOnly() {
	bestAlphaFixedTree bestAlpha(*_etPtr, _sc, _sp, _weights, 1.5);
	_treeLogLikelihood = bestAlpha.getBestL();
	out()<<"# Best alpha (for fixed branch lengths)"<<endl;
	out()<<bestAlpha.getBestAlpha() <<endl;
	out()<<"# The likelihood of the tree"<<endl;
	out() <<_treeLogLikelihood<<endl;
	LOG(1,<<"# Best alpha (for fixed branch lengths)"<<endl);
	LOG(1,<<bestAlpha.getBestAlpha() <<endl);
	LOG(1,<<"# The likelihood of the tree"<<endl);
	LOG(1,<<_treeLogLikelihood<<endl);
}

void mainSemphy::computeLikelihoodAndLikelihoodPerPosition() {
	_treeLogLikelihood = 0.0;
	_llpp.clear();
	computePijGam cpij;
	cpij.fillPij(*_etPtr,_sp);
	for (int pos=0; pos < _sc.seqLen() ;++pos) {
		MDOUBLE tmpLL = log(likelihoodComputation::getLofPos(pos,*_etPtr,_sc,cpij,_sp));
		_treeLogLikelihood += tmpLL;
		_llpp.push_back(tmpLL);
	}
}


void mainSemphy::computeLikelihoodAndLikelihoodPerPositionAndPosterior() {
	_treeLogLikelihood = 0.0;
	_llpp.clear();
	_posterior.clear();
	VdoubleRep posPost(_sp.categories());
	//	getPosteriorOfRatesAndLLPP(*_etPtr, _sc, _sp, cup, 
	computePijGam cpij;
	cpij.fillPij(*_etPtr,_sp);
	for (int pos=0; pos < _sc.seqLen() ;++pos) {
	  MDOUBLE tmpLL = log(likelihoodComputation::getLofPosAndPosteriorOfRates(pos,*_etPtr,_sc,cpij,_sp, posPost));
		_treeLogLikelihood += tmpLL;
		_llpp.push_back(tmpLL);
		_posterior.push_back(posPost);
	}
}

void mainSemphy::printLikelihoodAndLikelihoodPerPosition() const {
	out()<<"# The log likelihood of the tree is:"<<endl;
	out()<<_treeLogLikelihood<<endl;
	LOG(3,<<"# The log likelihood of the tree is:"<<endl);
	LOG(1,<<_treeLogLikelihood<<endl);

	if (_args_info.PerPosLike_given){
		out()<<"# The log likelihood per position:"<<endl;
		out()<<_llpp<<endl;
		LOG(3,<<"# The log likelihood per position:"<<endl);
		LOG(1, <<_llpp<<endl);
	} 
}

void  mainSemphy::printPosterior() const {
	if (_args_info.PerPosPosterior_given){  
	  out()<<"# The posterior of the rates is:"<<endl;
	  out()<<_posterior<<endl;
	  LOG(8,<<"# centroieds for the rate"<<endl);
	  for (int i=0;i<_sp.categories();++i)
		LOG(8,<<"  "<< _sp.rates(i));
	  LOG(8,<< endl);
	  LOG(3,<<"# The posterior of the rates is:"<<endl);
	  LOG(1,<<_posterior<<endl);
	}
}

void mainSemphy::setWeights(const Vdouble& weights) {
	if (_weights) delete _weights;
	_weights = new Vdouble(weights);
}


void mainSemphy::computeTree(bool doBootstrapOneIteration) {
	if (_args_info.SEMPHY_given) {
		computeSemphyTree(); // Note that computeSemphyTree calls computeNJtree
	} else if (_args_info.DistanceTableEstimationMethod_group_counter) { // Do NJ of some sort without SEMPHY
		computeNJtree(doBootstrapOneIteration);
	}
}
 
		
	
void mainSemphy::optimizeParameters() {
	// Don't optimize parameters in case SEMPHY was not called, and we
	// ran an iterative NJ method that already did the optimization
	if (!_args_info.SEMPHY_given
		&& (_args_info.commonAlphaDTME_given || _args_info.posteriorDTME_given))
		return;

	// 1. OPTIMIZE BRANCH LENGTH
	// 2. OPTIMIZE ALPHA
	// 3. OPTIMIZE BOTH
	if (_args_info.bbl_given) {// this will optimize both branch lengths and alpha if needed.
		optimizeBranchLengths();
	} else if (_args_info.rate_flag) { // DO GLOBAL RATE OPTIMIZATION (WITH OR WITHOUT ALPHA)
		optimizeGlobalRate();
	} else if (_args_info.optimizeAlpha_flag) { // here we do alpha but NO bbl, and no global rate.
		optimizeAlphaOnly();
	}
}


void mainSemphy::compute(bool doBootstrapOneIteration) {
	// The program has layers. The first layer computs a tree.
	computeTree(doBootstrapOneIteration);

	// The second layer is parameter optimization
	optimizeParameters(); //(branch lengths, alpha, both, ...) 
	
	// The third layer computes some likelihoods on a given tree.
	computeLikelihoodAndLikelihoodPerPositionAndPosterior();
}

void mainSemphy::output() const {
	// The forth layer prints results.
	out()<<"# Finished tree reconstruction."<<endl;
	LOG(3,<<"# Finished tree reconstruction."<<endl);
	printLikelihoodAndLikelihoodPerPosition();
	printPosterior();
	printTree();
	printTreeToTreeFile();
}

// this function is NOT needed now for SEMPHY, per-ce, but is used with SEMPHY LIB.
void mainSemphy::extractPosteriorProb(VVdoubleRep & posteriorProbVV) const {
	likelihoodComputation::getPosteriorOfRates(*_etPtr,_sc,_sp,posteriorProbVV,_weights);
}
