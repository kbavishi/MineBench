// 	$Id: optimizeFanctors.h 2399 2014-03-13 22:43:51Z wkliao $	
#ifndef __OPTIMIZE_FANCTORS
#define __OPTIMIZE_FANCTORS
#include "likelihoodComputation.h"
#include "JTTcodonModel.h"

class evaluate_L_given_w{
public:
  explicit evaluate_L_given_w(	const sequenceContainer& sc,
								const tree& t,
								const stochasticProcess& sp,
								const int pos)
			:_sc(sc),_t(t),_pos(pos), _sp(sp){}
private:
	const sequenceContainer& _sc;
	const tree& _t;
	const int _pos;
	const stochasticProcess& _sp;

public:
	MDOUBLE operator() (const MDOUBLE w){
		static_cast<jttCodonModel*>(_sp.getPijAccelerator()->getReplacementModel())->setW(w); 
		MDOUBLE tmp = likelihoodComputation::getLofPos(_pos,_t,_sc,_sp,1); 
		#ifdef VERBOS
			LOG(5,<<" w = "<<w<<" l = "<<log(tmp)<<endl);
		#endif
		return -log(tmp);  //to check if log or not?????
	}
};

#endif
