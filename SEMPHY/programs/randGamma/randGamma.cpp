// 	$Id: randGamma.cpp 2399 2014-03-13 22:43:51Z wkliao $	
#include <string.h>
#include "definitions.h"  
#include <iostream>
#include <fstream>
#include <cassert>
#include <string>
#include <cstdio>
using namespace std;


#include "randGamma_cmdline.h"
#include "logFile.h"
#include "talRandom.h"
#include "readDatMatrix.h"
#include "datMatrixHolder.h"
#include "likelihoodComputation.h"

 void setLog(const char* logfilename, const int loglvl);


 int main(int argc, char* argv[]) {
	// 1. Getting the command line info
	if (argc < 2) errorMsg::reportError("The program must get some parameters in the command line, use -h for list");
	gengetopt_args_info args_info;
	if (cmdline_parser (argc, argv, &args_info) != 0) {
	  errorMsg::reportError("Error in command line parsing",1);
	}

	// 2. Set random seed
	if	(args_info.seed_given) {
	    talRandom::setSeed(args_info.seed_arg);
	}

	// 3. Set log file
	setLog(args_info.Logfile_arg, args_info.verbose_arg);

	// 4. Setting the output stream.
	ostream * outP=NULL;
	ofstream out_tmp;
	if (strcmp(args_info.outputfile_arg,"-")==0)  // cout
	  outP=&cout;
	else {
	  out_tmp.open(args_info.outputfile_arg);
	  outP=&out_tmp;
	}
	ostream& out = *outP;
	
	LOG(3,<<"generating "<< args_info.length_arg<<" random numbers with alpha="<<args_info.alpha_arg<<endl);
	if	(args_info.seed_given) {
		LOG(3,<<"using seed = "<< args_info.seed_arg<<endl);
	}

	// 5. Producing the number and outputing them..
	for (int i=0;i<args_info.length_arg;++i) {
	  out << talRandom::SampleGamma(args_info.alpha_arg)<<endl;
	}
	
	if (strcmp(args_info.outputfile_arg,"-")!=0) out_tmp.close();
} // end of main.

 //=======================================================================================
 void setLog(const char* logfilename, const int loglvl) {
  if (!strcmp(logfilename,"-")||!strcmp(logfilename,""))
	  {
	    myLog::setLogOstream(&cout);
	  }
	else
	  {
	    ofstream* outLF = new ofstream;
	    outLF->open(logfilename);
	    if (!outLF->is_open()) {
	      errorMsg::reportError("unable to open file for reading");
	    }
	    myLog::setLogOstream(outLF);
	  }
	myLog::setLogLvl(loglvl);
	LOG(3,<<"START OF LOG FILE\n\n");
}
