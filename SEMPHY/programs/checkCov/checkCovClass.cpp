// 	$Id: checkCovClass.cpp 2399 2014-03-13 22:43:51Z wkliao $	
#include "checkCovClass.h"
#include "checkcovFanctors.h"
#include "fromInstructionFile.h"
#include "likelihoodComputation.h"
#include "readDatMatrix.h"
#include "chebyshevAccelerator.h"
#include "uniDistribution.h"
#include "recognizeFormat.h"
#include "bestAlpha.h"
#include "amino.h"
#include "treeUtil.h"
#include "numRec.h"

#include <fstream>
#include <iostream>
#include <cassert>
using namespace std;

//#define VERBOS

/**** checkCOV: given a tree and a specific branch, the 
	  program  tests (for each position) whether one rate or two rates 
	  (for the 2 subtrees defined by the given branch) best describe the data (LRT)*/	


checkCov::checkCov(string seqFile, string treeFile, string modelName,string logFile,
					 string outFile, int nodeNumberForSplit)
{
	setInputParameters(seqFile, treeFile, modelName, logFile,outFile,nodeNumberForSplit);
	//	run();
}


checkCov::checkCov(cmdline2EvolObjs<gengetopt_args_info> &c2e, const int nodeNumberForSplit)
{
  //void checkCov::setInputParameters(string seqFile, string treeFile,
  //string modelName, string logFile, string outFile, int
  //nodeNumberForSplit){
	/**** setting stochastic process */
  //  replacementModel *modelP = c3e.cmdline2ReplacementModelAAOnly();
  _sp=c2e.cmdline2HomogenuisStochasticProcessAAOnly();
  
  /**** setting sequence container */	
  amino alph;
  _sd = c2e.cmdline2SequenceContainer(&alph);
  _sd.changeGaps2MissingData();

  _startPos=0;
  _endPos=_sd.seqLen();
  if (c2e.getArgsInfo().startPos_given) _startPos=c2e.getArgsInfo().startPos_arg;
  if (c2e.getArgsInfo().endPos_given) _endPos=c2e.getArgsInfo().endPos_arg+1;
  if (_endPos > _sd.seqLen()) _endPos = _sd.seqLen();
	/**** setting treeFile */	
  tree* etp = c2e.cmdline2Tree();
  if (etp == NULL) 		errorMsg::reportError("failed to read tree file");
	_et= *etp;

	/**** setting outputFile */	
	_outFile = c2e.cmdline2OutputStream();

	/**** setting logFile */	
	c2e.initializeLogFile();
	// setNodeToSplitAt;
	//run();
}

  
checkCov::checkCov(int argc, char* argv[]){
  string seqFile, treeFile, modelName, logFile,outFile;
  int nodeNumberForSplit=-1;
  for (int ix=0; ix<argc; ix++) {
		char *pchar=argv[ix];
		switch (pchar[0]) {
		case '-':
			switch (pchar[1]) {
			case 'h':
				cout <<"USAGE:	"<<argv[0]<<" [-options] "<<endl <<endl;
				cout <<"+----------------------------------------------+"<<endl;
				cout <<"|-s    sequence file                           |"<<endl;
				cout <<"|-t    tree file                               |"<<endl;
				cout <<"|----------------------------------------------|"<<endl;
				cout <<"|-m    model:                                  |"<<endl;
				cout <<"|options are:                                  |"<<endl;
				cout <<"|day (dayhoff)                                 |"<<endl;
				cout <<"|jtt (JTT)                                     |"<<endl;
				cout <<"|rev (REV)                                     |"<<endl;
				cout <<"|wag (WAG)                                     |"<<endl;
				cout <<"|default is JTT                                |"<<endl;
				cout <<"|----------------------------------------------|"<<endl;
				cout <<"|-n    number of branch for covarion analysis  |"<<endl;
				cout <<"|----------------------------------------------|"<<endl;
				cout <<"|-o    output file (significant positions)     |"<<endl;
				cout <<"|-l    log file								   |"<<endl;
				cout <<"+----------------------------------------------+"<<endl;
			cout<<endl;	cerr<<" please press 0 to exit "; int d; cin>>d;exit (0);
			case 'l':
				logFile=argv[++ix];
				break;
			case 'm':
				logFile=argv[++ix];
				break;
			case 'n':
				nodeNumberForSplit=atoi(argv[++ix]);
				break;
			case 'o':
				outFile=argv[++ix];
				break;
			case 's':
				seqFile=argv[++ix];
				break;
			case 't':
				treeFile=argv[++ix];
				break;
			}
		}
	}
	setInputParameters(seqFile, treeFile, modelName, logFile,outFile,nodeNumberForSplit);
	for (int a=0; a<argc; a++) //printing arguments to log file
		LOG(3,<<"argv="<<argv[a]<<endl;);
	//	run();
	
//	gettingParametersFromInputFile(argc,argv);

}

checkCov::~checkCov() {
}

void checkCov::setInputParameters(string seqFile, string treeFile, string modelName,
								 string logFile, string outFile, int nodeNumberForSplit){
	/**** setting stochastic process */
	if (modelName=="")
		modelName="jtt"; //default
	replacementModel *probMod=NULL;
	pijAccelerator *pijAcc=NULL;
	if (modelName=="day")
		probMod=new pupAll(datMatrixHolder::dayhoff);
	else if (modelName=="jtt")
		probMod=new pupAll(datMatrixHolder::jones);
	else if (modelName=="wag")
		probMod=new pupAll(datMatrixHolder::wag);
	else if (modelName=="rev")
		probMod=new pupAll(datMatrixHolder::mtREV24);
	else
		errorMsg::reportError("This model is currently unimplemented. Please choose another model");
	pijAcc = new chebyshevAccelerator(probMod); 
	distribution *dist =  new uniDistribution;
	_sp = stochasticProcess(dist, pijAcc);
	if (dist) delete dist;
	if (probMod) delete probMod;
	if (pijAcc) delete pijAcc;

	/**** setting sequence container */	
	if (seqFile == "") {
		errorMsg::reportError("Please give a sequence file name in the command line");
	}
	amino alph;
	ifstream in(seqFile.c_str());
	_sd = recognizeFormat::read(in, &alph);
	in.close();
	_sd.changeGaps2MissingData();

	/**** setting treeFile */	
	if (treeFile == "") {
		errorMsg::reportError("Please give a tree file name in the command line");
	}
	_et= tree(treeFile);

	_et.rootToUnrootedTree();

	/**** setting outputFile */	
	if (outFile=="") outFile = "output";
	//	_outPutFile=outFile; //does not create an ofstream yet!

	/**** setting logFile */	
	if (logFile=="") logFile = "log";
	myLog::setLog(logFile, 5);
	
	/**** setting branch (split) for covarion analysis */	
	if (nodeNumberForSplit==-1) //user did not yet give an input node to split
		_nodeToSplitName=getUserNodeToSplitName();
	else
		_nodeToSplitName="N"+int2string(nodeNumberForSplit);
}

bool checkCov::setNodeToCut(const string newName)
{
  tree tmp1, tmp2;
  bool success=cutTreeToTwo(_et,newName,tmp1,tmp2);
  if (!success) return success;	// failed
  _small1=tmp1;
  _small2=tmp2;

  _nodeToSplitName=newName;		// accept the new node

	LOG(5,<<"tree1"<<endl;);
	LOGDO(5,_small1.output(myLog::LogFile()));
	LOG(5,<<"tree2"<<endl;);
	LOGDO(5,_small2.output(myLog::LogFile()));
	
	_small1.output(cout);
	_small2.output(cout);
	return true;
}

/**** run: (1) cut tree into 2 according to user-defined node, 
(2) positional LRT to test separate versus joint rate */	
bool checkCov::run(const string nodeName){
  if (!setNodeToCut(nodeName)) return (false); // failed to split the tree
  LOG(3,<<"splitting at "<<_nodeToSplitName<<endl); 
  cout <<endl<<"computing covarion likelihood ratio test around node "<<nodeName<<endl;
  Vdouble lrateSmall1(_sd.seqLen(),0.0);
  Vdouble lrateSmall2(_sd.seqLen(),0.0);
  Vdouble lrateJoint(_sd.seqLen(),0.0);
  //positional test:
  for (int pos =_startPos; pos < _endPos; ++pos) {
	covarLRTtestPos(_small1, _small2,pos,lrateSmall1[pos],lrateSmall2[pos],lrateJoint[pos]);
  }
  return true;
}


/**** If user doesn't input a node name, this is an interative funciton which prints the 
tree on the screen and requests the name of the node from the user */
string checkCov::getUserNodeToSplitName() {
	ofstream outT("labels.txt");
	printTreeWithHTUNamesBPStyle(outT);
	//_et.output(outT,tree::ANCESTOR,true);
	//outT.close();
	//_et.output(cout,tree::ANCESTOR,true);
	printTreeWithHTUNamesBPStyle(cout);
	cerr<<"[tree topology including internal nodes labels was also written to file labels.txt]";
	cerr<<"please enter the name of the node BELOW the branch that you would like to split"<<endl;
	string nodeName;
	cin>>nodeName;
	return nodeName;
}


void checkCov::printTreeWithHTUNamesBPStyle(ostream &out) const{
	recursivePrintTree(out,_et.getRoot());
	out<<";";
}

void checkCov::recursivePrintTree(ostream &out,const tree::nodeP &myNode) const {
	if (myNode->isLeaf()) {
		out << myNode->name();
		out << ":"<<myNode->dis2father();
		return;
	} else {
		out <<"(";
		for (int i=0;i<myNode->getNumberOfSons();++i) {
			if (i>0) out <<",";
			recursivePrintTree(out, myNode->getSon(i));
		}
		out <<")";
		if (myNode->isRoot()==false) {
			out<<":"<<myNode->dis2father(); 
			string val=myNode->name();
			out << "["<<val<<"]";
		}
	}
}

/**** Given a position, uses LRT to test whether two different rates or one equal rate
best describe the two subtrees */
void checkCov::covarLRTtestPos(const tree& small1,
							  const tree& small2,
							  const int pos,
							  MDOUBLE& resLsmall1,
							  MDOUBLE& resLsmall2,
							  MDOUBLE& resLsmallTogether){
  //	ofstream outFile(_outPutFile.c_str());
  //	(*_outFile)<<"Positions where likelihood of separate rates for subtrees is significant:"<<endl;
	MDOUBLE ax=0.0f,bx=5.0f,cx=20.0f,tol=0.0001f;
	MDOUBLE maxRboth=-1.0;
	MDOUBLE LmaxRboth =brent(ax,bx,cx,
		  Cevaluate_L_sum_given_r(_sp,_sd,small1,small2,pos),
		  tol,
		  &maxRboth);
	LmaxRboth = log(-LmaxRboth); //brent return the positive valvue of the likelihood
				  //here we put it to its correct negative form
 
	resLsmallTogether = LmaxRboth; // NEW GLOBAL

	//calculating the r of each
	MDOUBLE maxR1=-1.0;
	MDOUBLE LmaxR1 =brent(ax,bx,cx,
		Cevaluate_L_given_r(_sd,small1,_sp,pos),
		tol,
		&maxR1);

	MDOUBLE maxR2=-1.0;	
	MDOUBLE LmaxR2 =brent(ax,bx,cx,
		  Cevaluate_L_given_r(_sd,small2,_sp,pos),
		  tol,
		  &maxR2);

	resLsmall1 = log(-LmaxR1);// NEW GLOBAL
	resLsmall2 = log(-LmaxR2);// NEW GLOBAL

	MDOUBLE LsumOfTrees = resLsmall1+resLsmall2;

	Cevaluate_L_given_r ftl(_sd,_et,_sp,pos);
	MDOUBLE LfullMaxRboth = ftl(maxRboth);
	 LfullMaxRboth = log(-LfullMaxRboth);
	MDOUBLE chiSquareVal = 2.0*(LsumOfTrees-LmaxRboth);
	
	//	cout<<"POS: "<<pos<<"...";

//	LOG(3,<<"Node: "<<_nodeToSplitName<<" POS: "<<pos<<":"<<_alphabet->fromInt(_sd[0][pos])<<"...";);
	LOG(5,<<"Node: "<<_nodeToSplitName<<" POS: "<<pos<<" Likelihood of subTree1   = "<<resLsmall1<<endl;);
	LOG(5,<<"Node: "<<_nodeToSplitName<<" POS: "<<pos<<" Likelihood of subTree2   = "<<resLsmall2<<endl;);
	LOG(5,<<"Node: "<<_nodeToSplitName<<" POS: "<<pos<<" Likelihood split Tree    = "<<LsumOfTrees<<endl;);
	LOG(5,<<"Node: "<<_nodeToSplitName<<" POS: "<<pos<<" Likelihood with same rate= "<<LmaxRboth<<endl;);
	LOG(5,<<"Node: "<<_nodeToSplitName<<" POS: "<<pos<<" Likelihood of full tree  = "<<LfullMaxRboth<<"\t"<<LfullMaxRboth-LmaxRboth<<endl;);
	LOG(5,<<"Node: "<<_nodeToSplitName<<" POS: "<<pos<<" rate of subTree1= "<<maxR1<<endl;);
	LOG(5,<<"Node: "<<_nodeToSplitName<<" POS: "<<pos<<" rate of subTree2= "<<maxR2<<endl;);
	LOG(5,<<"Node: "<<_nodeToSplitName<<" POS: "<<pos<<" rate of joind   = "<<maxRboth<<endl;);
	LOG(5,<<"Node: "<<_nodeToSplitName<<" POS: "<<pos<<" chi Square Value= "<<chiSquareVal<<endl;);

	LOG(3,<<_nodeToSplitName <<" "<<pos<<" "<<resLsmall1<<" "<<resLsmall2<<" "<<LsumOfTrees<<" "<<LmaxRboth<<" "<<LfullMaxRboth<<" "<<chiSquareVal<<" "<<maxR1<<" "<<maxR2<<" "<<maxRboth<<" ");
	if (chiSquareVal > 10.828) {
		(*_outFile)<<"pos: "<<pos<<" is significant at P value 0.001 (****)"<<endl;
	} else if (chiSquareVal > 7.879) {
		(*_outFile)<<"pos: "<<pos<<" is significant at P value 0.005 (***)"<<endl;
	} else if (chiSquareVal > 6.635) {
		(*_outFile)<<"pos: "<<pos<<" is significant at P value 0.01 (**)"<<endl;
	} else if (chiSquareVal > 3.841) {
		(*_outFile)<<"pos: "<<pos<<" is significant at P value 0.05 (*)"<<endl;
	} 
	//else 
	//	  *_outFile<<endl;


	LOG(3,<<endl);
	//	(*_outFile).close();
}
