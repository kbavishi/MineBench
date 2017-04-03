/*
  $Id: Journal.h 2396 2014-03-13 21:13:27Z wkliao $

  $Log: Journal.h,v $
  Revision 1.2  1995/09/05 21:12:55  jussi
  Added/update CVS header.
*/

/*
   Journal.h: keeps track of events to selection, and stop/go buttons
*/

#ifndef Journal_h
#define Journal_h
#include "DeviseTypes.h"
#include "VisualArg.h"

class View;
class Selection;

const int JournalMagicLength= 4; /* length of magic header for journal file */

class Journal {
public:
	/* start == push of Start button.
		Stop == push stop button
		Step == push step button.
		PushSelection == push selection
		Completion == completion of Push/Change/Pop
	*/
	enum EventType { Start, Stop, Step, PushSelection, ChangeSelection,
		PopSelection, Completion};
	
	static void Init(char *journalName, int argc, char **argv);

	/* record event into the playback file */
	static void RecordEvent(EventType type, Selection *selection,
		View *v, VisualFilter *filter, VisualFilter *hint,
		int numGetPage, int numHits, int numPrefetch, int numPrefetchHits);
	
	static Journal::EventType LastEvent(){ return _lastEvent; };

	static void StopRecording();

	/* Playback from the given file.
	printHeader == TRUE to print header. */
	static void InitPlayback(char *file, Boolean printHeader= false);

	/* get next event, but returning only the ids of selection and view .
	Return true if done */
	static Boolean NextEvent(long &time,
		EventType &type, int &selectionId, int &viewId, VisualFilter &filter,
		VisualFilter &hint, int &numGetPage, int &numHits,
		int &numPrefetch, int &numPrefetchHits);

	/* Get next event. Return TRUE if no more.
	The actually selections and views are returned. */
	static Boolean NextEvent(long &time,
		EventType &type, Selection *&selection, View *&v, VisualFilter &filter,
		VisualFilter &hint, int &numGetPage, int &numHits,
		int &numPrefetch, int &numPrefetchHits);

private:
	static int _fd;	/* file descriptor */
	static int _playFd;	/* file descriptor for playback */
	static long _time; /* last time something was done */
	static char *_magic; /* magic header to identify trace file */
	static EventType _lastEvent; /* type of last event recorded */
	static Boolean _opened; /* true if journal file opened */
};

#endif
