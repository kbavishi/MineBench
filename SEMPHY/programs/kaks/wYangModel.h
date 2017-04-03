// 	$Id: wYangModel.h 2399 2014-03-13 22:43:51Z wkliao $	
#ifndef _W_YANG_MODEL
#define _W_YANG_MODEL

#include "replacementModel.h"
#include "fromQtoPt.h"
#include "gammaDistribution.h"
#include "gammaUtilities.h"
#include "codon.h"

class wYangModel : public replacementModel {
public:
	const int alphabetSize() const {return _freq.size();}
	virtual replacementModel* clone() const { return new wYangModel(*this); }
	explicit wYangModel(codon &inCodonAlpa,const MDOUBLE inW=1, const MDOUBLE tr=5,const MDOUBLE tv=1,const MDOUBLE trtr=0000005,const MDOUBLE tvtv=0.0000005,const MDOUBLE trtv=0.0000005,const MDOUBLE threeSub=0.0000001,bool globalW=true);
	explicit wYangModel(codon &inCodonAlpa,const Vdouble& freq,const MDOUBLE inW=1, const MDOUBLE tr=5,const MDOUBLE tv=1,const MDOUBLE trtr=1,const MDOUBLE tvtv=0.0001,const MDOUBLE trtv=0.05,const MDOUBLE threeSub=0.0001,bool globalW=true);
	const MDOUBLE Pij_t(const int i,const int j, const MDOUBLE d) const {
		return _q2pt.Pij_t(i,j,d);
	}
	const MDOUBLE dPij_dt(const int i,const int j, const MDOUBLE d) const{
		return _q2pt.dPij_dt(i,j,d);
	}
	const MDOUBLE d2Pij_dt2(const int i,const int j, const MDOUBLE d) const{
		return _q2pt.d2Pij_dt2(i,j,d);
	}
	const MDOUBLE freq(const int i) const {return _freq[i];};
	
	void homogenousFreq(){ _freq.erase(_freq.begin(),_freq.end()),_freq.resize(alphabetSize(),1.0/alphabetSize());}

	MDOUBLE getTr() const{return _tr;}
	MDOUBLE getTv() const{return _tv;}
	MDOUBLE getTrTr() const{return _trtr;}
	MDOUBLE getTvTv() const{return _tvtv;}
	MDOUBLE getTrTv()const {return _trtv;}
	MDOUBLE getThreeSub()const {return _threeSub;}
	MDOUBLE getW() const {return _w;}
	VVdouble& getQ()  {return _Q;};

	void setTr(const MDOUBLE tr) { _tr = tr; updateQ();}
	void setTv(const MDOUBLE tv) { _tv = tv; updateQ();}
	void setTrTr(const MDOUBLE trtr) { _trtr = trtr; updateQ();}
	void setTvTv(const MDOUBLE tvtv) { _tvtv = tvtv; updateQ();}
	void setTrTv(const MDOUBLE trtv) { _trtv = trtv; updateQ();}
	void setThreeSub(const MDOUBLE threeSub) { _threeSub = threeSub; updateQ();}
	void setW(const MDOUBLE newW) { _w = newW;updateQ();}
	
	MDOUBLE getQij(const int i,const int j)const {return _Q[i][j];}
	void setGlobalW(bool globalW){_globalW = globalW;}
	void norm(MDOUBLE scale);
	MDOUBLE sumPijQij();
	void updateQ();


private:
	
	MDOUBLE _w; //selection factor.
	MDOUBLE _tr; // Tr
	MDOUBLE _tv; //Tv
	MDOUBLE _trtr; // TrTr
	MDOUBLE _tvtv; // TvTv
	MDOUBLE _trtv; // TrTv
	MDOUBLE _threeSub;
	q2pt _q2pt;	
	VVdouble _Q;
	bool _globalW;   //false when compute w per site
	Vdouble _freq;
	codon _codonAlph;
};


#endif
