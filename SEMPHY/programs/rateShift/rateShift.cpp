// 	$Id: rateShift.cpp 2399 2014-03-13 22:43:51Z wkliao $	
#include "rateShiftOptions.h"
#include "rateShift.h"

int main(int argc, char** argv)
{
	rateShift* program = new rateShift();
	program->run(argc,argv);
	if (program) delete program;
	return 0;
}


rateShift::~rateShift()
{
	if (_options) delete _options;
	if (_alph) delete _alph;
	if (_baseAlph) delete _baseAlph;	
	if (_model) delete _model;

	vector<posDataSSRV*>::iterator itr = _SSRV4site.begin();
	for (;itr != _SSRV4site.end(); ++itr)
		delete (*itr);
	
	if (_rateShiftProbs4branch) 
		delete _rateShiftProbs4branch;
}

void rateShift::run(int argc, char** argv) 
{
	fillOptionsParameters(argc,argv);
	if (!_options->logFile.empty())
		myLog::setLog(_options->logFile, _options->logValue);
	printrateShiftInfo(cout);
	printOptionParameters();

	cout << "get Sequence data" << endl;
	getStartingSequenceData();

	cout << "init USSRV model" << endl;
	//init USSRV model
	getStartingUSSRVmodel();

	// get tree data
	cout << "get tree data" << endl;
	_et = tree(_options->treefile);
	
	// find likelihood
	cout << "calculate likelihood" << endl;
	calcLikelihood(); // similar to rate4site::getStartingBranchLengthsAndAlpha 
	cout << "print output tree" << endl;
	printOutputTree();
	cout << "fill reference sequence" << endl;
	fillReferenceSequence();
	// calculate the probability of the ssrv model for every site.
	cout << "calculate the probability of the ssrv model for every site" << endl;
	calcSSRV4site();
	ofstream outFile(_options->outFile.c_str());
	cout << "print SSRV4site" << endl;
	printSSRV4site(outFile);
	sortSSRV4site();
	cout << "print sorted SSRV4site" << endl;
	printSSRV4site(outFile);
	printGapsPositionsInRefSeq(outFile);
	// caculate the probability of rateShift (acceleration and deceleration) for every branch
	cout << "calculate shifts4branch" << endl;
	calcShifts4branch();
	// print the branched data results:
	// outFile : the branched data.
	// outPositions4EachBranch = outFile+".detailed" : for each branch the data of every position
	// outTreeWithBranchesNames = treeOutFile+".namesBS.ph" : The tree with the rateShift probability of
	// every branch as its length, and the id of every node as the bootstrap value.
	cout << "print the branches data" << endl;
	ofstream outTreeWithBranchesNames((_options->treeOutFile + string(".namesBS.ph")).c_str());
	ofstream outPositions4EachBranch((_options->outFile + string(".detailed")).c_str());
	
	printBranchesData(outFile,outTreeWithBranchesNames,outPositions4EachBranch);
	
	// close files
	outFile.close();
	outTreeWithBranchesNames.close();
	outPositions4EachBranch.close();
}


void rateShift::printrateShiftInfo(ostream& out) {
	out<<endl;
	out<<" ======================================================="<<endl;
	out<<" the rateShift project:					              "<<endl;
	out<<" Version: 1.0. Last updated 31.07.06                    "<<endl;
	out<<" ======================================================="<<endl;
	out<<endl;
}


void rateShift::fillOptionsParameters(int argc, char** argv) {
    _options = new rateShiftOptions(argc, argv);
}

void rateShift::printOptionParameters() {
	cout<<"\n ---------------------- THE PARAMETERS ----------------------------"<<endl;
	if (_options->treefile.size()>0) { 
		cout<<"tree file is: "<<_options->treefile<<endl;
		LOG(2,<<"tree file is: "<<_options->treefile<<endl);}
		if (_options->seqfile.size()>0) {
			cout<<"seq file is: "<<_options->seqfile<<endl;
			LOG(2,<<"seq file is: "<<_options->seqfile<<endl);}
		if (_options->outFile.size()>0) {
			cout<<"output file is: "<<_options->outFile<<endl;
			LOG(2,<<"output file is: "<<_options->outFile<<endl);}

	switch (_options->modelName){
		case (rateShiftOptions::day): 
			cout<< "probablistic_model is: DAY" <<endl;
			LOG(2,<< "probablistic_model is: DAY" <<endl); break;
		case (rateShiftOptions::jtt): 
			cout<< "probablistic_model is: JTT" <<endl; 
			LOG(2,<< "probablistic_model is: JTT" <<endl); 
			break;
		case (rateShiftOptions::rev): 
			cout<< "probablistic_model is: REV" <<endl;
			LOG(2,<< "probablistic_model is: REV" <<endl);
			break;
		case (rateShiftOptions::aajc): 
			cout<< "probablistic_model is: AAJC" <<endl;
			LOG(2,<< "probablistic_model is: AAJC" <<endl);
			break;
		case (rateShiftOptions::nucjc): 
			cout<< "probablistic_model is: NUCJC" <<endl;
			LOG(2,<< "probablistic_model is: NUCJC" <<endl);
			break;
		case (rateShiftOptions::wag): 
			cout<< "probablistic_model is: WAG" <<endl; 
			LOG(2,<< "probablistic_model is: WAG" <<endl); 
			break;
		case (rateShiftOptions::cprev):
			cout<< "probablistic_model is: CPREV" <<endl;
			LOG(2,<< "probablistic_model is: CPREV" <<endl);
			break;
		//case (rateShiftOptions::tamura): cout<< "probablistic_model is: TAMURA" <<endl; break;
	}

	if (_options->optimizeBranchLengths) {
		cout<<"branch lengths were optimized using ML."<<endl;
		LOG(2,<<"branch lengths were optimized using ML."<<endl);
	}
	else {
		cout<<"branch lengths were not optimized."<<endl;
		LOG(2,<<"branch lengths were not optimized."<<endl);
	}
	
	cout << "number of discrete categories: " << _options->numberOfDiscreteCategories << endl;
	cout << "Init Alpha: " << _options->userInputAlpha << endl;
	cout << "Init Nu: " << _options->userInputNu << endl;
	cout << "Init F: " << _options->userInputF << endl;

	cout<<"\n -----------------------------------------------------------------"<<endl;

	LOG(2, << "number of discrete categories: " << _options->numberOfDiscreteCategories << endl);
	LOG(2, << "Init Alpha: " << _options->userInputAlpha << endl);
	LOG(2, << "Init Nu: " << _options->userInputNu << endl);
	LOG(2, << "Init F: " << _options->userInputF << endl);

	LOG(2,<<"\n -----------------------------------------------------------------"<<endl);
}

//get Sequence data
void rateShift::getStartingSequenceData(){
	if (_options->seqfile == "") {
		errorMsg::reportError("Please give a sequence file name in the command line");
	}
	
	ifstream in(_options->seqfile.c_str());
	int alphabetSize = _options->alphabet_size;

	if (alphabetSize==4) _baseAlph = new nucleotide;
	else if (alphabetSize == 20) _baseAlph = new amino;
	else errorMsg::reportError("no such alphabet in function rateShift::getStartingSequenceData");

	_alph = new mulAlphabet(_baseAlph,_options->numberOfDiscreteCategories);
	_baseSc = new sequenceContainer(recognizeFormat::read(in, _baseAlph));
	_baseSc->changeGaps2MissingData(); // @@@@ is this reasonable ?
	
	_scSSRV = new sequenceContainer(*_baseSc,_alph);

	//_scSSRV =  new sequenceContainer(recognizeFormat::read(in, _alph));
	//_scSSRV->changeGaps2MissingData();
	
	in.close();
}

void rateShift::getStartingUSSRVmodel() {
	if (_options->numberOfDiscreteCategories<1 || _options->numberOfDiscreteCategories>50) {
		errorMsg::reportError("number of discrete rate categories should be between 1 and 50");
	}
	distribution *dist = new gammaDistribution(_options->userInputAlpha,_options->numberOfDiscreteCategories);
	
	replacementModel* pRM = NULL;
	pijAccelerator *basePijAcc=NULL; // Accelerator for the base model, without SSRV.
	
	switch (_options->modelName) {
		case (rateShiftOptions::day): 
			pRM=new pupAll(datMatrixHolder::dayhoff);
			basePijAcc = new chebyshevAccelerator(pRM);
			break;
		case (rateShiftOptions::jtt):
			pRM = new pupAll(datMatrixHolder::jones);
			basePijAcc = new chebyshevAccelerator(pRM);
			break;
		case (rateShiftOptions::rev):
			pRM=new pupAll(datMatrixHolder::mtREV24);
			basePijAcc = new chebyshevAccelerator(pRM);
			break;
		case (rateShiftOptions::wag):
			pRM=new pupAll(datMatrixHolder::wag);
			basePijAcc = new chebyshevAccelerator(pRM);
			break;
		case (rateShiftOptions::cprev):
			pRM=new pupAll(datMatrixHolder::cpREV45);
			basePijAcc = new chebyshevAccelerator(pRM);
			break;
		case (rateShiftOptions::nucjc):
			pRM = new nucJC;
			basePijAcc = new trivialAccelerator(pRM);
			break;
		case (rateShiftOptions::aajc):
			pRM=new aaJC;
			basePijAcc = new trivialAccelerator(pRM);
			break;
		// case (rateShiftOptions::hky): pRM = new
		//case (rateShiftOptions::tamura): pRM = new hky(; break;
		default: errorMsg::reportError("rateShift::getStartingStochasticProcess modelName is not supported");
		
	}
	stochasticProcess baseSp(dist, basePijAcc);
	replacementModel* pProbMod = new replacementModelSSRV(dist,pRM,_options->userInputNu) ;
	pijAccelerator *pijAcc = new trivialAccelerator(pProbMod); // accelarator for the SSRV model
	stochasticProcessSSRV sp(pijAcc);
	_model = new ussrvModel(baseSp,sp,_options->userInputF);

	if (pProbMod) delete pProbMod;
	if (pRM) delete pRM;
	if (basePijAcc) delete basePijAcc;
	if (pijAcc) delete pijAcc;
}

void rateShift::calcLikelihood()
{
	bool AlphaOptimization(false),NuOptimization(false),bblOptimization(false);
	bool FOptimization(_options->optimizeF);
		
	if ((_options->optimizationType == rateShiftOptions::noOptimization) && (FOptimization==false)) {
		calcLikelihoodWithoutOptimization();
	}
	else {
		if (_options->optimizeBranchLengths == rateShiftOptions::mlBBL)
			bblOptimization=true;
		if (_options->optimizationType == rateShiftOptions::alpha)
			AlphaOptimization=true;
		else if (_options->optimizationType == rateShiftOptions::nu)
			NuOptimization=true;
		else if (_options->optimizationType == rateShiftOptions::alphaAndNu)
			AlphaOptimization=NuOptimization=true;
		calcLikelihoodWithOptimization(AlphaOptimization,NuOptimization,FOptimization,bblOptimization);
	}
	LOG(4,<<"Normalize factor is: " << _model->calcNormalizeFactor() << endl);
}

void rateShift::calcLikelihoodWithoutOptimization()
{
	MDOUBLE LogLikelihood = likelihoodComputation2USSRV::getTreeLikelihoodAllPosAlphTheSame(_et,*_scSSRV,*_baseSc,*_model);
	LOGDO(4,printTime(myLog::LogFile()));
	LOG(4, << "LogLikelihood: " << LogLikelihood << endl);
	cout  << "LogLikelihood: " << LogLikelihood << endl;
}

void rateShift::calcLikelihoodWithOptimization(bool AlphaOptimization, bool NuOptimization,
											   bool FOptimization, bool bblOptimization)
{
	LOGDO(4,printTime(myLog::LogFile()));
	LOG(4,<< "find likelihood with optimization of: ");
	if (AlphaOptimization) LOG(4,<< "Alpha ");
	if (NuOptimization) LOG(4,<< "Nu ");
	if (FOptimization) LOG(4,<< "F ");
	if (bblOptimization) LOG(4,<< "bbl ");
	LOG(4,<<endl);
	
	bestParamUSSRV generalOptimization(AlphaOptimization,NuOptimization,FOptimization,bblOptimization);		
	generalOptimization(_et,*_scSSRV,*_baseSc,*_model);
				
	LOGDO(4,printTime(myLog::LogFile()));
	LOG(4,<< "LogLikelihood is: " << endl);
	LOG(4, << "bestL: " << generalOptimization.getBestL() << endl);
	LOG(4,<< "Parameters are: " << endl);
	LOG(4, << "best alpha " << generalOptimization.getBestAlpha() << endl);
	LOG(4, << "best nu " << generalOptimization.getBestNu() << endl);
	LOG(4, << "best f " << generalOptimization.getBestF() << endl << endl);
}


void rateShift::setOriginalAlphaAndNuAndF()
{
	cout << "change back to initial Nu = " << _options->userInputNu ;
	cout << " alpha = " << _options->userInputAlpha ;
	cout << " f = " << _options->userInputF << endl;
	_model->updateAlpha(_options->userInputAlpha);
	_model->updateNu(_options->userInputNu);
	_model->updateF(_options->userInputF);
}

// calculate the probability of the ssrv model for every site.
//                        P(Data | model = ssrv) * P(model = ssrv)							L(ssrv)*f
// P(model=ssrv | Data) = -----------------------------------------------------  = --------------------------
//						  P(Data|model=ssrv)P(ssrv) + P(Data|model=base)P(base)    L(ssrv)*f + L(base)*(1-f)
void rateShift::calcSSRV4site()
{
	LOGDO(4,printTime(myLog::LogFile()));
	LOG(4,<<"calcSSRV4site()" << endl);
	// similar to likelihoodComputation2USSRV::getTreeLikelihoodAllPosAlphTheSame
	computePijHom piSSRV;
	piSSRV.fillPij(_et,_model->getSSRVmodel());
	
	computePijGam piBase;
	piBase.fillPij(_et,_model->getBaseModel());
	
	MDOUBLE f = _model->getF();
	doubleRep LofPosSSRV(0.0),LofPosBase(0.0),LofPos(0.0);
	int k;
	for (k=0; k < _scSSRV->seqLen(); ++k) {
	    if (f<1.0)
			LofPosBase = likelihoodComputation::getLofPos(k,_et,*_baseSc,piBase,_model->getBaseModel());
		if (f>0.0) {
			LofPosSSRV = likelihoodComputation::getLofPos(k,_et,*_scSSRV,piSSRV,_model->getSSRVmodel());
			if (f<1.0) 
				LofPos = LofPosSSRV*f+(1-f)*LofPosBase;
			else // f == 1.0
				LofPos = LofPosSSRV;
		}
		else // f == 0.0
			LofPos = LofPosBase;

		// positions Data start from 1, not from 0 !!!
		_SSRV4site.push_back(new posDataSSRV(k+1,
											_refSeq->getAlphabet()->fromInt((*_refSeq)[k]),
											_baseSc->numberOfSequencesWithoutGaps(k),
											_baseSc->numberOfSeqs(),
											convert(LofPosSSRV*f/LofPos)));

		// deubg OZ
		LOG(10,<<"position: " << k+1 << endl);
		LOG(10,<<" LofPosSSRV: " << LofPosSSRV << " LofPosBase: " << LofPosBase);
		LOG(10,<<" LofPos: " << LofPos << " f: " << f << endl);
		if ((f<0.0) || (f>1.0))
			errorMsg::reportError("rateShift::calcSSRV4site() error, invalid f value");
		// end of debug
	}
	LOGDO(4,printTime(myLog::LogFile()));
	LOG(4,<<"finished calcSSRV4site" << endl);
}

// same as rate4site::fillReferenceSequence
void rateShift::fillReferenceSequence(){
	if (strcmp(_options->referenceSeq.c_str(),"non")==0) {
		_refSeq = &((*_baseSc)[0]);
	}
	else {
		int id1 = _baseSc->getId(_options->referenceSeq,true);
		_refSeq = &((*_baseSc)[id1]);
	}
}

void rateShift::printSSRV4site(ofstream& out) const
{
	out << "Probability of ssrv model for each site:" << endl;
	vector<posDataSSRV*>::const_iterator itr = _SSRV4site.begin();
	for (; itr != _SSRV4site.end(); ++itr)
		out << **itr << endl;
	out << endl << endl;
}

void rateShift::printGapsPositionsInRefSeq(ofstream& out) const
{
	out << "The refSeq is : " << _refSeq->name() << endl;
	int k(0);
	for (k=0; k < _scSSRV->seqLen(); ++k) {
		out << _refSeq->getAlphabet()->fromInt((*_refSeq)[k]);
		if ((k+1)%10 == 0)
			out << endl;
	}
	out << endl;
	out << "Gaps\\Unknown positions (starting from 1 !!!) of the refSeq: "  << endl;
	for (k=0; k < _scSSRV->seqLen(); ++k) {
		if (_refSeq->isSpecific(k) == false)
		{
			out << k+1 << " ";
			out << endl;
		}
	}
}
// similar to rate4site::printOutputTree()
void rateShift::printOutputTree() {
	ofstream f(_options->treeOutFile.c_str());
	_et.output(f);
	f.close();
	cout<<"The tree was written to a file name called "<<_options->treeOutFile<<endl;
}


void rateShift::calcShifts4branch()
{
	LOGDO(4,printTime(myLog::LogFile()));
	LOG(4,<<"calcShifts4branch()" << endl);
	_rateShiftProbs4branch = new rateShiftProbs4branch(_et,*_scSSRV,*_baseSc,*_model);
//	_rateShiftProbs4branch->runSSRV();
	_rateShiftProbs4branch->runUSSRV();
	_rateShiftProbs4branch->calculateBinomialPvalues();
	LOG(8,<<"calculatePoissonPvalues"<< endl);
	_rateShiftProbs4branch->calculatePoissonPvaluesAmpiricLamda();
	_rateShiftProbs4branch->calculatePoissonPvaluesTheoreticalLamda();
	LOG(4,<<"finished calcShifts4branch()" << endl);
}

void rateShift::printBranchesData(ostream& out,ostream& outTreeWithBranchesNames,ostream& outPositions4EachBranch) const
{
	out << "**** Branches Data ****" << endl;
	out << "// Format: " << endl;
	out << "// branch: nodeNum branchAccelerationProb branchDecelerationProb branchRateShiftProb branchLength" << endl;

	_rateShiftProbs4branch->printBranchesData(out);
	_rateShiftProbs4branch->printBinomialSignificantBranches(out);
	_rateShiftProbs4branch->printPoissonSignificantBranches(out);
	printTreeWithNodeIdBPStyle(outTreeWithBranchesNames);

	outPositions4EachBranch << "**** Branches Data With Positions****" << endl;
	outPositions4EachBranch << "nodeNum posNum posAccelerationProb posDecelerationProb posRateShiftProb" << endl;

	_rateShiftProbs4branch->printPositions4EachBranch(outPositions4EachBranch);
}

// copied from the original covarion code , from the file checkov.cpp
void rateShift::printTreeWithNodeIdBPStyle(ostream &out) const{
	recursivePrintTree(out,_et.getRoot());
	out<<";";
}

// similar to the file checkov.cpp from the original covarion code
// Changed so the branches length will indicate the rate-shift probability
// and the bootstrap values will be the nodes id.
void rateShift::recursivePrintTree(ostream &out,const tree::nodeP &myNode) const {
	if (myNode->isLeaf()) {
		out << myNode->name() << "_" << myNode->id();
		out << ":"<< (*_rateShiftProbs4branch)[myNode->id()]->getRateShiftProb();
		return;
	} else {
		out <<"(";
		for (int i=0;i<myNode->getNumberOfSons();++i) {
			if (i>0) out <<",";
			recursivePrintTree(out, myNode->getSon(i));
		}
		out <<")";
		if (myNode->isRoot()==false) {
			out<<":"<< (*_rateShiftProbs4branch)[myNode->id()]->getRateShiftProb();
			out << "["<<myNode->id()<<"]";
		}
	}
}

//int main(int argc, char** argv)
//{
//	const MDOUBLE alpha=0.2;
//	const int numberOfCategories =4;
//	string msaFileName = "myExample_fasta.aln";
//	string treeFileName = "myExample.ph";
//	string logFile = "testLogFile.txt";
//	myLog::setLog(logFile,8);
//	LOGDO(4,printTime(myLog::LogFile()));
//
//	LOG(5, << "msaFile: " << msaFileName << " treeFile: " << treeFileName << endl);
//
//	LOG(5, << "get Sequence data" << endl);
//	alphabet* alph = NULL;
//	//get Sequence data
//	ifstream in(msaFileName.c_str());
//	alph = new nucleotide;
//	sequenceContainer sc = recognizeFormat::read(in, alph);
//	sc.changeGaps2MissingData();
//	in.close();
//
//	LOG(5,<<"init stochastic process" << endl);
//	//init stochastic process
//	distribution *dist =NULL;
//	dist = new gammaDistribution(alpha,numberOfCategories);
//	//dist = new uniDistribution();
//	
//	replacementModel* pProbMod = NULL;
//	pijAccelerator* pijAcc = NULL;
//	pProbMod = new nucJC;
//	pijAcc = new trivialAccelerator(pProbMod);
//	 
//	stochasticProcess sp(dist, pijAcc);
//	
//	// get tree data
//	LOG(5, << "get tree data" << endl);
//	tree tr(treeFileName);
//
//	//find likelihood
//	LOG(5, << "find Likelihood" << endl);
//	
//	MDOUBLE LogLikelihood = likelihoodComputation::getTreeLikelihoodAllPosAlphTheSame(tr,sc,sp);
//	LOG(5,  << "LogLikelihood: " << LogLikelihood << endl);
//	bestAlphaAndBBL bblEM1(tr,sc,sp);
//  	LOG(5, << "after bblEM, likelihood: " << bblEM1.getBestL() << endl);
////	bblEM bblEM1(tr,sc,sp,NULL,50,0.05,0.01);
////	LOG(5, << "after bblEM, likelihood: " << bblEM1.getTreeLikelihood() << endl);
//	// calculate manually
//
//	//float fij02 = 0.25-0.25*exp(-4.0*0.2/3.0);
//	//float fij03 = 0.25-0.25*exp(-4.0*0.3/3.0);
//	//float fii02 = 1-3*fij02;
//	//float fii03 = 1-3*fij03;
//	//float Lpos = fii02*fii03*fij02;
//	//Lpos += fij02*fij03*fij02;
//	//Lpos += fij02*fij03*fii02;
//	//Lpos += fij02*fij03*fij02;
//	//Lpos /= 4.0;
//	//float L = Lpos*Lpos*Lpos*Lpos;
//	//cout << "Lpos: " << Lpos << " L: " << L << endl;
//	//cout << "LogLpos: " << log(Lpos) << " LogL: " << log(L) << endl;
//	//
//	// free memory
//	if (pProbMod) delete pProbMod;
//	if (pijAcc) delete pijAcc;
//	if (dist) delete dist;
//	if (alph) delete alph;
//
//	return 0;
//}
