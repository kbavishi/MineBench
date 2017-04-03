/* Parallel Recursive ETI Miner
 * Coded by Blayne Field
 */

#include <stdlib.h>
#include "ETIMiner.h"
#include "SparseItemset.h"

int main(int argc, char **argv)
{
/*	if (argc != 5 && argc != 6) {
		cerr << "usage: eti dataset support epsilon hconf" << endl;
		exit(1);
	}
*/
	int threshold = (int) ((1-atof(argv[3])) * atoi(argv[2]));
	int rank = 0;
	
#ifdef USE_MPI
	int size;
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	DataSet data(argv[1], threshold, size, rank);
	SparseItemset::SetDataSet(&data);
#else	
	DataSet data(argv[1], threshold);
	SparseItemset::SetDataSet(&data);
#endif
	
	ETIMiner miner(&data, atof(argv[3]), atof(argv[3]), atoi(argv[2]), 0.0, rank);		
	miner.HorizontalMine();
  
#ifdef USE_MPI
	MPI_Finalize();
#endif
	
	return 0;
}

ETIMiner::ETIMiner() {}

ETIMiner::ETIMiner(DataSet *data, double pEps, double pepsr, int minsup, double hConf, int pRank):
  itemCount(data->GetItemCount()), dataSet(data), minimumSupport(minsup), eps(pEps), epsRow(pepsr),
	hConfidence(hConf),	rank(pRank), findRecursiveWeak(true), findRecursiveStrong(false) {}

vector<SparseItemset *> ETIMiner::GenerateCandidates(vector<SparseItemset *> previousLevel)
{
	vector<SparseItemset *> candidates;
	int previousCount = previousLevel.size();
	
	// compare each pair of itemsets from the previous level to
	// create the next level of itemsets
	for (int i = 0; i < previousCount - 1; i++) {
		bool keepGoing = true;
		
		for (int j = i+1; j < previousCount && keepGoing; j++) {

			if (previousLevel[i]->SharesPrefixWith(previousLevel[j])) {
				SparseItemset *combinedSparseItemset = new SparseItemset(previousLevel[i], previousLevel[j]);

				// determine whether the subsets have the properties that we desire
				combinedSparseItemset->DetermineRecursiveProperties(previousLevel);
				bool valid = false;
				valid |= (findRecursiveStrong && combinedSparseItemset->DoesRecursivePropertyHold(STRONG_ETI));
				valid |= (findRecursiveWeak && combinedSparseItemset->DoesRecursivePropertyHold(WEAK_ETI));
			
				if (valid) {
					candidates.push_back(combinedSparseItemset);
				}
			}
			else {
				keepGoing = false;
			}
		}
	}

	return candidates;
}

void ETIMiner::HorizontalMine()
{
	// create the initial set of itemsets (just the singletons)
	vector<SparseItemset *> previousLevel = dataSet->GetSingletonItemsets();
	int candidateCount = previousLevel.size();
	
	// for each level that there exists candidates, 
	// determine which candidates are frequent itemsets
	// and generate the candidates for the next level 
	int level = 1;
	while (candidateCount > 0) {

		// print the itemsets
		cerr << "level " << level << ":" << endl;
		for (int i = 0; i < candidateCount; i++) {
			if (level > 1 && rank == 0 /* && previousLevel[i]->IsStrongETI(minimumSupport, epsRow)*/)
				cout << *(previousLevel[i]);
		}
		cerr << "computing level " << ++level << " candidates" << endl;

		// generate the next level of the lattice
		vector<SparseItemset *> candidateSparseItemsets = GenerateCandidates(previousLevel);

		// determine which of the candidate itemsets are ETIs
		// by first determining the support for each of the itemsets
		dataSet->ComputeSupport(candidateSparseItemsets);
		previousLevel.clear();

		// for each itemset, determine if it is interesting
		for (int i = 0; i < (int) candidateSparseItemsets.size(); i++) {
			bool validCandidate = false;
			validCandidate |= (findRecursiveWeak && 
												 candidateSparseItemsets[i]->IsRecursiveETI(minimumSupport, eps, hConfidence, WEAK_ETI));
			validCandidate |= (findRecursiveStrong && 
												 candidateSparseItemsets[i]->IsRecursiveETI(minimumSupport, epsRow, hConfidence, STRONG_ETI));
			if (validCandidate) {
				previousLevel.push_back(candidateSparseItemsets[i]);		
			}
		}
			
		candidateCount = previousLevel.size();
	}
}

