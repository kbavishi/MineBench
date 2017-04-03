// 	$Id: simulateSequnce.h 2399 2014-03-13 22:43:51Z wkliao $	
#ifndef ___MAIN_SEMPHY_NEW
#define ___MAIN_SEMPHY_NEW

#include "alphabet.h"
#include "definitions.h"
#include "sequenceContainer.h"
#include "stochasticProcess.h"
//#include "treeInterface.h"
#include "nucleotide.h"
#include "amino.h"
#include "uniDistribution.h"
#include "gammaDistribution.h"
#include "readDatMatrix.h"
#include "aaJC.h"
#include "nucJC.h"
#include "hky.h"
#include "trivialAccelerator.h"
#include "chebyshevAccelerator.h"
#include "phylipFormat.h"
#include "maseFormat.h"
#include "fastaFormat.h"
#include "clustalFormat.h"
#include "molphyFormat.h"
#include "simulateTree.h"

#include <iostream>

using namespace std;
//using namespace treeInterface;


class simulateSequnce {
public:

static void printSemphyTitle(ostream & out) {
	out<<"*******************************************************************************"<<endl;
	out<<"SEMPHY::simulateSequnce - sequence simulation for Phylogeney                  "<<endl;
	out<<"for information, please send email to semphy@cs.huji.ac.il                    "<<endl;
	out<<"*******************************************************************************"<<endl;
	out<<endl;
}
};


#endif


