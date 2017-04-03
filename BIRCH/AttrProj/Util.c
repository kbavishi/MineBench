/*
  ========================================================================
  DEVise Data Visualization Software
  (c) Copyright 1992-1996
  By the DEVise Development Group
  Madison, Wisconsin
  All Rights Reserved.
  ========================================================================

  Under no circumstances is this software to be copied, distributed,
  or altered in any way without prior permission from the DEVise
  Development Group.
*/

/*
  $Id: Util.c 2396 2014-03-13 21:13:27Z wkliao $

  $Log: Util.c,v $
  Revision 1.20  1996/12/03 20:24:22  jussi
  Added readn() and writen().

  Revision 1.19  1996/12/02 18:44:35  wenger
  Fixed problems dealing with DST in dates (including all date composite
  parsers); added more error checking to date composite parsers.

  Revision 1.18  1996/11/19 15:23:28  wenger
  Minor changes to fix compiles on HP, etc.

  Revision 1.17  1996/11/05 18:23:11  wenger
  Minor mods to get things to compile on SGI systems.

  Revision 1.16  1996/10/18 20:34:08  wenger
  Transforms and clip masks now work for PostScript output; changed
  WindowRep::Text() member functions to ScaledText() to make things
  more clear; added WindowRep::SetDaliServer() member functions to make
  Dali stuff more compatible with client/server library.

  Revision 1.15  1996/10/09 14:33:45  wenger
  Had to make changes to get my new code to compile on HP and Sun.

  Revision 1.14  1996/10/07 22:53:50  wenger
  Added more error checking and better error messages in response to
  some of the problems uncovered by CS 737 students.

  Revision 1.13  1996/08/23 16:55:44  wenger
  First version that allows the use of Dali to display images (more work
  needs to be done on this); changed DevStatus to a class to make it work
  better; various minor bug fixes.

  Revision 1.12  1996/07/14 20:04:47  jussi
  Made code to compile in OSF/1.

  Revision 1.11  1996/07/05 14:39:47  jussi
  Fixed minor problem with null-termination in DateString().

  Revision 1.10  1996/05/20 18:44:42  jussi
  Replaced PENTIUM flag with SOLARIS.

  Revision 1.9  1996/03/27 17:54:56  wenger
  Changes to get DEVise to compile and run on Linux.

  Revision 1.8  1996/02/13 16:20:16  jussi
  Fixed for AIX.

  Revision 1.7  1996/01/10 19:11:17  jussi
  Added error checking to CopyString.

  Revision 1.6  1995/12/28 18:48:14  jussi
  Small fixes to remove compiler warnings.

  Revision 1.5  1995/12/14 17:12:38  jussi
  Small fixes.

  Revision 1.4  1995/10/18 14:55:32  jussi
  Changed mask of created directory to 0777 from 0755.

  Revision 1.3  1995/10/15 18:36:40  jussi
  Added HPUX-specific code.

  Revision 1.2  1995/09/05 21:13:13  jussi
  Added/updated CVS header.
*/

#include <sys/types.h>
#include <stdio.h>
#include <sys/stat.h>

#if defined (SGI)
  #define STAT_BAVAIL f_bfree
#else
  #define STAT_BAVAIL f_bavail
#endif

#if defined(SOLARIS)
  #include <sys/statvfs.h>
  #define STAT_STRUCT statvfs
  #define STAT_FUNC statvfs
  #define STAT_FRSIZE f_frsize
#elif defined(AIX)
  #define _KERNEL
  #define _VOPS
  #include <sys/vfs.h>
  #include <sys/statfs.h>
  #define STAT_STRUCT statfs
  #define STAT_FUNC VFS_STATFS
  #define STAT_FRSIZE f_bsize
#else
  #include <sys/vfs.h>
  #define STAT_STRUCT statfs
  #define STAT_FUNC statfs
  #define STAT_FRSIZE f_bsize

  #if defined(SUN)
    extern "C" int statfs(const char *, struct statfs *);
  #else
    #if defined(SGI)
      #include <sys/statfs.h>
    #endif
  #endif
#endif

#if defined(SOLARIS) || defined(HPUX) || defined(AIX)
#include <dirent.h>
#else
#include <sys/dir.h>
#endif

#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

#include "Util.h"
#include "Exit.h"
#include "DevError.h"

long ModTime(char *fname)
{
  struct stat sbuf;
  if (stat(fname, &sbuf) < 0) {
    fprintf(stderr, "Cannot get modtime of %s\n", fname);
    Exit::DoExit(2);
  }
  return (long)sbuf.st_mtime;
}

DevStatus
ReadFile(char *filename, int &size, char *&buffer)
{
  DevStatus result = StatusOk;

  struct stat sbuf;
  if (stat(filename, &sbuf) < 0)
  {
    reportError("Can't get size of file", errno);
    result = StatusFailed;
  }
  else
  {
    size = sbuf.st_size;
    buffer = new char[size];
    if (buffer == NULL)
    {
      reportError("Out of memory", errno);
      result = StatusFailed;
    }
    else
    {
      int fd = open(filename, O_RDONLY);
      if (fd < 0)
      {
        reportError("Can't open file", errno);
        result = StatusFailed;
      }
      else
      {
        if (read(fd, buffer, size) != size)
	{
          reportError("Error reading file", errno);
          result = StatusFailed;
	}

	if (close(fd) < 0)
	{
          reportError("Error closing file", errno);
          result = StatusWarn;
	}
      }

      if (!result.IsComplete()) delete [] buffer;
    }
  }

  return result;
}

char *CopyString(char *str)
{
  if (str == NULL) return str;
  char *result = new char[strlen(str) + 1];
  if (!result) {
    fprintf(stderr, "Insufficient memory for new string\n");
    Exit::DoExit(2);
  }
  strcpy(result, str);
  return result;
}

static char dateBuf[21];

char *DateString(time_t tm)
{
#if 0
  if (tm < 0) {
    char errBuf[1024];
    sprintf(errBuf, "Illegal time value %ld\n", tm);
    reportErrNosys(errBuf);
  }
#endif

  char *dateStr = ctime(&tm);
  int i;
  for(i = 0; i < 7; i++)
    dateBuf[i] = dateStr[i + 4];

  for(i = 7; i < 11; i++)
    dateBuf[i] = dateStr[i + 13];

  dateBuf[11] = ' ';
  
  for(i = 12; i < 20; i++)
    dateBuf[i] = dateStr[i - 1];

  dateBuf[20] = '\0';

  return dateBuf;
}

void ClearDir(char *dir)
{
  /* clear directory */

  DIR *dirp = opendir(dir);
  if (dirp != NULL){
#if defined(SOLARIS) || defined(HPUX) || defined(AIX) || defined(LINUX) \
      || defined(OSF)
    struct dirent *dp;
#else
    struct direct *dp;
#endif
    for (dp = readdir(dirp); dp != NULL; dp = readdir(dirp)){
#if defined(SOLARIS) || defined(HPUX) || defined(AIX) || defined(LINUX) \
      || defined(OSF)
      struct dirent *realdp = (struct dirent *)dp;
#else
      struct direct *realdp = dp;
#endif
      if (strcmp(realdp->d_name,".") != 0 &&
	  strcmp(realdp->d_name,"..") != 0 ){
	char buf[512];
	sprintf(buf,"%s/%s",dir,realdp->d_name);
	/*
	   printf("unlinking %s\n", buf);
	*/
	unlink(buf);
      }
    }
    closedir(dirp);
  }
}

/* Check if directory exists. Make directory if not already exists
   Clear directory if clear == true*/

void CheckAndMakeDirectory(char *dir, int clear )
{
  struct stat sbuf;
  int ret = stat(dir,&sbuf);
  if (ret >=  0 ) {
    if (!(sbuf.st_mode & S_IFDIR)){
      fprintf(stderr,"Init:: %s not a directory\n", dir);
      Exit::DoExit(1);
    }
    if (clear){
      ClearDir(dir);
    }
  } else {
    /* make new directory */
    int code = mkdir(dir,0777);
    if (code < 0 ){
      printf("Init::can't make directory %s\n",dir);
      perror("");
      Exit::DoExit(1);
    }
  }
}

/* Check whether we have enough space in a given directory. */
void CheckDirSpace(char *dirname, char *envVar, int warnSize, int exitSize)
{
  struct STAT_STRUCT stats;

  if (STAT_FUNC(dirname, &stats
#if defined(SGI)
    , sizeof(stats), 0
#endif
    ) != 0)
  {
    reportErrSys("Can't get status of file system");
  }
  else
  {
    int bytesFree = stats.STAT_BAVAIL * stats.STAT_FRSIZE;
    if (bytesFree < exitSize)
    {
      char errBuf[1024];
      sprintf(errBuf, "%s directory (%s) has less than %d bytes free\n",
	envVar, dirname, exitSize);
      Exit::DoAbort(errBuf, __FILE__, __LINE__);
    }
    else if (bytesFree < warnSize)
    {
      fprintf(stderr,
	"Warning: %s directory (%s) has less than %d bytes free\n",
	envVar, dirname, warnSize);
    }
  }

  return;
}

//
// Read specified number of bytes. Recover from interrupted system calls.
//

int readn(int fd, char *buf, int nbytes)
{
    int nleft = nbytes;
    while (nleft > 0) {
        int nread = read(fd, buf, nleft);
        if (nread < 0) {
            if (errno == EINTR)
                continue;
            perror("read");
            return nread;
        }
        if (nread == 0)                 // EOF?
            break;
        nleft -= nread;
        buf   += nread;
    }
    
    return nbytes - nleft;
}
  
//
// Write specified number of bytes. Recover from interrupted system calls.
//

int writen(int fd, char *buf, int nbytes)
{
    int nleft = nbytes;
    while (nleft > 0) {
        int nwritten = write(fd, buf, nleft);
        if (nwritten < 0) {
            if (errno == EINTR)
                continue;
            return nwritten;
        }
        nleft -= nwritten;
        buf   += nwritten;
    }

    return nbytes - nleft;
}
