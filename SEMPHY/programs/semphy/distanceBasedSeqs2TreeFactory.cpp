// 	$Id: distanceBasedSeqs2TreeFactory.cpp 2399 2014-03-13 22:43:51Z wkliao $	
#include "distanceBasedSeqs2TreeFactory.h"

distanceBasedSeqs2Tree* distanceBasedSeqs2TreeFactory(const distanceBasedMethod_t distanceBasedMethod, 
													  stochasticProcess& sp, // may change sp (alpha)
													  const bool   useJcDistance,
													  const bool   optimizeAlpha,
													  const double epsilonLikelihoodImprovement4iterNJ,
													  const double epsilonLikelihoodImprovement4pairwiseDistance,
													  const double epsilonLikelihoodImprovement4alphaOptimiz,
													  const double epsilonLikelihoodImprovement4BBL)
	//									,const int     maxNumOfBBLIter)
{

	// 1. Construct the object of the requested distance-based tree-reconstruction method (class distances2Tree)
	NJalg NJa;
	distances2Tree* d2tPtr = &NJa; // for future polymorphism

	// 2. Construct the object of the requested distance estimation method (class distanceMethod) distanceMethod *dmPtr = NULL;
	// And construct the object of the distance-based sequences-to-tree method (class distanceBasedSeqs2Tree)
	//	distanceMethod *dmPtr = NULL;
	distanceBasedSeqs2Tree *s2tPtr = NULL;
	
	
	switch (distanceBasedMethod) {
	case homogeneousRatesDTME:	
		if (useJcDistance) {
			jcDistance jcd(sp.alphabetSize());
			s2tPtr = new distanceBasedSeqs2Tree(jcd, *d2tPtr);  
		} else {
			likeDist ld(sp,epsilonLikelihoodImprovement4pairwiseDistance);
			s2tPtr = new distanceBasedSeqs2Tree(ld, *d2tPtr);  

		}
		break;
	case pairwiseGammaDTME:
		if (optimizeAlpha) {
			pairwiseGammaDistance pgd(sp,epsilonLikelihoodImprovement4pairwiseDistance); // distance method that optimizes alpha for the pair of sequences
			s2tPtr = new distanceBasedSeqs2Tree(pgd, *d2tPtr);
		} else {
			likeDist ld1(sp,epsilonLikelihoodImprovement4pairwiseDistance); // distance method that uses the given alpha with no optimization
			s2tPtr = new distanceBasedSeqs2Tree(ld1, *d2tPtr);
		}
		break;
	case commonAlphaDTME: {
		likeDist ld2(sp,epsilonLikelihoodImprovement4pairwiseDistance);
		s2tPtr = new commonAlphaDistanceSeqs2Tree(ld2, *d2tPtr, NULL, 
												  epsilonLikelihoodImprovement4iterNJ,
												  epsilonLikelihoodImprovement4alphaOptimiz,
												  epsilonLikelihoodImprovement4BBL); }
		break;
	case rate4siteDTME: {
		givenRatesMLDistance grd(sp,epsilonLikelihoodImprovement4pairwiseDistance);
		s2tPtr = new rate4siteDistanceSeqs2Tree(grd, *d2tPtr, NULL,
												epsilonLikelihoodImprovement4iterNJ,
												epsilonLikelihoodImprovement4alphaOptimiz,
												epsilonLikelihoodImprovement4BBL); }
		break;
	case posteriorDTME: {
		posteriorDistance posd(sp,epsilonLikelihoodImprovement4pairwiseDistance);
		s2tPtr = new posteriorDistanceSeqs2Tree(posd, *d2tPtr, NULL,
												epsilonLikelihoodImprovement4iterNJ,
												epsilonLikelihoodImprovement4alphaOptimiz,
												epsilonLikelihoodImprovement4BBL);}
		break;
	default: errorMsg::reportError("bad distanceBasedMethod - method not in the list of implimented methods");
	}
	return s2tPtr;
}






