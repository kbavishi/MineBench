// 	$Id: checkCov.cpp 2399 2014-03-13 22:43:51Z wkliao $	

#include "definitions.h"
#include "checkCovClass.h"
#include "checkCov_cmdline.h"
#include "cmdline2EvolObjs.h"
#include "someUtil.h"
#include <iostream>

using namespace std;

int main(int argc,char* argv[]) {	
	gengetopt_args_info args_info;
	cmdline_parser(argc,argv, &args_info);
    myLog::setLog(args_info.Logfile_arg, args_info.verbose_arg);
	LOG(3,<<endl);
	LOG(3,<<" =================================================== "<<endl);
	LOG(3,<<" CheckCov - A program for covarion testing. ver 3.00 "<<endl);
	LOG(3,<<"           Pupko Tal and Nicolas Galtier             "<<endl);
	LOG(3,<<" =================================================== "<<endl<<endl);

	myLog::printArgv(3, argc, argv);

	cmdline2EvolObjs<gengetopt_args_info> c2e(args_info,true);
	//	checkCov c1(args_info.sequence_arg, args_info.tree_arg, modelP, args_info.Logfile_arg, args_info.outputfile_arg,args_info.branch_arg);
	checkCov c1(c2e,args_info.branch_arg); 


	/**** setting branch (split) for covarion analysis */	
	if (args_info.branch_arg >=0) {// we have a node name
	  string nodeName="N"+int2string(args_info.branch_arg);
	  return (c1.run(nodeName)==true);
	} else 
	  if (!args_info.branch_given ||
		  args_info.branch_arg<-1){ //user did not yet give an input node
		//to split, so we go over all the comments that contain a
		//duplication
		vector<tree::nodeP> nv;
		c1.getTree().getAllNodes(nv, c1.getTree().getRoot());
		bool success=false;
		for (vector<tree::nodeP>::iterator i=nv.begin();i!=nv.end();++i){
		  LOG(15,<<"comment is: "<<(*i)->getComment()<<endl);
		  if (((*i)->getComment()).find("D=Y") < ((*i)->getComment()).length())  { // we have a duplication
			//			LOG(3,<<"splitting at "<<(*i)->name()<<endl); 
			success &= c1.run ((*i)->name()); // we want to succeed at least once
		  }
		}
		return(success);
	  } else { // default -> if (args_info.branch_arg==-1){ //user did not yet give an input node to split
		string nameOfNodeToCut(c1.getUserNodeToSplitName());
		//	  LOG(3,<<"splitting at "<<nameOfNodeToCut<<endl); 
		return (c1.run(nameOfNodeToCut));
	  }
	return 2;					// we should never get here!
}

	// get tree, sequence information, and output file option from instruction file.
 //   covarion c1(t1,s1,outputTo);
/*	
	// make one calculation.

	cerr<<"please enter the the test you want to perform: "<<endl;
	cerr<<"1. A likelihood test for covarion"<<endl;
	cerr<<"2. A likelihood ratio test for detecting covarion positions"<<endl;
	cerr<<"3. Estimation of rate and standard deviation of rate at each position"<<endl;
	cerr<<"4. Bayesian test for covarion"<<endl;
	cerr<<"5. Calculate number of replacements expected given a tree"<<endl;
	cerr<<"6. Print splited trees"<<endl;

	
	int option;
	cin>>option;

    switch (option) {
    case 1:{cerr<<" How many bootstrap repetition would you like?"<<endl;
			int bp_num;
			cin>>bp_num;
			//=======================================================
			bool optimizeBrInEachIteration=false;
			bool optimizeBrInEachIterationQuestionAnswered= false;
			while (optimizeBrInEachIterationQuestionAnswered == false) {
			cerr<<" would you like to optimize the branch length in each iteration of the test"<<endl;
			cin>>OptAnswer;
				if (OptAnswer != 'n' && OptAnswer != 'N' &&
					OptAnswer != 'y' && OptAnswer != 'Y') {
					cerr<<" you must enter either Y or N "<<endl;
				}
				else optimizeBrInEachIterationQuestionAnswered=true;
			}

			if (OptAnswer == 'y' || OptAnswer == 'Y') {
				optimizeBrInEachIteration = true;
			}
			//=======================================================

			c1.Test(bp_num,optimizeBrInEachIteration); // number of repetations
			break;}
	case 2:{c1.covarLRTtest();
			break;}
	case 5:{c1.numberOfChangesExpectedPlusStd(t1,s1,100,0);
			break;}
	case 6:{c1.printSplitedTrees();
			break;}


		
	default:{errorMsg::reportError( "option does not exist, or is not implemented yet");}

	}

	
//	c1.RstdsTest();
	delete probmodel; 
	delete distr;
*/
