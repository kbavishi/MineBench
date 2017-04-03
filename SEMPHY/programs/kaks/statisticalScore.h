// 	$Id: statisticalScore.h 2399 2014-03-13 22:43:51Z wkliao $	
#ifndef STATISTICALSCORE_H
#define STATISTICALSCORE_H

#include "definitions.h"
#include "errorMsg.h"
#include <iostream>
#include <fstream>

#include "sequenceContainer.h"
#include "stochasticProcess.h"
#include "goldmanYangModel.h"
#include "computeUpAlg.h"
#include "likelihoodComputation.h"
#include "seqContainerTreeMap.h"
#include "amino.h"
#include "codon.h"
#include "gammaUtilities.h"

// Degrees of Freedom
enum ChiSquareDF
{
	ONE_DF = 1,
	TWO_DF = 2,
	THREE_DF = 3,
	FOUR_DF = 4
};

// 5 differnet p-values (confidence levels) are used.
// For readability, they're given names in this enumeration
/*enum ChiSquarePVal
{
	CHI_SQUARE_PVAL_0_1 = 0,
	CHI_SQUARE_PVAL_0_05 = 1,
	CHI_SQUARE_PVAL_0_025 = 2,
	CHI_SQUARE_PVAL_0_01 = 3,
	CHI_SQUARE_PVAL_0_005 = 4
};*/

// this partial chi-square table contains only four rows, for 1 2,3 and 4 DF,
// and for each of them it contains the values for 5 various p-values:
// 1. p = 0.1 
// 2. p = 0.05 
// 3. p = 0.025
// 4. p = 0.01 
// 5. p = 0.005
// 
// The values for this hard-coded initialization are taken from 
// http://home.comcast.net/~fellmanstat/HowToReadTheChiSquareTable.pdf
// where one can find a bigger table as well as explanation about how it should be read.
// Additionally, a very good explanation about the Likelihood Ratio Test and the usage
// of chi-square for it can be found at 
// http://www.itl.nist.gov/div898/handbook/apr/section2/apr233.htm
/*static double ChiSquareTable[4][5] =
{
//PVAL=	0.1	   0.05	  0.025  0.01 0.005
	{ 2.706, 3.841,  5.02,  6.635, 7.88 }, //ONE_DF
	{ 4.605,  5.991,  7.378,  9.21,   10.597 }, // TWO_DF 
	{6.251, 7.815, 9.35, 11.345 ,12.84},// THREE_DF
	{ 7.779,  9.488,  11.143, 13.277, 14.86 },  // FOUR_DF
	
};*/

//three categorios of significant for:
// 0 = significant positive selection.
// 1= significant negative  selection.
// 2= non significant.
//the significant is according a cut off p-Value (0.05 is the default)
enum sigType {posSig=1, negativeSig, nonSig}; 

//////////////////////////////////////////////////////////////////////
// StatisticalScorer class definition 
//////////////////////////////////////////////////////////////////////

class StatisticalScore  {
public:
		
	explicit StatisticalScore(const tree &t,const sequenceContainer &sc,const stochasticProcess& nullSp,const codon &co,MDOUBLE pval = 0.05):
	_t(t),_sc(sc),_nullSp(nullSp),_co(co),_chiSquarePVAL(pval) {};
	
	
	// Apply the Likelihood Ratio Test (LRT) to each site, for testing
	// which of them is a positive selection, negative selection or not natural(and not significant)
	// The following stages are performed:
	// 1. calculate  for each site the L(natural) -> log likelihood under the natural model: c = 0.
	// 2. calculate 2*(L(not-natural)- L(natural) ) , L(not-natural) values per site were calculated
	// in MLKaKs4Site.
	// 3. compare 2dL with  chi square distribution.
	// 4. dividing site to 3 categoris: significat-positive,significat-negative, notsignificant.
	// 5. dividing the significat-negative to three more categoris: according it's kaks values.
	// 6. output the sites with it's categories.
	
	void applyLRT(Vdouble kaksVec, Vdouble likesVec,const sequence & refSeq);

	MDOUBLE applyLRTSite(const int site,  seqContainerTreeMap & tmap, 
						suffStatGlobalHomPos & ssc,const computePijHom &pi);


	// If the given value (2 * ln(LR)) is larger than the value in the chi-square table,
	// the meaning is that we deal with 2 distinct distributions, in our case - significant
	// positive or negative selection.
	// Alternatively, if the given value is smaller than the value in the chi-square table,
	// we know that we deal with one distribution, that is - the site we're examining
	// is NOT significant.
	//
	// See http://www.itl.nist.gov/div898/handbook/apr/section2/apr233.htm for a nice explanation.
	//
	// This method examines the chi-square table, according to the given parameters 
	// (degrees of freedom and p-value), and returns:
	// 1. true - the given p-value is smaller then _chiSquarePVAL cut off--> significant
	// 2. false - the given p-value is greater then _chiSquarePVAL cut off --> NOT significant
	
	bool areSignificant(const double p_Val)
	{	return ( p_Val <= _chiSquarePVAL);	}

	MDOUBLE getPVal(const double p_dVal, const ChiSquareDF p_DF){
		return chiSquareProb(p_dVal,p_DF);	
	}

	
	//the function get kaks value and if significant
	// if true and kaks>1 the type will be significant postive selction
	// if true and kaks<1 the type will be significant negative selction.
	// else return not significant.
	static sigType significantType(const bool significant, const MDOUBLE kaks);

	// compute the p-value of given a value (2* (log(likelihhood_1)-log(likeloihood_0)) - chiSquare
	//with df  - freedom degree. 
	// this function uses the incopmlete gamma fumctio: gammq from numerical recipes (pg.226)
	MDOUBLE chiSquareProb(const MDOUBLE chiSquare, const MDOUBLE df){
		return gammq(df/2.0,chiSquare/2.0);
	}

	/////////////////////
	//outputs functions//
	/////////////////////

	void outToServer(string fileName,const sequence & refSc);

	void outStatisticScores(string fileName,const sequence & refSc);

	vector<int>  getColorVec(const sequence & refSc) const;

	void outToFDR(string fdrInput,string fdrOut ,const sequence & refSc);

private:
	const tree & _t;
	const sequenceContainer & _sc;
	const stochasticProcess & _nullSp;
	MDOUBLE _chiSquarePVAL; 
	vector<sigType> _significantTypeVec;
	Vdouble _likeDiffVec;
	Vdouble _pValues;
	Vdouble _kaksVec;
	const codon &_co;
};

#endif
