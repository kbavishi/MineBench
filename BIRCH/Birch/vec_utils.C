using namespace std;

#include <iostream>
#include <strstream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <math.h>

#include "global.h"
#include "vector.h"

//#include "Set.h"
//#include "utils.h"
//#include "globals.h"
//#include "sample.h"

//const int VEC_BLOCK_SIZE=1024;

static int dimensionality;
#ifdef ASCII
ifstream ifile;
#else
static int fd;
//static ifstream ifile;
static int curOffset;
static char *tmpBuf;
static int bytesRead;
#endif


void 
SetupVectorScan(char *fileName, char *buffer, int dimn)
{
#ifdef ASCII

//  cout << "Opening ASCII file " << fileName << endl;
  dimensionality = dimn;
  ifile.open(fileName);
  assert(ifile != NULL);
#else
  fd = open(fileName, O_RDONLY);
  assert(fd != -1);
  assert(0 == VEC_BLOCK_SIZE%sizeof(double));

  assert(dimn > 0);
  dimensionality = dimn;
  curOffset = 0;
  bytesRead = 0;
  tmpBuf = buffer + VEC_BLOCK_SIZE;
#endif
}


/*
 * fd: file descriptor of the open file
 * currentOffset: offset in *chars* into the current file from which point the reading should start
 * bufSize: the size of the block to be read in *chars*
 * buffer: space for at least bufSize chars
 *
 * return value:
 * size of the block in *chars*
 */

const int extraBufferSize=10;

int
getNextBlock(int fd, int currentOffset, int bufSize, char* buffer)
{
  int bytesRead, currentPos, endPos;
  char bufferHere[extraBufferSize*sizeof(double)*dimensionality];
  //int* checkElement;
  bool done;


  assert((0 == (bufSize % sizeof(double))));
	 //(0 == (extraBufferSize % sizeof(double))));

  done = true; // indicates whether the end of the file is reached


  if (0 == buffer) {
    // seek until a little bit before the end of the buffer
    endPos = lseek(fd, 0, SEEK_END);
    if (currentOffset + bufSize > endPos)
      bufSize = endPos - currentOffset;
    assert(bufSize >= 0);
    currentPos = lseek(fd, currentOffset+bufSize-extraBufferSize, SEEK_SET);
    assert(currentPos <= endPos - extraBufferSize);
    bytesRead = read(fd, (char*)bufferHere, extraBufferSize); // then read the last piece
    if (bytesRead < extraBufferSize) // if the end of the file is reached
      return bufSize - extraBufferSize + bytesRead;
    
    
    //checkElement = (int*)&bufferHere[extraBufferSize-sizeof(int)];
  }
  else {
    lseek(fd, currentOffset, SEEK_SET);     // seek for the current offset
    bytesRead = read(fd, (char*)buffer, bufSize); // read the whole block into memory
    if (bytesRead < bufSize) { // end of file?
      return bytesRead;
    }

    //checkElement = (int*)&buffer[bufSize-sizeof(int)];
  }

  //while (*checkElement != -1) {
  //bufSize -= sizeof(int);
  //checkElement--;
  //}
  assert(bufSize > 0);
  return bufSize;

}



int
getNextVector(Vector& vec, char *buffer)
{
  int i;

#ifdef ASCII
  char buf[1000];
  double val;
  ifile.getline(buf, 1000);
  if (!ifile)
    {
      ifile.close();
      return -1;
    }
  istrstream istr(buf);
  vec.SetDim(dimensionality);
  for (i = 0; i < dimensionality; i++)
    {
      istr >> val;
      vec.SetVal(i, val);
    }
#else
  if ((tmpBuf - buffer) >= bytesRead)
    {
      bytesRead = getNextBlock(fd, curOffset, VEC_BLOCK_SIZE*sizeof(double)*dimensionality, buffer);
      if (bytesRead <= 0)
	{
	  close(fd);
	  return -1;
	}
      tmpBuf = buffer;
      curOffset += bytesRead;
    }
  
  vec.newArr(dimensionality, (double*)(tmpBuf));
  tmpBuf += dimensionality * sizeof(double);
#endif

  return 1;
}


