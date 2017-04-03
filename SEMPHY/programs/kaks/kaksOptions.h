// 	$Id: kaksOptions.h 2399 2014-03-13 22:43:51Z wkliao $	
#ifndef ___KAKS__OPTIONS_H
#define ___KAKS__OPTIONS_H
#include "utils.h"
#ifdef SunOS
  #include <unistd.h>
#else
	#ifndef __STDC__
	#define __STDC__ 1
	#include "getopt.h"
	#undef __STDC__
	#else
	#include "getopt.h"
	#endif
#endif

#include <string>
#include <fstream>
#include <iostream>

using namespace std;

class kaksOptions{
public:
	explicit kaksOptions(int& argc, char *argv[]);

public:
	string _inCodonSeqFile;
	string _inTreeFile;
	string _logFile;
	bool _isAlign;
	string _inQuerySeq;
	string _outTreeFile;
	string _outRasmolFile;  //script for rasTop.exe
	string _outSelectionFile;  //file for the serever for the coloring
	string _outGlobalRes;      //global result
	string _outRes4SiteFile; //file with p-value+kaks for site
	bool _isGamma;
	int _geneticCode; //grnrtic code 0 for standart
	
};
#endif
