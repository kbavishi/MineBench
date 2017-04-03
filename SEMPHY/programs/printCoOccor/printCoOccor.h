// 	$Id: printCoOccor.h 2399 2014-03-13 22:43:51Z wkliao $	

#include "definitions.h"  
#include "sequenceContainer.h"
#include "computeUpAlg.h"
#include "computeDownAlg.h"
#include "computePijComponent.h"
#include "logFile.h"
#include "likelihoodComputation.h"
#include <cmath>

// added by matan - for loggin perpeses
void PrintCoOccorence(const sequenceContainer& sc,
		      const stochasticProcess& sp,
		      const Vdouble * weights,
		      const int nodeId,
		      const suffStatGlobalGam& cup,
		      const suffStatGlobalGam& cdown,
		      const MDOUBLE dist,
		      ostream& out);

void printAllCC(const sequenceContainer& sc,
		const stochasticProcess& sp,
		const Vdouble * weights,
		const tree& et,
		const tree::nodeP in_nodep,
		const suffStatGlobalGam& cup,
		const suffStatGlobalGam& cdown,
		ostream& out);

