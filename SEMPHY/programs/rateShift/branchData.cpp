// 	$Id: branchData.cpp 2399 2014-03-13 22:43:51Z wkliao $	
#include "branchData.h"

void branchData::addPositionData(int pos, MDOUBLE probAcceleration, MDOUBLE probDeceleration)
{
	if (probAcceleration+probDeceleration > _jumpTreshold)
		_numJumps++;
	_positionsData.push_back(posDataBranchShift(pos, probAcceleration, probDeceleration));
}

// average over the positions
MDOUBLE branchData::getAccelerationProb() const
{
	vector<posDataBranchShift>::const_iterator itr = _positionsData.begin();
	MDOUBLE sumPositionsAccelerationProb(0.0);
	for (; itr != _positionsData.end(); ++itr)
		sumPositionsAccelerationProb += itr->getProbAcceleration();

	return (sumPositionsAccelerationProb/_positionsData.size());
}

// average over the positions
MDOUBLE branchData::getDecelerationProb() const
{
	vector<posDataBranchShift>::const_iterator itr = _positionsData.begin();
	MDOUBLE sumPositionsDecelerationProb(0.0);
	for (; itr != _positionsData.end(); ++itr)
		sumPositionsDecelerationProb += itr->getProbDeceleration();

	return (sumPositionsDecelerationProb/_positionsData.size());
}

MDOUBLE branchData::getSumOfRateShiftProb() const
{
	vector<posDataBranchShift>::const_iterator itr = _positionsData.begin();
	MDOUBLE sumPositionsRateShiftProb(0.0);
	for (; itr != _positionsData.end(); ++itr) {
		sumPositionsRateShiftProb += itr->getProbAcceleration();
		sumPositionsRateShiftProb += itr->getProbDeceleration();
	}
	return sumPositionsRateShiftProb;
}

// average over the positions
MDOUBLE branchData::getRateShiftProb() const
{
	return (getSumOfRateShiftProb() /_positionsData.size());
}

ostream& branchData::printBranchData(ostream &out) const
{
	out << "branch: " << left << setw(5) << _nodeId ;
	out << left << setprecision(4) << setw(7) << fixed << getAccelerationProb();
	out << left << setprecision(4) << setw(7) << fixed << getDecelerationProb();
	out << left << setprecision(4) << setw(7) << fixed << getRateShiftProb()  ;
	out << left << setprecision(4) << setw(7) << fixed << _length  ;
	return out;
}
ostream& operator<<(ostream &out, const branchData &branch)
{
	vector<posDataBranchShift>::const_iterator itr = branch._positionsData.begin();
	for (;itr!= branch._positionsData.end(); ++itr) {
		out << left << setw(5) << branch._nodeId ;
		out << *itr << endl;
	}
	
	return out;
}

// Poisson P-values : 


void branchData::calculatePoissonPvalue(const MDOUBLE& lamda, int type)
{
	if (lamda<0)
		errorMsg::reportError("branchData::calculatePoissonPvalue, lamda<0");
	
	// real poisson calculation. no approximation
	if (lamda*_length <= 20) 
		calculatePoissonPvalueNoApproximation(lamda,type);
	// Normal approximation
	else
		calculatePoissonPvalueNormalAproximation(lamda,type);
}

// For large values of lamda (in this case lamda*branchLength), 
// say over 20, one may use the Normal approximation to calculate Poisson probabilities.  (http://home.ubalt.edu/ntsbarsh/Business-stat/opre504.htm)
// type = 0 --> Ampiric Lamda
// type = 1 --> Theoretical Lamda
void branchData::calculatePoissonPvalueNormalAproximation(const MDOUBLE& lamda, int type)
{
	if (lamda<0)
		errorMsg::reportError("branchData::calculatePoissonPvalueNormalAproximation, lamda<0");
	if (lamda*_length <= 20) cout << "ERROR: Normal Approximation is not valid" << endl;
	MDOUBLE pvalue;
	MDOUBLE sumOfRateShiftProb = getSumOfRateShiftProb();
//	MDOUBLE obserevedLamda = getRateShiftProb();
	// The bbl sometimes creates branches with length = 0
	if (_length == 0)
	{
		pvalue = -1; 
	}
	else
	{
		cout << "id= " << _nodeId <<" sumOfRateShiftProb : " << sumOfRateShiftProb << " lamda*_length: " << lamda*_length << endl;
		MDOUBLE std = sqrt(lamda*_length);
		MDOUBLE zScore = (sumOfRateShiftProb - (lamda*_length)) / std; 
		pvalue = 1.0 - Phi(zScore);
		cout << "zScore: " << zScore << " pvalue: " << pvalue << endl;
	}

	if (type ==0)
		_poissonPvalue4AmpiricLamda=pvalue;
	else if (type == 1)
		_poissonPvalue4TheoreticalLamda=pvalue;

}


 //this uses the real poisson distribution and not the normal approximation.
 //The problam is that th observed lamda for each length is an int and not a double...
// The solution: sumOfRateShiftProb ~ Poisson(lamda*branch length)
// type = 0 --> Ampiric Lamda
// type = 1 --> Theoretical Lamda
void branchData::calculatePoissonPvalueNoApproximation(const MDOUBLE& lamda,int type)
{
	if (lamda<0)
		errorMsg::reportError("branchData::calculatePoissonPvalue, lamda<0");
	
	MDOUBLE pvalue;
	MDOUBLE sumOfRateShiftProb = getSumOfRateShiftProb();
//	MDOUBLE rateShiftProb = getRateShiftProb();
	// The bbl sometimes creates branches with length = 0
	if (_length != 0)
	{
		cout << "id= " << _nodeId << " sumOfRateShiftProb: " << sumOfRateShiftProb << " lamda*_length: " << lamda*_length << endl;
//		cout << "id= " << _nodeId << " rateShiftProb: " << rateShiftProb << " lamda*_length: " << lamda*_length << endl;
	    pvalue = copmutePoissonProbability((int)sumOfRateShiftProb,lamda*_length); // #### watch the casting. May cause problems because of the need of factorial of a large number
	}
	else
		pvalue = -1; // #### I should think of another solution for branches with length=0

	if (type ==0)
		_poissonPvalue4AmpiricLamda=pvalue;
	else if (type == 1)
		_poissonPvalue4TheoreticalLamda=pvalue;
}
