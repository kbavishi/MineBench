// 	$Id: utils.cpp 2399 2014-03-13 22:43:51Z wkliao $	
#include "utils.h"
#include <algorithm>

void printHelp(){

		cout <<"+----------------------------------------------+"<<endl;
		cout <<"|-b    output list of sites and number of bin  |"<<endl;
		cout <<"|-c    input Codon seq file must be align      |"<<endl;
		cout <<"|      (accepted formats: Fasta, Mase, Molphy, |"<<endl;
		cout <<"|       Phylip, Clustal)                       |"<<endl;
		cout <<"|-g    output file of global results           |"<<endl;
		cout <<"|-i    output file with scores and p-values    |"<<endl;
		cout <<"|-l    output log file                         |"<<endl;
		cout <<"|-r    name of reference sequence              |"<<endl;
		cout <<"|-s    output script to rasTop                 |"<<endl;
		cout <<"|-t    output file of tree                     |"<<endl;
		cout <<"|-u    input User Tree (optional)              |"<<endl;				
		cout <<"|----------------------------------------------|"<<endl;
		cout <<"|-h or -? or -H     help                       |"<<endl;
		cout <<"|capital and no captial letters are ok         |"<<endl;
		cout <<"+----------------------------------------------+"<<endl;
}

//read cono file and fill sequence container the sequences must be aligned, 
//divisable by three and not include stop codon.
sequenceContainer readCodonSeqs(string codonFile, codon *co){
	checkInputSeqLength(codonFile); //check if the sequences divided by three
	sequenceContainer codonSc;
	ifstream in(codonFile.c_str());
	codonSc = recognizeFormat::readUnAligned(in, co);
	codonSc.changeGaps2MissingData();
	in.close();
	int i,j;
	bool endStop = false; //check if there is stop codon in the end of the sequence if exist remove this position
	//check For STOP codons	
    for (i = 0; i < codonSc.numberOfSeqs(); ++i){
		for (j = 0; j < codonSc[i].seqLen() - 1; ++j){
			if (codonSc[i][j]>co->size()&& codonSc[i][j]<64){	//Stop Codon 
				string textToPrint = "USER ERROR: unable to read sequence: " + codonSc[i].name() + "\nStop codon in the sequence";
				cout<<j<<" "<<codonSc[i][j]<<endl;
				errorMsg::reportError(textToPrint);			
			}
		}
		if (codonSc[i][j]>co->size() && codonSc[i][j]<64) //check if the last position is stop codon
			endStop = true;
	}
	
	if (endStop){ 
		Vint removesPos(codonSc.seqLen(),0);
		removesPos[codonSc.seqLen()-1] = 1;
		codonSc.removePositions(removesPos);
	}
	return codonSc;
}

//check that the input sequences are divisable by 3
void checkInputSeqLength(string codonFile){
	nucleotide alph;
	ifstream in(codonFile.c_str());
	sequenceContainer inputSc = recognizeFormat::readUnAligned(in, &alph);
	in.close();
	int i;
    for (i = 0; i < inputSc.numberOfSeqs(); ++i){
		int seqLen = inputSc[i].seqLen();
		if ((seqLen % 3) != 0){
			string textToPrint = "USER ERROR: unable to read sequence: " + inputSc[i].name() + "\nSequence length is not divisable by three";
			errorMsg::reportError(textToPrint);
		}
	}
}

//this function convert codon sequences to amino sequences.
sequenceContainer convertCodonToAmino(sequenceContainer &codonSc, const codon &co){
	amino aaAlph;
	sequenceContainer aaSc;
	for (int i = 0; i < codonSc.numberOfSeqs(); ++i){
		sequence codonSeq = codonSc[i];
		sequence aaSeq("", codonSeq.name(), codonSeq .remark(), codonSeq.id(), &aaAlph);
		for (int pos = 0; pos < codonSeq .seqLen(); ++pos)
            aaSeq.push_back(codonUtility::aaOf(codonSeq[pos],co));
		aaSc.add(aaSeq);
	}
	if (codonSc.numberOfSeqs() != aaSc.numberOfSeqs())
		errorMsg::reportError("RevTrans: number of codon and Amino sequences is not the same");
	
	return aaSc;
}




//positive significant in color yellow , negative significant in
//color red the others sites dividing according to pvalues
vector<vector<int> > create7ColorValues(){
	vector<vector<int> > colorsValue;
	colorsValue.resize(7);
	for (int i=0;i<7;i++)
		colorsValue[i].resize(3);

	colorsValue[0][0] = 255; //yellow positive significant
	colorsValue[0][1] = 220 ;
	colorsValue[0][2] = 0;

	colorsValue[1][0] =255 ; //light yellow - not  significant positive selection
	colorsValue[1][1] = 255;
	colorsValue[1][2] = 120;
	
	//three categorious of not significant negative selection according colors like conseq

	colorsValue[2][0] = 255; //white  
	colorsValue[2][1] = 255;
	colorsValue[2][2] = 255;

	colorsValue[3][0] = 252;
	colorsValue[3][1] = 237;
	colorsValue[3][2] = 244;

	colorsValue[4][0] = 250;
	colorsValue[4][1] = 201;
	colorsValue[4][2] = 222;

	colorsValue[5][0] = 240;
	colorsValue[5][1] = 125;
	colorsValue[5][2] = 171;
	
	//significant negative selection
	colorsValue[6][0] = 130; 
	colorsValue[6][1] = 67;
	colorsValue[6][2] = 96;

	return colorsValue;
}

//this function write rasmol script
void outToRasmolFile(string fileName,vector<int>& color4Site){
	ofstream out(fileName.c_str());	
	vector<vector<int> > colorsValue = create7ColorValues();
	int numberOfColor = colorsValue.size();
	vector<vector<int> > colors; //for each color (1-9/3) holds vector of sites.
	colors.resize(numberOfColor+1);
	int i;
	for (i=0;i<color4Site.size();i++){
		int color=color4Site[i];
		if (color>numberOfColor){
			errorMsg::reportError("Error in outToColorFile - unknown color");
		}
		colors[color].push_back(i+1); //add site (position in the vector +1)	
	}
	out<<"select all"<<endl;
	out<<"color [200,200,200]"<<endl<<endl;
	
	for (int c=1;c<numberOfColor+1;c++){
		out<<"select ";
		for (i=0;i<colors[c].size();i++){
			if (i==0)
				out<<colors[c][i];
			else if ((i+1)%6==0)
				out<<endl<<"select selected or "<<colors[c][i];
			 
			else out<<" , "<<colors[c][i];
		}
		out<<endl<<"select selected and :a"<<endl;
		out<<"color [" <<colorsValue[c-1][0]<<","<<colorsValue[c-1][1]<<","<<colorsValue[c-1][2]<<"]"<<endl;
		out<<"spacefill"<<endl<<endl;
	}
	
	out.close();
}


void normalizMatrices(vector<stochasticProcess> & spVec,const distribution * forceDistr){
	MDOUBLE sumPijQij=0.0;
	for (int categor=0; categor<forceDistr->categories();categor++)
		//sumPijQij+=forceDistr->ratesProb(categor)*static_cast<wYangModel*>(spVec[categor].getPijAccelerator()->getReplacementModel())->sumPijQij();	
		sumPijQij+=forceDistr->ratesProb(categor)*static_cast<jttCodonModel*>(spVec[categor].getPijAccelerator()->getReplacementModel())->sumPijQij();	
	assert(sumPijQij!=0);
	for (int categor=0; categor<forceDistr->categories();categor++)
		//static_cast<wYangModel*>(spVec[categor].getPijAccelerator()->getReplacementModel())->norm(1/sumPijQij);
		static_cast<jttCodonModel*>(spVec[categor].getPijAccelerator()->getReplacementModel())->norm(1/sumPijQij);	
}


Vdouble freqCodonF3x4(sequenceContainer &nucSc,const codon &co){
	VVdouble nucFeqPos(3);
	int pos= 0;
	int nPos = 0;
	for (nPos=0;nPos<3;nPos++)
		nucFeqPos[nPos].resize(nucSc.alphabetSize(),0.0);

	sequenceContainer::constTaxaIterator tIt;
	sequenceContainer::constTaxaIterator tItEnd;
	tIt.begin(nucSc);
	tItEnd.end(nucSc);
	while (tIt!= tItEnd) {
		pos = 0;
		sequence::constIterator sIt;
		sequence::constIterator sItEnd;
		sIt.begin(*tIt);
		sItEnd.end(*tIt);
		while (sIt != sItEnd) {
			if ((*sIt >= 0) && (*sIt <nucFeqPos[pos%3].size())) ++nucFeqPos[pos%3][(*sIt)];
			if (*sIt == 4) ++nucFeqPos[pos%3][3]; //for T (4) to U (3)
			++sIt;
			++pos;
		}
		++tIt;
	}
	for (nPos=0;nPos<3;nPos++)
		changeCountsToFreqs(nucFeqPos[nPos]);
	
	int alphaCodon = co.size();
	Vdouble freqCodon(alphaCodon,0.0);
	nucleotide n;
	MDOUBLE sum = 0;
	for (int c = 0; c<freqCodon.size();c++){
		
		string s = co.fromInt(c);
		int nuc0 = n.fromChar(s[0]);
		int nuc1 = n.fromChar(s[1]);
		int nuc2 = n.fromChar(s[2]);
		freqCodon[c] = nucFeqPos[0][nuc0]*nucFeqPos[1][nuc1]*nucFeqPos[2][nuc2];
		sum +=freqCodon[c];
	}
	int T = n.fromChar('T'),G = n.fromChar('G'),A = n.fromChar('A'),C = n.fromChar('C');
	
	MDOUBLE stopFreq  = 1.0 - sum;
	MDOUBLE  ep = stopFreq/alphaCodon;
	cout<<" T	G	A	C"<<endl;
	cout<< "pos 1 "<<nucFeqPos[0][T]<<" "<<nucFeqPos[0][G]<<" "<<nucFeqPos[0][A]<< " "<<nucFeqPos[0][C];
	cout<<endl<< "pos 2 "<<nucFeqPos[1][T]<<" "<<nucFeqPos[1][G]<<" "<<nucFeqPos[1][A]<< " "<<nucFeqPos[1][C];
	cout<<endl<< "pos 3 "<<nucFeqPos[2][T]<<" "<<nucFeqPos[2][G]<<" "<<nucFeqPos[2][A]<< " "<<nucFeqPos[2][C];
	
	MDOUBLE epsilon = 1e-005;
	sum = 0;
	for (int i=0;i<alphaCodon;i++){
		freqCodon[i] += ep;
		sum+=freqCodon[i];
	}
	if (fabs(sum - 1.0)> epsilon){
		cout<<"******ERROR freqCodonF3x4 **********" <<fabs(sum - 1.0)<<endl;
		exit(0);
	}
	
	return freqCodon;


}


Vdouble fromFreqCodonToFreqAmino(Vdouble & codonFreq,  codon &co){
	int i;
	Vdouble aminoFreq(20,0.0);
	for (i=0;i<aminoFreq.size();i++){		
		vector<int> codonOfI = aminoUtility::codonOf(i,co);
		int size = codonOfI.size();
		for (int j=0; j<size; j++)
			aminoFreq[i]+= codonFreq[codonOfI[j]];

	}
	MDOUBLE sum=0,epsilon = 1e-005;
	for (i=0;i<20;i++)
		sum+=aminoFreq[i];
	if (fabs(sum - 1.0)> epsilon){
		cout<<"******ERROR fromFreqCodonToFreqAmino **********" <<fabs(sum - 1.0)<<endl;
		exit(0);
	}
	return aminoFreq;
}

//this function get the kaks values of each site and reference to sequnence 
//it create file of colors to be used in the server for rasmol script.
void kaks2Color(const Vdouble & kaksVec, const sequence & refSeq, string fileName,const codon &co) {
	vector<int> colors;
	int vecSize = kaksVec.size();
	Vdouble negativesKaksVec;
	Vint negativesSite;
	int i,gap=0;
	for ( i=0;i<vecSize;i++){
		if (codonUtility::aaOf(refSeq[i],co) == -1) gap++; 
		
	}

	colors.resize(vecSize-gap);
	gap=0;
	MDOUBLE veryPos = 1.5;

	for (i=0;i<vecSize;i++){
		if (codonUtility::aaOf(refSeq[i],co) == -1){
			gap++;
			continue;
		}
		if (kaksVec[i] > veryPos)
			colors[i-gap]=1;
		else if (kaksVec[i]>1)
			colors[i-gap]=2;
		else  {
			negativesKaksVec.push_back(kaksVec[i]);  //add the value of kaks < 1
			negativesSite.push_back(i-gap);   //add the number of site of the kaks 
		}
	
	}

	Vdouble orderVec = negativesKaksVec;
	sort(orderVec.begin(), orderVec.end());  //sort the kaks values to be divided to 5 groups
	MDOUBLE percentileNum = 5.0;
	int percentileNumInt = 5;

	Vdouble maxScoreForPercentile(percentileNumInt);
	
	if (orderVec.size()>0){
		maxScoreForPercentile[0] = orderVec[0];
		for (int c = 1; c < percentileNumInt; ++c){
			int place =(int)((c / percentileNum) * negativesKaksVec.size());
			MDOUBLE maxScore = orderVec[place];
			maxScoreForPercentile[c] = maxScore;
		}
	}

	//loop over all the kaks < 1  
	for (int j=0; j < negativesKaksVec.size(); ++j){
			MDOUBLE r = negativesKaksVec[j]; //the kaks of the site.
			int s = negativesSite[j];  //the  site.
			if (r > maxScoreForPercentile[4]) 
					colors[s] =3;
			else if (r > maxScoreForPercentile[3]) 
					colors[s] = 4;
			else if (r> maxScoreForPercentile[2])
					colors[s] = 5;
			else if (r > maxScoreForPercentile[1])
					colors[s] = 6;
			else if (r >= maxScoreForPercentile[0])
					colors[s] = 7;
	}
	
	//print to file
	ofstream out(fileName.c_str());
	gap=0;
	amino aminoAcid;
	for (i=0;i<refSeq.seqLen();i++){	 
		int aa = codonUtility::aaOf(refSeq[i],co);
		if (aa==-1){
			gap++;
			continue;
		}
		string aaStr = aminoAcid.fromInt(aa);
		out<<i+1-gap <<" "<<aaStr<<" "<<colors[i-gap] <<endl;
	}
		
	out.close();

}




void printCodonTable(codon & co){
	amino am;
	int i ,j;
	Vint v;
	for (i = 0;i<am.size();i++){
		v = aminoUtility::codonOf(i,co);
		cout<<endl<<am.fromInt(i);
		for (j=0;j<v.size();j++){
			cout<<" "<<co.fromInt(v[j]);

		}
	}

}

void printMatrix(const VVdouble &mat){

		
	LOG(5,<<"\n\n\n ===================================== \n");
	int a1,a2;
	for (a1=0;a1<mat.size();++a1){
		
		for (a2=0;a2<mat[a1].size();++a2){
			LOG(5,<<mat[a1][a2]<<"\t");
		}
		LOG(5,<<endl);
	}
	
	LOG(5,<<endl<<endl);
	
}

void printVec(const Vdouble & vec){
	LOG(5,<<"\n\n\n ===================================== \n");
	for (int a=0; a<vec.size();a++){
		LOG(5,<<vec[a]<<"\t");
	}
	LOG(5,<<endl<<endl);
}


