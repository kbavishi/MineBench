/****************************************************************
File Name:   contree.C
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
#include "status.h"
#include "cftree.h"
#include "contree.h"
#include "components.h"

ConNode::ConNode(int size, Stat* Stats)
{
actsize=size;
entry=new Entry[size];
child=new ConNode*[size];
for (int i=0; i<size; i++) {
	entry[i].Init(Stats->Dimension);
	child[i]=NULL;
	}
}

ConNode::~ConNode()
{
	delete [] entry;
	delete [] child;
}

void ConNode::Free() {
	for (int i=0; i<actsize; i++) 
		if (child[i]!=NULL) child[i]->Free();
	delete [] entry;
	delete [] child;
	}

int ConNode::N() const {
	int tmp = 0;
	for (int i=0; i<actsize; i++) 
		tmp+=entry[i].n;
	return tmp;
}
	
void ConNode::SX(Vector& tmpsx) const {
	tmpsx.Reset();
	for (int i=0; i<actsize; i++)
		tmpsx+=entry[i].sx;
	}

double ConNode::SXX() const {
	double tmp=0;
	for (int i=0; i<actsize; i++) 
		tmp+=entry[i].sxx;
	return tmp;
}

void ConNode::CF(Entry& tmpcf) const {
	tmpcf.Reset();
	for (int i=0; i<actsize; i++)
		tmpcf+=entry[i];
	}

double ConNode::Radius() const {
	Entry tmpent;
	tmpent.Init(entry[0].sx.dim);
	this->CF(tmpent);
	return tmpent.Radius();
	}

double ConNode::Diameter() const {
	Entry tmpent;
	tmpent.Init(entry[0].sx.dim);
	this->CF(tmpent);
	return tmpent.Diameter();
	}

double ConNode::Fitness(short ftype) const {
	Entry tmpent;
	tmpent.Init(entry[0].sx.dim);
	this->CF(tmpent);
	return tmpent.Fitness(ftype);
	}

#ifdef RECTANGLE
void ConNode::Rect(Rectangle& tmprect) const {
	tmprect.Reset();
	for (int i=0; i<actsize; i++)
		tmprect+=entry[i].rect;
	}
#endif RECTANGLE

int ConNode::Size() const { 
int size=1;
if (child[0]==NULL) return size; 
else   {
	for (int i=0; i<actsize; i++)
		size+=child[i]->Size();
	return size;
	}

}

int ConNode::Depth() const {
if (child[0]==NULL) return 1; 
else	return 1+child[0]->Depth();
}

int ConNode::LeafNum() const { 
int num=0;
if (child[0]==NULL) return 1; 
else 	{
	for (int i=0; i<actsize; i++)
		num+=child[i]->LeafNum();
	return num;
	}
}

int ConNode::NonleafNum() const { 
int num=1;
if (child[0]==NULL) return 0; 
else 	{
	for (int i=0; i<actsize; i++)
		num+=child[i]->NonleafNum();
	return num;
	}
}

int ConNode::NumLeafEntry() const {
int num=0;
if (child[0]==NULL) return actsize; 
else 	{
	for (int i=0; i<actsize; i++)
		num+=child[i]->NumLeafEntry();
	return num;
	}
}

int ConNode::NumNonleafEntry() const { 
int num=actsize;
if (child[0]==NULL) return 0; 
else 	{
	for (int i=0; i<actsize; i++)
		num+=child[i]->NumNonleafEntry();
	return num;
	}
}

int ConNode::NumEntry() const { 
int num=actsize;
if (child[0]==NULL) return actsize;
else 	{
	for (int i=0; i<actsize; i++)
		num+=child[i]->NumEntry();
	return num;
	}
}

void ConNode::Print_Tree(short ind, ostream &fo) const
{
int i;
if (child[0]==NULL) { // leaf
	for (i=0; i<actsize; i++) {
		indent(ind,fo); 
		fo<<entry[i]<< endl;
		}
	}
else { // nonleaf
	for (i=0; i<actsize; i++) {
		indent(ind,fo); 
		fo<<entry[i]<<endl;
		child[i]->Print_Tree(ind+5,fo);
		}
	}
}

void ConNode::Print_Tree(short ind, ofstream &fo) const
{
int i;
if (child[0]==NULL) { // leaf
	for (i=0; i<actsize; i++) {
		indent(ind,fo); 
		fo<<entry[i]<< endl;
		}
	}
else { // nonleaf
	for (i=0; i<actsize; i++) {
		indent(ind,fo); 
		fo<<entry[i]<<endl;
		child[i]->Print_Tree(ind+5,fo);
		}
	}
}

void ConNode::Print_Summary(ostream &fo) const 
{
Entry tmpent;
tmpent.Init(entry[0].sx.dim);
CF(tmpent);
fo<<"Root CF\t"<<tmpent<<endl;
fo<<"FootPrint\t"<<sqrt(tmpent.Radius())<<"\t"<<sqrt(tmpent.Diameter())<<endl;

#ifdef RECTANGLE
Rectangle tmprect;
tmprect.Init(entry[0].sx.dim);
Rect(tmprect);
fo<<"Root Rectangle\t"<<tmprect<<endl;
#endif RECTANGLE

fo<<"Leaf Nodes\t"<<LeafNum()<<endl;
fo<<"Nonleaf Nodes\t"<<NonleafNum()<<endl;
fo<<"Tree Size\t"<<Size()<<endl;
fo<<"Tree Depth\t"<<Depth()<<endl;

fo<<"Leaf Entries\t"<<NumLeafEntry()<<endl;
fo<<"Nonleaf Entries\t"<<NumNonleafEntry()<<endl;
fo<<"Entries\t"<<NumEntry()<<endl;
}

void ConNode::Print_Summary(ofstream &fo) const 
{
Entry tmpent;
tmpent.Init(entry[0].sx.dim);
CF(tmpent);
fo<<"Root CF\t"<<tmpent<<endl;
fo<<"FootPrint\t"<<sqrt(tmpent.Radius())<<"\t"<<sqrt(tmpent.Diameter())<<endl;

#ifdef RECTANGLE
Rectangle tmprect;
tmprect.Init(entry[0].sx.dim);
Rect(tmprect);
fo<<"Root Rectangle\t"<<tmprect<<endl;
#endif RECTANGLE

fo<<"Leaf Nodes\t"<<LeafNum()<<endl;
fo<<"Nonleaf Nodes\t"<<NonleafNum()<<endl;
fo<<"Tree Size\t"<<Size()<<endl;
fo<<"Tree Depth\t"<<Depth()<<endl;

fo<<"Leaf Entries\t"<<NumLeafEntry()<<endl;
fo<<"Nonleaf Entries\t"<<NumNonleafEntry()<<endl;
fo<<"Entries\t"<<NumEntry()<<endl;
}

void ConNode::Connect(Stat* Stats)
{
int 	   i,j;
double     density;

Graph 	   *graph;
Components *Compos;
Component  *Compo;

int	   newsize,allsize;
Entry	   *newentry;
ConNode    **newchild;

if (child[0]==NULL) { // leaf 

	density=Stats->NoiseRate*Stats->NewRoot->N()/Stats->CurrEntryCnt;

	// connect graph based on density and connectivity
	graph=new Graph(actsize,entry,Stats->Ftype,Stats->CurFt,density);
	Compos=graph->Connected_Components();

	Compos->ResetComponent();

	newsize=allsize=Compos->Size();

	for (i=0;i<allsize;i++) {
		Compo=Compos->CurComponent();
		if (Compo->Size()==1 && Compo->TupleCnt(entry)<density) {
			newsize--;
			Stats->OutlierEntryCnt++;
			Stats->OutlierTupleCnt+=Compo->TupleCnt(entry);
			}
		}

	if (newsize<actsize) {
		newentry=new Entry[newsize];
		newchild=new ConNode*[newsize];
		for (i=0;i<newsize;i++) {
			newentry[i].Init(entry[i].sx.dim);
			newchild[i]=NULL;
			}
		
		Compos->ResetComponent(); 

		j=0;
		for (i=0;i<allsize;i++) {
			Compo=Compos->CurComponent();
			if (!(Compo->Size()==1&&Compo->TupleCnt(entry)<density)) {
				Compo->EntryChild(Stats,entry,child,
						  newentry[j],newchild[j]);
				j++;
				}
			}

		actsize=newsize;
		delete [] entry;
		entry=newentry;
		delete [] child;
		child=newchild;
		}
	}

else { // nonleaf

	graph=new Graph(actsize,entry);
	Compos=graph->Connected_Components();

	newsize=Compos->Size();

	if (newsize<actsize) {
		newentry=new Entry[newsize];
		newchild=new ConNode*[newsize];
		for (i=0;i<newsize;i++) {
			newentry[i].Init(entry[i].sx.dim);
			newchild[i]=NULL;
			Compo=Compos->CurComponent();
			Compo->EntryChild(Stats,entry,child,
					  newentry[i],newchild[i]);
			}
		actsize=newsize;
		delete [] entry;
		entry=newentry;
		delete [] child;
		child=newchild;
		}

	for (i=0;i<actsize;i++) child[i]->Connect(Stats);
	}
}

