// $Id: fromCountTableComponentToDistance2Codon.cpp 2399 2014-03-13 22:43:51Z wkliao $

#include "fromCountTableComponentToDistance2Codon.h"
#include "likeDist2Codon.h"
#include "likeDist.h"
#include <cassert>

fromCountTableComponentToDistance2Codon::fromCountTableComponentToDistance2Codon(
		const countTableComponentGam& ctc,
		const empiriSelectionModel &model,
		const MDOUBLE toll,
		const MDOUBLE brLenIntialGuess ) : _model(model), _ctc(ctc) {
	_distance = brLenIntialGuess ;//0.03;
	_toll = toll;
}

void fromCountTableComponentToDistance2Codon::computeDistance() {
	likeDist2Codon likeDist1(_model,_toll);
	MDOUBLE initGuess = _distance;
	_distance = likeDist1.giveDistance(_ctc,_likeDistance,initGuess);
	assert(_distance>=0);
}
