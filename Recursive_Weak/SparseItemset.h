/* Parallel Recursive ETI Miner
 * Coded by Blayne Field
 */

#ifndef SPARSE_ITEMSET_H
#define SPARSE_ITEMSET_H

class DataSet;

#include <iostream>
#include <cmath>
#include "DataSet.h"

using namespace std;

enum EtiType {WEAK_ETI, STRONG_ETI};

class SparseItemset 
{
	public:
		SparseItemset();
		SparseItemset(int singleton, int support);
		SparseItemset(vector<int> items);
		SparseItemset(SparseItemset *rhs, int index);
		SparseItemset(SparseItemset *lhs, SparseItemset *rhs);
		~SparseItemset();
		
		// useful functions for determining if the itemset is interesting	
		bool IsRecursiveETI(int threshold, double eps, double hconf, EtiType type); 
		bool DoesRecursivePropertyHold(EtiType type) const; 
		bool IsStrongETI(int threshold, double eps) const;

		// useful for candidate generation
		bool SharesPrefixWith(SparseItemset *rhs) const;
		void DetermineRecursiveProperties(vector<SparseItemset *> candidates);

		// useful for computing the support and recursive properties
		void ClearCounter(int transactions);
		bool IncrementSupportCounts(SparseItemset *transaction);
		
		static void SetDataSet(DataSet *data);		
		friend ostream &operator<<(ostream &out, const SparseItemset &);
	
		// overloaded operators (useful for doing a binary search)
		bool operator==(const SparseItemset &rhs) const;
		bool operator<(const SparseItemset &rhs) const;
		
		int itemCount;
		int *supportCounts;
	private:

		// the most interesting part (the array of items)
		int *theItems;
		bool leftOneOut;		
		// support information		
		int weakETISupport;
		int strongETISupport;
		int maxSingletonSupport;
		bool isRecursiveWeakETI;
		bool isRecursiveStrongETI;

		// ETI information
		//bool isWeakETI;
		//bool isStrongETI;
		
		// recursion information
		bool areSubsetsWeakETI;
		bool areSubsetsStrongETI;
		
		static DataSet *dataSet;

		int * GetSpaceForItems();
		int WeakETISupport(double eps) const;
		int StrongETISupport(double eps) const;
		void Print(DataSet *data) const;
};



#endif

