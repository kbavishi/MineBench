/* Parallel Recursive ETI Miner
 * Coded by Blayne Field
 */

#include "DataSet.h"

void DataSet::ComputeSupport(vector<SparseItemset *> candidateSparseItemsets) const
{
	int itemsetCount = candidateSparseItemsets.size();
	
	// reset all the counts to 0
	for (int i = 0; i < itemsetCount; i++) {
		candidateSparseItemsets[i]->ClearCounter(transactionCount);
	}
		
	// for each transaction, compare it to each of the candidate itemsets
	for (int t = 0; t < transactionCount; t++) {
		for (int i = 0; i < itemsetCount; i++) {
//			if (!candidateSparseItemsets[i]->IncrementSupportCounts(transactions[t]))
	//			break;	
			candidateSparseItemsets[i]->IncrementSupportCounts(transactions[t]);
		}
	}

#ifdef USE_MPI
	// create the global count arrays
	int itemsetSize = candidateSparseItemsets[0]->itemCount;
	int entriesNeeded = (itemsetSize+1) * candidateSparseItemsets.size();
	int *sendGlobalCountArray = new int[entriesNeeded];
	int *receiveGlobalCountArray = new int[entriesNeeded];
	for (int i = 0; i < itemsetCount; i++) {
		memcpy(sendGlobalCountArray + i * (itemsetSize+1), candidateSparseItemsets[i]->supportCounts, 
					 sizeof(int) * (itemsetSize+1));
	}

	// send the results to everybody!
	MPI_Allreduce(sendGlobalCountArray, receiveGlobalCountArray, entriesNeeded, MPI_INT, MPI_SUM, MPI_COMM_WORLD);

	// put the results back into the correct itemsets
	for (int i = 0; i < itemsetCount; i++) {
		memcpy(candidateSparseItemsets[i]->supportCounts, receiveGlobalCountArray + i * (itemsetSize+1),
					 sizeof(int) * (itemsetSize+1));
	}

	// delete the no longer needed memory
	delete [] sendGlobalCountArray;
	delete [] receiveGlobalCountArray;
#endif
}

#ifdef USE_MPI
DataSet::DataSet(string filename, int threshold, int procs, int id):
	processCount(procs), processID(id)
#else
DataSet::DataSet(string filename, int threshold)
#endif
{
	itemCount = 0;
	transactionCount = 0;
	int maxItem = 0;
	int frequentItems;
	vector< vector<int> > theData;

	// get the correct file name (we assume if we are using MPI, the filename is followed by _id)
#ifdef USE_MPI
	stringstream ss;
	string actualFilename;
	ss << filename << "_" << id;
	ss >> actualFilename;	
	ifstream inFile(actualFilename.c_str());
#else
	ifstream inFile(filename.c_str());
#endif


	// load in the data
	string thisLine;		
	while (getline(inFile, thisLine)) {
		vector<int> currentLine;
		int temp;
		stringstream stream(thisLine, stringstream::in);

		while (stream >> temp) {
			currentLine.push_back(temp-1);
			if (temp > maxItem)
				maxItem = temp;
		}
		theData.push_back(currentLine);
	}
	inFile.close();
	
	
	// find out how many items there were across processes

#ifdef USE_MPI
	MPI_Allreduce(&maxItem, &itemCount, 1, MPI_INT, MPI_MAX, MPI_COMM_WORLD);
#else
	itemCount = maxItem;
#endif
	
	// calculate the support
#ifdef USE_MPI
	int *sendSupport = new int[itemCount];
	memset(sendSupport, 0, sizeof(int) * itemCount);
#endif
	int *support = new int[itemCount];
	memset(support, 0, sizeof(int) * itemCount);
	for (int t = 0; t < (int) theData.size(); t++) {
		for (int i = 0; i < (int) theData[t].size(); i++) {
#ifdef USE_MPI			
			sendSupport[theData[t][i]]++;		
#else
			support[theData[t][i]]++;
#endif
		}
	}
#ifdef USE_MPI
	MPI_Allreduce(sendSupport, support, itemCount, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
#endif

	// find the number of frequent items
	vector<int> frequentID;
	frequentItems = 0;
	for (int i = 0; i < itemCount; i++) {
		frequentID.push_back(frequentItems);
		if (support[i] >= threshold) {
			singletons.push_back(new SparseItemset(frequentItems, support[i]));
			itemMap.push_back(i);
			frequentItems++;
		}
	}	
	
	// make the itemsets!!!
	for (int t = 0; t < (int) theData.size(); t++) {
		vector<int> thisTrans;
		for (int i = 0; i < (int) theData[t].size(); i++) {
			if (support[theData[t][i]] >= threshold)
				thisTrans.push_back(frequentID[theData[t][i]]);
		}
		if (thisTrans.size() > 0)
			transactions.push_back(new SparseItemset(thisTrans));
	}

	itemCount = frequentItems;
	transactionCount = transactions.size();


#ifdef USE_MPI
	delete [] sendSupport;
#endif
	delete [] support;
	
	cerr << itemCount << " frequent items found, and there were " << transactionCount << " interesting transactions" << endl;
}

int DataSet::GetItemCount() const {return itemCount;}
vector<SparseItemset *> DataSet::GetSingletonItemsets() const {return singletons;}

