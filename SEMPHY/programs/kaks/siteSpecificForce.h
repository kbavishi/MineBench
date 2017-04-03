// 	$Id: siteSpecificForce.h 2399 2014-03-13 22:43:51Z wkliao $	
#ifndef ___SITE_SPECIFIC_FORCE
#define ___SITE_SPECIFIC_FORCE

#include "definitions.h"
#include "JTTcodonModel.h"
#include "likelihoodComputation.h"
#include "optimizeFanctors.h"
#include "wYangModel.h"
#include "empirSelectionModel.h"


MDOUBLE computeML_siteSpecificKaKs(Vdouble & KaKsV,
								   Vdouble & likelihoodsV,
								   const sequenceContainer& sc,
								   const stochasticProcess& sp,
								   const tree& et,
								   const MDOUBLE maxKaKs=20.0,
								   const MDOUBLE tol=0.0001f);


void computeML_siteSpecificKaKs(int pos,
								 const sequenceContainer& sc,
								 const stochasticProcess& sp,
								 const tree &et,
								 MDOUBLE& bestKaKs,
								 MDOUBLE& posL,
								 const MDOUBLE maxKaKs,
								 const MDOUBLE tol) ;


void computeEB_EXP_siteSpecificKaks(int pos,
								 const sequenceContainer& sc,
								 const empiriSelectionModel & model,
								 const computePijGam& cpg,
								 const tree &et,
								 MDOUBLE& bestForce,
								 MDOUBLE & stdForce,
								 MDOUBLE & lowerConf,
								 MDOUBLE & upperConf,
								 const MDOUBLE alphaConf);


void computeEB_EXP_siteSpecificKaKs(Vdouble & forcesV,
								   Vdouble & stdV,
								   Vdouble & lowerBoundV,
								   Vdouble & upperBoundV,
								   const sequenceContainer& sc,   
								   const empiriSelectionModel & model,
								   const tree& et,
								   const MDOUBLE alphaConf= 0.05);



#endif
