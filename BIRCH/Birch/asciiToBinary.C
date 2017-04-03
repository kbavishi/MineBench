using namespace std;

#include <iostream>
#include <fstream.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <math.h>
#include <strstream.h>

#include "global.h"
#include "vector.h"

extern void SetupVectorScan(char *fileName, char *buffer, int dimensionality);
extern int getNextVector(Vector& vec, char *buffer);


int
main(int argc, char **argv)
{
  int dim;
  char fileName[MAX_NAME_LENGTH];
  char outputFileName[MAX_NAME_LENGTH];
  char buffer[VEC_BLOCK_SIZE];
  Vector vec;
  double *outputArr;
  int fd;
  int i;
  int nbytes;
  int totSize = 0;

  if (argc != 4)
    {
      cout << "Usage: transform <asciifile> <dimn> <outputfile>" << endl;
      exit(1);
    }

  strcpy(fileName, argv[1]);
  dim = atoi(argv[2]);
  strcpy(outputFileName, argv[3]);

  SetupVectorScan(fileName, buffer, dim);

  vec.Init(dim);
  outputArr = new double[dim];

  fd = open(outputFileName, O_WRONLY | O_CREAT, 511);
  assert(fd != -1);
//  cout << fd << " " << outputFileName << endl;
  while (getNextVector(vec, buffer) != -1)
    {
      for (i = 0; i < dim; i++)
	{
	  outputArr[i] = vec.Value(i);
	}
      nbytes = write(fd, (void*)outputArr, sizeof(double)*dim);
      assert(nbytes == sizeof(double)*dim);
      totSize += nbytes;
    }
//  cout << totSize << endl;
  close(fd);
  delete [] outputArr;
}


