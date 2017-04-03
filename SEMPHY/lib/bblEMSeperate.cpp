// $Id: bblEMSeperate.cpp 2399 2014-03-13 22:43:51Z wkliao $
#include "bblEM.h"
#include "bblEMSeperate.h"
#include "logFile.h"
//#define VERBOS


bblEMSeperate::bblEMSeperate(vector<tree>& et,
									const vector<sequenceContainer>& sc,
									const vector<stochasticProcess> &sp,
									const vector<Vdouble *> * weights,
									const int maxIterations,
									const MDOUBLE epsilon,
									const MDOUBLE tollForPairwiseDist) {
	MDOUBLE newL =0;
	for (int i=0; i < et.size(); ++i) {
		#ifdef VERBOS
			LOG(5,<<" OPTIMIZING GENE "<<i<<" ... "<<endl);
		#endif
		bblEM bblEM1(et[i],sc[i],sp[i],(weights?(*weights)[i]:NULL),maxIterations,epsilon);
		MDOUBLE resTmp =  bblEM1.getTreeLikelihood();
		#ifdef VERBOS
			LOG(5,<<" GENE "<<i<<" LOG-L = "<< resTmp<<endl);
		#endif
		newL += resTmp;
	}
	_treeLikelihood = newL;
}
