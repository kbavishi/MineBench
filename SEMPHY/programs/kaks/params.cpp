// 	$Id: params.cpp 2399 2014-03-13 22:43:51Z wkliao $	
#include "params.h"
#include "talRandom.h"

params::params( const tree t){
	MDOUBLE upperValue = 10.0;
	MDOUBLE upperValueAlpha = 5;
	MDOUBLE upperValueBeta = 15;
	_tr = talRandom::giveRandomNumberBetweenZeroAndEntry(upperValue);
	_tv = talRandom::giveRandomNumberBetweenZeroAndEntry(upperValue);
	_trtv = talRandom::giveRandomNumberBetweenZeroAndEntry(upperValue);
	_tvtv = talRandom::giveRandomNumberBetweenZeroAndEntry(upperValue);
	_trtr = talRandom::giveRandomNumberBetweenZeroAndEntry(upperValue);
	_f = talRandom::giveRandomNumberBetweenZeroAndEntry(1.0);
	_alpha = talRandom::giveRandomNumberBetweenZeroAndEntry(5.0);
	_beta = talRandom::giveRandomNumberBetweenZeroAndEntry(15.0);
	_et = t;
	
}
params::params(const params & other){
	_tr=other._tr;
	_tv=other._tv;
	_trtr=other._trtr;
	_tvtv=other._tvtv;
	_trtv=other._trtv;
	_f=other._f;
	_alpha=other._alpha;
	_beta = other._beta;
	_et= other._et;
}

void params::setParams(const MDOUBLE tr,const MDOUBLE tv,const MDOUBLE trtr,const MDOUBLE tvtv,
					   const MDOUBLE trtv,const MDOUBLE f,const MDOUBLE alpha,const MDOUBLE beta, const tree t){
	_tr=tr;
	_tv=tv;
	_trtr=trtr;
	_tvtv=tvtv;
	_trtv=trtv;
	_f=f;
	_alpha=alpha;
	_beta = beta;
	_et = t;
}
