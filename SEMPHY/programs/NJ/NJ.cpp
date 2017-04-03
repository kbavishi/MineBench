// 	$Id: NJ.cpp 2399 2014-03-13 22:43:51Z wkliao $	

/* TODO: 
 - add time printing each iteration
*/

#include "NJ_cmdline.h"
#include "tree.h"
#include "sequenceContainer.h"

#include "fromCountTableComponentToDistance.h"
#include "njGamma.h"
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

#include "distanceTable.h"
#include "nj.h"

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

// optimize branch lengths and/or alpha
// NOTE: newAlpha is not set if alpha is not optimized
void optimizeBranchLengthsAndOrAlpha(tree &et, const sequenceContainer &sc, stochasticProcess &sp, 
				     const gengetopt_args_info &args_info, MDOUBLE &newAlpha, MDOUBLE &newTreeLogLikelihood) {
  if (args_info.optimizeAlpha_flag && !args_info.dontOptimizeBranchLengths_flag) { // b.l. & alpha
    bestAlphaAndBBL optimizer(et, sc, sp, NULL);//_weights , 1.5,5,0.01,0.05,_args_info.ADVNumOfBBLIterInBBLPlusAlpha_arg);
    newAlpha=optimizer.getBestAlpha();
    newTreeLogLikelihood = optimizer.getBestL();
  } else if (args_info.optimizeAlpha_flag) { // only alpha
    bestAlphaFixedTree optimizer(et, sc, sp, NULL);
    newAlpha=optimizer.getBestAlpha();
    newTreeLogLikelihood = optimizer.getBestL();
  } else if (!args_info.dontOptimizeBranchLengths_flag) { // only b.l.
    bblEM optimizer(et, sc, sp, NULL);
    newTreeLogLikelihood = optimizer.getTreeLikelihood();
    // newAlpha=(static_cast<gammaDistribution*>(sp.distr()))->getAlpha();
  } else {
    newTreeLogLikelihood = likelihoodComputation::getTreeLikelihoodAllPosAlphTheSame(et, sc, sp, NULL);
  }
}

int main(int argc,char* argv[]) {
  
  const MDOUBLE epsilonLikelihoodImprovement=0.01;

  if (argc < 2) errorMsg::reportError("The program must get some parameters in the command line, use -h for help");
  gengetopt_args_info args_info;
  if (cmdline_parser(argc, argv, &args_info) != 0) {
    errorMsg::reportError("error reading command line",1);
  }
  
  cmdline2EvolObjs<gengetopt_args_info> cmd2Objs(args_info);
  
  // Initialize log
  cmd2Objs.initializeLogFile();
  
  myLog::printArgv(1, argc, argv);
  
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
  //	tree *etPtr = cmd2Objs.cmdline2Tree();

  // Create homogeneous stochastic process
  stochasticProcess homogeneousSP = cmd2Objs.cmdline2HomogenuisStochasticProcess();

  // Create integrated Gamma stochastic process
  // 	stochasticProcess exactGammaSP = cmd2Objs.cmdline2ExactGammaStochasticProcess();
  // 	likeDist exactGammaLD(exactGammaSP);

  // Create stochastic process with gamma ASRV. If alpha was given it will be set
  stochasticProcess sp;
  if (args_info.optimizeAlpha_flag) {
    sp = cmd2Objs.cmdline2StochasticProcessThatRequiresAlphaOptimization();
  } else {
    sp = cmd2Objs.cmdline2StochasticProcess();
  }
  assert(sp.categories() >1); // see that we actually got gamma

  // Create output stream
  ostream *outP = cmd2Objs.cmdline2OutputStream();
  ostream &out = *outP;


  // **********************************************************************
  // *** 1st iteration of NJ with either homogeneous rates ****************
  // *** or using a given alpha *******************************************
  // **********************************************************************

  // Calculate distance table
  VVdouble distTable;
  vector<string> vNames;
  if (!args_info.alpha_given) { // homogeneous rates
    likeDist ld(homogeneousSP);
    giveDistanceTable(&ld,sc,distTable,vNames,NULL);
    LOG(3,<<"# The initial homogeneous rates NJ tree"<<endl);
  } else {			// use the given alpha
    likeDist ld(sp);
    giveDistanceTable(&ld,sc,distTable,vNames,NULL);
    LOG(3,<<"# The initial NJ tree based on given alpha"<<endl);
  }
  
  // Do NJ
  MDOUBLE treeLogLikelihood=0.0;
  NJalg nj;
  MDOUBLE newAlpha = 99.0; 	// temporarly only
  if (args_info.alpha_given) newAlpha = args_info.alpha_arg;
  tree et = nj.computeTree(distTable,vNames);
  LOGDO(3,et.output(myLog::LogFile()));

  // Optimize branch lengths and/or alpha
  optimizeBranchLengthsAndOrAlpha(et, sc, sp, args_info, newAlpha, treeLogLikelihood);
  LOG(3,<<endl<<"# The initial NJ tree after branch length and/or alpha optimisation"<<endl);
  LOG(3,<<"# alpha:"<<endl<<newAlpha<<endl);
  if (newAlpha == 99.0) LOG(3,<<"# No alpha given, and optimization was not done"<<endl);
  LOG(3,<<"# Log likelihood:"<<endl<<treeLogLikelihood<<endl);
  LOGDO(3,et.output(myLog::LogFile()));

  // If iteration is not required then output results and return
  if (args_info.dontIterate_flag || !args_info.optimizeAlpha_flag) {
    out <<"No iterations were done."<<endl<<"Final results:"<<endl;
    LOG(3,<<"No iterations were done."<<endl<<"Final results:"<<endl);
    printLikelihood(treeLogLikelihood, out);
    printTree(et, out, myLog::LogLevel());

    if (outP != &cout) delete outP;
    delete alphPtr;
    return 0;
  }

  // **********************************************************************
  // *** Iterative NJ with common alpha distance measure ******************
  // **********************************************************************

  MDOUBLE bestLL, bestAlpha;
  tree bestTree;
  int iterationNum = 0;
  do {
    ++iterationNum;
    bestLL=treeLogLikelihood;
    bestTree=et;
    bestAlpha=newAlpha;
    (static_cast<gammaDistribution*>(sp.distr()))->setAlpha(bestAlpha); // set alpha to the new value

    likeDist ld(sp);
    vNames.clear();			// will be refilled later
    VVdouble disTable;

    giveDistanceTable(&ld,sc,disTable,vNames,NULL);
    et=nj.computeTree(disTable,vNames);
    optimizeBranchLengthsAndOrAlpha(et, sc, sp, args_info, newAlpha, treeLogLikelihood);

    LOG(3,<<"# The NJ tree after iteration "<<iterationNum<<endl);
    LOGDO(3,et.output(myLog::LogFile()));
    LOG(3,<<"# Log likelihood for the NJ tree"<<endl<<treeLogLikelihood<<endl);
    LOG(3,<<"# Alpha for the NJ tree"<<endl<<newAlpha<<endl);
  } while (treeLogLikelihood > bestLL + epsilonLikelihoodImprovement);

  out <<"Finished "<<iterationNum<<" iterations"<<endl<<"Final results:"<<endl;
  LOG(3,<<"Finished "<<iterationNum<<" iterations"<<endl<<"Final results:"<<endl);
  printLikelihood(bestLL, out);
  out<<"Alpha:"<<endl<<newAlpha<<endl;
  LOG(3,<<"Alpha:"<<endl<<newAlpha<<endl);
  printTree(bestTree, out, myLog::LogLevel());

  if (outP != &cout) delete outP;
  delete alphPtr;
  return 0;
}
