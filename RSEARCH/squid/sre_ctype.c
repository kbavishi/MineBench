/*****************************************************************
 * INFERNAL - inference of RNA secondary structure alignments
 * Copyright (C) 2002-2003 Washington University, Saint Louis 
 * 
 *     This source code is freely distributed under the terms of the
 *     GNU General Public License. See the files COPYRIGHT and LICENSE
 *     for details.
 *****************************************************************/

/* sre_ctype.c
 * 
 * For portability. Some systems have functions tolower, toupper
 * as macros (for instance, MIPS M-2000 RISC/os!)
 * 
 * RCS $Id: sre_ctype.c 2280 2013-12-20 21:25:03Z wkliao $
 */

#include <ctype.h>
#include "squid.h"

int
sre_tolower(int c)
{
  if (isupper(c)) return tolower(c);
  else return c;
}

int
sre_toupper(int c)
{
  if (islower(c)) return toupper(c);
  else return c;
}

