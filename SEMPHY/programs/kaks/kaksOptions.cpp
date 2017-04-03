// 	$Id: kaksOptions.cpp 2399 2014-03-13 22:43:51Z wkliao $	
#include "kaksOptions.h"



kaksOptions::kaksOptions(int& argc, char *argv[]):

// DEFAULTS VALUES:
_inCodonSeqFile(""), //must be filled
_inTreeFile(""), //optional
_logFile("log.txt"), //log file
_inQuerySeq(""), //if missing will be the first sequence in the file
_outTreeFile("optimizedTree.txt"), //output tree after bbl
_outRasmolFile("colorToRasmol.txt"),  //script for rasTop.exe
_outSelectionFile("selection4Site.txt"),  //file for the serever for the coloring
_outGlobalRes("globalResult.txt"),      //global result
_outRes4SiteFile("r4s.res"), //file with p-value+kaks for site
_isGamma(true), //beta is default
_geneticCode(0)//default standart genetic code.
{
	int option_index = 0;
	int c=0;
	bool algo_set=false;

	while (c >= 0) {
#ifdef WIN32
		c = getopt_long(argc, argv,"b:B:c:C:d:D:g:G:l:L:r:R:s:S:t:T:u:U:i:I:x:X",NULL,&option_index);
#else
		c = getopt(argc, argv,"b:B:c:C:d:D:g:G:l:L:r:R:s:S:t:T:u:U:i:I:x:X");
#endif
		switch (c) {
		case 'b':case 'B': 
			// in this output file, each site will be given a bin
			// for example site 1: bin 7
			//             site 2: bin 6
			_outSelectionFile = optarg;
			break;
		case 'c':case 'C': 
			//input file of codons sequences must be alignment
			_inCodonSeqFile = optarg; 			
			break;
		case 'd':  case 'D':
			switch (optarg[0]) {
				case 'g': case 'G':  _isGamma=true; break; //-dg: distribution=gamma
				case 'b': case 'B':  _isGamma=false; break; //-db: distribution=beta
				default: _isGamma=true; break;
			}
			break;
		case 'g':case 'G': //genetic code .
			_geneticCode = atoi(optarg);
             break;
		case 'i':case 'I': 
			//in this file, each site will be given a kaks score
			_outRes4SiteFile = optarg;
            break;
		case 'h':case 'H': case '?':
			cout <<"USAGE:	"<<argv[0]<<" [-options] "<<endl <<endl;
			printHelp();
			cout<<endl;	
			exit (0);

		case 'l':case 'L': //log file 
			_logFile=optarg; 
			break;
		case 'r':case 'R': //input of the name of the reference sequnce.
			_inQuerySeq = optarg;
            break;
		case 's':case 'S': 
			//this output file includes the a rasmol script 
			//(each site with its color according the bin file)
			//which can be projectd on the 3D structure of the protein
			_outRasmolFile = optarg;
            break;		
		case 't':case 'T': 
			//output file which includes the tree after optomization 
			_outTreeFile = optarg;
            break;	
		case 'u':case 'U': 
			//input file which includes user tree (optional)
			_inTreeFile = optarg;
            break;
		case 'x':case 'X': //output file that includes the global values of the model: tr/tv, likelihood, etc.
			_outGlobalRes = optarg;
             break;
		}
	}
}

