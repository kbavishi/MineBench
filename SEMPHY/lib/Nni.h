// $Id: Nni.h 2399 2014-03-13 22:43:51Z wkliao $

#ifndef ___NNI
#define ___NNI

#include "definitions.h"
#include "tree.h"
#include "sequenceContainer.h"
#include "stochasticProcess.h"
#include <vector>
using namespace std;

class NNI {
public:
	explicit NNI(const sequenceContainer& sc,
				   const stochasticProcess& sp,
				const Vdouble * weights);

	tree NNIstep(tree et);
	MDOUBLE bestScore(){ return _bestScore;} 

private:
	tree _bestTree;
	MDOUBLE _bestScore;
	const sequenceContainer& _sc;
	const stochasticProcess& _sp;
	const Vdouble * _weights;
	MDOUBLE evalTree(tree& et,const sequenceContainer& sd);
	tree NNIswap1(tree et,tree::nodeP mynode);
	tree NNIswap2(tree et,tree::nodeP mynode);
};
#endif
