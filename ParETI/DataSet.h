/* Parallel Recursive ETI Miner
 * Coded by Blayne Field
 */

#ifndef DATASET_H
#define DATASET_H

class SparseItemset;

#include <stdlib.h>
#include <string.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>

#include "SparseItemset.h"

#define USE_MPI
#ifdef USE_MPI
#include "mpi.h"
#endif

using namespace std;

struct Item
{
	int freqIdNum;
	int support;
};

typedef map< string, Item *, less< string > > ItemMap;

class DataSet
{
 public:
#ifdef USE_MPI
	DataSet(string filename, int threshold, int processes, int processID);
#else
	DataSet(string filename, int threshold);
#endif
	
	void ComputeSupport(vector<SparseItemset *> candidateSparseItemsets) const;
	vector<SparseItemset *> GetSingletonItemsets() const;
	
	// get information about the size of the dataset
  int GetItemCount() const;

	vector<int> itemMap;
	
 private:
  vector<SparseItemset *> transactions;
	vector<SparseItemset *> singletons;
	string *stringMap;	
	
	// MPI information
	bool useMPI;
	int processCount;
	int processID;
	
  int transactionCount;
  int itemCount;
};

#endif

