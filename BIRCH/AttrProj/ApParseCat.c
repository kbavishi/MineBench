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
  Module for reading physical and logical schemas.
 */

/*
  $Id: ApParseCat.c 2396 2014-03-13 21:13:27Z wkliao $

  $Log: ApParseCat.c,v $
  Revision 1.10  1996/10/10 16:45:16  wenger
  Changed function names, etc., in ApParseCat.c to get rid of name clashes
  when Donko puts transformation engine code into DEVise.

  Revision 1.9  1996/08/27 19:06:58  flisakow
  Added ifdef's around some information printf's.

  Revision 1.8  1996/08/15 19:54:46  wenger
  Added 'pure' targets for attrproj and devread; fixed some dynamic
  memory problems.  Found some bugs while demo'ing for soils science
  people.

  Revision 1.7  1996/07/01 20:36:59  jussi
  Minor changes to reflect the new TDataAscii/TDataBinary constructor
  interface.

  Revision 1.6  1996/06/27 18:11:59  wenger
  Re-integrated most of the attribute projection code (most importantly,
  all of the TData code) into the main code base (reduced the number of
  modules used only in attribute projection).

  Revision 1.5  1996/06/17 20:01:03  wenger
  First version of 'show' program for dumping projections to stdout.

  Revision 1.4  1996/06/07 19:40:34  wenger
  Integrated some of the special attribute projection sources back
  into the regular Devise sources.

  Revision 1.3  1996/04/30 15:31:37  wenger
  Attrproj code now reads records via TData object; interface to Birch
  code now in place (but not fully functional).

  Revision 1.2  1996/04/25 19:25:10  wenger
  Attribute projection code can now parse a schema, and create the
  corresponding TData object.

  Revision 1.1  1996/04/22 18:01:47  wenger
  First version of "attribute projection" code.  The parser (with
  the exception of instantiating any TData) compiles and runs.

*/

#include <stdio.h>

#include "ApParseCat.h"
#include "AttrList.h"
#include "GroupDir.h"
#include "Parse.h"
#include "ApInit.h"
#include "DeviseTypes.h"
#include "TDataAsciiInterp.h"
#include "TDataBinaryInterp.h"
#include "Util.h"

//#define DEBUG

static GroupDir *gdir = new GroupDir();

#define LINESIZE 512

static int numAttrs          = 0;
static AttrList *attrs       = 0;

static int _line = 0;

/*------------------------------------------------------------------------------
 * function: SetVal
 * Set the value field in aval to the value equivalent of valstr based
 * on the valtype.
 */
static void
SetVal(AttrVal *aval, char *valstr, AttrType valtype)
{
  double tempval;

  switch(valtype) {
    case IntAttr: 
      aval->intVal = atoi(valstr);
      break;
    case FloatAttr:
      aval->floatVal = atof(valstr);
      break;
    case DoubleAttr:
      aval->doubleVal = atof(valstr);
      break;
    case StringAttr:
      aval->strVal = CopyString(valstr);
      break;
    case DateAttr:
      (void)ParseFloatDate(valstr, tempval);
      aval->dateVal = (time_t)tempval;
      break;
    default:
      fprintf(stderr,"unknown attr value\n");
      Exit::DoExit(2);
      break;
    }
}

#ifndef NO_GEN_CLASS_INFO

const int MAX_GENCLASSINFO = 20;
static int _numGenClass = 0;

static struct { 
  char *source;
  GenClassInfo *genInfo;
} _genClasses[MAX_GENCLASSINFO];

/*------------------------------------------------------------------------------
 * function: ApRegisterGenClassInfo
 * Register the TData class generator for a given source.
 */
void
RegisterGenClassInfo(char *source, GenClassInfo *gen)
{
  if (_numGenClass == MAX_GENCLASSINFO) {
    fprintf(stderr, "too many interpreted TData class generator\n");
    Exit::DoExit(1);
  }
  _genClasses[_numGenClass].source = source;
  _genClasses[_numGenClass++].genInfo = gen;
}

/*------------------------------------------------------------------------------
 * function: FindGenClass
 * Find the TData generator for a given source.
 */
static GenClassInfo *
FindGenClass(char *source)
{
  for(int i = 0; i < _numGenClass; i++) {
    if (strcmp(_genClasses[i].source,source) == 0)
      return _genClasses[i].genInfo;
  }

  fprintf(stderr,"Can't find TData generator for input source %s\n",source);
  Exit::DoExit(1);
  
  // keep compiler happy
  return 0;
}
#endif

/*------------------------------------------------------------------------------
 * function: ParseChar
 * Parse a character, Return false if can't parse.
 */
static Boolean
ParseChar(char *instr, char &c)
{
  char *str = instr;
  if (*str == '\\') {
    str++;
    switch(*str) {
    case 'n':
      c = '\n';
      break;
    case 'r':
      c = '\r';
      break;
    case 't':
      c = '\t';
      break;
    case '\'':
      c = '\'';
      break;
    default:
      goto error;
      break;
    }
  } else
    c = *str;
  return true;

 error:
  fprintf(stderr, "ParseCat: invalid separator %s\n", instr);
  return false;
}

/* Parse separators */
const int MAX_SEPARATORS = 50;
static char separators[MAX_SEPARATORS];
static int numSeparators;

/*------------------------------------------------------------------------------
 * function: ParseSeparator
 * Parse a separator; return false if can't parse.
 */
static Boolean
ParseSeparator(int numArgs, char **args)
{
  if (numArgs >= MAX_SEPARATORS) {
    fprintf(stderr, "ParseCat: too many separators, max = %d\n",
	    MAX_SEPARATORS);
    return false;
  }

  for(int i = 1; i < numArgs; i++) {
    if (!ParseChar(args[i], separators[i - 1]))
      return false;
  }

  numSeparators = numArgs - 1;
  return true;
}


static char whitespaces[MAX_SEPARATORS];
static int numWhitespace;

/*------------------------------------------------------------------------------
 * function: ParseWhiteSpace
 * Parse whitespace; return false if can't parse.
 */
static Boolean
ParseWhiteSpace(int numArgs, char **args)
{
  if (numArgs >= MAX_SEPARATORS) {
    fprintf(stderr, "ParseCat: too many separators, max = %d\n",
	    MAX_SEPARATORS);
    return false;
  }

  for(int i = 1; i < numArgs; i++) {
    if (!ParseChar(args[i], whitespaces[i - 1]))
      return false;
  }

  numWhitespace = numArgs - 1;
  return true;
}

/*------------------------------------------------------------------------------
 * function: ParseAttr
 * Parse an attribute.
 */
static DevStatus
ParseAttr(
	int &	numArgs,
	char **	args,
	int &	recSize,
	Boolean	hasFileType,
	char *	fileType)
{
	int			attrLength;
	AttrType	attrType;
	DevStatus	result = StatusOk;

	/* an attribute */
	Boolean isSorted = false;
	if (strcmp(args[0],"sorted") == 0)
	{
		/* sorted attribute */
		isSorted = true;
		if (strcmp(args[1],"attr") && strcmp(args[1],"compattr"))
		{
			fprintf(stderr,"'sorted' must be followed by 'attr' or 'compattr'\n");
			result = StatusFailed;
			return result;
		}
		args = &args[1];
		numArgs--;
		isSorted = true;
	}

	Boolean isComposite;
	if (strcmp(args[0],"attr") == 0)
		isComposite = false;
	else isComposite = true;

	/* get attr type */
	int attrNum = 0;
	if (numArgs < 3)
	{
		fprintf(stderr,"attr needs at least 3 args\n");
		result = StatusFailed;
		return result;
	}

	if (strcmp(args[2],"int") == 0)
	{
		attrType = IntAttr;
		attrLength = sizeof(int);
		attrNum = 3;
	}
	else if (strcmp(args[2],"double") == 0)
	{
		attrType = DoubleAttr;
		attrLength = sizeof(double);
		attrNum = 3;
	}
	else if (strcmp(args[2],"float") == 0)
	{
		attrType = FloatAttr;
		attrLength = sizeof(float);
		attrNum = 3;
	}
	else if (strcmp(args[2],"date") == 0)
	{
		attrType = DateAttr;
		attrLength = sizeof(long);
		attrNum = 3;
	}
	else if (strcmp(args[2],"string") == 0)
	{
		attrType = StringAttr;
		if (numArgs < 4)
		{
			fprintf(stderr,"string attr needs length\n");
			result = StatusFailed;
			return result;
		}
		attrLength = atoi(args[3]);
		attrNum = 4;
	}
	else
	{
		fprintf(stderr,"unknown type %s\n",args[2]);
		result = StatusFailed;
		return result;
	}

	char *attrName = CopyString(args[1]);

	Boolean hasMatchVal = false;
	AttrVal matchVal;
	Boolean hasHi = false;
	Boolean hasLo = false;
	AttrVal hiVal, loVal;

	if ((attrNum < numArgs) && (!strcmp(args[attrNum], "=")))
	{
		attrNum++;
		if (attrNum > numArgs-1)
		{
	    	fprintf(stderr,"expecting default value after '='\n");
			result = StatusFailed;
			return result;
		}
		hasMatchVal = true;
		SetVal(&matchVal, args[attrNum], attrType);
		attrNum++;
	}
			
	if ((attrNum < numArgs) && 
	    (strcmp(args[attrNum], "hi")) && 
	    (strcmp(args[attrNum], "lo"))) 
	{
		fprintf(stderr, "Unrecognized chars in an attribute definition line\n");
		result = StatusFailed;
		return result;
	} 
	else if (attrNum < numArgs)
	{
		if (!strcmp(args[attrNum], "hi"))
		{
	    	hasHi = true;
	    	attrNum++;
	    	if (attrNum >= numArgs)
	    	{
	    		fprintf(stderr, "Expecting value after keyword hi\n");
				result = StatusFailed;
				return result;
			}
			SetVal(&hiVal, args[attrNum], attrType);
			attrNum++;
		}
			  
		if ((attrNum < numArgs) && 
			(!strcmp(args[attrNum], "lo")))
		{
			hasLo = true;
			attrNum++;
			if (attrNum >= numArgs)
			{
				fprintf(stderr, "Expecting value after keyword lo\n");
				return result;
			}
			SetVal(&loVal, args[attrNum], attrType);
			attrNum++;
		}

		if (attrNum < numArgs)
		{
			fprintf(stderr, "Unrecognized chars in an attribute definition line\n");
			result = StatusFailed;
			return result;
		} 
	}

	if (attrs == NULL)
	{
	    if (!hasFileType )
		{
	        fprintf(stderr,"no file type yet\n");
			result = StatusFailed;
			return result;
		}
		attrs = new AttrList(fileType);
	}

	int roundAmount = 0;
	switch(attrType)
	{
	  case FloatAttr:
	    roundAmount = sizeof(float);
	    break;
	  case DoubleAttr:
	    roundAmount = sizeof(double);
	    break;
	  case StringAttr:
	    roundAmount = sizeof(char);
	    break;
	  case DateAttr:
	    roundAmount = sizeof(time_t);
	    break;
	  case IntAttr:
	    roundAmount = sizeof(int);
	    break;
	  default:
	    fprintf(stderr,"ParseCat: don't know type\n");
	    Exit::DoExit(2);
	}

	if (recSize/roundAmount*roundAmount != recSize)
	{
		/* round to rounding boundaries */
		recSize = (recSize/roundAmount+1)*roundAmount;
	}

	attrs->InsertAttr(numAttrs, attrName, recSize,
			  attrLength, attrType, hasMatchVal,
			  &matchVal, isComposite, isSorted,
			  hasHi, &hiVal, hasLo, &loVal);
	numAttrs++;
	recSize += attrLength;

	delete attrName;

	return result;
}

/*------------------------------------------------------------------------------
 * function: ParseCatPhysical
 * Read and parse a physical schema from a catalog file.
 * physicalOnly should be true if only a physical schema (not a physical
 * schema and a logical schema) is being read.
 */
static char *
ParseCatPhysical(char *catFile, char *dataFile, Boolean physicalOnly,
	TData *&tDataP)
{
	FILE *file= NULL;
	Boolean hasSource = false;
	char *source = 0; /* source of data. Which interpreter we use depends
			     on this */

	char buf[LINESIZE];
	Boolean hasFileType = false;
	Boolean hasSeparator = false;
	Boolean hasWhitespace = false;
	Boolean hasComment = false;

	Boolean isAscii = false;
	Boolean GLoad = true;
	char *fileType = 0;
	int numArgs;
	char **args;
	int recSize = 0;
	char *sep = 0;
	int numSep = 0;
	char *commentString = 0;
	Group *currgrp = NULL;

#if 0
	if (attrs != NULL) delete attrs;
#endif
	attrs = NULL;
	numAttrs = 0;

	/*
	printf("opening file %s\n", catFile);
	*/
	file = fopen(catFile, "r");
	if (file == NULL){
		fprintf(stderr,"ParseCat: can't open file %s\n", catFile);
		goto error;
	}
	_line = 0;
	while ((fgets(buf,LINESIZE, file) != NULL) && strcmp(buf, "endSchema\n"))
	{
		StripTrailingNewline(buf);

		_line++;
		/*
		printf("getting line %s\n", buf);
		*/
		if (buf[0] == '#' || buf[0] == '\n' || buf[0] == '\r')
			continue;
		Parse(buf,numArgs, args);
		if (numArgs == 0)
			continue;

#ifdef DEBUG
		printf("parse: ");
		for(int ind = 0; ind < numArgs; ind++)
		  printf("'%s' ", args[ind]);
		printf("\n");
#endif

		if (strcmp(args[0],"end")== 0)
		{
			break;
		}
		else if (strcmp(args[0],"source") == 0)
		{
			source = CopyString(args[1]);
			hasSource = true;
		}
		else if (strcmp(args[0],"separator")== 0)
		{
			/* parse separator */
			hasSeparator = ParseSeparator(numArgs, args);
			if (!hasSeparator){
				fprintf(stderr,"can't parse separator\n");
				goto error;
			}
		}
		else if (strcmp(args[0],"whitespace")== 0)
		{
			/* parse separator */
			hasWhitespace = ParseWhiteSpace(numArgs, args);
		}
		else if (strcmp(args[0],"comment") == 0)
		{
			if (numArgs != 2){
				fprintf(stderr,"can't parse comment string\n");
				goto error;
			}
			hasComment = true;
			commentString = CopyString(args[1]);
		}
		else if (strcmp(args[0],"type") == 0)
		{
			if (numArgs != 3)
			{
				fprintf(stderr,"can't parse file type need 3 args\n");
				goto error;
			}
			if (strcmp(args[2],"ascii") == 0)
			{
				isAscii = true;
			}
			else if (strcmp(args[2],"binary") == 0)
			{
				isAscii = false;
			}
			else
			{
				fprintf(stderr,"don't know file type %s, must be ascii or binary", args[2]);
				goto error;
			}
			fileType = CopyString(args[1]);
			hasFileType = true;
			if (physicalOnly)
			{
				/* Let's add the schema name to the directory now */
				/* First check if the schema is already loaded, in
				   which case we do nothing more */
				if (gdir->find_entry(StripPath(catFile)))
				{
				  GLoad = false;
				}
				else
				{
#ifdef    DEBUG
				  printf("Adding schema %s to directory\n", StripPath(catFile));
#endif
				  gdir->add_entry(StripPath(catFile));
				  GLoad = true;
				}
			}
		}
		else if (strcmp(args[0],"attr") == 0 ||
			   strcmp(args[0],"compattr") == 0 ||
			   strcmp(args[0],"sorted") == 0)
		{
			if (ParseAttr(numArgs, args, recSize, hasFileType, fileType) !=
				StatusOk) goto error;
		}
		else if (physicalOnly && !strcmp(args[0], "group"))
		{
		  if (GLoad) {
		      if (!currgrp)		/* Top level */
		      {
			currgrp = new Group(args[1], NULL, TOPGRP);
			gdir->add_topgrp(StripPath(catFile), currgrp);
		      }
		      else
			currgrp = currgrp->insert_group(args[1]);
		    }
		}
		else if (physicalOnly && !strcmp(args[0], "item"))
		{
		  if (GLoad)
		  {
		      currgrp->insert_item(args[1]);
		  }
		}
		else if (physicalOnly && !strcmp(args[0], "endgroup"))
		{
		  if (GLoad)
		  {
		      if (!currgrp)
		      {
			fprintf(stderr, "Group begins and ends not matched\n");
			goto error;
		      }
		      currgrp = currgrp->parent_group();
		    }
		}
		else
		{
	    	fprintf(stderr,"ParseCat: unknown command %s\n", args[0]);
	    	goto error;
		}
	}

	/* round record size */
	if (recSize/8*8 != recSize){
		/* round to rounding boundaries */
		recSize = (recSize/8+1)*8;
	}

	if (!hasFileType ){
		fprintf(stderr,"ParseCat: no file type specified\n");
		goto error;
	}

	if (numAttrs == 0){
		fprintf(stderr,"ParseCat: no attribute specified\n");
		goto error;
	}

	int i,j;

	if (physicalOnly)
	{
	/* If no group has been defined, create a default group */
	if (GLoad && (gdir->num_topgrp(StripPath(catFile)) == 0))
	{
	  Group *newgrp = new Group("__default", NULL, TOPGRP);
	  gdir->add_topgrp(StripPath(catFile), newgrp);
	  for (i=0; i < numAttrs; i++) {
	    AttrInfo *iInfo = attrs->Get(i);
	    if (iInfo->type != StringAttr)
	      newgrp->insert_item(iInfo->name);
	  }
	}
	}

	/* test attribute names */
	for (i=0 ; i < numAttrs-1;i++) {
		AttrInfo *iInfo = attrs->Get(i);
		if (strcmp(iInfo->name,"recId") == 0){
			fprintf(stderr,"attribute name 'recId' is reserved\n");
			goto error;
		}
		for (j=i+1; j < numAttrs; j++){
			AttrInfo *jInfo = attrs->Get(j);
			if (strcmp(iInfo->name,jInfo->name)== 0){
				fprintf(stderr,"ParseCat:duplicate attribute name %s\n",
					iInfo->name);
				goto error;
			}
		}
	}

	if (isAscii) {
	  if (hasSeparator && hasWhitespace){
	    fprintf(stderr,"can't specify both whitespace and separator\n");
	    goto error;
	  }
	  if (!(hasSeparator || hasWhitespace)){
	    fprintf(stderr,"must specify either whitespace or separator\n");
	    goto error;
	  }
	}

	if (hasSeparator) {
	  sep = new char[numSeparators];
	  for (i=0; i < numSeparators; i++){
	    sep[i] = separators[i];
	  }
	  numSep = numSeparators;
	}
	if (hasWhitespace) {
	  sep = new char[numWhitespace];
	  for (i=0; i < numWhitespace; i++){
	    sep[i] = whitespaces[i];
	    }
	  numSep = numWhitespace;
	}
	
	if (!hasComment)
	  commentString = "#";
	  
	if (hasSource)
	{
#ifndef NO_GEN_CLASS_INFO
		if (physicalOnly)
		{
			printf("source: %s\n",source);
		}
		else
		{
			printf("schema: %s\n",source);
		}
		GenClassInfo *genInfo = FindGenClass(source);
		ControlPanel::RegisterClass(
			genInfo->Gen(source, isAscii, fileType,
			attrs, recSize,sep, numSep, hasSeparator, commentString),
			true);
#else
		fprintf(stderr, "Illegal token 'source' in schema\n");
		Exit::DoExit(1);
#endif
	}
	else
	{
		// strdups because TData destructor will try to free type
		// strings -- make sure they're dynamic.
		if (isAscii) {
#ifdef    DEBUG
		  printf("default source, recSize %d\n",recSize);
#endif
		  tDataP = new TDataAsciiInterp(catFile, strdup("UNIXFILE"), dataFile,
			recSize, attrs, sep, numSep, hasSeparator, commentString);
		}
		else
		{
#ifdef    DEBUG
		  printf("default binary source, recSize %d\n",recSize);
#endif
		  // Note: the second use of recSize is for the physical
		  // record size.  This needs to get changed.  RKW 96/06/27.
		  tDataP = new TDataBinaryInterp(catFile, strdup("UNIXFILE"), dataFile,
			recSize, recSize/*TEMP*/, attrs);
		}
	}

	fclose(file);

	if (Init::PrintTDataAttr()) attrs->Print();
	return fileType;

error:
	if (file != NULL) fclose(file);

	if (attrs != NULL) delete attrs;
	fprintf(stderr,"error at line %d\n", _line);
	return NULL;
}

/*------------------------------------------------------------------------------
 * function: ParseCatLogical
 * Read and parse a logical schema from a catalog file.
 */
static char *
ParseCatLogical(char *catFile, char *sname)
{
  Group *currgrp = NULL;
  FILE *file= NULL;
  Boolean GLoad = true;
  char buf[LINESIZE];
  int numArgs;
  char **args;

  file = fopen(catFile, "r");
  if (file == NULL) {
    fprintf(stderr,"ParseCat: can't open file %s\n", catFile);
    goto error;
  }
  _line = 0;
  
  /* read the first line first */
  fgets(buf, LINESIZE, file);
  
  /* Let's add the group name to the directory now */
  /* The groups for a particular logical schema are identified by the 
     schema file name. This is bcos the type name of the physical schema
     is not a unique identifier - several logical schemas may use the same
     physical schema */
  /* First check if the schema is already loaded, in
     which case we do nothing more */

  if (gdir->find_entry(StripPath(catFile)))
    GLoad = false;
  else
  {
    printf("Adding schema %s to directory \n", StripPath(catFile));
    gdir->add_entry(StripPath(catFile));
    GLoad = true;
  }
 
  while (fgets(buf,LINESIZE, file) != NULL) {
	  StripTrailingNewline(buf);
      
      _line++;
      /*
	 printf("getting line %s\n", buf);
	 */
      if (buf[0] == '#' || buf[0] == '\n' || buf[0] == '\r')
	continue;
      Parse(buf,numArgs, args);
      if (numArgs == 0)
	continue;
     
#ifdef DEBUG
      printf("parse: ");
      for(int ind = 0; ind < numArgs; ind++)
	printf("'%s' ", args[ind]);
      printf("\n");
#endif

      if (strcmp(args[0], "group") == 0)
      {
	if (GLoad) {
	    if (!currgrp)		/* Top level */
	    {
	      currgrp = new Group(args[1], NULL, TOPGRP);
	      gdir->add_topgrp(StripPath(catFile), currgrp);
	    }
	    else
	      currgrp = currgrp->insert_group(args[1]);
	  }
      }
      else if (strcmp(args[0], "item") == 0)
      {
	if (GLoad) {
	    currgrp->insert_item(args[1]);
	}
      }
      else if (strcmp(args[0], "endgroup") == 0)
      {
	if (GLoad) {
	    if (!currgrp)
	    {
	      fprintf(stderr, "Group begins and ends not matched\n");
	      goto error;
	    }
	    currgrp = currgrp->parent_group();
	  }
      }
      else {
	  fprintf(stderr,"ParseCat: unknown command %s\n", args[0]);
	  goto error;
      }
  }

  /* If no group has been defined, create a default group */
  if (GLoad && (gdir->num_topgrp(StripPath(catFile)) == 0))
  {
    Group *newgrp = new Group("__default", NULL, TOPGRP);
    gdir->add_topgrp(StripPath(catFile), newgrp);
    for(int i = 0; i < numAttrs; i++) {
      AttrInfo *iInfo = attrs->Get(i);
      if (iInfo->type != StringAttr)
	newgrp->insert_item(iInfo->name);
    }
  }

  fclose(file);

  return sname;

 error:
  if (file != NULL)
    fclose(file);
  
  fprintf(stderr,"error at line %d\n", _line);
  return NULL;
}

/*------------------------------------------------------------------------------
 * function: ApParseCat
 * Read and parse a schema file.
 */
char *
ApParseCat(char *catFile, char *dataFile, TData *&tDataP) 
{
  // Check the first line of catFile - if it is "physical abc",
  // call ParseCatPhysical(abc, false) and then ParseCatLogical(catFile)
  // Otherwise, simply call ParseCatPhysical(catFile, true).

  char *	result = NULL;

  FILE *fp = fopen(catFile, "r");
  if (!fp)
  {
    fprintf(stderr,"ParseCat: can't open file %s\n", catFile);
  }
  else
  {
    char buf[100];
    if (fscanf(fp, "%s", buf) != 1 || strcmp(buf, "physical"))
	{
      fclose(fp);
      result = ParseCatPhysical(catFile, dataFile, true, tDataP);
    }
	else
	{
      // Read in the file name
      fscanf(fp, "%s", buf);
      fclose(fp);

      char *sname;
      if (!(sname = ParseCatPhysical(buf, dataFile, false, tDataP)))
	  {
		result = NULL;
	  }

      result = ParseCatLogical(catFile, sname);
	}
  }

  return result;
}

/*------------------------------------------------------------------------------
 * function: ApParseSchema
 * Parse a schema from buffer(s).
 */
char *
ApParseSchema(char *schemaName, char *physSchema, char *logSchema)
{
	char *		result = NULL;

	return result;
}
