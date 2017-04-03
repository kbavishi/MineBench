// 	$Id: empirSelectionModel.h 2399 2014-03-13 22:43:51Z wkliao $	
#ifndef __EMPIRI_SELECTION_MODEL
#define __EMPIRI_SELECTION_MODEL

#include "replacementModel.h"
#include "fromQtoPt.h"
#include "readDatMatrix.h"
#include "datMatrixHolder.h"
#include "gammaDistribution.h"
#include "generalGammaDistribution.h"
#include "betaOmegaDistribution.h"
#include "JTTcodonModel.h"
#include "wYangModel.h"
#include "trivialAccelerator.h"
#include "uniDistribution.h"
#include "gammaUtilities.h"
#include "stochasticProcess.h"
#include "codon.h"

class empiriSelectionModel {
public:
	explicit empiriSelectionModel(){errorMsg::reportError("This constractor shold never be used");};
	explicit empiriSelectionModel(Vdouble freqCodon,Vdouble freqAmino,codon coAlph,bool isGamma = true,int noOfCategor=10,
		MDOUBLE tr = 5,MDOUBLE tv = 1,MDOUBLE trtr=0.0000005,MDOUBLE tvtv=0.0000005,MDOUBLE trtv=0.0000005,MDOUBLE threeSub = 0.0000001,MDOUBLE f=1,MDOUBLE alpha=0.5,MDOUBLE beta=0.5);
	virtual ~empiriSelectionModel();
	explicit empiriSelectionModel(const empiriSelectionModel& other);
	empiriSelectionModel& operator=(const empiriSelectionModel& other);
	const int alphabetSize() const {return _freqCodon.size();}
	Vdouble freqCodon()const {return _freqCodon;}
	Vdouble freqAmino()const {return _freqAmino;}
	const MDOUBLE getF() const {return _f;}
	const MDOUBLE getTr() const {return _tr;}
	const MDOUBLE getTv() const {return _tv;}
	const MDOUBLE getTrTr() const {return _trtr;}
	const MDOUBLE getTrTv() const {return _trtv;}
	const MDOUBLE getTvTv() const {return _tvtv;}
	const MDOUBLE getThreeSub()const {return _threeSub;}
	const MDOUBLE getAlpha() const {return (_isGamma? static_cast<generalGammaDistribution*>(_forceDistr)->getAlpha():static_cast<betaOmegaDistribution*>(_forceDistr)->getAlpha() );}
	const MDOUBLE getBeta() const {return (_isGamma? static_cast<generalGammaDistribution*>(_forceDistr)->getBeta():static_cast<betaOmegaDistribution*>(_forceDistr)->getBeta() );}
	const MDOUBLE getOmega()const;
	const MDOUBLE getBetaProb()const;
	const bool getIsGamma()const {return _isGamma;}
	distribution* getDistr()const {return _forceDistr;} 
	MDOUBLE freq(int codon)const {return _freqCodon[codon];}
	vector<stochasticProcess> & getSelectioModel(){return _spVec;}
	const stochasticProcess & getCategor(int i)const {return _spVec[i];}
	const MDOUBLE getSelection(const int i)const {return _forceDistr->rates(i);} 
	const MDOUBLE getCategorProb(const int i)  const {return _forceDistr->ratesProb(i);}
	const int noOfCategor()const {return _spVec.size();}
	const stochasticProcess & getNoSelectionModel() const{return _noSelectionModel;}
	void normalizeModel();
	void updateF(MDOUBLE f); 
	void updateTr(MDOUBLE Tr);
	void updateTv(MDOUBLE Tv);
	void updateTrTr(MDOUBLE TrTr);
	void updateTvTv(MDOUBLE TvTv);
	void updateTrTv(MDOUBLE TrTv);
	void updateAlpha(MDOUBLE alpha);
	void updateBeta(MDOUBLE beta);
	void updateThreeSub(MDOUBLE threeSub);

	void updateOmega(MDOUBLE omega);  //for betaOmega distribution
	void updateBetaProb(MDOUBLE betaProb); //for betaOmega distribution

	const MDOUBLE getKaKsNoSelection()const;
	const MDOUBLE getKaKsCategor(const int i)const {return _KaKsSelection[i];}
	void fillKaKsSelection();
	//For test
	void updateW(MDOUBLE W);


private:
	Vdouble _freqCodon;
	Vdouble _freqAmino;	
	MDOUBLE _tr; // Tr
	MDOUBLE _tv; //Tv
	MDOUBLE _trtr; // TrTr
	MDOUBLE _tvtv; // TvTv
	MDOUBLE _trtv; // TrTv
	MDOUBLE _threeSub;
	MDOUBLE _f; //probability of selection model
	distribution *_forceDistr; //can be gamma or beta+W
	vector<stochasticProcess> _spVec; //mode of selection
	stochasticProcess  _noSelectionModel;
	Vdouble _KaKsSelection; //hold the kaks for each catregor( define different Q matrix)
	bool _isGamma; // true for gamma
	codon _codonAlph; 

};


#endif
