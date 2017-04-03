// 	$Id: branchData.h 2399 2014-03-13 22:43:51Z wkliao $	
#ifndef BRANCH_DATA
#define BRANCH_DATA

#include "someUtil.h"
#include "posData.h"
#include "normalDist.h"
#include "errorMsg.h"
#include <cmath>

class branchData
{
public:
	// @@@@ to what should we set the default jumpTreshold ???
	branchData(int nodeId,MDOUBLE length,MDOUBLE jumpTreshold=0.5) : 
	  _nodeId(nodeId), _length(length),_jumpTreshold(jumpTreshold), _numJumps(0), _binomialPvalue(-1)
	  , _poissonPvalue4TheoreticalLamda(-1), _poissonPvalue4AmpiricLamda(-1) {}
	
	// for position 
	// @@@@ maybe should be doubleRep
	void addPositionData(int pos, MDOUBLE probAcceleration, MDOUBLE probDeceleration); 
	
	// for the whole branch
	MDOUBLE getAccelerationProb() const; // returns the average of all positions
	MDOUBLE getDecelerationProb() const; // returns the average of all positions
	MDOUBLE getSumOfRateShiftProb() const;
	MDOUBLE getRateShiftProb() const; // returns the average of all positions
	MDOUBLE getNoRateShiftProb() const { return 1.0-getRateShiftProb();}
	MDOUBLE getBranchLength() const {return _length;}
	MDOUBLE getBinomialPvalue() const {return _binomialPvalue;}
	MDOUBLE getPoissonPvalue4TheoreticalLamda() const {return _poissonPvalue4TheoreticalLamda;}
	MDOUBLE getPoissonPvalue4AmpiricLamda() const {return _poissonPvalue4AmpiricLamda;}
	void setBinomialPvalue(MDOUBLE binomialPvalue) { _binomialPvalue = binomialPvalue;}
	int getNumJumps() const {return _numJumps;}
	int getNodeId() const { return _nodeId; }
	friend ostream& operator<<(ostream &out, const branchData &branch);
	ostream& printBranchData(ostream &out) const;

	void calculatePoissonPvalueNoApproximation(const MDOUBLE& lamda, int type);
	void calculatePoissonPvalueNormalAproximation(const MDOUBLE& lamda, int type);
	void calculatePoissonPvalue (const MDOUBLE& lamda, int type);  



private:
	int _nodeId; // the branch is from the node to its father
	MDOUBLE _length; // the length of the branch
	MDOUBLE _jumpTreshold; // The probability of rate-shift in a site in this branch, above which we consider it as a jump.
	int _numJumps; // The number of sites involved in a rateShift according to the _jumpTreshold.
	MDOUBLE _binomialPvalue; // The pValue of jump in this branch. It's calculated in class rateShiftProbs4branch
	vector<posDataBranchShift> _positionsData; // size = number of positions

	MDOUBLE _poissonPvalue4TheoreticalLamda;
	MDOUBLE _poissonPvalue4AmpiricLamda;
	
};

#endif // BRANCH_DATA
