// $Id: distable.cpp 2399 2014-03-13 22:43:51Z wkliao $

#include "tree.h"
#include "sequenceContainer.h"
#include "mainSemphy.h"
#include "njGamma.h"
#include "logFile.h"
#include "treeUtil.h"
#include "likeDist.h"
#include "someUtil.h"

#include <iostream>
#include <cassert>
using namespace std;


int main(int argc,char* argv[]) {
  if (argc < 2) errorMsg::reportError("The program must get some parameters in the command line, use -h for help");
  semphy_args_info args_info;
  if (semphy_cmdline_parser(argc, argv, &args_info) != 0) {
    errorMsg::reportError("error reading command line",1);
  }

  mainSemphy ms(args_info);
  stochasticProcess tmpSp(ms.getStochasticProcess());
  likeDist ld(tmpSp, 0.01);
  VVdouble disTab;
  vector<string> vNames;
  ms.getDistanceTableAndNames(disTab,vNames,&ld);
  ms.out()<<disTab<<endl<<endl;
//  //  uncomment this to get list of names
//   for (int vNames_I=0;vNames_I<vNames.size();++vNames_I){
//     ms.out()<<vNames[vNames_I]<<endl;
//   }
    return 0;
}
