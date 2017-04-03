/****************************************************************
File Name: status.h   
Author: Tian Zhang, CS Dept., Univ. of Wisconsin-Madison, 1995

               Copyright(c) 1995 by Tian Zhang

                   All Rights Reserved

Permission to use, copy and modify this software must be granted
by the author and provided that the above copyright notice appear 
in all relevant copies and that both that copyright notice and this 
permission notice appear in all relevant supporting documentations. 

Comments and additions may be sent the author at zhang@cs.wisc.edu.

******************************************************************/

#ifndef STATUS_H
#define STATUS_H

#define NUM_BUCKETS 100

class Node;
class Leaf;
class Nonleaf;

class Para;
class BufferClass;
class RecyQueueClass;

extern Para *Paras;

// for main tree or for outlier tree

class Stat {

public:

// status of execution of one tree

// 1: static information from user input

short	Dimension;

short WMflag;   // 0 or 1
Vector W;       // weighting vecotr
Vector M;       // moving vector

int	PageSize;

int 	MemSize; 	// in pages
int 	BufferSize;     // in pages
int 	QueueSize;      // in pages
int	OutlierTreeSize;// in pages

// from experiment settings
short   BDtype;   // Phase1 and 2 distance type
short   Ftype;    // Fitness type for leaf entries
short   Phase1Scheme; // 1
short   RebuiltAlg;   // 2
int	StayTimes;    // 3 

double  NoiseRate;      // 0.25

int	Range; 		      // Large to skip phase 2

short 	CFDistr;  // uniform or normal
double 	H;        // default 0, Smoothing parameter
int 	*Bars;    // 1000, Smoothing bars

int 	K;          // number of clusters
double 	InitFt;     // Initial fitness threshold
double 	Ft;         // Ending fitness threshold
short 	Gtype;      // Global Clustering Algorithm Label
short 	GDtype;     // Phase3 distance type
short 	Qtype;      // Quality type
short 	RefineAlg;  // Refinement Algorithm
short 	NoiseFlag;  // removing noise in phase4 or not
int   	MaxRPass;   // Maximal Refine Pass

// dynamic information from running status

char    name[MAX_NAME];

short   Phase;
short 	Passi;
double	CurFt;
int	MemUsed;
int 	TreeSize;

int	PrevEntryCnt;
int 	CurrEntryCnt;

int 	PrevDataCnt;
int 	CurrDataCnt;

int 	NoiseCnt;

double  AvgDensity;

Node	*OldRoot;
Node 	*NewRoot;

Leaf	*OldLeafHead;
Leaf	*NewLeafHead;

Leaf	*RestLeafPtr;
int 	RestLeafK;

Rectangle Ranges; 

BufferClass 	*SplitBuffer;
RecyQueueClass  *OutlierQueue;
Stat		*OStats;

Entry		*Entries;

int	OutlierEntryCnt;
int 	OutlierTupleCnt;

explicit Stat(char *str);
~Stat();

void SelectInitFt1();

// for phase1: scheme A:
void Accept1A(const Entry &ent);
void ShiftTree1A();
void CompactTree1A();
void ScanLeaf1A();
void RebuiltTree1A(short inc_flag);

void SelectFtA();
void SelectFtB();

// for phase1: scheme B:
void Accept1B(const Entry &ent);
void RebuiltTree1B(short inc_flag);

void SelectInitFt2();

// for phase2
void Accept2(const Entry &ent);
void ShiftTree2();
void CompactTree2();
void ScanLeaf2();
void RebuiltTree2(short inc_flag);

// general use
short NextEntryFromLeafHead(int &pos, Entry &ent, Leaf **tmp);
short NextEntryFreeOldLeafHead(int &pos, Entry &ent);
short NextEntryFreeRestLeafPtr(int &pos, Entry &ent);

void ScanSplitBuffer();
void ScanOutlierQueue();

// be careful:: return value is not sqr_ed
double AvgRDScanRoot();
double AbsVScanLeafEntry();
double AbsVScanLeafNode();
double AvgDNNScanLeafEntry(short dtype);

// be careful: return value is not sqr_ed
double FtSurvey1();
double FtSurvey2();
double FtSurvey3(double DistortPercent);


// for ShiftTree1A
void MakeNewTree();
// for CompactTree1A
void MarkNewTree();
// for ScanLeaf1A
void StartNewTree();

// relevant to dimension and RECTANGLE option
int EntrySize() const;

void CreateNewRoot(Node* child1, Node* child2);

void Print_Tree(ofstream &fo) {fo<<NewRoot;}

// outlier tree inherits main tree properties
void Inherit(const Stat *Stats);

friend istream& operator>>(istream &fi, Stat* Stats);
friend ifstream& operator>>(ifstream &fi, Stat* Stats);
friend ostream& operator<<(ostream &fo, Stat* Stats);
friend ofstream& operator<<(ofstream &fo, Stat* Stats);

friend ostream& operator<<(ostream &fo, Stat** Stats);
friend ofstream& operator<<(ofstream &fo, Stat** Stats);
};

istream& operator>>(istream &fi, Stat* Stats);
ifstream& operator>>(ifstream &fi, Stat* Stats);
ostream& operator<<(ostream &fo, Stat* Stats);
ofstream& operator<<(ofstream &fo, Stat* Stats);

ostream& operator<<(ostream &fo, Stat** Stats);
ofstream& operator<<(ofstream &fo, Stat** Stats);

#endif STATUS_H

