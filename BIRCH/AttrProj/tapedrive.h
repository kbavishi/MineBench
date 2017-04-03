/*
  This file contributed by the Tape Join project and modified locally.
  See attached copyright notice.
*/

/*
  $Id: tapedrive.h 2396 2014-03-13 21:13:27Z wkliao $

  $Log: tapedrive.h,v $
  Revision 1.10  1996/12/03 20:24:59  jussi
  Renamed preprocessor flags.

  Revision 1.9  1996/11/23 21:33:35  jussi
  Fixed some bugs when compiled with PROCESS_TASK.

  Revision 1.8  1996/10/04 19:59:45  wenger
  Temporarily (?) removed threads from Devise (removed -lpthread from
  some Makefiles so Purify works); other minor cleanups.

  Revision 1.7  1996/09/26 18:55:43  jussi
  Added support for 64-bit file offsets. Tape commands are now
  executed in a subprocess or thread which increases parallelism
  in the system.

  Revision 1.6  1996/07/18 02:20:01  jussi
  Made this compile in Ultrix.

  Revision 1.5  1996/07/12 00:55:40  jussi
  Updated copyright information to reflect original source.

  Revision 1.4  1995/10/31 17:12:59  jussi
  Added tar archive handling routines and data structures.

  Revision 1.3  1995/09/22 15:43:33  jussi
  Added copyright message.

  Revision 1.2  1995/09/05 20:31:57  jussi
  Added CVS header.
*/

/*
  Copyright 1993-1996 by Jussi Myllymaki
  
  Permission to use, copy, modify, and distribute this software and its
  documentation for any purpose and without fee is hereby granted,
  provided that the above copyright notice appear in all copies and that
  both that copyright notice and this permission notice appear in
  supporting documentation, and that the name(s) of the copyright holder(s)
  not be used in advertising or publicity pertaining to distribution of
  the software without specific, written prior permission. The copyright
  holder(s) make no representations about the suitability of this
  software for any purpose.  It is provided "as is" without express
  or implied warranty.
  
  Author: Jussi Myllymaki
*/

#ifndef TAPEDRIVE_H
#define TAPEDRIVE_H

// uncomment the following #defines to enable features
//#define TAPE_DEBUG                      // print out debugging information
//#define TAPE_DEBUG2                     // print out debugging information
#define TAPE_BLOCKING                   // blocked access to tape
//#define TAPE_BLOCK_PADDING              // records do not cross blocks
//#define PARTIAL_BLOCK_ERROR             // error if partial blocks read
//#define TAPE_THREAD                     // use threads

#ifdef SOLARIS
//#define TAPE_THREAD
#endif

#ifdef TAPE_DEBUG
  #define TAPEDBG(s) s
#else
  #define TAPEDBG(s) // nothing
#endif

#ifdef TAPE_DEBUG2
  #define TAPEDBG2(s) s
#else
  #define TAPEDBG2(s) // nothing
#endif

#include <stdio.h>
#ifndef ULTRIX
  #include <tar.h>
#endif
#include "machdep.h"

#ifdef TAPE_THREAD
#include <pthread.h>
#endif

#ifndef TBLOCK
#define TBLOCK 512                      // tar file block size
#endif

#ifndef NAMSIZ
#define NAMSIZ 100                      // tar header block filename size
#endif

const int TAPE_BLOCKSIZE = 32768;       // blocking factor on tape

class TapeDrive {
public:
  // Initialize tape interface
  TapeDrive(char *name, char *mode, int fileno = -1,
	    int blockSize = TAPE_BLOCKSIZE);

  // Clean up
  virtual ~TapeDrive();

  // Print usage statistics
  virtual void printStats();

  // Return 1 if properly initialized
  virtual operator int() { return initialized; }

  // Read tar header block
  void readTarHeader();

  // Convert octal number (in ASCII) to decimal number (in binary)
  unsigned long int oct2int(char *buf);

  // Return block size
  int getBlockSize() { return blockSize; }

  // Move on tape
  virtual long long seek(long long offset);

  // Return current position on tape
  virtual long long tell() {
    if (!bufferBlock)                   // haven't read any data yet?
      return 0;
    return (bufferBlock - 1) * blockSize + bufferOffset;
  }

  // Read a newline-terminated ASCII record from tape
  virtual int gets(void *buf, int recSize) { return read(buf, recSize, 0); }
  
  // Read a binary record from tape
  virtual int read(void *buf, int recSize) { return read(buf, recSize, 1); }
  
  // Write to tape
  virtual int write(void *buf, int recSize);
  
  // Append to tape
  virtual int append(void *buf, int recSize);

protected:
  // Read a record from tape
  int read(void *buf, int recSize, int binary);

  // Perform tape operation
  int command(short mt_op, daddr_t mt_count);

  // Get tape status
  void getStatus();

  // Fill buffer with next block from tape
  void fillBuffer();

  // Flush full or partial buffer block to tape
  void flushBuffer();

  // Go to specified block
  void gotoBlock(long block);

  // Resynchronize tape
  void resynchronize();

  // Go to beginning of file
  void gotoBeginningOfFile();

  // Go to end of file
  void gotoEndOfFile();

  // Start timer
  void startTimer() {
    gettimeofday(&startTime, 0);
  }

  // Get current timer value
  double getTimer() {
    float diff;
    gettimeofday(&stopTime, 0);
    diff = stopTime.tv_sec - startTime.tv_sec
           + ((int)((stopTime.tv_usec - startTime.tv_usec) / 1e3)) / 1e3;
    return diff;
  }

  // Wait for child process to complete
  void waitForChildProcess();

  int initialized;                      // 1 if properly initialized

  FILE *file;                           // pointer to open file
  int fileNo;                           // file number on tape

  static const int _max_mt_op = 15;
  long mt_ios[_max_mt_op];              // number of I/O calls
  long mt_cnt[_max_mt_op];              // number of I/O counts
  double mt_tim[_max_mt_op];            // times of I/O requests
  static char *_mt_op_name[];           // names of I/O operations

  long read_cnt;                        // number of records read
  long read_ios;                        // number of read() calls
  double read_time;                     // time spent in read()

  long write_cnt;                       // number of records written
  long write_ios;                       // number of write() calls
  double write_time;                    // time spent in write()

  struct timeval startTime;             // start time of timer
  struct timeval stopTime;              // end time of timer

  struct mtget tstat;                   // tape status
  const int blockSize;                  // tape block size

  enum { writeBuffer,
	 readBuffer } bufferType;       // write or read buffer
  int  bufferOffset;                    // current offset into buffer
  int  bufferBytes;                     // number of bytes in buffer
  long bufferBlock;                     // block number of cached block
  char *buffer;                         // block on tape
  char *_fwriteBuf;                     // block given to fwrite()
  int  atEof;                           // 1 if tape at EOF

#ifdef TAPE_THREAD
  static void *ProcessCmd(void *arg);   // thread/process for commands
  pthread_t _child;                     // ID of child thread
  short _proc_mt_op;                    // cached value for ProcessCmd()
  daddr_t _proc_mt_count;               // cached value for ProcessCmd()
#else
  pid_t _child;                         // ID of child process
#endif

  void *ProcessCmd(short mt_op,
                   daddr_t mt_count);   // recoverable tape command processing
  int ProcessCmdNR(short mt_op,
                   daddr_t mt_count);   // nonrecoverable processing
  void Recover(struct mtget &tstat,
               short mt_op,
               daddr_t mt_count);       // recover from failed tape command

  int haveTarHeader;                    // 1 if we've read a tar header
  unsigned long int tarFileSize;        // file size for file in tar archive
  unsigned long int tarFileOffset;      // file offset for file in tar archive

public:
  union hblock {                        // tar header block
    char dummy[TBLOCK];
    struct header {
      char name[NAMSIZ];
      char mode[8];
      char uid[8];
      char gid[8];
      char size[12];
      char mtime[12];
      char chksum[8];
      char linkflag;
      char linkname[NAMSIZ];
    } dbuf;
  } tarHeader;
};

#endif
