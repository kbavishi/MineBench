// 	$Id: wYangModel.cpp 2399 2014-03-13 22:43:51Z wkliao $	
#include "wYangModel.h"
#include "codon.h"
#include "readDatMatrix.h" // for the normalizeQ function.

wYangModel::wYangModel(codon &inCodonAlpa,const MDOUBLE inW, const MDOUBLE tr,const MDOUBLE tv,const MDOUBLE trtr,const MDOUBLE tvtv,const MDOUBLE trtv,const MDOUBLE threeSub,bool globalW):_codonAlph(inCodonAlpa),_w(inW),_tr(tr),_tv(tv),_trtr(trtr),_tvtv(tvtv),_trtv(trtv),_threeSub(threeSub),_globalW(globalW){
	
	homogenousFreq();
	_Q.resize(alphabetSize());
	for (int z=0; z < _Q.size();++z) _Q[z].resize(alphabetSize(),0.0);
	updateQ();
}

wYangModel::wYangModel(codon &inCodonAlpa,const Vdouble& freq,const MDOUBLE inW, const MDOUBLE tr,const MDOUBLE tv,const MDOUBLE trtr,const MDOUBLE tvtv,const MDOUBLE trtv,const MDOUBLE threeSub,bool globalW):_codonAlph(inCodonAlpa),_w(inW),_tr(tr),_tv(tv),_trtr(trtr),_tvtv(tvtv),_trtv(trtv),_threeSub(threeSub),_globalW(globalW),_freq(freq){
	_Q.resize(alphabetSize());
	for (int z=0; z < _Q.size();++z) _Q[z].resize(alphabetSize(),0.0);
	updateQ();
}
void wYangModel::updateQ() {
	// building q.	
	int i,j;
	MDOUBLE sum=0.0;
	for (i=0; i < _Q.size();++i) {
		sum=0;
		for (j=0; j < _Q.size();++j) {
			if (i==j) continue;
			else if(codonUtility::codonDiff(i,j,_codonAlph) == codonUtility::tr) _Q[i][j] = _tr;	
			else if(codonUtility::codonDiff(i,j,_codonAlph) == codonUtility::tv) _Q[i][j] = _tv;
			else if(codonUtility::codonDiff(i,j,_codonAlph) == codonUtility::twoTrs) _Q[i][j] = _trtr;
			else if(codonUtility::codonDiff(i,j,_codonAlph) == codonUtility::twoTvs) _Q[i][j] = _tvtv;
			else if(codonUtility::codonDiff(i,j,_codonAlph) == codonUtility::trtv) _Q[i][j] = _trtv;
			else if(codonUtility::codonDiff(i,j,_codonAlph) == codonUtility::threesub )_Q[i][j] = _threeSub;
			
			if (codonUtility::codonReplacement(i,j,_codonAlph) == codonUtility::non_synonymous)
				_Q[i][j]*=_w;

			_Q[i][j]*=_freq[j];
			sum += _Q[i][j];
		}		
		_Q[i][i]=-sum;
	}
	if (_globalW == true)
		normalizeQ(_Q,_freq);
	
	
	_q2pt.fillFromRateMatrix(_freq,_Q); 
}


void wYangModel::norm(MDOUBLE scale){
	
	for (int i=0; i < _Q.size(); ++i) {
		for (int j=0; j < _Q.size(); ++j) {
			_Q[i][j]*=scale;
		}
	}
	
	_q2pt.fillFromRateMatrix(_freq,_Q);
}


MDOUBLE wYangModel::sumPijQij(){
	MDOUBLE sum=0.0;
	for (int i=0; i < _Q.size(); ++i) {
		sum -= _Q[i][i]*_freq[i];
	}
	return sum;
}
