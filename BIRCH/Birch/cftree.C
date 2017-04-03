/****************************************************************
File Name:   cftree.C
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
#include "contree.h"
#include "cftree.h"
#include "path.h"

int Node::N() const {
	int tmp = 0;
	for (int i=0; i<actsize; i++) 
		tmp+=entry[i].n;
	return tmp;
}
	
void Node::SX(Vector& tmpsx) const {
	tmpsx.Reset();
	for (int i=0; i<actsize; i++)
		tmpsx+=entry[i].sx;
	}

double Node::SXX() const {
	double tmp=0;
	for (int i=0; i<actsize; i++) 
		tmp+=entry[i].sxx;
	return tmp;
}

void Node::CF(Entry& tmpcf) const {
	tmpcf.Reset();
	for (int i=0; i<actsize; i++)
		tmpcf+=entry[i];
	}

double Node::Radius() const {
	Entry tmpent;
	tmpent.Init(entry[0].sx.dim);
	this->CF(tmpent);
	return tmpent.Radius();
	}

double Node::Diameter() const {
	Entry tmpent;
	tmpent.Init(entry[0].sx.dim);
	this->CF(tmpent);
	return tmpent.Diameter();
	}

double Node::Fitness(short ftype) const {
	Entry tmpent;
	tmpent.Init(entry[0].sx.dim);
	this->CF(tmpent);
	return tmpent.Fitness(ftype);
	}

#ifdef RECTANGLE
void Node::Rect(Rectangle& tmprect) const {
	tmprect.Reset();
	for (int i=0; i<actsize; i++)
		tmprect+=entry[i].rect;
	}
#endif RECTANGLE

double Leaf::AbsVofLevel(int i, short ftype, short dim) const {
if (i>0) print_error("Leaf::AbsVofLevel","can not go further");
if (i==0) return pow(sqrt(this->Fitness(ftype)),dim);
}

double Nonleaf::AbsVofLevel(int i, short ftype, short dim) const {
double AbsV=0.0;
if (i>Depth()-1) print_error("Nonleaf::AbsVofLevel","can not go further"); 
if (i==0) return pow(sqrt(this->Fitness(ftype)),dim);
else { for (int j=0; j<actsize; j++) 
	AbsV+=child[j]->AbsVofLevel(i-1,ftype,dim);
       return AbsV;
       }
}

int Leaf::Size() const { return 1; }

int Nonleaf::Size() const {
	int size=1;
	for (int i=0; i<actsize; i++) 
		size+=child[i]->Size();
	return size;
}

int Leaf::Depth() const { return 1; }

int Nonleaf::Depth() const {
	if (actsize==0) return 1; 
        else return 1+child[0]->Depth();
	}

int Leaf::LeafNum() const { return 1; }

int Nonleaf::LeafNum() const {
	int num=0;
	for (int i=0; i<actsize; i++) 
		num+=child[i]->LeafNum();
	return num;
}

int Leaf::NonleafNum() const { return 0; }

int Nonleaf::NonleafNum() const {
	int num=1;
	for (int i=0; i<actsize; i++)
		num+=child[i]->NonleafNum();
	return num;
}

int Leaf::NumLeafEntry() const { return actsize; }

int Leaf::NumNonleafEntry() const { return 0; }

int Nonleaf::NumLeafEntry() const {
	int tmpn = 0;
	for (int i=0; i<actsize; i++)
		tmpn+=child[i]->NumLeafEntry();
	return tmpn;
	}

int Nonleaf::NumNonleafEntry() const {
	int tmpn = actsize;
	for (int i=0; i<actsize; i++)
		tmpn+=child[i]->NumNonleafEntry();
	return tmpn;
	}

int Leaf::NumEntry() const{ return actsize; }

int Nonleaf::NumEntry() const {
	int tmpn = actsize;
	for (int i=0; i<actsize; i++) 
		tmpn+=child[i]->NumEntry();
	return tmpn;
}

double Leaf::Occupancy(Stat *Stats) const {
	return actsize/(1.0*MaxSize(Stats));
}

double Nonleaf::Occupancy(Stat *Stats) const {
	int leafsize, nonleafsize;
#ifdef RECTANGLE
	leafsize=(Stats->PageSize-2*sizeof(int))/
		 (sizeof(int)+sizeof(double)*(3*Stats->Dimension+1));
#else 
	leafsize=(Stats->PageSize-2*sizeof(int))/
		 (sizeof(int)+sizeof(double)*(Stats->Dimension+1));
#endif
	nonleafsize=MaxSize(Stats);
	return NumEntry()/(1.0*nonleafsize*NonleafNum()+1.0*leafsize*LeafNum());
}

void Node::Print_Summary(Stat *Stats, ostream &fo) const 
{
Entry tmpent;
tmpent.Init(entry[0].sx.dim);
CF(tmpent);
fo<<"Root CF\t"<<tmpent<<endl;
fo<<"FootPrint\t"<<sqrt(tmpent.Radius())<<endl;

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
fo<<"Nonleaf Entries\t"<<NumNonleafEntry()<< endl;
fo<<"Occupancy\t"<<Occupancy(Stats)<<endl;
}

void Node::Print_Summary(Stat *Stats, ofstream &fo) const 
{
Entry tmpent;
tmpent.Init(entry[0].sx.dim);
CF(tmpent);
fo<<"Root CF\t"<<tmpent<<endl;
fo<<"FootPrint\t"<<sqrt(tmpent.Radius())<<endl;

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
fo<<"Nonleaf Entries\t"<<NumNonleafEntry()<< endl;
fo<<"Occupancy\t"<<Occupancy(Stats)<<endl;
}

Node* Leaf::DenseNode()
{
if (actsize>1) return this;
else return NULL;
}

short Leaf::FreeEmptyNode(Stat *Stats)
{
short flag=TRUE;
for (int i=0; i<actsize; i++) 
	if (entry[i].n>0) {flag=FALSE; break;}

if (flag==TRUE) {
	this->AssignNextPrev(Stats); 
	delete this; 
	Stats->MemUsed--; 
	Stats->TreeSize--;
	}

return flag;
}

// fit without expanding or splitting
short Leaf::BestFitPath1(Stat *Stats, const Entry &ent, Path& BestPath)
{
int EntryI=ClosestOne(Stats,ent);
if (EntryI>=0) {
	BestPath.Push(EntryI,this);
	return TRUE;
	}
else return FALSE;
}

// fit with expanding but without splitting
short Leaf::BestFitPath2(Stat *Stats, const Entry &ent, Path& BestPath)
{
int EntryI=ClosestOne(Stats,ent);
if (EntryI>=0) {
	BestPath.Push(EntryI,this);
	return TRUE;
	}
else if (actsize<MaxSize(Stats)) {
	BestPath.Push(actsize,this);
	return TRUE;
	}
else return FALSE;
}

// absorb without expanding and splitting
short Leaf::AbsorbEntry1(Stat *Stats, const Entry &ent)
{
int EntryI;
EntryI=ClosestOne(Stats,ent);
if (EntryI>=0) {
	entry[EntryI]+=ent;
	return TRUE;
	}
else return FALSE;
}

// absorb with expanding but without splitting
short Leaf::AbsorbEntry2(Stat *Stats, const Entry &ent)
{
int EntryI;
EntryI=ClosestOne(Stats,ent);
if (EntryI>=0) {
	entry[EntryI]+=ent;
	return TRUE;
	}
else if (actsize<MaxSize(Stats)) {
	entry[actsize]=ent;
	actsize++; 
	Stats->CurrEntryCnt++;
	return TRUE;
	}
     else return FALSE;
}

ConNode* Leaf::Copy(Stat *Stats) const
{
ConNode* node=new ConNode(actsize,Stats);
for (int i=0; i<actsize; i++) node->entry[i]=entry[i];
return node;
}

ConNode* Nonleaf::Copy(Stat *Stats) const
{
ConNode* node=new ConNode(actsize,Stats);
for (int i=0; i<actsize; i++) {
	node->entry[i]=entry[i];
	node->child[i]=child[i]->Copy(Stats);
	}
return node;
}

Node* Leaf::AdjustTree(Stat *Stats, const Entry &ent) 
{
int  EntryI;
Node *NewNode;

if (actsize==0) {
	entry[actsize]=ent;
	actsize++; 
	Stats->CurrEntryCnt++;
	return NULL;
	}

EntryI=ClosestOne(Stats,ent);
if (EntryI>=0) {
	entry[EntryI]+=ent;
	return NULL;
	}
else {
	NewNode = InsertMightSplit(Stats,ent,NULL);
	if (NewNode==NULL) return NULL;
	else {
		if (this!=Stats->OldRoot)
			return NewNode;
		else {  Stats->CreateNewRoot(this,NewNode);
			return NULL;
	             }
             }
	}
}

Node* Nonleaf::DenseNode()
{
Node *tmp;

// less than 2 entries: this->N()<=1||this->actsize<=1 
if (this->N()<=1) return NULL;
if (this->actsize<=1) {
	tmp = child[0]->DenseNode();
	if (tmp==NULL) return NULL;
	else return tmp;
	}

// more than 2 entries: this->N()>1&&this->actsize>1
int EntryI = DensestEntry();
tmp=child[EntryI]->DenseNode();
if (tmp==NULL) return this;
else return tmp;
}

short Nonleaf::FreeEmptyNode(Stat *Stats)
{
short flag=TRUE;
int i=0;
while (i<actsize) {
	if (child[i]->FreeEmptyNode(Stats)==TRUE) {
		actsize--;
		entry[i]=entry[actsize];
		child[i]=child[actsize];
		}
	else {  flag=FALSE;
		i++;
		}
	}

if (flag==TRUE) {
	delete this; 
	Stats->MemUsed--; 
	Stats->TreeSize--;
	}
return flag;
}
		
// fit without expanding and splitting
short Nonleaf::BestFitPath1(Stat *Stats, const Entry &ent, Path& BestPath)
{
int EntryI=ClosestOne(Stats,ent);
if (EntryI>=0) {
	BestPath.Push(EntryI,this);	
	if (child[EntryI]->BestFitPath1(Stats,ent,BestPath)==TRUE) 
		return TRUE;
	else 	return FALSE;
	}
else return FALSE;
}

// fit with expanding but without splitting
short Nonleaf::BestFitPath2(Stat *Stats, const Entry &ent, Path& BestPath)
{
int EntryI=ClosestOne(Stats,ent);
if (EntryI>=0) {
	BestPath.Push(EntryI,this);	
	if (child[EntryI]->BestFitPath2(Stats,ent,BestPath)==TRUE) 
		return TRUE;
	else 	return FALSE;
	}
else return FALSE;
}

// absorb without expanding and splitting
short Nonleaf::AbsorbEntry1(Stat *Stats, const Entry &ent)
{
int EntryI;
EntryI = ClosestOne(Stats,ent);
if (child[EntryI]->AbsorbEntry1(Stats,ent)==TRUE) {
	entry[EntryI]+=ent;
	return TRUE;
	}
else return FALSE;
}

// absorb with expanding but without splitting
short Nonleaf::AbsorbEntry2(Stat *Stats, const Entry &ent)
{
int EntryI;
EntryI=ClosestOne(Stats,ent);
if (child[EntryI]->AbsorbEntry2(Stats,ent)==TRUE) {
	entry[EntryI]+=ent;
	return TRUE;
	}
else return FALSE;
}

Node* Nonleaf::AdjustTree(Stat *Stats, const Entry &ent)
{
int EntryI;
int i,j;
double d;

Node *ResNode, *NewNode;
Entry ResEnt;
ResEnt.Init(Stats->Dimension);

EntryI=ClosestOne(Stats,ent);
ResNode=child[EntryI]->AdjustTree(Stats,ent);

if (ResNode!=NULL) { // Split Propagate
	child[EntryI]->CF(entry[EntryI]);
	ResNode->CF(ResEnt);
	NewNode=InsertMightSplit(Stats,ResEnt,ResNode); 
	if (NewNode==NULL) { // Split Propagate Stops
	   if (actsize>2) {
		d=ClosestTwo(Stats, i, j);
		if (!(i==EntryI&&j==actsize-1))
	       		MergeMightResplit(Stats, i, j);
		}
	  return NULL;
	  }
	 else { if (this!=Stats->OldRoot) 
			return NewNode;
	        else { // Create New Root
	  		Stats->CreateNewRoot(this,NewNode);
	  		return NULL;
	        	}
              }
	}
else { // No Split Coming Up
	entry[EntryI]+=ent; 
	return NULL;
	}
}

int Nonleaf::DensestEntry() const
{
int i, imax, nmax;
imax = 0;
nmax = entry[0].n;
for (i=1; i<actsize; i++) {
	if (entry[i].n>nmax) {
		imax = i;
		nmax = entry[i].n;
		}
	}
return imax;
}

int Nonleaf::ClosestOne(Stat *Stats, const Entry& ent) const 
{
int i, imin;
double d, dmin;

// empty node
if (actsize<=1) return 0;

// nonemptry node
imin = 0;
dmin = HUGE_DOUBLE;

for (i=0; i<actsize; i++) {
    if (entry[i].n>0) {
	d = distance(Stats->BDtype,entry[i],ent);
	if (d<dmin) {dmin=d;imin=i;}
	}
    }
if (dmin<HUGE_DOUBLE) return imin;
else return -1;
}

int Leaf::DensestEntry() const
{
// do nothing and never be called.
}

int Leaf::ClosestOne(Stat *Stats, const Entry& ent) const 
{
int i, imin;
double d, dmin;

// empty node
if (actsize<1) return 0;

// nonempty node
imin = 0;
dmin = HUGE_DOUBLE;

/***************************************************
// option1: min average D: tend to cause overlapping 
for (i=0; i<actsize; i++) {
    if (entry[i].n>0) {
	d = fitness(Stats->Ftype,entry[i],ent);
	if (d<dmin) {
		dmin=d;
		imin=i;
		}
	}
    }
if (dmin<=Stats->CurFt+PRECISION_ERROR) return imin;
else return -1;

***************************************************/

// option2: min average Di: less overlapping
for (i=0; i<actsize; i++) {
    if (entry[i].n>0) {
	d = distance(Stats->BDtype,entry[i],ent);
	if (d<dmin) {
		dmin=d;
		imin=i;
		}
	}
    }
if (dmin<HUGE_DOUBLE) {
	dmin = fitness(Stats->Ftype,entry[imin],ent);
        if (dmin<=Stats->CurFt+PRECISION_ERROR) return imin;
        else return -1;
	}
else return -1;
}

double Leaf::ClosestTwo(Stat *Stats, int &i, int &j) const {
int i1,j1,imin,jmin;
double d, dmin;

if (actsize<2) 
	print_error("Leaf::ClosestTwo","Less than 2 entries");

if (actsize==2) {
	i=0; j=1; 
	return distance(Stats->BDtype,entry[0],entry[1]);
	}

imin = 0; 
jmin = 1;
dmin = distance(Stats->BDtype,entry[0],entry[1]);

for (i1=0;i1<actsize-1;i1++)
   for (j1=i1+1;j1<actsize;j1++) {
		d = distance(Stats->BDtype,entry[i1],entry[j1]);
		if (d<dmin) { 
			imin = i1; 
			jmin = j1; 
			dmin = d;
			}
	}
i=imin; 
j=jmin;
return dmin;
}

// only relevant to phase 3 and hierarchical clustering

double Leaf::ClosestDiffTwo(Stat *Stats, int &i, int &j) const 
{
Entry tmpent;
tmpent.Init(Stats->Dimension);
int i1,j1,imin,jmin;
double d, dmin;

if (actsize<2) 
	print_error("Leaf::ClosestDiffTwo","Less than 2 entries");

if (actsize==2) 
	{d=distance(Stats->GDtype,entry[0],entry[1]);
	 if (d<=0) 
	 print_error("Leaf::ClosestDiffTwo",
		"Same 2 entries in a leaf: should not happen");
	 }

dmin = HUGE_DOUBLE;
imin=0; 
jmin=1;
for (i1=0;i1<actsize-1;i1++)
   for (j1=i1+1;j1<actsize;j1++) {
		d = distance(Stats->GDtype,entry[i1],entry[j1]);
		if (d>0 && d<dmin) { 
			imin = i1; 
			jmin = j1; 
			dmin = d;
			}
	}
i=imin; 
j=jmin;
tmpent.Add(entry[i],entry[j]);
return tmpent.Fitness(Stats->Ftype);
}

void Leaf::FarthestTwo(Stat *Stats, int &i, int &j) const {
int i1,j1,imax,jmax;
double d, dmax;

if (actsize<2) 
	print_error("Leaf::FarthestTwo","Less than 2 entries");

if (actsize==2) 
	{i=0; j=1; return;}

imax = 0; 
jmax = 1;
dmax = distance(Stats->BDtype,entry[0],entry[1]);

for (i1=0;i1<actsize-1;i1++)
   for (j1=i1+1;j1<actsize;j1++) {
		d = distance(Stats->BDtype,entry[i1],entry[j1]);
		if (d>dmax) { 
			imax = i1; 
			jmax = j1; 
			dmax = d;}
	}
i=imax; j=jmax;
}

double Nonleaf::ClosestTwo(Stat *Stats, int &i, int &j) const {
int i1,j1,imin,jmin;
double d, dmin;

if (actsize<2) 
	print_error("Nonleaf::ClosestTwo","Less than 2 entries");

if (actsize==2) {
	i=0; j=1; 
	return distance(Stats->BDtype,entry[0],entry[1]);
	}

imin = 0; 
jmin = 1;
dmin = distance(Stats->BDtype,entry[0],entry[1]);

for (i1=0;i1<actsize-1;i1++)
   for (j1=i1+1;j1<actsize;j1++) {
		d = distance(Stats->BDtype,entry[i1],entry[j1]);
		if (d<dmin) { 
			imin = i1; 
			jmin = j1; 
			dmin = d;
			}
	}
i=imin; 
j=jmin;
return dmin;
}

double Nonleaf::ClosestDiffTwo(Stat *Stats, int &i, int &j) const 
{
Entry tmpent;
tmpent.Init(Stats->Dimension);
int i1,j1,imin,jmin;
double d, dmin;

if (actsize<2) 
	print_error("Nonleaf::ClosestDiffTwo","Less than 2 entries");

if (actsize==2) {
	d=distance(Stats->GDtype,entry[0],entry[1]);
	if (d==0) 
	print_error("Nonleaf::ClosestDiffTwo",
		    "Same 2 entries in a nonleaf: should not happen");
	}

dmin=HUGE_DOUBLE;
imin=0;
jmin=1;
for (i1=0;i1<actsize-1;i1++)
   for (j1=i1+1;j1<actsize;j1++) {
		d = distance(Stats->GDtype,entry[i1],entry[j1]);
		if (d>0 && d<dmin) { 
			imin = i1; 
			jmin = j1; 
			dmin = d;}
	}
i=imin; 
j=jmin;
tmpent.Add(entry[i],entry[j]);
return tmpent.Fitness(Stats->Ftype);
}

void Nonleaf::FarthestTwo(Stat *Stats, int &i, int &j) const {
int i1,j1,imax,jmax;
double d, dmax;

if (actsize<2) 
	print_error("Nonleaf::FarthestTwo","Less than 2 entries");

if (actsize==2) 
	{i=0; j=1; return;}

imax = 0; 
jmax = 1;
dmax = distance(Stats->BDtype,entry[0],entry[1]);
for (i1=0;i1<actsize-1;i1++)
   for (j1=i1+1;j1<actsize;j1++) {
		d = distance(Stats->BDtype,entry[i1],entry[j1]);
		if (d>dmax) { 
			imax = i1; 
			jmax = j1; 
			dmax = d;}
	}
i=imax; j=jmax;
}

// follow chain of leaves
int Leaf::count_leaf_nodes() const {
int num=1;
if (this->next!=NULL) num+=this->next->count_leaf_nodes();
return num;
}

int Leaf::count_leaf_entries() const {
int num=actsize;
if (this->next!=NULL) num+=this->next->count_leaf_entries();
return num;
}

int Leaf::count_leaf_tuples() const {
int num=0;
for (int i=0; i<actsize; i++) num += entry[i].n;
if (this->next!=NULL) num+=this->next->count_leaf_tuples();
return num;
}

void Nonleaf::Print_Tree(short ind, ostream &fo) const {
for (int i=0; i<actsize; i++) {
	indent(ind,fo); fo<<entry[i]<<endl;
	child[i]->Print_Tree(ind+5,fo);
	}
}

void Nonleaf::Print_Tree(short ind, ofstream &fo) const {
for (int i=0; i<actsize; i++) {
	indent(ind,fo); fo<<entry[i]<<endl;
	child[i]->Print_Tree(ind+5,fo);
	}
}

void Leaf::Print_Tree(short ind, ostream &fo) const {
for (int i=0; i<actsize; i++) {
	indent(ind,fo); fo<<entry[i]<< endl;
	}
}
void Leaf::Print_Tree(short ind, ofstream &fo) const {
for (int i=0; i<actsize; i++) {
	indent(ind,fo); fo<<entry[i]<<endl;
	}
}
	
void Leaf::visualize_leaf_entries(Stat *Stats, ostream &fo) {
if (Stats->Dimension>2) 
print_error("Nonleaf::Visualize",
	    "can't visualize higher than 2 dimensions");
int i;
Leaf *tmp;
tmp=this;
while (tmp!=NULL) {
    for (i=0; i<tmp->actsize; i++) 
	tmp->entry[i].Visualize_Rectangle(fo);
    tmp=tmp->next;
    }
tmp=this;
while (tmp!=NULL) {
    for (i=0; i<tmp->actsize; i++) 
	tmp->entry[i].Visualize_Circle(fo);
    tmp=tmp->next;
    }
}

void Leaf::visualize_leaf_entries(Stat *Stats, ofstream &fo) {
  if (Stats->Dimension>2) 
    print_error("Nonleaf::Visualize",
		"can't visualize higher than 2 dimensions");
  int i;
  Leaf *tmp;
  tmp=this;
  while (tmp!=NULL) {
    for (i=0; i<tmp->actsize; i++) 
      tmp->entry[i].Visualize_Rectangle(fo);
    tmp=tmp->next;
  }
  tmp=this;
  while (tmp!=NULL) {
    for (i=0; i<tmp->actsize; i++) 
      tmp->entry[i].Visualize_Circle(fo);
    tmp=tmp->next;
  }
}

// follow chain of leaves
void Leaf::print_leaf_entries_bychain(ofstream &fo) const {
fo<<"#"<<actsize<<endl;
for (int i=0; i<actsize; i++) 
	fo<<entry[i]<<endl;
if (this->next!=NULL) 
	this->next->print_leaf_entries_bychain(fo);
}

// follow chain of leaves
void Leaf::print_leaf_entries_bychain(ostream &fo) const {
for (int i=0; i<actsize; i++) 
	fo<<entry[i]<<endl;
if (this->next!=NULL) 
	this->next->print_leaf_entries_bychain(fo);
}

// topdown search for leaves
void Leaf::print_leaf_entries_topdown(ofstream &fo) const {
fo<<"#"<<actsize<<endl;
for (int i=0; i<actsize; i++) 
	fo<<entry[i]<<endl;
}

// topdown search for leaves
void Nonleaf::print_leaf_entries_topdown(ofstream &fo) const {
for (int i=0; i<actsize; i++) 
	child[i]->print_leaf_entries_topdown(fo);
}

Node* Leaf::InsertMightSplit(Stat *Stats, const Entry &Ent, Node *ptr) 
{
short 	samegroup;
Leaf 	*NewNode;
int 	i1,i2,n1,n2,head1,head2;
double 	d1, d2;
Entry 	ent1,ent2;
ent1.Init(Stats->Dimension);
ent2.Init(Stats->Dimension);

if (NotFull(Stats)) {
	entry[actsize]=Ent;
	actsize++; 
	Stats->CurrEntryCnt++;
	return NULL;
	}

NewNode = new Leaf(Stats); 
Stats->MemUsed++; 
Stats->TreeSize++;
NewNode->entry[NewNode->actsize]=Ent;
NewNode->actsize++; 
Stats->CurrEntryCnt++;

// chain leaf nodes
if (this->next!=NULL) {
	this->next->prev=NewNode;
	}
NewNode->next=this->next;
NewNode->prev=this;
this->next=NewNode;

this->FarthestTwoOut(Stats,this,NewNode,samegroup,i1,i2);
switch (samegroup) {
case 0: this->swap(this,0, this,i1);
        NewNode->swap(NewNode,0, NewNode,i2);
        break;
case 1: this->swap(this,0, this,i1);
        NewNode->swap(NewNode,0, this,i2);
        break;
default: print_error("Leaf::InsertMightSplit","Invalid group flag");
}

n1 = MaxSize(Stats);
n2 = 1;

head1 = 1; head2 = 1;
ent1 = this->entry[0];
ent2 = NewNode->entry[0];
while (head1<n1) {
	d1 = distance(Stats->BDtype,ent1,this->entry[head1]);
	d2 = distance(Stats->BDtype,ent2,this->entry[head1]);
	if (d1<d2) {
		ent1+=this->entry[head1];
		head1++;
		}
	else {  this->assign(NewNode,head2,this,head1);
		this->assign(this,head1,this,n1-1);
		ent2+=NewNode->entry[head2];
		head2++;
		n1--;
		n2++;
		}
	}
this->actsize=n1;
NewNode->actsize=n2;
return(NewNode);
}

Node* Nonleaf::InsertMightSplit(Stat *Stats, const Entry &Ent, Node *Ptr) 
{
short 	samegroup;
Nonleaf *NewNode;
int 	i1,i2,n1,n2,head1,head2;
double 	d1, d2;

Entry 	ent1,ent2;
ent1.Init(Stats->Dimension);
ent2.Init(Stats->Dimension);

if (NotFull(Stats)) {
	entry[actsize]=Ent;
	child[actsize]=Ptr;
	actsize++;
	return NULL;
	}

NewNode=new Nonleaf(Stats); 
Stats->MemUsed++; 
Stats->TreeSize++;
NewNode->entry[NewNode->actsize]=Ent;
NewNode->child[NewNode->actsize]=Ptr;
NewNode->actsize++;

this->FarthestTwoOut(Stats,this,NewNode,samegroup,i1,i2);
switch (samegroup) {
case 0: this->swap(this,0, this,i1);
	NewNode->swap(NewNode,0, NewNode,i2);
	break;
case 1: this->swap(this,0, this,i1);
	NewNode->swap(NewNode,0, this,i2);
	break;
default: print_error("Nonleaf::InsertMightSplit","Invalid group flag");
}

n1=MaxSize(Stats);
n2=1;
head1=1; head2=1;
ent1 = this->entry[0];
ent2 = NewNode->entry[0];
while (head1<n1) {
	d1 = distance(Stats->BDtype,ent1,this->entry[head1]);
	d2 = distance(Stats->BDtype,ent2,this->entry[head1]);
	if (d1<d2) {
		ent1+=this->entry[head1];
		head1++;
		}
	else {  this->assign(NewNode,head2,this,head1);
		this->assign(this,head1,this,n1-1);
		ent2+=NewNode->entry[head2];
		head2++;
		n1--;
		n2++;
		}
	}
this->actsize=n1; 
NewNode->actsize=n2;
return(NewNode);
}

void Nonleaf::MergeMightResplit(Stat *Stats, int i, int j) 
{
int 	head1,head2,k,j1,j2,n1,n2,total;
short 	samegroup;
double 	d1, d2;

Entry 	ent1,ent2;
ent1.Init(Stats->Dimension);
ent2.Init(Stats->Dimension);

n1 = child[i]->ActSize();
n2 = child[j]->ActSize();
total = n1+n2;

// Merge: no overflow
if (total<child[i]->MaxSize(Stats)) {

	child[i]->AssignActSize(total);
	entry[i]+=entry[j];
	for (k=0;k<n2;k++) 
		child[i]->assign(child[i],n1+k, child[j],k);
	child[j]->AssignNextPrev(Stats);
	delete child[j]; 
	Stats->MemUsed--; 
	Stats->TreeSize--;
	this->assign(this,j, this,actsize-1);
	actsize--;
	}

// ReSplit: overflow
else {

//farthest pair in tmpentry
child[i]->FarthestTwoOut(Stats, child[i],child[j],samegroup,j1,j2);
switch (samegroup) {
case 0: child[i]->swap(child[i],0, child[i],j1);
	child[j]->swap(child[j],0, child[j],j2);
	break;
case 1: child[i]->swap(child[i],0, child[i],j1);
	child[j]->swap(child[j],0, child[i],j2);
	break;
case 2: child[i]->swap(child[i],0, child[j],j1);
	child[j]->swap(child[j],0, child[j],j2);
	break;
default: print_error("Nonleaf::MergeMightResplit","Invalid group flag");
}

head1=1; head2=1;
ent1 = *(child[i]->TheEntry(0));
ent2 = *(child[j]->TheEntry(0));
while (head1<n1 && head2<n2) {
   d1 = distance(Stats->BDtype,ent1,*(child[i]->TheEntry(head1)));
   d2 = distance(Stats->BDtype,ent2,*(child[i]->TheEntry(head1)));
   if (d1<d2) {
	ent1+=*(child[i]->TheEntry(head1));
        head1++;
	}
   else { 
	child[i]->swap(child[i],head1,child[j],head2);
	ent2 += *(child[j]->TheEntry(head2));
	head2++;
	}
    }

// child[j] has left over
if (head1>=n1 && head2<n2) {
  while (head2<n2 && n1<child[i]->MaxSize(Stats)) {
   d1 = distance(Stats->BDtype,ent1,*(child[j]->TheEntry(head2)));
   d2 = distance(Stats->BDtype,ent2,*(child[j]->TheEntry(head2)));
   if (d2<d1) {
      	ent2 += *(child[j]->TheEntry(head2));
	head2++;
	}
   else {
	child[i]->assign(child[i],head1,child[j],head2);
	child[j]->assign(child[j],head2,child[j],n2-1);
	ent1+=*(child[i]->TheEntry(head1));
	head1++;
	n1++;
	n2--;
	}
   }
}

// child[i] has left over
else if (head1<n1 && head2>=n2) {
   while (head1<n1 && n2<child[j]->MaxSize(Stats)) {
    d1 = distance(Stats->BDtype,ent1,*(child[i]->TheEntry(head1)));
    d2 = distance(Stats->BDtype,ent2,*(child[i]->TheEntry(head1)));
    if (d1<d2) {
   	ent1 += *(child[i]->TheEntry(head1));
	head1++;
	}
    else {
	child[j]->assign(child[j],head2,child[i],head1);
	child[i]->assign(child[i],head1,child[i],n1-1);
	ent2+=*(child[j]->TheEntry(head2));
	head2++;
	n1--;
	n2++;
	}
    }
}
child[i]->AssignActSize(n1);
child[j]->AssignActSize(n2);	
child[i]->CF(entry[i]);
child[j]->CF(entry[j]);
}
}

void Leaf::FarthestTwoOut(Stat *Stats, Node *node1, Node *node2, 
	                  short &samegroup, int &i, int &j) const 
{
int 	i1, i2, j1, j2, n1, n2;
double 	d, dmax;
Leaf 	*leaf1, *leaf2;

leaf1 = (Leaf *)node1;
leaf2 = (Leaf *)node2;
n1 = leaf1->ActSize();
n2 = leaf2->ActSize();

if (n1==0 || n2==0)
        print_error("Leaf::FarthestTwoOut","empty node");

samegroup = 0;
i=0; j=0;
dmax = distance(Stats->BDtype,leaf1->entry[i],leaf2->entry[j]);

for (i1=0; i1<n1; i1++)
        for (i2=0; i2<n2; i2++) {
           d = distance(Stats->BDtype,leaf1->entry[i1],leaf2->entry[i2]);
           if (d>dmax) {
                i=i1;
                j=i2;
                dmax=d;
                }
        }
for (i1=0; i1<n1-1; i1++)
        for (j1=i1+1; j1<n1; j1++) {
           d = distance(Stats->BDtype,leaf1->entry[i1],leaf1->entry[j1]);
           if (d>dmax) {
                i=i1;
                j=j1;
                dmax=d;
                samegroup=1;
                }
        }
for (i2=0; i2<n2-1; i2++)
        for (j2=i2+1; j2<n2; j2++) {
           d = distance(Stats->BDtype,leaf2->entry[i2],leaf2->entry[j2]);
           if (d>dmax) {
                i=i2;
                j=j2;
                dmax=d;
                samegroup=2;
                }
	}
}


void Nonleaf::FarthestTwoOut(Stat *Stats, Node *node1, Node *node2, 
	                     short &samegroup, int &i, int &j) const 
{
int 	i1, i2, j1, j2, n1, n2;
double 	d, dmax;
Nonleaf *nleaf1, *nleaf2;

nleaf1 = (Nonleaf *)node1;
nleaf2 = (Nonleaf *)node2;
n1 = nleaf1->ActSize();
n2 = nleaf2->ActSize();

if (n1==0 || n2==0)
        print_error("Nonleaf::FarthestTwoOut","empty node");

samegroup = 0;
i=0; j=0;
dmax=distance(Stats->BDtype,nleaf1->entry[i],nleaf2->entry[j]);

for (i1=0; i1<n1; i1++)
   for (i2=0; i2<n2; i2++) {
        d = distance(Stats->BDtype,nleaf1->entry[i1],nleaf2->entry[i2]);
        if (d>dmax) {
                i=i1;
                j=i2;
                dmax=d;
                }
        }
for (i1=0; i1<n1-1; i1++)
    for (j1=i1+1; j1<n1; j1++) {
        d = distance(Stats->BDtype,nleaf1->entry[i1],nleaf1->entry[j1]);
        if (d>dmax) {
                i=i1;
                j=j1;
                dmax=d;
                samegroup=1;
                }
        }
for (i2=0; i2<n2-1; i2++)
    for (j2=i2+1; j2<n2; j2++) {
        d = distance(Stats->BDtype,nleaf2->entry[i2],nleaf2->entry[j2]);
        if (d>dmax) {
                i=i2;
                j=j2;
                dmax=d;
                samegroup=2;
                }
	}
}



