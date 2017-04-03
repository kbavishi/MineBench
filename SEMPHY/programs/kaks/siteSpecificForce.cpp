// 	$Id: siteSpecificForce.cpp 2399 2014-03-13 22:43:51Z wkliao $	
#include "siteSpecificForce.h"
#include "numRec.h"

MDOUBLE computeML_siteSpecificKaKs(Vdouble & KaKsV,
								   Vdouble & likelihoodsV,
								   const sequenceContainer& sc,
								   const stochasticProcess& sp,
								   const tree& et,
								   const MDOUBLE maxKaKs,//20.0
								   const MDOUBLE tol){//=0.0001f;

	KaKsV.resize(sc.seqLen());
	likelihoodsV.resize(sc.seqLen());
	MDOUBLE Lsum = 0.0;

	for (int pos=0; pos < sc.seqLen(); ++pos) {
		computeML_siteSpecificKaKs(pos,sc,sp,et,KaKsV[pos],likelihoodsV[pos],maxKaKs,tol);
		Lsum += likelihoodsV[pos];
		LOG(5,<<" KaKs of pos: "<<pos<<" = "<<KaKsV[pos]<<endl);
	}
	LOG(5,<<" number of sites: "<<sc.seqLen()<<endl);
	return Lsum;
}



void computeML_siteSpecificKaKs(int pos,
								 const sequenceContainer& sc,
								 const stochasticProcess& sp,
								 const tree &et,
								 MDOUBLE& bestKaKs,
								 MDOUBLE& posL,
								 const MDOUBLE maxKaKs,
								 const MDOUBLE tol) {
	LOG(5,<<".");
	MDOUBLE ax=0.0f,bx=maxKaKs*0.25,cx=maxKaKs;
	posL=-brent(ax,bx,cx,evaluate_L_given_w(sc,et,sp,pos),tol,&bestKaKs);
}


// THE BAYESIAN EB_EXP PART OF KAKS ESTIMATION. //

//this function compute kaks for a specific site can be used
//for the two model (yang or JTT)
//CHANGE
void computeEB_EXP_siteSpecificKaKs(int pos,
								 const sequenceContainer& sc,
								 const empiriSelectionModel & model,
								 const computePijGam& cpg,
								 const tree &et,
								 MDOUBLE& bestKaKs,
								 MDOUBLE & stdKaKs,
								 MDOUBLE & lowerConf,
								 MDOUBLE & upperConf,
								 const MDOUBLE alphaConf) {// alpha of 0.05 is considered 0.025 for each side.
	// here we compute P(w | data)
	
	Vdouble pGivenW(model.noOfCategor(),0.0);
	MDOUBLE sum=0;
	for (int i=0; i < model.noOfCategor(); ++i) {
		pGivenW[i] = likelihoodComputation::getLofPos(pos,et,sc,cpg[i],model.getCategor(i))*model.getCategorProb(i);		
		sum+=pGivenW[i];
	}
	assert(sum!=0);
	
	// here we compute sigma (ka/ks) * P(w | data)
	bestKaKs = 0.0;
	MDOUBLE Ex2 = 0.0; // this is the sum of squares.
	for (int j=0; j < model.noOfCategor(); ++j) {
		pGivenW[j]/=sum; // so that pGivenW is probability.
		LOG(5,<<"  "<<pGivenW[j]<<"  ");
		MDOUBLE tmp = pGivenW[j]*model.getKaKsCategor(j);//getSelection(j);
		bestKaKs += tmp;
		Ex2 += (tmp*model.getKaKsCategor(j));
	}

	MDOUBLE tmp2 = Ex2 - bestKaKs*bestKaKs;
	assert(tmp2>=0);
	stdKaKs = sqrt(tmp2);

	// detecting the confidence intervals.
	MDOUBLE oneSideConfAlpha = alphaConf/2.0; // because we are computing the two tail.
	MDOUBLE cdf = 0.0; // cumulative density function.
	int k=0;
	while (k < model.noOfCategor()){
		cdf += pGivenW[k];
		if (cdf >oneSideConfAlpha) {
			lowerConf = model.getKaKsCategor(k);//distr->rates(k);
			break;
		} 
		k++;
	}
	while (k < model.noOfCategor()) {
		if (cdf >(1.0-oneSideConfAlpha)) {
			upperConf = model.getKaKsCategor(k);//distr->rates(k);
			break;
		}
		++k;
		cdf += pGivenW[k];
	}
	if (k==model.noOfCategor()) upperConf = model.getKaKsCategor(k-1);
}

//this function compute kaks for each site for the JTT codon Model only.
void computeEB_EXP_siteSpecificKaKs(Vdouble & KaKsV,
								   Vdouble & stdV,
								   Vdouble & lowerBoundV,
								   Vdouble & upperBoundV,
								   const sequenceContainer& sc,
								   const empiriSelectionModel & model,	   
								   const tree& et,
								   const MDOUBLE alphaConf){
	int seqLen = sc.seqLen();
	KaKsV.resize(seqLen);
	stdV.resize(seqLen);
	lowerBoundV.resize(seqLen);
	upperBoundV.resize(seqLen);
	computePijGam cpg; 
	cpg._V.resize(model.noOfCategor());
	for (int i=0; i < model.noOfCategor(); ++i) {
		LOG(5,<<" "<< model.getSelection(i)<<" ");	
		cpg._V[i].fillPij(et,model.getCategor(i));
	}
	
	LOG(5,<<endl<<endl);
	MDOUBLE noSelectionKaKs = model.getKaKsNoSelection();
	cout<<"noSelectionKaKs= "<<noSelectionKaKs<<endl;
	LOG(5,<<"======================================================================="<<endl);
	LOG(5,<<"==========================RESULTS======================================"<<endl);
	LOG(5,<<"======================================================================="<<endl);
	for (int pos=0; pos < sc.seqLen(); ++pos) {
		LOG(5,<<pos<<" ");
		computeEB_EXP_siteSpecificKaKs(pos,sc,model,cpg,et,KaKsV[pos],stdV[pos],lowerBoundV[pos],upperBoundV[pos],alphaConf);
		KaKsV[pos] = (KaKsV[pos]*model.getF() + (1-model.getF())*noSelectionKaKs)/noSelectionKaKs;//????????
		lowerBoundV[pos] = (lowerBoundV[pos]*model.getF() + (1-model.getF())*noSelectionKaKs)/noSelectionKaKs;
		upperBoundV[pos] = (upperBoundV[pos]*model.getF() + (1-model.getF())*noSelectionKaKs)/noSelectionKaKs;
		LOG(5,<<" 		"<<KaKsV[pos]<<" [ "<<lowerBoundV[pos]<<" , "<<upperBoundV[pos]<<" ] "<<endl);
	}
	LOG(5,<<" number of sites: "<<sc.seqLen()<<endl);
}











