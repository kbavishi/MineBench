/*****************************************************************
 * INFERNAL - inference of RNA secondary structure alignments
 * Copyright (C) 2002-2003 Washington University, Saint Louis 
 * 
 *     This source code is freely distributed under the terms of the
 *     GNU General Public License. See the files COPYRIGHT and LICENSE
 *     for details.
 *****************************************************************/

/* sqdconfig_main.c
 * SRE, Tue Mar  5 15:58:27 2002 [St. Louis]
 * 
 * Small C program designed to print out information on squid's 
 * compile-time configuration options - testsuite scripts can
 * call this to determine what optional stuff is compiled in. 
 * 
 * CVS $Id: sqdconfig_main.c 2280 2013-12-20 21:25:03Z wkliao $
 */


#include <stdio.h>
#include <stdlib.h>
#include "squid.h"

int main(void)
{
#ifdef HAS_64BIT_FILE_OFFSETS
  printf("%-30s true\n", "HAS_64BIT_FILE_OFFSETS");
#else
  printf("%-30s false\n", "HAS_64BIT_FILE_OFFSETS");
#endif
}
