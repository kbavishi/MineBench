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
  $Id: TDataAppend.c 2396 2014-03-13 21:13:27Z wkliao $

  $Log: TDataAppend.c,v $
  Revision 1.4  1996/06/21 19:33:20  jussi
  Replaced MinMax calls with MIN() and MAX().

  Revision 1.3  1996/01/12 15:24:21  jussi
  Replaced libc.h with stdlib.h.

  Revision 1.2  1995/09/05 22:15:47  jussi
  Added CVS header.
*/

/* Append-only Textual Data */

#include <stdio.h>
#include <stdlib.h>

#include "Exit.h"
#include "TDataAppend.h"
#include "BufMgr.h"
#include "PageFile.h"

/********************************************************************
Constructors
********************************************************************/
TDataAppend::TDataAppend(char *name, BufMgr *mgr, int recSize){
	Initialize(name, mgr, recSize);
}

TDataAppend::TDataAppend(char *name, int recSize){
	Initialize(name, NULL, recSize);
}


/**************************************************************
Initialization for constructors
***************************************************************/
void TDataAppend::Initialize(char *name, BufMgr *mgr, int recSize){

	_name = name;

	/* Get memory for returning records */
	_createdMgr = false;
	if ((_mgr=mgr) == NULL){
		_createdMgr = true;
		_mgr = new BufMgr();
	}
	_pfile = new PageFile(name, _mgr);
	if (_pfile->NumPages() == 0){
		/* a newly created file */
		_header.numRecs = 0;
		_header.recSize = recSize;
	}
	else {
		/* Read header from file */
		_pfile->ReadHeader(&_header, sizeof(_header));
		if (_header.recSize != recSize){
			fprintf(stderr,"TDataAppend::TDataAppend(%s,,%d):recSize shoudl be %d\n", name, recSize, _header.recSize);
			Exit::DoExit(1);
		}
	}

	_lastGetPageNum = -1;
	_returnRecs = new (void *[RecordsPerPage()]);
}

/**********************************************************************
Destructor
***********************************************************************/
TDataAppend::~TDataAppend(){
/*
printf("TDataAppend: destructor\n");
*/

	/* Write header back */
	_pfile->WriteHeader(&_header, sizeof(_header));

	/* delete the page file */
	delete _pfile;

	if (_createdMgr)
		/* delete the buffer manager */
		delete _mgr;
	
	delete _returnRecs;
}

/******************************************************************
Return # of pages
******************************************************************/
int TDataAppend::NumPages(){
	return _header.numRecs / RecordsPerPage() + 
		( (_header.numRecs % RecordsPerPage()) == 0? 0 : 1);
}

/*****************************************************************
Return page number of 1st page 
*******************************************************************/
int TDataAppend::FirstPage(){ return 1;}

/******************************************************************
Return page number of last page
*******************************************************************/
int TDataAppend::LastPage() { return NumPages(); };

/***********************************************************************
Get page numbers of pages currently in memory 
***********************************************************************/
void TDataAppend::PagesInMem(int &numPages, int *&pageNums){
	/* Report all pages except page 0 */
	_pfile->PagesInMem(numPages, pageNums);
}

/*********************************************************************
Fetch page in mem and return all records
**********************************************************************/
void TDataAppend::GetPage(int pageNum, int &numRecs, RecId &startRid, 
				void **&recs, Boolean isPrefetch){
	if (pageNum < 1 || pageNum > NumPages()){
		fprintf(stderr,
			"TDataAppend::GetPage: invalid page number %d\n", pageNum);
		Exit::DoExit(1);
	}
	BufPage *bpage = _pfile->GetPage(pageNum, isPrefetch);
	RecId firstRid = MakeRecId(pageNum, 0);
	numRecs = MIN(RecordsPerPage(), (int)(_header.numRecs - firstRid));
	RecId lastRid = firstRid+numRecs-1;

	/* set return params */
	startRid = firstRid;
	int i;
	char *ptr = (char *)bpage->PageData();
	for (i=0; i < numRecs; i++){
		_returnRecs[i] = ptr;
		ptr += _header.recSize;
	}
	recs = _returnRecs;

	_lastGetPageNum = pageNum;
	_lastGetPageBuf = bpage;

}

/*********************************************************************
Return true if page in mem
**********************************************************************/
Boolean TDataAppend::PageInMem(int pageNum){
	return _pfile->PageInMem(pageNum);
}

/*********************************************************************
Fetch the page containing the specified record id.
Returns:
	pageNum: page number of the page
	rec: pointer to record data.
FreePage() must be called to free the page 
************************************************************************/
void TDataAppend::GetRecPage(RecId recId, int &pageNum, void *&rec, 
	Boolean isPrefetch){
	CheckRecId(recId);

	pageNum = PageNum(recId);
	BufPage *bpage = _pfile->GetPage(pageNum, isPrefetch);
	_lastGetPageNum = pageNum;
	_lastGetPageBuf = bpage;

	rec = RecordAddress(recId, bpage->PageData());
}


/*******************************************************************
Free page, called when page is no longer needed.
*********************************************************************/
void TDataAppend::FreePage(int pageNum, BufHint hint){
	if (pageNum == _lastGetPageNum){
		/* free last page we got */
		_pfile->UnfixPage(_lastGetPageBuf, hint);
		_lastGetPageNum = -1; /* page no longer cached */
	}
	else {
		/* get the page back. Free it twice: once for
		when we got it last time. Once for now.*/
		BufPage *bpage = _pfile->GetPage(pageNum);
		_pfile->UnfixPage(bpage, hint);
		_pfile->UnfixPage(bpage, hint);
	}
}

/**********************************************************************
Record Oriented Interface
**********************************************************************/

/****************************************************************
Return # of records
*****************************************************************/
int TDataAppend::NumRecords(){
	return _header.numRecs;
}

/*****************************************************************
Return record size
********************************************************************/
int TDataAppend::RecSize(){
	return _header.recSize;
}


/*********************************************************************
Insert a new record 
**********************************************************************/
void TDataAppend::InsertRec(void *data){
	/* update record Id */
	int recId = _header.numRecs++;

	/* Read page from page file */
	int page = PageNum(recId);
	BufPage *bpage;
	int temp;
	if (page > _pfile->NumPages())
		/* Create a new page */
		bpage = _pfile->CreatePage(temp);
	else /* read an existing page */
		bpage = _pfile->GetPage(page);

	/* Write the new record onto disk */
	char *recAddr = RecordAddress(recId, bpage->PageData());
	bcopy((char *)data, recAddr, _header.recSize);

	/* mark page dirty and no longer needed */
	_pfile->DirtyPage(bpage);
	_pfile->UnfixPage(bpage, Stay);

	/* Report insertion of new record */
	TData::ReportNewRec(recId);
}

/**********************************************************************
Check record id
***********************************************************************/
void TDataAppend::CheckRecId(RecId id){
	if (id < 0 ||id >= _header.numRecs){
		fprintf(stderr,"TDataAppend::invalid recId %d, only %d recs\n",
				id, _header.numRecs);
		Exit::DoExit(1);
	}
}
