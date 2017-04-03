/* Approximate Frequent Itemset Miner
 * Coded by Blayne Field
 * Algorithm developed by J. Liu, S. Paulsen, et. al
*/

#include "AFIMiner.h"

int main(int argc, char **argv)
{
  // make sure parameters are specified to the program
  if (argc == 1) {
    cerr << "usage: afi dataset minsupport epsCol epsRow" << endl;
    exit(1);
  }    
  
	DataSet data(argv[1]);
  AFIMiner miner(&data, atof(argv[3]), atof(argv[4]), atoi(argv[2]));
  miner.MineAFI();

  return 0;
}

AFIMiner::AFIMiner() {}

AFIMiner::AFIMiner(DataSet *data, double epsc, double epsr, int minsup):
  dataSet(data)
{
  if (epsc >= 0 && epsc <= 1.0)
    epsCol = epsc;
  else {
    epsCol = 0;
    cerr << "epsCol should be in the range [0..1]" << endl;
  }

  if (epsr >= 0 && epsr <= 1.0)
    epsRow = epsr;
  else {
    epsRow = 0;
    cerr << "epsRow should be in the range [0..1]" << endl;
  }

  minSup = minsup;  
}

/* search for AFIs within the data
 */
void AFIMiner::MineAFI()
{  
  ComputePruningSupport();
  InitializeSupportSet();

  // iterate through the possible item set sizes, 
  // or until no more sets can be merged to form AFI's
  int numberOfItems = dataSet->GetItemCount();
  for (int itemSetSize = 2; itemSetSize <= numberOfItems; itemSetSize++) {
   
    int numberOfSets = afiCandidates[itemSetSize - 2].size();

    //    cerr << "number of sets of size " << (itemSetSize-1) 
    //    	 << ": " << numberOfSets << endl;

    // attempt to merge all combinations of the next smallest size
    if (numberOfSets > 1) {      
      MergeSets(afiCandidates[itemSetSize - 2], itemSetSize);
    } 
    else {
      break;
    }
    
    // these sets are no longer needed
    afiCandidates[itemSetSize - 2].clear();
  }
}

/* this function takes all the item sets of the given size,
 * finds which ones can be merged to form sets of one item larger
 * finds the support sets for these new sets, and adds them to the
 * list of partial AFIs
 */
void AFIMiner::MergeSets(vector< ItemSetTreeNode *> prevLevel, int size)
{
  // the partial AFIs of this larger length
  vector < ItemSetTreeNode * > thisLevel;

  //  cerr << "size is: " << size << endl;
  int numberOfSets = prevLevel.size();

  // go through each combination of sets 
  for (int i = 0; i < numberOfSets; i++) {
    for (int j = i+1; j < numberOfSets; j++) {
      //if ((++count % threshold) == 0)
      //cerr << "*";

      // determine whether these sets should be merged,
      // and do so if they should
      int *mergedIndices = NULL;
      if (prevLevel[i]->meetsThreshold && prevLevel[j]->meetsThreshold) {
				mergedIndices = ExtendTwoSets(prevLevel[i]->items, prevLevel[j]->items, size - 1);
      }

      // if the sets were merged, find the resulting support set
      if (mergedIndices) {
  			int **supportSets = new int * [size];
			int *supportCounts = new int[size];
			bool subsetsFrequent = true;

				// leave out each of the items in order to get the
				// subsets of the new set	
				for (int skip = 0; skip < size; skip++) {
					
					// get the supporting transactions for the current subset
					ItemSetTreeNode *nodePtr = setTree->GetNode(mergedIndices, 
												size, skip);
					if (nodePtr == NULL) {
						subsetsFrequent = false;
						break;
					}	      

					supportSets[skip] = nodePtr->supportingTransactions;
					supportCounts[skip] = nodePtr->supportCount;

					// if the subset wasn't frequent enough, this set can not
					// be frequent enough
					if (!nodePtr->meetsThreshold) {
						subsetsFrequent = false;
						break;
					}
			}

			// if all of the subsets were frequent enough
			// find the supporting transactions
			if (subsetsFrequent) {
				// cout << "subsets frequent" << endl;
				int *supportSet;
				int supportCount = 0;
	
				// perform either a 1-extension or 0-extension
				if (floor((size-1) * epsRow) == floor((size) * epsRow)) {
					// intersection (1-extension)
					supportSet = Intersection(supportSets, supportCounts,
				      size, &supportCount);	  
				} 
				else {
					// union (0-extension)
					supportSet = Union(supportSets, supportCounts,
			       size, &supportCount);	  
				}

				// determine if this node should be pruned
	  		bool meetsThreshold = (supportCount >= pruningSupport[size]);

			  // if the set is an AFI, print it to the screen,
	  		// along with the support set	
			  if (meetsThreshold && IsAFI(mergedIndices, size, supportSet, supportCount)) {
			    //	    cout << "---------------AFI (" 
	  		  //	 << supportCount << ")-------------" << endl;
	    	   // print the items
			    for (int item = 0; item < size; item++)
	  		    cout << setw(5) << dataSet->GetActualIndex(mergedIndices[item]);
					//cout << endl;
	
					int exactSupport = ExactSupport(mergedIndices, size);
					cout << " (";
					for (int i = 10; i <= 100; i += 10) {
						if (exactSupport > (int) (0.01 * i * pruningSupport[0])) {
							cout << "a" << i << " ";
						}
					}					
			    cout << ")" << endl;
				} 
		
				// add this set to the support set tree, and to the sets of this size
	  		thisLevel.push_back(setTree->CreateNode(mergedIndices, size,
						  supportSet, supportCount, 
						  meetsThreshold));	}
      }
    }
  }

  // add this level to the candidates list
  afiCandidates.push_back(thisLevel);  
}

/* sets up the Item Set Support Tree
 * by finding the support for each item, and adding 
 * sufficiently frequent ones to the tree
 */
void AFIMiner::InitializeSupportSet()
{
  //nt transactions = dataSet->GetTransactionCount();
  int items = dataSet->GetItemCount();
  int length;

  setTree = new ItemSetTree(items);
  vector< ItemSetTreeNode *> levelOne;
  
  for (int i = 0; i < items; i++) {
    int *suppSet = dataSet->GetSupportSet(i, &length);
    int itemSet[1] = {i};
    //  int requiredLength = (int) (pruningSupport[1] * transactions);

    if (length >= pruningSupport[1]) {
      ItemSetTreeNode *tempNode = setTree->CreateNode(itemSet, 1,
						      suppSet, length, true);
      levelOne.push_back(tempNode);
    }
  }
  afiCandidates.push_back(levelOne);
}

/* determines whether the given set is an AFI
 *
 * Parameters:
 * itemSet - the item indices of the candidate set
 * itemCount - the number of items in the set
 * transSet - the set of supporting transactions
 * transCount - the number of supporting transactions
 *
 * Returns:
 * true if the set is an AFI, false if not
 */
bool AFIMiner::IsAFI(int *itemSet, int itemCount, int *transSet, int tCount)
{
  // determine the thresholds that must be met
  int overallThreshold = pruningSupport[0];
  int subsetThreshold = (int) (epsCol * tCount);	// * 1


  // if the support is too low, reject it
  if (tCount < overallThreshold)
    return false;

  // if any column (item) has too many zeros, reject the set
  for (int i = 0; i < itemCount; i++) {  
    int misses = 0;
    for (int t = 0; t < tCount; t++) {
      if (!dataSet->ValueAt(transSet[t], itemSet[i]))
	misses++;
      if (misses > subsetThreshold)
	return false;
    }
  }

  return true;
}

/* computes the pruning support threshold for each
 * itemset length (below which itemsets should be pruned)
 */
void AFIMiner::ComputePruningSupport()
{
  int maxLength = dataSet->GetItemCount();
  int trans = dataSet->GetTransactionCount();
  pruningSupport = new int[maxLength];
  pruningSupport[0] = minSup;//(int) ceil(minSup * trans);

  for (int length = 1; length < maxLength; length++) {
    double factor =
      trans - (trans * length * epsCol) / (floor(length * epsRow) + 1);    
    pruningSupport[length] =       
      (int) ceil((double) minSup / trans * (factor > 0 ? factor : 0)); 
    //cout << "support for " << length << "-itemsets: "
    //<< pruningSupport[length] << endl;
  }
}

/* attempts to create a itemset from two existing itemsets by
 * seeing if their first length - 1 items match
 *
 * Parameters:
 * set1 - the first set to use (assumed to be sorted, ascending)
 * set2 - the second set to use (assumed to be sorted, ascending)
 * length - the length of the two itemsets (they should be the same)
 *
 * Returns:
 * a pointer to an extended itemset, if they two sets match,
 * otherwise NULL is returned
 */
int * AFIMiner::ExtendTwoSets(int *set1, int *set2, int length)
{
  // determine whether the first (length-1) items 
  // match in the two sets
  bool match = true;
  for (int i = 0; i < length - 1; i++) {
    if (set1[i] != set2[i]) {
      match = false;
      break;
    }
  }

  // if they match, create the extended item set (also sorted)
  if (match) {

    // copy the common items
     int *set3 = new int[length+1];
    for (int i = 0; i < length - 1; i++)
      set3[i] = set1[i];    
    
    // add the last items in lexicographic order
    if (set1[length - 1] < set2[length - 1]) {
      set3[length - 1] = set1[length - 1];
      set3[length] = set2[length - 1];
    }
    else {
      set3[length - 1] = set2[length - 1];
      set3[length] = set1[length - 1];
    }

    return set3;
  }
  else {
    return NULL;
  }  
}

/* Finds the union of the given support sets
 * 
 * Parameters:
 * supportSets - the support sets to be combined
 *               assumes each set has unique, sorted (ascending) integers
 * supportCounts - an array containing the number of items for each
 *                 of the above sets
 * length - the number of supporting sets
 * supportCount - a pointer to the location to store the number of
 *                elements in the union
 *
 * Returns:
 * an array of items that are in at least one of the sets
 */
int * AFIMiner::Union(int **supportSets, int *supportCounts,
		      int length, int *supportCount)
{
  // keep track of where we are at in each set
  int *indices = new int[length];
  for (int i = 0; i < length; i++) {
    indices[i] = 0;
  }

  // keep track of all unique items
  vector<int> supportSet;

  // loop through the sets, finding new items,
  // until each set's end has been reached
  bool morePossibilities = true;
  while (morePossibilities) {

    // find the smallest valued supporting transaction
    int min = INT_MAX;
    for (int i = 0; i < length; i++) {
      if (indices[i] < supportCounts[i]) {
	int currentValue = supportSets[i][indices[i]];
	if (currentValue < min)
	  min = currentValue;
      }
    }

    // find all itemsets that are supported by the current transaction
    // and move one step along in those sets
    for (int i = 0; i < length; i++) {
      if (indices[i] < supportCounts[i]
	  && min == supportSets[i][indices[i]]) {
	indices[i]++;
      }
    }

    // add the item to the union
    supportSet.push_back(min);
    
    // determine whether there is anything left to find
    morePossibilities = false;
    for (int i = 0; i < length; i++) {
      if (indices[i] < supportCounts[i]) {
	morePossibilities = true;
	break;
      }
    }
  }
  
  // copy the vector into an array, and return the result
  // (and the count, through use of a int pointer)
  int size = supportSet.size();
  if (size > 0) {
    int *sset = new int[size];
    for (int i = 0; i < size; i++) {
      sset[i] = supportSet[i];
    }
    *supportCount = size;
    return sset;
  }
  else {
    *supportCount = 0;
    return NULL;
  }
}

int AFIMiner::ExactSupport(int *items, int size) 
{
	int trans = dataSet->GetTransactionCount();
	int support = trans;
	
	for (int t = 0; t < trans; t++) {		
		for (int i = 0; i < size; i++) {
			if (!dataSet->ValueAt(t, items[i])) {
				support--;
				break;
			}
		}
	}

	return support;
}
	


/* Finds the intersection of the given support sets
 * 
 * Parameters:
 * supportSets - the support sets to be intersected
 *               assumes each set has unique, sorted (ascending) integers
 * supportCounts - an array containing the number of items for each
 *                 of the above sets
 * length - the number of supporting sets
 * supportCount - a pointer to the location to store the number of
 *                elements in the intersection
 *
 * Returns:
 * an array of items that are in all of the sets
 *
 * Improvements:
 * - search from the smallest set (that is guarenteed to be the max number 
 *   in the intersection)
 */
int * AFIMiner::Intersection(int **supportSets, int *supportCounts,
			     int length, int *supportCount)
{
  // keep track of what elements have been found in each set
  int *intersection = new int[supportCounts[0]];
  int matches = 0;

  // keep track of where we are in each set
  int *indices = new int[length];
  for (int i = 0; i < length; i++) {
    indices[i] = 0;
  }

  // for each item in set 1, attempt to find it in each of the other sets
  for (indices[0] = 0; indices[0] < supportCounts[0]; indices[0]++) {
    
    // the number to be found in each other set
    int searchingFor = supportSets[0][indices[0]];

    // assume it was found
    bool foundInEachSet = true;

    // for each of the remaining sets, search for the item
    for (int i = 1; i < length; i++) {

      // march through the set until the end is hit, 
      // or the value in the set exceeds what we are looking for
      while (indices[i] < supportCounts[i]
	     && supportSets[i][indices[i]] < searchingFor) {
	indices[i]++;
      }

      // if it wasn't in the set, the number can't be in the intersection
      if (supportSets[i][indices[i]] != searchingFor) {
	foundInEachSet = false;
	break;
      }
    }
    
    // if it was in each set, add the number to the intersection
    if (foundInEachSet) {
      intersection[matches] = searchingFor;
      matches++;
    }
  }

  // indicate what was found, and how many were found
  *supportCount = matches;
  return intersection;
}


