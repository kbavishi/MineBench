// $Id: likeDist2Codon.h 2399 2014-03-13 22:43:51Z wkliao $

#ifndef ___LIKE_DIST_2_CODON_H
#define ___LIKE_DIST_2_CODON_H

#include "definitions.h"
#include "countTableComponent.h"
#include "distanceMethod.h"
#include "stochasticProcess.h"
#include "logFile.h"
#include "JTTcodonModel.h"
#include "wYangModel.h"
#include "empirSelectionModel.h"
#include <cmath>
using namespace std;

class likeDist2Codon : public distanceMethod {
public:
	explicit likeDist2Codon(const empiriSelectionModel& model,
					  const MDOUBLE toll =0.0001,
					  const MDOUBLE maxPairwiseDistance = 50.0) :  _model(model) ,_toll(toll),_maxPairwiseDistance(maxPairwiseDistance) {
	}
  
  likeDist2Codon (const likeDist2Codon& other):  _model(other._model) ,_toll(other._toll),_maxPairwiseDistance(other._maxPairwiseDistance) {};
        virtual likeDist2Codon* clone() const {return new likeDist2Codon(*this);}

	// THIS FUNCTION DOES NOT RETURN THE LOG LIKELIHOOD IN RESQ, BUT RATHER "Q", THE CONTRIBUTION of this edge
    // TO THE EXPECTED LOG-LIKELIHOOD (SEE SEMPHY PAPER).
	// NEVERTHELESS, THE t that optimizes Q is the same t that optimizes log-likelihood.
	const MDOUBLE giveDistance(	const countTableComponentGam& ctc,
								MDOUBLE& resQ,
								const MDOUBLE initialGuess= 0.03) const; // initial guess
	
	
	// returns the estimated ML distance between the 2 sequences.
	// if score is given, it will be the log-likelihood.
	//!!!!!!!!!!!!!!TO DO
	const MDOUBLE giveDistance(const sequence& s1,
						const sequence& s2,
						const vector<MDOUBLE>  * weights,
						MDOUBLE* score=NULL) const { return 1;}

	const MDOUBLE giveDistanceBrent(	const countTableComponentGam& ctc,
										   MDOUBLE& resL,
										   const MDOUBLE initialGuess) const;
	
private:
	const empiriSelectionModel& _model;
	const MDOUBLE _toll;
	const MDOUBLE _maxPairwiseDistance;

};


class C_evalLikeDist2Codon{
private:
	const countTableComponentGam& _ctc;
	const empiriSelectionModel& _model;
public:
	C_evalLikeDist2Codon(const countTableComponentGam& ctc,
					const empiriSelectionModel& model):_ctc(ctc), _model(model) {};

	MDOUBLE operator() (MDOUBLE dist) {
		const MDOUBLE epsilonPIJ = 1e-10;
		MDOUBLE sumL=0.0;
		MDOUBLE pij;
		int categor, alph1,alph2;
		for (alph1=0; alph1 < _ctc.alphabetSize(); ++alph1){
			for (alph2=0; alph2 <  _ctc.alphabetSize(); ++alph2){
				for (categor = 0; categor<_model.noOfCategor(); ++categor) {				
					pij= _model.getCategor(categor).Pij_t(alph1,alph2,dist);
					if (pij<epsilonPIJ) pij = epsilonPIJ;//SEE REMARK (1) FOR EXPLANATION
					sumL += _ctc.getCounts(alph1,alph2,categor)*(log(pij)-log(_model.freq(alph2)));//*_sp.ratesProb(rateCategor);// removed.
					
				}
				 
				pij = _model.getNoSelectionModel().Pij_t(alph1,alph2,dist);
				if (pij<epsilonPIJ) pij = epsilonPIJ;
				sumL+=_ctc.getCounts(alph1,alph2,categor)*(log(pij)-log(_model.freq(alph2)));//*_sp.ratesProb(rateCategor);// removed.
			}
		}
	//	LOG(5,<<"check bl="<<dist<<" gives "<<sumL<<endl);

		return -sumL;
	};
};

// REMARK 1: THE LINE if if (pij<epsilonPIJ) pij = epsilonPIJ
// There are cases when i != j, and t!=0, and yet pij =0, because of numerical problems
// For these cases, it is easier to assume pij is very small, so that log-pij don't fly...

class C_evalLikeDist_d_2Codon{ // derivative.
public:
  C_evalLikeDist_d_2Codon(const countTableComponentGam& ctc,
				 const empiriSelectionModel& model)    : _ctc(ctc), _model(model) {};
private:
	const  countTableComponentGam& _ctc;
	const empiriSelectionModel& _model;
public:
	MDOUBLE operator() (MDOUBLE dist) {
		MDOUBLE	sumDL=0.0;
		MDOUBLE pij, dpij;
		int categor, alph1,alph2;
		for (alph1=0; alph1 <  _ctc.alphabetSize(); ++alph1){
			for (alph2=0; alph2 <  _ctc.alphabetSize(); ++alph2){
				for (categor = 0; categor<_model.noOfCategor(); ++categor) {
					MDOUBLE selection = _model.getDistr()->rates(categor);
					MDOUBLE pij= _model.getCategor(categor).Pij_t(alph1,alph2,dist);
					MDOUBLE dpij= _model.getCategor(categor).dPij_dt(alph1,alph2,dist);
					
					sumDL+= _ctc.getCounts(alph1,alph2,categor)*dpij
									*selection/pij; //to check why selection?
				}
				pij= _model.getNoSelectionModel().Pij_t(alph1,alph2,dist);
				dpij= _model.getNoSelectionModel().dPij_dt(alph1,alph2,dist);
				sumDL+= _ctc.getCounts(alph1,alph2,categor)*dpij/pij; //selection=1;
			}
		}
		//LOG(5,<<"derivation = "<<-sumDL<<endl);
		return -sumDL;
	};
};

#endif

