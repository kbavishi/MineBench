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
  Implementation of Version class.
 */

/*
  $Id: Version.c 2396 2014-03-13 21:13:27Z wkliao $

  $Log: Version.c,v $
  Revision 1.20  1996/12/15 20:23:08  wenger
  Incremented revision; prints current architecture at startup.

  Revision 1.19  1996/12/13 18:20:58  wenger
  Added release.linux; incremented DEVise revision.

  Revision 1.18  1996/12/05 15:31:13  wenger
  Bumped up revision.

  Revision 1.17  1996/11/26 15:44:21  wenger
  Added features and fixed bugs in PostScript-related parts of the
  client/server library and the PSWindowRep class; page size can now be
  set in PSDisplay; did some cleanup of the DeviseDisplay and WindowRep
  methods (allowed elimination of many typecasts).

  Revision 1.16  1996/11/20 20:35:22  wenger
  Fixed bugs 062, 073, 074, and 075; added workaround for bug 063; make
  some Makefile improvements so compile works first time; fixed up files
  to correspond to new query catalog name.

  Revision 1.15  1996/11/18 23:11:32  wenger
  Added procedures to generated PostScript to reduce the size of the
  output and speed up PostScript processing; added 'small font' capability
  and trademark notice to PostScript output; improved text positioning in
  PostScript output (but still a ways to go); added a little debug code;
  fixed data/axis area bugs (left gaps); fixed misc. bugs in color handling.

  Revision 1.14  1996/10/23 14:57:13  wenger
  Renamed LandsendDateDiffComposite composite parser to
  MailorderDateDiffComposite as part of getting rid of the "Land's
  End" name in visible places; schema types changed from "LANDSEND..."
  to "MAILORDER...".

  Revision 1.13  1996/10/08 21:49:10  wenger
  ClassDir now checks for duplicate instance names; fixed bug 047
  (problem with FileIndex class); fixed various other bugs.

  Revision 1.12  1996/10/04 19:58:50  wenger
  Temporarily (?) removed threads from Devise (removed -lpthread from
  some Makefiles so Purify works); other minor cleanups.

  Revision 1.11  1996/09/27 21:09:38  wenger
  GDataBin class doesn't allocate space for connectors (no longer used
  anyhow); fixed some more memory leaks and made notes in the code about
  some others that I haven't fixed yet; fixed a few other minor problems
  in the code.

  Revision 1.10  1996/09/19 19:32:52  wenger
  Devise now complains about illegal command-line flags (fixes bug 042).

  Revision 1.9  1996/09/10 20:07:24  wenger
  High-level parts of new PostScript output code are in place (conditionaled
  out for now so that the old code is used until the new code is fully
  working); changed (c) (tm) in windows so images are not copyrighted
  by DEVise; minor bug fixes; added more debug code in the course of working
  on the PostScript stuff.

  Revision 1.8  1996/09/04 21:25:03  wenger
  'Size' in mapping now controls the size of Dali images; improved Dali
  interface (prevents Dali from getting 'bad window' errors, allows Devise
  to kill off the Dali server); added devise.dali script to automatically
  start Dali server along with Devise; fixed bug 037 (core dump if X is
  mapped to a constant); improved diagnostics for bad command-line arguments.

  Revision 1.7  1996/08/30 15:56:11  wenger
  Modified View and QueryProcessor code to work correctly with current
  dispatcher semantics (call back forever unless cancelled).

  Revision 1.6  1996/08/28 00:19:51  wenger
  Improved use of Dali to correctly free images (use of Dali is now fully
  functional with filenames in data).

  Revision 1.5  1996/08/14 21:22:56  wenger
  Minor dispatcher cleanups, etc.  Fixed release script to release
  statically-linked executables for HP and Sun.

  Revision 1.4  1996/08/05 19:48:59  wenger
  Fixed compile errors caused by some of Kevin's recent changes; changed
  the attrproj stuff to make a .a file instead of a .o; added some more
  TData file writing stuff; misc. cleanup.

  Revision 1.3  1996/07/29 21:45:08  wenger
  Fixed various compile errors and warnings.

  Revision 1.2  1996/07/23 20:13:08  wenger
  Preliminary version of code to save TData (schema(s) and data) to a file.

  Revision 1.1  1996/07/09 16:00:24  wenger
  Added master version number and compile date to C++ code (also displayed
  in the user interface); added -usage and -version command line arguments;
  updated usage message.

 */

#define _Version_c_

#include <stdio.h>

#include "Version.h"
#include "CompDate.h"
#include "machdep.h"


/*
 * Static global variables.
 */

// Master DEVise version number.
static const char *	version = "1.2.4";

// Master DEVise copyright dates.
static const char *	copyright = "Copyright (c) 1992-1996";

// Trademark logo for each DEVise window.
static const char *	winLogo = "Visualization by DEVise (tm) 1996";


#if !defined(lint) && defined(RCSID)
static char		rcsid[] = "$RCSfile: Version.c,v $ $Revision: 2396 $ $State: Exp $";
#endif

static char *	srcFile = __FILE__;

/*------------------------------------------------------------------------------
 * function: Version::Get
 * Get the current DEVise version.
 */
const char *
Version::Get()
{
	return version;
}

/*------------------------------------------------------------------------------
 * function: Version::GetCopyright
 * Get the DEVise copyright message.
 */
const char *
Version::GetCopyright()
{
	return copyright;
}

/*------------------------------------------------------------------------------
 * function: Version::GetWinLogo
 * Get the DEVise window trademark logo message.
 */
const char *
Version::GetWinLogo()
{
	return winLogo;
}

/*------------------------------------------------------------------------------
 * function: Version::PrintInfo
 * Print version and copyright information.
 */
void
Version::PrintInfo()
{
/*  printf("DEVise Data Visualization Software\n");
  printf("%s\n", copyright);
  printf("By the DEVise Development Group\n");
  printf("All Rights Reserved.\n");
  printf("Version %s (%s)\n", version, ARCH_NAME);
  printf("Compile date: %s\n", CompDate::Get());
*/
}

/*============================================================================*/
