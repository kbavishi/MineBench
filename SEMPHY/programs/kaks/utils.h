// 	$Id: utils.h 2399 2014-03-13 22:43:51Z wkliao $	
#ifndef UTILS_H
#define UTILS_H

#include <iostream>
#include "nucleotide.h"
#include "codon.h"
#include "amino.h"
#include "nucleotide.h"
#include "logFile.h"
#include "fastaFormat.h"
#include "clustalFormat.h"
#include "recognizeFormat.h"
#include "someUtil.h"
#include "definitions.h"
#include "sequenceContainer.h"
#include "stochasticProcess.h"
#include "wYangModel.h"

#include "likelihoodComputation.h"
#include "trivialAccelerator.h"
#include "datMatrixHolder.h"
#include "JTTcodonModel.h"
#include "uniDistribution.h"
#include "gammaDistribution.h"
#include "tree.h"
#include "evaluateCharacterFreq.h"
using namespace std;


void printHelp();
sequenceContainer readCodonSeqs(string codonFile, codon *co);
void checkInputSeqLength(string codonFile);
sequenceContainer convertCodonToAmino(sequenceContainer &codonSc,const codon &co);
vector<vector<int> > create7ColorValues();
void outToRasmolFile(string fileName,vector<int>& color4Site);

void normalizMatrices(vector<stochasticProcess> & spVec,const distribution * forceDistr);
void changDoubleToString (string& ioString, const MDOUBLE inValue);

Vdouble freqCodonF3x4(sequenceContainer &codonSc,const codon &co);
Vdouble fromFreqCodonToFreqAmino(Vdouble & codonFreq, codon &co);
void kaks2Color(const Vdouble & kaksVec, const sequence & refSeq,string fileName,const codon &co);


void printCodonTable(codon & co);
void printMatrix(const VVdouble & mat);
void printVec(const Vdouble & vec);


#endif
