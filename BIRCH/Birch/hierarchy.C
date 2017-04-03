/****************************************************************
File Name: hierarchy.C  
Author: Tian Zhang, CS Dept., Univ. of Wisconsin-Madison, 1995

               Copyright(c) 1995 by Tian Zhang

                   All Rights Reserved

Permission to use, copy and modify this software must be granted
by the author and provided that the above copyright notice appear 
in all relevant copies and that both that copyright notice and this 
permission notice appear in all relevant supporting documentations. 

Comments and additions may be sent the author at zhang@cs.wisc.edu.

******************************************************************/

#include <assert.h>
#include "global.h"
#include "util.h"
#include "vector.h"
#include "rectangle.h"
#include "cfentry.h"
#include "cutil.h"
#include "parameter.h"
#include "hierarchy.h"

/* for MergeHierarchy use only */
static void Update_Distance(int n1, int n2, int CurI, int NextI, 
			    int n, int *checked, double *dist)
{
int 	i, j;
double  d1, d2;
for (i=0; i<CurI; i++) {
      if (checked[i]!=0) {
	if (i!=NextI) {
      		d1 = dist[i*n-i*(i+1)/2+CurI-i-1];
      		if (i<NextI) d2 = dist[i*n-i*(i+1)/2+NextI-i-1];
      		else d2 = dist[NextI*n-NextI*(NextI+1)/2+i-NextI-1];
      		dist[i*n-i*(i+1)/2+CurI-i-1] = (n1*d1+n2*d2)/(n1+n2);
      	}}}
for (j=CurI+1; j<n; j++) {
      if (checked[j]!=0) {
	if (j!=NextI) {
		d1 = dist[CurI*n-CurI*(CurI+1)/2+j-CurI-1];
		if (j<NextI) d2 = dist[j*n-j*(j+1)/2+NextI-j-1];
		else d2 = dist[NextI*n-NextI*(NextI+1)/2+j-NextI-1];
		dist[CurI*n-CurI*(CurI+1)/2+j-CurI-1] = (n1*d1+n2*d2)/(n1+n2);
      	}}}
}

/* for MergeHierarchy use only */
static int Pick_One_Unchecked(int n, int *checked)
{
int i,j = rand() % n;
for (i=0;i<n;i++) 
	if (checked[(i+j)%n]!=0) return (i+j)%n;
return -1;
}

/* for MergeHierarchy use only */
static int Nearest_Neighbor(int CurI, int n, int *checked, double *dist)
{
int    i, imin=0;
double d, dmin = HUGE;
for (i=0;i<CurI;i++) {
    if (checked[i]!=0){
	d = dist[i*n-i*(i+1)/2+CurI-i-1];
	if (d<dmin) { dmin=d; imin=i; }
	}}
for (i=CurI+1;i<n;i++) {
    if (checked[i]!=0){
	d = dist[CurI*n-CurI*(CurI+1)/2+i-CurI-1];
	if (d<dmin) { dmin=d; imin=i; }
	}}
if (dmin<HUGE) return imin;
else return -1;
}

/* for SplitHierarchy use only */
static int Farthest_Merge(int chainptr, int *chain, double *dd) 
{
if (chainptr<=0) return chainptr;

double d, dmax = 0;
int    i, imax = -1;
for (i=0; i<=chainptr; i++) {
	if (chain[i]<0) {
		d = dd[-chain[i]-1];
		if (d>dmax) {imax = i; dmax = d;}
		}}
return imax;
}

/* for SplitHierarchy use only */
static int Largest_Merge(int chainptr, int *chain, Entry *cf, short ftype, double ft)
{
for (int i=0; i<=chainptr; i++) {
	if (chain[i]<0) {
		if (cf[-chain[i]-1].Fitness(ftype)>ft)
			return i;
			}}
return -1;
}

/*********************************************************************
This is an O(n^2) algorithm, but works only for D2 and D4 due to the 
"REDUCIBILITY PROPERTY" that is given as below:
if d(i,j) < p, d(i,k) > p, d(j,k) > p then d(i+j,k) > p.
Algorithm:
Part 1: MergeHierarchy:
	form the complete hierarchy in O(n^2) time
	step1: Pick any cluster i[1].
	step2: Determine the chain i[2]=NN(i[1]), i[3]=NN(i[2]) ... 
	       until i[k]=NN(i[k-1]) and i[k-1]=NN(i[k]), where NN 
	       means nearest neighbor.
	step3: Merge clusters i[k-1] and i[k].
	step4: If more than 1 clusters left, goto step using i[k-2] 
	       as start if (k-2>0), otherwise choose arbitrary i[1].
Part 2: SplitHierarchy:
	cut from the complete hierarchy in O(n^2) 
		by number of clusters K, 
		by the threshold Ft, 
		by tree height H,
		...
*********************************************************************/

void Hierarchy::MergeHierarchy(short gdtype,    // distance type
			       int nentry,      // number of entries
                               Entry *entries)  // array storing entries
{
if (gdtype!=D2 && gdtype!=D4) 
	print_error("MergeHierarchy","only support distance D2 and D4");

int 	i,j,n1,n2;

int	CurI, PrevI, NextI;
int 	uncheckcnt = nentry;
int 	*checked = new int[nentry];
for (i=0;i<nentry;i++) checked[i]=i+1;
	// 0: invalid after being merged to other entries
	// positive 1..nentry+1 :     original entries
	// negative -1..-(nentry-1) : merged entries

// get initial distances
double *dist=new double[nentry*(nentry-1)/2];
for (i=0; i<nentry-1; i++)
  for (j=i+1; j<nentry; j++) 
	dist[i*nentry-i*(i+1)/2+j-i-1] = 
		distance(gdtype,entries[i],entries[j]);
	
CurI = rand() % nentry;			// step1 
chain[++chainptr]=CurI;

while (uncheckcnt>1) {

   if (chainptr==-1) {			// step4
	chainptr++; 
 	chain[chainptr]=Pick_One_Unchecked(nentry,checked);
 	}
   if (chainptr>0) 
	PrevI=chain[chainptr-1];
   else PrevI=-1;
   stopchain=FALSE;

   while (stopchain==FALSE) {		// step2	
	CurI=chain[chainptr];
	NextI = Nearest_Neighbor(CurI,nentry,checked,dist);
	// it is impossible NextI be -1 because uncheckcnt>1
	if (NextI==PrevI)
	  stopchain = TRUE;
	else {
	  chain[++chainptr]=NextI;
	  PrevI = CurI;
	  }
	} // end of while for step 2

   step++;
   ii[step] = checked[CurI];			// step3
   jj[step] = checked[NextI];
   if (CurI<NextI) 
	dd[step] = dist[CurI*nentry-CurI*(CurI+1)/2+NextI-CurI-1];
   else 
	dd[step] = dist[NextI*nentry-NextI*(NextI+1)/2+CurI-NextI-1];
   if (checked[CurI]>0) {
	n1 = entries[CurI].n;
	if (checked[NextI]>0) {
		n2 = entries[NextI].n;
   		cf[step].Add(entries[CurI],entries[NextI]);
		}
	else  {
		n2 = cf[-checked[NextI]-1].n;
		cf[step].Add(entries[CurI],cf[-checked[NextI]-1]);
		}
	}
   else {
	n1 = cf[-checked[CurI]-1].n;
	if (checked[NextI]>0) {
		n2 = entries[NextI].n;
		cf[step].Add(cf[-checked[CurI]-1],entries[NextI]);
		}
	else {
		n2 = cf[-checked[NextI]-1].n;
		cf[step].Add(cf[-checked[CurI]-1],cf[-checked[NextI]-1]);
		}
	}
   Update_Distance(n1,n2,CurI,NextI,nentry,checked,dist);
   uncheckcnt--;
   checked[CurI] = -(step+1);	    
   checked[NextI] = 0;
   chainptr--; chainptr--;
   } //end of while (uncheckcnt>1)

if (checked) delete [] checked;
if (dist) delete [] dist;

// prepare for SplitHierarchy
stopchain = FALSE;
chainptr = 0;
chain[chainptr] = -(step+1);
}

short Hierarchy::SplitHierarchy(short option,short ftype,double ft)
{
int i, j;
switch (option) {
case BY_THRESHOLD:
	if (stopchain==TRUE) return FALSE;
	i = Largest_Merge(chainptr,chain,cf,ftype,ft);
	if (i!=-1) {j = -chain[i]-1;
		    chain[i] = ii[j];
		    chain[++chainptr]=jj[j];
		    return TRUE;
		    }
	else {stopchain = TRUE; return FALSE;}
	break;	
case BY_KCLUSTERS:
	if (chainptr==size) return FALSE;
	i = Farthest_Merge(chainptr,chain,dd);
	if (i!=-1) {j = -chain[i]-1;
		    chain[i]=ii[j];
		    chain[++chainptr]=jj[j];
		    return TRUE;
		    }
        else return FALSE;
	break;
	}
}

double Hierarchy::MinFtMerged(short ftype)
{
int MinI;
double tmpFt, MinFt=HUGE_DOUBLE;
for (int i=0; i<=chainptr; i++) {
	if (chain[i]<0) {
		tmpFt=cf[-chain[i]-1].Fitness(ftype);
		if (tmpFt<MinFt) {MinFt=tmpFt; MinI=i;}
		}
	}

// prepare for physically merging the MinI cluster
stopchain=FALSE;
chainptr=0;
chain[0]=chain[MinI];
return MinFt;
}

double Hierarchy::DistortionEdge(Entry *entries)
{
double TotalDistort=0;
for (int i=0; i<=chainptr; i++) {
	if (chain[i]<0)
		TotalDistort+=cf[-chain[i]-1].N()*cf[-chain[i]-1].Radius();
	else TotalDistort+=entries[chain[i]-1].N()*entries[chain[i]-1].Radius();
	}
return TotalDistort;
}

double Hierarchy::NmaxFtMerged(short ftype)
{
double NmaxFt;
int    maxn;
maxn = 0; NmaxFt=0;
for (int i=0; i<=chainptr; i++) {
	if (chain[i]<0 && cf[-chain[i]-1].N()>maxn) {
		maxn = cf[-chain[i]-1].N();
		NmaxFt = cf[-chain[i]-1].Fitness(ftype);
		}
	}
return NmaxFt;
}

double Hierarchy::TotalFtDEdge(short ftype, short d, Entry *entries)
{
double TotalFtD=0;
for (int i=0; i<=chainptr; i++) {
	if (chain[i]<0) 
		TotalFtD+=pow(cf[-chain[i]-1].Fitness(ftype),1.0*d);
	else TotalFtD+=pow(entries[chain[i]-1].Fitness(ftype),1.0*d);
	}
return TotalFtD;
}

/***********************************************************
This is an O(n^3) algorithm, but works fine for all 5
(D0,D1,D2,D3,D4) distance definitions.  
It is used to make sure Hierarchy1 and Hierarchy0 can 
produce the same results for debugging purpose.
************************************************************/

void Hierarchy0(int &n, 	 // final number of clusters
		const int K,     // final number of clusters
		Entry **entries,
		short GDtype,
		short Ftype,
		double Ft)
{

if (n<=1) return;

int 	i, j, imin, jmin, done;
short  	*checked = new short[n];
memset(checked,0,n*sizeof(short));
	// 0: unchecked;
	// -1: exceeds the given threshold if merged with nearest neighbor;
	// -2: nonexistant after merging.

double 	*dist = new double[n*(n-1)/2];
double 	d, dmin;
Entry 	tmpent;
tmpent.Init((*entries)[0].sx.dim);

dmin = HUGE;	// compute all initial distances and closest pair
for (i=0; i<n-1; i++)
  for (j=i+1; j<n; j++) {
	d = distance(GDtype,(*entries)[i],(*entries)[j]);
	dist[i*n-i*(i+1)/2+j-i-1] = d;
	if (d<dmin) {
		dmin = d;
		imin = i;
		jmin = j;
		}
	}

if (K==0) {// ****** case 1 ****** cluster by threshold ft
done = FALSE;
while (done==FALSE) {
	tmpent.Add((*entries)[imin],(*entries)[jmin]);
	if (tmpent.Fitness(Ftype) < Ft) { 
	// within the threshold
		(*entries)[imin] += (*entries)[jmin];
		checked[jmin] = -2;
		for (i=0; i<imin; i++) {
		   if (checked[i]==0) {
		   dist[i*n-i*(i+1)/2+imin-i-1] = 
			distance(GDtype,(*entries)[i],(*entries)[imin]);
		   }}
		for (j=imin+1; j<n; j++) {
		   if (checked[j]==0) {
		   dist[imin*n-imin*(imin+1)/2+j-imin-1] =
			distance(GDtype,(*entries)[imin],(*entries)[j]);
		   }}
		}
	else { 
	// exceeds the threshold
		checked[imin] = -1;
		checked[jmin] = -1;
		}

	done = TRUE;
	dmin = HUGE;
	for (i=0; i<n-1; i++) {
	  if (checked[i]==0) {
	  for (j=i+1; j<n; j++) {
		if (checked[j]==0) {
			d = dist[i*n-i*(i+1)/2+j-i-1];
			if (d<dmin) {
				done = FALSE;
				dmin = d;
				imin = i;
				jmin = j;
	}}}}} 
	} // end of while
} // end of if
else { // ***** case 2 ***** cluster by number k	
done = n;
while (done > K) {
	(*entries)[imin] += (*entries)[jmin];
	checked[jmin] = -2;
	done--;
	for (i=0; i<imin; i++) {
	   if (checked[i]==0) {
	   dist[i*n-i*(i+1)/2+imin-i-1] = 
		distance(GDtype,(*entries)[i],(*entries)[imin]);
	   }}
	for (j=imin+1; j<n; j++) {
	   if (checked[j]==0) {
	   dist[imin*n-imin*(imin+1)/2+j-imin-1] =
		distance(GDtype,(*entries)[imin],(*entries)[j]);
	   }}

	dmin = HUGE;
	for (i=0; i<n-1; i++) {
	  if (checked[i]==0) {
	  for (j=i+1; j<n; j++) {
		if (checked[j]==0) {
		d = dist[i*n-i*(i+1)/2+j-i-1];	
		if (d<dmin) {
			dmin = d;
			imin = i;
			jmin = j;
	}}}}}
	} // end of while
} // end of else

j = 0;
for (i=0; i<n; i++)
   if (checked[i]!=-2) {
	(*entries)[j]=(*entries)[i];
	j++;
	}
n=j;

delete [] checked;
delete [] dist;
}

/*********************************************************************
This is an O(n^2) algorithm, but works only for D2 and D4 due to the 
"REDUCIBILITY PROPERTY" that is given as below:
if d(i,j) < p, d(i,k) > p, d(j,k) > p then d(i+j,k) > p.
Algorithm:
Part 1: Form the complete hierarchy in O(n^2) time
	step1: Pick any cluster i[1].
	step2: Determine the chain i[2]=NN(i[1]), i[3]=NN(i[2]) ... 
	       until i[k]=NN(i[k-1]) and i[k-1]=NN(i[k]), where NN 
	       means nearest neighbor.
	step3: Merge clusters i[k-1] and i[k].
	step4: If more than 1 clusters left, goto step using i[k-2] 
	       as start if (k-2>0), otherwise choose arbitrary i[1].
Part 2: Split from the complete hierarchy in O(n^2) 
		by number of clusters K, 
		by the threshold Ft, 
		by tree height H,
		...
*********************************************************************/

void Hierarchy1(int &n, 	// final number of clusters
		const int K,    // final number of clusters
		Entry **entries,// array storing clusters
		short GDtype,
		short Ftype,
		double Ft)
{
if (GDtype!=D2 && GDtype!=D4) 
	print_error("Hierarchy1","only support distance D2 and D4");

if (n<=1) return;
if (n==K) return;

// ****************** Part 1 **********************

Hierarchy *h = new Hierarchy(n-1,(*entries)[0].sx.dim);
h->MergeHierarchy(GDtype,n,*entries);

// ******************* Part 2 *********************

if (K==0) { 	// by the threshold
	while (h->SplitHierarchy(BY_THRESHOLD,Ftype,Ft)==TRUE);
	}
else 	{		// by the number of clusters
	while (h->chainptr+1<K && 
	       h->SplitHierarchy(BY_KCLUSTERS,Ftype,Ft)==TRUE);
	}

// ******************* Part 3 *********************

int j;
Entry *tmpentries = new Entry[h->chainptr+1];
for (j=0;j<=h->chainptr;j++) 
	tmpentries[j].Init((*entries)[0].sx.dim);

for (j=0;j<=h->chainptr;j++) {
	if (h->chain[j]<0) 
		tmpentries[j] = h->cf[-h->chain[j]-1];
	else 	tmpentries[j] = (*entries)[h->chain[j]-1];
	}

delete [] *entries;
*entries = tmpentries;
n=h->chainptr+1;
delete h;
}

