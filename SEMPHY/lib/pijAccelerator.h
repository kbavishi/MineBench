// $Id: pijAccelerator.h 2399 2014-03-13 22:43:51Z wkliao $

#ifndef ___PIJ_ACCELERATOR
#define ___PIJ_ACCELERATOR

#include "definitions.h"
#include "replacementModel.h"

class pijAccelerator {
public:
	virtual pijAccelerator* clone() const = 0;
	virtual ~pijAccelerator() = 0;
	virtual const MDOUBLE Pij_t(const int i, const int j, const MDOUBLE t) const = 0;
	virtual const MDOUBLE freq(const int i) const = 0;	// P(i)
	virtual const MDOUBLE dPij_dt(const int i, const int j, const MDOUBLE t) const =0;
	virtual const MDOUBLE d2Pij_dt2(const int i, const int j, const MDOUBLE t) const =0;
	virtual replacementModel* getReplacementModel() const =0; // @@@@ this const is a lie !!!
	virtual const int alphabetSize() const =0;
};





#endif

