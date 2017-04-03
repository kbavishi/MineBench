// 	$Id: shortenTree.cpp 2399 2014-03-13 22:43:51Z wkliao $	
#include "tree.h"
#include "logFile.h"
#include "shortenTree_cmdline.h"

int main(int argc,char* argv[]) 
{
  gengetopt_args_info args_info;
  cmdline_parser(argc,argv, &args_info);
  myLog::setLog(args_info.Logfile_arg, args_info.verbose_arg);
  myLog::printArgv(5, argc, argv);
  
  double max=1e+32;
  double min=-max;
  if (args_info.max_given) 
	max=args_info.max_arg;
  if (args_info.min_given) 
	min=args_info.min_arg;
  tree t(args_info.tree_arg);
  vector<tree::nodeP> nv;
  t.getAllNodes(nv, t.getRoot());
  for (vector<tree::nodeP>::iterator i=nv.begin();i!=nv.end();++i){
	if (!(*i)->isRoot()) {
	  if ((*i)->dis2father()>max) {
		LOG(4,<< "changing then brnach above "<<(*i)->name()<<" from "<<(*i)->dis2father()<<" to "<<max<<endl);
		  (*i)->setDisToFather(max);
		}
		if ((*i)->dis2father()<min){
		  LOG(4,<< "changing then brnach above "<<(*i)->name()<<" from "<<(*i)->dis2father()<<" to "<<min<<endl);
		  (*i)->setDisToFather(min);
		}
	  }
	}
	if (string(args_info.outputTree_arg)!=string("-")) {
	  ofstream treeO(args_info.outputTree_arg);
		if (! treeO.is_open()) {
			errorMsg::reportError("can not open tree output file");
		}
		t.output(treeO);
		treeO.close();
	} else { 
	  t.output(cout);
	}
}
