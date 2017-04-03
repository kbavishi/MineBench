/****************************************************************
File Name: phase3.h  
Author: Tian Zhang, CS Dept., Univ. of Wisconsin-Madison, 1995

               Copyright(c) 1995 by Tian Zhang

                   All Rights Reserved

Permission to use, copy and modify this software must be granted
by the author, provided that the above copyright notice appear in 
all relevant copies and that both that copyright notice and this 
permission notice appear in all relevant supporting documentations. 

Comments and additions may be sent the author at zhang@cs.wisc.edu.

******************************************************************/

#ifndef PHASE3_H
#define PHASE3_H

#ifndef HIERARCHY0
#define HIERARCHY0 0
#endif

#ifndef HIERARCHY1 
#define HIERARCHY1 1
#endif

#ifndef CLARANS0
#define CLARANS0  2
#endif

#ifndef CLARANS1
#define CLARANS1  3
#endif

#ifndef LLOYD
#define LLOYD     4
#endif

#ifndef KMEANS 
#define KMEANS 	  5
#endif

void BirchPhase3(Stat **Stats); 

#endif PHASE3_H
