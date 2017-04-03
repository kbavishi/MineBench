/*
  This file contributed by the Tape Join project and modified locally.
  See attached copyright notice.
*/

/*
  $Id: tapedrive.C,v 1.15 1996/12/03 20:24:58 jussi Exp $

  $Log: tapedrive.C,v $
  Revision 1.15  1996/12/03 20:24:58  jussi
  Renamed preprocessor flags.

  Revision 1.14  1996/11/23 21:33:35  jussi
  Fixed some bugs when compiled with PROCESS_TASK.

  Revision 1.13  1996/10/02 15:24:02  wenger
  Improved error handling (modified a number of places in the code to use
  the DevError class).

  Revision 1.12  1996/09/26 18:55:42  jussi
  Added support for 64-bit file offsets. Tape commands are now
  executed in a subprocess or thread which increases parallelism
  in the system.

  Revision 1.11  1996/07/18 02:48:24  jussi
  Make this code compile in Ultrix.

  Revision 1.10  1996/07/12 00:55:39  jussi
  Updated copyright information to reflect original source.

  Revision 1.9  1996/06/27 16:04:25  jussi
  Added a cast in memchr() so that the code compiles cleanly in Linux.

  Revision 1.8  1996/04/16 20:56:25  jussi
  Replaced assert() calls with DOASSERT macro.

  Revision 1.7  1996/01/13 03:22:51  jussi
  Removed #include <tar.h> -- this is in tapedrive.h.

  Revision 1.6  1995/12/28 17:50:00  jussi
  Small fixes to remove new compiler warnings.

  Revision 1.5  1995/11/09 22:23:28  jussi
  Added debugging statements.

  Revision 1.4  1995/10/31 17:13:17  jussi
  Added tar archive handling routines and data structures.

  Revision 1.3  1995/09/22 15:46:22  jussi
  Added copyright message.

  Revision 1.2  1995/09/05 20:31:56  jussi
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

//#define TAPE_DEBUG
//#define TAPE_DEBUG2
using namespace std;

#include <stdlib.h>
#include <iostream>
#include <unistd.h>
#include <string.h>

#include "tapedrive.h"
#include "DCE.h"
#include "Exit.h"
#include "DevError.h"

// Use fake fileno and blkno to make this file compile in Alpha
// until a real fix is found. The problem is that in Alpha/OSF,
// struct mtget (mtio.h) does not include fields mt_fileno and
// mt_blkno. See warning printed out below.

#if defined(__alpha) || defined(__ultrix)
#define mt_fileno mt_resid * 0
#define mt_blkno  mt_resid * 0
#endif

#define USE_FWRITEBUF
//#define USE_FREAD

char *TapeDrive::_mt_op_name[] = { "WEOF", "FSF", "BSF", "FSR",
                                   "BSR", "REW", "OFFL", "NOP",
                                   "RETEN", "ERASE", "EOM", "NBSF",
                                   "SRSZ", "GRSZ", "LOAD" };

TapeDrive::TapeDrive(char *name, char *mode, int fno, int blockSz) :
	initialized(0), fileNo(fno), blockSize(blockSz),
	haveTarHeader(0), tarFileSize(0), tarFileOffset(0)
{
#if defined(__alpha) || defined(__ultrix)
  cerr << "Warning: In Ultrix and Alpha/OSF/1, file number and block number"
       << endl;
  cerr << "         inquiry does not work. TapeDrive will probably fail."
       << endl;
#endif

  _child = 0;

  if (!(file = fopen(name, mode))) {
    reportErrSys("fopen");
    return;
  }

#ifdef USE_FWRITEBUF
  // fwrite() in flushBuffer() will write 8 kB blocks to tape even
  // if we request larger blocks; we have to force it to use
  // the specified block size
  int fwriteBufSize = blockSize + 8;
  _fwriteBuf = new char [fwriteBufSize];
  DOASSERT(_fwriteBuf, "Out of memory");
  if (setvbuf(file, _fwriteBuf, _IOFBF, fwriteBufSize) != 0) {
      reportErrSys("setvbuf");
      exit(1);
  }
#endif

#if 0
  setbuf(file, 0);
#endif

  atEof = (mode[0] == 'w');

  for(unsigned int i = 0; i < _max_mt_op; i++)
    mt_tim[i] = mt_ios[i] = mt_cnt[i] = 0;
  read_time = read_ios = read_cnt = 0;
  write_time = write_ios = write_cnt = 0;

  // If the caller gave us the 'target' file number, let's use it.
  // If no number was given, use the current file on the tape.
  if (fileNo < 0) {
    getStatus();
    fileNo = tstat.mt_fileno;
  }
  gotoBeginningOfFile();

  buffer = new char [blockSize];
  DOASSERT(buffer, "Out of memory");
  bufferType = readBuffer;
  bufferBlock = 0;
  bufferOffset = 0;
  bufferBytes = 0;

  initialized = 1;
}

TapeDrive::~TapeDrive()
{
  if (!initialized)
    return;

  if (bufferType == writeBuffer)
    flushBuffer();
  delete buffer;

  waitForChildProcess();

  setbuf(file, 0);

  if (fclose(file))
    reportErrSys("fclose");

#ifdef USE_FWRITEBUF
  delete _fwriteBuf;
#endif
}

void TapeDrive::printStats()
{
/*  cout << "Tape usage statistics:" << endl;
  cout << "  cmd\tcalls\tcount\tavgtime" << endl;
  for(unsigned int i = 0; i < _max_mt_op; i++) {
    if (mt_ios[i] > 0) {
      cout << "  " << _mt_op_name[i]
           << "\t" << mt_ios[i]
           << "\t" << mt_cnt[i]
	   << "\t" << mt_tim[i] / (mt_cnt[i] ? mt_cnt[i] : 1) << endl;
    }
  }
  cout << "  read\t" << read_ios << "\t" << read_cnt
       << "\t" << read_time / (read_ios ? read_ios : 1) << endl;
  cout << "  write\t" << write_ios << "\t" << write_cnt
       << "\t" << write_time / (write_ios ? write_ios : 1) << endl;
*/
}

void TapeDrive::readTarHeader()
{
  DOASSERT(!haveTarHeader, "Invalid tar header flag");

  int bytes = read(&tarHeader, sizeof tarHeader);
  DOASSERT(bytes == sizeof tarHeader, "Invalid tar header size");
  tarFileSize = oct2int(tarHeader.dbuf.size);
  tarFileOffset = 0;

  TAPEDBG(cout << "Read tar header: " << tarHeader.dbuf.name << ", size "
	  << tarFileSize << endl);

  haveTarHeader = 1;
}

unsigned long int TapeDrive::oct2int(char *buf)
{
  unsigned long int num = 0;
  while(*buf == ' ') buf++;
  while(*buf != ' ')
    num = 8 * num + (*buf++ - '0');
  return num;
}

void TapeDrive::waitForChildProcess()
{
  if (_child > 0) {
    TAPEDBG(cout << "Waiting for tape " << fileno(file)
            << " to become idle" << endl);
#ifdef TAPE_THREAD
    (void)pthread_join(_child, 0);
#else
    while(1) {
        int status;
        pid_t child = wait(&status);
        if (child < 0) {
            if (errno == EINTR)
                continue;
            if (errno != ECHILD) {
                reportErrSys("wait");
                exit(1);
            }
        } else
            break;
    }
#endif
    TAPEDBG(cout << "Tape " << fileno(file) << " has become idle" << endl);
    _child = 0;
  }
}

long long TapeDrive::seek(long long offset)
{
  TAPEDBG(cout << "Seek to offset " << offset << " of tape "
          << fileno(file) << endl);

  DOASSERT(offset >= 0, "Invalid tape offset");

  if (bufferType == writeBuffer) {      // flush out write buffer
    flushBuffer();
    bufferType = readBuffer;
    bufferBlock = 0;
  }

  long long lblock = offset / blockSize;
  long long loff = offset % blockSize;

  long block = lblock;
  long off = loff;

  TAPEDBG(cout << "Seeking to lblock " << lblock << ", block "
          << block << " of tape " << fileno(file) << endl);

  if (block != bufferBlock - 1) {       // not current block?
    gotoBlock(block);                   // goto new block location
    fillBuffer();                       // read it into buffer
  }

  bufferBlock = block + 1;              // on tape, just past current block
  bufferOffset = off;

  if (bufferOffset > bufferBytes) {
    cerr << "Seeking past end of file" << endl;
    exit(1);
  }

  return offset;
}

int TapeDrive::read(void *buf, int recSize, int binary)
{
  TAPEDBG2(cout << "Read request for " << recSize << " "
           << (binary ? "binary" : "ASCII")
           << " bytes to " << (void *)buf << endl);

  if (bufferType != readBuffer) {
    cerr << "Must do a seek before switching from writing to reading" << endl;
    exit(1);
  }

  DOASSERT(bufferOffset >= 0 && bufferOffset <= blockSize,
	   "Inconsistent data");
  DOASSERT(bufferBytes >= 0 && bufferBytes <= blockSize, "Inconsistent data");
  DOASSERT(bufferOffset <= bufferBytes, "Inconsistent data");

#ifdef TARFILESIZE
  if (haveTarHeader                     // is file in a tar archive?
      && tarFileOffset >= tarFileSize)  // and at end of file?
    atEof = 1;
#endif

  if (atEof)                            // already at end of tape file?
    return 0;

  if (bufferOffset >= bufferBytes) {    // no more bytes in buffer?
    fillBuffer();                       // get next block from file
    if (!bufferBytes) {                 // end of file?

#ifdef TARFILESIZE
      // for non-tar files, end of tape file is the natural end of
      // user file; for tar files, the file size indicated in the
      // tar header should trigger the atEof statement a few lines
      // up; it is an error if the tape file (tar file) ends before
      // the file inside the tar file
      if (haveTarHeader)
	cerr << "File in tar archive prematurely terminated." << endl;
#endif

      return 0;
    }
  }

  read_cnt++;
  if (read_cnt % 1000 == 0)
    TAPEDBG2(cout << read_cnt << " " << flush);

#ifdef TAPE_BLOCK_PADDING
  char *start = buffer + bufferOffset;        // starting point for this record
  DOASSERT(*start != 0, "Unexpected record"); // must not be an empty record

  if (recSize > bufferBytes - bufferOffset)
    recSize = bufferBytes - bufferOffset;

#ifdef TARFILESIZE
  if (haveTarHeader                     // past EOF of file in tar archive?
      && recSize > tarFileSize - tarFileOffset)
    recSize = tarFileSize - tarFileOffset;
#endif

  if (!binary) {                        // reading an ASCII record?
    char *end = (char *)memchr(start, 0, recSize);
    DOASSERT(end, "End of record not found");
    recSize = end - start;              // do not include record separator
  }
  TAPEDBG2(cout << "Copying " << recSize << " bytes to "
           << (void *)buf << endl);
  memcpy(buf, start, recSize);
  bufferOffset += recSize + 1;          // go past record separator too
  if (!binary                           // in ASCII mode?
      && bufferOffset < bufferBytes     // still data left but...
      && !buffer[bufferOffset]))        // last record in block?
    bufferOffset = bufferBytes;         // must fetch new block next time

#else

  int bytesLeft = recSize;

#ifdef TARFILESIZE
  if (haveTarHeader                     // past EOF of file in tar archive?
      && bytesLeft > tarFileSize - tarFileOffset)
    bytesLeft = tarFileSize - tarFileOffset;
#endif

  recSize = 0;
  char *p = (char *)buf;
  while(bytesLeft > 0) {
    char *start = buffer + bufferOffset;
    int b = bufferBytes - bufferOffset; // bytes left in buffer
    if (bytesLeft < b)                  // caller doesn't want that many?
      b = bytesLeft;
    char *end = 0;
    if (!binary) {                      // reading an ASCII record?
      end = (char *)memchr((void *)start, '\n', b);
      if (end)                          // found newline = end of record?
	b = end - start + 1;
    }
    TAPEDBG2(cout << "Copying " << b << " bytes to " << (void *)p << endl);
    memcpy(p, start, b);
    bufferOffset += b;
    bytesLeft -= b;
    recSize += b;
    p += b;
    if (end)                            // found newline = end of record?
      break;
    if (bufferOffset >= bufferBytes)    // need next block?
      fillBuffer();
    if (!bufferBytes)                   // end of physical file?
      break;
  }

  if (!binary                           // in ASCII mode?
      && bufferOffset < bufferBytes     // still data left but...
      && !buffer[bufferOffset])         // end of logical file (NULL char)?
    bufferOffset = bufferBytes;         // must try to fetch block next time
#endif

#ifdef TARFILESIZE
  if (haveTarHeader) {
     tarFileOffset += recSize;
     DOASSERT(tarFileOffset <= tarFileSize, "Inconsistent data");
  }
#endif

  return recSize;
}

int TapeDrive::write(void *buf, int recSize)
{
  buf = buf;                            // make compiler happy
  recSize = recSize;

  write_cnt++;
  DOASSERT(0, "Random writes not supported on tapes");
  return 0;
}

int TapeDrive::append(void *buf, int recSize)
{
  write_cnt++;

  if (!atEof) {
    gotoEndOfFile();
    atEof = 1;
    bufferBlock = tstat.mt_blkno;
  }

  if (bufferType != writeBuffer) {
    // can simply discard read buffer
    bufferOffset = 0;
    bufferType = writeBuffer;
  }

#ifdef TAPE_BLOCK_PADDING
  if (recSize > blockSize - 1)
    recSize = blockSize - 1;

  if (bufferOffset + recSize + 1 > blockSize)
    flushBuffer();

  DOASSERT(bufferOffset + recSize + 1 <= blockSize, "Inconsistent data");
  memcpy(buffer + bufferOffset, buf, recSize);
  *(buffer + bufferOffset + recSize) = '\0';
  bufferOffset += recSize + 1;
#else
  int bytes = recSize;
  char *p = (char *)buf;
  while(bytes > 0) {
    int spaceLeft = blockSize - bufferOffset;
    int b = (bytes <= spaceLeft ? bytes : spaceLeft);
    DOASSERT(bufferOffset + b <= blockSize, "Inconsistent data");
    memcpy(buffer + bufferOffset, p, b);
    bufferOffset += b;
    bytes -= b;
    p += b;
    if (bufferOffset >= blockSize)
      flushBuffer();
  }
#endif

  return recSize;
}

void TapeDrive::Recover(struct mtget &otstat, short mt_op,
                        daddr_t mt_count)
{
  mt_count = mt_count;                  // make compiler happy
  otstat = otstat;

  int status = 0;

  switch(mt_op) {
    case MTFSF:
      status = ProcessCmdNR(MTBSF, 1);
      if (status >= 0)
          ProcessCmdNR(MTFSF, 1);
      break;
    case MTBSF:
      status = ProcessCmdNR(MTFSF, 1);
      if (status >= 0)
          ProcessCmdNR(MTBSF, 1);
      break;
    case MTFSR:
      status = ProcessCmdNR(MTBSR, 1);
      if (status >= 0)
          ProcessCmdNR(MTFSR, 1);
      break;
    case MTBSR:
      status = ProcessCmdNR(MTFSR, 1);
      if (status >= 0)
          ProcessCmdNR(MTBSR, 1);
      break;
    case MTREW:
      status = ProcessCmdNR(MTFSF, 1);
      break;
    default:
//      cout << "Don't know how to recover from an error with op "
//           << mt_op << endl;
      break;
  }

  DOASSERT(status >= 0, "Recovery attempt failed");
}

#ifdef TAPE_THREAD
void *TapeDrive::ProcessCmd(void *arg)
{
  TapeDrive &me = *(TapeDrive *)arg;
  return me.ProcessCmd(me._proc_mt_op, me._proc_mt_count);
}
#endif

void *TapeDrive::ProcessCmd(short mt_op, daddr_t mt_count)
{
  static struct mtget otstat;           // original tape status
  int status = ioctl(fileno(file), MTIOCGET, (char *)&otstat);
  if (status < 0)
    reportErrSys("ioctl4");
  DOASSERT(status >= 0, "Cannot get tape status");

  for(int attempt = 0; attempt < 10; attempt++) {
      if (attempt > 0) {
//          cout << "Sleeping 2 seconds..." << endl;
          sleep(2);
//          cout << "Recovering..." << endl;
          Recover(otstat, mt_op, mt_count);
//          cout << "Retrying..." << endl;
      }
      status = ProcessCmdNR(mt_op, mt_count);
      if (status >= 0)
          break;
//      cout << "Tape command " << mt_op << ", count " << mt_count
//           << " failed, attempt " << attempt << endl;
  }
  
  return (void *)0;
}

int TapeDrive::ProcessCmdNR(short mt_op, daddr_t mt_count)
{
  static struct mtop cmd;
  cmd.mt_op = mt_op;
  cmd.mt_count = mt_count;

  DOASSERT(mt_op >= 0 && mt_op < _max_mt_op, "Invalid tape command");
  mt_ios[mt_op]++;
  mt_cnt[mt_op] += (mt_count >= 0 ? mt_count : -mt_count);
  
  TAPEDBG(cout << "Tape " << fileno(file) << ", command " << mt_op
          << ", count " << mt_count << " started" << endl);

  startTimer();
  int status = ioctl(fileno(file), MTIOCTOP, (char *)&cmd);
  mt_tim[mt_op] += getTimer();

  if (status < 0)
      reportErrSys("ioctl");

  TAPEDBG(cout << "Tape " << fileno(file) << ", command " << mt_op
          << ", count " << mt_count << " finished, status = " << status
          << endl);

  return status;
}

int TapeDrive::command(short mt_op, daddr_t mt_count)
{
  waitForChildProcess();
  DOASSERT(_child <= 0, "Invalid child process ID");

#ifdef TAPE_THREAD
  _proc_mt_op = mt_op;
  _proc_mt_count = mt_count;
  if (pthread_create(&_child, 0, ProcessCmd, this)) {
      reportErrSys("pthread_create");
      return -1;
  }
#else
  // There seems to be a problem forking the tape command. Do it
  // in same process for now.
  _child = fork();

  if (!_child) {
    (void)ProcessCmd(mt_op, mt_count);
    exit(1);
  }

  if (_child < 0) {
      reportErrSys("fork");
      return -1;
  }
#endif

  return 0;
}

void TapeDrive::getStatus()
{
  waitForChildProcess();

#if defined(__aix) || defined(_AIX)
  tstat.mt_resid  = 0;
  tstat.mt_fileno = 0;
  tstat.mt_blkno  = 0;
#else
  int status = ioctl(fileno(file), MTIOCGET, (char *)&tstat);
  if (status < 0)
    reportErrSys("ioctl2");
  DOASSERT(status >= 0, "Cannot get tape status");
#endif
}

void TapeDrive::fillBuffer()
{
  waitForChildProcess();

  read_ios++;

  TAPEDBG2(cout << "Reading " << blockSize << " bytes to " << (void *)buffer
           << " from fd " << fileno(file) << endl);
  TAPEDBG2(cout << "Bufferblock " << bufferBlock << ", bufferOffset "
           << bufferOffset << endl);

#if 0
  startTimer();
#endif

#ifdef USE_FREAD
  size_t status;
#else
  int status;
#endif

  while (1) {
#ifdef USE_FREAD
      status = fread(buffer, blockSize, 1, file);
      if (!status && ferror(file) && errno == EINTR)
          continue;
#else
      status = ::read(fileno(file), buffer, blockSize);
      if (status < 0 && errno == EINTR)
          continue;
#endif
      break;
  }

#if 0
  read_time += getTimer();
#endif

  bufferBlock++;
  bufferOffset = 0;
  bufferBytes = status;

#ifdef USE_FREAD
  if (!status && feof(file)) {          // end of tape file?
#else
  if (!status) {                        // end of tape file?
#endif
    atEof = 1;
    TAPEDBG(cout << "Backing up past file mark we just passed" << endl);
    int status = command(MTBSF, 1);
    DOASSERT(status >= 0, "Cannot operate tape drive");
    return;
  }

  atEof = 0;

#ifdef USE_FREAD
  if (!status && ferror(file)) {        // read error?
#else
  if (status < 0) {                     // read error?
#endif
    cerr << "Read failed: fd " << fileno(file) << ", buffer "
         << (void *)buffer << ", bytes " << blockSize << endl;
    reportErrSys("read");
    exit(1);
  }
  
#ifdef PARTIAL_BLOCK_ERROR
  if (status < blockSize) {             // partial block read?
    cerr << "Partial block read: " << status << " vs. " << blockSize << endl;
    exit(1);
  }
#endif
}

void TapeDrive::flushBuffer()
{
  waitForChildProcess();

  DOASSERT(bufferType == writeBuffer, "Inconsistent data");
  if (!bufferOffset)
    return;

  write_ios++;
  if (bufferOffset < blockSize)
    memset(buffer + bufferOffset, 0, blockSize - bufferOffset);

#if 0
  startTimer();
#endif

  while (1) {
      size_t status = fwrite(buffer, blockSize, 1, file);
      if (!status && ferror(file) && errno == EINTR)
          continue;
      if (status < 1) {
          reportErrSys("fwrite");
          exit(1);
      }
      break;
  }

#if 0
  write_time += getTimer();
#endif

  bufferBlock++;
  bufferOffset = 0;
}

void TapeDrive::gotoBlock(long block)
{
  TAPEDBG(cout << "Go to block " << block << " of tape "
          << fileno(file) << endl);

  getStatus();

  if (fileNo != tstat.mt_fileno         // oops, we're in another file
      || tstat.mt_blkno > 900000000) {  // unsure about location
    gotoBeginningOfFile();
    getStatus();
  }

  long diff = block - tstat.mt_blkno;   // difference in block numbers
  if (!diff)                            // no movement required?
    return;

  if (!block) {                         // do we just want to go to BOF?
    gotoBeginningOfFile();
    return;
  }

  int status = 0;
  if (diff > 0)
    status = command(MTFSR, diff);      // go forward
  else
    status = command(MTBSR, -diff);     // go backward
  DOASSERT(status >= 0, "Cannot operate tape drive");
}

void TapeDrive::gotoBeginningOfFile()
{
  TAPEDBG(cout << "Going to beginning of file" << endl);

  int status = fseek(file, 0, SEEK_SET);
  if (status < 0)
    reportErrSys("fseek");
  DOASSERT(status >= 0, "Cannot operate tape drive");

  if (!fileNo) {                        // first file? just rewind
    int status = command(MTREW, 1);
    DOASSERT(status >= 0, "Cannot operate tape drive");
#if !defined(__alpha) && !defined(__ultrix)
    tstat.mt_fileno = 0;
    tstat.mt_blkno = 0;
    tstat.mt_resid = 0;
#endif
    return;
  }

  getStatus();
  long diff = fileNo - tstat.mt_fileno;

  if (diff > 0) {                       // go forward?
    int status = command(MTFSF, diff);
    DOASSERT(status >= 0, "Cannot operate tape drive");
  } else {                              // else go backward
    int status = command(MTBSF, -diff + 1);
    DOASSERT(status >= 0, "Cannot operate tape drive");
    status = command(MTFSF, 1);
    DOASSERT(status >= 0, "Cannot operate tape drive");
  }

#if !defined(__alpha) && !defined(__ultrix)
  tstat.mt_fileno = fileNo;
  tstat.mt_blkno = 0;
  tstat.mt_resid = 0;
#endif
}

void TapeDrive::gotoEndOfFile()
{ 
  if (atEof)
    return;

  waitForChildProcess();

  const unsigned long skipSize = 1000000;
  static struct mtop cmd;
  cmd.mt_op = MTFSR;
  cmd.mt_count = skipSize;

  while(1) {
    TAPEDBG(cout << "Tape command " << MTFSR << ", count " << skipSize
	    << ", " << flush);
    int status = ioctl(fileno(file), MTIOCTOP, (char *)&cmd);
    TAPEDBG(cout << "status = " << status << endl);
    if (status < 0) {
      getStatus();
      if (tstat.mt_resid > 0) {
	TAPEDBG(cout << "At end of file, drive status " << tstat.mt_dsreg
		<< ", error status " << tstat.mt_erreg << endl);
	mt_ios[MTFSR] -= tstat.mt_resid;
	return;
      }
    }
    reportErrSys("ioctl3");
    DOASSERT(status >= 0, "Cannot operate tape drive");
  }
}
