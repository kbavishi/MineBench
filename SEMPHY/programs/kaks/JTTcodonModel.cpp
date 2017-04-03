// 	$Id: JTTcodonModel.cpp 2399 2014-03-13 22:43:51Z wkliao $	
#include "JTTcodonModel.h"
#include "utils.h"
#include "codon.h"
#include "amino.h"

//#define VERBOS
jttCodonModel::jttCodonModel(const MDOUBLE inW, const MDOUBLE inTr, const MDOUBLE inTv,
							 const MDOUBLE inTrTr, const MDOUBLE inTvTv,const MDOUBLE inTrTv,const MDOUBLE inThreeSub,codon &inCodonAlph,const bool globalW) :
     _w(inW),_tr(inTr),_tv(inTv),_trtr(inTrTr),_tvtv(inTvTv),_trtv(inTrTv),_threeSub(inThreeSub),_globalW(globalW),_codonAlph(inCodonAlph){
	homogenousAminoFreq();
	fromFreqAminoToFreqCodon();
	updateJTT();
	_Q.resize(alphabetSize());
	for (int z=0; z < _Q.size();++z) _Q[z].resize(alphabetSize(),0.0);
	updateQ();
}

jttCodonModel::jttCodonModel(const MDOUBLE inW,const MDOUBLE inTr, const MDOUBLE inTv, 
							 const MDOUBLE inTrTr, const MDOUBLE inTvTv,const MDOUBLE inTrTv,const MDOUBLE inThreeSub, const Vdouble& freqC, const Vdouble& freqA,codon &inCodonAlph,const bool globalW) :
	_w(inW),_tr(inTr),_tv(inTv),_trtr(inTrTr),_tvtv(inTvTv),_trtv(inTrTv),_threeSub(inThreeSub),_freqCodon(freqC),_freqAmino(freqA),_globalW(globalW),_codonAlph(inCodonAlph) {
	updateJTT();
	_Q.resize(alphabetSize());
	for (int z=0; z < _Q.size();++z) _Q[z].resize(alphabetSize(),0);
	updateQ();	
}

void jttCodonModel::updateQ() {
	// building q.
	vector<int> codonOfI,codonOfJ;
	int i,j;
	//non-synonymous substitutions 
	for (i=0;i<_JTT.size();i++){
		codonOfI = aminoUtility::codonOf(i,_codonAlph);
		for (j=i+1;j<_JTT.size();j++){
			codonOfJ = aminoUtility::codonOf(j,_codonAlph);
			findX(_JTT[i][j]*_freqAmino[i],codonOfI,codonOfJ);
		}
	}
	
	//synonymous substitutions
	int x = 1;
	for (i=0;i<_JTT.size();i++){
		codonOfI = aminoUtility::codonOf(i,_codonAlph);
		for (int l = 0; l < codonOfI.size(); l++){
			int c1 = codonOfI[l];		
			for (int m = l+1; m<codonOfI.size(); m++){
				int c2 = codonOfI[m];
				if (codonUtility::codonDiff(c1,c2,_codonAlph) == codonUtility::threesub){
					_Q[c1][c2] = freq(c2)*x*_threeSub;
					_Q[c2][c1] = freq(c1)*x*_threeSub;
				}else if (codonUtility::codonDiff(c1,c2,_codonAlph) == codonUtility::twoTrs){
					_Q[c1][c2] = freq(c2)*_trtr*x;
					_Q[c2][c1] = freq(c1)*_trtr*x;
				}else if (codonUtility::codonDiff(c1,c2,_codonAlph) == codonUtility::twoTvs){
					_Q[c1][c2] = freq(c2)*_tvtv*x;
					_Q[c2][c1] = freq(c1)*_tvtv*x;
				}else if (codonUtility::codonDiff(c1,c2,_codonAlph) == codonUtility::trtv){
					_Q[c1][c2] = freq(c2)*_trtv*x;
					_Q[c2][c1] = freq(c1)*_trtv*x;
				} else if (codonUtility::codonDiff(c1,c2,_codonAlph) == codonUtility::tr) {
					_Q[c1][c2] = freq(c2)*_tr*x;
					_Q[c2][c1] = freq(c1)*_tr*x;			
				} else if (codonUtility::codonDiff(c1,c2,_codonAlph) == codonUtility::tv) {
					_Q[c1][c2] = freq(c2)*_tv*x;
					_Q[c2][c1] = freq(c1)*_tv*x;
				}

			}
		}		
	}

	//LOG(5,<<"**********"<<endl);
	updateW();
	for (i=0;i<_Q.size();++i){
		MDOUBLE sum=0.0;
		for (j=0;j<_Q.size();++j){
			if (i==j) continue;		
			sum += _Q[i][j];
				
		}
		_Q[i][i]= (-sum);
	}
	
/*	LOG(5,<<"----------Q matrix of amion --------------"<<endl);
	for (i=0;i<_JTT.size();++i){
		for (j=0;j<_JTT.size();++j){
			LOG(5,<< " "<<_JTT[i][j]);
				
		}
		LOG(5,<<endl);
	}
	LOG(5,<<endl);
	LOG(5,<<endl);
	LOG(5,<<"----------Q matrix of codon --------------"<<endl);
	for (i=0;i<_Q.size();++i){
		for (j=0;j<_Q.size();++j){
			LOG(5,<< " "<<_Q[i][j]);
				
		}
		LOG(5,<<endl);
	}
*/
	
	if (_globalW == true)
		normalizeQ(_Q,_freqCodon);
	
//	_q2pt.fillFromRateMatrix(_freqCodon,_Q); 
}


//find parameter x when only in case of nonsynonymous
MDOUBLE jttCodonModel::findX(MDOUBLE val, vector<int> codonOfI, vector<int> codonOfJ){
	int l,m;
	MDOUBLE sum=0.0,freqCodon1,freqCodon2,res=0.0;	
	int c1, c2;
	for (l = 0; l < codonOfI.size(); l++){
		c1 = codonOfI[l];
		freqCodon1 = _freqCodon[c1];
		for (m = 0; m < codonOfJ.size(); m++){
			c2 = codonOfJ[m];
			freqCodon2 = _freqCodon[c2];
			if (codonUtility::codonDiff(c1,c2,_codonAlph) == codonUtility::threesub){
				sum += freqCodon1*freqCodon2*_threeSub;
			}else if (codonUtility::codonDiff(c1,c2,_codonAlph) == codonUtility::twoTrs){
				sum+= freqCodon1*freqCodon2*_trtr;
			}else if (codonUtility::codonDiff(c1,c2,_codonAlph) == codonUtility::twoTvs){
				sum+= freqCodon1*freqCodon2*_tvtv;
			}else if (codonUtility::codonDiff(c1,c2,_codonAlph) == codonUtility::trtv){
				sum+= freqCodon1*freqCodon2*_trtv;
			} else if (codonUtility::codonDiff(c1,c2,_codonAlph) == codonUtility::tr) {
				sum+= freqCodon1*freqCodon2*_tr;				
			} else if (codonUtility::codonDiff(c1,c2,_codonAlph) == codonUtility::tv) {
				sum+= freqCodon1*freqCodon2*_tv;
			}
				
		}
	}
	MDOUBLE x=0.0;
	if (sum==0) 
	  errorMsg::reportError("sum==0 in building Q matrix");
	else 
	  x=val/sum;
	for (l = 0; l < codonOfI.size(); l++){
		c1 = codonOfI[l];
		for (m = 0; m<codonOfJ.size(); m++){
			c2 = codonOfJ[m];
			if (codonUtility::codonDiff(c1,c2,_codonAlph) == codonUtility::threesub) {
					_Q[c1][c2] = freq(c2)*x*_threeSub;
					_Q[c2][c1] = freq(c1)*x*_threeSub;
			}else if (codonUtility::codonDiff(c1,c2,_codonAlph) == codonUtility::tr) {
					_Q[c1][c2] = freq(c2)*_tr*x;
					_Q[c2][c1] = freq(c1)*_tr*x;
			} else if (codonUtility::codonDiff(c1,c2,_codonAlph) == codonUtility::tv) {
					_Q[c1][c2] = freq(c2)*_tv*x;
					_Q[c2][c1] = freq(c1)*_tv*x;
			}else if (codonUtility::codonDiff(c1,c2,_codonAlph) == codonUtility::twoTvs) {
					_Q[c1][c2] = freq(c2)*_tvtv*x;
					_Q[c2][c1] = freq(c1)*_tvtv*x;
			}else if (codonUtility::codonDiff(c1,c2,_codonAlph) == codonUtility::twoTrs) {
					_Q[c1][c2] = freq(c2)*_trtr*x;
					_Q[c2][c1] = freq(c1)*_trtr*x;
			}else if (codonUtility::codonDiff(c1,c2,_codonAlph) == codonUtility::trtv) {
					_Q[c1][c2] = freq(c2)*_trtv*x;
					_Q[c2][c1] = freq(c1)*_trtv*x;
			}			
			res+= freq(c1)*_Q[c1][c2];
		}
			
	}

	if (fabs(val-res)>0.00000001){
		cout<<"******ERROR In filling q matrix in JTT model**********" <<endl<<val<< " " <<res<<endl;
		exit(0);
	}

	return x;
}

void jttCodonModel::updateJTT(){
	VVdouble sMatrix;
	readDatMatrixFromString(datMatrixHolder::jones.Val,sMatrix,_freqAmino);
	cout<<"Running with JTT matrix"<<endl;
	//readDatMatrixFromString(datMatrixHolder::wag.Val,sMatrix,_freqAmino);
	//readDatMatrixFromString(datMatrixHolder::mtREV24.Val,sMatrix,_freqAmino);
	//readDatMatrixFromString(datMatrixHolder::cpREV45.Val,sMatrix,_freqAmino);
	_freqAmino = fromFreqCodonToFreqAmino(_freqCodon,_codonAlph);
	_JTT = fromWagSandFreqToQ(sMatrix,_freqAmino); //_JTT hold Q of JTT
	
}

 
void jttCodonModel::fromFreqAminoToFreqCodon(){
	int i;
	for (i=0;i<alphabetSize();i++) _freqCodon.push_back(0);
	for (i=0;i<_freqAmino.size();i++){		
		vector<int> codonOfI = aminoUtility::codonOf(i,_codonAlph);
		int size = codonOfI.size();
		MDOUBLE aminoFreq = _freqAmino[i];
		for (int j=0; j<size; j++)
			_freqCodon[codonOfI[j]] = aminoFreq/size;
	
	}


	
//#ifdef VERBOS 
	MDOUBLE sum=0,epsilon = 1e-005;
	for (i=0;i<alphabetSize();i++)
		sum+=_freqCodon[i];
	if (fabs(sum - 1.0)> epsilon){
		cout<<"******ERROR fromFreqAminoToFreqCodon **********" <<fabs(sum - 1.0)<<endl;
		exit(0);
	}
//#endif;
}

void jttCodonModel::fromJTTFreqAminoToFreqCodonFromData(){
	int i;
	for (i=0;i<_freqAmino.size();i++){		
		vector<int> codonOfI = aminoUtility::codonOf(i,_codonAlph);
		int size = codonOfI.size();
		MDOUBLE aminoFreq = _freqAmino[i];
		MDOUBLE min = 1;
		MDOUBLE elem = 0.0;
		MDOUBLE sum = 0.0;
		MDOUBLE epsilon = 1e-005;
		int j;
		for (j=0; j<size; j++)	
			min = (min<_freqCodon[codonOfI[j]] ? min : _freqCodon[codonOfI[j]]);
		for (j=0; j<size; j++)
			elem+=(_freqCodon[codonOfI[j]]/min);
		elem = aminoFreq/elem;
		for (j=0; j<size; j++)
			_freqCodon[codonOfI[j]]=(_freqCodon[codonOfI[j]]/min)*elem;
		for (j=0; j<size; j++)
			sum +=_freqCodon[codonOfI[j]];
		if (fabs(sum - aminoFreq)> epsilon){
			cout<<"******ERROR fromFreqAminoToFreqCodonFromData **********" <<fabs(sum - aminoFreq)<<endl;
			exit(0);
		}
	}


	
//#ifdef VERBOS 
	MDOUBLE sum=0,epsilon = 1e-005;
	for (i=0;i<alphabetSize();i++)
		sum+=_freqCodon[i];
	if (fabs(sum - 1.0)> epsilon){
		cout<<"******ERROR fromFreqAminoToFreqCodonFromData **********" <<fabs(sum - 1.0)<<endl;
		exit(0);
	}
//#endif;
}




void jttCodonModel::updateW(){
	int i,j;
	vector<int> codonOfJ, codonOfI;
	for (i=0;i<_JTT.size();i++){
		codonOfI = aminoUtility::codonOf(i,_codonAlph);
		for (j=0;j<_JTT.size();j++){
			if (i==j) continue;
			codonOfJ = aminoUtility::codonOf(j,_codonAlph);
			int l,m;
			int c1, c2;
			for (l = 0; l < codonOfI.size(); l++){
				c1 = codonOfI[l];			
				for (m = 0; m<codonOfJ.size(); m++){
					c2 = codonOfJ[m];
					_Q[c1][c2]*=_w;	
				}
			}
		}
	}
}

/*
void jttCodonModel::evaluateCodonFreqFromAmino(const sequenceContainer & sc, const Vdouble freqAmino){

	Vdouble codonFreqData = evaluateCharacterFreq(sc);
	Vdouble freqAminoData(20,0);
	vector<int> codonVec;
	for (int j = 0; j<freqAminoData.size();j++){
		codonVec = aminoUtility::codonOf(j);
		for (int i=0; i<codonVec.size();i++)
			freqAminoData[j]+=codonFreqData[codonVec[i]];
		for (int c = 0; c<codonVec.size();c++)
		
	
	}

	
	
#ifdef VERBOS 
	MDOUBLE sum=0;
	for (i=0;i<alphabetSize();i++)
		sum+=_freqCodon[i];
	cout<<sum;
#endif;
}*/
/*
void jttCodonModel::checkModel(){
	MDOUBLE epsilon= 0.00000001;
	int i;
	for (i=0;i<alphabetSize();i++){
		for (int j=i+1;j<alphabetSize();j++){
			if (fabs(_freqCodon[i]*_Q[i][j] - _freqCodon[j]*_Q[j][i])>epsilon){
				cout<<"ERROR  "<<_freqCodon[i]*_Q[i][j]<<" " <<_freqCodon[j]*_Q[j][i]<<endl;
				exit(0);
			}
		
		}	
	}
	
	cout<<"(1) pi(i)*Q[i][j] == p(j)*Q[j][i] !"<<endl;

	for (i=0;i<alphabetSize();i++){
		MDOUBLE sum=0;
		for (int j=0;j<alphabetSize();j++){
			if (i!=j && _Q[i][j]<0){
				cout<<"ERROR -  negative value"<<endl;
				exit(0);
			}
			sum+=_Q[i][j];		
		
		}
		if (fabs(sum-0)>epsilon){
			cout<<i<<" sum "<< sum<<endl;
			cout<<"ERROR - sum of row differ then zero"<<endl;
			exit(0);
		}
	}
	
	cout<<"(2) sum of row in codon matrix is zero !"<<endl;
	
	for (i=0;i<_JTT.size(); i++){
		vector<int> codonOfI = aminoUtility::codonOf(i);
		LOG(5,<<endl);
		for (int j=0;j<_JTT.size(); j++){
			//if (j==i) continue;
			vector<int> codonOfJ = aminoUtility::codonOf(j);
			MDOUBLE sum=0;
			for (int c1 = 0; c1<codonOfI.size();c1++){
				for (int c2 = 0; c2<codonOfJ.size();c2++){
					sum+=_freqCodon[codonOfI[c1]]*_Q[codonOfI[c1]][codonOfJ[c2]];
				}
			
			}
			LOG(5,<<" "<<_JTT[i][j]);
			if (fabs(sum - _freqAmino[i]*_JTT[i][j])>epsilon){
				cout<<"ERROR - freqAmino[i]*_JTT[i][j] ! = sum(freqCodon[l]*Q[l][k]) "<<fabs(sum-_freqAmino[i]*_JTT[i][j])<<endl;
				cout<<i<<" "<<j<<endl;
				exit(0);
			}
			
			
		}
		
		
	}

	cout<<"(3) for each i differ from j: "<<endl<< "freqAmino[i]*_JTT[i][j] = sum(freqCodon[l]*Q[l][k]) -  where l codes for i and k codes for j"<<endl;

	
	for (i=0;i<_JTT.size();i++){
		MDOUBLE sum =0;
		for (int j=0; j<_JTT.size();j++){
			if (i==j) continue;
			sum-=_JTT[i][j];
		}
		if (fabs(_JTT[i][i]-sum)>epsilon){
			cout<<"ERROR - in the assumption of the modol -- sum of row of amino acid differs then zero"<<endl;
		
			exit(0);
		
		}
	
	}

	cout<<"(4) sum of row in amino (JTT) matrix is zero !"<<endl<<endl;



	
	for (i=0;i<_JTT.size();i++){
		MDOUBLE sum=0;
		for (int j=0;j<_JTT.size();j++){
			if (i==j) continue;
			sum-=_freqAmino[i]*_JTT[i][j];			
		}
		if (fabs((_freqAmino[i]*_JTT[i][i]- sum))>epsilon){
			cout<<"ERROR in phase 1"<<endl;
			exit(0);
		
		}
	
	}
	cout<<"phase 1 ok"<<endl;


	for (i=0;i<_JTT.size();i++){
		vector<int> codonOfI = aminoUtility::codonOf(i);
		MDOUBLE sum=0;
		for (int j=0;j<_JTT.size();j++){
			if (i==j) continue;
			vector<int> codonOfJ = aminoUtility::codonOf(j);
			for (int l = 0; l < codonOfI.size(); l++){
				int c1 = codonOfI[l];
				for (int m = 0; m<codonOfJ.size(); m++){
					int c2 = codonOfJ[m];
					sum-=freq(c1)*_Q[c1][c2];
				}
			}		
		
		}

		if (fabs((_freqAmino[i]*_JTT[i][i]- sum))>epsilon){
			cout<<"ERROR in phase 2"<<endl;
			exit(0);
		
		} 
	
	}


	cout<<"phase 2 ok"<<endl;


	for (i=0;i<_JTT.size();i++){
		vector<int> codonOfI = aminoUtility::codonOf(i);
		MDOUBLE res=0;
		for (int l = 0; l < codonOfI.size(); l++){
			MDOUBLE sum=0;
			for (int j=0;j<_JTT.size();j++){
				if (j==i) continue;
				vector<int> codonOfJ = aminoUtility::codonOf(j);
				for (int m = 0; m < codonOfJ.size(); m++){
					sum+=_Q[codonOfI[l]][codonOfJ[m]];
				}
			}
			res+=freq(codonOfI[l])*sum;
		
		}


		if (fabs((_freqAmino[i]*_JTT[i][i]+ res))>epsilon){
			cout<<"ERROR in phase 3"<<endl;
			exit(0);
		
		} 
	}
	
	

	cout<<"phase 3 ok"<<endl;


	for (i=0;i<_JTT.size();i++){
		vector<int> codonOfI = aminoUtility::codonOf(i);
		MDOUBLE res=0;
		for (int c1 = 0;c1<codonOfI.size();c1++){
			MDOUBLE sum = 0;
			for (int c2 = 0;c2<alphabetSize();c2++){
				if (i== codonUtility::aaOf(c2))
					continue;
				sum+=_Q[codonOfI[c1]][c2];
				
			}
			res+=freq(codonOfI[c1])*sum;
		
		}
		
		if (fabs((_freqAmino[i]*_JTT[i][i]+ res))>epsilon){
			cout<<"ERROR in phase 4"<<endl;
			exit(0);
		
		} 
		
	
	}
	cout<<"phase 4 ok"<<endl;

	for (i=0;i<_JTT.size();i++){
		vector<int> codonOfI = aminoUtility::codonOf(i);
		MDOUBLE res = 0;
		for (int j=0;j<codonOfI.size();j++){
			MDOUBLE rowSum=0;
			for (int c1 = 0; c1<alphabetSize();c1++){
				if (codonUtility::aaOf(c1) == i) continue;
				rowSum+=_Q[codonOfI[j]][c1];
			}
			res-=freq(codonOfI[j])*rowSum;
		}
			
		if ((res - _freqAmino[i]*_JTT[i][i])>epsilon){
				cout<<"ERROR ----- in the assumption of the model "<<res<< " " <<_freqAmino[i]*_JTT[i][i]<<endl;
				exit(0);
		}
		
	}
}
*/
void jttCodonModel::norm(MDOUBLE scale){
	
	for (int i=0; i < _Q.size(); ++i) {
		for (int j=0; j < _Q.size(); ++j) {
			_Q[i][j]*=scale;
		}
	}
	_q2pt.fillFromRateMatrix(_freqCodon,_Q);
}


MDOUBLE jttCodonModel::sumPijQij(){
	MDOUBLE sum=0.0;
	for (int i=0; i < _Q.size(); ++i) {
		sum -= _Q[i][i]*_freqCodon[i];
	}
	return sum;
}


