// 	$Id: statisticalScore.cpp 2399 2014-03-13 22:43:51Z wkliao $	
// StatisticalScorer.cpp: implementation of the StatisticalScorer class.
//
//////////////////////////////////////////////////////////////////////

#include "statisticalScore.h"
//#include "FDR.h"
#include <algorithm>




void StatisticalScore::applyLRT(Vdouble kaksVec, Vdouble likesVec, const sequence & refSeq){
	
	assert(kaksVec.size() == likesVec.size());;
	assert(kaksVec.size() == _sc.seqLen());
	

	seqContainerTreeMap tmap(_sc,_t);
	suffStatGlobalHomPos ssc;
	ssc.allocatePlace(_t.getNodesNum(),_nullSp.alphabetSize());
	int noOfSites = _sc.seqLen();
	_significantTypeVec.resize(noOfSites);
	_likeDiffVec.resize(noOfSites);
	_pValues.resize(noOfSites);
	_kaksVec.resize(noOfSites);
	_kaksVec = kaksVec;
	int s;

	computePijHom pi; 
	pi.fillPij(_t,_nullSp,_sc.alphabetSize()); 
	
	

	for (s=0;s<noOfSites;s++){
		MDOUBLE res = applyLRTSite(s,tmap,ssc,pi);
	//	cout<<"pos model = "<<likesVec[s]<<" null model = " << res<<" different = "<<(likesVec[s]-res)*2<<endl;		
		_pValues[s] = getPVal((likesVec[s]-res)*2,ONE_DF);
		bool sig = areSignificant(_pValues[s]);
		_significantTypeVec[s] = significantType(sig,kaksVec[s]);
		_likeDiffVec[s] = (likesVec[s]-res)*2;
	}

	
	string fdrInput = "FDR_input.txt";
	string fdrOut = "FDR_output.txt";
//	outToFDR(fdrInput,fdrOut,refSeq);
//	FDR fdr;
//fdr.ApplyFDR();

}


//the function return log(likelihood) under the no selection model.
MDOUBLE StatisticalScore::applyLRTSite(const int site,  seqContainerTreeMap &tmap,
									   suffStatGlobalHomPos &ssc,
										const computePijHom &pi){


	treeIterDownTopConst tIt(_t);
	for (tree::nodeP mynode = tIt.first(); mynode != tIt.end(); mynode = tIt.next()) {
		int letter;
		if (mynode->getNumberOfSons() == 0) {// leaf
			for(letter=0; letter<_nullSp.alphabetSize();letter++) {
				const int seqID = tmap.seqIdOfNodeI(mynode->id());
				MDOUBLE val = _sc.getAlphabet()->relations(_sc[seqID][site],letter);
				ssc.set(mynode->id(),letter,val);
			}
		}
		else {
			for(letter=0; letter<_nullSp.alphabetSize();letter++) {
				MDOUBLE total_prob=1.0;
				for(int i=0; i < mynode->getNumberOfSons();++i){				
					MDOUBLE prob=0.0;
					for(int letInSon=0; letInSon<_nullSp.alphabetSize();letInSon++) {
						prob +=	ssc.get(mynode->getSon(i)->id(), letInSon)*
								pi.getPij(mynode->getSon(i)->id(),letter,letInSon);
						
					}
					assert(prob>=0);
					total_prob*=prob;
				}
				ssc.set(mynode->id(),letter,total_prob);
			}
		}
	}
				
		
	MDOUBLE res = log(likelihoodComputation::getProbOfPosWhenUpIsFilledHom(site,_t,_sc,_nullSp,ssc));

	return res;


}


sigType StatisticalScore::significantType(const bool significant, const MDOUBLE kaks){

	if (significant==true){
		if (kaks>1) return posSig;
		else return negativeSig;
	}
	
	return  nonSig;

}


void StatisticalScore::outToServer(string fileName,const sequence & refSc){

	ofstream out(fileName.c_str());
	assert(_significantTypeVec.size() == refSc.seqLen());
	amino aminoAcid;
	vector<int> color4Site = getColorVec(refSc); //this vector hold only colors for the reference's sites
	int gap=0;

	for (int i=0;i<refSc.seqLen();i++){	 
		int aa = codonUtility::aaOf(refSc[i],_co);
		if (aa==-1){
			gap++;
			continue;
		}
		string aaStr = aminoAcid.fromInt(aa);
		out<<i+1-gap <<" "<<aaStr<<" "<<color4Site[i-gap] <<endl;
	}

	/*for (int i=0;i<_significantTypeVec.size();i++){	 
		int aa = codonUtility::aaOf(refSc[i]);
		string aaStr = aminoAcid.fromInt(aa);
		out<<i+1 <<" "<<aaStr<<" "<<_significantTypeVec[i] <<endl;
	}*/
		
	out.close();

}


void StatisticalScore::outStatisticScores(string fileName,const sequence & refSc){
	ofstream out(fileName.c_str());
	assert(_significantTypeVec.size() == refSc.seqLen());
	amino aminoAcid;
	int gap=0;
	
	out<<"// "<<" amino "<<" ka/ks "<<" pValue "<<" " << " likeDiff " <<endl;
	for (int i=0;i<_significantTypeVec.size();i++){	 
		int aa = codonUtility::aaOf(refSc[i],_co);  //For Idit
	/*	if (aa == -1) { // in case of a gap --> continue
			gap++;
			continue;
		}*/
		string aaStr = aminoAcid.fromInt(aa);
		out<<i-gap+1 <<" "<<aaStr<<" "<< _kaksVec[i]<<" "<<_pValues[i]<<" "<< _likeDiffVec[i]<<endl;
	
	}
		
	out.close();
}

/*	
MDOUBLE StatisticalScore::getPVal(const double p_dVal, const ChiSquareDF p_DF){
	int noOfPvalues = 5;
	for (int i = 0;i<noOfPvalues;i++){
		if (ChiSquareTable[p_DF][i] >=  p_dVal)
			break;	
	}

	if (i==1)
		return 0.1;
	if (i==2)
		return 0.05;
	if (i==3)
		return 0.025;
	if (i==4)
		return 0.01;
	if (i==5)
		return 0.005;

	return -1;

	return chiSquareProb(p_dVal,p_DF);
}*/


vector<int> StatisticalScore::getColorVec(const sequence & refSeq) const{
	vector<int> colors;
	Vdouble negativesKaksVec,negativesSite;
	int i,gap=0;
	for ( i=0;i<_significantTypeVec.size();i++){
		if (codonUtility::aaOf(refSeq[i],_co) == -1) gap++; 
		
	}

	colors.resize(_kaksVec.size()-gap);
	gap=0;

	for (i=0;i<_significantTypeVec.size();i++){
		if (codonUtility::aaOf(refSeq[i],_co) == -1){
			gap++;
			continue;
		}
		if (_significantTypeVec[i] == posSig)
			colors[i-gap]=1;
		else if (_significantTypeVec[i] == negativeSig)
			colors[i-gap]=7;
		else if (_kaksVec[i]>1)
			colors[i-gap]=2;
		else  {
			//negativesKaksVec.push_back(_kaksVec[i]);  //add the value of kaks
			negativesKaksVec.push_back(_pValues[i]); //add value of pvalue.
			negativesSite.push_back(i-gap);   //add the number of site of the kaks 
		}
	
	}

	Vdouble orderVec = negativesKaksVec;
	sort(orderVec.begin(), orderVec.end());  //sort the kaks values to be divided to 4 groups
	MDOUBLE percentileNum = 4.0;
	int percentileNumInt = 4;

	Vdouble maxScoreForPercentile(percentileNumInt);
	
	maxScoreForPercentile[0] = orderVec[0];
	
	for (int c = 1; c < percentileNumInt; ++c){
		int place =((c / percentileNum) * negativesKaksVec.size());
		MDOUBLE maxScore = orderVec[place];
		maxScoreForPercentile[c] = maxScore;
	}

	//loop over all the kaks < 1 that not significant 
	for (int j=0; j < negativesKaksVec.size(); ++j){
			MDOUBLE r = negativesKaksVec[j]; //the kaks of the site.
			int s = negativesSite[j];  //the  site.
			if (r > maxScoreForPercentile[3]) 
					colors[s] = 3;
			else if (r> maxScoreForPercentile[2])
					colors[s] = 4;
			else if (r > maxScoreForPercentile[1])
					colors[s] = 5;
			else if (r >= maxScoreForPercentile[0])
					colors[s] = 6;
	}
	return colors;


}


//the function create file to be used in the FDR statistic test
//and the paramet file as an input to the FDR.
void StatisticalScore::outToFDR(string fdrInput,string fdrOut, const sequence & refSc){
	ofstream out(fdrInput.c_str());
	assert(_significantTypeVec.size() == refSc.seqLen());
	amino aminoAcid;

	for (int i=0;i<_significantTypeVec.size();i++){	 
		int aa = codonUtility::aaOf(refSc[i],_co);
		string aaStr = aminoAcid.fromInt(aa);
		out<<i+1 <<" "<<aaStr<<" "<< _kaksVec[i]<<" "<<_pValues[i]<<endl;
	}
		
	out.close();

	string optionsFile = "program.options";
	ofstream out2(optionsFile.c_str());
	out2<<"[IO]"<<endl<<endl<<"# the name of the file that FDR takes as input"<<endl;
	out2<<"FDRInputFile = "<<fdrInput<<endl<<endl;
	out2<<"# the name of the file that FDR takes as input"<<endl;
	out2<<"FDROutputFile  = "<<fdrOut<<endl<<endl;
	out2<<"[FDR]"<<endl<<endl;
	out2<<"# the significance level taken by the FDR calculation"<<endl<<"FDRSignificanceLevel = 0.05"<<endl;
	out2.close();

}




