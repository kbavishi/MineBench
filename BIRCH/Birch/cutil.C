/****************************************************************
File Name: cutil.C   
Author: Tian Zhang, CS Dept., Univ. of Wisconsin-Madison, 1995

               Copyright(c) 1995 by Tian Zhang

                   All Rights Reserved

Permission to use, copy and modify this software must be granted
by the author, provided that the above copyright notice appear in 
all relevant copies and that both that copyright notice and this 
permission notice appear in all relevant supporting documentations. 

Comments and additions may be sent the author at zhang@cs.wisc.edu.

******************************************************************/

#include "global.h"
#include "util.h"
#include "vector.h"
#include "rectangle.h"
#include "parameter.h"
#include "cfentry.h"
#include "status.h"
#include "cftree.h"
#include "cutil.h"

double distance(short dtype, const Entry &ent1, const Entry &ent2) 
{
switch (dtype) {
	case D0: return (ent1 || ent2);
	case D1: return (ent1 ^ ent2);
	case D2: return (ent1 | ent2);
	case D3: return (ent1 & ent2);
	case D4: return (ent1 && ent2);
	default: print_error("distance","Invalid distance type");
	}
}

double fitness(short ftype, const Entry &ent1, const Entry &ent2) 
{
switch (ftype) {
	case AVG_DIAMETER: return (ent1 & ent2);
	default: print_error("fitness","Invalid fitness type");
	}
}


void ClosestTwoOut(Entry *tmpentry, int head, int tail, 
		   short dtype, int &i, int &j) 
{
int i1,j1,imin,jmin;
double d, dmin;
if (tail-head<1) 
	print_error("ClosestTwoOut3","Less than 2 entries");
if (tail-head==1) 
	{i=head; j=tail; return;}
imin = head; 
jmin = head+1;
dmin = distance(dtype,tmpentry[imin],tmpentry[jmin]);
for (i1=head;i1<tail;i1++)
   for (j1=i1+1;j1<=tail;j1++) {
		d = distance(dtype,tmpentry[i1],tmpentry[j1]);
		if (d<dmin) { 
			imin = i1; 
			jmin = j1; 
			dmin = d;}
	}
i=imin; j=jmin;
}

void FarthestTwoOut(Entry *tmpentry, int head, int tail, 
  		    short dtype, int &i, int &j) 
{
int i1,j1,imax,jmax;
double d, dmax;
if (tail-head<1) 
	print_error("FarthestTwoOut3","Less than 2 entries");
if (tail-head==1) 
	{i=head; j=tail; return;}
imax = head; 
jmax = head+1;
dmax = distance(dtype,tmpentry[imax],tmpentry[jmax]);
for (i1=head;i1<tail;i1++)
   for (j1=i1+1;j1<=tail;j1++) {
		d = distance(dtype,tmpentry[i1],tmpentry[j1]);
		if (d>dmax) { 
			imax = i1; 
			jmax = j1; 
			dmax = d;}
	}
i=imax; j=jmax;
}

double Quality(short qtype, const int NumCluster, const Entry *Clusters) {
switch (qtype) {
case OVERALL_AVG_DIAMETER: return Quality1(NumCluster,Clusters);
case OVERALL_AVG_RADIUS:   return Quality2(NumCluster,Clusters);
default: print_error("Quality","Invalid quality type");
}
}

double Quality1(const int NumCluster, const Entry *Clusters) {
int i;
double cnt_all=0.0, sum_all=0.0, tmp0,tmp1;

for (i=0; i<NumCluster; i++) {
	tmp0 = Clusters[i].n*(Clusters[i].n-1.0);
	tmp1 = 2*Clusters[i].n*Clusters[i].sxx-2*(Clusters[i].sx&&Clusters[i].sx);
	cnt_all += tmp0;
	sum_all += tmp1;

#ifdef DEBUG
//logfile << "cluster " << i << " with " << Clusters[i].n << " points\n";
if (Clusters[i].n==1) {//logfile << "quality: 0\n"} ;
else { //logfile << "quality: " << sqrt(tmp1/tmp0) << endl};
#endif DEBUG
}
return(sqrt(sum_all/cnt_all));
}

double Quality2(const int NumCluster, const Entry *Clusters) {
int i;
double cnt_all=0.0, sum_all=0.0, tmp0,tmp1,tmp2;

for (i=0; i<NumCluster; i++) {
	tmp1 = 0;
	for (short j=0; j<Clusters[i].sx.dim; j++) {
		tmp0 = Clusters[i].sx.value[j]/Clusters[i].n;
		tmp1 += tmp0*tmp0;
		}
	tmp2 = Clusters[i].sxx - Clusters[i].n*tmp1;
	cnt_all += Clusters[i].n;
	sum_all += tmp2;
#ifdef DEBUG
//logfile << "cluster " << i << " with " << Clusters[i].n << " points\n";
if (Clusters[i].n==1) {//logfile << "quality: 0\n"} ;
else {//logfile << "quality: " << sqrt(tmp2/Clusters[i].n) << endl};
#endif DEBUG
	}
return(sqrt(sum_all/cnt_all));
}


