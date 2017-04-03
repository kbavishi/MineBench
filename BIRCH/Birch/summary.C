using namespace std;

#include <iostream>
#include <fstream>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "vector.h"

#define NUMCLUSTERS 2000
#define NUMATTRS    2

void Summary(char *labelfile, char *inputfile) 
{ 
  ifstream ifile;
  ifstream labFile;
  ofstream ofile;
  Vector summary[NUMCLUSTERS];
  Vector temp;
  temp.Init(NUMATTRS);

  char oFileName[200];

  int		 z=-1;
  int		 number=0;
  int		 numvecs[NUMCLUSTERS];
  double	 metric = 0.0;
  
  ifile.open(inputfile);
  labFile.open(labelfile);
  strcpy(oFileName, inputfile);
  sprintf(oFileName+strlen(oFileName),".metric");
  ofile.open(oFileName);

//  cout << oFileName << endl;

  for (int i=0; i < NUMCLUSTERS; i++){
    summary[i].Init(NUMATTRS);
    numvecs[z] = 0;
  }
  while(labFile.peek() != EOF) {
    for (int i=0; i < NUMATTRS; i++){
      ifile >> temp.value[i];
    }
    labFile >> z;
    summary[z] += temp;
    numvecs[z]++;
    number++;
  }
//  cout << " NUMBER :" << number << endl;
  for ( z=0; z < NUMCLUSTERS; z++){
    if (numvecs[z] != 0)
      {
	summary[z] /= numvecs[z];
	ofile << z << "   " << numvecs[z] << "   ";
	ofile << summary[z] << endl;
      }
  }
  ifile.close();
  labFile.close();
  ifile.open(inputfile);
  labFile.open(labelfile);
  
  while (ifile.peek() != EOF){
    for (int i=0; i < NUMATTRS; i++){
      ifile >> temp.value[i];
    }
    labFile >> z;
    metric += sqrt( summary[z] || temp);
  }
  ofile << " The cluster metric is " << metric << endl;
}

main(int argc, char** argv) 
{
  char labelFile[200];
  char inputFile[200];

  if (argc < 3)
    {
      cout << " Usage: summary <label filename> <data filename>" << endl;
      exit(1);
    }
  strcpy(labelFile, argv[1]);
  strcpy(inputFile, argv[2]);
  
//  cout << " Label file : " << labelFile << endl;
//  cout << " input file : " << inputFile << endl;

  Summary(labelFile, inputFile);
}

