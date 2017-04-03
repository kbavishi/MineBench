/****************************************************************
File Name: samples.C  
Author: Tian Zhang, CS Dept., Univ. of Wisconsin-Madison, 1995

               Copyright(c) 1995 by Tian Zhang

                   All Rights Reserved

Permission to use, copy and modify this software must be granted
by the author and provided that the above copyright notice appear 
in all relevant copies and that both that copyright notice and this 
permission notice appear in all relevant supporting documentations. 

Comments and additions may be sent the author at zhang@cs.wisc.edu.

******************************************************************/

#include "global.h"
#include "util.h"
#include "vector.h"
#include "rectangle.h"
#include "cfentry.h"
#include "cutil.h"
#include "parameter.h"
#include "status.h"
#include "cftree.h"
#include "buffer.h"
#include "samples.h"


Sample1::Sample1(int s, Stat *Stats) {
size=s;
cnt=0;
ptr=0;
CFS=new Entry[s];
for (int i=0; i<s; i++) CFS[i].Init(Stats->Dimension);
PrevA=PrevB=CurrA=CurrB=0.0;
}

Sample1::~Sample1() {
if (CFS) delete [] CFS;
}

void Sample1::AvgRRegression(Stat *Stats) {
double sumn=0.0, sumnn=0.0, sumnr=0.0, sumr=0.0;
if (cnt==0) {CurrA=-1;CurrB=-1;return;}
for (int i=0; i<cnt; i++) {
	sumn += CFS[i].n*1.0;
	sumnn += CFS[i].n*1.0*CFS[i].n*1.0;
	sumr += CFS[i].Fitness(Stats->Ftype);
	sumnr += CFS[i].n*CFS[i].Fitness(Stats->Ftype);
	}
CurrA = (sumnr-sumn*sumr/cnt)/(sumnn-sumn*sumn/cnt);
CurrB = sumr/cnt-CurrA*sumn/cnt;
}

void Sample1::Take_Sample1(Stat *Stats)
{

Entry tmpent1, tmpent2;
tmpent1.Init(Stats->Dimension);
tmpent2.Init(Stats->Dimension);

PrevA=CurrA;
PrevB=CurrB;

Stats->NewRoot->CF(tmpent1);
Stats->SplitBuffer->CF(tmpent2);
CFS[ptr].Add(tmpent1,tmpent2);
ptr=(ptr+1)%size;
cnt++;
if (cnt>size) cnt=size;
AvgRRegression(Stats);
}

Sample2::Sample2(int s) {
size=s;
cnt=0;
ptr=0;
NS = new int[s];
FTS = new double[s];
PrevA=PrevB=CurrA=CurrB=0.0;
}

Sample2::~Sample2() {
if (NS) delete [] NS;
if (FTS) delete [] FTS;
}

void Sample2::FtDRegression(Stat *Stats) {
double 	fti, sumn=0.0, sumnn=0.0, sumnftd=0.0, sumftd=0.0;
int 	i,ni;
short 	flagA=TRUE, flagB=TRUE;

ni = NS[0];
fti = FTS[0];

for (i=1;i<cnt;i++) 
	if (ni!=NS[i]) {flagA=FALSE; break;}

for (i=1;i<cnt;i++)
	if (fti!=FTS[i]) {flagB=FALSE; break;}

// Ft2>Ft1 but N2=N1, so Ft's are too small
if (flagA==TRUE && flagB==FALSE) {
	CurrA = -1; CurrB = -1;  return;
    	}

for (i=0; i<cnt; i++) {
	sumn += NS[i]*1.0;
	sumnn += NS[i]*1.0*NS[i]*1.0;
	sumftd += pow(FTS[i],Stats->Dimension);
	sumnftd += NS[i]*pow(FTS[i],Stats->Dimension);
	}
CurrA = (sumnftd-sumn*sumftd/cnt)/(sumnn-sumn*sumn/cnt);
CurrB = sumftd/cnt-CurrA*sumn/cnt;
}

void Sample2::Take_Sample2(Stat *Stats) {
PrevA=CurrA;
PrevB=CurrB;
NS[ptr]=Stats->CurrDataCnt;
FTS[ptr]=Stats->CurFt;
ptr=(ptr+1)%size;
cnt++;
if (cnt>size) cnt=size;
FtDRegression(Stats);
}

Sample3::Sample3(int s) {
size=s;
cnt=0;
ptr=0;
logR=new double[s];
logNR=new double[s];
}

Sample3::~Sample3() {
delete [] logR;
delete [] logNR;
}

void Sample3::Take_Sample3(Stat *Stats)
{
logR[ptr] = log(sqrt(Stats->CurFt));
logNR[ptr] = log(1.0*Stats->CurrEntryCnt);
ptr=(ptr+1)%size;
cnt++;
if (cnt>size) cnt=size;
}

double Sample3::Regression(const double n)
{
double A, B, sumlogR=0,sumlogRlogR=0,sumlogRlogNR=0,sumlogNR=0;

for (int i=0; i<cnt; i++) {
	sumlogR+=logR[i];
	sumlogRlogR+=logR[i]*logR[i];
	sumlogNR+=logNR[i];
	sumlogRlogNR+=logR[i]*logNR[i];
	}
A=(sumlogRlogNR-sumlogR*sumlogNR/cnt)/(sumlogRlogR-sumlogR*sumlogR/cnt);
B=sumlogNR/cnt-A*sumlogR/cnt;
return exp((log(n)-B)/A)*exp((log(n)-B)/A);
}

