/****************************************************************
File Name: phase4.C   
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
#include "timeutil.h"
#include "vector.h"
#include "rectangle.h"
#include "cfentry.h"
#include "cutil.h"
#include "parameter.h"
#include "status.h"
#include "phase4.h"

extern Para* Paras;

static int ClosestNorm(double tmpnorm,double *norms,int start,int end)
{
if (end-start==1) {
	if (tmpnorm>norms[end]) return end;
	if (tmpnorm<norms[start]) return start;
	if (tmpnorm-norms[start]<norms[end]-tmpnorm) 
		return start;
	else 	return end;
	}
int median=(start+end)/2;
if (tmpnorm>norms[median]) 
	return ClosestNorm(tmpnorm,norms,median,end);
else 	return ClosestNorm(tmpnorm,norms,start,median);
}


static int MinLargerThan(double tmpnorm,double *norms,int start,int end)
{
int median;
if (start>=end) return start;
median=(start+end)/2;
if (tmpnorm>norms[median])
	return MinLargerThan(tmpnorm,norms,median+1,end);
else 	return MinLargerThan(tmpnorm,norms,start,median);
}


static int MaxSmallerThan(double tmpnorm,double *norms,int start,int end)
{
int median;
if (start>=end) return start;
median=(start+end+1)/2;
if (tmpnorm<norms[median])
	return MaxSmallerThan(tmpnorm,norms,start,median-1);
else 	return MaxSmallerThan(tmpnorm,norms,median,end);
}


static int ClosestCenter(int &i, double &idist, const Vector &v,
			 const Vector *centers, int imin, int imax,
			 const double *matrix, int n)
{
int k,count=0;
double d;
k=imin;
while (k<=imax) {
  if (k<i && matrix[k*n-k*(k+1)/2+i-k-1]<=4*idist) {
	d=v||centers[k]; count++;
	if (d<idist) {idist=d;i=k;}
	}
  else if (k>i && matrix[i*n-i*(i+1)/2+k-i-1]<=4*idist) {
	d=v||centers[k]; count++;
	if (d<idist) {idist=d;i=k;}
	}
  k++;
  }
return count;
}

static void ClosestCenter(int &imin, double &dmin, const Vector &v, 
			  const Vector *centers, int num) 
{
double d;
imin=0; dmin = v||centers[0];
for (int i=1; i<num; i++) {
	d = v||centers[i];
	if (d<dmin) {imin=i; dmin=d;}
	}
}


static void Swap(Vector *centers, double *norms, int i, int j)
{
Vector tmpcenter;
tmpcenter.Init(centers[0].dim);
double tmpnorm;

tmpcenter=centers[i];
tmpnorm=norms[i];
centers[i]=centers[j];
norms[i]=norms[j];
centers[j]=tmpcenter;
norms[j]=tmpnorm;
}

/** Relevant to QuickSort: which is not critical because
it is only run once for each scan of the whole dataset. */

static void Swap(Vector *centers, double *norms, double *radii, int i, int j)
{
Vector tmpcenter;
tmpcenter.Init(centers[0].dim);
double tmpnorm;
double tmpradius;

tmpcenter=centers[i];
tmpnorm=norms[i];
tmpradius=radii[i];
centers[i]=centers[j];
norms[i]=norms[j];
radii[i]=radii[j];
centers[j]=tmpcenter;
norms[j]=tmpnorm;
radii[j]=tmpradius;
}

static int Split(Vector *centers, double *norms, int start, int end)
{
int median=(start+end)/2;
Swap(centers,norms,start,median);

int i=start+1;
int j=end;
while (i<=j) {
    if (norms[i]<=norms[start]) i++;
    else {Swap(centers,norms,i,j); j--;}
    }
Swap(centers,norms,start,j);
return j;
}

static int Split(Vector *centers, double *norms, double *radii, int start, int end)
{
int median=(start+end)/2;
Swap(centers,norms,radii,start,median);

int i=start+1;
int j=end;
while (i<=j) {
    if (norms[i]<=norms[start]) i++;
    else {Swap(centers,norms,radii,i,j); j--;}
    }
Swap(centers,norms,radii,start,j);
return j;
}

static void QuickSort(Vector *centers, double *norms, int start, int end)
{
int SplitPoint;
if (start<end) {SplitPoint=Split(centers,norms,start,end);
	  QuickSort(centers,norms,start,SplitPoint-1);
	  QuickSort(centers,norms,SplitPoint+1,end);
	 }
}

static void QuickSort(Vector *centers, double *norms, double *radii, int start, int end)
{
int SplitPoint;
if (start<end) {SplitPoint=Split(centers,norms,radii,start,end);
	  QuickSort(centers,norms,radii,start,SplitPoint-1);
	  QuickSort(centers,norms,radii,SplitPoint+1,end);
	 }
}

static int MaxRefinePass(Stat **Stats)
{
int maxpass=0;
for (int i=0; i<Paras->ntrees; i++) 
	if (Stats[i]->MaxRPass>maxpass) maxpass=Stats[i]->MaxRPass;
return maxpass;
}

static int RedistributeA(Stat* Stats,Vector* centers,double *radii,const Vector& tmpv) 
{
     int i;
     double idist;

     ClosestCenter(i,idist,tmpv,centers,Stats->CurrEntryCnt);

     if (Stats->NoiseFlag==1 && idist>4*radii[i]) {
		Stats->NoiseCnt++;
		return -1;
		}
     else {
		Stats->Entries[i].n += 1;
		Stats->Entries[i].sx += tmpv;
		Stats->Entries[i].sxx += (tmpv&&tmpv);
		return i;
		}
}

static void UpdateMatrix(int n, Vector *centers, double *matrix)
{
int i,j;
for (i=0;i<n-1;i++)
   for (j=i+1;j<n;j++)
	   matrix[i*n-i*(i+1)/2+j-i-1]=centers[i]||centers[j];
}

static int RedistributeB(Stat* Stats,Vector* centers,double *norms,double *radii,double *matrix,const Vector& tmpv)
{
int    imin,imax,i,k,n,start,end,median;
double d,tmpnorm,idist,tmpdist;

  n=Stats->CurrEntryCnt;
  tmpnorm=sqrt(tmpv&&tmpv);

  // i=ClosestNorm(tmpnorm,norms,0,n-1);
  // for efficiency, replace recursion by iteration
  start=0;
  end=n-1;
  while(start<end) {
    if (end-start==1) {
      if (tmpnorm>norms[end]) i=start=end;
      else if (tmpnorm<norms[start]) i=end=start;
           else if (tmpnorm-norms[start]<norms[end]-tmpnorm) 
			i=end=start;
                else i=start=end;
      }
    else {
      median=(start+end)/2;
      if (tmpnorm>norms[median]) start=median;
      else end=median;
      }
    }

    idist=tmpv||centers[i];

    // imin=MinLargerThan(tmpnorm-sqrt(idist),norms,0,n-1);
    // imax=MaxSmallerThan(tmpnorm+sqrt(idist),norms,0,n-1);
    // for efficiency, replace recursion by iteration

    tmpdist=tmpnorm-sqrt(idist);
    start=0;
    end=n-1;
    while (start<end) {
      median=(start+end)/2;
      if (tmpdist>norms[median]) start=median+1;
      else end=median;
      }
    imin=start;

    tmpdist=tmpnorm+sqrt(idist);
    start=0;
    end=n-1;
    while(start<end) {
      median=(start+end+1)/2;
      if (tmpdist<norms[median]) end=median-1;
      else start=median;
      }
    imax=start;

    // ClosestCenter(i,idist,tmpv,centers,imin,imax,matrix,n);
    // for efficiency, replace procedure by inline
    k=imin;
    while (k<=imax) {
      if (k<i && matrix[k*n-k*(k+1)/2+i-k-1]<=4*idist) {
         d=tmpv||centers[k];
	 if (d<idist) {idist=d;i=k;}
	 }
      else if (k>i && matrix[i*n-i*(i+1)/2+k-i-1]<=4*idist) {
         d=tmpv||centers[k];
	 if (d<idist) {idist=d;i=k;}
	 }
      k++;
      }

     if (Stats->NoiseFlag==1 && idist>4*radii[i]) {
		Stats->NoiseCnt++;
		return -1;
		}
     else {
		Stats->Entries[i].n += 1;
		Stats->Entries[i].sx += tmpv;
		Stats->Entries[i].sxx += (tmpv&&tmpv);
		return i;
		}
}
	
void BirchPhase4(Stat **Stats) 
{
ofstream tmpfile;
char     tmpname[MAX_NAME];

int i, j, k, maxk, clusteri;

// 1. allocate space
Vector **centers = new Vector*[Paras->ntrees];
double **norms = new double*[Paras->ntrees];
double **radii = new double*[Paras->ntrees];
double **matrix = new double*[Paras->ntrees];

#ifdef LABEL 
ofstream *labfiles=new ofstream[Paras->ntrees];
#endif LABEL 

#ifdef FILTER
ofstream **filterfiles=new ofstream*[Paras->ntrees];
#endif FILTER

#ifdef SUMMARY
int	 **n = new int*[Paras->ntrees];
Vector   **sx = new Vector*[Paras->ntrees];
Vector   **sxx = new Vector*[Paras->ntrees];
#endif SUMMARY

for (i=0; i<Paras->ntrees; i++) {

	centers[i]=new Vector[Stats[i]->CurrEntryCnt];
	for (j=0;j<Stats[i]->CurrEntryCnt;j++) 
		centers[i][j].Init(Stats[i]->Dimension);
	norms[i]=new double[Stats[i]->CurrEntryCnt];
	radii[i]=new double[Stats[i]->CurrEntryCnt];
	matrix[i]=new double[Stats[i]->CurrEntryCnt*(Stats[i]->CurrEntryCnt-1)/2];

#ifdef LABEL 
	sprintf(tmpname,"%s-label",Stats[i]->name);
	labfiles[i].open(tmpname); 
#endif LABEL

#ifdef FILTER
	filterfiles[i]=new ofstream[Stats[i]->CurrEntryCnt];
	for (j=0;j<Stats[i]->CurrEntryCnt;j++) {
		sprintf(tmpname,"%s-dat-%d",Stats[i]->name,j);
		filterfiles[i][j].open(tmpname);
		}
#endif FILTER

#ifdef SUMMARY
	n[i]=new int[Stats[i]->CurrEntryCnt];
	sx[i]=new Vector[Stats[i]->CurrEntryCnt];
	sxx[i]=new Vector[Stats[i]->CurrEntryCnt];
	for (j=0;j<Stats[i]->CurrEntryCnt;j++) {
		n[i][j]=0;
		sx[i][j].Init(Paras->attrcnt);
		sxx[i][j].Init(Paras->attrcnt);
		}
#endif SUMMARY

	}

// 2. prepare to read data
RecId recid1,recidi;
DevStatus dstat;

Paras->attrproj->FirstRecId(recid1);

Vector vector;
vector.Init(Paras->attrcnt);

VectorArray *vectors;
Paras->attrproj->CreateRecordList(vectors);

Vector *tmpvecs=new Vector[Paras->ntrees];
for (i=0;i<Paras->ntrees;i++)
	tmpvecs[i].Init(Stats[i]->Dimension);

for (i=0;i<Paras->ntrees;i++) {
	Stats[i]->Phase=4;
	Stats[i]->Passi=0;
	}

maxk=MaxRefinePass(Stats);

// 2.1 as long as there is one tree needed to refine
for (k=0; k<maxk; k++) {

   // 1. find initial centers, radii, norms, matrix
   for (i=0; i<Paras->ntrees; i++) 
	if (k<Stats[i]->MaxRPass) {
	     Stats[i]->Passi=k;
	     Stats[i]->NoiseCnt=0;
	     for (j=0; j<Stats[i]->CurrEntryCnt; j++) {
	        centers[i][j].Div(Stats[i]->Entries[j].sx,
				  Stats[i]->Entries[j].n);
	        radii[i][j]=Stats[i]->Entries[j].Radius();
	        Stats[i]->Entries[j]=0;
	        }
	     if (Stats[i]->RefineAlg==1) {
	        for (j=0; j<Stats[i]->CurrEntryCnt; j++)
	           norms[i][j]=sqrt(centers[i][j]&&centers[i][j]);
	        QuickSort(centers[i],norms[i],radii[i],0,Stats[i]->CurrEntryCnt-1);
	        UpdateMatrix(Stats[i]->CurrEntryCnt,centers[i],matrix[i]);
	        }
	     }

   // 2. scan data
   for (recidi=recid1; ; recidi++) {

#ifdef FILTER
	dstat=Paras->attrproj->ReadWholeRec(recidi,vector);
	if (dstat!=StatusOk) break;
#endif FILTER

#ifdef SUMMARY
	dstat=Paras->attrproj->ReadWholeRec(recidi,vector);
	if (dstat!=StatusOk) break;
#endif SUMMARY

	dstat=Paras->attrproj->ReadRec(recidi,*vectors);
	if (dstat!=StatusOk) break;

	for (i=0; i<Paras->ntrees; i++) {

	   if (k<Stats[i]->MaxRPass) {

		tmpvecs[i]=*(vectors->GetVector(i));
		if (Stats[i]->WMflag)
		  tmpvecs[i].Transform(Stats[i]->W,Stats[i]->M);

		switch (Stats[i]->RefineAlg) {
		  case 0: clusteri=RedistributeA(Stats[i],centers[i],radii[i],tmpvecs[i]);
	 	          break;
		  case 1: clusteri=RedistributeB(Stats[i],centers[i],norms[i],radii[i],matrix[i],tmpvecs[i]);
	 		  break;
		  default: print_error("BirchPhase4","Invalid refine method");
			  break;
		  }

#ifdef LABEL 
		labfiles[i]<<clusteri<<endl;
#endif LABEL 

#ifdef FILTER
		filterfiles[i][clusteri]<<vector<<endl;
#endif FILTER

#ifdef SUMMARY
		if (clusteri>=0) {		
		    	n[i][clusteri]++;
		    	sx[i][clusteri]+=vector;
			sxx[i][clusteri].AddSqr(vector);
			}
#endif SUMMARY
		}
	    }
	}
   }

delete vectors;
delete [] tmpvecs;

// 3 output results
for (i=0; i<Paras->ntrees; i++) {

	delete [] centers[i];
	delete [] norms[i];
	delete [] radii[i];
	delete [] matrix[i];


   if (Stats[i]->MaxRPass>0) {
       // quality and output
       sprintf(tmpname,"%s-refcluster",Stats[i]->name);
       tmpfile.open(tmpname);
       Paras->logfile<<Stats[i]->name<<":"
	       	     <<"Quality of Phase4 "
	             <<Quality(Stats[i]->Qtype,
			     Stats[i]->CurrEntryCnt,
			     Stats[i]->Entries)
		     <<endl;

     for (j=0;j<Stats[i]->CurrEntryCnt;j++)
	 tmpfile<<Stats[i]->Entries[j]<<endl;
     tmpfile.close();

#ifdef LABEL 
     labfiles[i].close();
#endif LABEL 

#ifdef FILTER
     for (j=0;j<Stats[i]->CurrEntryCnt;j++)
     	filterfiles[i][j].close();
     delete [] filterfiles[i];
#endif FILTER

#ifdef SUMMARY
     sprintf(tmpname,"%s-summary",Stats[i]->name);
     tmpfile.open(tmpname);
     for (j=0;j<Stats[i]->CurrEntryCnt;j++) 
		tmpfile<<n[i][j]<<" "
		       <<sx[i][j]
		       <<sxx[i][j]
		       <<endl;
     delete [] n[i];
     delete [] sx[i];
     delete [] sxx[i];
     tmpfile.close();
#endif SUMMARY

     }
   }

delete [] centers;
delete [] norms;
delete [] radii;
delete [] matrix;

#ifdef LABEL 
delete [] labfiles;
#endif LABEL

#ifdef FILTER
delete [] filterfiles;
#endif FILTER

#ifdef SUMMARY
delete [] n;
delete [] sx;
delete [] sxx;
#endif SUMMARY
}

