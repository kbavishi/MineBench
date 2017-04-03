/****************************************************************
File Name: status.C  
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
#include "path.h"
#include "contree.h"
#include "buffer.h"
#include "recyqueue.h"

#include "hierarchy.h"

Stat::Stat(char *str) {

// initialize dynamic status information

strcpy(name,str);

Bars=NULL;

Phase=1;
Passi=0;
CurFt=0.0;
MemUsed=0;
TreeSize=0;
 
PrevEntryCnt = 0;
CurrEntryCnt = 0;
PrevDataCnt = 0;
CurrDataCnt = 0;

NoiseCnt = 0;

AvgDensity = 0;

OldRoot = NULL;
NewRoot = NULL;

OldLeafHead = NULL;
NewLeafHead = NULL;

RestLeafPtr = NULL;
RestLeafK = 0;

SplitBuffer = NULL;
OutlierQueue = NULL;
OStats=NULL;

Entries = NULL;

OutlierEntryCnt=0;
OutlierTupleCnt=0;
}

Stat::~Stat() {
if (Bars) delete [] Bars;
if (NewRoot!=NULL) NewRoot->free_nonleaf(this);
if (NewLeafHead!=NULL) NewLeafHead->free_leaf(this);
if (SplitBuffer!=NULL) delete SplitBuffer;
if (OutlierQueue!=NULL) delete OutlierQueue;
if (OStats!=NULL) delete OStats;
if (Entries) delete [] Entries;
}

void Stat::Accept1A(const Entry &ent) 
{
// keep trying until accepted anyway
while (1) {

  // 1: memory available, accepted
  if (MemUsed<=MemSize) {
	CurrDataCnt+=ent.n;
	Ranges+=ent.sx;		// valid only for Stats, not for OStats
	OldRoot->AdjustTree(this,ent);
	OldRoot=NewRoot;
	return;
	}

  // 2: buffer splits: accepted
  if (SplitBuffer!=NULL && !SplitBuffer->Full()) {
	CurrDataCnt+=ent.n;
	Ranges+=ent.sx;		// valid only for Stats, not for OStats
	if (OldRoot->AbsorbEntry2(this,ent)==FALSE) 
		SplitBuffer->AddEnt(ent);
	return;	
	}

  // 3: memory out and buffer full: 
  //    increase threshold, throw outliers, rebuild tree, 
  //    re-try to see if accepted

//cout<<"#"<<name<<" "<<Phase<<" "<<Passi<<" "<<MemUsed<<" "
//    <<CurrDataCnt<<" "<<CurrEntryCnt<<" "<<sqrt(CurFt)<<endl;

  RebuiltTree1A(1);

//cout<<"#"<<name<<" "<<Phase<<" "<<Passi<<" "<<MemUsed<<" "
//    <<CurrDataCnt<<" "<<CurrEntryCnt<<" "<<sqrt(CurFt)<<endl;

  if (SplitBuffer!=NULL) ScanSplitBuffer();
  if (OutlierQueue!=NULL && OutlierQueue->Full()) ScanOutlierQueue();
  }
}

void Stat::Accept1B(const Entry &ent) 
{
// keep trying until accepted anyway
while (1) {

  // 1: memory available, accepted
  if (MemUsed<=MemSize) {
	CurrDataCnt+=ent.n;
	Ranges+=ent.sx; 	// valid only for Stats, not for OStats
	OldRoot->AdjustTree(this,ent);
	OldRoot=NewRoot;
	return;
	}

  // 2: buffer splits: accepted
  if (SplitBuffer!=NULL && !SplitBuffer->Full()) {
	CurrDataCnt+=ent.n;
	Ranges+=ent.sx;		// valid only for Stats, not for OStats
	if (OldRoot->AbsorbEntry2(this,ent)==FALSE) 
		SplitBuffer->AddEnt(ent);
	return;	
	}

  // 3: memory out and buffer full: 
  //    increase threshold, throw outliers, rebuild tree, 
  //    re-try to see if accepted

//cout<<"#"<<name<<" "<<Phase<<" "<<Passi<<" "<<MemUsed<<" "
  //  <<CurrDataCnt<<" "<<CurrEntryCnt<<" "<<sqrt(CurFt)<<endl;

  RebuiltTree1B(1);

//cout<<"#"<<name<<" "<<Phase<<" "<<Passi<<" "<<MemUsed<<" "
 //   <<CurrDataCnt<<" "<<CurrEntryCnt<<" "<<sqrt(CurFt)<<endl;

  if (SplitBuffer!=NULL) ScanSplitBuffer();
  if (OutlierQueue!=NULL && OutlierQueue->Full()) ScanOutlierQueue();
  }
}

// TZ: work here
void Stat::SelectInitFt1() 
{ if (InitFt<=0.0) CurFt=0.0; else CurFt=InitFt*InitFt; }

void Stat::SelectFtB()
{
   if (CurFt==0.0)
       	CurFt=pow(AvgDNNScanLeafEntry(BDtype),2.0);
   else CurFt=MaxOne(CurFt,pow(AvgDNNScanLeafEntry(BDtype),2.0));
}

void Stat::SelectFtA()
{
   if (CurFt==0.0) 
       	CurFt=pow(AvgDNNScanLeafEntry(BDtype),2.0);
   else CurFt=MaxOne(CurFt,pow(AvgDNNScanLeafEntry(BDtype),2.0));
}

void Stat::RebuiltTree1B(short inc_flag)
{
	AvgDensity=1.0*NewRoot->N()/(1.0*CurrEntryCnt);

        if (inc_flag==1 && Passi%StayTimes==0) SelectFtB(); 	

	Passi++;
	switch (RebuiltAlg) {
 	 	 case 0: ScanLeaf1A(); break;
	 	 case 1: CompactTree1A(); break;
	 	 case 2: ShiftTree1A(); break;
	 	 }
}

void Stat::RebuiltTree1A(short inc_flag)
{
	AvgDensity=1.0*NewRoot->N()/(1.0*CurrEntryCnt);

        if (inc_flag==1 && Passi%StayTimes==0) SelectFtA(); 	

	Passi++;
	switch (RebuiltAlg) {
 	 	 case 0: ScanLeaf1A(); break;
	 	 case 1: CompactTree1A(); break;
	 	 case 2: ShiftTree1A(); break;
	 	 }
}

// shift the tree:
void Stat::ShiftTree1A()
{
int i;

Entry ent;
ent.Init(Dimension);

Node *tmpnode;

MakeNewTree();

int height=OldRoot->Depth();

Path CurrPath(height), BestPath(height);

// initialize CurrPath to the leftmost path (leaf entry) in old tree

tmpnode=OldRoot;
for (i=0; i<height; i++) {
	CurrPath.Push(0,tmpnode);
	tmpnode=tmpnode->TheChild(0);
	}

tmpnode=CurrPath.TopLeaf();

while (tmpnode!=NULL) {

  // Process all entries in the leaf node
  for (i=0; i<tmpnode->actsize; i++) {
      ent=tmpnode->entry[i];
      if (strcmp(name,"outlier")!=0 && 
	  ent.n<NoiseRate*AvgDensity &&
	  OutlierQueue!=NULL) // write out all qualified outliers
    		OutlierQueue->AddEnt(ent,this);
      else {
          // find BestPath for current entry in new tree 
	  BestPath.Reset();
	  if (NewRoot->BestFitPath2(this,ent,BestPath)==TRUE)
	  	BestPath.AddonPath(this,ent,NewRoot);
	  else  CurrPath.AddonLeaf(this,ent,NewRoot);
	  }
      }

  // Process next leaf node
  tmpnode=CurrPath.NextRightLeafFreeSpace(this);
  if (tmpnode!=NULL) CurrPath.InsertLeaf(this,NewRoot);
  }

OldRoot=NewRoot;
OldLeafHead=NewLeafHead;
NewRoot->FreeEmptyNode(this);
}

// compact the tree:
void Stat::CompactTree1A()
{
int i;

Entry ent;
ent.Init(Dimension);

MarkNewTree();

int height = OldRoot->Depth();

Path CurrPath(height), BestPath(height);

// initialize to the leftmost path (or leaf entry) in the tree

Node *tmpnode=OldRoot;
for (i=0; i<height; i++) {
	CurrPath.Push(0,tmpnode);
	tmpnode=tmpnode->TheChild(0);
	}

while (CurrPath.Exists()) {

	// takeoff current path (or leaf entry) from the tree
	ent=*(CurrPath.TopLeafEntry());
	CurrPath.TakeoffPath(ent);

	if (strcmp(name,"outlier")!=0 && 
	    ent.n<NoiseRate*AvgDensity &&
	    OutlierQueue!=NULL) { // write out all qualified outliers
	    OutlierQueue->AddEnt(ent,this);
	    CurrPath.CollectSpace(this);
	    }
	else {// find bestpath for current leaf entry in tree and put back
            BestPath.Reset();
	    if (OldRoot->BestFitPath2(this,ent,BestPath)==TRUE 
		&& BestPath<CurrPath) {
	   		BestPath.AddonPath(this,ent,OldRoot);
	   		CurrPath.CollectSpace(this);
	   		}
	        else { CurrPath.AddonPath(this,ent,OldRoot);
		       CurrEntryCnt++;
		       CurrPath.NextRightPath();
		       }
	    }
	}
}

// responsible for old leaves
// does not guarantee S2<=S1 if T2>=T1.
void Stat::ScanLeaf1A()
{
int k = 0;

Entry ent;
ent.Init(Dimension);

short res=TRUE;

StartNewTree();

while (res!=FALSE) {
     res = NextEntryFreeOldLeafHead(k,ent); 
     if (res==TRUE) {
        if (strcmp(name,"outlier")!=0 && 
	    ent.n<NoiseRate*AvgDensity &&
	    OutlierQueue!=NULL) // write out all qualified outliers
		OutlierQueue->AddEnt(ent,this);
	else {
		OldRoot->AdjustTree(this,ent);
		OldRoot = NewRoot;
		}
	}
      } 
}

void Stat::ScanSplitBuffer()
{
Entry ent;
ent.Init(Dimension);

int count=SplitBuffer->CountEntry();

while (count>0 && MemUsed<=MemSize) {
	SplitBuffer->DeleteEnt(ent);
	count--;
	OldRoot->AdjustTree(this,ent);
	OldRoot=NewRoot;
	}

while (count>0) {
	SplitBuffer->DeleteEnt(ent);
	count--;
        if (OldRoot->AbsorbEntry2(this,ent)==FALSE)
	   if (OutlierQueue!=NULL) 
		OutlierQueue->AddEnt(ent,this); 
	   else SplitBuffer->AddEnt(ent);
	}
}

void Stat::ScanOutlierQueue()
{
Entry ent;
ent.Init(Dimension);

int count=OutlierQueue->CountEntry();

// without secondary tree for outliers
if (OStats==NULL) { 
      while (count>0) {
	OutlierQueue->DeleteEnt(ent); 
	count--;
	if (OldRoot->AbsorbEntry1(this,ent)==FALSE)
		OutlierQueue->AddEnt(ent,this); 
	}
      }

// with secondary tree for outliers
else { 
     // if can't absorb by main tree, accept to outlier tree
     while (count>0) {
	OutlierQueue->DeleteEnt(ent); 
	count--;
	if (OldRoot->AbsorbEntry1(this,ent)==FALSE) {
		switch (OStats->Phase1Scheme) {
		  case 0: OStats->Accept1A(ent); break;
		  case 1: OStats->Accept1B(ent); break;
		  default: print_error("ScanOutlierQueue","Invalid Phase1Scheme"); break;
		  }
		NoiseCnt+=ent.n;
		}
	}
     }
}

void Stat::Inherit(const Stat *Stats) {
	Dimension=Stats->Dimension;
	PageSize=Stats->PageSize;
	MemSize=Stats->OutlierTreeSize;
	BufferSize=0;
	QueueSize=0;
	OutlierTreeSize=0;
	BDtype=Stats->BDtype;
	Ftype=Stats->Ftype;
	Phase1Scheme=Stats->Phase1Scheme;
	RebuiltAlg=Stats->RebuiltAlg;
	StayTimes=Stats->StayTimes;
	NoiseRate=Stats->NoiseRate;
	Range=Stats->Range;
	CFDistr=Stats->CFDistr;
	H=Stats->H;
	K=Stats->K;
	InitFt=Stats->InitFt;
	Ft=Stats->Ft;
	Gtype=Stats->Gtype;
	GDtype=Stats->GDtype;
	Qtype=Stats->Qtype;
	RefineAlg=Stats->RefineAlg;
	NoiseFlag=Stats->NoiseFlag;
	MaxRPass=Stats->MaxRPass;
	Ranges.Init(Dimension);
	}

istream& operator>>(istream &fi,Stat *Stats) {
fi>>Stats->WMflag;
Stats->W.Init(Stats->Dimension);
fi>>Stats->W;
Stats->M.Init(Stats->Dimension);
fi>>Stats->M;

fi>>Stats->PageSize;
Stats->MemSize/=Stats->PageSize;
Stats->BufferSize/=Stats->PageSize;
Stats->QueueSize/=Stats->PageSize;
Stats->OutlierTreeSize/=Stats->PageSize;

fi>>Stats->BDtype;
fi>>Stats->Ftype;
fi>>Stats->Phase1Scheme;
fi>>Stats->RebuiltAlg;
fi>>Stats->StayTimes;

fi>>Stats->NoiseRate;

fi>>Stats->Range;

fi>>Stats->CFDistr;
fi>>Stats->H;

Stats->Bars=new int[Stats->Dimension];
for (int i=0;i<Stats->Dimension;i++) 
	fi>>Stats->Bars[i];

fi>>Stats->K;
fi>>Stats->InitFt;
fi>>Stats->Ft;
fi>>Stats->Gtype;
fi>>Stats->GDtype;
fi>>Stats->Qtype;
fi>>Stats->RefineAlg;
fi>>Stats->NoiseFlag;
fi>>Stats->MaxRPass;

Stats->Ranges.Init(Stats->Dimension);

if (Stats->BufferSize>0) 
	Stats->SplitBuffer=new BufferClass(Stats);
if (Stats->QueueSize>0) Stats->
	OutlierQueue=new RecyQueueClass(Stats);
if (Stats->OutlierTreeSize>0) {
	Stats->OStats=new Stat("outlier"); 
	Stats->OStats->Inherit(Stats);
	}

return fi;
}

ifstream& operator>>(ifstream &fi,Stat *Stats) {
fi>>Stats->WMflag;
Stats->W.Init(Stats->Dimension);
fi>>Stats->W;
Stats->M.Init(Stats->Dimension);
fi>>Stats->M;

fi>>Stats->PageSize;
Stats->MemSize/=Stats->PageSize;
Stats->BufferSize/=Stats->PageSize;
Stats->QueueSize/=Stats->PageSize;
Stats->OutlierTreeSize/=Stats->PageSize;

fi>>Stats->BDtype;
fi>>Stats->Ftype;
fi>>Stats->Phase1Scheme;
fi>>Stats->RebuiltAlg;
fi>>Stats->StayTimes;

fi>>Stats->NoiseRate;

fi>>Stats->Range;

fi>>Stats->CFDistr;
fi>>Stats->H;

Stats->Bars=new int[Stats->Dimension];
for (int i=0;i<Stats->Dimension;i++) 
	fi>>Stats->Bars[i];

fi>>Stats->K;
fi>>Stats->InitFt;
fi>>Stats->Ft;
fi>>Stats->Gtype;
fi>>Stats->GDtype;
fi>>Stats->Qtype;
fi>>Stats->RefineAlg;
fi>>Stats->NoiseFlag;
fi>>Stats->MaxRPass;

Stats->Ranges.Init(Stats->Dimension);

if (Stats->BufferSize>0) 
	Stats->SplitBuffer=new BufferClass(Stats);
if (Stats->QueueSize>0) 
	Stats->OutlierQueue=new RecyQueueClass(Stats);
if (Stats->OutlierTreeSize>0) {
	Stats->OStats=new Stat("outlier"); 
	Stats->OStats->Inherit(Stats);
	}

return fi;
}

ostream& operator<<(ostream &fo,Stat** Stats) {
for (int i=0; i<Paras->ntrees; i++)
	fo<<Stats[i]<<endl;
return fo;
}

ofstream& operator<<(ofstream &fo,Stat** Stats) {
for (int i=0; i<Paras->ntrees; i++)
	fo<<Stats[i]->name<<endl;
return fo;
}

ostream& operator<<(ostream &fo,Stat* Stats) {
fo<<"***************Status of "<<Stats->name<<endl;
if (strcmp(Stats->name,"outlier")!=0) {
fo<<"WMflag\t"<<Stats->WMflag<<endl;
fo<<"W\t"<<Stats->W<<endl;
fo<<"M\t"<<Stats->M<<endl;
}
fo<<"Dimension\t"<<Stats->Dimension<<endl;
fo<<"PageSize\t"<<Stats->PageSize<<endl;
fo<<"MemSize\t"<<Stats->MemSize<<endl;
fo<<"BufferSize\t"<<Stats->BufferSize<<endl;
fo<<"QueueSize\t"<<Stats->QueueSize<<endl;
fo<<"OutlierTreeSize\t"<<Stats->OutlierTreeSize<<endl;

fo<<"BDtype\t"<<Stats->BDtype<<endl;
fo<<"Ftype\t"<<Stats->Ftype<<endl;
fo<<"Phase1Scheme\t"<<Stats->Phase1Scheme<<endl;
fo<<"RebuiltAlg\t"<<Stats->RebuiltAlg<<endl;
fo<<"StayTimes\t"<<Stats->StayTimes<<endl;

fo<<"NoiseRate\t"<<Stats->NoiseRate<<endl;

fo<<"Range\t"<<Stats->Range<<endl;

fo<<"CFDistr\t"<<Stats->CFDistr<<endl;
fo<<"H\t"<<Stats->H<<endl;

if (Stats->Bars!=NULL) {
	fo<<"Bars\t";
	for (short i=0;i<Stats->Dimension;i++) 
		fo<<Stats->Bars[i]<<"\t";
	fo<<endl;
	}

fo<<"K\t"<<Stats->K<<endl;
fo<<"InitFt\t"<<Stats->InitFt<<endl;
fo<<"Ft\t"<<Stats->Ft<<endl;
fo<<"Gtype\t"<<Stats->Gtype<<endl;
fo<<"GDtype\t"<<Stats->GDtype<<endl;
fo<<"Qtype\t"<<Stats->Qtype<<endl;
fo<<"RefineAlg\t"<<Stats->RefineAlg<<endl;
fo<<"NoiseFlag\t"<<Stats->NoiseFlag<<endl;
fo<<"MaxRPass\t"<<Stats->MaxRPass<<endl;

fo<<"Phase\t"<<Stats->Phase<<endl;
fo<<"Pass\t"<<Stats->Passi<<endl;
fo<<"CurFt\t"<<sqrt(Stats->CurFt)<<endl;
fo<<"MemUsed \t"<<Stats->MemUsed<<endl;
fo<<"TreeSize\t"<<Stats->TreeSize<<endl;

fo<<"PrevEntryCnt\t"<<Stats->PrevEntryCnt<<endl;
fo<<"CurrEntryCnt\t"<<Stats->CurrEntryCnt<<endl;
fo<<"PrevDataCnt\t"<<Stats->PrevDataCnt<<endl;
fo<<"CurrDataCnt\t"<<Stats->CurrDataCnt<<endl;

fo<<"AvgDensity\t"<<Stats->AvgDensity<<endl;
fo<<"NoiseCnt\t"<<Stats->NoiseCnt<<endl;

fo<<"Ranges\t"<<Stats->Ranges<<endl;

if (Stats->Phase==1 || Stats->Phase==2) 
	if (Stats->OldRoot) Stats->OldRoot->Print_Summary(Stats,fo);

if (Stats->SplitBuffer!=NULL)
fo<<"SplitBuffer\n"<<Stats->SplitBuffer<<endl;
if (Stats->OutlierQueue!=NULL)
fo<<"OutlierQueue\n"<<Stats->OutlierQueue<<endl;
if (Stats->OStats!=NULL)
fo<<"OutlierTree Status\n"<<Stats->OStats<<endl;

fo<<"OutlierEntryCnt\t"<<Stats->OutlierEntryCnt<<endl;
fo<<"OutlierTupleCnt\t"<<Stats->OutlierTupleCnt<<endl;

return fo;
}

ofstream& operator<<(ofstream &fo,Stat* Stats) {
fo<<"***************Status of "<<Stats->name<<endl;
if (strcmp(Stats->name,"outlier")!=0) {
fo<<"WMflag\t"<<Stats->WMflag<<endl;
fo<<"W\t"<<Stats->W<<endl;
fo<<"M\t"<<Stats->M<<endl;
}
fo<<"Dimension\t"<<Stats->Dimension<<endl;
fo<<"PageSize\t"<<Stats->PageSize<<endl;
fo<<"MemSize\t"<<Stats->MemSize<<endl;
fo<<"BufferSize\t"<<Stats->BufferSize<<endl;
fo<<"QueueSize\t"<<Stats->QueueSize<<endl;
fo<<"OutlierTreeSize\t"<<Stats->OutlierTreeSize<<endl;

fo<<"BDtype\t"<<Stats->BDtype<<endl;
fo<<"Ftype\t"<<Stats->Ftype<<endl;
fo<<"Phase1Scheme\t"<<Stats->Phase1Scheme<<endl;
fo<<"RebuiltAlg\t"<<Stats->RebuiltAlg<<endl;
fo<<"StayTimes\t"<<Stats->StayTimes<<endl;

fo<<"NoiseRate\t"<<Stats->NoiseRate<<endl;

fo<<"Range\t"<<Stats->Range<<endl;

fo<<"CFDistr\t"<<Stats->CFDistr<<endl;
fo<<"H\t"<<Stats->H<<endl;

if (Stats->Bars!=NULL) {
	fo<<"Bars\t";
	for (short i=0;i<Stats->Dimension;i++) 
		fo<<Stats->Bars[i]<<"\t";
	fo<<endl;
	}

fo<<"K\t"<<Stats->K<<endl;
fo<<"InitFt\t"<<Stats->InitFt<<endl;
fo<<"Ft\t"<<Stats->Ft<<endl;
fo<<"Gtype\t"<<Stats->Gtype<<endl;
fo<<"GDtype\t"<<Stats->GDtype<<endl;
fo<<"Qtype\t"<<Stats->Qtype<<endl;
fo<<"RefineAlg\t"<<Stats->RefineAlg<<endl;
fo<<"NoiseFlag\t"<<Stats->NoiseFlag<<endl;
fo<<"MaxRPass\t"<<Stats->MaxRPass<<endl;

fo<<"Phase\t"<<Stats->Phase<<endl;
fo<<"Pass\t"<<Stats->Passi<<endl;
fo<<"CurFt\t"<<sqrt(Stats->CurFt)<<endl;
fo<<"MemUsed \t"<<Stats->MemUsed<<endl;
fo<<"TreeSize\t"<<Stats->TreeSize<<endl;

fo<<"PrevEntryCnt\t"<<Stats->PrevEntryCnt<<endl;
fo<<"CurrEntryCnt\t"<<Stats->CurrEntryCnt<<endl;
fo<<"PrevDataCnt\t"<<Stats->PrevDataCnt<<endl;
fo<<"CurrDataCnt\t"<<Stats->CurrDataCnt<<endl;

fo<<"AvgDensity\t"<<Stats->AvgDensity<<endl;
fo<<"NoiseCnt\t"<<Stats->NoiseCnt<<endl;

fo<<"Ranges\t"<<Stats->Ranges<<endl;

if (Stats->Phase==1 || Stats->Phase==2) 
	if (Stats->OldRoot) Stats->OldRoot->Print_Summary(Stats,fo);

if (Stats->SplitBuffer!=NULL)
fo<<"SplitBuffer\n"<<Stats->SplitBuffer<<endl;
if (Stats->OutlierQueue!=NULL)
fo<<"OutlierQueue\n"<<Stats->OutlierQueue<<endl;
if (Stats->OStats!=NULL)
fo<<"OutlierTree Status\n"<<Stats->OStats<<endl;

fo<<"OutlierEntryCnt\t"<<Stats->OutlierEntryCnt<<endl;
fo<<"OutlierTupleCnt\t"<<Stats->OutlierTupleCnt<<endl;

return fo;
}

// sqrt_ed:

double Stat::AvgRDScanRoot() {
Entry tmpent;
tmpent.Init(Dimension);
OldRoot->CF(tmpent);
return sqrt(tmpent.Fitness(Ftype));
}

double Stat::AbsVScanLeafEntry() {
int    i;
Leaf*  tmp=NewLeafHead;
double abs_v=0.0;
while (tmp!=NULL) {
	for (i=0;i<tmp->actsize;i++) 
       	   abs_v+=pow(sqrt(tmp->entry[i].Fitness(Ftype)),Dimension);
	tmp=tmp->next;
	}
return abs_v;
}

double Stat::AbsVScanLeafNode() {
Leaf*	tmp=NewLeafHead;
double  abs_v=0.0;
while (tmp!=NULL) {
	abs_v+=pow(sqrt(tmp->Fitness(Ftype)),Dimension);
	tmp=tmp->next;
	}
return abs_v;
}

double Stat::AvgDNNScanLeafEntry(short dtype) {
Leaf*   tmp=NewLeafHead;
int 	i,j,total_n=0;
double  *dmin;
double  d,total_d=0.0;

while (tmp!=NULL) {
  if (tmp->actsize>1) {
    dmin=new double[tmp->actsize];
    for (i=0; i<tmp->actsize; i++) dmin[i]=HUGE_DOUBLE;
    total_n+=tmp->actsize;
    for (i=0; i<tmp->actsize-1; i++)
      for (j=i+1; j<tmp->actsize; j++) {
	   d=distance(dtype,tmp->entry[i],tmp->entry[j]);
	   if (d>=0) d=sqrt(d); else d=0.0;
	   if (d<dmin[i]) dmin[i]=d;
	   if (d<dmin[j]) dmin[j]=d;
	   }	
    for (i=0; i<tmp->actsize; i++) total_d+=dmin[i];
    delete [] dmin;
    }
  tmp=tmp->next;
  }
return total_d/total_n; 
}

double Stat::FtSurvey1()
{
int i,j;

// approximate crowdest leaf node
Node *node = OldRoot->DenseNode(); 

if (node==NULL) 
   print_error("FtSurvey1","crowded place wrong");

// merge the closest pair
return sqrt(node->ClosestDiffTwo(this,i,j));
}

double Stat::FtSurvey2()
{
Entry tmpent;
tmpent.Init(Dimension);

// approximate crowdest leaf node
Node *node = OldRoot->DenseNode(); 

if (node==NULL) 
   print_error("FtSurvey2","crowded place wrong");

// merge the whole leaf node 
node->CF(tmpent);
return sqrt(tmpent.Fitness(Ftype));
}

// more general in terms of distortion:
// distortpercent=0% ==> FtSurvey1
// distortpercent=100% ==> FtSurvey2
double Stat::FtSurvey3(double distortpercent)
{
int i,j;
double oldV, newV, origin;

// approximate crowdest leaf node 
Node *node = OldRoot->DenseNode(); 

if (node==NULL) 
   print_error("FtSurvey3","crowded place is wrong");

if (node->actsize==1)
   print_error("FtSurvey3","only one leaf entry exists in crowded place");

origin=0;
for (i=0;i<node->actsize;i++) 
	origin+=node->entry[i].n*node->entry[i].Radius();

// merge to form a natural cluster hierarchy in the leaf node
Hierarchy *h = new Hierarchy(node->actsize-1, Dimension);
h->MergeHierarchy(GDtype, node->actsize,node->Entries());

// split on the hierarchy to reach distortpercent
oldV = h->DistortionEdge(node->Entries());
do {  newV = h->DistortionEdge(node->Entries());
      if ((newV-origin)/(oldV-origin)<=distortpercent||
	  h->chainptr==node->actsize-2) 
	  break;
     } while (h->SplitHierarchy(BY_KCLUSTERS,Ftype,Ft)==TRUE); 

// choose the cluster with MinFt along the split Edge
double FutFt=h->MinFtMerged(Ftype);

// replace entries in the node by newentries after physically merging the cluster
int leafsize = node->MaxSize(this);
Entry *newentry = new Entry[leafsize];
for (i=0; i<leafsize; i++) newentry[i].Init(Dimension);

j=0; newentry[0]=h->cf[-h->chain[0]-1];
// find the leaf entries forming that cluster and label them
while (h->SplitHierarchy(BY_KCLUSTERS,Ftype,Ft)==TRUE);
for (i=0; i<=h->chainptr; i++) {
	if (h->chain[i]>0) {
		node->entry[h->chain[i]-1].n=0;
		}
	}

for (i=0; i<node->actsize; i++) {
	if (node->entry[i].n>0) {
		newentry[++j]=node->entry[i];
		}
	}
delete [] node->entry;
node->actsize = j+1;
node->entry=newentry;
return sqrt(FutFt);
}

// Scan Leaf Nodes:
// AvgDensity 

void Stat::MakeNewTree() {

PrevEntryCnt=CurrEntryCnt;
PrevDataCnt=CurrDataCnt;

CurrEntryCnt=0;

// CurrDataCnt keep going up unless specified

TreeSize=OldRoot->Depth();
MemUsed+=TreeSize;
Node *tmpnode;

// initialize new tree
if (TreeSize==1) {
   tmpnode=new Leaf(this);
   tmpnode->actsize=0;
   NewRoot=tmpnode;
   NewLeafHead=(Leaf*)tmpnode;
   OldLeafHead=(Leaf*)tmpnode;
}
else {
   tmpnode=new Nonleaf(this);
   tmpnode->actsize=1;
   NewRoot=tmpnode;
   for (int i=0; i<TreeSize-2; i++) {
	tmpnode->NewNonleafChildI(this,0);
	tmpnode=tmpnode->TheChild(0);
	tmpnode->actsize=1;
	}
   tmpnode->NewLeafChildI(this,0);
   tmpnode=tmpnode->TheChild(0);
   tmpnode->actsize= 0;
   NewLeafHead=(Leaf*)tmpnode;
   OldLeafHead=(Leaf*)tmpnode;
   }
}

void Stat::MarkNewTree() {

PrevEntryCnt=CurrEntryCnt;
PrevDataCnt=CurrDataCnt;

CurrEntryCnt = 0;
// CurrDataCnt keep going up unless specified

OldLeafHead = NewLeafHead;
}

void Stat::StartNewTree() {

PrevEntryCnt=CurrEntryCnt;
PrevDataCnt=CurrDataCnt;

CurrEntryCnt = 0;
// CurrDataCnt keep going up unless specified

if (OldRoot) OldRoot->free_nonleaf(this);
OldRoot = new Leaf(this);
NewRoot = OldRoot;
MemUsed++;
TreeSize = 1;
OldLeafHead = NewLeafHead;
NewLeafHead = (Leaf *)OldRoot;
}

void Stat::CreateNewRoot(Node *child1, Node *child2) {
NewRoot=new Nonleaf(this); MemUsed++; TreeSize++;
NewRoot->AssignActSize(2);
NewRoot->AssignChild(0,child1);
child1->CF(*(NewRoot->TheEntry(0)));
NewRoot->AssignChild(1,child2);
child2->CF(*(NewRoot->TheEntry(1)));
}

int Stat::EntrySize() const {
#ifdef RECTANGLE
return sizeof(int)+sizeof(double)*(3*Dimension+1);
#else
return sizeof(int)+sizeof(double)*(Dimension+1);
#endif
}

// shift the tree:
void Stat::ShiftTree2()
{
int i;

Entry ent;
ent.Init(Dimension);

Node *tmpnode;

MakeNewTree();

int height = OldRoot->Depth();

Path CurrPath(height), BestPath(height);

// initialize CurrPath to the leftmost path (leaf entry) in old tree
tmpnode=OldRoot;
for (i=0; i<height; i++) {
	CurrPath.Push(0,tmpnode);
	tmpnode=tmpnode->TheChild(0);
	}

tmpnode=CurrPath.TopLeaf();

while (tmpnode!=NULL) {

  // Process all entries in the leaf node
  for (i=0; i<tmpnode->actsize; i++) {
      ent=tmpnode->entry[i];
      // find BestPath for current entry in new tree 
      BestPath.Reset();
      if (NewRoot->BestFitPath2(this,ent,BestPath)==TRUE)
 	 	BestPath.AddonPath(this,ent,NewRoot);
      else  CurrPath.AddonLeaf(this,ent,NewRoot);
      }

   // Process next leaf node
   tmpnode=CurrPath.NextRightLeafFreeSpace(this);
   if (tmpnode!=NULL) CurrPath.InsertLeaf(this,NewRoot);
   }

OldRoot=NewRoot;
OldLeafHead=NewLeafHead;
NewRoot->FreeEmptyNode(this);
}

// compact the tree:
void Stat::CompactTree2()
{
int i;

Entry ent;
ent.Init(Dimension);

MarkNewTree();

int height = OldRoot->Depth();

Path CurrPath(height), BestPath(height);

// initialize to the leftmost path (or leaf entry) in the tree
Node *tmpnode=OldRoot;
for (i=0; i<height; i++) {
	CurrPath.Push(0,tmpnode);
	tmpnode=tmpnode->TheChild(0);
	}

while (CurrPath.Exists()) {

	// takeoff current path (or leaf entry) from the tree
	ent=*(CurrPath.TopLeafEntry());
	CurrPath.TakeoffPath(ent);

	// find bestpath for current leaf entry in tree and put back
	BestPath.Reset();
	if (OldRoot->BestFitPath2(this,ent,BestPath)==TRUE
	    && BestPath<CurrPath) {
		BestPath.AddonPath(this,ent,OldRoot);
	   	CurrPath.CollectSpace(this);
	   	}
	else {  CurrPath.AddonPath(this,ent,OldRoot);
		CurrEntryCnt++;
	   	CurrPath.NextRightPath();
	   	}
	}
}

// responsible for old leaves
void Stat::ScanLeaf2()
{
int k = 0;

Entry ent;
ent.Init(Dimension);

short res=TRUE;

StartNewTree();

while (res!=FALSE) {
     res = NextEntryFreeOldLeafHead(k,ent); 
     if (res==TRUE) {
		OldRoot->AdjustTree(this,ent);
		OldRoot = NewRoot;
		}
      } 
}

void Stat::Accept2(const Entry &ent) 
{
// keep trying until accepted anyway

while (1) {

  // 1: within range, accepted
  if (CurrEntryCnt<=Range) {
	CurrDataCnt+=ent.n;
	OldRoot->AdjustTree(this,ent);
	OldRoot = NewRoot;
	return;
	}

  // 2: out of range, increase threshold, rebuild tree 
//cout<<"#"<<name<<" "<<Phase<<" "<<Passi<<" "<<MemUsed<<" "
//    <<CurrDataCnt<<" "<<CurrEntryCnt<<" "<<sqrt(CurFt)<<endl;

  RebuiltTree2(1);

//cout<<"#"<<name<<" "<<Phase<<" "<<Passi<<" "<<MemUsed<<" "
//    <<CurrDataCnt<<" "<<CurrEntryCnt<<" "<<sqrt(CurFt)<<endl;
  }
}

void Stat::RebuiltTree2(short inc_flag)
{
if (inc_flag==1 && (Passi+1)%StayTimes==0) SelectFtA(); 	

Passi++;
switch (RebuiltAlg) {
 case 0: ScanLeaf2(); break;
 case 1: CompactTree2(); break;
 case 2: ShiftTree2(); break;
 }
}

void Stat::SelectInitFt2()
{

CurFt=MaxOne(CurFt,pow(AbsVScanLeafEntry()/Range,2.0/Dimension));

/*
ConNode *contree;
contree=NewRoot->Copy(this);
contree->Connect(this);
CurFt=MaxOne(CurFt,pow(contree->CoverUpLeafNodes(Range*1.0/CurrEntryCnt),2.0));
contree->Free();
*/
}

short Stat::NextEntryFromLeafHead(int &pos, Entry &ent, Leaf **tmp) {
while ((*tmp)!=NULL && pos>=(*tmp)->actsize) {
        pos=0;
        (*tmp)=(*tmp)->NextLeaf();
        }
if ((*tmp)==NULL) return FALSE;
ent=*((*tmp)->TheEntry(pos));
pos++;
if (pos==(*tmp)->actsize) {
        pos=0;
        *tmp=(*tmp)->NextLeaf();
        }
return TRUE;
}

short Stat::NextEntryFreeOldLeafHead(int &pos, Entry &ent) {
Leaf *tmp;
while (OldLeafHead!=NULL && pos>=OldLeafHead->actsize) {
        pos = 0;
        tmp = OldLeafHead;
        OldLeafHead = OldLeafHead->NextLeaf();
        delete tmp; MemUsed--;
        }

if (OldLeafHead==NULL) return FALSE;

ent=*(OldLeafHead->TheEntry(pos));
pos++;
if (pos==OldLeafHead->actsize) {
        pos=0;
        tmp = OldLeafHead; 
        OldLeafHead = OldLeafHead->NextLeaf();
        delete tmp; MemUsed--;
        }
return TRUE;
}

short Stat::NextEntryFreeRestLeafPtr(int &pos, Entry &ent) {
Leaf *tmp;
while (RestLeafPtr!=NULL && pos>=RestLeafPtr->actsize) {
        pos = 0;
        tmp = RestLeafPtr;
        RestLeafPtr = RestLeafPtr->NextLeaf();
        delete tmp; MemUsed--;
        }

if (RestLeafPtr==NULL) return FALSE;

ent=*(RestLeafPtr->TheEntry(pos));
pos++;
if (pos==RestLeafPtr->actsize) {
        pos=0;
        tmp = RestLeafPtr; 
        RestLeafPtr = RestLeafPtr->NextLeaf();
        delete tmp; MemUsed--;
        }
return TRUE;
}

