// 	$Id: rateShiftOptions.h 2399 2014-03-13 22:43:51Z wkliao $	
#if !defined ___RATE_SHIFT__OPTION__T__
#define ___RATE_SHIFT__OPTION__T__

#ifdef SunOS
  #include <unistd.h>
#else
	#ifndef __STDC__
	#define __STDC__ 1
	#include "getopt.h"
	#undef __STDC__
	#else
	#include "getopt.h"
	#endif
#endif

#include <string>
#include <fstream>
#include "errorMsg.h"
#include <iostream>
#include "definitions.h"

using namespace std;

class rateShiftOptions{
public:

	explicit rateShiftOptions(int argc, char** argv);
	
	string treefile;
	string seqfile;
	string logFile;
	string referenceSeq; // the results are printed with this seq in each positions.
	int logValue;
	string outFile;
	string treeOutFile;
	
	enum modelNameType {rev,jtt,day,aajc,nucjc,wag,cprev};//,customQ,manyQ , hky,tamura
	modelNameType modelName;
	
	int alphabet_size;
	
	enum optimizeBranchLengthsType {noBBL,mlBBLUniform,mlBBL};
	optimizeBranchLengthsType optimizeBranchLengths;
	
	enum optimizationType {noOptimization,alpha,nu,alphaAndNu};
	optimizationType optimizationType;

  enum rateEstimationMethodType {ebExp, mlRate};
  rateEstimationMethodType rateEstimationMethod;

  int numberOfDiscreteCategories;
  MDOUBLE userInputAlpha;
  MDOUBLE userInputF;
  MDOUBLE userInputNu;
  MDOUBLE userInputGC;
  bool optimizeF;

private:
  ostream* outPtr;
  ofstream out_f;

  
public:
  ostream& out() const { return *outPtr;}

};


#endif
