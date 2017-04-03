// 	$Id: changeShortBLToMinimalEpsilon.cpp 2399 2014-03-13 22:43:51Z wkliao $	
#include <iostream>
#include <fstream>

#include "tree.h"

// Purpose: Replace all branch length that are shorter than epsilon with epsilon
int main(int argc,char* argv[]) {
  if (argc < 2) errorMsg::reportError("The program must get some parameters in the command line, use -h for help");
  gengetopt_args_info args_info;
  assert(cmdline_parser(argc, argv, &args_info) == 0);

  // Read input tree
  string treeFileName(args_info.inputTree_arg);
  tree et(treeFileName);
  
  // Replace all short branch lengths with epsilon
  et.makeSureAllBranchesAreLargerThanEpsilon(args_info.epsilon_arg);

  // Print output tree
  ofstream out(args_info.outputTree_arg);
  if (!out.good()) errorMsg::reportError(string("Can't open for writing the file ")+args_info.outputTree_arg);
  et.output(out);

  return 0;
}
