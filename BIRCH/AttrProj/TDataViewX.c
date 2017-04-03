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
  $Id: TDataViewX.c 2396 2014-03-13 21:13:27Z wkliao $

  $Log: TDataViewX.c,v $
  Revision 1.46  1996/11/26 16:51:39  ssl
  Added support for piled viws

  Revision 1.45  1996/11/20 20:35:21  wenger
  Fixed bugs 062, 073, 074, and 075; added workaround for bug 063; make
  some Makefile improvements so compile works first time; fixed up files
  to correspond to new query catalog name.

  Revision 1.44  1996/11/19 20:21:08  jussi
  Overlapping record elimination now done on for BAR shapes.

  Revision 1.43  1996/11/18 23:11:32  wenger
  Added procedures to generated PostScript to reduce the size of the
  output and speed up PostScript processing; added 'small font' capability
  and trademark notice to PostScript output; improved text positioning in
  PostScript output (but still a ways to go); added a little debug code;
  fixed data/axis area bugs (left gaps); fixed misc. bugs in color handling.

  Revision 1.42  1996/11/18 18:10:56  donjerko
  New files and changes to make DTE work with Devise

  Revision 1.41  1996/11/13 16:57:11  wenger
  Color working in direct PostScript output (which is now enabled);
  improved ColorMgr so that it doesn't allocate duplicates of colors
  it already has, also keeps RGB values of the colors it has allocated;
  changed Color to GlobalColor, LocalColor to make the distinction
  explicit between local and global colors (_not_ interchangeable);
  fixed global vs. local color conflict in View class; changed 'dali'
  references in command-line arguments to 'tasvir' (internally, the
  code still mostly refers to Dali).

  Revision 1.40  1996/09/27 15:53:22  wenger
  Fixed a number of memory leaks.

  Revision 1.39  1996/08/05 18:37:40  beyer
  Minor simplification of stats gathering

  Revision 1.38  1996/08/04 21:59:55  beyer
  Added UpdateLinks that allow one view to be told to update by another view.
  Changed TData so that all TData's have a DataSource (for UpdateLinks).
  Changed all of the subclasses of TData to conform.
  A RecFile is now a DataSource.
  Changed the stats buffers in ViewGraph to be DataSources.

  Revision 1.37  1996/08/03 15:19:38  jussi
  The visual filter is not applied in 3D views.

  Revision 1.36  1996/07/26 16:17:42  guangshu
  Assign yMax and yMin from tdata so the width of histogram is fixed for the whole data sets

  Revision 1.35  1996/07/25 14:32:36  guangshu
  Added linked list to keep track of the gstat records so it doesnot need to scann the range from xmin to xmax and fixed bugs for histograms

  Revision 1.34  1996/07/23 18:00:53  jussi
  Ifdef'd out the statistics code because large datasets cause
  unlimited memory to be consumed.

  Revision 1.33  1996/07/23 17:26:06  jussi
  Added support for piled views.

  Revision 1.32  1996/07/22 23:44:19  guangshu
  Added statistics for gdata. The statistics includes count, ysum, max,
  mean, min.

  Revision 1.31  1996/07/20 17:06:35  guangshu
  Small fixes when wirte to HistBuffer.

  Revision 1.30  1996/07/19 18:00:30  guangshu
  Added support for histograms.

  Revision 1.29  1996/07/19 17:31:31  jussi
  Fixed problem in InsertSymbol().

  Revision 1.28  1996/07/19 17:26:04  jussi
  Added code that prevents record overlap elimination when
  records are of LineShape or LineShadeShape.

  Revision 1.27  1996/07/19 02:43:07  jussi
  Added point storage for missing data points.

  Revision 1.26  1996/07/13 17:25:07  jussi
  When view is iconified, statistics are collected but data
  is not drawn.

  Revision 1.25  1996/07/12 19:40:14  jussi
  View statistics are now printed into a memory buffer.

  Revision 1.24  1996/07/02 22:46:01  jussi
  The bounding box of symbols is now correctly computed. Scatter
  plots sometimes did not have all necessary data displayed in
  them, as bounding box used to be incorrectly computed.

  Revision 1.23  1996/06/27 15:48:02  jussi
  Added some debugging statements.

  Revision 1.22  1996/06/20 17:10:24  guangshu
  Added support for color statistics.

  Revision 1.21  1996/06/20 16:49:50  jussi
  Added Y condition to filter comparison.

  Revision 1.20  1996/06/13 00:16:31  jussi
  Added support for views that are slaves of more than one record
  link. This allows one to express disjunctive queries.

  Revision 1.19  1996/05/31 15:41:25  jussi
  Added support for record links.

  Revision 1.18  1996/04/22 21:38:06  jussi
  Fixed problem with simultaneous view refresh and record query
  activities. Previously, there was a single iterator over the
  mappings of a view, which caused the system to crash when a record
  was queried while the data was still being displayed. Each activity
  now gets its own iterator.

  Revision 1.17  1996/04/16 20:50:13  jussi
  Replaced assert() calls with DOASSERT macro.

  Revision 1.16  1996/04/15 16:07:45  jussi
  Improved the way x and y attributes are extracted from a GData
  record in ReturnGData(). Also added code for extracting shape
  and color which will be used for statistics collection later
  on.

  Revision 1.15  1996/04/15 15:07:26  jussi
  Interface to ViewGraph's mapping iterator has changed. Added
  call to DrawLegend().

  Revision 1.14  1996/04/10 15:33:30  jussi
  Cleaned up some debugging statements.

  Revision 1.13  1996/04/10 02:22:32  jussi
  Added support for > 1 mappings in a view.

  Revision 1.12  1996/04/09 22:53:41  jussi
  Added View parameter to DrawGDataArray.

  Revision 1.11  1996/04/05 20:13:30  wenger
  Fixed error causing pure virtual function to be called
  if a session was closed during a query; fixed an error
  in one of the Linux Makefiles; updated other Makefiles
  to allow testWindowRep to be built on all architectures.

  Revision 1.10  1995/12/29 22:41:28  jussi
  Added support for line connectors.

  Revision 1.9  1995/12/14 21:20:00  jussi
  Replaced 0x%x with 0x%p.

  Revision 1.8  1995/12/05 17:07:34  jussi
  Statistics are now part of ViewGraph, the subclass of TDataViewX.

  Revision 1.7  1995/11/29 17:11:21  jussi
  Relaxed condition which determines which GData records are included
  in statistics computation. Bar graphs may have y values beyond
  filter.yHigh, and the bar still appears on the screen and so should
  be included in statistics.

  Revision 1.6  1995/11/28  05:23:00  ravim
  Support for statistics.

  Revision 1.5  1995/11/25 01:20:18  jussi
  This code now uses Transform matrix operations to convert user/world
  coordinates to screen pixel coordinates. This is to avoid any future
  inconsistencies in how the different code locations compute the
  conversion. xPerPixel and yPerPixel are now obsolete coefficients.

  Revision 1.4  1995/11/24 21:24:39  jussi
  Fixed inconsistencies in computing xPerPixel vs. scaling done by
  View.

  Revision 1.3  1995/11/24 16:10:40  jussi
  Added copyright notice and cleaned up the code.

  Revision 1.2  1995/09/05 22:16:04  jussi
  Added CVS header.
*/

#include <assert.h>
#include <string.h>

// #define DEBUG

#include "Init.h"
#include "TDataViewX.h"
#include "TDataMap.h"
#include "ConnectorShape.h"
#include "Shape.h"
#include "Util.h"
#include "RecordLink.h"

//#define DEBUG

ImplementDList(BStatList, BasicStats *)

TDataViewX::TDataViewX(char *name, VisualFilter &initFilter, QueryProc *qp, 
		       GlobalColor fg, GlobalColor bg, AxisLabel *xAxis,
		       AxisLabel *yAxis, Action *action) :
	ViewGraph(name, initFilter, xAxis, yAxis, fg, bg, action)
{
  _dataBin = new GDataBin();
  DOASSERT(_dataBin, "Out of memory");

  _map = 0;
  _index = -1;
  _queryProc = qp;
  _totalGData = _numBatches = 0;
  _batchRecs = Init::BatchRecs();
  _queryFilter = initFilter;

  _dispSymbols = true;
  _dispConnectors = false;
  _cMap = 0;
}

TDataViewX::~TDataViewX()
{
  // SubClassUnmapped aborts any current query; this _must_ be done
  // before this destructor exits, or members needed to do the abort
  // will no longer be defined.
  SubClassUnmapped();

  delete _dataBin;

  int index = _blist.InitIterator();
  while (_blist.More(index)) {
    delete _blist.Next(index);
  }
  _blist.DoneIterator(index);
  _blist.DeleteAll();
  _gstat.Clear();
  _glist.DeleteAll();
}

void TDataViewX::InsertMapping(TDataMap *map)
{
  ViewGraph::InsertMapping(map);
  
  Coord xMin;
  if (_queryProc->GetMinX(map, xMin))
    View::SetXMin(true, xMin);
  
  Refresh();
}

void TDataViewX::DerivedStartQuery(VisualFilter &filter, int timestamp)
{
#ifdef DEBUG
//  printf("start query\n");
#endif

  _queryFilter = filter;
  _timestamp = timestamp;

  // Initialize statistics collection
  _allStats.Init(this);

  for(int i = 0; i < MAXCOLOR; i++)
    _stats[i].Init(this);

  int index = _blist.InitIterator();
  while (_blist.More(index)) {
    delete _blist.Next(index);
  }
  _blist.DoneIterator(index);
  _blist.DeleteAll();
  _gstat.Clear();     /* Clear the hashtable and calculate it again */
  _glist.DeleteAll(); /* Clear the gdata list */

  // Initialize record links whose master this view is
  index = _masterLink.InitIterator();
  while(_masterLink.More(index)) {
    RecordLink *link = _masterLink.Next(index);
    link->Initialize();
  }
  _masterLink.DoneIterator(index);

  _index = InitMappingIterator(true);   // open iterator backwards
  if (MoreMapping(_index)) {
    _map = NextMapping(_index)->map;
#ifdef DEBUG
    //printf("Submitting query 1 of %d: 0x%p\n", _mappings.Size(), _map);
#endif
    _pstorage.Clear();
    _queryProc->BatchQuery(_map, _queryFilter, this, 0, _timestamp);
  } else {
#ifdef DEBUG
    //printf("View has no mappings; reporting query as done\n");
#endif
    ReportQueryDone(0);
    DoneMappingIterator(_index);
    _map = 0;
    _index = -1;
  }
}

void TDataViewX::DerivedAbortQuery()
{
#ifdef DEBUG
    //printf("TDataViewX::Abort query, index = %d\n", _index);
#endif

  if (_map) {
    _queryProc->AbortQuery(_map, this);
    DOASSERT(_index >= 0, "Invalid iterator index");
    DoneMappingIterator(_index);
    _map = 0;
    _index = -1;
  }

  // Abort record links whose master this view is
  int index = _masterLink.InitIterator();
  while(_masterLink.More(index)) {
    RecordLink *link = _masterLink.Next(index);
    link->Abort();
  }
  _masterLink.DoneIterator(index);

  _dataBin->Final();
}

Boolean TDataViewX::DisplaySymbols(Boolean state)
{ 
#if defined(DEBUG)
  //printf("TDataViewX::DisplaySymbols(%d)\n", state);
#endif

  if (state == _dispSymbols)
    return state;

  Boolean oldState = _dispSymbols;
  _dispSymbols = state;

#ifdef DEBUG
  //printf("DisplaySymbols now %s\n", (_dispSymbols ? "on" : "off"));
#endif

  InvalidatePixmaps();
  Refresh();

  return oldState;
}

Boolean TDataViewX::DisplayConnectors(Boolean state)
{ 
#if defined(DEBUG)
  //printf("TDataViewX::DisplayConnectors(%d)\n", state);
#endif

#if !USE_CONNECTORS
  return false;
#endif

  if (state == _dispConnectors)
    return state;

  if (!state) {
    DOASSERT(_cMap, "No connector mapping");
    delete _cMap;
    _cMap = NULL;
  } else {
    DOASSERT(!_cMap, "Unexpected connector mapping");
    _cMap = new TDataCMap;
    DOASSERT(_cMap, "Could not create connector mapping");
    _cMap->SetDefaultColor(GetFgColor());
  }

  Boolean oldState = _dispConnectors;
  _dispConnectors = state;

#ifdef DEBUG
  //printf("DisplayConnectors now %s\n", (_dispConnectors ? "on" : "off"));
#endif

  InvalidatePixmaps();
  Refresh();

  return oldState;
}

/* Query data ready to be returned. Do initialization here. */

void TDataViewX::QueryInit(void *userData)
{ 
  _dataBin->Init(_map, &_queryFilter, GetWindowRep()->TopTransform(),
		 _dispSymbols, _dispConnectors, _cMap, this);
}

void TDataViewX::ReturnGData(TDataMap *mapping, RecId recId,
			     void *gdata, int numGData)
{
#if defined(DEBUG)
  //printf("TDataViewX::ReturnGData()\n");
#endif
  DOASSERT(_index >= 0, "Invalid iterator index");

  mapping->UpdateMaxSymSize(gdata, numGData);
  int gRecSize = mapping->GDataRecordSize();

  _totalGData += numGData;
  _numBatches++;

  char *tp = (char *)gdata;
  GDataAttrOffset *offset = mapping->GetGDataOffset();

  // Can do record elimination only for constant bar shape
  Boolean canElimRecords = false;
  if (offset->shapeOffset < 0 && mapping->GetDefaultShape() == 2)
      canElimRecords = true;

  // Collect statistics and update record links only for last mapping
  if (!MoreMapping(_index)) {

    int firstRec = 0;

    for(int i = 0; i < numGData; i++) {
      // Extract X, Y, shape, and color information from gdata record
      Coord x = GetX(tp, mapping, offset);
      Coord y = GetY(tp, mapping, offset);
      ShapeID shape = GetShape(tp, mapping, offset);
      GlobalColor color = mapping->GetDefaultColor();
      if (offset->colorOffset >= 0)
	color = *(GlobalColor *)(tp + offset->colorOffset);
      Boolean complexShape = mapping->IsComplexShape(shape);
      complexShape |= (GetNumDimensions() == 3);

      // Compute statistics only for records that match the filter''s
      // X range, regardless of the Y boundary
      if (x >= _queryFilter.xLow && x <= _queryFilter.xHigh) {
	  if (color < MAXCOLOR)
	    _stats[color].Sample(x, y);
	  _allStats.Sample(x, y);
	  _allStats.Histogram(y);
	  if(_glist.Size() <= MAX_GSTAT) {
	      int X = (int) x;
	      BasicStats *bs;
	      if(_gstat.Lookup(X, bs)) {
		  bs->Sample(x,y);
	      } else {
		  bs = new BasicStats();
		  DOASSERT(bs, "Out of memory");
		  bs->Init(0);
		  _glist.InsertOrderly(X, 1);
		  bs->Sample(x, y);
		  _gstat.Insert(X, bs);
		  _blist.Insert(bs);
	      } 
	  } 
      }

      // Contiguous ranges which match the filter''s X *and* Y range
      // are stored in the record link
      if (!complexShape &&
          (x < _queryFilter.xLow || x > _queryFilter.xHigh
           || y < _queryFilter.yLow || y > _queryFilter.yHigh)) {
	if (i > firstRec)
	  WriteMasterLink(recId + firstRec, i - firstRec);

	// Next contiguous batch of record id''s starts at i+1
	firstRec = i + 1;
      }
      
      tp += gRecSize;
    }

    if (numGData > firstRec)
      WriteMasterLink(recId + firstRec, numGData - firstRec);
  }
  
  // Draw data only if window is not iconified
  if (!Iconified()) {
    if (_batchRecs) {
      _dataBin->InsertSymbol(recId, gdata, numGData, 0, 1, canElimRecords);
#ifdef DEBUG
      _dataBin->PrintStat();
#endif
    } else {
      char *ptr = (char *)gdata;
      for(int i = 0; i < numGData; i++) {
        _dataBin->InsertSymbol(recId, ptr, 1, 0, 1, canElimRecords);
        recId++;
        ptr += gRecSize;
      }
    }
  }
}

/* Done with query */

void TDataViewX::QueryDone(int bytes, void *userData)
{
#ifdef DEBUG
    //printf("TDataViewX::Query done, index = %d, bytes = %d\n", _index, bytes);
#endif

  _pstorage.Clear();

  DOASSERT(_index >= 0, "Invalid iterator index");

  if (MoreMapping(_index)) {
    _map = NextMapping(_index)->map;
#ifdef DEBUG
    //printf("Submitting next query 0x%p\n", _map);
#endif
    _queryProc->BatchQuery(_map, _queryFilter, this, 0, _timestamp);
    return;
  }

  DoneMappingIterator(_index);
  _map = 0;
  _index = -1;

  _allStats.Done();
  _allStats.Report();

  for(int i = 0; i < MAXCOLOR; i++)
    _stats[i].Done();

  PrepareStatsBuffer();

  _dataBin->Final();

  DrawLegend();

  // Finish record links whose master this view is
  int index = _masterLink.InitIterator();
  while(_masterLink.More(index)) {
    RecordLink *link = _masterLink.Next(index);
    link->Done();
  }
  _masterLink.DoneIterator(index);

  ReportQueryDone(bytes);
}

void TDataViewX::ReturnGDataBinRecs(TDataMap *map, void **recs, int numRecs)
{
#ifdef DEBUG
  //printf("TDataViewX %d recs buf start 0x%p\n", numRecs, recs);
#endif

  WindowRep *win = GetWindowRep();

  if (IsInPileMode()) {
    ViewWin *parent = GetParent();
    DOASSERT(parent, "View has no parent");
    int index = parent->InitIterator();
    DOASSERT(parent->More(index), "Parent view has no children");
    ViewWin *vw = parent->Next(index);
    win = vw->GetWindowRep();
    parent->DoneIterator(index);
#ifdef DEBUG
    //printf("Drawing view %s in view %s, window 0x%p\n",
           GetName(), vw->GetName(), win);
#endif
  }
      
  map->DrawGDataArray(this, win, recs, numRecs);
}

void TDataViewX::ReturnGDataBinConnectors(TDataCMap *cmap,
					  Connector **connectors, int num)
{
#ifdef DEBUG
  //printf("TDataViewX drawing %d connectors\n", num);
#endif

  for(int i = 0; i < num; i++)
    ConnectorShapeRegistrar::DrawConnection(GetWindowRep(), connectors[i]);
}

void TDataViewX::PrintStat()
{
  if (Init::PrintViewStat()) {
    View::PrintStat();
    //printf("%d GData records, %d batches, %.2f per batch\n",
	   _totalGData, _numBatches, 1.0 * _totalGData / _numBatches);
    _dataBin->PrintStat();
  }
}
