// $Id: distanceBasedSeqs2TreeSA.cpp 2399 2014-03-13 22:43:51Z wkliao $	

/* TODO: 
   + Implement the option for bootstraping a given tree with given side info for rate4site and posterior
   + Remove alpha optimization in BBL in bootstrap iterations
   + add time printing each iteration
*/

#include "distanceBasedSeqs2TreeSA_cmdline.h"
#include "distanceBasedSeqs2Tree.h"

#include "tree.h"
#include "sequenceContainer.h"

#include "logFile.h"
#include "treeUtil.h"
#include "someUtil.h" 
#include "codon.h"
#include "recognizeFormat.h"

#include "pDistance.h"
#include "jcDistance.h"
#include "likeDist.h"
#include "givenRatesMLDistance.h"
#include "pairwiseGammaDistance.h"
#include "likelihoodComputation.h"
#include "bblEM.h"
#include "bestAlpha.h"
#include "readDatMatrix.h"
#include "chebyshevAccelerator.h"
#include "cmdline2EvolObjs.h"

#include "nj.h"
#include "bootstrap.h"

#include <iostream>
#include <cassert>
#include <iomanip>
using namespace std;


void checkArgs(gengetopt_args_info &args_info)
{
    // Check that no useless parameters were given, and that the configuration chosen has been implemented in this beta version of the program
    if (args_info.constraint_given)
		errorMsg::reportError("Can't take -constraint: This program doesn't know to use for a constraint tree yet",1);
    if (args_info.dontOptimizeBranchLengths_given)
		errorMsg::reportError("Can't take -dontOptimizeBranchLengths: This beta version can only be used with optimization of branch lengths",1);

	// Check options related to a given tree and to bootstrap
    if (args_info.tree_given) {
		if (!(args_info.bootstrap_given || args_info.commonAlpha_given || args_info.rate4site_given || args_info.posterior_given))
			errorMsg::reportError("Parameter -tree can only be given for bootstrap or as initial tree for an iterative method",1);
		if (!args_info.alpha_given) {
			if (args_info.bootstrap_given && (args_info.commonAlpha_given || args_info.posterior_given))
				errorMsg::reportError("When giving -tree for bootstrapping with gamma-ASRV, alpha must also be given",1);
		}
	}

	// Check that the method chosen is compatible with the configuration of the other parameters and flags
	if (args_info.commonAlpha_given || args_info.posterior_given) {
		if (!args_info.optimizeAlpha_given)
			errorMsg::reportError("Must use --optimizeAlpha with --commonAlpha or -posterior: This beta version can only be used with iterative optimization",1);
	}
	if (args_info.rate4site_given || args_info.homogeneousRates_given || args_info.jc_given || args_info.pD_given){
		if (!args_info.homogeneous_flag) {
			args_info.homogeneous_flag=1;
		}
		if (args_info.optimizeAlpha_given || args_info.alpha_given) // problematic with homogeneous
			errorMsg::reportError("Must not use --optimizeAlpha or --alpha with --rate4site or --homogeneousRates because the later can work only with --homogeneous",1);
	}
	else if (args_info.homogeneous_flag)
		errorMsg::reportError("Can't take --homogeneous: Currently implemented methods must use ASRV",1);
}







int main(int argc,char* argv[]) {
  
	if (argc < 2) errorMsg::reportError("The program must get some parameters in the command line, use -h for help");
	gengetopt_args_info args_info;
	if (cmdline_parser(argc, argv, &args_info) != 0) {
		errorMsg::reportError("error reading command line",1);
	}
  
	checkArgs(args_info);


	cmdline2EvolObjs<gengetopt_args_info> cmd2Objs(args_info);


	// Initialize log
	cmd2Objs.initializeLogFile();
  
	myLog::printArgv(1, argc, argv);
  
	/*
	 * Create and initialize evolutionary objects as specified by command line parameters
	 */
  
	// 1. Initialize seed
	cmd2Objs.initializeRandomSeed();

	// 2. Create alphabet
	alphabet *alphPtr = cmd2Objs.cmdline2Alphabet();

	// 3. Create sequenceContainer (read from file)
	sequenceContainer sc = cmd2Objs.cmdline2SequenceContainer(alphPtr);

	// Take care of gaps
	cmd2Objs.takeCareOfGaps(sc);

	/******************************************
	 * 4. Construct the object of the requested distance-based sequences-to-tree method
	 */

	// 4.1. Construct stochastic process. If alpha was given it will be set
	stochasticProcess sp;
	if (args_info.rate4site_given || args_info.homogeneousRates_given) {
		sp = cmd2Objs.cmdline2HomogenuisStochasticProcess();
	}
	if (args_info.commonAlpha_given || args_info.posterior_given) {
		if (args_info.optimizeAlpha_flag) {
			sp = cmd2Objs.cmdline2StochasticProcessThatRequiresAlphaOptimization();
			assert(sp.categories() >1); // see that we actually got gamma
		} else {
			errorMsg::reportError("The alpha optimization (--optimizeAlpha) must be chosen for iterative methods that use gamma-ASRV.",1);
		}
	}
	if (args_info.pairwiseGamma_given) {
		if (args_info.optimizeAlpha_flag) {
			sp = cmd2Objs.cmdline2StochasticProcessThatRequiresAlphaOptimization();
		} else if (args_info.alpha_given) {
			sp = cmd2Objs.cmdline2StochasticProcess();
		} else {
			errorMsg::reportError("--pairwizeGamma must work with either --alpha or --optimizeAlpha or both.",1);
		}
		assert(sp.categories() >1); // see that we actually got gamma
	}

	// 4.2. Construct the object of the requested distance-based tree-reconstruction method (class distances2Tree)
	distances2Tree* d2tPtr = new NJalg();

	// 4.3. Construct the object of the requested distance estimation method (class distanceMethod) distanceMethod *dmPtr = NULL;
	// And construct the object of the distance-based sequences-to-tree method (class distanceBasedSeqs2Tree)
	distanceMethod *dmPtr = NULL;
	distanceBasedSeqs2Tree *s2tPtr = NULL;
	if (args_info.pD_given) {
		dmPtr = new pDistance();
		s2tPtr = new distanceBasedSeqs2Tree(*dmPtr, *d2tPtr);

	} else if (args_info.jc_given) {
		dmPtr = new jcDistance(alphPtr->size());
		s2tPtr = new distanceBasedSeqs2Tree(*dmPtr, *d2tPtr);

	} else if (args_info.homogeneousRates_given) {
		dmPtr = new likeDist(sp,args_info.epsilonLikelihoodImprovement_arg);
		s2tPtr = new distanceBasedSeqs2Tree(*dmPtr, *d2tPtr);

	} else if (args_info.pairwiseGamma_given) {
		if (args_info.optimizeAlpha_flag) {
			dmPtr = new pairwiseGammaDistance(sp,args_info.epsilonLikelihoodImprovement_arg); // distance method that optimizes alpha for the pair of sequences
		} else {
			dmPtr = new likeDist(sp,args_info.epsilonLikelihoodImprovement_arg); // distance method that uses the given alpha with no optimization
		}
		s2tPtr = new distanceBasedSeqs2Tree(*dmPtr, *d2tPtr);

		// Iterative methods
	} else if (args_info.commonAlpha_given) {
		likeDist* ldPtr = new likeDist(sp,args_info.epsilonLikelihoodImprovement_arg);
		s2tPtr = new commonAlphaDistanceSeqs2Tree(*ldPtr, *d2tPtr,
												  NULL, args_info.epsilonLikelihoodImprovement_arg,
												  args_info.epsilonLikelihoodImprovement4alphaOptimiz_arg,
												  args_info.epsilonLikelihoodImprovement4BBL_arg);
		dmPtr= ldPtr;

	} else if (args_info.posterior_given) {
		posteriorDistance* pdPtr = new posteriorDistance(sp,args_info.epsilonLikelihoodImprovement_arg);
		s2tPtr = new posteriorDistanceSeqs2Tree(*pdPtr, *d2tPtr,
												NULL, args_info.epsilonLikelihoodImprovement_arg,
												args_info.epsilonLikelihoodImprovement4alphaOptimiz_arg,
												args_info.epsilonLikelihoodImprovement4BBL_arg);
		dmPtr=pdPtr;
	} else if (args_info.rate4site_given) {
		givenRatesMLDistance* grdPtr = new givenRatesMLDistance(sp,args_info.epsilonLikelihoodImprovement_arg);
		s2tPtr = new rate4siteDistanceSeqs2Tree(*grdPtr, *d2tPtr,
												NULL, args_info.epsilonLikelihoodImprovement_arg,
												args_info.epsilonLikelihoodImprovement4alphaOptimiz_arg,
												args_info.epsilonLikelihoodImprovement4BBL_arg);
		dmPtr=grdPtr;
	} else {
		errorMsg::reportError("BUG: The given method was not implemented! Please report this to the people in charge of distanceBasedSeqs2TreeSA.");
	}

	// 5. Get tree, if given
	tree *givenTreePtr = NULL;
	if (args_info.tree_given) {
		givenTreePtr = cmd2Objs.cmdline2Tree();
		if (!givenTreePtr) errorMsg::reportError(string("Failed to read input tree")+args_info.tree_arg,1);
	}

	// 6. Create output stream
	ostream *outP = cmd2Objs.cmdline2OutputStream();
	ostream &out = *outP;

	// 7. Create tree output file
	ostream* treeOutP =  cmd2Objs.cmdline2TreeOutputStream();
	ostream &treeOut = *treeOutP;

	/*
	 * Performe distance-based tree reconstrustion from the sequences
	 */
	tree et;

	// If only bootstrap was requested (i.e., when a tree is given) then use the given tree and skip straight to the bootstrap stage
	if (args_info.bootstrap_given && args_info.tree_given) {
		et = *givenTreePtr;
		// no need to use alpha_arg because it was already set into sp by cmdline2EvolObjs::cmdline2StochasticProcessThatRequiresAlphaOptimization
		//	alpha = args_info.alpha_arg;

	} else {
		if (!args_info.tree_given) {
			if (args_info.commonAlpha_given) {
				commonAlphaDistanceSeqs2Tree *caS2tPtr = static_cast<commonAlphaDistanceSeqs2Tree*>(s2tPtr);
			
				if (args_info.alpha_given) { // use the given alpha
				  et = caS2tPtr->seqs2TreeIterative(sc, args_info.alpha_arg, NULL);
				} else {		       // homogeneous rates in first iteration
				  et = caS2tPtr->seqs2TreeIterative(sc, (Vdouble *)NULL, NULL);
				}
				
			} else {  // all other methods
				et = s2tPtr->seqs2Tree(sc);
			}
		// If an initial tree was given then pass it to the iterative seqs2Tree method
		} else {
			iterativeDistanceSeqs2Tree *itS2tPtr = static_cast<iterativeDistanceSeqs2Tree*>(s2tPtr);
			if (args_info.alpha_given) {
			  et = itS2tPtr->seqs2TreeIterative(sc, *givenTreePtr, args_info.alpha_arg, NULL);
			} else {
			  et = itS2tPtr->seqs2TreeIterative(sc, *givenTreePtr);
			}
		}
		
		// Output tree
		LOGDO(3,printTime(myLog::LogFile()));
		out <<"Finished iterative tree reconstruction"<<endl<<"Final results:"<<endl;
		LOG(3,<<"Finished iterative tree reconstruction"<<endl<<"Final results:"<<endl);

		if (args_info.pD_given) {
			out<<"No likelihood information for p-distance"<<endl;
		} else if (args_info.jc_given) {
			out<<"No likelihood information for jc distance"<<endl;
		} else if (args_info.homogeneousRates_given) {
			out<<"# The log likelihood of the tree is:"<<endl;
			out<<s2tPtr->getLogLikelihood()<<endl;
			LOG(3,<<"# The log likelihood of the tree is:"<<endl);
			LOG(3,<<s2tPtr->getLogLikelihood()<<endl);
		} else {
			out<<"# The log likelihood of the tree is:"<<endl;
			out<<s2tPtr->getLogLikelihood()<<endl;
			LOG(3,<<"# The log likelihood of the tree is:"<<endl);
			LOG(3,<<s2tPtr->getLogLikelihood()<<endl);

			MDOUBLE alpha = static_cast<iterativeDistanceSeqs2Tree *>(s2tPtr)->getAlpha();
			out<<"# Alpha:"<<endl<<alpha<<endl;
			LOG(3,<<"# Alpha:"<<endl<<alpha<<endl);
		}

		out<<"# The tree"<<endl;
		et.output(out);
		LOG(3,<<"# The tree"<<endl);
		LOGDO(3,et.output(myLog::LogFile()));
	}
	
	/*
	 * Do bootstrap iterations if asked
	 */
	if (!args_info.bootstrap_given) {
		if (!args_info.treeoutputfile_given)
			et.output(treeOut);
	} else {
		out<<endl<<"Starting "<< args_info.bootstrap_arg <<" bootstrap iterations...\n";
		LOG(3,<<"Starting "<< args_info.bootstrap_arg <<" bootstrap iterations...\n");
	  
		bootstrap::treeVec bootstrapTrees;
		Vdouble weights(sc.seqLen());
		Vstring names = sc.names();
		tree bootstrapTree;
		for (int treeNumber=0; treeNumber<args_info.bootstrap_arg; treeNumber++) {
			LOG(5,<<endl<<"Calculating bootstrap tree "<<treeNumber<<"...\n");
			LOGDO(5,printTime(myLog::LogFile()));
			for (int pos=0; pos<sc.seqLen(); pos++) 
				weights[pos] = (rand() % sc.seqLen());
		
			// compute tree
			bootstrapTree = s2tPtr->seqs2TreeBootstrap(sc, &weights);
			bootstrapTrees.push_back(bootstrapTree);
		}
		bootstrap bs(bootstrapTrees);
		map<int,MDOUBLE> mymap=bs.getWeightsForTree(et);

		// Output bootstrapped tree
		out <<endl<<"Finished bootstrap"<<endl;
		LOG(3,<<endl<<"Finished bootstrap"<<endl);
		bs.printTreeWithBPvalues(out,et,mymap);
		if (args_info.treeoutputfile_given)
			bs.printTreeWithBPvalues(treeOut,et,mymap);
		LOGDO(3,bs.printTreeWithBPvalues(myLog::LogFile(),et,mymap));
	}

	if (outP != &cout) delete outP;
	if (treeOutP != &cout) delete treeOutP;
	delete alphPtr;
	delete s2tPtr;
	delete d2tPtr;
	delete dmPtr;
	if (givenTreePtr) delete givenTreePtr;

	return 0;
}
