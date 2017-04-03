// $Id: errorMsg.cpp 2399 2014-03-13 22:43:51Z wkliao $

// version 1.01
// last modified 1 Jan 2004
#include <stdlib.h>
#include <string.h>
#include "definitions.h"
#include <cassert>
#include "errorMsg.h"
#include "logFile.h"
#include <errno.h>

ostream *errorMsg::_errorOut= NULL;

void errorMsg::reportError(const vector<string>& textToPrint, const int exitCode) {
	for (int i =0 ; i < textToPrint.size() ; ++i) {
		LOG(1,<<textToPrint[i]<<endl);
		cerr<<textToPrint[i]<<endl;
		if (_errorOut != NULL && *_errorOut != cerr)  {
			(*_errorOut)<<textToPrint[i]<<endl;
		}
	}
	if (errno!=0){
	  LOG(1,<<"System Error: "<<strerror(errno)<<endl);
	  cerr<<"System Error: "<<strerror(errno)<<endl;
	}
	assert(0); // always stop here if in DEBUG mode.
	exit(exitCode);
}

void errorMsg::reportError(const string& textToPrint, const int exitCode) {
	LOG(1,<<endl<<textToPrint<<endl);
	cerr<<endl<<textToPrint<<endl;
	if (_errorOut != NULL && *_errorOut != cerr)  {
		(*_errorOut)<<textToPrint<<endl;
	}
	if (errno!=0){
	  LOG(1,<<"System Error: "<<strerror(errno)<<endl);
	  cerr<<"System Error: "<<strerror(errno)<<endl;
	}
	assert(0); // always stop here if in DEBUG mode.
	exit(exitCode);
}


