// 	$Id: distanceEstimation.cpp 2399 2014-03-13 22:43:51Z wkliao $	

#include "distanceEstimation_cmdline.h"
#include "tree.h"
#include "sequenceContainer.h"

#include "fromCountTableComponentToDistance.h"
#include "posteriorDistance.h"
#include "logFile.h"
#include "treeUtil.h"
#include "likeDist.h"
#include "someUtil.h" 
#include "codon.h"
#include "recognizeFormat.h"

#include "pDistance.h"
#include "jcDistance.h"
#include "likeDist.h"
#include "givenRatesMLDistance.h"
#include "likelihoodComputation.h"
#include "bblEM.h"
#include "bestAlpha.h"
#include "readDatMatrix.h"
#include "chebyshevAccelerator.h"
#include "cmdline2EvolObjs.h"

#include <iostream>
#include <cassert>
#include <iomanip>
using namespace std;

void printLikelihood(const MDOUBLE treeLogLikelihood, ostream &out) {
	out<<"# The log likelihood of the tree is:"<<endl;
	out<<treeLogLikelihood<<endl;
	LOG(3,<<"# The log likelihood of the tree is:"<<endl);
	LOG(1,<<treeLogLikelihood<<endl);
}
void printLikelihoodPerPosition(const Vdouble &LLPerPos, ostream &out) {
	out<<"# The log likelihood per position:"<<endl;
	out<<LLPerPos<<endl;
	LOG(3,<<"# The log likelihood per position:"<<endl);
	LOG(1, <<LLPerPos<<endl); 
}
void printTree(const tree &et, ostream &out, const int logLvl = 3) {
	int printMsg=max(3,logLvl);
	out<<"# The tree"<<endl;
	et.output(out);
	LOG(printMsg,<<"# The tree"<<endl);
	LOGDO(logLvl,et.output(myLog::LogFile()));
}

int main(int argc,char* argv[]) {
	
	if (argc < 2) errorMsg::reportError("The program must get some parameters in the command line, use -h for help");
	gengetopt_args_info args_info;
	if (cmdline_parser(argc, argv, &args_info) != 0) {
		errorMsg::reportError("error reading command line",1);
	}

	cmdline2EvolObjs<gengetopt_args_info> cmd2Objs(args_info);

	// Initialize log
	cmd2Objs.initializeLogFile();

	myLog::printArgv(1, argc, argv);

	// defualt action
	// if not methods are given, do all.
	if (! args_info.pD_given &&
		! args_info.jc_given &&
		! args_info.homogeneous_given &&
		! args_info.asrvExactGamma_given && 
		! args_info.pairwiseGamma_given &&
		! args_info.allPairwise_given &&
		! args_info.asrvGivenAlpha_given && 
		! args_info.asrvGivenRates_given &&
		! args_info.posterior_given &&
		! args_info.all_given )
	  args_info.all_given=1;
	

	if(args_info.all_given||args_info.allPairwise_given) {
	  args_info.pD_given=1;
	  args_info.jc_given=1;
	  args_info.homogeneous_given=1;
	  args_info.asrvExactGamma_given=1;
	  args_info.pairwiseGamma_given=1;
	}
	if(args_info.all_given) {
	  args_info.asrvGivenAlpha_given=1;
	  args_info.asrvGivenRates_given=1;
	  args_info.posterior_given=1;
	}



	// check that we have all we need:
	if (!args_info.tree_given) {
		errorMsg::reportError("Must enter input tree",1);
	}
	if (!args_info.alpha_given) {
		errorMsg::reportError("Must set gamma",1);
	}

	if (args_info.optimizeAlpha_flag) {
		errorMsg::reportError("Gamma optimisation not allowed",1);
	}
	if (args_info.asrvGivenRates_given && !args_info.ratesFilename_given) {
		errorMsg::reportError("Must give list of rates to use the asrvGivenRates method",1);
	}

	/*
	 * Create and initialize evolutionary objects as specified by command line parameters
	 */

	// Initialize seed
	cmd2Objs.initializeRandomSeed();

	// Create alphabet
	alphabet *alphPtr = cmd2Objs.cmdline2Alphabet();

	// Create sequenceContainer (read from file)
	sequenceContainer sc = cmd2Objs.cmdline2SequenceContainer(alphPtr);

	// Take care of gaps
	cmd2Objs.takeCareOfGaps(sc);

	// Create trees (read from file, if given, else - exits)
	tree *etPtr = cmd2Objs.cmdline2Tree();
	assert(etPtr!=NULL); 
	tree &et = *etPtr;

	// Create homogeneous stochastic process
	stochasticProcess homogeneousSP = cmd2Objs.cmdline2HomogenuisStochasticProcess();
	likeDist homogeneousLD(homogeneousSP);

	// Create integrated Gamma stochastic process
	stochasticProcess exactGammaSP = cmd2Objs.cmdline2ExactGammaStochasticProcess();
	likeDist exactGammaLD(exactGammaSP);

	// Create stochastic process
	stochasticProcess sp = cmd2Objs.cmdline2StochasticProcess();
	assert(sp.categories() >1); // see that we actually got gamma
	LOG(5, <<"alpha = "<<(static_cast<gammaDistribution*>(sp.distr()))->getAlpha() <<"\t");

	for (int i=0;i<sp.categories();++i)
		LOG(5,<<"category["<<i<<"]="<<sp.rates(i)<<"\t"<<sp.ratesProb(i)<<endl);

	likeDist gammaFixedLD(sp);


	// Create pupAll object (what's the meaning of this name???)
	//	replacementModel *probModPtr = cmd2Objs.cmdline2ReplacementModel();

	// Create output stream
	ostream *outP = cmd2Objs.cmdline2OutputStream();
	ostream &out = *outP;


	// Read corresponding rates (there is only one rates file per run)
	Vdouble rates;

	if (args_info.asrvGivenRates_given) {
	  ifstream ratesListFile;
	  
	  //	above we require that args_info.asrvGivenRates_given=TURE
	  ratesListFile.open(args_info.ratesFilename_arg);
	  if (! ratesListFile.good()) {
	    string errorMessage ="can not open rates file \"";
	    errorMessage += args_info.ratesFilename_arg;
	    errorMessage += "\"";
	    errorMsg::reportError(errorMessage);
	  }
	  string line;
	  while (!ratesListFile.eof()) {
	    if (!getline(ratesListFile, line)) break;
	    rates.push_back(atof(line.c_str()));
	  }
	  if (rates.size() != sc.seqLen()){
	    string errorMessage = "Length of rates list '";
	    errorMessage += args_info.ratesFilename_arg;
	    errorMessage += "' mismatches the length of the sequences ";
	    errorMsg::reportError(errorMessage);
	  }
	  ratesListFile.close();
	}
	
	givenRatesMLDistance givenRatesLD(homogeneousSP, rates);


	VVdouble posteriorProbVV; // pos * rate
	if (args_info.posterior_given){			// this part is only relavent when side-information based methods are used
	  /*
	   * Likelihood computation possibly after branch lengths and alpha optimization
	   */
	  MDOUBLE treeLogLikelihood = 0;
	  Vdouble LLPerPos;
	  treeLogLikelihood = likelihoodComputation::computeLikelihoodAndLikelihoodPerPosition(sc, et, sp, LLPerPos);
	  
	  // Print likelihood and tree
	  LOGDO(3,printLikelihood(treeLogLikelihood, myLog::LogFile()));
	  LOGDO(3,printLikelihoodPerPosition(LLPerPos, myLog::LogFile()));
	  LOGDO(3,printTree(et, myLog::LogFile()));
	  
	  // Extract and print posterior probability
	  likelihoodComputation::getPosteriorOfRates(et, sc, sp, posteriorProbVV);
	  LOG(3,<<posteriorProbVV<<endl);
	}


	// Create stochastic process for alpha optimisation
	stochasticProcess spOptAlpha = cmd2Objs.cmdline2StochasticProcess();
	assert(spOptAlpha.categories() >1); // see that we actually got gamma
	(static_cast<gammaDistribution*>(spOptAlpha.distr()))->setAlpha(1.0); // default value - so that the alpha 
	// will not start off at the right nubmer and couse a problem.


	// this is used in both pairwise alpha and side-information based methods.
	// posteriorProbVV is not used in pairwise gamma, so it can be ampty when used here.
	posteriorDistance gammaLD(spOptAlpha, posteriorProbVV);

	/*
	 * Read pairs of sequances for distance estimation
	 */
	sequenceContainer pairSC;

	ifstream seqListFile;
	string seqid,pairFileName,seqidCompare,ratesFileName;	// this is not ideal!
	seqListFile.open(args_info.pairListFilename_arg);
	if (! seqListFile.good()){
	  string errorMessage = "can not open sequence pair list file ";
	  errorMessage + args_info.pairListFilename_arg;
	  errorMsg::reportError(errorMessage);
	}
	// Loop per pair of sequences in list file seqListFile
	while (!seqListFile.eof()){
		// 1. Read pair from file
		if (!getline(seqListFile, pairFileName, ' ')) break;
		if (seqListFile.eof()) break;
		if (!getline(seqListFile, seqid, '\n')) break;
		out<<seqid <<"\t";
		out<<pairFileName<<"\t";
		LOG(5,<<endl<< pairFileName <<"\t" << seqid <<endl);

		ifstream ins;
		ins.open(pairFileName.c_str());
		if (! ins.good()) {
		  vector<string> message;
		  message.push_back("can not open sequence pair file "+pairFileName);
		  message.push_back(pairFileName);
		  errorMsg::reportError(message);
		}
		pairSC = recognizeFormat::read(ins,alphPtr);
		ins.close();
		
		out<<fixed<<setprecision(7)<<right;

		// 2. Estimate distance
		// 2.1. p distance
		if (args_info.pD_given) {
		    pDistance pD;
		    out <<setw(10)<<pD.giveDistance(pairSC[0], pairSC[1], NULL)<<"\t";
		}
		
		// 2.2. JC distance
		if (args_info.jc_given) {
		    jcDistance jcD(args_info.alphabet_arg);
		    out<<setw(10)<<jcD.giveDistance(pairSC[0], pairSC[1], NULL)<<"\t";
		}

		// 2.3. ML distance using given replacement model, with uniform rates
		if (args_info.homogeneous_given) {
		    out<<setw(10)<<homogeneousLD.giveDistance(pairSC[0], pairSC[1], NULL)<<"\t";
		}

		// 2.4. ML distance using given replacement model, with Gamma ASRV
		if (args_info.pairwiseGamma_given) {
		    LOG (8,<<"pairwise gamma"<<endl);	       
		    MDOUBLE alpha=0.0;	 // for best alpha over the branch
		    gammaLD.setAlpha(1.0); // see above
		    out << setw(10) << 
			gammaLD.giveDistanceOptAlphaForPairOfSequences(pairSC[0], pairSC[1], NULL, NULL, &alpha);
		    LOGDO(12,out <<" "<<setw(10)<<alpha);
		    out <<"\t";\
		}

		// 2.5 ML distance using given replacement model, with Gamma ASRV
		if (args_info.asrvGivenAlpha_given)
		    out<<setw(10)<<gammaFixedLD.giveDistance(pairSC[0], pairSC[1], NULL)<<"\t";


		// 2.5.2 ML distance using given integrated gamma replacement model
		if (args_info.asrvExactGamma_given) {
		    out<<"ExactGamma "<<setw(10)<<exactGammaLD.giveDistance(pairSC[0], pairSC[1], NULL)<<"\t";
		}
		
		  
		// 2.6 Use given/inferred rates of all sites for
		// estimation of the pairwise distance
		if (args_info.asrvGivenRates_given)
		    out<<setw(10)<<givenRatesLD.giveDistance(pairSC[0], pairSC[1], NULL)<<"\t";

 		// 2.7. Estimate posterior probability for each
 		// descreate rate category and each site (based on the
 		// given tree and tree-sequences) and then use them for
 		// an estimation of the pairwise distance
 		if (args_info.posterior_given) {
		    posteriorDistance gammaLD2(sp,posteriorProbVV);
		    out <<gammaLD2.giveDistance(pairSC[0], pairSC[1], NULL)<<"\t";
		}
		out  << endl;
	}
	seqListFile.close();

	if (outP != &cout) delete outP;
	delete alphPtr;
	return 0;
}
