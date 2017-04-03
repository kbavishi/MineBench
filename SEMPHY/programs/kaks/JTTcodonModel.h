// 	$Id: JTTcodonModel.h 2399 2014-03-13 22:43:51Z wkliao $	
#ifndef ___JTT_CODON_MODEL
#define ___JTT_CODON_MODEL

#include "replacementModel.h"
#include "fromQtoPt.h"
#include "readDatMatrix.h"
#include "datMatrixHolder.h"
#include "gammaDistribution.h"
#include "gammaUtilities.h"
#include "codon.h"
class jttCodonModel : public replacementModel {
public:
	
	const int alphabetSize() const {return _freqCodon.size();}
	virtual replacementModel* clone() const { return new jttCodonModel(*this); }
	explicit jttCodonModel(const MDOUBLE inW, const MDOUBLE inTr, const MDOUBLE inTv,
							const MDOUBLE inTrTr, const MDOUBLE inTvTv,const MDOUBLE inTrTv, const MDOUBLE inThreeSub,codon &inCodonAlph,const bool globalV=true);
	explicit jttCodonModel(const MDOUBLE inW, const MDOUBLE inTr, const MDOUBLE inTv,
							 const MDOUBLE inTrTr, const MDOUBLE inTvTv,const MDOUBLE inTrTv,const MDOUBLE inThreeSub,
							 const Vdouble& freqC, const Vdouble& freqA,codon &inCodonAlph,const bool globalW=true);
	const MDOUBLE Pij_t(const int i,const int j, const MDOUBLE d) const {
		return _q2pt.Pij_t(i,j,d);
	}
	const MDOUBLE dPij_dt(const int i,const int j, const MDOUBLE d) const{
		return _q2pt.dPij_dt(i,j,d);
	}
	const MDOUBLE d2Pij_dt2(const int i,const int j, const MDOUBLE d) const{
		return _q2pt.d2Pij_dt2(i,j,d);
	}
	const MDOUBLE freq(const int i) const {return _freqCodon[i];};
	const MDOUBLE freqAmino(const int i) const {return _freqAmino[i];};
	void homogenousCodonFreq(){ _freqCodon.erase(_freqCodon.begin(),_freqCodon.end()),_freqCodon.resize(alphabetSize(),1.0/alphabetSize());}
	void homogenousAminoFreq(){ _freqAmino.erase(_freqAmino.begin(),_freqAmino.end()),_freqAmino.resize(20,1.0/20);}
	MDOUBLE getQij(const int i,const int j)const {return _Q[i][j];}
	MDOUBLE findX(MDOUBLE val, vector<int> codonsOfI, vector<int> codonsOfJ);
	void updateJTT();
	void fromFreqAminoToFreqCodon();
	void fromJTTFreqAminoToFreqCodonFromData();
	void checkModel();
	void setGlobalW(const bool globalW){ _globalW=globalW;}
	void setTr(const MDOUBLE tr) { _tr = tr; updateQ();}
	void setTv(const MDOUBLE tv) { _tv = tv; updateQ();}
	void setTrTr(const MDOUBLE trtr) { _trtr = trtr; updateQ();}
	void setTvTv(const MDOUBLE tvtv) { _tvtv = tvtv; updateQ();}
	void setTrTv(const MDOUBLE trtv) { _trtv = trtv; updateQ();}
	void setThreeSub(const MDOUBLE threeSub) { _threeSub = threeSub; updateQ();}
	void setW(const MDOUBLE newW) { _w = newW; updateQ();}
	void updateQ();
	void updateW();
	
	MDOUBLE getTr() const{return _tr;}
	MDOUBLE getTv() const{return _tv;}
	MDOUBLE getTrTr() const{return _trtr;}
	MDOUBLE getTvTv() const{return _tvtv;}
	MDOUBLE getTrTv()const {return _trtv;}
	MDOUBLE getThreeSub()const {return _threeSub;}
	MDOUBLE getW() const{return _w;}
	void norm(MDOUBLE scale);
	MDOUBLE sumPijQij();

	



private:
	
	MDOUBLE _w; //selection factor.
	MDOUBLE _tr; // Tr
	MDOUBLE _tv; //Tv
	MDOUBLE _trtr; // TrTr
	MDOUBLE _tvtv; // TvTv
	MDOUBLE _trtv; // TrTv
	MDOUBLE _threeSub;
	q2pt _q2pt;
	VVdouble _JTT;
	VVdouble _Q;
	Vdouble _freqCodon;
	Vdouble _freqAmino;
	bool _globalW;   //false when compute w per site
	codon  _codonAlph; //hold the codon alphabet according a specific genetic code
};

#endif
