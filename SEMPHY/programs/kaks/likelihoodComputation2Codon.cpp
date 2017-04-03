// 	$Id: likelihoodComputation2Codon.cpp 2399 2014-03-13 22:43:51Z wkliao $	
#include "likelihoodComputation2Codon.h"

#include "JTTcodonModel.h"
#include "wYangModel.h"
#include "definitions.h"
#include "tree.h"
#include "computeUpAlg.h"
#include "likelihoodComputation.h"
#include <cmath>
#include <cassert>

using namespace likelihoodComputation2Codon;


//compute likelihood for the selection and no-selection part.
//assuming distribution on the selection part
MDOUBLE likelihoodComputation2Codon::getTreeLikelihoodAllPosAlphTheSame(const tree& et,
							const sequenceContainer& sc,
							const empiriSelectionModel & model){
							//const vector<stochasticProcess>& spVec,const distribution * distr){
	computePijGam pi;
	distribution *distr = model.getDistr();
	int noOfCategor = distr->categories();
	pi._V.resize(noOfCategor);
	for (int i=0; i < noOfCategor; ++i) {
		pi._V[i].fillPij(et,model.getCategor(i));
	}
	
	suffStatGlobalGam ssc;
	computeUpAlg cup;
	cup.fillComputeUp(et,sc,pi,ssc);

	computePijHom pi2;
	pi2.fillPij(et,model.getNoSelectionModel());
	
	MDOUBLE res = 0.0;
	int k;
	MDOUBLE f = model.getF();
	for (k=0; k < sc.seqLen(); ++k) {
		MDOUBLE likeSelection = likelihoodComputation2Codon::getProbOfPosUpIsFilledSelectionGam(k,//pos,
			  et,//const tree& 
			  sc,// sequenceContainer& sc,
			  model.getCategor(0),
			  ssc[k],//const computePijGam& ,
			  distr);//W distribution ,
		
		MDOUBLE likeNoSelection = likelihoodComputation::getLofPos(k,et,sc,pi2,model.getNoSelectionModel());
		
		MDOUBLE lnL = log(likeSelection*f+(1-f)*likeNoSelection); 
		//LOG(5,<<"pos= "<<k<<" lnL= "<<lnL<<endl);
		res += lnL;
		//if (k==20) exit(0);
			  
	}
	return res;
	


}

//only for the selection part
MDOUBLE likelihoodComputation2Codon::getProbOfPosUpIsFilledSelectionGam(const int pos,const tree& et,
						const sequenceContainer& sc,
						const stochasticProcess& sp,
						const suffStatGlobalGamPos& cup,const distribution * distr){

	MDOUBLE tmp=0.0;
	for (int categor = 0; categor < distr->categories(); ++categor) {
		MDOUBLE veryTmp =0;
		for (int let =0; let < sc.alphabetSize(); ++let) {
			veryTmp+=cup.get(categor,et.getRoot()->id(),let) * sp.freq(let);
			
		}
		//cout<<"category= "<<categor<<" fh= "<<veryTmp<<" freqCategor= "<<distr->ratesProb(categor)<<endl;
		tmp += veryTmp*distr->ratesProb(categor);
	}
	
	
	if(tmp<0.0) errorMsg::reportError("like< 0 in likelihoodComputation2Codon::getProbOfPosUpIsFilledSelectionGam");
	return tmp;
}

MDOUBLE likelihoodComputation2Codon::getTreeLikelihoodFromUp2(const tree& et,
						const sequenceContainer& sc,
						const empiriSelectionModel & model,
						const suffStatGlobalGam& cup,
						Vdouble& posLike, // fill this vector with each position likelihood but without the weights.
						const Vdouble * weights) {
	posLike.clear();
	MDOUBLE like = 0;
	MDOUBLE f = model.getF();
	//computing the likelihood from up:
	for (int pos = 0; pos < sc.seqLen(); ++pos) {
		MDOUBLE tmp=0;
		MDOUBLE tmp2 = 0; //like for the non selection part
		for (int let =0; let < sc.alphabetSize(); ++let) {
				tmp2+=cup.get(pos,model.noOfCategor(),et.getRoot()->id(),let) * model.freq(let);
		}
		for (int categor = 0; categor < model.noOfCategor(); ++categor) {
			MDOUBLE veryTmp =0;
			for (int let =0; let < sc.alphabetSize(); ++let) {
				veryTmp+=cup.get(pos,categor,et.getRoot()->id(),let) * model.freq(let);
			}
			tmp += veryTmp*model.getCategorProb(categor);
		}

		if(tmp<0.0) errorMsg::reportError("like< 0 in likelihoodComputation2Codon::getTreeLikelihoodFromUp2");

		like += log(f*tmp+(1-f)*tmp2) * (weights?(*weights)[pos]:1);
		posLike.push_back(f*tmp+(1-f)*tmp2);
	}
	return like;

}
