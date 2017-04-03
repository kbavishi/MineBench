/****************************************************************
File Name: component.C   
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
#include "contree.h"
#include "components.h"

Unit1::Unit1() {
	unit=new int[UNIT_SIZE];
	next = NULL;
	}

Unit1::~Unit1() { 
	delete [] unit;
	}

Component::Component() { 
	orig = head = tail = new Unit1();
	size = start = end = 0;
	}

Component::~Component() {
	Unit1 *ptr;
	while (head!=tail) { 
		ptr=head; 
		head=head->next; 
		delete ptr;
		}
	delete tail;
	}

int Component::Size() const {return size;}

void Component::AddVertex(int i) {
	Unit1 *ptr;
	tail->unit[end]=i; end++; 
	size++;
	if (end==UNIT_SIZE) { 
		end=0;
		ptr=new Unit1();
		tail->next=ptr;
		tail=tail->next;
		}
	}

int Component::CurVertex() {
	int i;
	if (size==0) return -1;
	if (head==tail && start==end) return -1;
	i=head->unit[start]; start++; 
	if (start==UNIT_SIZE) { 
		start=0;
		head=head->next;
		}
	return i;
	}

void Component::ResetVertex() {
	head=orig;
	start=0;
	}

int Component::TupleCnt(Entry *entry)
{
int i,j,count=0;
ResetVertex();
for (i=0;i<size;i++)  {
	j=CurVertex();
	count+=entry[j].n;
	}
return count;
}

void Component::EntryChild(Stat* Stats, Entry *entry,ConNode **child, 
			   Entry &newentry,ConNode* &newchild)
{
int 	i,j,k,v,childsize;

if (child[0]==NULL) { // leaf

	childsize=size;
	newchild=new ConNode(childsize,Stats);

	ResetVertex();
	for (i=0;i<size;i++) {
		v=CurVertex();
		newentry+=entry[v];
		newchild->entry[i]=entry[v];
		newchild->child[i]=NULL;
		}
	}
else { // nonleaf

	childsize=0;
	ResetVertex();
	for (i=0;i<size;i++) {
		v=CurVertex();
		childsize+=child[v]->actsize;
		}
	newchild=new ConNode(childsize,Stats);

	k=0; 
	ResetVertex();
	for (i=0;i<size;i++) {
		v=CurVertex();
		newentry+=entry[v];
		for (j=0;j<child[v]->actsize;j++) {
			newchild->entry[k]=child[v]->entry[j];
			newchild->child[k]=child[v]->child[j];
			k++;
			}
		delete child[v];
		}
	}
}

ostream& operator<<(ostream &fo, Component *Compo)
{
fo << "    Size: " << Compo->size << endl;
Unit1 *ptr=Compo->orig;
for (int i=0; i<Compo->size; i++) {
	fo<<ptr->unit[i%UNIT_SIZE]<<"\t";
	if ((i+1)%UNIT_SIZE==0) ptr=ptr->next; 
	}
return fo;
}

ofstream& operator<<(ofstream &fo, Component *Compo)
{
fo << "    Size: " << Compo->size << endl;
Unit1 *ptr=Compo->orig;
for (int i=0; i<Compo->size; i++) {
	fo<<ptr->unit[i%UNIT_SIZE]<<"\t";
	if ((i+1)%UNIT_SIZE==0) ptr=ptr->next; 
	}
return fo;
}

Unit2::Unit2() {
	unit=new Component*[UNIT_SIZE];
	next = NULL;
	}

Unit2::~Unit2() { 
	delete [] unit;
	}

Components::Components() { 
	orig = head = tail = new Unit2();
	ncluster =noutlier = start = end = 0;
	}

Components::~Components() {
	Unit2 *ptr;
	while (head!=tail) { 
		ptr=head; 
		head=head->next; 
		delete ptr;
		}
	delete tail;
	}

int Components::Size() const {return ncluster+noutlier;}

int Components::NumCluster() const {return ncluster;}

int Components::NumOutlier() const {return noutlier;}

void Components::AddComponent(Component *Compo) {
	Unit2 *ptr;
	tail->unit[end]=Compo; end++; 
	if (Compo->Size()>1) ncluster++; else noutlier++;
	if (end==UNIT_SIZE) { 
		end=0;
		ptr=new Unit2();
		tail->next=ptr;
		tail=tail->next;
		}
	}

Component* Components::CurComponent() {
	Component *Compo;
	if (ncluster+noutlier==0) return NULL;
	if (head==tail && start==end) return NULL;
	Compo=head->unit[start]; start++; 
	if (start==UNIT_SIZE) { 
		start=0;
		head=head->next;
		}
	return Compo;
	}

void Components::ResetComponent() {
	head=orig;
	start=0;
	}

ostream& operator<<(ostream &fo, Components *Compos)
{
fo << "    Size: " << Compos->ncluster+Compos->noutlier << endl;
Unit2 *ptr=Compos->orig;
for (int i=0; i<Compos->ncluster+Compos->noutlier; i++) {
	fo<<ptr->unit[i%UNIT_SIZE]<<"\n";
	if ((i+1)%UNIT_SIZE==0) ptr=ptr->next; 
	}
return fo;
}

ofstream& operator<<(ofstream &fo, Components *Compos)
{
fo << "    Size: " << Compos->ncluster+Compos->noutlier << endl;
Unit2 *ptr=Compos->orig;
for (int i=0; i<Compos->ncluster+Compos->noutlier; i++) {
	fo<<ptr->unit[i%UNIT_SIZE]<<"\n";
	if ((i+1)%UNIT_SIZE==0) ptr=ptr->next; 
	}
return fo;
}

// construct graph for nonleaf node
Graph::Graph(int n, Entry* entries)
{
int i, j;

size = n;

flag = new short[n];
for (i=0;i<n;i++) flag[i]=FALSE;

matrix = new short[n*(n-1)/2];

for (i=0;i<n-1;i++)
   for (j=i+1;j<n;j++) {
      	if (connected(entries[i],entries[j])==TRUE)
	    	matrix[i*n-i*(i+1)/2+j-i-1]=1;
	else 	matrix[i*n-i*(i+1)/2+j-i-1]=0;
	}
}

// construct graph for leaf node
Graph::Graph(int n, Entry* entries, 
	     short ftype, double Ft, double density)
{
int 	i, j;

size = n;

flag = new short[n];
for (i=0;i<n;i++) flag[i]=FALSE;

matrix = new short[n*(n-1)/2];

for (i=0;i<n-1;i++)
   for (j=i+1;j<n;j++) {
      	if (connected(entries[i],entries[j],ftype,Ft,density)==TRUE)
	    	matrix[i*n-i*(i+1)/2+j-i-1]=1;
	else 	matrix[i*n-i*(i+1)/2+j-i-1]=0;
	}
}

Graph::~Graph() {
if (flag) delete [] flag;
if (matrix) delete [] matrix;
}

Components* Graph::Connected_Components()
{
Components* Compos;
Component*  Compo;
int	    i,j,k;

Compos=new Components();

for (k=0;k<size;k++) if (flag[k]==FALSE) break;

while (k<size) {

    // create a new component
    Compo=new Component();
    Compo->AddVertex(k);
    flag[k]=TRUE;

    // fuse all connected vertex into the new component
    k=Compo->CurVertex();
    while (k!=-1) { 
	for (i=0;i<k;i++)
		if (flag[i]==FALSE && matrix[i*size-i*(i+1)/2+k-i-1]==1) {
			Compo->AddVertex(i);
			flag[i]=TRUE;
			}
	for (j=k+1;j<size;j++) 
		if (flag[j]==FALSE && matrix[k*size-k*(k+1)/2+j-k-1]==1) {
			Compo->AddVertex(j);
			flag[j]=TRUE;
			}
	k=Compo->CurVertex();
        } 

    // add the new component into components
    Compos->AddComponent(Compo);

    // search for the first vertex for the next new component
    for (k=0;k<size;k++) if (flag[k]==FALSE) break;
    }

return Compos;
}


