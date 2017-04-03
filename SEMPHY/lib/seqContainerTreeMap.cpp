// $Id: seqContainerTreeMap.cpp 2399 2014-03-13 22:43:51Z wkliao $

#include <stdlib.h>
#include "seqContainerTreeMap.h"

void checkThatNamesInTreeAreSameAsNamesInSequenceContainer(const tree& et,const sequenceContainer & sc){
	treeIterDownTopConst tIt(et);
	//cout<<"tree names:"<<endl;
	for (tree::nodeP mynode = tIt.first(); mynode != tIt.end(); mynode = tIt.next()) {
		if (mynode->isInternal()) 
			continue;

		bool bFound = false;
		sequenceContainer::constTaxaIterator it=sc.constTaxaBegin();
		for (;it != sc.constTaxaEnd(); ++it) 
		{
			if (it->name() == mynode->name()) 
			{
				bFound = true;
				break;
			}
		}
		if (bFound == false) 
		{
			cerr<<"The sequences' name in the sequence file don't match the names in the tree file."<<endl;
			cerr<<"In the tree file there is the name: "<<mynode->name()<<" ";
			cerr<<"that is not found in the sequence file"<<endl;
			system("PAUSE");
			exit(0);
		}
	}
}

