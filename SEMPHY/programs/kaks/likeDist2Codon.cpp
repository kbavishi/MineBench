// 	$Id: likeDist2Codon.cpp 2399 2014-03-13 22:43:51Z wkliao $	

#include "likeDist2Codon.h"
#include "numRec.h"


const MDOUBLE likeDist2Codon::giveDistance(	const countTableComponentGam& ctc,
										   MDOUBLE& resQ,
										   const MDOUBLE initialGuess) const {
	return giveDistanceBrent(ctc,resQ,initialGuess);
}

const MDOUBLE likeDist2Codon::giveDistanceBrent(	const countTableComponentGam& ctc,
										   MDOUBLE& resL,
										   const MDOUBLE initialGuess) const {
	const MDOUBLE ax=0,bx=initialGuess,cx=_maxPairwiseDistance,tol=_toll;
	MDOUBLE dist=-1.0;
	resL = -brent(ax,bx,cx,
		  C_evalLikeDist2Codon(ctc,_model),
		  tol,
		  &dist);
	/*resL = -dbrent(ax,bx,cx,
		  C_evalLikeDist2Codon(ctc,_model),
		  C_evalLikeDist_d_2Codon(ctc,_model),
		  tol,
		  &dist);*/
	return dist;
}
