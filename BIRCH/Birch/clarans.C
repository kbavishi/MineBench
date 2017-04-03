/****************************************************************
File Name: clarans.C  
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
#include "clarans.h"

/* for CLARANS0 use only */
static double local_search0(int n, Vector *entries, int **group, int k, int neighbor,int passi)
{
int min, i, j, g, h, no_test;
double dist, mindist, total_dist, djh, dji, cih ;

int *oldgroup = new int[n];
assert(oldgroup!=NULL);
int *newgroup = new int[n];
assert(newgroup!=NULL);
int *tmpgroup;

short *selected = new short[n];
assert(selected!=NULL);
memset(selected,0,n*sizeof(short));

int *repre = new int[k];
for (i=0; i<k; i++)
{ j = n/k*i+passi;
  repre[i]=j;
  selected[j]=1;
  }
	
for (j=0, total_dist=0.0; j<n; j++) {
	for (i=0, mindist = HUGE; i<k; i++) {
		dist = entries[j]^entries[repre[i]];
		if (dist<mindist) {
			mindist = dist;
			oldgroup[j]=i;
			}
		}
	total_dist += mindist;
	}

for (; ;) {
for (no_test=0; no_test<neighbor;) {
i = rand() % k;
h = rand() % n;
if (selected[h]) continue;
memcpy(newgroup,oldgroup,n*sizeof(int));

for (j=0, cih=0; j<n; j++) {
	djh = entries[j]^entries[h];
	if (oldgroup[j]==i) {
		dji = entries[j]^entries[repre[i]];
		if (djh <= dji) cih += djh-dji;
		else {
		  for (g=0,mindist=HUGE;g<k;g++) {
		   if (g==i) continue;
		   dist = entries[j]^entries[repre[g]];
			if (dist<mindist) {
				mindist=dist;
				min = g;
				}
			}
		  if (mindist<djh) {
			newgroup[j]=min;
			cih += mindist-dji;
			}
		  else cih += djh-dji;
		  }
	}
	else {
		mindist = entries[j]^entries[repre[oldgroup[j]]];
		if (djh < mindist) {
			newgroup[j]=i;
			cih += djh - mindist;
			}
		}
	}
if (cih >= 0.0) no_test++;
else { // cih < 0 : improved
	selected[repre[i]]=0;
	selected[h]=1;
	repre[i]=h;
	tmpgroup = oldgroup;
	oldgroup = newgroup;
	newgroup = tmpgroup;
	total_dist += cih;
	break;
	}
}

if (no_test==neighbor) break;
}
delete [] selected;
delete [] repre;
delete [] newgroup;
*group = oldgroup;
return(total_dist);
}
	
/* for local_search1 use only */
static void entrycpy(Entry *ents1, Entry *ents2, int k)
{
for (int i=0;i<k;i++) 
	ents1[i] = ents2[i];
}

/* for CLARANS1 use only */
static double local_search1(int n, Entry *entries, Entry **clusters, int k, short dtype, short qtype, int neighbor, int passi)
{
int i, j, g, h, min, no_test;
double dist, mindist, total_dist, qua, min_qua, djh, dji, cih;

int *repre = new int[k];
assert(repre!=NULL);

Entry *oldclusters = new Entry[k];
assert(oldclusters!=NULL);
for (i=0;i<k;i++) 
	oldclusters[i].Init(entries[0].sx.dim);

Entry *newclusters = new Entry[k];
assert(newclusters!=NULL);
for (i=0;i<k;i++) 
	newclusters[i].Init(entries[0].sx.dim);

Entry *tmpclusters;

short *selected = new short[n];
assert(selected!=NULL);
memset(selected,0,n*sizeof(short));

int *oldgroup = new int[n];
assert(oldgroup!=NULL);
int *newgroup = new int[n];
assert(newgroup!=NULL);
int *tmpgroup;

// select initial medoids : not randomly
for (i=0;i<k;i++) {
	j = i*n/k+passi;
	repre[i] = j;
	oldclusters[i] = entries[j];
	oldgroup[j] = i;
	selected[j] = 1;
	}

// assign initial groups and clusters
for (i=0,total_dist=0;i<n;i++) {
   if (selected[i]==1) continue;
   for (j=0, mindist = HUGE;j<k;j++) {
	dist = distance(dtype,entries[i],entries[repre[j]]);
	if (dist < mindist) {
		mindist = dist;
		oldgroup[i]=j;
		}
	}
   oldclusters[oldgroup[i]] += entries[i];
   total_dist += mindist;
   }

min_qua = Quality(qtype,k,oldclusters);

// random search
for (; ; ) {
for (no_test = 0; no_test < neighbor;) {
	i = rand() % k;
	h = rand() % n;
	if (selected[h]) continue;
	entrycpy(newclusters, oldclusters, k);
	memcpy(newgroup, oldgroup, n * sizeof(int));

	// for multiple distance definitions to work fine.
	selected[repre[i]]=0;
	selected[h]=1;
	newgroup[h] = i;
	newclusters[i] += entries[h];
	newclusters[oldgroup[h]] -= entries[h];

	for (j=0; j<n; j++) {	// scan data set
	 if (selected[j]==1) continue;
	 djh = distance(dtype,entries[j],entries[h]);
	 if (oldgroup[j]==i) {
		dji = distance(dtype,entries[j],entries[repre[i]]);
		if (djh <= dji) cih += djh-dji;
		else {	
		   for (g=0,mindist=HUGE; g<k; g++) {
			if (g==i) continue;
		 	dist = distance(dtype,entries[j],entries[repre[g]]);
			if (dist < mindist) {
				mindist = dist;
				min = g;
				}
			}
		   if (mindist<djh) {
				newgroup[j]=min;
				cih += mindist-dji;
				newclusters[i] -= entries[j];
				newclusters[min] += entries[j];
				}
		   else cih += djh-dji;
		   }
     		}
	else { // (group[j]!=i)
		mindist = distance(dtype,entries[j],entries[repre[oldgroup[j]]]);
		// case1 djh >= djx : no operations
		// case2 djh < djx  : do operations
		if (djh < mindist) {
		   newgroup[j] = i;
		   cih += djh-mindist;
		   newclusters[oldgroup[j]] -= entries[j];
		   newclusters[i] += entries[j];
		   }
		}
	} // end for j

	qua = Quality(qtype, k, newclusters);
	if (qua >= min_qua) {
		selected[i]=1;
		selected[h]=0;
	 	no_test++;
		}
	else {
		min_qua=qua;
		total_dist += cih;
		repre[i]=h;
		tmpgroup = oldgroup;
		oldgroup = newgroup;
		newgroup = tmpgroup;
		tmpclusters = oldclusters;
		oldclusters = newclusters;
		newclusters = tmpclusters;
		break;
		}
      } // end for no_test

      if (no_test==neighbor) break;
   } // end of for (; ;)

delete [] repre;
delete [] selected;
delete [] oldgroup;
delete [] newgroup;
delete [] oldclusters;
*clusters = newclusters;
return(total_dist);
}
	
void Clarans0(int &n, const int K, Entry *entries)
{
int *group, *min_group=NULL;
int i;
int no_local = 2;
int no_neighbor = K*(n-K);
double total_dist, min_total_dist;

Vector *centroids = new Vector[n];
for (i=0;i<n;i++) 
	centroids[i].Init(entries[0].sx.dim);

if (PERCENT(no_neighbor)>LOW_BOUND && PERCENT(no_neighbor)<HIGH_BOUND) 
        no_neighbor = (int) floor(PERCENT(no_neighbor));
else if (PERCENT(no_neighbor)>=HIGH_BOUND) 
	no_neighbor = HIGH_BOUND;
     else if (no_neighbor>LOW_BOUND) 
        no_neighbor = LOW_BOUND;

for (i=0; i<n; i++) centroids[i].Div(entries[i].sx,entries[i].n);

for (i=0, min_total_dist=HUGE*K*n; i<no_local; i++)
{	total_dist = local_search0(n,centroids,&group,K,no_neighbor,i);
	if (total_dist<min_total_dist) {
		min_total_dist = total_dist;
		if (min_group!=NULL) delete [] min_group;
		min_group = group;
		}
}

Entry *clusters = new Entry[K];
for (i=0; i<K; i++) 
	clusters[i].Init(entries[0].sx.dim);

for (i=0; i<n; i++) 
	clusters[min_group[i]] += entries[i];
for (i=0; i<K; i++) 
	entries[i]=clusters[i];
n = K;

delete [] min_group;
delete [] clusters;
delete [] centroids;
}

void Clarans1(int &n, const int K, Entry *entries, short GDtype, short Qtype)
{
int i;
int no_local = 2;
int no_neighbor = K * (n - K);
double min_qua,qua,min_total_dist,total_dist;
Entry *clusters , *min_clusters=NULL;

if (PERCENT(no_neighbor)>LOW_BOUND && PERCENT(no_neighbor)<HIGH_BOUND) 
        no_neighbor = (int) floor(PERCENT(no_neighbor));
else if (PERCENT(no_neighbor)>=HIGH_BOUND) 
	no_neighbor = HIGH_BOUND;
     else if (no_neighbor>LOW_BOUND) 
        no_neighbor = LOW_BOUND;

min_qua = HUGE * n * K;

for (i=0, min_qua=HUGE*n*K, min_total_dist=HUGE*n*K;i<no_local;i++) {
  total_dist = local_search1(n,entries,&clusters,K,GDtype,Qtype,no_neighbor,i);
  qua = Quality(Qtype,K,clusters);
  if (qua < min_qua) {
	min_qua= qua;
	if (min_clusters!=NULL) delete [] min_clusters;
	min_clusters = clusters;
	}
  }

for (i=0; i<K; i++) 
	entries[i]=min_clusters[i];
delete [] min_clusters;
n = K;
}

