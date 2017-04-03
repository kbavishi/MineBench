// 	$Id: simulateSequnce.cpp 2399 2014-03-13 22:43:51Z wkliao $	
#include <string.h>
#include "definitions.h"  
#include "simulateSequnce.h"
#include "simulateSequnce_cmdline.h"
#include "logFile.h"
#include "talRandom.h"
#include "readDatMatrix.h"
#include "datMatrixHolder.h"
#include "likelihoodComputation.h"
#include "codon.h"
#include <iostream>
#include <fstream>
#include <cassert>
#include <string>
#include <cstdio>
using namespace std;

// to do:
//1. add simulations from a discrete distribution
//2. add simulations from a continuous distribution
//3. change code so that simulations will only get a replacement model.
// 4. check why simulate sequences have to get the alphabet as input.

void setLog(const char* logfilename, const int loglvl);


 int main(int argc, char* argv[]) {
	// 1. Get command line info
	if (argc < 2) errorMsg::reportError("The program must get some parameters in the command line, use -h for list");
	gengetopt_args_info args_info;
	if (cmdline_parser (argc, argv, &args_info) != 0) {
	  errorMsg::reportError("error in command line parsing",1);
	}


	// 2. Set log file
	setLog(args_info.Logfile_arg, args_info.verbose_arg);

	// 3. Set seed
	if	(args_info.seed_given) {
	  talRandom::setSeed(args_info.seed_arg);
	  LOG(3,<<"using seed = "<< args_info.seed_arg<<endl);
	}

	// 4. Get alphabet

	alphabet*  alphP = NULL;
	switch (args_info.alphabet_arg) 
	  {
	  case 4:	
	    alphP = new nucleotide; 
	    break;
	  case 20: 
	    alphP = new amino; 
	    break;
	  case 61: case 64:
	    alphP = new codon; 
	    break;
	  default: errorMsg::reportError("error getting the alphabet");
	}

	// 5. Getting the tree
	tree et(args_info.tree_arg);
	LOGDO(5,et.output(myLog::LogFile()));
    // 6. Getting the rate vector if it is supplied or creating a uniform rate dist.
    Vdouble rates;
	if (args_info.rateVector_given) {
	  ifstream rateVectFile(args_info.rateVector_arg);
	  if (rateVectFile.good()){
	    rates.resize(args_info.length_arg);
	    int i;
	    for (i=0;i<args_info.length_arg && !rateVectFile.eof();++i){
	      rateVectFile>>rates[i];
	    }
	    if (i<args_info.length_arg)
	      errorMsg::reportError("Failed to read all required rates from the file");
	    LOG(3,<<"using rates from "<< args_info.rateVector_arg<<endl);
	  }
	  else 
	    errorMsg::reportError("could not open the file containing the rate vector");
	} else {// rate vector not supplied.
		if (args_info.gamma_given == false) {
			rates.resize(args_info.length_arg,1.0);
		} else {
			errorMsg::reportError(" Currently, we can simulate either with user supplying the rate vector or with uniform distribution of rates");
		}
	}

	// 7. Setting the output stream.
	string outputFileName = args_info.outputfile_arg;
	ostream * outP=NULL;
	ofstream out_tmp;
	if (strcmp(args_info.outputfile_arg,"-")==0)  // cout
	  outP=&cout;
	else {
	  out_tmp.open(args_info.outputfile_arg);
	  outP=&out_tmp;
	}
	ostream& out = *outP;

	// 8. Creating the stochastic process
	distribution *dist = new uniDistribution;

	replacementModel *probMod=NULL;
	if (args_info.day_given) {
		LOG(5,<<"Using day evolutionary model"<<endl);
		probMod=new pupAll(datMatrixHolder::dayhoff);
	} else if (args_info.jtt_given) {
		LOG(5,<<"Using JTT evolutionary model"<<endl);
		probMod=new pupAll(datMatrixHolder::jones);
	} else if (args_info.rev_given) {
		LOG(5,<<"Using rev evolutionary model"<<endl);
		probMod=new pupAll(datMatrixHolder::mtREV24);
	} else if (args_info.wag_given) {
		LOG(5,<<"Using wag evolutionary model"<<endl);
		probMod=new pupAll(datMatrixHolder::wag);
	} else if (args_info.cprev_given) {
		LOG(5,<<"Using cprev evolutionary model"<<endl);
		probMod=new pupAll(datMatrixHolder::cpREV45);
	} else if (args_info.nucjc_given) {
		LOG(5,<<"Using nucjc evolutionary model"<<endl);
		probMod=new nucJC;
	} else if (args_info.aaJC_given) {
		LOG(5,<<"Using aaJC evolutionary model"<<endl);
		probMod=new aaJC;
	} else if ((args_info.hky_given) || (args_info.k2p_given)) {
		LOG(5,<<"Using hky evolutionary model"<<endl);
		MDOUBLE ratio =args_info.ratio_arg;
		MDOUBLE Ap(0.25), Cp(0.25), Gp(0.25), Tp(0.25);
		sscanf(args_info.ACGprob_arg,"%lf,%lf,%lf", &Ap, &Cp, &Gp);
		Tp=1.0-(Ap+Cp+Gp);
		probMod=new hky(Ap,Cp,Gp,Tp,ratio);
	} else if ((args_info.alphabet_arg == 20) && 
		(args_info.modelfile_given)) { // try to read the name as a file name
		LOG(5,<<"Using user supplied evolutionary model from the file "<<args_info.modelfile_arg<<endl);
		probMod=new pupAll(args_info.modelfile_arg);
	} else { /* default = if (strcmp(args_info.model_arg,"jtt")==0) */
		LOG(5,<<"Using default model (JTT) "<<endl);
		probMod=new pupAll(datMatrixHolder::jones);
	}
	pijAccelerator * pijAcc = new trivialAccelerator(probMod);
	stochasticProcess sp(dist, pijAcc);
	if (probMod) delete probMod;
	if (pijAcc) delete pijAcc;
	if (dist) delete dist;

	// if GLOBAL rate is give as input, set it.
	if (args_info.inputRate_given) {
	  sp.setGlobalRate(args_info.inputRate_arg);
	  for (int j=0; j < rates.size(); ++j) {
		  rates[j]*=sp.getGlobalRate();
	  }
	}

	// 8. Generating the sequences
	LOG(3,<<"Generating a sequence of length "<< args_info.length_arg<<endl);
	simulateTree st1(et, sp, alphP);
	//if (args_info.rateVector_given)
	  st1.generate_seqWithRateVector(rates,args_info.length_arg);
	//else {
	//  if (args_info.continuous_flag) 
	//	  st1.generate_seq_continuous_gamma(args_info.length_arg);
	//  else
	//    st1.generate_seq(args_info.length_arg);
	//}
	sequenceContainer sc = st1.toSeqDataWithoutInternalNodes();

	if      (strcmp(args_info.format_arg,"mase")   ==0) {maseFormat::   write(out,sc);}
	else if (strcmp(args_info.format_arg,"clustal")==0) {clustalFormat::write(out,sc);}
	else if (strcmp(args_info.format_arg,"molphy") ==0) {molphyFormat:: write(out,sc);}
	else if (strcmp(args_info.format_arg,"phylip") ==0) {phylipFormat:: write(out,sc);}
	else if (strcmp(args_info.format_arg,"fasta")  ==0) {fastaFormat::  write(out,sc);}
	else { errorMsg::reportError(" format not implemented yet in this version... ");}
	if (strcmp(args_info.outputfile_arg,"-")!=0) out_tmp.close();
	delete alphP;
}

void setLog(const char* logfilename, const int loglvl){
  if (!strcmp(logfilename,"-")||!strcmp(logfilename,""))
	  {
	    myLog::setLogOstream(&cout);
	  }
	else
	  {
	    ofstream* outLF = new ofstream;
	    outLF->open(logfilename);
	    if (!outLF->is_open()) {
	      errorMsg::reportError(string("unable to open log file '")+logfilename+"' for reading");
	    }
	    myLog::setLogOstream(outLF);
	  }
	myLog::setLogLvl(loglvl);
	LOG(3,<<"START OF Simulate Sequence LOG FILE \n\n");
}

// JUNK:
	// (in the future, 
/*	distribution *dist = NULL;
	if (args_info.gamma_given == false) 
	  dist =  new uniDistribution;
	else {
	  dist =  new gammaDistribution(args_info.gamma_arg,4);
	  LOG(5,<<"Using Gamma ASRV with 4 bins"<<endl);
	}
*/
