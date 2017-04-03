/*
  $Id: TimeStamp.h 2396 2014-03-13 21:13:27Z wkliao $

  $Log: TimeStamp.h,v $
  Revision 1.2  1995/09/05 21:13:08  jussi
  Added/update CVS header.
*/

#ifndef TimeStamp_h
#define TimeStamp_h

class TimeStamp {
public:
	static int NextTimeStamp() { return _timeStamp++; }
private:
	static int _timeStamp;
};
#endif
