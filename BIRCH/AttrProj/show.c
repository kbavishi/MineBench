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
  Program to "show" a DEVise/TData file (a file containing data and the
  relevant schema(s).
 */

/*
  $Id: show.c 2396 2014-03-13 21:13:27Z wkliao $

  $Log: show.c,v $
  Revision 1.1  1996/06/17 20:01:09  wenger
  First version of 'show' program for dumping projections to stdout.

 */

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

#include "AttrProj.h"
#include "ApInit.h"

/*------------------------------------------------------------------------------
 * function: main
 * Main function for the show program.
 */
int
main(
	int		argc,
	char **	argv)

{													/* start main */
	int			attrCount = 0;
	const int	maxAttrs = 128;
	char *		attrList[maxAttrs];
	char *		dataFile = NULL;
	char *		projectionFile = NULL;
	int			result = 0;
	char *		schemaFile = NULL;

	/* Parse the command line arguments. */
	{
		extern char *optarg;
		extern int	opterr;
		extern int	optind;

		int			option;
		Boolean		printUsage = false;

		while ((option = getopt(argc, argv, "a:f:u")) != -1)
		{
			switch (option)
			{
			case 'a':
				if (attrCount >= maxAttrs)
				{
					fprintf(stderr,
						"Too many attributes specified; maxiumum is %d\n",
						maxAttrs);
					result = 2;
				}
				else
				{
					attrList[attrCount] = optarg;
					attrCount++;
				}
				break;

			case 'f':
				dataFile = optarg;
				break;

			case 'u':
				printUsage = true;
				break;

			default:
				printUsage = true;
				result = 2;
				break;
			}
		}

		if (!printUsage && ((dataFile == NULL) || (attrCount == 0)))
		{
			fprintf(stderr, "Must specify file and at least one attribute\n");
			printUsage = true;
			result = 2;
		}

		if (printUsage)
		{
			fprintf(stderr,
				"Usage: show -f <file> -a <attribute> -a <attribute> ... [-u]\n");
			return result;
		}
	}

	/* Save the specified attributes to a temporary projection file. */
	{
		projectionFile = tempnam("/tmp", NULL);
		FILE * stream = fopen(projectionFile, "w");
		int		count;

		fprintf(stream, "%d ", attrCount);
		for (count = 0; count < attrCount; count++)
		{
			fprintf(stream, "%s ", attrList[count]);
		}

		fclose(stream);
	}


	/* Initialize Devise stuff for command-line arguments. */
	Init::DoInit();

	AttrProj *		apP;
	RecId			firstId;
	RecId			lastId;
	RecId			recId;
	VectorArray *	vecArrayP;

	/* Create the AttrProj object, and then delete the temporary projection
	 * file, which we don't need anymore. */
	apP = new AttrProj(schemaFile, projectionFile, dataFile);
	unlink(projectionFile);
	delete projectionFile;

	/* Print out each projected record. */
	apP->CreateRecordList(vecArrayP);

	apP->FirstRecId(firstId);
	apP->LastRecId(lastId);

	for (recId = firstId; recId <= lastId; recId++)
	{
		apP->ReadRec(recId, *vecArrayP);
		int			vecCount = vecArrayP->GetVecCount();

		int		vecNum;
		for (vecNum = 0; vecNum < vecCount; vecNum++)
		{
			Vector *	vecP = vecArrayP->GetVector(vecNum);
			int			vecDim = vecP->dim;
			int			count;

/*
			for (count = 0; count < vecDim; count++)
			{
				printf("%f\t", vecP->value[count]);
			}
			printf("\n");
*/
		}
	}

	delete vecArrayP;

	return result;
}													/* end main */

/*============================================================================*/
