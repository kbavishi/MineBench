/****************************************************************
File Name: cutil.h  
Author: Tian Zhang, CS Dept., Univ. of Wisconsin-Madison, 1995

               Copyright(c) 1995 by Tian Zhang

                   All Rights Reserved

Permission to use, copy and modify this software must be granted
by the author and provided that the above copyright notice appear 
in all relevant copies and that both that copyright notice and this 
permission notice appear in all relevant supporting documentations. 

Comments and additions may be sent the author at zhang@cs.wisc.edu.

******************************************************************/

#ifndef CUTIL_H
#define CUTIL_H

double distance(short dtype, const Entry &ent1, const Entry &ent2);
double fitness(short ftype, const Entry &ent1, const Entry &ent2);
double Quality(short qtype, const int NumCluster, const Entry *entries);
double Quality1(const int NumCluster, const Entry *entries);
double Quality2(const int NumCluster, const Entry *entries);

Entry CFRegression(int m, Entry *CFS, int n);

void ClosestTwoOut(Entry *tmpentry, int head, int tail, short dtype, int &i, int &j);
void FarthestTwoOut(Entry *tmpentry, int head, int tail, short dtype, int &i, int &j);

#endif CUTIL_H
