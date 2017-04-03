/* Parallel Recursive ETI Miner
 * Coded by Blayne Field
 */
 
#ifndef ETI_MINER_H
#define ETI_MINER_H

#include <fstream>
#include <iostream>
#include <iomanip>
#include <cmath>
#include <vector>

#include "DataSet.h"
#include "SparseItemset.h"

using namespace std;

class ETIMiner
{
 public:
  ETIMiner();
  ETIMiner(DataSet *data, double epsc, double epsr, int minsup, double hConf, int rank);
  void HorizontalMine();
  
 private:
  int itemCount;
//  int transactionCount;
  DataSet *dataSet;
  int minimumSupport;
  double eps;
	double epsRow;
  double hConfidence;
	int rank;
  bool findWeak, findRecursiveWeak, findStrong, findRecursiveStrong;

	vector<SparseItemset *> CreateInitialCandidates();
	vector<SparseItemset *> GenerateCandidates(vector<SparseItemset *> previousLevel);
};

#endif
