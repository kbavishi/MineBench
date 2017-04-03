// 	$Id: posData.cpp 2399 2014-03-13 22:43:51Z wkliao $	
#include "posData.h"

ostream& operator<<(ostream &out, const posData &pos)
{
	out << left<<setw(5) << pos._num;
	out << left<<setw(5) << pos._content;
	out << right<<setw(4) << pos._numOfSeqsWithoutGaps << "/" << left<<setw(4)<<pos._numOfSeqs;
	return out;
}

ostream& operator<<(ostream &out, const posDataSSRV &pos)
{
	out << (posData)pos;
	out << left << setprecision(4) << setw(7) << fixed << pos._probSSRV;
	return out;
}


int posDataSSRV::operator<(const posDataSSRV other) const {
	if (_probSSRV == other._probSSRV)
		return (_num < other._num);
	return (_probSSRV < other._probSSRV);
}


posDataBranchShift::posDataBranchShift(int num, MDOUBLE probAcceleration, MDOUBLE probDeceleration) :
	_num(num),
	_probAcceleration(probAcceleration),
	_probDeceleration(probDeceleration)
{}

// format:  posNum	probAcceleration	probDeceleration	probRateShift
// posNum in the output starts from 1, not from 0. 
ostream& operator<<(ostream &out, const posDataBranchShift &pos)
{
	out << left<<setw(5) << pos._num+1;
	out << left << setprecision(4) << setw(7) << fixed << pos._probAcceleration;
	out << left << setprecision(4) << setw(7) << fixed << pos._probDeceleration;
	out << left << setprecision(4) << setw(7) << fixed << pos.getProbRateShift();
	return out;
}
