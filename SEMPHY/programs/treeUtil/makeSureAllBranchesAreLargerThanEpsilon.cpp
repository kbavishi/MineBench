// 	$Id: makeSureAllBranchesAreLargerThanEpsilon.cpp 2399 2014-03-13 22:43:51Z wkliao $	
#include "makeSureAllBranchesAreLargerThanEpsilon_cmdline.h"
#include <cassert>
#include <iostream>
#include <fstream>

#include "tree.h"
#include "logFile.h"

// Purpose: Replace all branch length that are shorter than epsilon with epsilon
int main(int argc,char* argv[]) {
  //  if (argc < 2) errorMsg::reportError("The program must get some parameters in the command line, use -h for help");
  gengetopt_args_info args_info;
  assert(cmdline_parser(argc, argv, &args_info) == 0);

  // Read input tree
  string inFileName(args_info.inputTree_arg);
  tree et(inFileName);
  
  // Replace all short branch lengths with epsilon
  et.makeSureAllBranchesAreLargerThanEpsilon(args_info.epsilon_arg);

  // Read input tree
  ofstream out;
  ostream *outPtr = &cout;
  string outFileName(args_info.outputTree_arg);
  if (outFileName != "-") {
	out.open(outFileName.c_str());
	if (!out.good()) errorMsg::reportError(string("Can't open for writing the file ")+string(args_info.outputTree_arg));
	outPtr = &out;
  }
  et.output(*outPtr);
  if (out.is_open())
	out.close();
  return 0;
}
