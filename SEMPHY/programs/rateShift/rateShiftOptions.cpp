// 	$Id: rateShiftOptions.cpp 2399 2014-03-13 22:43:51Z wkliao $	
#include "rateShiftOptions.h"


rateShiftOptions::rateShiftOptions(int argc, char** argv) :

// DEFAULTS VALUES:
treefile(""),
seqfile(""),
logFile(""), //log.txt
referenceSeq("non"),
logValue(3),
outFile("rateShift.res"),
treeOutFile("TheTree.txt"),
modelName(jtt),
alphabet_size(20), // this is 20
optimizeBranchLengths(mlBBL),
optimizationType(alphaAndNu),
rateEstimationMethod(ebExp),
numberOfDiscreteCategories(4),
userInputAlpha(0.0),
userInputF(0.5),
userInputNu(0.0),
userInputGC(0.5),
optimizeF(false),
outPtr(&cout)
{


	//static struct option long_options[] =  {{0, 0, 0, 0}};
#ifdef _MSC_VER
	int option_index = 0;
#endif
	int c=0;
	//	bool algo_set=false;

	out_f.open(outFile.c_str()); // default output file
	outPtr=&out_f; // default output file

	while (c >= 0) {
#ifdef _MSC_VER
		c = getopt_long(argc, argv,"A:a:b:B:d:D:f:F:Hh?i:I:k:K:l:L:m:M:n:N:O:o:s:S:T:t:v:V:x:X:Yy:z:Z:",NULL,&option_index);
#else
		c = getopt(argc, argv,"A:a:b:B:d:D:f:F:Hh?i:I:k:K:l:L:m:M:n:N:O:o:s:S:T:t:v:V:x:X:Yy:z:Z:");
#endif
		switch (c) {

			// tree file, seqfile 
			case 'a':case 'A': referenceSeq=optarg; break;
			case 'b':case 'B': {
				switch (optarg[0]) {
					case 'g': case 'G':  optimizeBranchLengths=mlBBL; break;	
					case 'h': case 'H':  optimizeBranchLengths=mlBBLUniform; break;	
					case 'n': case 'N':  optimizeBranchLengths=noBBL; break;
					default: optimizeBranchLengths=mlBBL; break;
				}
			}
			case 'c':case 'C': userInputGC=atof(optarg); break;
			case 'd':case 'D': userInputAlpha=atof(optarg); break;
			case 'f':case 'F': userInputF=atof(optarg); break;
			case 'h':case 'H': case '?':
				cout <<"USAGE:	"<<argv[0]<<" [-options] "<<endl <<endl;
				cout <<"+----------------------------------------------+"<<endl;
				cout <<"|-t    tree file                               |"<<endl;
				cout <<"|-s    seq file (accepted formats:             |"<<endl;
				cout <<"|      Fasta, Mase, Molphy, Phylip, Clustal)   |"<<endl;
				cout <<"|-o    out file                                |"<<endl;
				cout <<"|-x    tree out file                           |"<<endl;
				cout <<"|----------------------------------------------|"<<endl;
				cout <<"|-M     model name                             |"<<endl;
				cout <<"|-Mj    JTT                                    |"<<endl;
				cout <<"|-Mr    REV (for mitochondrial genomes)        |"<<endl;
				cout <<"|-Md    DAY                                    |"<<endl;
				cout <<"|-Mw    WAG                                    |"<<endl;
				cout <<"|-MC    cpREV (for chloroplasts genomes)       |"<<endl;
				cout <<"|-Ma    JC amino acids                         |"<<endl;
				cout <<"|-Mn    JC nucleotides                         |"<<endl;
				//cout <<"|-Mt    Tamura                                 |"<<endl;
				//cout <<"|-Mh    HKY (nucleotides)                      |"<<endl;
				cout <<"|----------------------------------------------|"<<endl;
				cout <<"|-i    rate inference method                   |"<<endl; 
				cout <<"|-im   ML rate inference                       |"<<endl;
				cout <<"|-ib   empirical Bayesian (eb-Exp)             |"<<endl;
				cout <<"|-k    number of categories for Bayesian rates |"<<endl;
				cout <<"|----------------------------------------------|"<<endl;
				cout <<"|-b     branch lengths optimization            |"<<endl;
				cout <<"|-bn    no branch lengths optimization         |"<<endl;
				cout <<"|-bg    optimization using a gamma model       |"<<endl;
				cout <<"|default: optimization using a gamma model     |"<<endl;
				cout <<"|----------------------------------------------|"<<endl;
				cout <<"|-a		reference sequence                     |"<<endl;
				cout <<"|default: first sequence in the alignment      |"<<endl;
				cout <<"|----------------------------------------------|"<<endl;
				cout <<"|-h or -? or -H     help                       |"<<endl;
				cout <<"|capital and no captial letters are ok         |"<<endl;
				cout <<"+----------------------------------------------+"<<endl;
				cout <<"|-y     optimize f (SSRV proportion)           |"<<endl;
				cout <<"+----------------------------------------------+"<<endl;
				cout <<"|-z     optimization                           |"<<endl;
				cout <<"|-za    only alpha optimization                |"<<endl;
				cout <<"|-zn    only nu optimization                   |"<<endl;
				cout <<"|-zx    no optimization                        |"<<endl;
				cout <<"|-zs    SSRV = alpha and nu optimization       |"<<endl;
				cout <<"|default: SSRV							       |"<<endl;
				cout <<"|----------------------------------------------|"<<endl;
				// these options are supported but are for Debugging!
				//cout <<"|-c    user input CG content                   |"<<endl;
				cout <<"|-d    user input alpha                        |"<<endl;
				cout <<"|-n    user input nu						   |"<<endl;
				cout <<"|-f    user input f (proportions of SSRV)      |"<<endl;
				cout <<"|-l    logfile                                 |"<<endl;
				cout <<"|-v    log level                               |"<<endl;
				cout <<"|----------------------------------------------|"<<endl;

			cout<<endl;	cerr<<" please press 0 to exit "; int d; cin>>d;exit (0);
			case 'i':case 'I': {
				switch (optarg[0]) {
					case 'b': case 'B': rateEstimationMethod=ebExp; break;
					case 'm': case 'M': rateEstimationMethod=mlRate; break;
					default: rateEstimationMethod=ebExp; break;
				} break;
			}
			case 'k':case 'K': numberOfDiscreteCategories=atoi(optarg); break;
			case 'l':case 'L': logFile=optarg; break;
			case 'm':case 'M':	{
				switch (optarg[0]) {
					case 'd': case 'D':  modelName=day;alphabet_size=20; break;
					case 'j': case 'J':  modelName=jtt;alphabet_size=20; break;
					case 'r': case 'R':  modelName=rev;alphabet_size=20; break;
					case 'w': case 'W':  modelName=wag;alphabet_size=20; break;
					case 'c': case 'C':  modelName=cprev;alphabet_size=20; break;
					case 'a': case 'A':  modelName=aajc;alphabet_size=20; break;
					case 'n': case 'N':  modelName=nucjc;alphabet_size=4; break;
			//		case 't': case 'T':  modelName=tamura;alphabet_size=4; break;
			//		case 'h': case 'H':  modelName=hky;alphabet_size=4; break;
					//case 'q': case 'Q':  modelName=customQ;alphabet_size=20; break;
					//case 'm': case 'M':  modelName=manyQ;alphabet_size=20; break;
					default:modelName=jtt;alphabet_size=20;
					break;
				}
			} break;
			case 'n':case 'N': userInputNu=atof(optarg); break;
			case 'o':case 'O': {
				out_f.close(); // closing the default
				outFile=optarg;
				out_f.open(outFile.c_str());
				if (out_f == NULL) errorMsg::reportError(" unable to open output file for writing. ");
				outPtr=&out_f;
			}; break;
			case 's':case 'S': seqfile=optarg; break;
			case 't':case 'T': treefile=optarg; break;
			case 'v':case 'V': logValue=atoi(optarg); break;
			case 'x':case 'X': treeOutFile=optarg; break;
			case 'y':case 'Y': optimizeF=true; break; // @@@@ -y doesn't work. -Y does work.
			case 'z':case 'Z': 
			{
				switch (optarg[0]) 
				{
					case 'a': case 'A':  optimizationType=alpha; break;	
					case 'n': case 'N':  optimizationType=nu; break;	
					case 's': case 'S':  optimizationType=alphaAndNu; break;
					case 'x': case 'X':  optimizationType=noOptimization; break;
					default: optimizationType=alphaAndNu; break;
				}
			}
		}
	}
}

