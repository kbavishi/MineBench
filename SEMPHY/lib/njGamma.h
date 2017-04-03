// $Id: njGamma.h 2399 2014-03-13 22:43:51Z wkliao $


#ifndef NJ_GAMMA_H
#define NJ_GAMMA_H

#include "distanceMethod.h"
#include "stochasticProcess.h"
#include "definitions.h"
#include "sequence.h"
#include "gammaDistribution.h"
#include "logFile.h"

#include <cmath>
using namespace std;

class gammaMLDistances : public distanceMethod{
public:
    explicit gammaMLDistances(const stochasticProcess & sp,
			      const VVdouble & posteriorProb, // pos * rate
			      const MDOUBLE toll =0.0001,
			      const MDOUBLE maxPairwiseDistance = 5.0);

    const MDOUBLE giveDistanceBasedOnPosterior(const sequence& s1,
					       const sequence& s2,
					       const vector<MDOUBLE>  * weights,
					       MDOUBLE* score=NULL) const;
  
    const MDOUBLE giveDistance(const sequence& s1,
			       const sequence& s2,
			       const vector<MDOUBLE>  * weights,
			       MDOUBLE* score=NULL) const;
  
    MDOUBLE giveDistanceOptAlphaForEachPairOfSequences(const sequence& s1,
						       const sequence& s2,
						       const vector<MDOUBLE>  * weights,
						       MDOUBLE* score=NULL,
						       MDOUBLE* alpha=NULL) const;

    MDOUBLE giveDistanceOptAlphaForPairOfSequences(const sequence& s1,
						   const sequence& s2,
						   const vector<MDOUBLE>  * weights,
						   MDOUBLE* score,
						   MDOUBLE* alpha) const;
    void setAlpha(MDOUBLE alpha) {
	(static_cast<gammaDistribution*>(_sp.distr()))->setAlpha(alpha);
    }

private:
    const stochasticProcess &_sp;
    const VVdouble & _posteriorProb;
    const MDOUBLE _toll;
    const MDOUBLE _maxPairwiseDistance;
    MDOUBLE giveInitialGuessOfDistance(const sequence& s1,
				       const sequence& s2,
				       const vector<MDOUBLE>  * weights,
				       MDOUBLE* score) const;
};



#endif
