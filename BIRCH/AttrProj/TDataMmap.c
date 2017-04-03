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
  $Id: TDataMmap.c 2396 2014-03-13 21:13:27Z wkliao $

  $Log: TDataMmap.c,v $
  Revision 1.4  1996/06/21 19:33:22  jussi
  Replaced MinMax calls with MIN() and MAX().

  Revision 1.3  1996/03/05 21:25:32  jussi
  Added copyright notice and cleaned up the code a bit.

  Revision 1.2  1995/09/05 22:15:58  jussi
  Added CVS header.
*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <limits.h>

#include "TDataMmap.h"

/**********************************************************************
Constructor
***********************************************************************/

TDataMmap::TDataMmap(char *name, int recSize)
{
  /* open the file */
  if ((_fd = open(name, O_RDONLY, 0666)) < 0) {
    perror("TDataMmap: open file");
    Exit::DoExit(2);
  }

  struct stat sbuf;
  if (fstat(_fd, &sbuf) < 0) {
    perror("TDataMmap: fstat");
    Exit::DoExit(2);
  }
  _len = sbuf.st_size;
  if ((_len % DISKFILE_PAGESIZE) != 0) {
    fprintf(stderr, "file size %d not multiple of page size\n");
    Exit::DoExit(2);
  }
  _numPages = _len / DISKFILE_PAGESIZE;
  
  _addr = mmap(0, _len, PROT_READ, MAP_FIXED, _fd, 0);
  if (_addr == (caddr_d)-1) {
    perror("TDataMmap: mmap");
    Exit::DoExit(2);
  }
  _start = (Page *)_addr;
  _header = (FileHeader *)_addr;
  if (_header->recSize != recSize) {
    fprintf(stderr, "TDataMap: wrong record size\n");
    Exit::DoExit(2);
  }
}

/********************************************************************
Page Oriented Interface to get records.
*********************************************************************/

/* Return # of pages in the Tdata */
int TDataMmap::NumPages()
{
  return _numPages-1;
}

/* Return page number of 1st page */
virtual int TDataMmap::FirstPage()
{
  return 1;
}

/* Return page number of last page */
int TDataMmap::LastPage()
{
  return _numPages;
}

/* Get page numbers of pages currently in memory */
virtual void TDataMmap::PagesInMem(int &numPages, int *&pageNums)
{
  numPages = 0;
  pageNums = NULL;
}

/* Fetch page into memory and return all records
   residing in the page. numRecs == # of records in the page.
   startRid == starting record id of the records on the page.
   recs == array of pointers to records.
   isPrefetch == true if doing prefetch. Used for keeping statistics.
   Not: FreePage() must be called to free the page.
   Note: static internal data is used to keep the pointers to records
   == must be saved before same method can be called again.*/

virtual void TDataMmap::GetPage(int pageNum, int &numRecs, RecId &startRid,
				void **&recs, Boolean isPrefetch = false)
{
  if (pageNum < 1 || pageNum > _numPages) {
    fprintf(stderr, "TDataMMap::GetPage: invalid page number %d\n", pageNum);
    Exit::DoExit(1);
  }

  Page *page = &_start[pageNum];
  RecId firstRid = MakeRecId(pageNum, 0);
  numRecs = MIN(RecordsPerPage(), (int)(_header.numRecs - firstRid));
  RecId lastRid = firstRid+numRecs-1;
}
		
/* Fetch the page containing the specified record id.
   isPrefetch: set to true if doing prefetch.
   Returns:
     pageNum: page number of the page
     rec: pointer to record data.
   FreePage() must be called to free the page */

void TDataMmap::GetRecPage(RecId recId, int &pageNum, void *&rec,
			   Boolean isPrefetch = false)
{
}

/* Called when page is no longer needed.
   BufHint gives hint about which list to store the page.*/

void TDataMmap::FreePage(int pageNum, BufHint hint= Stay)
{
}
	
/******************************************************************
Info about records in the file.	
*******************************************************************/

/* Return # of records */
int TDataMmap::NumRecords()
{
  return _header->numRecs;
}

/* Return Record size */
int TDataMmap::RecSize()
{
  return _header->recSize;
}

/* Insert a record into the page */
void TDataMmap::InsertRec(void *rec)
{
  fprintf(stderr, "TDataMmap::InsertRec not implemented\n");
  Exit::DoExit(2);
}


