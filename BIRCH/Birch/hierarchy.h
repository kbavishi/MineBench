/****************************************************************
File Name: hierarchy.h  
Author: Tian Zhang, CS Dept., Univ. of Wisconsin-Madison, 1995

               Copyright(c) 1995 by Tian Zhang

                   All Rights Reserved

Permission to use, copy and modify this software must be granted
by the author and provided that the above copyright notice appear 
in all relevant copies and that both that copyright notice and this 
permission notice appear in all relevant supporting documentations. 

Comments and additions may be sent the author at zhang@cs.wisc.edu.

******************************************************************/

#ifndef HIERARCHY_H
#define HIERARCHY_H

#define BY_THRESHOLD 0
#define BY_KCLUSTERS 1

class Hierarchy {
public:
int    size;
int    step;
int    *ii;
int    *jj;
Entry  *cf;
double *dd;

short   stopchain;
int 	chainptr;
int     *chain;

Hierarchy(int n, short d) { 
	size = n;
	step = -1;
	ii = new int[n];
	jj = new int[n];
	cf = new Entry[n]; for (int i=0;i<n;i++) cf[i].Init(d);
	dd = new double[n];

	// parallel arrays for merging ii and jj with cf and dd
	// positive 1..n: orginal entries
	// negative -1..-(n-1): merged entries 

	stopchain=FALSE;
	chainptr=-1; // empty
	chain = new int[n+1];
	}

~Hierarchy() {
	if (ii) delete [] ii;
	if (jj) delete [] jj;
	if (cf) delete [] cf;
	if (dd) delete [] dd;
	if (chain) delete [] chain;
	}

void MergeHierarchy(short gdtype, int nentry, Entry *entries);
short SplitHierarchy(short option, short ftype, double ft);

double NmaxFtMerged(short ftype);
double MinFtMerged(short ftype);

double TotalFtDEdge(short ftype, short d, Entry *entries);
double DistortionEdge(Entry *entries);

friend void Hierarchy0(int &n, const int K, Entry **entries, short GDtype, short Ftype, double Ft);
friend void Hierarchy1(int &n, const int K, Entry **entries, short GDtype, short Ftype, double Ft);

};

void Hierarchy0(int &n, const int K, Entry **entries, short GDtype, short Ftype, double Ft);
void Hierarchy1(int &n, const int K, Entry **entries, short GDtype, short Ftype,
 double Ft);

#endif HIERARCHY_H

