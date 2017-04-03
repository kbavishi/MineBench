// $Id: fromCountTableComponentToDistance2Codon.h 2399 2014-03-13 22:43:51Z wkliao $

#ifndef ___FROM_COUNT_TABLE_COMPONENT_TO_DISTANCE_2_CODON
#define ___FROM_COUNT_TABLE_COMPONENT_TO_DISTANCE_2_CODON

#include "definitions.h"
#include "countTableComponent.h"
#include "stochasticProcess.h"
#include "empirSelectionModel.h"

static const MDOUBLE startingGuessForTreeBrLen = 0.029;

class fromCountTableComponentToDistance2Codon {

public:
	explicit fromCountTableComponentToDistance2Codon(
		const countTableComponentGam& ctc,
		const empiriSelectionModel &model,
		const MDOUBLE toll,
		const MDOUBLE brLenIntialGuess);// =startingGuessForTreeBrLen

	void computeDistance();// return the likelihood
	MDOUBLE getDistance() { return _distance;} // return the distance.
	MDOUBLE getLikeDistance() { return _likeDistance;} // return the distance.
private:
	const empiriSelectionModel & _model;
	const countTableComponentGam& _ctc;
	MDOUBLE _toll;
	MDOUBLE _distance;
	MDOUBLE _likeDistance;
	int alphabetSize() {return _ctc.alphabetSize();}
};

#endif

