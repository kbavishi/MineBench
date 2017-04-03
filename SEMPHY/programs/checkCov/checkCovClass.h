// 	$Id: checkCovClass.h 2399 2014-03-13 22:43:51Z wkliao $	
#ifndef ___CHECKCOV
#define ___CHECKCOV

#include "definitions.h"
#include "alphabet.h"
#include "sequenceContainer.h"
//#include "treeInterface.h"
#include "treeUtil.h"
#include "stochasticProcess.h"
#include "cmdline2EvolObjs.h"
#include "checkCov_cmdline.h"
//using namespace treeInterface;

class checkCov {
public:
	explicit checkCov(int argc, char* argv[]);
	explicit checkCov(string seqFile, string treeFile, string modelName,string logFile,
					 string outFile, int nodeNumberForSplit);
    explicit checkCov(cmdline2EvolObjs<gengetopt_args_info> &c2e, const int nodeNumberForSplit);

	virtual ~checkCov();
	bool run(const string nodeName);
	void printTreeWithHTUNamesBPStyle(ostream &out) const;
  const tree& getTree() const {return _et;}
	string getUserNodeToSplitName();

private:
	void setInputParameters(string seqFile, string treeFile, string modelName,string logFile,
		string outFile, int nodeNumberForSplit);
	void gettingParametersFromInputFile(int argc, char* argv[]);
    bool setNodeToCut(const string newName);
	
	void covarLRTtestPos(const tree& small1,const tree& small2,const int pos,
							  MDOUBLE& resLsmall1,
							  MDOUBLE& resLsmall2,
							  MDOUBLE& resLsmallTogether);
	void recursivePrintTree(ostream &out,const tree::nodeP &myNode) const;
private:
	sequenceContainer _sd;
	tree _et;
	tree _small1, _small2;		// cut up trees
	stochasticProcess _sp;
	//ostream *_out;
  //	string _outPutFile;
  ostream* _outFile;
	string _nodeToSplitName;
  int _startPos, _endPos;
};

#endif
