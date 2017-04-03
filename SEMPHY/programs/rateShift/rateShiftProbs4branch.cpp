// 	$Id: rateShiftProbs4branch.cpp 2399 2014-03-13 22:43:51Z wkliao $	
#include "rateShiftProbs4branch.h"

rateShiftProbs4branch:: rateShiftProbs4branch(const tree& et,
				const sequenceContainer& scSSRV,const sequenceContainer& baseSc,
				const ussrvModel& model):
_treeLogLikelihood(0.0),
_et(et),_scSSRV(scSSRV),_baseSc(baseSc),
_model(model) 
{
	_branchesData.resize(_et.getNodesNum(),NULL);
}

rateShiftProbs4branch::~rateShiftProbs4branch()
{
	vector<branchData*>::iterator itr = _branchesData.begin();
	for (;itr != _branchesData.end(); ++itr) {
		if (*itr)
			delete (*itr);
	}
}

void rateShiftProbs4branch::init()
{
	allocatePlaceSSRV(); 
	_pijSSRV.fillPij(_et,_model.getSSRVmodel(),0);
	computeUpSSRV();
	computeDownSSRV();
}
void rateShiftProbs4branch::runSSRV()
{
	LOG(4,<< "rateShiftProbs4branch::runSSRV" << endl);
	init();
	fillBranchesDataSSRV();
	// debug OZ
	//fillBranchesDataSSRVWithDebug();
	// end of debug
}

void rateShiftProbs4branch::runUSSRV()
{
	LOG(4,<< "rateShiftProbs4branch::runUSSRV" << endl);
	init();
	_pijBase.fillPij(_et,_model.getBaseModel());
	fillBranchesDataUSSRV();
	// debug OZ
	// fillBranchesDataUSSRVWithDebug();
	// end of debug
}

// similar to bblEM2USSRV::computeUp()
void rateShiftProbs4branch::computeUpSSRV()
{
	computeUpAlg cupAlg;
	for (int pos=0; pos < _scSSRV.seqLen(); ++pos) {
		cupAlg.fillComputeUp(_et,_scSSRV,pos,_pijSSRV,_cupSSRV[pos]);
	}
}

void rateShiftProbs4branch::computeDownSSRV()
{
	computeDownAlg cdownAlg;
	for (int pos=0; pos < _scSSRV.seqLen(); ++pos) {
		cdownAlg.fillComputeDown(_et,_scSSRV,pos,_pijSSRV,_cdownSSRV[pos],_cupSSRV[pos]);
	}
}

void rateShiftProbs4branch::allocatePlaceSSRV()
{
	_cupSSRV.allocatePlace(_scSSRV.seqLen(), _et.getNodesNum(), _scSSRV.alphabetSize());
	_cdownSSRV.allocatePlace(_scSSRV.seqLen(), _et.getNodesNum(), _scSSRV.alphabetSize());
}



void rateShiftProbs4branch::fillBranchesDataSSRV()
{
	treeIterDownTopConst tIt(_et);
	int letter,fatherLetter,pos;
	const mulAlphabet* alphabetSSRV = static_cast<const mulAlphabet*>(_scSSRV.getAlphabet());
	const stochasticProcessSSRV ssrvSp = _model.getSSRVmodel();
	
	for (tree::nodeP mynode = tIt.first(); mynode != tIt.end(); mynode = tIt.next()) {
		doubleRep likelihoodPos(0.0),val(0.0);
		if (mynode->isRoot() == false)
		{
			branchData* data = new branchData(mynode->id(),mynode->dis2father());
			for (pos=0; pos < _scSSRV.seqLen(); ++pos) {
				likelihoodPos=0.0;
				doubleRep probAcceleration(0.0),probDeceleration(0.0),probNoRateShift(0.0);
				for(letter=0; letter<alphabetSSRV->size();++letter) {
					for (fatherLetter=0; fatherLetter<alphabetSSRV->size();++fatherLetter) {						
						val=calcVal(mynode->id(),pos,letter,fatherLetter,ssrvSp);
						int categoryCompare = alphabetSSRV->compareCategories(fatherLetter,letter);
						switch (categoryCompare) {
							case -1: probDeceleration+=val; break;
							case 0: probNoRateShift+=val; break;
							case 1: probAcceleration+=val; break;
						}
						likelihoodPos+=val;
					}
				}
				probAcceleration/=likelihoodPos;
				probDeceleration/=likelihoodPos;
				probNoRateShift/=likelihoodPos;
				
				data->addPositionData(pos,convert(probAcceleration),convert(probDeceleration));
				
				LOG(12,<<"position: "<< pos << " probDecelartaion= " << probDeceleration);
				LOG(12,<<" probAcceleration= " << probAcceleration );
				LOG(12,<<" probNoRateShift= " << probNoRateShift << endl);
				MDOUBLE sumProbPos= convert(probDeceleration)+convert(probAcceleration)+convert(probNoRateShift);
				MDOUBLE epsilon = 0.000001;
				if ((sumProbPos + epsilon < 1.0) || (sumProbPos > 1.0 + epsilon))
					LOG(4,<<" rateShiftProbs4branch::fillBranchesData() ERROR !!! sumProbPos = " << sumProbPos << endl);
			}
			_branchesData[mynode->id()] = data;
		}
	}	
}



// all the testLikelihood calculation here and the likelihoodComputation::getTreeLikelihoodAllPosAlphTheSame
// is just for debug OZ
void rateShiftProbs4branch::fillBranchesDataSSRVWithDebug()
{
	treeIterDownTopConst tIt(_et);
	int letter,fatherLetter,pos;
	const mulAlphabet* alphabetSSRV = static_cast<const mulAlphabet*>(_scSSRV.getAlphabet());
	const stochasticProcessSSRV ssrvSp = _model.getSSRVmodel();
	MDOUBLE likelihood = likelihoodComputation::getTreeLikelihoodAllPosAlphTheSame(_et,_scSSRV,ssrvSp);
	for (tree::nodeP mynode = tIt.first(); mynode != tIt.end(); mynode = tIt.next()) {
		MDOUBLE testLikelihood(0.0);
		doubleRep likelihoodPos(0.0),val(0.0);
		if (mynode->isRoot() == false)
		{
			cout << mynode->id() << endl;
			branchData* data = new branchData(mynode->id(),mynode->dis2father());
			LOG(8,<< '\t' << "nodeId = " << mynode->id() << " isleaf?: " << mynode->isLeaf() << endl);
			for (pos=0; pos < _scSSRV.seqLen(); ++pos) {
				likelihoodPos=0.0;
				doubleRep probAcceleration(0.0),probDeceleration(0.0),probNoRateShift(0.0);
				for(letter=0; letter<alphabetSSRV->size();++letter) {
					for (fatherLetter=0; fatherLetter<alphabetSSRV->size();++fatherLetter) {						
						val=calcVal(mynode->id(),pos,letter,fatherLetter,ssrvSp);
						int categoryCompare = alphabetSSRV->compareCategories(fatherLetter,letter);
						switch (categoryCompare) {
							case -1: probDeceleration+=val; break;
							case 0: probNoRateShift+=val; break;
							case 1: probAcceleration+=val; break;
						}
						likelihoodPos+=val;
					}
				}
				LOG(4,<< "pos= " << pos << " LofPos= " << likelihoodComputation::getLofPos(pos,_et,_scSSRV,_pijSSRV,ssrvSp) << " likelihoodPos= " << likelihoodPos << endl);
				testLikelihood+=log(likelihoodPos);
				probAcceleration/=likelihoodPos;
				probDeceleration/=likelihoodPos;
				probNoRateShift/=likelihoodPos;
				
				data->addPositionData(pos,convert(probAcceleration),convert(probDeceleration));
				
				LOG(12,<<"position: "<< pos << " probDecelartaion= " << probDeceleration);
				LOG(12,<<" probAcceleration= " << probAcceleration );
				LOG(12,<<" probNoRateShift= " << probNoRateShift << endl);
				MDOUBLE sumProbPos= convert(probDeceleration)+convert(probAcceleration)+convert(probNoRateShift);
				MDOUBLE epsilon = 0.000001;
				if ((sumProbPos + epsilon < 1.0) || (sumProbPos > 1.0 + epsilon))
					LOG(4,<<" rateShiftProbs4branch::fillBranchesData() ERROR !!! sumProbPos = " << sumProbPos << endl);
			}
			_branchesData[mynode->id()] = data;
		}
		else
		{ // only for debug (we don't really need to check the likelihood of the root in order to detect rate shifts)
			LOG(8,<< '\t' << "root: nodeId = " << mynode->id() << endl);
			for (pos=0; pos < _scSSRV.seqLen(); ++pos) {
				likelihoodPos=0.0;
				for(letter=0; letter<alphabetSSRV->size();++letter) {
					val= ssrvSp.freq(letter) * _cupSSRV.get(pos,mynode->id(),letter);
					likelihoodPos+=val;
				}
				testLikelihood += log(likelihoodPos);
			}
		}

		if (testLikelihood != likelihood)
			LOG(4,<<" rateShiftProbs4branch::fillBranchesDataSSRVWithDebug ERROR !!! likelihood= " << likelihood << " testLikelihood= " << testLikelihood << endl);
	}
}

doubleRep rateShiftProbs4branch::calcVal(int id, int pos, int letter, int fatherLetter,
									const stochasticProcess& sp)
{
	doubleRep downVal(0.0),upVal(0.0),val(0.0);
	MDOUBLE pijVal(0.0),freqVal(0.0);

	freqVal = sp.freq(fatherLetter);
	pijVal = _pijSSRV.getPij(id,fatherLetter,letter);
	upVal = _cupSSRV.get(pos,id,letter);
	// note: this is the downAlg result for the father of mynode
	downVal = _cdownSSRV.get(pos,id,fatherLetter);
	LOG(9,<< "letter: " << letter << " fatherLetter: " << fatherLetter);
	LOG(9,<< " freqVal: " << freqVal << " upVal= " << upVal << " downVal = " << downVal );
	val= freqVal*upVal*downVal*pijVal;
	LOG(9,<< " val= " << val << endl);
	return val;
}


void rateShiftProbs4branch::fillBranchesDataUSSRV()
{
	treeIterDownTopConst tIt(_et);
	int letter,fatherLetter,pos;
	const mulAlphabet* alphabetSSRV = static_cast<const mulAlphabet*>(_scSSRV.getAlphabet());
	const stochasticProcessSSRV ssrvSp = _model.getSSRVmodel();
	MDOUBLE f = _model.getF();

//	LOG(8,<<"pos" << '\t' << "nodeId" << '\t' << "LofPosBase" << '\t' << "LofPosSSRV" << '\t' << "f" << '\t' << "likelihoodPos" << '\t' << "probAcceleration" << '\t' << "probDeceleration" << '\t' << "probNoRateShift" << endl);
	for (tree::nodeP mynode = tIt.first(); mynode != tIt.end(); mynode = tIt.next()) {
		doubleRep val(0.0),LofPosSSRV(0.0),LofPosBase(0.0), likelihoodPos(0.0);
		if (mynode->isRoot() == false)
		{
			branchData* data = new branchData(mynode->id(),mynode->dis2father());
			for (pos=0; pos < _scSSRV.seqLen(); ++pos) {
				LofPosSSRV=0.0;
				LofPosBase = likelihoodComputation::getLofPos(pos,_et,_baseSc,_pijBase,_model.getBaseModel());
				doubleRep probAcceleration(0.0),probDeceleration(0.0),probNoRateShift(0.0);
				for(letter=0; letter<alphabetSSRV->size();++letter) {
					for (fatherLetter=0; fatherLetter<alphabetSSRV->size();++fatherLetter) {						
						val = calcVal(mynode->id(),pos,letter,fatherLetter,ssrvSp);
						int categoryCompare = alphabetSSRV->compareCategories(fatherLetter,letter);
						switch (categoryCompare) {
							case -1: probDeceleration+= f * val; break;
							case 0: probNoRateShift+= f * val; break;
							case 1: probAcceleration+= f * val; break;
						}
						LofPosSSRV+=val;
					}
				}
				likelihoodPos = (LofPosSSRV * f) + ((1-f)*LofPosBase);
				probNoRateShift+=(1-f)*LofPosBase;
//				LOG(8,<< pos << '\t' << mynode->id() << '\t' << LofPosBase << '\t' << LofPosSSRV << '\t' << f << '\t' << likelihoodPos << '\t' << probAcceleration << '\t' << probDeceleration << '\t' << probNoRateShift << '\t');

				probAcceleration/=likelihoodPos;
				probDeceleration/=likelihoodPos;
				probNoRateShift/=likelihoodPos;
				//LOG(8,<< probAcceleration << '\t' << probDeceleration << '\t' << probNoRateShift << endl);
				data->addPositionData(pos,convert(probAcceleration),convert(probDeceleration));
				
				LOG(12,<<"position: "<< pos << " probDecelartaion= " << probDeceleration);
				LOG(12,<<" probAcceleration= " << probAcceleration );
				LOG(12,<<" probNoRateShift= " << probNoRateShift << endl);
				MDOUBLE sumProbPos= convert(probDeceleration)+convert(probAcceleration)+convert(probNoRateShift);
				MDOUBLE epsilon = 0.000001;
				if ((sumProbPos + epsilon < 1.0) || (sumProbPos > 1.0 + epsilon))
					LOG(4,<<" rateShiftProbs4branch::fillBranchesDataUSSRV ERROR !!! sumProbPos = " << sumProbPos << endl);
			}
			_branchesData[mynode->id()] = data;
		}
	}	
}

// all the testLikelihood calculation here and the likelihoodComputation2USSRV::getTreeLikelihoodAllPosAlphTheSame
// is just for debug OZ
void rateShiftProbs4branch::fillBranchesDataUSSRVWithDebug()
{
	treeIterDownTopConst tIt(_et);
	int letter,fatherLetter,pos;
	const mulAlphabet* alphabetSSRV = static_cast<const mulAlphabet*>(_scSSRV.getAlphabet());
	const stochasticProcessSSRV ssrvSp = _model.getSSRVmodel();
	MDOUBLE likelihood = likelihoodComputation2USSRV::getTreeLikelihoodAllPosAlphTheSame(_et,_scSSRV,_baseSc,_model);
	MDOUBLE f = _model.getF();
	for (tree::nodeP mynode = tIt.first(); mynode != tIt.end(); mynode = tIt.next()) {
		MDOUBLE testLikelihood(0.0);
		doubleRep likelihoodPos(0.0),val(0.0),LofPosSSRV(0.0),LofPosBase(0.0);
		if (mynode->isRoot() == false)
		{
			cout << mynode->id() << endl;
			branchData* data = new branchData(mynode->id(),mynode->dis2father());
			LOG(8,<< '\t' << "nodeId = " << mynode->id() << " isleaf?: " << mynode->isLeaf() << endl);
			for (pos=0; pos < _scSSRV.seqLen(); ++pos) {
				LofPosSSRV=0.0;
				LofPosBase = likelihoodComputation::getLofPos(pos,_et,_baseSc,_pijBase,_model.getBaseModel());
				doubleRep probAcceleration(0.0),probDeceleration(0.0),probNoRateShift(0.0);
				for(letter=0; letter<alphabetSSRV->size();++letter) {
					for (fatherLetter=0; fatherLetter<alphabetSSRV->size();++fatherLetter) {						
						val= calcVal(mynode->id(),pos,letter,fatherLetter,ssrvSp);
						int categoryCompare = alphabetSSRV->compareCategories(fatherLetter,letter);
						switch (categoryCompare) {
							case -1: probDeceleration+= f*val; break;
							case 0: probNoRateShift+= f*val; break;
							case 1: probAcceleration+= f*val; break;
						}
						LofPosSSRV+=val;
					}
				}
				likelihoodPos = (LofPosSSRV * f) + ((1-f)*LofPosBase);
				LOG(4,<< "pos= " << pos << " real LofPosSSRV= " << likelihoodComputation::getLofPos(pos,_et,_scSSRV,_pijSSRV,ssrvSp) << " calculated LofPosSSRV= " << LofPosSSRV << endl);
				testLikelihood+=log(likelihoodPos);

				probNoRateShift+=(1-f)*LofPosBase;

				probAcceleration/=likelihoodPos;
				probDeceleration/=likelihoodPos;
				probNoRateShift/=likelihoodPos;
				
				data->addPositionData(pos,convert(probAcceleration),convert(probDeceleration));
				
				LOG(12,<<"position: "<< pos << " probDecelartaion= " << probDeceleration);
				LOG(12,<<" probAcceleration= " << probAcceleration );
				LOG(12,<<" probNoRateShift= " << probNoRateShift << endl);
				MDOUBLE sumProbPos= convert(probDeceleration)+convert(probAcceleration)+convert(probNoRateShift);
				MDOUBLE epsilon = 0.000001;
				if ((sumProbPos + epsilon < 1.0) || (sumProbPos > 1.0 + epsilon))
					LOG(4,<<" rateShiftProbs4branch::fillBranchesDataUSSRVWithDebug() ERROR !!! sumProbPos = " << sumProbPos << endl);
			}
			_branchesData[mynode->id()] = data;
		}
		else
		{ // only for debug (we don't really need to check the likelihood of the root in order to detect rate shifts)
			LOG(8,<< '\t' << "root: nodeId = " << mynode->id() << endl);
			for (pos=0; pos < _scSSRV.seqLen(); ++pos) {
				likelihoodPos=0.0;
				LofPosBase = likelihoodComputation::getLofPos(pos,_et,_baseSc,_pijBase,_model.getBaseModel());
				for(letter=0; letter<alphabetSSRV->size();++letter) {
					val= ssrvSp.freq(letter) * _cupSSRV.get(pos,mynode->id(),letter);					
					likelihoodPos+= val;
				}
				likelihoodPos*=f;
				likelihoodPos+= (1-f)*LofPosBase;
				testLikelihood += log(likelihoodPos);
			}
		}

		if (testLikelihood != likelihood)
			LOG(4,<<" rateShiftProbs4branch::fillBranchesDataUSSRVWithDebug() ERROR !!! likelihood= " << likelihood << " testLikelihood= " << testLikelihood << endl);
	}
}


void rateShiftProbs4branch::printBranchesData(ostream& out) const
{
	vector<branchData*>::const_iterator itr = _branchesData.begin();
	for (; itr != _branchesData.end(); ++itr) {
		if (*itr != NULL) {
			(*itr)->printBranchData(out);
			out << endl;
		}
	}
}

void rateShiftProbs4branch::printPositions4EachBranch(ostream& out) const
{
	vector<branchData*>::const_iterator itr = _branchesData.begin();
	for (; itr != _branchesData.end(); ++itr) {
		if (*itr != NULL)
			out << **itr ;
	}
}
// Set alpha at a level determined by how great a risk of a Type I error (falsely rejecting a true null) you are willing to take
void rateShiftProbs4branch::printBinomialSignificantBranches(ostream& out,MDOUBLE alpha/*=0.1*/) const
{
	vector<branchData*>::const_iterator itr = _branchesData.begin();
	out << "****    Binomial Significant Branches    ****" << endl;
	out << "branch" << '\t' << "pvalue" << endl;
	for (; itr != _branchesData.end(); ++itr) {
		if (*itr != NULL) {
			MDOUBLE pvalue = (*itr)->getBinomialPvalue();
			if ( pvalue < alpha) 
				out << (*itr)->getNodeId() << '\t' << pvalue << endl;
		}
	}
}

// Set alpha at a level determined by how great a risk of a Type I error (falsely rejecting a true null) you are willing to take
void rateShiftProbs4branch::printPoissonSignificantBranches(ostream& out,MDOUBLE alpha/*=0.1*/) const
{
	vector<branchData*>::const_iterator itr;
	out << "****    Poisson Significant Branches    ****" << endl;
	
	out << "****	Ampiric Lamda	****" << endl;
	// debug OZ
	cout << "****	Ampiric Lamda	****" << endl;
	cout << "branch" << '\t' << "pvalue" << endl;
	// end of debug
	out << "branch" << '\t' << "pvalue" << endl;
	for (itr = _branchesData.begin(); itr != _branchesData.end(); ++itr) {
		if (*itr != NULL) {
			MDOUBLE pvalue = (*itr)->getPoissonPvalue4AmpiricLamda();
			// debug OZ
			cout << (*itr)->getNodeId() << '\t' << pvalue << endl;
			// end of debug
			if (( pvalue < alpha) && (pvalue>0)) // #### currently pvalue is set to 0 when the branch length == 0.
				out << (*itr)->getNodeId() << '\t' << pvalue << endl;
		}
	}

	out << "****	Theortical Lamda	****" << endl;
	// debug OZ
	cout << "****	Theortical Lamda	****" << endl;
	cout << "branch" << '\t' << "pvalue" << endl;
	// end of debug
	out << "branch" << '\t' << "pvalue" << endl;
	for (itr = _branchesData.begin(); itr != _branchesData.end(); ++itr) {
		if (*itr != NULL) {
			MDOUBLE pvalue = (*itr)->getPoissonPvalue4TheoreticalLamda();
			// debug OZ
			cout << (*itr)->getNodeId() << '\t' << pvalue << endl;
			// end of debug
			if (( pvalue < alpha) && (pvalue>0)) // #### currently pvalue is set to 0 when the branch length == 0.
				out << (*itr)->getNodeId() << '\t' << pvalue << endl;
		}
	}

}

// for each branch, the jumps are according to binomial distribution B(l/L , J).
// The binomial p = l/L , where l is the branch length, and L is the total length of the tree.
// The binomial n = J, where J is the total jumps in the tree.
// Denote by j = the number of jumps observed in the branch.
void rateShiftProbs4branch::calculateBinomialPvalues()
{
	vector<branchData*>::iterator itr;
	// calculate the number of total jumps in the tree and the total length of the tree
	int totalNumJumps(0); // this is actually the binomial n = J.
	MDOUBLE totalTreeLength(0.0); // denote by L.
	for (itr = _branchesData.begin(); itr != _branchesData.end(); ++itr)
	{
		if (*itr == NULL)
			continue;
		totalNumJumps += (*itr)->getNumJumps();
		totalTreeLength += (*itr)->getBranchLength();
	}

	if (totalNumJumps <= 30)
		LOG(4,<< "rateShiftProbs4branch::calculateBinomialPvalues totalNumJumps = " << totalNumJumps << " is too small for the Normal Approximation " << endl);

	// cacluate the pValue for each branch, and set it in the branchData.
	// Use the Normal Approximation of a Binomial Probability
	for (itr = _branchesData.begin(); itr != _branchesData.end(); ++itr)
	{
		if (*itr == NULL)
			continue;
		
		MDOUBLE binomialP = (*itr)->getBranchLength() / totalTreeLength; // l/L
		MDOUBLE expectation = binomialP*totalNumJumps; // p*n
		MDOUBLE std = sqrt(totalNumJumps*binomialP*(1-binomialP)); // sqrt(n*p*(1-p))
		MDOUBLE zScore = (((*itr)->getNumJumps()-0.5) - expectation) / std; // (j-0.5-expectation)/std . I reduce 0.5 from j for the correction for continuity
		(*itr)->setBinomialPvalue(1.0 - Phi(zScore));
	}
}

void rateShiftProbs4branch::calculatePoissonPvaluesAmpiricLamda()
{
	cout << "rateShiftProbs4branch::calculatePoissonPvaluesAmpiricLamda" << endl;
	vector<branchData*>::iterator itr;
	MDOUBLE totalTreeLength(0.0);
	// calculate lamda of the whole tree
	MDOUBLE lamda(0.0);
	for (itr = _branchesData.begin(); itr != _branchesData.end(); ++itr)
	{
		if (*itr == NULL)
			continue;
		MDOUBLE branchLength = (*itr)->getBranchLength(); 
		if (branchLength>0){
			lamda += (*itr)->getSumOfRateShiftProb();
//			lamda += (*itr)->getRateShiftProb();
			totalTreeLength += branchLength;
		}
	}
	lamda /= totalTreeLength;

	// calculate the pValue for each branch and set it in the branchData.
	for (itr = _branchesData.begin(); itr != _branchesData.end(); ++itr)
	{
		if (*itr == NULL)
			continue;
		(*itr)->calculatePoissonPvalue(lamda,0);
	}
}

void rateShiftProbs4branch::calculatePoissonPvaluesTheoreticalLamda()
{
	cout << "rateShiftProbs4branch::calculatePoissonPvaluesTheoreticalLamda" << endl;
	vector<branchData*>::iterator itr;
	MDOUBLE totalTreeLength(0.0);
	for (itr = _branchesData.begin(); itr != _branchesData.end(); ++itr)
	{
		if (*itr == NULL)
			continue;
		MDOUBLE branchLength = (*itr)->getBranchLength(); 
		totalTreeLength += branchLength;
	}

	MDOUBLE numCategories = _model.noOfCategor();
	MDOUBLE theoreticalLamda = (_model.getNu() * (numCategories-1)) / numCategories;
	theoreticalLamda /= totalTreeLength;
//	theoreticalLamda *= _scSSRV.seqLen();
	// calculate the pValue for each branch and set it in the branchData.
	for (itr = _branchesData.begin(); itr != _branchesData.end(); ++itr)
	{
		if (*itr == NULL)
			continue;
		(*itr)->calculatePoissonPvalue(theoreticalLamda,1);
	}
}

//void rateShiftProbs4branch::debug()
//{
//	allocatePlaceSSRV(); 
//	_pijSSRV.fillPij(_et,_model.getBaseModel(),0);
//	computeUpSSRV();
//	computeDownSSRV();
//	treeIterDownTopConst tIt(_et);
//
//	int letter,fatherLetter,pos;
//	const alphabet* alphabetBase = _scSSRV.getAlphabet();
//	const stochasticProcess sp = _model.getBaseModel();
//
//		MDOUBLE likelihood = likelihoodComputation::getTreeLikelihoodAllPosAlphTheSame(_et,_scSSRV,sp);
//		for (tree::nodeP mynode = tIt.first(); mynode != tIt.end(); mynode = tIt.next()) {
//		MDOUBLE testLikelihoodPos(0.0),testLikelihood(0.0),upVal(0.0),downVal(0.0),freqVal(0.0),val(0.0),pijVal(0.0);
//		if (mynode->isRoot() == false)
//		{		
//			LOG(8,<< '\t' << "nodeId = " << mynode->id() << endl);
//			for (pos=0; pos < _scSSRV.seqLen(); ++pos) {
//				testLikelihoodPos=0.0;
//				for(letter=0; letter<alphabetBase->size();++letter) {
//					for (fatherLetter=0; fatherLetter<alphabetBase->size();++fatherLetter) {						
//						freqVal = sp.freq(fatherLetter);
//						upVal = _cupSSRV.get(pos,mynode->id(),letter);
////						downVal = _cdownSSRV.get(pos,mynode->father()->id(),fatherLetter);
//						downVal = _cdownSSRV.get(pos,mynode->id(),fatherLetter);
//						pijVal = _pijSSRV.getPij(mynode->id(),fatherLetter,letter);
//						LOG(8,<< "letter: " << letter << " fatherLetter: " << fatherLetter);
//						LOG(8,<< " freqVal: " << freqVal << " upVal= " << upVal << " downVal = " << downVal << " pijVal = " << pijVal );
//						val = freqVal*upVal*downVal*pijVal;
//						LOG(8,<< " val= " << val << endl);
//						testLikelihoodPos+=val;
//					}
//				}
//				LOG(4,<< "pos= " << pos << " LofPos= " << likelihoodComputation::getLofPos(pos,_et,_scSSRV,_pijSSRV,sp) << " testLikelihoodPos= " << testLikelihoodPos << endl);
//				testLikelihood+=log(testLikelihoodPos);
//			}
//		}
//		else
//		{
//			LOG(8,<< '\t' << "root: " << "nodeId = " << mynode->id() << endl);
//			testLikelihood =0;
//			for (pos=0; pos < _scSSRV.seqLen(); ++pos) {
//				MDOUBLE verytemp=0.0;
//				for(letter=0; letter<alphabetBase->size();++letter) {
//					val= sp.freq(letter);
//					val*= _cupSSRV.get(pos,mynode->id(),letter);
//					verytemp+=val;
//				}
//				testLikelihood += log(verytemp);
//			}	
//		}
//		
//		if (testLikelihood != likelihood)
//			LOG(4,<<" rateShiftProbs4branch::debug() ERROR !!! ");
//			LOG(4,<<"likelihood= " << likelihood << " testLikelihood= " << testLikelihood << endl);
//	}
//}
