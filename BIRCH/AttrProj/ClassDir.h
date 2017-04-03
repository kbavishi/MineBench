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
  $Id: ClassDir.h 2396 2014-03-13 21:13:27Z wkliao $

  $Log: ClassDir.h,v $
  Revision 1.6  1996/05/11 19:09:52  jussi
  Minor fix.

  Revision 1.5  1996/05/11 01:54:28  jussi
  Added DestroyAllInstances() method.

  Revision 1.4  1996/04/13 23:39:38  jussi
  Added copyright notice and cleaned up the code a bit.

  Revision 1.3  1995/09/09 00:24:52  jussi
  Increased MaxClasses from 50 to 100.

  Revision 1.2  1995/09/05 21:12:26  jussi
  Added/update CVS header.
*/

#ifndef ClassDir_h
#define ClassDir_h

#include "DeviseTypes.h"

/*
   class information. This is set up to store both class and instance
   information. For classInfo, only methods dealing with class are
   used. For instance info, methods dealing with instances are alos used
*/

class InstanceInfo;

class ClassInfo {
public:
	ClassInfo();
	virtual ~ClassInfo();

	/* Set/test transient status. A transient class is destroyed
	upon closing a session */
	void SetTransient(Boolean transient) { _transient = transient; }
	Boolean IsTransient() { return _transient; }

	/* Info for category */
	virtual char *CategoryName() = 0;

	/* Info for class */
	virtual char *ClassName()=0; 	/* name of class */

	/* Get name of parameters and default/current values */
	virtual void ParamNames(int &argc, char **&argv) = 0;

	/* Create instance using the supplied parameters. Return 
	the instance info if successful, otherwise return NULL. */
	virtual ClassInfo *CreateWithParams(int argc, char **argv) = 0;

	/* Return true is parameters can be changed dynamically at run time */
	virtual Boolean Changeable() { return false; }

	/* Change parameters dynamically at run time */
	virtual void ChangeParams(int argc, char **argv){};


	/* Set default parameters */
	void SetDefaultParams(int argc, char **argv);

	/* Get default parameters */
	void GetDefaultParams(int &argc, char **&argv);

	/* Get user info */
	virtual void *UserInfo() { return 0; }

	/**************************************************
	Instance Info. 
	***************************************************/
	virtual char *InstanceName();
	virtual void *GetInstance();

	/* Get parameters that can be used to re-create this instance */
	virtual void CreateParams(int &argc, char **&argv);

private:
	char **_defaultParams;
	int _numDefaultParams;
	Boolean _transient; /* true if a transient class */
};


const int MaxCategories = 10;	/* max # of categories */
const int MaxClasses = 100;	/* # of classes for each category */
const int MaxInstances = 100;	/* # of instances for each class */
struct ClassRec {
	ClassInfo *classInfo;
	int _numInstances;
	ClassInfo *_instances[MaxInstances];
};

struct CategoryRec {
	char *name;
	ClassRec *_classRecs[MaxClasses];
	int _numClasses;
};

class ClassDir {
public:
	ClassDir();

	void InsertCategory(char *name);
	void InsertClass(ClassInfo *cInfo);

	/* Get name of all classes in a category */
	void ClassNames(char *category, int &numClasses, char **&classNames);

	/* Get creation parameters for a class */
	void GetParams(char *category, char *className, 
		int &numParams, char **&paramNames);
	void GetParams(char *inst, int &numParams, char **&paramNames);

	/* Get user info for a class */
	void *UserInfo(char *category, char *className);

	/* Set default parameter values for a class */
	void SetDefault(char *category, char *className, int numParams,
		char **params);

	/* Create a new instance with parameters. Return the name  of
	new instance, or NULL if not successful */
	char *CreateWithParams(char *category, char *className,
		int numParams, char **paramNames);


	/* Get name of all instances for a given class */
	void InstanceNames(char *category, char *className,
		int &num, char **&instanceNames);

	/* Get param names for instance */
	void InstanceNames(char *name, int &num, char **&instanceNames);

	/* Get pointer to all instances for a given class */
	void InstancePointers(char *category, char *className,
		int &num, char **&instancePointers);

	/* Find instance with given name */
	void *FindInstance(char *name);

	/* Find name for instance */
	char *FindInstanceName(void *instance);

	/* Destroy all instances */
	void DestroyAllInstances();

	/* Destroy an instance */
	void DestroyInstance(char *instanceName);

	/* Destroy all transient classes */
	void DestroyTransientClasses();

	/* REturn true if instance is changeable */
	Boolean Changeable(char *name);

	/* Change with params */
	void ChangeParams(char *instance, int num, char **paramNames);


	/* Get the creation parameters for an instance */
	void CreateParams(char *category, char *className, char *instanceName,
		int &numParams, char **&params);

	void Print();

private:
	CategoryRec *_categories[MaxCategories];
	int _numCategories;
};

#endif
