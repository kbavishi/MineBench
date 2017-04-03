// $Id: semphySearchBestTree.h 2399 2014-03-13 22:43:51Z wkliao $

#ifndef ___SEMPHY_SEARCH_BEST_TREE
#define ___SEMPHY_SEARCH_BEST_TREE

#include "alphabet.h"
#include "sequenceContainer.h"
#include "tree.h"
#include "stochasticProcess.h"

#include <iostream>

using namespace std;


class semphySearchBestTree {
public:
	explicit semphySearchBestTree(sequenceContainer& sc,
								  tree& startTree,
								  const tree* ctPtr,
								  stochasticProcess& sp,
								  ostream& out,
								  const int numOfRandomStart = 1,
								  const bool optimizeAlpha = false,
								  const Vdouble * weights = NULL);
	virtual ~semphySearchBestTree(){}


private:
	MDOUBLE semphyBasicSearchBestTree(
								  sequenceContainer& sc,
								  tree& et,
								  const tree* ctPtr,
								  stochasticProcess& sp,
								  const bool optimizeAlpha = false,
								  const Vdouble * weights = NULL);
	MDOUBLE semphyBasicSearchBestTree(
								  sequenceContainer& sc,
								  tree& startTree,
								  const tree* ctPtr,
								  stochasticProcess& sp,
								  ostream& out,
								  const bool optimizeAlpha = false,
								  const Vdouble * weights = NULL);

	MDOUBLE semphyBasicSearchBestTreeManyRandomStarts(
								  sequenceContainer& sc,
								  tree& et,
								  const tree* ctPtr, 
								  stochasticProcess& sp,
								  ostream& out,
								  const int nRanStart,
								  const bool optimizeAlpha = false,
								  const Vdouble * weights = NULL);

};

#endif
