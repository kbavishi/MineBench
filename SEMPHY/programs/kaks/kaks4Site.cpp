// 	$Id: kaks4Site.cpp 2399 2014-03-13 22:43:51Z wkliao $	
#include "kaks4Site.h"

using namespace std;

kaks4Site::kaks4Site(int argc, char* argv[]) {
	printInfo(cout);
	fillOptionsParameters(argc,argv);
	myLog::setLog(_pOptions->_logFile, 5);
	printOptionParameters();
	initCodonScAndTree();
	createModel();
	cout<<"after create model"<<endl;
	computeAlphaAndKsAndBBL();
	cout<<"After param optimaization"<<endl;	
	computeEB_EXPKaKs4Site();
	cout<<"after kaks4site"<<endl;
	myLog::endLog();
}
kaks4Site::~kaks4Site() {
	delete _pOptions;
	delete _codonAlph;
}

void kaks4Site::printInfo(ostream& out) {
	out<<endl;
	out<<" ======================================================="<<endl;
	out<<" KaKs4Site:				                              "<<endl;
	out<<" Version: 2.00. Last updated 2.02.06                   "<<endl;
	out<<" Adi Doron and Tal Pupko                                "<<endl;
	out<<" For program support, please contact Adi Doron          "<<endl;
	out<<" Adi Doron:  doronadi@post.tau.ac.il                    "<<endl;
	out<<" ======================================================="<<endl;
	out<<endl;
}

void kaks4Site::fillOptionsParameters(int argc, char* argv[]) {
	if (argc == 1 ) {	
		printHelp();
		exit(0); // here the -h option will be printed
	}
	_pOptions = new kaksOptions(argc, argv);
	if (_pOptions->_inCodonSeqFile.size() == 0)	{
		errorMsg::reportError("the input codon file is missing");
	}
}


void kaks4Site::printOptionParameters() {
	cout<<"\n ---------------------- THE PARAMETERS ----------------------------"<<endl;
	cout<<"input Codon file is:            "<< _pOptions->_inCodonSeqFile <<endl;
	cout<<"input Tree file is:             "<< _pOptions->_inTreeFile <<endl;
	cout<<"input distribution:             "<< _pOptions->_isGamma<<endl;
	cout<<"output Log file is:             "<< _pOptions->_logFile        <<endl;	
	cout<<"output Tree file is:            "<< _pOptions->_outTreeFile <<endl;
	cout<<"output info file is:            "<< _pOptions->_outRes4SiteFile <<endl;
	cout<<"output rastop script is:        "<< _pOptions->_outRasmolFile <<endl;
	cout<<"output global result is:        "<< _pOptions->_outGlobalRes <<endl;
	cout<<"output bin numbers for site is: "<< _pOptions->_outSelectionFile <<endl;
}


//this function read the codon file that must be aligned 
//and read a tree file if is given or create NJ tree according the codon sequence 
//with a simple codon model (Yang where w = 1, k = 1). 
void kaks4Site::initCodonScAndTree(){
	switch (_pOptions->_geneticCode) {
		case 0 : _codonAlph = new codon(geneticCodeHolder::nuclearStandard);  break;
		case 1 : _codonAlph = new codon(geneticCodeHolder::mitochondriaVertebrate); break;
		case 2 : _codonAlph = new codon(geneticCodeHolder::mitochondriaAscidian); break; 
		case 3 : _codonAlph = new codon(geneticCodeHolder::mitochondriaEchinoderm); break; 
		case 4 : _codonAlph = new codon(geneticCodeHolder::mitochondriaFlatworm); break; 
		case 5 : _codonAlph = new codon(geneticCodeHolder::mitochondriaInvertebrate); break; 
		case 6 : _codonAlph = new codon(geneticCodeHolder::mitochondriaProtozoan); break; 
		case 7 : _codonAlph = new codon(geneticCodeHolder::mitochondriaYeast); break; 
		case 8 : _codonAlph = new codon(geneticCodeHolder::nuclearBlepharisma); break; 
		case 9 : _codonAlph = new codon(geneticCodeHolder::nuclearCiliate); break; 
		case 10 : _codonAlph = new codon(geneticCodeHolder::nuclearEuplotid); break; 
	
	}
	_codonSc = readCodonSeqs(_pOptions->_inCodonSeqFile,_codonAlph);  //fill _codonSc
	if (_pOptions->_inTreeFile.size()!=0){ //here checking if there is a user tree
		LOG(5,<<"-------------------------"<<endl);
		LOG(5,<<"    reading user tree "<<endl<<endl;); 
		tree et(_pOptions->_inTreeFile);
		_t = et;
		return;
	}
	createTree();
}


//this function create NJ tree according the aligned codon sequence 
//with a simple codon model (Yang model where w = 1, k = 1). 
void kaks4Site::createTree(){

	LOG(5,<<"-------------------------"<<endl);
	LOG(5,<<"    creating NJ tree "<<endl<<endl;); 
	VVdouble distM;
	vector<string> names;
	names.resize(_codonSc.numberOfSeqs());
	distM.resize(_codonSc.numberOfSeqs());
	int i;
	for (i=0;i<distM.size();i++){
		distM[i].resize(distM.size());
		names[i]=(_codonSc[i].name());		
	}	
	//creating stochastic process for computing the likelihood distances
	//the process based on simple codon model (w = 1 i.e. no selection and  k=1)
	//Vdouble pi = evaluateCharacterFreq(_codonSc);
	Vdouble pi = copmuteFreq();
	replacementModel * codonM = new wYangModel(*_codonAlph,pi);	
	distribution * dist = new uniDistribution();
	pijAccelerator* pij = new trivialAccelerator(codonM);
	stochasticProcess sp(dist,pij);
	delete dist;
	delete pij;
	delete codonM;
	likeDist distr(sp);
	for ( i=0;i<_codonSc.numberOfSeqs();i++){
		for(int j=i+1;j<_codonSc.numberOfSeqs();j++){
			distM[i][j]= distr.giveDistance(_codonSc[i],_codonSc[j],NULL);
		}
	}

	NJalg nj;	
	_t = nj.computeTree(distM,names);

}


Vdouble kaks4Site::copmuteFreq(){
	Vdouble pi;
	nucleotide alph;
	sequenceContainer nucSc;
	ifstream in(_pOptions->_inCodonSeqFile.c_str());
	nucSc = recognizeFormat::readUnAligned(in, &alph);
	nucSc.changeGaps2MissingData();
	in.close();
	pi = freqCodonF3x4(nucSc,*_codonAlph);
	makeSureNoZeroFreqs(pi);
	return pi;
}
void kaks4Site::createModel(){
	Vdouble pi = copmuteFreq();
	Vdouble aminofreq = fromFreqCodonToFreqAmino(pi,*_codonAlph);
	empiriSelectionModel model(pi,aminofreq,*_codonAlph,_pOptions->_isGamma);
	_model = model;
	_model.getKaKsNoSelection();
}




//this function optimaize the codon model parameters: k's, alpha and BBL
void kaks4Site::computeAlphaAndKsAndBBL(){
	//evaluate best global k and bl and Alpha
	bestAlphaAndKsAndBBL bestParams(_t,_codonSc,_model);
	_t.output(_pOptions->_outTreeFile);
	ofstream out(_pOptions->_outGlobalRes.c_str());
	out<<"The Best likelihood: "<<bestParams.getBestL()<<endl<<
		" Best tr: "<<bestParams.getBestTr()<<endl<<
		" Best tv: "<<bestParams.getBestTv()<<endl<<
		" Best trtr: "<<bestParams.getBestTrTr()<<endl<<
		" Best tvtv: "<<bestParams.getBestTvTv()<<endl<<
		" Best trtv: "<<bestParams.getBestTrTv()<<endl<<
		" Best threeSub: "<<bestParams.getBestThreeSub()<<endl<<		
		" Best F: "<<bestParams.getBestF()<<endl<<
		" Best alpha: "<<bestParams.getBestAlpha()<<
		" Best beta: "<<bestParams.getBestBeta()<<endl;
	if (!_pOptions->_isGamma)
		out<<" Best Omega: "<<bestParams.getBestOmega()<<endl<<
		" Best Beta prob: "<<bestParams.getBestBetaProb()<<endl;
	out.close();
	_model.fillKaKsSelection();
	cout<<"After fill kaks"<<endl;
}

//this function compute the kaks value for each site. 
void kaks4Site::computeEB_EXPKaKs4Site(){

	LOG(5,<<"-------------------------"<<endl);
	LOG(5,<<" computing EB_EXP kaks for site "<<endl<<endl);
	//evaluate  EB_EXP for site 
	Vdouble  KaKsV,stdV,lowerBoundV,upperBoundV;							  
	computeEB_EXP_siteSpecificKaKs(KaKsV,stdV,lowerBoundV,upperBoundV,_codonSc,_model,_t);
	int id;
	if (_pOptions->_inQuerySeq.size()==0) id = _codonSc.placeToId(0);
	else id  = _codonSc.getId(_pOptions->_inQuerySeq);
	sequence refSeq = _codonSc[id];

	ofstream out(_pOptions->_outRes4SiteFile.c_str());
	amino aminoAcid;
	int gap=0;
	
	out<<"// "<<" amino "<<" ka/ks "<<endl;
	for (int i=0;i<_codonSc.seqLen();i++){	 
		int aa = codonUtility::aaOf(refSeq[i],*_codonAlph);  
		string aaStr = aminoAcid.fromInt(aa);
		out<<i-gap+1 <<" "<<aaStr<<" "<< KaKsV[i]<<endl;	
	}		
	out.close();
	cout<<"printing to selection file = "<<_pOptions->_outSelectionFile<<endl;
	kaks2Color(KaKsV,refSeq,_pOptions->_outSelectionFile,*_codonAlph);
}


MDOUBLE kaks4Site::findBestParamManyStarts(const Vint pointsNum, const Vint iterNum, const Vdouble tols){
	//make sure that the number of points in each cycle is not bigger than the previous cycle.
	int i;	
	for (i = 0; i < pointsNum.size()-1; ++i)
	{
		if (pointsNum[i] < pointsNum[i+1])
			errorMsg::reportError("input error in kaks4Site::findBestParamManyStarts()");
	}

	//create starting param sets 
	vector<params*> paramsV;
	const params dParams(_model.getTr(),_model.getTv(),_model.getTrTr(),_model.getTvTv(),
									_model.getTrTv(),_model.getF(),_model.getAlpha(),_model.getBeta(),_t);
	for (i = 0; i < pointsNum[0]; ++i)
	{
		//the first paramVec will be the current one
		if (i == 0)
			paramsV.push_back(new params(dParams)); 
		else //for the random initalization
			paramsV.push_back(new params(_t)); 
	}

	//make a small number of iterations for all random starts 
	int numOfOptCycles = pointsNum.size();
	Vdouble likelihoodVec;
	for (i = 0; i < numOfOptCycles; ++i)
	{
		if (i != 0)
		{
			vector<params*> tmpParamVec(0);
			//sort results and continue optimization only with the best (pointsNum[i]) points
			Vdouble sortedL = likelihoodVec;
			sort(sortedL.begin(),sortedL.end());
			MDOUBLE threshold = sortedL[sortedL.size()- pointsNum[i]];
			for (int j = 0; j < likelihoodVec.size(); ++j)
			{
				if (likelihoodVec[j] >= threshold) 
					tmpParamVec.push_back(paramsV[j]);
				else
					delete paramsV[j];
			}
			paramsV.clear();
			paramsV = tmpParamVec;
		} 
		
		likelihoodVec.clear();
		likelihoodVec.resize(pointsNum[i]); 
		int c; 		
		for (c = 0; c < pointsNum[i]; ++c)
		{
			cerr <<"optimizing point " <<c<<endl;
			LOG(5,<<endl<<endl<<"optimizing point " <<c<<" "<<paramsV[c]->tr()<<" "<<paramsV[c]->tv()<<" "<<paramsV[c]->trtr()<<" "<<paramsV[c]->tvtv()<<" "<<paramsV[c]->trtv()<<" "<<paramsV[c]->f()<<" "<<paramsV[c]->alpha()<<" "<<paramsV[c]->beta()<<" "<<endl);
				
			empiriSelectionModel model(_model.freqCodon(),_model.freqAmino(),*_codonAlph,_model.getIsGamma(),_model.noOfCategor(),paramsV[c]->tr(),paramsV[c]->tv(),paramsV[c]->trtr(),paramsV[c]->tvtv(),paramsV[c]->trtv()
				,paramsV[c]->f(),paramsV[c]->alpha(),paramsV[c]->beta());
			tree t(paramsV[c]->getTree());
			bestAlphaAndKsAndBBL bestParams(t,_codonSc,model,5.0,20.0, 0.01,tols[i],iterNum[i]);
			MDOUBLE ll = bestParams.getBestL();
			cerr<<endl<<"pointi: "<<c<<"  likelihood = "<<ll<<endl;
			LOG(5,<<endl<<"pointi: "<<c<<"  likelihood = "<<ll<<endl);
			likelihoodVec[c] = ll;
			paramsV[c]->setParams(bestParams.getBestTr(),bestParams.getBestTv(),bestParams.getBestTrTr(),bestParams.getBestTvTv(),bestParams.getBestTrTv(),bestParams.getBestF(),bestParams.getBestAlpha(),bestParams.getBestBeta(),t);
		}
	}

	Vdouble sortedL = likelihoodVec;
	sort(sortedL.begin(),sortedL.end());
	MDOUBLE bestL = sortedL[likelihoodVec.size() - 1];
	for (i = 0; i < likelihoodVec.size(); ++i)
	{
		if (bestL == likelihoodVec[i]) 
		{	
			LOG(5,<<"**The Best L = " <<bestL<<" And The params = "<<paramsV[i]->tr()<<" "<<paramsV[i]->tv()<<" "<<paramsV[i]->trtr()<<" "<<paramsV[i]->tvtv()<<" "<<paramsV[i]->trtv()<<" "<<paramsV[i]->f()<<" "<<paramsV[i]->alpha()<<" "<<paramsV[i]->beta()<<" "<<endl);
			empiriSelectionModel model(_model.freqCodon(),_model.freqAmino(),*_codonAlph,_model.getIsGamma(),_model.noOfCategor(),paramsV[i]->tr(),paramsV[i]->tv(),paramsV[i]->trtr(),paramsV[i]->tvtv(),paramsV[i]->trtv()
				,paramsV[i]->f(),paramsV[i]->alpha(),paramsV[i]->beta());
			_t = paramsV[i]->getTree(); //update the best tree
			_model= model; //update the best model
			LOG(5,<<"--The like = "<<likelihoodComputation2Codon::getTreeLikelihoodAllPosAlphTheSame(_t,_codonSc,_model)<<endl);//_spVec,_distr);
		}
		delete paramsV[i];
	}	
	paramsV.clear();
	return bestL;
}


void kaks4Site::printGlobalRes(){
	_t.output(_pOptions->_outTreeFile);
	ofstream out(_pOptions->_outGlobalRes.c_str());
	out<<"The Best likelihood: "<<likelihoodComputation2Codon::getTreeLikelihoodAllPosAlphTheSame(_t,_codonSc,_model)<<endl<<
		" Best tr: "<<_model.getTr()<<endl<<
		" Best tv: "<<_model.getTv()<<endl<<
		" Best trtr: "<<_model.getTrTr()<<endl<<
		" Best trtv: "<<_model.getTrTv()<<endl<<
		" Best tvtv: "<<_model.getTvTv()<<endl<<
		" Best F: "<<_model.getF()<<endl<<
		" Best alpha: "<<_model.getAlpha()<<
		" Best beta: "<<_model.getBeta()<<endl;
	out.close();
	_model.fillKaKsSelection();
	cout<<"After fill kaks"<<endl;


}


/////////////////////////////////////////////////////////
//////////This part is for goldman and yang model///////
////////////////////////////////////////////////////////

//thsi function compute parameter value for the goldman and yang model and BBL
//  the copmutation is done on the all protein.
/*void kaks4Site::computeGlobalParams(MDOUBLE initV,MDOUBLE initK){
	//evaluate best global w, k's and bl 
	bestGlobalVKAndBBL bgvk(initV,initK);
	bgvk.optimizeCodonModelAndBBL(_t,_codonSc,_spVec[0]);
	_t.output(_pOptions->_outTreeFile);
	ofstream out(_pOptions->_outGlobalRes.c_str());
	out<<"The Best likelihood: "<<bgvk.getBestL()<<endl<<
		" best k: "<<bgvk.getBestK()<<endl<<
		" best v: "<<bgvk.getBestV()<<endl<<endl;
		//" ka/ks: "<<computeKaKs()<<endl;
	out.close();
}


//this function compute ML kaks for site according the Goldman+Yang model (c model) 
void kaks4Site::computeMLKaKs4Site(){

	LOG(5,<<"-------------------------"<<endl);
	LOG(5,<<" computing ML kaks for site "<<endl<<endl);

	//evaluate c for site - k and bl are fixed.
	goldmanYangModel *pmodel = static_cast<goldmanYangModel*>(_spVec[0].getPijAccelerator()->getReplacementModel());
	pmodel->setGlobalV(false); //change to kaks for site - no need to normalize Q.
	Vdouble  KaKsV;
	Vdouble  likelihoodsV;							  
	computeML_siteSpecificKaKs(KaKsV,likelihoodsV,_codonSc,_spVec[0],_t);
	
	//create no selection model.
	pmodel->setV(0);  
	int id;	
	if (_pOptions->_inQuerySeq.size()==0) id = _codonSc.placeToId(0);
	else id  = _codonSc.getId(_pOptions->_inQuerySeq);
	sequence refSeq = _codonSc[id];

	StatisticalScore statScore(_t,_codonSc,_spVec[0],0.05);
	LOG(5,<<"-------------------------"<<endl);
	LOG(5,<<"  Applying LRT and FDR   "<<endl<<endl);
	statScore.applyLRT(KaKsV,likelihoodsV,refSeq);
	
	//output function 
	statScore.outStatisticScores(_pOptions->_outRes4SiteFile,refSeq); //create file with kaks, pvalue and like-different for each site
	statScore.outToServer(_pOptions->_outSelectionFile,refSeq);  //create file with significant 7 categories - for server

	vector<int> colorVec = statScore.getColorVec(refSeq);
	outToRasmolFile(_pOptions->_outRasmolFile,colorVec); //create file for rasmol according significant
	
}



//return nonSyn/syn of the given model 
MDOUBLE kaks4Site::nonSyn2Syn(const stochasticProcess & sp){
	int i,j;
	MDOUBLE nonsynonymous=0.0,synonymous=0.0;
	goldmanYangModel *pmodel = static_cast<goldmanYangModel*>(_spVec[0].getPijAccelerator()->getReplacementModel());
	for (i=0;i<pmodel->alphabetSize();i++){
		for(j=i+1;j<pmodel->alphabetSize();j++){
			if (codonUtility::aaOf(i) != codonUtility::aaOf(j)){  //nonsynonymous
				nonsynonymous += pmodel->freq(i) * pmodel->getQij(i,j);
			}else{ //synonymous
				synonymous += pmodel->freq(i) * pmodel->getQij(i,j);						
			}
		}		
	}
	return nonsynonymous/synonymous;
}
//this function compute the nonSyn/Syn dividing by nonSyn/Syn 
//for the no selection goldman and yang model 
MDOUBLE kaks4Site::computeKaKs(){
	MDOUBLE res = nonSyn2Syn(_spVec[0]);
	goldmanYangModel *pmodel = static_cast<goldmanYangModel*>(_spVec[0].getPijAccelerator()->getReplacementModel());
	pmodel->setV(0); //changing to no selection model
	MDOUBLE resNorm = nonSyn2Syn(_spVec[0]);	
	return (res/resNorm);
}
*/
