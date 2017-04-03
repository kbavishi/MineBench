// $Id: bblEM2codon.cpp 2399 2014-03-13 22:43:51Z wkliao $
#include "bblEM2codon.h"
#include "likelihoodComputation.h"
#include "likelihoodComputation2Codon.h"
using namespace likelihoodComputation;
using namespace likelihoodComputation2Codon;
#include "computeUpAlg.h"
#include "computeDownAlg.h"
#include "computeCounts2Codon.h"
#include "treeIt.h"
#include "fromCountTableComponentToDistance2Codon.h"
#include "errorMsg.h"
#include "logFile.h"
#include <ctime>

bblEM2codon::bblEM2codon(tree& et,
				const sequenceContainer& sc,
				const empiriSelectionModel &model,
				const Vdouble * weights,
				const int maxIterations,
				const MDOUBLE epsilon,
				const MDOUBLE tollForPairwiseDist) :
	_et(et),_sc(sc),_model(model),_weights (weights) {
	
	LOG(5,<<"******BBL EM*********"<<endl<<endl);
	_treeLikelihood = compute_bblEM(maxIterations,epsilon,tollForPairwiseDist);
}


MDOUBLE bblEM2codon::compute_bblEM(
			const int maxIterations,
			const MDOUBLE epsilon,
			const MDOUBLE tollForPairwiseDist){
	allocatePlace();
	MDOUBLE oldL = VERYSMALL;
	MDOUBLE currL = VERYSMALL;
	tree oldT = _et;
	for (int i=0; i < maxIterations; ++i) {
		computeUp();
		currL = likelihoodComputation2Codon::getTreeLikelihoodFromUp2(_et,_sc,_model,_cup,_posLike,_weights);
		//////////////
		LOG(5,<<"iteration no "<<i << " in BBL "<<endl);
		LOG(5,<<"old best  L= "<<oldL<<endl);
		LOG(5,<<"current best  L= "<<currL<<" = "<<likelihoodComputation2Codon::getTreeLikelihoodAllPosAlphTheSame(_et,_sc,_model)<<endl<<endl);
	
	
		//oldT = _et;
		if (currL < oldL + epsilon) { // need to break
			if (currL<oldL) {
				cout<<"******** PROBLEMS IN BBL *********"<<endl;
				LOG(5,<<"The like in if = "<<likelihoodComputation2Codon::getTreeLikelihoodAllPosAlphTheSame(_et,_sc,_model)<<endl);
				_et = oldT;
				return oldL; // keep the old tree, and old likelihood
			} else {
                //update the tree and likelihood and return
				LOG(5,<<"The like in else  = "<<likelihoodComputation2Codon::getTreeLikelihoodAllPosAlphTheSame(_et,_sc,_model)<<endl);
				return currL;
			}
		}
		oldT = _et;
		bblEM_it(tollForPairwiseDist);
		oldL = currL;
	}
	// in the case were we reached max_iter, we have to recompute the likelihood of the new tree...
	computeUp();
	currL = likelihoodComputation2Codon::getTreeLikelihoodFromUp2(_et,_sc,_model,_cup,_posLike,_weights);
	if (currL<oldL) {
		_et = oldT;
		return oldL; // keep the old tree, and old likelihood
	} 
	else 
        return currL;
}

void bblEM2codon::allocatePlace() {
	_computeCountsV.resize(_et.getNodesNum()); //initiateTablesOfCounts
	for (int i=0; i < _computeCountsV.size(); ++i) {
		_computeCountsV[i].countTableComponentAllocatePlace(_model.alphabetSize(),_model.noOfCategor()+1);
	}
	_cup.allocatePlace(_sc.seqLen(),_model.noOfCategor()+1, _et.getNodesNum(), _sc.alphabetSize());
	_cdown.allocatePlace(_model.noOfCategor()+1, _et.getNodesNum(), _sc.alphabetSize());

}

void bblEM2codon::bblEM_it(const MDOUBLE tollForPairwiseDist){
	for (int i=0; i < _computeCountsV.size(); ++i) {
		_computeCountsV[i].zero();
	}
	for (int i=0; i < _sc.seqLen(); ++i) {
		computeDown(i);
		addCounts(i); // computes the counts and adds to the table.
	}
	optimizeBranches(tollForPairwiseDist);
}

void bblEM2codon::optimizeBranches(const MDOUBLE tollForPairwiseDist){
	treeIterDownTopConst tIt(_et);
	for (tree::nodeP mynode = tIt.first(); mynode != tIt.end(); mynode = tIt.next()) {
		if (!tIt->isRoot()) {
			fromCountTableComponentToDistance2Codon from1(_computeCountsV[mynode->id()],_model,tollForPairwiseDist,mynode->dis2father());
			from1.computeDistance();
			mynode->setDisToFather(from1.getDistance());
		}
	}
}

void bblEM2codon::computeUp(){
	_pij._V.resize(_model.noOfCategor()+1);
	int i;
	for (i=0; i < _model.noOfCategor(); ++i) {
		_pij._V[i].fillPij(_et,_model.getCategor(i));
	}
	_pij._V[i].fillPij(_et,_model.getNoSelectionModel());

	computeUpAlg cupAlg;
	for (int pos=0; pos < _sc.seqLen(); ++pos) {
		for (int categor = 0; categor < _model.noOfCategor()+1; ++categor) {
			cupAlg.fillComputeUp(_et,_sc,pos,_pij[categor],_cup[pos][categor]);
		}
	}

	
	

 }

void bblEM2codon::computeDown(const int pos){
	computeDownAlg cdownAlg;
	for (int categor = 0; categor < _model.noOfCategor()+1; ++categor) {
		cdownAlg.fillComputeDown(_et,_sc,pos,_pij[categor],_cdown[categor],_cup[pos][categor]);
		
	}
}

void bblEM2codon::addCounts(const int pos){
	//MDOUBLE posProb = 
	//	likelihoodComputation::getProbOfPosWhenUpIsFilledGam(pos,_et,_sc,_sp,_cup);
						
	MDOUBLE weig = (_weights ? (*_weights)[pos] : 1.0);
	if (weig == 0) return;
	treeIterDownTopConst tIt(_et);
	for (tree::nodeP mynode = tIt.first(); mynode != tIt.end(); mynode = tIt.next()) {
		if (!tIt->isRoot()) {
			addCounts(pos,mynode,_posLike[pos],weig);
		}
	}
}

void bblEM2codon::addCounts(const int pos, tree::nodeP mynode, const MDOUBLE posProb, const MDOUBLE weig){

	computeCounts2Codon cc;
	int categor;
	for (categor =0; categor< _model.noOfCategor(); ++ categor) {
			cc.computeCountsNodeFatherNodeSonHomPos(_sc, 
										_pij[categor],
										_model,
										_cup[pos][categor],
										_cdown[categor],
										weig,
										posProb,
										mynode,
										_computeCountsV[mynode->id()][categor],
										_model.getCategorProb(categor)*_model.getF());
	
	}
	
	cc.computeCountsNodeFatherNodeSonHomPos(_sc, 
										_pij[categor],
										_model,
										_cup[pos][categor],
										_cdown[categor],
										weig,
										posProb,
										mynode,
										_computeCountsV[mynode->id()][categor],
										(1-_model.getF()));
}          

