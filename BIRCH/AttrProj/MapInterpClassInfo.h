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
  $Id: MapInterpClassInfo.h 2396 2014-03-13 21:13:27Z wkliao $

  $Log: MapInterpClassInfo.h,v $
  Revision 1.5  1996/06/15 14:25:02  jussi
  Rewrote so that a variable number of shape attributes is
  allowed when mapping is created.

  Revision 1.4  1996/04/14 00:18:15  jussi
  Removed the extraneous data structures and methods for recording
  new mapping class names. ClassDir records this information for
  us already!

  Revision 1.3  1996/01/13 23:09:56  jussi
  Added support for Z attribute and shape attribute 2.

  Revision 1.2  1995/09/05 22:15:00  jussi
  Added CVS header.
*/

#ifndef MapInterpClassInfo_h
#define MapInterpClassInfo_h

#include "ClassDir.h"
#include "VisualArg.h"

class MappingInterp;
class TData;
class MappingInterpCmd;

class MapInterpClassInfo: public ClassInfo {
public:
	/* constructor for prototype MapInterp */
	MapInterpClassInfo();
	virtual ~MapInterpClassInfo();

	/* return user info, which is whether this is an interpreted 
	mapping */
	void *UserInfo() { return &_isInterp; }

	/* constructor for new interpreted mapping class */
	MapInterpClassInfo(char *className);

	/* constructor for interpreted mapping instance */
	MapInterpClassInfo(char *className,
		char *fileAlias, char *name, VisualFlag *dimensionInfo,
		int numDimensions, MappingInterp *map, TData *tdata, 
		MappingInterpCmd *cmd, int cmdFlag, int attrFlag);

	/* Info for category */
	virtual char *CategoryName() { return "mapping"; }

	/* Info for class */
	virtual char *ClassName(){ return _className; }

	/* Get name of parameters */
	virtual void ParamNames(int &argc, char **&argv);

	/* Create instance using the supplied parameters. Return 
	the instance info if successful, otherwise return NULL. */
	virtual ClassInfo *CreateWithParams(int argc, char **argv) ;

	/* Return true is parameters can be changed dynamically at run time */
	virtual Boolean Changeable() { return true; }

	/* Change parameters dynamically at run time */
	virtual void ChangeParams(int argc, char **argv);

	/* Set default parameters */
	void SetDefaultParams(int argc, char **argv);

	/* Get default parameters */
	void GetDefaultParams(int &argc, char **&argv);

	/**************************************************
	Instance Info. 
	***************************************************/
	virtual char *InstanceName();
	virtual void *GetInstance();

	/* Get parameters that can be used to re-create this instance */
	virtual void CreateParams(int &argc, char **&argv);

private:
	void ExtractCommand(int argc, char **argv, MappingInterpCmd *cmd,
			    unsigned long int &cmdFlag,
			    unsigned long int &attrFlag,
			    VisualFlag *dimensionInfo, int &numDimensions,
			    char *&tdataAlias, TData *&tdata, char *&name);

	char *_fileAlias;
	char *_className; /* name of this interpreted mapping class */
	MappingInterpCmd *_cmd;
	unsigned long int _attrFlag;
	unsigned long int _cmdFlag;
	char *_name;
	MappingInterp *_map;
	TData *_tdata;
	VisualFlag *_dimensionInfo;
	int _numDimensions;

	int _isInterp; /* always set to true */
};

#endif
