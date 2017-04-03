// 	$Id: sametree.cpp 2399 2014-03-13 22:43:51Z wkliao $	
#include <stdlib.h>
#include "treeUtil.h"
#include "sametree_cmdline.h"

int main(int argc,char* argv[]) 
{
  //  if (argc < 2) errorMsg::reportError("The program must get some parameters in the command line, use -h for help");
  gengetopt_args_info args_info;
  if (cmdline_parser(argc, argv, &args_info) != 0) {
	errorMsg::reportError("error reading command line",1);
  }
  
  tree t1(args_info.tree_arg);
  tree t2(args_info.other_arg);

  t1.rootToUnrootedTree();
  t2.rootToUnrootedTree();


  if (sameTreeTolopogy(t1,t2)){
	cout <<"yes"<<endl;
	exit(0);
  } else {
	cout <<"no"<<endl;
	exit(1);
  }
}
	
	
