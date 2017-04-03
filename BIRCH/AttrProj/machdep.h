/*
  ========================================================================
  DEVise Software
  (c) Copyright 1992-1996
  By the DEVise Development Group
  University of Wisconsin at Madison
  All Rights Reserved.
  ========================================================================

  Under no circumstances is this software to be copied, distributed,
  or altered in any way without prior permission from the DEVise
  Development Group.
*/

/*
  $Id: machdep.h 2396 2014-03-13 21:13:27Z wkliao $

  $Log: machdep.h,v $
  Revision 1.16  1996/12/15 20:23:08  wenger
  Incremented revision; prints current architecture at startup.

  Revision 1.15  1996/12/03 20:29:57  jussi
  Fixed some AIX problems.

  Revision 1.14  1996/11/21 19:14:09  wenger
  Fixed more compile warnings; updated devise.dali to match current
  command-line flags.

  Revision 1.13  1996/09/26 18:58:31  jussi
  Removed extraneous definition on Alpha/OSF.

  Revision 1.12  1996/09/25 18:36:52  wenger
  Updated again for Linux compile (Linux 2.0.6, Tcl 7.4, Tk 4.0).

  Revision 1.11  1996/07/29 21:40:24  wenger
  Fixed various compile errors and warnings.

  Revision 1.10  1996/07/18 02:16:42  jussi
  Added support for Ultrix.

  Revision 1.9  1996/07/17 00:47:26  jussi
  Added a redefinition of SIG_ERR/SIG_DFL/SIG_IGN on the Ultrix
  platform.

  Revision 1.8  1996/07/13 04:59:45  jussi
  Added conditional for Linux.

  Revision 1.7  1996/05/31 15:41:44  jussi
  Minor change to remove compiler warnings in Linux.

  Revision 1.6  1996/05/20 18:42:03  jussi
  Replaced PENTIUM flag with SOLARIS.

  Revision 1.5  1996/03/27 17:55:11  wenger
  Changes to get DEVise to compile and run on Linux.

  Revision 1.4  1996/03/26 15:35:01  wenger
  Fixed various compile warnings and errors; added 'clean' and
  'mostlyclean' targets to makefiles; changed makefiles so that
  object lists are derived from source lists.

  Revision 1.3  1996/02/15 18:01:10  jussi
  Small fix for ioctl() on AIX.

  Revision 1.2  1996/02/13 16:35:44  jussi
  Minor fix for AIX.

  Revision 1.1  1996/01/13 03:23:44  jussi
  Moved file from tape/.

  Revision 1.4  1995/12/28 17:31:12  jussi
  Added SGI specific code.

  Revision 1.3  1995/09/22 15:43:32  jussi
  Added copyright message.

  Revision 1.2  1995/09/05 20:31:55  jussi
  Added CVS header.
*/

#ifndef MACHDEP_H
#define MACHDEP_H

/*
   Make sure macros indicating the platform are there.
*/

#if !defined(__aix) && defined(_AIX)
  #define __aix
#endif
  
#if !defined(__sgi) && defined(sgi)
  #define __sgi
#endif

#if !defined(__sun)
  #if defined(sun2) || defined(sun3) || defined(sun3x) || defined(sun4) \
      || defined(sun) || defined(sparc)
    #define __sun
  #endif
#endif

#ifdef SOLARIS
#define __solaris
#endif

#if defined(__solaris) && defined(__sun)
  #undef __sun
#endif

/*
   Check that any application-defined platform macros
   are consistent with compiler-defined macros.
*/

#if defined(AIX) && !defined(__aix)
#error "Are you sure this is an AIX machine?"
#endif

#if defined(ALPHA) && !defined(__alpha)
#error "Are you sure this is an Alpha machine?"
#endif

#if defined(HPUX) && !defined(__hpux)
#error "Are you sure this is an HP machine?"
#endif

#if defined(SOLARIS) && !defined(__solaris)
#error "Are you sure this is a Solaris machine?"
#endif

#if defined(SGI) && !defined(__sgi)
#error "Are you sure this is an SGI machine?"
#endif

#if defined(SUN) && !defined(__sun)
#error "Are you sure this is a Sun machine?"
#endif

#if defined(ULTRIX) && !defined(__ultrix)
#error "Are you sure this is an Ultrix machine?"
#endif

#if defined(WINNT) && !defined(__winnt)
#error "Are you sure this is a Windows NT machine?"
#endif

#if defined(LINUX) && !defined(__linux__)
#error "Are you sure this is a Linux machine?"
#endif

/*
   Prevent Linux from loading /usr/include/linux/uio.h and
   conflict with /usr/include/sys/uio.h.
*/

#ifdef LINUX
  #define __LINUX_UIO_H
#endif

/*
   Define general routines and data structures that are typically
   needed in low-level programming.
*/

#if defined(__hpux) && !defined(_INCLUDE_HPUX_SOURCE)
  #define _INCLUDE_HPUX_SOURCE
#endif

#include <limits.h>
#include <sys/types.h>
#include <sys/param.h>

#ifndef MIN
  #define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif
  
#ifndef MAX
  #define MAX(a,b) ((a) > (b) ? (a) : (b))
#endif

#ifdef __cplusplus
  #define EXTERNC extern "C"
#else
  #define EXTERNC extern
#endif

/*
   Definitions related to time.
*/

#include <sys/time.h>
#include <sys/wait.h>

#ifdef __aix
  #include <time.h>
#endif

#if defined(__sun) || defined(__aix) || defined(__ultrix)
  EXTERNC int gettimeofday(struct timeval *,
			   struct timezone *);
#endif

/*
   Definitions related to pipes, shared memory, and semaphores.
*/

#ifdef __hpux
  #if PIPE_MAX > 1048576
    #undef PIPE_MAX
    #define PIPE_MAX 1048576
  #endif
#endif

#if defined(__sun) || defined(__aix) || defined(__alpha)
  #define PIPE_MAX PIPE_BUF
#endif

#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>

#if defined(__sun)
  #define SHM_R 0400
  #define SHM_W 0200

/*  EXTERNC int shmget(key_t, u_int, u_int); */
  EXTERNC int shmget(key_t, int, int);
  EXTERNC int shmctl(int, int, struct shmid_ds *);
  EXTERNC char *shmat(int, char *, int);
  EXTERNC int shmdt(char *);
  EXTERNC int select(int, fd_set *, fd_set *, fd_set *, struct timeval *);
  EXTERNC int setitimer(int which, struct itimerval *value,
			   struct itimerval *ovalue);
  EXTERNC int bcmp(const void *s1, const void *s2, size_t n);
  EXTERNC void bcopy(const void *s1, void *s2, size_t n);
  EXTERNC void bzero(void *s, size_t n);
#endif

#if defined(__ultrix)
  EXTERNC int select(int, fd_set *, fd_set *, fd_set *, struct timeval *);
  EXTERNC int setitimer(int which, struct itimerval *value,
			   struct itimerval *ovalue);
#endif

#if defined(__alpha) || defined(__sun)
  EXTERNC int semget(key_t, int, int);
  EXTERNC int semctl(int, int, int, ...);
  EXTERNC int semop(int, struct sembuf *, unsigned int);
#endif

#if defined(__alpha) || defined(__sun) || defined(__linux)
  #ifndef SEM_A
    #define SEM_A 0200                  // alter permission
  #endif
  #ifndef SEM_R
    #define SEM_R 0400                  // read permission
  #endif
#endif

#if defined(__hpux) || defined(__alpha)
union semun {
  int val;
  struct semid_ds *buf;
  u_short *array;
};
#endif

/*
   Definitions related to signals.
*/

#if !defined(__hpux) && !defined(__solaris) && !defined(__sgi)
  EXTERNC int sigpause(int sigmask);
#endif

#if defined(__sun) || defined(__solaris)
  EXTERNC int sigblock(int);
  EXTERNC int sigsetmask(int);
#endif

/*
   Definitions related to magnetic tapes and other device I/O.
*/

#include <sys/ioctl.h>

#ifdef __sun
  #ifdef _cplusplus
    EXTERNC int aioread(int, char *, int, int, int, struct aio_result_t *);
    EXTERNC int aiowrite(int, char *, int, int, int, struct aio_result_t *);
    EXTERNC aio_result_t *aiowait(struct timeval *);
  #endif
#endif

#ifdef __ultrix
  #include <ansi_compat.h>
  #include <sys/devio.h>

/*
   Our current Ultrix installation has a signal.h file that defines
   SIG_ERR, SIG_DFL, and SIG_IGN in a way that is incompatible with
   the signal() definition in the same file!
*/

  #ifdef __STDC__
    #ifdef SIG_ERR
      #undef SIG_ERR
    #endif
    #define SIG_ERR ((void (*)(int))(-1))
    #ifdef SIG_DFL
      #undef SIG_DFL
    #endif
    #define SIG_DFL ((void (*)(int))( 0))
    #ifdef SIG_IGN
      #undef SIG_IGN
    #endif
    #define SIG_IGN ((void (*)(int))( 1))
  #endif
#endif

#if !defined(__aix)
  #ifdef __alpha
    #include "osf_mtio.h"
  #else
    #include <sys/mtio.h>
  #endif
#endif

#ifdef __aix
  #include <sys/tape.h>
  /* need to define mapping from mtio #defines to AIX ones */
  #define MTIOCTOP   STIOCTOP
  #define MTIOCGET   0 /* not supported */
  #define mtop       stop
  #define mt_op      st_op
  #define mt_count   st_count
  #define MTOFFL     STOFFL
  #define MTREW      STREW
  #define MTRETEN    STRETEN
  #define MTWEOF     STWEOF
  #define MTFSF      STFSF
  #define MTRSF      STRSF
  #define MTBSF      STRSF
  #define MTFSR      STFSR
  #define MTRSR      STRSR
  #define MTBSR      STRSR
  #define MTLOAD     STINSRT
  #define MTUNLOAD   STEJECT

  struct mtget {
    daddr_t mt_resid;
    daddr_t mt_fileno;
    daddr_t mt_blkno;
  };
#endif

#ifdef __hpux
  #define FILE2FD(f) (f->__fileL + (f->__fileH << 8))
#elif defined(__linux__)
  #define FILE2FD(f) (f->_fileno)
#else
  #define FILE2FD(f) (f->_file)
#endif

#ifdef __cplusplus
  EXTERNC {
    #include <sys/uio.h>
  }
#else
  #include <sys/uio.h>
#endif

#if defined(__ultrix) || defined(__sun)
  EXTERNC int ioctl(int, int, char *);
#endif

#if !defined(__alpha) && !defined(__hpux) && !defined(__solaris) \
    && !defined(__sgi) && !defined(__linux__)
  EXTERNC int readv(int, struct iovec *, int);
  EXTERNC int writev(int, struct iovec *, int);
#endif

#ifdef __sun
  #define SEEK_SET 0
  #ifndef __cplusplus
    EXTERNC int printf(char *, ...);
    EXTERNC int fprintf(FILE *, char *, ...);
    EXTERNC int fwrite(const void *, int, int, FILE *);
    EXTERNC int fputc(int, FILE *);
    EXTERNC int fflush(FILE *);
    EXTERNC int puts(const char *);
    EXTERNC int fseek(FILE *, long, int);
    EXTERNC int fread(void *, int, int, FILE *);
    EXTERNC int fclose(FILE *);
  #endif
  EXTERNC int strcasecmp(const char *, const char *);
#endif

/*
   Definitions related to network I/O.
*/

#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#ifndef __sun
  #include <arpa/inet.h>
#else
  EXTERNC char *inet_ntoa(struct in_addr);
#endif

#if !defined(__hpux) && !defined(__alpha) && !defined(__sgi) && \
    !defined(__linux) && !defined(__solaris)
  EXTERNC int socket(int, int, int);
  //EXTERNC int bind(int, struct sockaddr *, int);
  EXTERNC int listen(int, int);
  EXTERNC int accept(int, struct sockaddr *, int *);
  EXTERNC int connect(int, struct sockaddr *, int);
  EXTERNC int shutdown(int, int);
#endif

/*
   Miscellaneous definitions.
*/

#if defined(__sun) || defined(__hpux) || defined(__solaris)
  EXTERNC void perror(const char *);
#endif

/*
#if !defined(__hpux) && !defined(__solaris) && !defined(__sgi)
  EXTERNC key_t ftok(char *, char);
#endif
*/
  
EXTERNC int fsync(int);
EXTERNC unsigned sleep(unsigned);


/*
 * Name of architecture.
 */
#if defined(AIX)
  #define ARCH_NAME "AIX"
#elif defined(ALPHA)
  #define ARCH_NAME "Alpha"
#elif defined(HPUX)
  #define ARCH_NAME "HP-UX"
#elif defined(LINUX)
  #define ARCH_NAME "Linux"
#elif defined(OSF)
  #define ARCH_NAME "Alpha/OSF"
#elif defined(SGI)
  #define ARCH_NAME "SGI"
#elif defined(SOLARIS)
//TEMPTEMP sparc vs intel
  #define ARCH_NAME "Solaris"
#elif defined(SUN)
  #define ARCH_NAME "SPARC/SunOS"
#elif defined(ULTRIX)
  #define ARCH_NAME "Ultrix"
#elif defined(WINNT)
  #define ARCH_NAME "Windows NT"
#else
  #define ARCH_NAME "unknown"
#endif


#endif
