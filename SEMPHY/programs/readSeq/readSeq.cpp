// 	$Id: readSeq.cpp 2399 2014-03-13 22:43:51Z wkliao $	

/* TODO: 
 - add time printing each iteration
*/

#include "readSeq_cmdline.h"
#include "sequenceContainer.h"
#include "logFile.h"
#include "recognizeFormat.h"
#include "amino.h"

#include <iostream>
#include <iomanip>
using namespace std;

int main(int argc,char* argv[]) {
	
	gengetopt_args_info args_info;
	if (cmdline_parser(argc, argv, &args_info) != 0) {
		errorMsg::reportError("error reading command line",1);
	}

	ifstream ins;
	istream* inPtr = &cin;	
	string sequenceFileName(args_info.sequence_arg);
	if (sequenceFileName =="-" && args_info.inputs_num) sequenceFileName=args_info.inputs[0];

	if (sequenceFileName != "" && sequenceFileName != "-") {
	  ins.open(sequenceFileName.c_str());
	  if (! ins.is_open())
		errorMsg::reportError(string("can not open sequence file ")+sequenceFileName);
	  inPtr = &ins;
	}
	istream& in = *inPtr;
	amino alphaBet;
	sequenceContainer sc(recognizeFormat::read(in, &alphaBet));

	cout<<sc.seqLen()<<"\t"<<sc.numberOfSeqs()<<endl;


  return 0;
}
