// 	$Id: posData.h 2399 2014-03-13 22:43:51Z wkliao $	
#ifndef POS_DATA
#define POS_DATA

#include "someUtil.h"
#include <iostream>
#include <iomanip>

class posData {
public:
	posData(int num,const string& content, int numOfSeqsWithoutGaps, int numOfSeqs) : 
	  _num(num), _content(content), _numOfSeqsWithoutGaps(numOfSeqsWithoutGaps), _numOfSeqs(numOfSeqs) {}

	friend ostream& operator<<(ostream &out, const posData &pos);

protected:
	int _num; // the number of the position in the sequence (starting from 1)
	string _content; // the alphabet content of the position in the reference sequence
	int _numOfSeqsWithoutGaps;
	int _numOfSeqs;
};



class posDataSSRV : public posData {

public:
	posDataSSRV(int num, const string& content, int numOfSeqsWithoutGaps, int numOfSeqs,MDOUBLE probSSRV) :
	  posData(num,content,numOfSeqsWithoutGaps,numOfSeqs), _probSSRV(probSSRV) {}

	friend ostream& operator<<(ostream &out, const posDataSSRV &pos);

	int operator<(const posDataSSRV other) const;

private:
	MDOUBLE _probSSRV; 
};


// compare two pointers to posDataSSRV
class cmpPosDataSSRVPointers {

public:
	int operator()(const posDataSSRV* p1, const posDataSSRV* p2) const {return (*p2)<(*p1);}
	
};


class posDataBranchShift {
public:
	posDataBranchShift(int num, MDOUBLE probAcceleration, MDOUBLE probDeceleration);
	
	MDOUBLE getProbAcceleration() const {return _probAcceleration;}
	MDOUBLE getProbDeceleration() const {return _probDeceleration;}
	MDOUBLE getProbRateShift() const {return _probAcceleration+_probDeceleration;}
	MDOUBLE getProbNoRateShift() const {return 1.0-getProbRateShift();}

	friend ostream& operator<<(ostream &out, const posDataBranchShift &pos);

private:
	int _num;
	MDOUBLE _probAcceleration;
	MDOUBLE _probDeceleration;
};

#endif // POS_DATA
