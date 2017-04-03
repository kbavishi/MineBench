// $Id: weightVectors.h 2399 2014-03-13 22:43:51Z wkliao $

#ifndef ___WEIGHTS_VECTORS
#define ___WEIGHTS_VECTORS

#include "definitions.h"

class weightVectors {
public:
	explicit weightVectors(istream &infile); // return the number of vector read
	
	//This construction just allocates the place. The starting weights are 1.0
	//a second call to fill random weights is needed.
	explicit weightVectors(const int length, const int size); 
	
	int size() const {return _weights.size();}
	int length() const {return ((_weights.size() == 0) ? 0 : _weights[0].size());}

	// this function fills the weight vectors according to the standard bootsrap method
	void randomizeBPweights(); 

private:
	VVdouble _weights;
	int read(istream &infile); // return the number of vector read
	void checkLengthAndSize(const int length, const int size); 

};




#endif

