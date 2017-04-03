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
  Implementation of AttrProj (attribute projection) class.
 */

/*
  $Id: AttrProj.c 2396 2014-03-13 21:13:27Z wkliao $

  $Log: AttrProj.c,v $
  Revision 1.16  1996/12/03 20:26:03  jussi
  Updated to reflect new TData interfaces.

  Revision 1.15  1996/11/23 21:35:33  jussi
  Minor change to reflect change in TData::GetRecs().

  Revision 1.14  1996/11/05 19:46:48  wenger
  Fixed some bugs in the attribute projection code (handles blanks lines
  and much longer lines in projection files); removed unneeded -lpthread
  from Makefile for solaris attribute projection.

  Revision 1.13  1996/10/10 16:45:19  wenger
  Changed function names, etc., in ApParseCat.c to get rid of name clashes
  when Donko puts transformation engine code into DEVise.

  Revision 1.12  1996/08/23 16:54:59  wenger
  First version that allows the use of Dali to display images (more work
  needs to be done on this); changed DevStatus to a class to make it work
  better; various minor bug fixes.

  Revision 1.11  1996/08/15 19:54:48  wenger
  Added 'pure' targets for attrproj and devread; fixed some dynamic
  memory problems.  Found some bugs while demo'ing for soils science
  people.

  Revision 1.10  1996/08/02 15:53:36  wenger
  Added AttrProj member functions for reading entire records (no projection).

  Revision 1.9  1996/07/31 19:33:37  wenger
  Added AttrProj member functions for reading entire records (no projection).

  Revision 1.8  1996/06/27 18:12:04  wenger
  Re-integrated most of the attribute projection code (most importantly,
  all of the TData code) into the main code base (reduced the number of
  modules used only in attribute projection).

  Revision 1.7  1996/06/19 19:55:52  wenger
  Improved UtilAtof() to increase speed; updated code for testing it.

  Revision 1.6  1996/06/17 20:01:07  wenger
  First version of 'show' program for dumping projections to stdout.

  Revision 1.5  1996/05/14 15:34:48  wenger
  Added GetDataSize method to AttrProj class; removed vector.o from
  AttrProjLib.o; various cleanups.

  Revision 1.4  1996/05/01 16:19:32  wenger
  Initial version of code to project attributes now working.

  Revision 1.3  1996/04/30 18:53:32  wenger
  Attrproj now generates a single projection of all attributes of the
  real data.

  Revision 1.2  1996/04/30 15:31:50  wenger
  Attrproj code now reads records via TData object; interface to Birch
  code now in place (but not fully functional).

  Revision 1.1  1996/04/25 19:25:22  wenger
  Attribute projection code can now parse a schema, and create the
  corresponding TData object.

 */

#define _AttrProj_c_

//#define DEBUG

#include <stdio.h>
#include <string.h>

#include "AttrProj.h"
#include "ApParseCat.h"
#include "Util.h"
#include "TData.h"
#include "AttrList.h"
#include "ProjectionList.h"
#include "DataSeg.h"

/* The type of data stored in a Vector. */
#define VECTOR_TYPE	double

static double AttrToDouble(AttrType type, char *valP);

#if !defined(lint) && defined(RCSID)
static char		rcsid[] = "$RCSfile: AttrProj.c,v $ $Revision: 2396 $ $State: Exp $";
#endif

static char *	srcFile = __FILE__;

/*------------------------------------------------------------------------------
 * function: AttrProj::AttrProj
 * AttrProj constructor.
 */
AttrProj::AttrProj(char *schemaFile, char *attrProjFile, char *dataFile)
{
	DO_DEBUG(printf("AttrProj::AttrProj(%s, %s, %s)\n", schemaFile,
		attrProjFile, dataFile));

	DOASSERT(dataFile != NULL, "Can't have NULL datafile");

	// Provision for having the schema in the data file.
	if ((schemaFile == NULL) || !strcmp(schemaFile, ""))
	{
		schemaFile = dataFile;
	}

	// strdups because TData destructor will try to free all of these
	// strings -- make sure they're dynamic.
	schemaFile = strdup(schemaFile);
	attrProjFile = strdup(attrProjFile);
	dataFile = strdup(dataFile);

	DataSeg::Set(schemaFile, dataFile, 0, 0);

	char *schemaName = ApParseCat(schemaFile, dataFile, _tDataP);
	DOASSERT(schemaName != NULL, "Can' parse schema");

	DevStatus ppRes = ParseProjection(attrProjFile);
	DOASSERT(ppRes.IsComplete(), "Can't parse projection");

	_recBufSize = _tDataP->RecSize();
	_recBuf = new char[_recBufSize];

	_attrCounts = new int[_projList.GetProjCount()];
	_projSizes = new int[_projList.GetProjCount()];

	int		projNum = 0;
	Projection *	projP = _projList.GetFirstProj();
	while (projP != NULL)
	{
		_attrCounts[projNum] = projP->attrCount;
		_projSizes[projNum] = projP->attrCount * sizeof(VECTOR_TYPE);

		projP = _projList.GetNextProj();
		projNum++;
	}
}

/*------------------------------------------------------------------------------
 * function: AttrProj::~AttrProj
 * AttrProj destructor.
 */
AttrProj::~AttrProj()
{
	DO_DEBUG(printf("AttrProj::~AttrProj()\n"));

	delete _tDataP;
	delete [] _recBuf;
	delete [] _attrCounts;
	delete [] _projSizes;
}

/*------------------------------------------------------------------------------
 * function: AttrProj::FirstRecId
 * Get the ID of the first record available.
 */
DevStatus
AttrProj::FirstRecId(RecId &recId)
{
	DO_DEBUG(printf("AttrProj::FirstRecId()\n"));

	DevStatus	result = StatusOk;

	if (!_tDataP->HeadID(recId)) result = StatusFailed;

	return result;
}
/*------------------------------------------------------------------------------
 * function: AttrProj::LastRecId
 * Get the record ID of the last record available.
 */
DevStatus
AttrProj::LastRecId(RecId &recId)
{
	DO_DEBUG(printf("AttrProj::LastRecId()\n"));

	DevStatus	result = StatusOk;

	if (!_tDataP->LastID(recId)) result = StatusFailed;

	return result;
}

/*------------------------------------------------------------------------------
 * function: AttrProj::GetDataSize
 * Returns information about the size of data that will be produced when
 * a record is read.
 */
DevStatus
AttrProj::GetDataSize(int &projCount, const int *&attrCounts,
	const int *&projSizes)
{
	DO_DEBUG(printf("AttrProj::GetDataSize()\n"));

	DevStatus	result = StatusOk;

	projCount = _projList.GetProjCount();
	attrCounts = _attrCounts;
	projSizes = _projSizes;

	return result;
}

/*------------------------------------------------------------------------------
 * function: AttrProj::GetWholeRecSize
 * Returns information about the size of data that will be produced when
 * an entire record (not its projections) is read.
 */
DevStatus
AttrProj::GetWholeRecSize(int &attrCount, int &recSize)
{
	DO_DEBUG(printf("AttrProj::GetWholeRecSize()\n"));

	DevStatus	result = StatusOk;

	attrCount = 0;
	recSize = 0;

	AttrList *attrListP = _tDataP->GetAttrList();

	attrListP->InitIterator();
	while (attrListP->More())
	{
		AttrInfo *attrInfoP = attrListP->Next();
		attrCount++;
		recSize += sizeof(VECTOR_TYPE);
	}
	attrListP->DoneIterator();

	return result;
}

/*------------------------------------------------------------------------------
 * function: AttrProj::CreateRecordList
 * Creates a record list (VectorArray) that can hold the projected
 * records specified in the attribute projection file.
 */
DevStatus
AttrProj::CreateRecordList(VectorArray *&vecArrayP)
{
	DO_DEBUG(printf("AttrProj::CreateRecordList()\n"));

	DevStatus	result = StatusOk;

	vecArrayP = new VectorArray(_projList.GetProjCount());

	int		projNum = 0;
	Projection * projP = _projList.GetFirstProj();
	while (projP != NULL)
	{
		vecArrayP->Init(projNum, projP->attrCount);
		projP = _projList.GetNextProj();
		projNum++;
	}

	return result;
}

/*------------------------------------------------------------------------------
 * function: AttrProj::ReadRec
 * Read a record from the data source, and project it onto the attribute
 * combinations corresponding to this object.
 */
DevStatus
AttrProj::ReadRec(RecId recId, VectorArray &vecArray)
{
	DO_DEBUG(printf("AttrProj::ReadRec(%d)\n", (int) recId));

	int			dataSize;
	int			numRecs;
	DevStatus	result = StatusOk;

	_tDataP->InitGetRecs(recId, recId);

	if (!_tDataP->GetRecs(_recBuf, _recBufSize, recId, numRecs, dataSize))
	{
		result = StatusFailed;
	}
	else
	{
		AttrList *	attrListP = _tDataP->GetAttrList();
		int		projNum = 0;
		Projection * projP = _projList.GetFirstProj();
		while (projP != NULL)
		{
			Vector *	vectorP = vecArray.GetVector(projNum);
			int		projAttrNum;
			for (projAttrNum = 0; projAttrNum < projP->attrCount;
				projAttrNum++)
			{
				int			attrNum = projP->attrList[projAttrNum];
				AttrInfo *	attrInfoP = attrListP->Get(attrNum);

				vectorP->value[projAttrNum] = AttrToDouble(attrInfoP->type,
					_recBuf + attrInfoP->offset);
			}

			projP = _projList.GetNextProj();
			projNum++;
		}
	}

	return result;
}

/*------------------------------------------------------------------------------
 * function: AttrProj::ReadWholeRec
 * Read an entire record (not its projections).
 */
DevStatus
AttrProj::ReadWholeRec(RecId recId, Vector &vector)
{
	DO_DEBUG(printf("AttrProj::ReadWholeRec()\n"));

	DevStatus	result = StatusOk;

	_tDataP->InitGetRecs(recId, recId);

	int			dataSize;
	int			numRecs;

	if (!_tDataP->GetRecs(_recBuf, _recBufSize, recId, numRecs, dataSize))
	{
		result = StatusFailed;
	}
	else
	{
		AttrList *	attrListP = _tDataP->GetAttrList();
		int			attrNum = 0;

		attrListP->InitIterator();
		while (attrListP->More())
		{
			AttrInfo *attrInfoP = attrListP->Next();

			vector.value[attrNum] = AttrToDouble(attrInfoP->type,
				_recBuf + attrInfoP->offset);
			attrNum++;
		}
		attrListP->DoneIterator();
	}

	return result;
}

/*------------------------------------------------------------------------------
 * function: AttrProj::ParseProjection
 * Parse the attribute projection file and build up the corresponding
 * data structures.
 */
DevStatus
AttrProj::ParseProjection(char *attrProjFile)
{
	DO_DEBUG(printf("AttrProj::ParseProjection()\n"));

	DevStatus	result = StatusOk;

	FILE *		file = fopen(attrProjFile, "r");
	if (file == NULL)
	{
		fprintf(stderr, "Can't open attribute projection file\n");
		result = StatusFailed;
	}
	else
	{
		const int	bufSize = 4096;
		char		buf[bufSize];
		char		separators[] = " \t";

		/* Get each line in the attribute projection file. */
		while (fgets(buf, bufSize, file) != NULL)
		{
			DOASSERT(buf[strlen(buf)-1] == '\n',
				"Projection file line too long");

			/* TEMPTEMP -- we should look for some kind of comment char. */

			StripTrailingNewline(buf);
			DO_DEBUG(printf("%s\n", buf));

			Projection	projection;
			char *		token = strtok(buf, separators);
			if (token == NULL) continue;
			projection.attrCount = atoi(token);
			DO_DEBUG(printf("projection.attrCount = %d\n",
				projection.attrCount));
			projection.attrList = new int[projection.attrCount];

			AttrList *	attrListP = _tDataP->GetAttrList();
			int			attrCount = attrListP->NumAttrs();
			int			projAttrNum = 0;

			/* Find each attribute specified for this projection. */
			while ((token = strtok(NULL, separators)) != NULL)
			{
				projection.attrList[projAttrNum] = illegalAttr;
				DO_DEBUG(printf("  token = %s", token));

				int			attrNum;

				/* Now find the attribute in the TData corresponding to
				 * the name specified in the projection. */
				for (attrNum = 0; attrNum < attrCount; attrNum++)
				{
					AttrInfo *	attrInfoP = attrListP->Get(attrNum);

					if (!strcmp(token, attrInfoP->name))
					{
						DO_DEBUG(printf(" attrNum = %d\n", attrNum));
						projection.attrList[projAttrNum] = attrNum;
						break;
					}
				}
				DOASSERT(projection.attrList[projAttrNum] != illegalAttr,	
					"Illegal attribute name in attribute projection file");
				projAttrNum++;
			}
			DOASSERT(projAttrNum == projection.attrCount,
				"Incorrect number of attributes in projection file");


			_projList.AddProjection(projection);

		}

		fclose(file);
	}

	return result;
}

/*------------------------------------------------------------------------------
 * function: AttrToDouble
 * Convert an attribute value to a double.
 */
static double
AttrToDouble(AttrType type, char *valP)
{
	DO_DEBUG(printf("AttrToDouble()\n"));

	double		doubleVal;
	float		floatVal;
	int			intVal;
	double		result = 0.0;

	switch (type)
	{
	case IntAttr:
		intVal = *(int *) valP;
		DO_DEBUG(printf("        %d\n", intVal));
		result = (double) intVal;
		break;

	case FloatAttr:
		floatVal = *(float *) valP;
		DO_DEBUG(printf("        %f\n", floatVal));
		result = (double) floatVal;
		break;

	case DoubleAttr:
		doubleVal = *(double *) valP;
		DO_DEBUG(printf("        %f\n", doubleVal));
		result = (double) doubleVal;
		break;

	case StringAttr:
		DOASSERT(false, "Can't deal with string attribute");
		break;

	case DateAttr:
		DOASSERT(false, "Can't deal with date attribute");
		break;

	default:
		DOASSERT(false, "Illegal attribute type");
		break;
	}

	return result;
}

/*============================================================================*/
