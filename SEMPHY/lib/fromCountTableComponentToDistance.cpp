// $Id: fromCountTableComponentToDistance.cpp 2399 2014-03-13 22:43:51Z wkliao $

#include "fromCountTableComponentToDistance.h"
#include "likeDist.h"
#include <cassert>

fromCountTableComponentToDistance::fromCountTableComponentToDistance(
		const countTableComponentGam& ctc,
		const stochasticProcess &sp,
		const MDOUBLE toll,
		const MDOUBLE brLenIntialGuess ) : _sp(sp), _ctc(ctc) {
	_distance =brLenIntialGuess ;//0.03;
	_toll = toll;
}

void fromCountTableComponentToDistance::computeDistance() {
	likeDist likeDist1(_sp,_toll);
	MDOUBLE initGuess = _distance;
	_distance = likeDist1.giveDistance(_ctc,_likeDistance,initGuess);
	assert(_distance>=0);
}
