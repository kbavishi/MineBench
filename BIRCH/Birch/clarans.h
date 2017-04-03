/****************************************************************
File Name: clarans.h  
Author: Tian Zhang, CS Dept., Univ. of Wisconsin-Madison, 1995

               Copyright(c) 1995 by Tian Zhang

                   All Rights Reserved

Permission to use, copy and modify this software must be granted
by the author and provided that the above copyright notice appear 
in all relevant copies and that both that copyright notice and this 
permission notice appear in all relevant supporting documentations. 

Comments and additions may be sent the author at zhang@cs.wisc.edu.

******************************************************************/
#ifndef CLARANS_H
#define CLARANS_H

#ifndef PERCENT
#define PERCENT(x) (0.0125 * (x))
#endif

#ifndef LOW_BOUND
#define LOW_BOUND 250
#endif

#ifndef HIGH_BOUND
#define HIGH_BOUND 500
#endif

void Clarans0(int &n, const int K, Entry *entries);
void Clarans1(int &n, const int K, Entry *entries, short GDtype, short Qtype);

#endif CLARANS_H
