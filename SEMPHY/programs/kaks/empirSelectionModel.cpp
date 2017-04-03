// 	$Id: empirSelectionModel.cpp 2399 2014-03-13 22:43:51Z wkliao $	
#include "empirSelectionModel.h"

//this function create vector in size of the number categories of stochastic process  
empiriSelectionModel::empiriSelectionModel(Vdouble freqCodon,Vdouble freqAmino,codon coAlph,bool isGamma,int noOfCategor,
		MDOUBLE tr,MDOUBLE tv,MDOUBLE trtr,MDOUBLE tvtv,MDOUBLE trtv,MDOUBLE threeSub,MDOUBLE f,MDOUBLE alpha,MDOUBLE beta)
		:_freqCodon(freqCodon),_freqAmino(freqAmino),_tr(tr),_tv(tv),_trtr(trtr),_tvtv(tvtv),_trtv(trtv),_threeSub(threeSub),_f(f),_isGamma(isGamma),_codonAlph(coAlph){
	if (_isGamma) _forceDistr = new generalGammaDistribution(alpha,beta,noOfCategor);
	else _forceDistr = new betaOmegaDistribution(alpha,beta,noOfCategor,0.5,2);
	_spVec.resize(_forceDistr->categories());
	//here we create the selection model
	for (int categor=0; categor<_spVec.size();categor++){
		cout<<"w = "<<_forceDistr->rates(categor)<<endl;
		jttCodonModel * codonM = new jttCodonModel(_forceDistr->rates(categor),tr,tv,trtr,tvtv,trtv,threeSub,freqCodon,freqAmino,_codonAlph,0);
		distribution * dist = new uniDistribution();
		pijAccelerator* pij = new trivialAccelerator(codonM);
		stochasticProcess sp(dist,pij);
		_spVec[categor] = sp;
		delete dist;
		delete pij;
		delete codonM;
	}	
	
	//here we create the no selection model
	wYangModel * codonM = new wYangModel(_codonAlph,freqCodon,1.0,tr,tv,trtr,tvtv,trtv,threeSub,0);
	distribution * dist = new uniDistribution();
	pijAccelerator* pij = new trivialAccelerator(codonM);
	stochasticProcess selectionModel(dist,pij);
	delete dist;
	delete pij;
	delete codonM;
	_noSelectionModel = selectionModel;
		
	normalizeModel();
}
empiriSelectionModel& empiriSelectionModel::operator=(const empiriSelectionModel& other) {
	_freqCodon=other._freqCodon;
	_freqAmino=other._freqAmino;	
	_tr=other._tr;
	_tv=other._tv;
	_trtr=other._trtr;
	_tvtv=other._tvtv;
	_trtv=other._trtv;
	_threeSub=other._threeSub;
	_f=other._f;
	_forceDistr=other._forceDistr->clone();
	_spVec=other._spVec;
	_noSelectionModel=other._noSelectionModel;
	_KaKsSelection = other._KaKsSelection;
	_isGamma  = other._isGamma;
	_codonAlph = other._codonAlph; 


	return *this;
}
//copy constractor
empiriSelectionModel::empiriSelectionModel(const empiriSelectionModel& other):
_freqCodon(other._freqCodon),
_freqAmino(other._freqAmino),	
_tr(other._tr),
_tv(other._tv),
_trtr(other._trtr),
_tvtv(other._tvtv),
_trtv(other._trtv),
_threeSub(other._threeSub),
_f(other._f),
_forceDistr(other._forceDistr->clone()),
_spVec(other._spVec),
_noSelectionModel(other._noSelectionModel),
_KaKsSelection(other._KaKsSelection),
_isGamma(other._isGamma),
_codonAlph(other._codonAlph)

{
	
}
empiriSelectionModel::~empiriSelectionModel(){
	delete _forceDistr;
}

const MDOUBLE empiriSelectionModel::getOmega()const {
	if (!_isGamma) 
		return static_cast<betaOmegaDistribution*>(_forceDistr)->getOmega();
	else errorMsg::reportError("trying to call betaOmega distr while its gamma distr in getOmega func");
	return (-1);		// we shold/shall never get here
}
const MDOUBLE empiriSelectionModel::getBetaProb()const {
	if (!_isGamma)  return static_cast<betaOmegaDistribution*>(_forceDistr)->getBetaProb();
	else errorMsg::reportError("trying to call betaOmega distr while its gamma distr in getBetaProb func");
	return (-1);		// we shold/shall never get here
}

//here we normailze the matrices (noOfcategor + 1 matrices).
//TO CHECK IT!!!!!!!!
void empiriSelectionModel::normalizeModel(){
	MDOUBLE sumPijQij=0.0;
	for (int categor=0; categor<_forceDistr->categories();categor++)
		sumPijQij+=_forceDistr->ratesProb(categor)*static_cast<jttCodonModel*>(_spVec[categor].getPijAccelerator()->getReplacementModel())->sumPijQij();	
	sumPijQij*=_f;
	sumPijQij+=(1-_f)*static_cast<wYangModel*>(_noSelectionModel.getPijAccelerator()->getReplacementModel())->sumPijQij();	
	assert(sumPijQij!=0);
	for (int categor=0; categor<_forceDistr->categories();categor++)
		static_cast<jttCodonModel*>(_spVec[categor].getPijAccelerator()->getReplacementModel())->norm(1/sumPijQij);	
	static_cast<wYangModel*>(_noSelectionModel.getPijAccelerator()->getReplacementModel())->norm(1/sumPijQij);
}

/////////////////////////////////////////////////////////////////
//This part is for update the parameters and thus the matrices //
//(8 papameters = tr, tv,trtr,tvtv,trtv,alpha,beat,f)         //
////////////////////////////////////////////////////////////////
void empiriSelectionModel::updateF(MDOUBLE f){
	_f = f;
	for (int categor = 0; categor < _spVec.size();categor++)
		static_cast<jttCodonModel*>(_spVec[categor].getPijAccelerator()->getReplacementModel())->updateQ(); 
	static_cast<wYangModel*>(_noSelectionModel.getPijAccelerator()->getReplacementModel())->updateQ();
	normalizeModel();
}
void empiriSelectionModel::updateTr(MDOUBLE Tr){
	_tr = Tr;
	for (int categor = 0; categor < _spVec.size();categor++)
		static_cast<jttCodonModel*>(_spVec[categor].getPijAccelerator()->getReplacementModel())->setTr(Tr); 
	static_cast<wYangModel*>(_noSelectionModel.getPijAccelerator()->getReplacementModel())->setTr(Tr);
	normalizeModel();
}
void empiriSelectionModel::updateTv(MDOUBLE Tv){
	_tv=Tv;
	for (int categor = 0; categor < _spVec.size();categor++)
		static_cast<jttCodonModel*>(_spVec[categor].getPijAccelerator()->getReplacementModel())->setTv(Tv); 
	static_cast<wYangModel*>(_noSelectionModel.getPijAccelerator()->getReplacementModel())->setTv(Tv);
	normalizeModel();
}
void empiriSelectionModel::updateTrTr(MDOUBLE TrTr){
	_trtr = TrTr;
	for (int categor = 0; categor < _spVec.size();categor++)
		static_cast<jttCodonModel*>(_spVec[categor].getPijAccelerator()->getReplacementModel())->setTrTr(TrTr); 
	static_cast<wYangModel*>(_noSelectionModel.getPijAccelerator()->getReplacementModel())->setTrTr(TrTr);
	normalizeModel();
}
void empiriSelectionModel::updateTvTv(MDOUBLE TvTv){
	_tvtv = TvTv;
	for (int categor = 0; categor < _spVec.size();categor++)
		static_cast<jttCodonModel*>(_spVec[categor].getPijAccelerator()->getReplacementModel())->setTvTv(TvTv); 
	static_cast<wYangModel*>(_noSelectionModel.getPijAccelerator()->getReplacementModel())->setTvTv(TvTv);
	normalizeModel();
}
void empiriSelectionModel:: updateTrTv(MDOUBLE TrTv){
	_trtv = TrTv;
	for (int categor = 0; categor < _spVec.size();categor++)
		static_cast<jttCodonModel*>(_spVec[categor].getPijAccelerator()->getReplacementModel())->setTrTv(TrTv); 
	static_cast<wYangModel*>(_noSelectionModel.getPijAccelerator()->getReplacementModel())->setTrTv(TrTv);
	normalizeModel();
}

void empiriSelectionModel:: updateThreeSub(MDOUBLE threeSub){
	_threeSub = threeSub;
	for (int categor = 0; categor < _spVec.size();categor++)
		static_cast<jttCodonModel*>(_spVec[categor].getPijAccelerator()->getReplacementModel())->setThreeSub(threeSub); 
	static_cast<wYangModel*>(_noSelectionModel.getPijAccelerator()->getReplacementModel())->setThreeSub(threeSub);
	normalizeModel();
}
void empiriSelectionModel::updateAlpha(MDOUBLE alpha){
	if (_isGamma) static_cast<generalGammaDistribution*>(_forceDistr)->setAlpha(alpha);
	else static_cast<betaOmegaDistribution*>(_forceDistr)->setAlpha(alpha);
	for (int categor = 0; categor < _spVec.size();categor++)
		static_cast<jttCodonModel*>(_spVec[categor].getPijAccelerator()->getReplacementModel())->setW(_forceDistr->rates(categor)); 
	static_cast<wYangModel*>(_noSelectionModel.getPijAccelerator()->getReplacementModel())->updateQ(); //for returning the matrix before scaling
	normalizeModel();
}
void empiriSelectionModel::updateBeta(MDOUBLE beta){
	if (_isGamma) static_cast<generalGammaDistribution*>(_forceDistr)->setBeta(beta);
	else static_cast<betaOmegaDistribution*>(_forceDistr)->setBeta(beta);
	for (int categor = 0; categor < _spVec.size();categor++)
		static_cast<jttCodonModel*>(_spVec[categor].getPijAccelerator()->getReplacementModel())->setW(_forceDistr->rates(categor)); 
	static_cast<wYangModel*>(_noSelectionModel.getPijAccelerator()->getReplacementModel())->updateQ(); //for returning the matrix before scaling
	normalizeModel();
}


void empiriSelectionModel::updateOmega(MDOUBLE omega){
	int size = _spVec.size();
	static_cast<betaOmegaDistribution*>(_forceDistr)->setOmega(omega);
	static_cast<jttCodonModel*>(_spVec[size-1].getPijAccelerator()->getReplacementModel())->setW(omega); 
	for (int categor = 0; categor < _spVec.size()-1;categor++)
		static_cast<jttCodonModel*>(_spVec[categor].getPijAccelerator()->getReplacementModel())->updateQ(); 
	static_cast<wYangModel*>(_noSelectionModel.getPijAccelerator()->getReplacementModel())->updateQ(); //for returning the matrix before scaling
	
	normalizeModel();
}

void empiriSelectionModel::updateBetaProb(MDOUBLE betaProb){
	static_cast<betaOmegaDistribution*>(_forceDistr)->setBetaProb(betaProb);
	for (int categor = 0; categor < _spVec.size();categor++)
		static_cast<jttCodonModel*>(_spVec[categor].getPijAccelerator()->getReplacementModel())->updateQ(); 
	static_cast<wYangModel*>(_noSelectionModel.getPijAccelerator()->getReplacementModel())->updateQ(); //for returning the matrix before scaling
	
	normalizeModel();
}

const MDOUBLE empiriSelectionModel::getKaKsNoSelection()const {

	MDOUBLE nonsynonymous=0.0,synonymous=0.0;
	wYangModel *pmodel = static_cast<wYangModel*>(_noSelectionModel.getPijAccelerator()->getReplacementModel());
	for (int i=0;i<pmodel->alphabetSize();i++){
		for(int j=0;j<pmodel->alphabetSize();j++){
			if (codonUtility::aaOf(i,_codonAlph) != codonUtility::aaOf(j,_codonAlph)){  //nonsynonymous
				nonsynonymous += pmodel->freq(i) * pmodel->getQij(i,j);				
			}else{ //synonymous
				if (i==j)
					continue;
				synonymous += pmodel->freq(i) * pmodel->getQij(i,j);				
			}
		}		
	}
	cout<<"no selection kaks"<<endl;
	cout<<" nonsynonymous= "<< nonsynonymous<<" synonymous= "<<synonymous<<endl;
	cout<<"the ratio is= "<<nonsynonymous/synonymous<<endl;
	return (nonsynonymous/synonymous);
}


void empiriSelectionModel::fillKaKsSelection(){
	_KaKsSelection.resize(noOfCategor());
	for(int categor=0;categor<noOfCategor();categor++){
		MDOUBLE nonsynonymous=0.0,synonymous=0.0;
		jttCodonModel *pmodel = static_cast<jttCodonModel*>(_spVec[categor].getPijAccelerator()->getReplacementModel());
		for (int i=0;i<pmodel->alphabetSize();i++){
			for(int j=0;j<pmodel->alphabetSize();j++){
				if (codonUtility::aaOf(i,_codonAlph) != codonUtility::aaOf(j,_codonAlph)){  //nonsynonymous
					nonsynonymous += pmodel->freq(i) * pmodel->getQij(i,j);				
				}else{ //synonymous
					if (i==j)continue;
					synonymous += pmodel->freq(i) * pmodel->getQij(i,j);				
				}
			}		
		}
		cout<<" nonsynonymous= "<< nonsynonymous<<" synonymous= "<<synonymous;
		cout<<" the ratio is= "<<nonsynonymous/synonymous<<endl;
		_KaKsSelection[categor] = nonsynonymous/synonymous;
		
	}
}



//for test only/////
void empiriSelectionModel::updateW(MDOUBLE W){
	LOG(5,<<"updating w = "<<W<<endl); 
	static_cast<jttCodonModel*>(_spVec[0].getPijAccelerator()->getReplacementModel())->setW(W); 
	static_cast<wYangModel*>(_noSelectionModel.getPijAccelerator()->getReplacementModel())->updateQ(); //for returning the matrix before scaling
	normalizeModel();
}
