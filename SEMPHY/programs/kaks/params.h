// 	$Id: params.h 2399 2014-03-13 22:43:51Z wkliao $	
#ifndef ___PARAMS____H
#define ___PARAMS____H

#include "definitions.h"
#include "tree.h"



class params {

public:
	explicit params(const tree t);
	explicit params(const params &other);
	explicit params (const MDOUBLE tr,const MDOUBLE tv,const MDOUBLE trtr,const MDOUBLE tvtv,
		const MDOUBLE trtv,const MDOUBLE f,const MDOUBLE alpha,const MDOUBLE beta,  tree &t):_tr(tr),_tv(tv),_trtr(trtr),
												_tvtv(tvtv),_trtv(trtv),_f(f),_alpha(alpha),_beta(beta),_et(t){};
	MDOUBLE tr()const {return _tr;};
	MDOUBLE tv()const {return _tv;};
	MDOUBLE trtr()const {return _trtr;};
	MDOUBLE trtv()const {return _trtv;};
	MDOUBLE tvtv()const {return _tvtv;};
	MDOUBLE f()const {return _f;};
	MDOUBLE alpha()const {return _alpha;};
	MDOUBLE beta()const {return _beta;};
	tree getTree() {return _et;};
	void setParams(const MDOUBLE tr,const MDOUBLE tv,const MDOUBLE trtr,const MDOUBLE tvtv,
					const MDOUBLE trtv,const MDOUBLE f,const MDOUBLE alpha,const MDOUBLE beta,const tree et);

private:
	MDOUBLE _tr; // Tr
	MDOUBLE _tv; //Tv
	MDOUBLE _trtr; // TrTr
	MDOUBLE _tvtv; // TvTv
	MDOUBLE _trtv; // TrTv
	MDOUBLE _f; //probability
	MDOUBLE _alpha;
	MDOUBLE _beta;
	tree _et;
};


#endif
