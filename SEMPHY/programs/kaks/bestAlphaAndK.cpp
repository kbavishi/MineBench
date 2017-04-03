// 	$Id: bestAlphaAndK.cpp 2399 2014-03-13 22:43:51Z wkliao $	
#include "bestAlphaAndK.h"
#include "computePijComponent.cpp"
#include "utils.h"

bestAlphaAndKsAndBBL::bestAlphaAndKsAndBBL(tree& et, //find Best Alpha and best BBL
					   const sequenceContainer& sc,
					   empiriSelectionModel & model,
					   const MDOUBLE upperBoundOnAlpha,
					   const MDOUBLE upperBoundOnBeta,
					   const MDOUBLE epsilonFOptimization,
					   const MDOUBLE epsilon,
					   const int maxIterations){
   //initialization	
	MDOUBLE epsilonAlphaOptimization=epsilon;
	MDOUBLE epsilonKOptimization= epsilon;
    MDOUBLE epsilonLikelihoodImprovment=epsilon;
    //	int maxBBLIterations= maxIterations;
	int maxTotalIterations= maxIterations;
	MDOUBLE lowerValueOfParamK = 0;
	MDOUBLE lowerValueOfParamAlpha = 0.1;
	MDOUBLE lowerValueOfParamBeta = 0.1;
	MDOUBLE lowerValueOfParamF = 0.0;
	MDOUBLE upperValueOfParamK = 25;
	MDOUBLE upperValueOfParamF = 1.0;
	MDOUBLE initialGuessValueOfParamTr;
	MDOUBLE initialGuessValueOfParamTv ;
	MDOUBLE initialGuessValueOfParamTrTv;
	MDOUBLE initialGuessValueOfParamTrTr;
	MDOUBLE initialGuessValueOfParamTvTv;
	MDOUBLE initialGuessValueOfParamAlpha; // UPDATE!
	MDOUBLE initialGuessValueOfParamBeta; // UPDATE!
	MDOUBLE initialGuessValueOfParamF;
	MDOUBLE initialGuessValueOfParamThreeSub;

	_bestL = likelihoodComputation2Codon::getTreeLikelihoodAllPosAlphTheSame(et,sc,model);//_VERYSMALL;
	MDOUBLE newL = _bestL;
	MDOUBLE alphaFound ,trFound,tvFound,trtvFound,trtrFound,tvtvFound,betaFound,fFound,threeSubFound;
	bool changes = false;
	int i=0;

	_bestTr = model.getTr();
	_bestTv = model.getTv();
	_bestTrTr = model.getTrTr();
	_bestTrTv = model.getTrTv();
	_bestTvTv = model.getTvTv();
	_bestBeta = model.getBeta();
	_bestAlpha = model.getAlpha();
	_bestF = model.getF();
	_bestThreeSub = model.getThreeSub();
	for (i=0; i < maxTotalIterations; ++i) {
		LOG(5,<<"Iteration Number= " << i <<endl);
		LOG(5,<<"---------------------"<<endl);
		changes = false;
		trFound = alphaFound= tvFound = trtvFound = trtrFound = tvtvFound = threeSubFound =  betaFound = fFound=0;
//ALPHA
		initialGuessValueOfParamAlpha = model.getAlpha();		
		newL = -brent(lowerValueOfParamAlpha,
					initialGuessValueOfParamAlpha,
					upperBoundOnAlpha,
					evalAlphaOrKs(et,sc,model,-1),epsilonAlphaOptimization,&alphaFound); 
		

		LOG(5,<<"current best L= "<<_bestL<<endl<<endl);
		LOG(5,<<"new L After alpha= " << newL<<endl);
		LOG(5,<<"new alpha = " <<alphaFound<<endl<<endl);
	
		if (newL > _bestL+epsilonLikelihoodImprovment ) {// update of likelihood ,v and model.
			_bestL = newL;
			_bestAlpha = alphaFound;
			model.updateAlpha(alphaFound);
			changes = true;
		}
		LOG(5,<<"the paramets are = "<< model.getAlpha()<<" "<<model.getBeta()<<" "<<model.getTr()<<" "<<model.getTv()<<" "<<model.getTrTr()<<" "<<model.getTvTv()<<" "<<model.getTrTv()<<" "<<model.getThreeSub()<<" "<<model.getF()<<endl);
//Beta
		initialGuessValueOfParamBeta = model.getBeta();
		newL = -brent(lowerValueOfParamBeta,
					initialGuessValueOfParamBeta,
					upperBoundOnBeta,
					evalAlphaOrKs(et,sc,model,-2),epsilonAlphaOptimization,&betaFound); 


		LOG(5,<<"current best L= "<<_bestL<<endl<<endl);
		LOG(5,<<"new L After beta= " << newL<<endl);
		LOG(5,<<"new beta = " <<betaFound<<endl<<endl);
		
		if (newL > _bestL+epsilonLikelihoodImprovment ) {// update of likelihood ,v and model.
			_bestL = newL;
			_bestBeta = betaFound;
			model.updateBeta(betaFound);
			changes = true;
		}
//omega+ probBeta for beta distribution.
		if (!model.getIsGamma()){ //betaOmega distribution

			MDOUBLE initialGuessValueOfParamOmega = model.getOmega();
			cout<<"init omage  = "<<initialGuessValueOfParamOmega<<endl;
			MDOUBLE omegaFound;	
			newL = -brent(1.0,
					initialGuessValueOfParamOmega,
					5.0,
					evalAlphaOrKs(et,sc,model,6),0.01,&omegaFound); 

			LOG(5,<<"current best L= "<<_bestL<<endl<<endl);
			LOG(5,<<"new L After omega= " << newL<<endl);
			LOG(5,<<"new omega = " <<omegaFound<<endl<<endl);
	
			if (newL > _bestL+epsilonLikelihoodImprovment ) {// update of likelihood ,v and model.
				_bestL = newL;
				_bestOmega = omegaFound;
				model.updateOmega(omegaFound);
				changes = true;
			}

			MDOUBLE initialGuessValueOfParamBetaProb = model.getBetaProb();
			MDOUBLE betaProbFound;	
			newL = -brent(0.0,initialGuessValueOfParamBetaProb,1.0,
					evalAlphaOrKs(et,sc,model,7),0.01,&betaProbFound); 

			LOG(5,<<"current best L= "<<_bestL<<endl<<endl);
			LOG(5,<<"new L After betaprob= " << newL<<endl);
			LOG(5,<<"new betaProb = " <<betaProbFound<<endl<<endl);
	
			if (newL > _bestL+epsilonLikelihoodImprovment ) {// update of likelihood ,v and model.
				_bestL = newL;
				_bestBetaProb = betaProbFound;
				model.updateBetaProb(betaProbFound);
				changes = true;
			}

		}
//Tr parameter
		LOG(5,<<"the paramets are = "<< model.getAlpha()<<" "<<model.getBeta()<<" "<<model.getTr()<<" "<<model.getTv()<<" "<<model.getTrTr()<<" "<<model.getTvTv()<<" "<<model.getTrTv()<<" "<<model.getThreeSub()<<" "<<model.getF()<<endl);
		initialGuessValueOfParamTr = model.getTr();
		newL = -brent(lowerValueOfParamK,   //optimaize Tr
				initialGuessValueOfParamTr,
				upperValueOfParamK,
				evalAlphaOrKs(et,sc,model,0),epsilonKOptimization,&trFound); 
		
		LOG(5,<<"current best L= "<<_bestL<<endl<<endl);
		LOG(5,<<"new L After Tr= " << newL<<endl);
		LOG(5,<<"new tr = " <<trFound<<endl<<endl);

		if (newL > _bestL+epsilonLikelihoodImprovment ) {// update of likelihood and model.
			_bestL = newL;
			_bestTr = trFound;
			model.updateTr(trFound);
			changes = true;
		}
//Tv parameter
		LOG(5,<<"the paramets are = "<< model.getAlpha()<<" "<<model.getBeta()<<" "<<model.getTr()<<" "<<model.getTv()<<" "<<model.getTrTr()<<" "<<model.getTvTv()<<" "<<model.getTrTv()<<" "<<model.getThreeSub()<<" "<<model.getF()<<endl);
		initialGuessValueOfParamTv = model.getTv();
		newL = -brent(lowerValueOfParamK,   //optimaize Tv
				initialGuessValueOfParamTv,
				upperValueOfParamK,
				evalAlphaOrKs(et,sc,model,1),epsilonKOptimization,&tvFound); 
		
		LOG(5,<<"current best L= "<<_bestL<<endl<<endl);
		LOG(5,<<"new L After Tv = " << newL<<endl);
		LOG(5,<<"new Tv = " <<tvFound<<endl<<endl);
	
		if (newL > _bestL+epsilonLikelihoodImprovment ) {// update of likelihood and model.
			_bestL = newL;
			_bestTv = tvFound;
			model.updateTv(tvFound);
			changes = true;
		}
//TrTr Parameter
		LOG(5,<<"the paramets are = "<< model.getAlpha()<<" "<<model.getBeta()<<" "<<model.getTr()<<" "<<model.getTv()<<" "<<model.getTrTr()<<" "<<model.getTvTv()<<" "<<model.getTrTv()<<" "<<model.getThreeSub()<<" "<<model.getF()<<endl);
		initialGuessValueOfParamTrTr = model.getTrTr();
		newL = -brent(lowerValueOfParamK,  
				initialGuessValueOfParamTrTr,
				upperValueOfParamK,
				evalAlphaOrKs(et,sc,model,2),epsilonKOptimization,&trtrFound); 
		
		LOG(5,<<"current best L= "<<_bestL<<endl<<endl);
		LOG(5,<<"new L After TrTr= " << newL<<endl);
		LOG(5,<<"new TrTr = " <<trtrFound<<endl<<endl);

		if (newL > _bestL+epsilonLikelihoodImprovment ) {// update of likelihood and model.
			_bestL = newL;
			_bestTrTr = trtrFound;
			model.updateTrTr(trtrFound);
			changes = true;
		}
//TrTv Parameter
		LOG(5,<<"the paramets are = "<< model.getAlpha()<<" "<<model.getBeta()<<" "<<model.getTr()<<" "<<model.getTv()<<" "<<model.getTrTr()<<" "<<model.getTvTv()<<" "<<model.getTrTv()<<" "<<model.getThreeSub()<<" "<<model.getF()<<endl);
		initialGuessValueOfParamTrTv =model.getTrTv();
		newL = -brent(lowerValueOfParamK,   
				initialGuessValueOfParamTrTv,
				upperValueOfParamK,
				evalAlphaOrKs(et,sc,model,3),epsilonKOptimization,&trtvFound); 
		
		LOG(5,<<"current best L= "<<_bestL<<endl<<endl);
		LOG(5,<<"new L After TrTv= " << newL<<endl);
		LOG(5,<<"new TrTv = " <<trtvFound<<endl<<endl);

		if (newL > _bestL+epsilonLikelihoodImprovment ) {// update of likelihood and model.
			_bestL = newL;
			_bestTrTv = trtvFound;
			model.updateTrTv(trtvFound);
			changes = true;
		}
//TvTv Parameter
		LOG(5,<<"the paramets are = "<< model.getAlpha()<<" "<<model.getBeta()<<" "<<model.getTr()<<" "<<model.getTv()<<" "<<model.getTrTr()<<" "<<model.getTvTv()<<" "<<model.getTrTv()<<" "<<model.getThreeSub()<<" "<<model.getF()<<endl);
		initialGuessValueOfParamTvTv = model.getTvTv();
		newL = -brent(lowerValueOfParamK,   
				initialGuessValueOfParamTvTv,
				upperValueOfParamK,
				evalAlphaOrKs(et,sc,model,4),epsilonKOptimization,&tvtvFound); 
		
		LOG(5,<<"current best L= "<<_bestL<<endl<<endl);
		LOG(5,<<"new L After TvTv= " << newL<<endl);
		LOG(5,<<"new TvTv = " <<tvtvFound<<endl<<endl);
		

		if (newL > _bestL+epsilonLikelihoodImprovment ) {// update of likelihood and model.
			_bestL = newL;
			_bestTvTv = tvtvFound;
			model.updateTvTv(tvtvFound);
			changes = true;
		}
//threeSub
		LOG(5,<<"the paramets are = "<< model.getAlpha()<<" "<<model.getBeta()<<" "<<model.getTr()<<" "<<model.getTv()<<" "<<model.getTrTr()<<" "<<model.getTvTv()<<" "<<model.getTrTv()<<" "<<model.getThreeSub()<<" "<<model.getF()<<endl);
		initialGuessValueOfParamThreeSub = model.getThreeSub();
		newL = -brent(lowerValueOfParamK,   
				initialGuessValueOfParamThreeSub,
				upperValueOfParamK,
				evalAlphaOrKs(et,sc,model,5),epsilonKOptimization,&threeSubFound); 
		
		LOG(5,<<"current best L= "<<_bestL<<endl<<endl);
		LOG(5,<<"new L After threeSub= " << newL<<endl);
		LOG(5,<<"new threeSub = " <<threeSubFound<<endl<<endl);
		

		if (newL > _bestL+epsilonLikelihoodImprovment ) {// update of likelihood and model.
			_bestL = newL;
			_bestThreeSub= threeSubFound;
			model.updateThreeSub(threeSubFound);
			changes = true;
		}

//F
		LOG(5,<<"the paramets are = "<< model.getAlpha()<<" "<<model.getBeta()<<" "<<model.getTr()<<" "<<model.getTv()<<" "<<model.getTrTr()<<" "<<model.getTvTv()<<" "<<model.getTrTv()<<" "<<model.getThreeSub()<<" "<<model.getF()<<endl);
		initialGuessValueOfParamF = model.getF();
		newL = -brent(lowerValueOfParamF,   
				initialGuessValueOfParamF,
				upperValueOfParamF,
				evalAlphaOrKs(et,sc,model,-3),epsilonFOptimization,&fFound); 
		
		LOG(5,<<"current best L= "<<_bestL<<endl<<endl);
		LOG(5,<<"new L After F= " << newL<<endl);
		LOG(5,<<"new F = " <<fFound<<endl<<endl);

		if (newL > _bestL+epsilonLikelihoodImprovment ) {// update of likelihood and model.
			_bestL = newL;
			_bestF = fFound;
			model.updateF(fFound);
			changes = true;
		}

//BBL
		cout<<"the like before = "<<likelihoodComputation2Codon::getTreeLikelihoodAllPosAlphTheSame(et,sc,model);
		
		bblEM2codon bbl(et,sc,model,NULL,maxTotalIterations);
		newL = bbl.getTreeLikelihood();

		LOG(5,<<"current best L= "<<_bestL<<endl);
		LOG(5,<<"new L After BL = " << newL<< " = "<<likelihoodComputation2Codon::getTreeLikelihoodAllPosAlphTheSame(et,sc,model)<<endl);

		if (newL > _bestL+epsilonLikelihoodImprovment) {
			_bestL = newL;
			changes = true;
		}
		if (changes == false) break;
		
	}
	
	if (i==maxTotalIterations) {
		//errorMsg::reportError("Too many iterations in function optimizeCodonModelAndBBL");
		LOG(5,<<"Too many iterations in function optimizeCodonModelAndBBL"<<endl);
	}
	
	
}


MDOUBLE evalAlphaOrKs::operator()(MDOUBLE param){
	if (_alphaOrKs==-1) _model.updateAlpha(param);
	else if (_alphaOrKs==-2) _model.updateBeta(param);
	else if (_alphaOrKs==-3) _model.updateF(param);
	else if (_alphaOrKs==0) _model.updateTr(param);
	else if (_alphaOrKs==1) _model.updateTv(param);
	else if (_alphaOrKs==2) _model.updateTrTr(param);
	else if (_alphaOrKs==3) _model.updateTrTv(param);
	else if (_alphaOrKs==4) _model.updateTvTv(param);
	else if (_alphaOrKs==5) _model.updateThreeSub(param);
	else if (_alphaOrKs==6) _model.updateOmega(param);
	else if (_alphaOrKs==7) _model.updateBetaProb(param);
	MDOUBLE res = likelihoodComputation2Codon::getTreeLikelihoodAllPosAlphTheSame(_et,_sc,_model);
	return -res;	//return -log(likelihood).
}
