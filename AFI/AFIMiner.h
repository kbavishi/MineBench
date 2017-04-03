/* Approximate Frequent Itemset Miner
 * Coded by Blayne Field
 * Algorithm developed by J. Liu, S. Paulsen, et. al
 * find the proper citation here
 */

#ifndef AFI_MINER_H
#define AFI_MINER_H

#include <iostream>
#include <iomanip>
#include <cmath>
#include <vector>
#include <stdlib.h>
#include <limits.h>
#include <string.h>

#include "DataSet.h"
#include "ItemSetTree.h"

using namespace std;

class AFIMiner
{
 public:
  AFIMiner();
  AFIMiner(DataSet *data, double epsc, double epsr, int minsup);

  void MineAFI();

 private:
  DataSet *dataSet;
  ItemSetTree *setTree;
  double epsCol;
  double epsRow;
  int minSup;
  int *pruningSupport;

  bool IsAFI(int *, int, int *, int);
  void ComputePruningSupport();  
  void InitializeSupportSet();
	int ExactSupport(int *, int);
  void MergeSets(vector< ItemSetTreeNode * > prevLevel, int size);
  int * ExtendTwoSets(int *s1, int *s2, int len);
  int * Union(int **supportSets, int *supportCounts, int length,
	      int *supportCount);
  int * Intersection(int **supportSets, int *supportCounts, int length,
		     int *supportCount);


  vector< vector< ItemSetTreeNode * > > afiCandidates;
};

#endif
