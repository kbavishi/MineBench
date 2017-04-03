/* Parallel Recursive ETI Miner
 * Coded by Blayne Field
 */

#include "SparseItemset.h"
#include <algorithm>

int roundNumToInt(double num)
{
	return (int) num + (num - (int) num >= 0.5 ? 1 : 0);
}

// some static variables
/*int SparseItemset::levelCount = 0;
int * SparseItemset::levelSizes = NULL;
int ** SparseItemset::levels = NULL;*/
DataSet * SparseItemset::dataSet = NULL;

// default constructor
SparseItemset::SparseItemset() {}

// constructor for use when creating a singleton itemset
// ***THIS MAY NOT ACTUALLY BE NECESSARY SINCE WE ARE PASSING THE SUPPORT TO***
// ***THE CONSTRUCTOR BELOW AS WELL***
SparseItemset::SparseItemset(int singleton, int support):
	itemCount(1), supportCounts(NULL), 
	leftOneOut(false), maxSingletonSupport(support), 
	areSubsetsWeakETI(true), areSubsetsStrongETI(true)
{
	theItems = new int[1];
	theItems[0] = singleton;
}

// constructor for a set of items
SparseItemset::SparseItemset(vector<int> items):
	itemCount(items.size()), supportCounts(NULL), 
	leftOneOut(false), maxSingletonSupport(1)
{
	sort(items.begin(), items.end());
	
	theItems = new int[itemCount];
	for (int i = 0; i < itemCount; i++) {
		theItems[i] = items[i];
	}

	if (itemCount == 1) {
		areSubsetsStrongETI = true;
		areSubsetsWeakETI = true;
	}	
}

// constructor for making an itemset with one item left out (useful for finding subsets)
SparseItemset::SparseItemset(SparseItemset *rhs, int index):
	supportCounts(NULL), leftOneOut(true)
{
	itemCount = 0;
	theItems = new int[rhs->itemCount - 1];
	for (int i = 0; i < rhs->itemCount; i++) {
		if (i != index)
			theItems[itemCount++] = rhs->theItems[i];
	}
}

// constructor for creating an itemset from two itemsets with matching prefixes
SparseItemset::SparseItemset(SparseItemset *lhs, SparseItemset *rhs):
	itemCount(lhs->itemCount+1), supportCounts(NULL),
	leftOneOut(false)
{	
	theItems = new int[itemCount];
	for (int i = 0; i < itemCount - 2; i++)
		theItems[i] = lhs->theItems[i];
	theItems[itemCount - 2] = min(lhs->theItems[itemCount - 2], rhs->theItems[itemCount - 2]);
	theItems[itemCount - 1] = max(lhs->theItems[itemCount - 2], rhs->theItems[itemCount - 2]);
	maxSingletonSupport = max(lhs->maxSingletonSupport, rhs->maxSingletonSupport);		
}
// end constructors 

// destructor
SparseItemset::~SparseItemset()
{
	if (leftOneOut)
		delete [] theItems;
	if (supportCounts)
		delete [] supportCounts;
}

		

// determines if the itemset is a recursive weak ETI at the given support, tolerance, and h-confidence
// TODO: CAN THIS RESULT BE CACHED IN SOME MANNER? (WAIT FOR PROFILING TO SEE IF THIS IS NECESSARY)
bool SparseItemset::IsRecursiveETI(int threshold, double eps, double hconf, EtiType type) 
{
	int support = (type == WEAK_ETI ? WeakETISupport(eps) : StrongETISupport(eps));
	bool isETI = support >= threshold;
	bool isHclique = ((double) support / ((1-eps) * maxSingletonSupport)) >= hconf;
	bool isRecursive = (type == WEAK_ETI ? areSubsetsWeakETI : areSubsetsStrongETI);
	
	if (type == WEAK_ETI) {
		isRecursiveWeakETI = (isETI && isRecursive && isHclique);
	} else {
		isRecursiveStrongETI = (isETI && isRecursive && isHclique);
	}

	return (type == WEAK_ETI ? isRecursiveWeakETI : isRecursiveStrongETI);
}

// determine if the two itemsets share a prefix (all but the last item)
bool SparseItemset::SharesPrefixWith(SparseItemset *rhs) const
{
	for (int i = 0; i < itemCount - 1; i++) 
		if (theItems[i] != rhs->theItems[i]) 
			return false;
	return true;
}

// determine if all the subsets of the itemset are contained within the given set of itemsets
void SparseItemset::DetermineRecursiveProperties(vector<SparseItemset *> candidates) 
{
	areSubsetsWeakETI = true;
	areSubsetsStrongETI = true;

	// leave out each item (one at a time) in order to find the subset
	for (int leaveOut = 0; leaveOut < itemCount && (areSubsetsWeakETI || areSubsetsStrongETI); leaveOut++) {

		// create the subset that we would like to find
		SparseItemset *leftOut = new SparseItemset(this, leaveOut);
		bool found = false;
		
		// do a binary search to attempt to find the desired subset
		int left = 0;
		int right = candidates.size() - 1;
		int mid;
		while (left <= right && !found) {
			mid = (right + left) / 2;

			if (*(candidates[mid]) < *leftOut) {
				left = mid + 1;
			} else if (*(candidates[mid]) == *leftOut) {
				found = true;
				areSubsetsWeakETI &= candidates[mid]->areSubsetsWeakETI;
				areSubsetsStrongETI &= candidates[mid]->areSubsetsStrongETI;
			}	else {
				right = mid - 1;
			}
		}
		
		areSubsetsWeakETI &= found;
		areSubsetsStrongETI &= found;
	}
}

bool SparseItemset::DoesRecursivePropertyHold(EtiType type) const
{
	return (type == WEAK_ETI ? areSubsetsWeakETI : areSubsetsStrongETI);
}

bool SparseItemset::IsStrongETI(int threshold, double eps) const 
{
	return (StrongETISupport(eps) >= threshold);
}

// clears the support count if it exists, and if it doesn't, it creates the blank support count
void SparseItemset::ClearCounter(int transactions)
{
	if (!supportCounts)
		supportCounts = new int[itemCount + 1];
	
	memset(supportCounts, 0, sizeof(int) * (itemCount+1));
	supportCounts[0] = transactions;
}

// updates the support counts based on the given transaction
bool SparseItemset::IncrementSupportCounts(SparseItemset *transaction)
{
	// this just determines whether it is possible at 
	// all for there to be any overlap
	if (theItems[0] > transaction->theItems[transaction->itemCount-1]) {
		//supportCounts[0]++;
		return false;
	}

	if (theItems[itemCount-1] < transaction->theItems[0]) {
		//supportCounts[0]++;
		return true;
	}

	// would binary search work on larger transactions?
	// profiling seems to indicate it would not (well, for 50 items)
	// try this on the document data
	int overlap2 = 0;
	int transIndex = 0;
	for (int i = 0; i < itemCount; i++) {
		while (transaction->theItems[transIndex] < theItems[i] && transIndex < transaction->itemCount) {
			transIndex++;
		}
		if (transIndex >= transaction->itemCount)
			break;
		if (transaction->theItems[transIndex] == theItems[i]) {
			overlap2++;
			continue;
		}
	}
	
	if (overlap2 > 0) {
		supportCounts[overlap2]++;
		supportCounts[0]--;
	}
	return true;
}

void SparseItemset::SetDataSet(DataSet *ds) {SparseItemset::dataSet = ds;}

ostream &operator<<(ostream &out, const SparseItemset &itemset)
{
	for (int i = 0; i < itemset.itemCount; i++) {
		out << (1+SparseItemset::dataSet->itemMap[itemset.theItems[i]]) << " ";
	}
	out << endl;

	return out;
}

bool SparseItemset::operator==(const SparseItemset &rhs) const
{
	for (int i = 0; i <= itemCount / 2; i++) {
		if (theItems[i] != rhs.theItems[i])
			return false;
		if (theItems[itemCount - i - 1] != rhs.theItems[itemCount - i - 1])			
			return false;
	}
	return true;
}

bool SparseItemset::operator<(const SparseItemset &rhs) const
{
	for (int i = 0; i < itemCount; i++) {
		if (theItems[i] < rhs.theItems[i])
			return true;
		else if (theItems[i] > rhs.theItems[i])
			return false;
	}
	return false;
}
		

int SparseItemset::WeakETISupport(double eps) const 
{
  int transactions = 0;
  int onesFound = 0;

  double densityThreshold = 1 - eps;
  double density = 1;

  // greedily add transactions while it is still a weak ETI
  int i = itemCount;  

  // verify the things are being computed correctly here

  while (density >= densityThreshold && i >= 0) {
    
    double currentDensity = (double) i / itemCount;

    // if we are above the theshold, we can add transactions
    // with impunity (it's impossible to fall below the threshold)
    if (currentDensity >= densityThreshold) {
      transactions += supportCounts[i];
      onesFound += (i * supportCounts[i]);
      density = (double) onesFound / (transactions * itemCount);
    }
    // otherwise we have to determine how much we can add of 
    // this size in order to keep the density above the threshold
    else {
      int maxTransactions = (int)
				roundNumToInt((((density - densityThreshold) * transactions) /
							 				(densityThreshold - currentDensity)));

      int amountAvailable = min(supportCounts[i], maxTransactions);
      transactions += amountAvailable;
      onesFound += (i * amountAvailable);
      density = (double) onesFound / (transactions * itemCount);
    }   
      
    i--;
  }
	
	return transactions;
}

int SparseItemset::StrongETISupport(double eps) const
{
  int minItems = (int) ceil(((1-eps) * itemCount)); 
  int support = 0;

  for (int i = itemCount; i >= minItems; i--) {
    support += supportCounts[i];
  }

  return support;
}


