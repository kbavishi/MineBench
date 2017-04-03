// 	$Id: bootstrapProvider.h 2399 2014-03-13 22:43:51Z wkliao $	

#ifndef ___BOOTSTRAP_PROVIDER
#define ___BOOTSTRAP_PROVIDER

#include "bootstrap.h"
#include "definitions.h"
#include "mainSemphy.h"
#include "semphy_cmdline.h"
#include "stochasticProcess.h"
#include "tree.h"
#include <vector>
#include <map>
using namespace std;

class bootstrapProvider {
public:
	explicit bootstrapProvider(const semphy_args_info& gn);
	
	// this function creates the many tree.
	void computeBP(mainSemphy & ms);
	
	void computeConsensus(const MDOUBLE treshold);
	void computeConsensus();
	void output(ostream & out) const;


private:
	void createRandomWeights(const int seqLen);
	void computeTreeSupport(const tree& et);

	semphy_args_info _args_info;
	vector<tree> _treeVec;
	vector<MDOUBLE>	_likelihoodVec;
	vector<stochasticProcess> _stochasticProcessVec;
	vector<MDOUBLE> _weights;
	tree _consensusTree;
	bootstrap* _bp;

	tree _inputTree; // the tree on which support is computed.
	map<int, MDOUBLE> _treeSupport;
};

#endif


