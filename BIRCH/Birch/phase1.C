/****************************************************************
File Name: phase1.C  
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
#include "buffer.h"
#include "recyqueue.h"
#include "phase1.h"
#include "AttrProj.h"
#include <sys/time.h>
extern Para *Paras;
extern double t_io_1, t_io_2, t_io;
extern struct timeval tp;

void BirchPhase1(Stat **Stats) {

int i;

RecId		recid1,recidi;
DevStatus 	dstat;

Paras->attrproj->FirstRecId(recid1);

VectorArray *vectors;

Paras->attrproj->CreateRecordList(vectors);

Entry *entries=new Entry[Paras->ntrees];
for (i=0; i<Paras->ntrees; i++)
	entries[i].Init(Stats[i]->Dimension);

// 1. while scanning data
for (recidi=recid1; ; recidi++) {
//seconds(t_io_1);
	dstat=Paras->attrproj->ReadRec(recidi,*vectors);
//seconds(t_io_2);
t_io += t_io_2 - t_io_1;
	if (dstat!=StatusOk) break;
	for (i=0; i<Paras->ntrees; i++) {
	     entries[i]=*(vectors->GetVector(i));
	     if (Stats[i]->WMflag) 
		entries[i].Transform(Stats[i]->W,Stats[i]->M);

	     switch (Stats[i]->Phase1Scheme) {
	       case 0: Stats[i]->Accept1A(entries[i]); break;
	       case 1: Stats[i]->Accept1B(entries[i]); break;
	       default: print_error("BirchPhase1", "invalid Phase1Scheme"); break;
	       }
             }
	}

// 2. after scanning data
for (i=0; i<Paras->ntrees; i++) {

        if (Stats[i]->SplitBuffer!=NULL) Stats[i]->ScanSplitBuffer();
	if (Stats[i]->OutlierQueue!=NULL) Stats[i]->ScanOutlierQueue();

cout<<"#"<<Stats[i]->name<<" "
    <<Stats[i]->Phase<<" "<<Stats[i]->Passi<<" "
    <<Stats[i]->MemUsed<<" "<<Stats[i]->CurrDataCnt<<" "
    <<Stats[i]->CurrEntryCnt<<" "<<sqrt(Stats[i]->CurFt)<<endl;

	switch (Stats[i]->Phase1Scheme) {
	       case 0: Stats[i]->RebuiltTree1A(0); break;
	       case 1: Stats[i]->RebuiltTree1B(0); break;
	       default: print_error("BirchPhase1", "invalid Phase1Scheme"); break;
	       }
cout<<"#"<<Stats[i]->name<<" "
    <<Stats[i]->Phase<<" "<<Stats[i]->Passi<<" "
    <<Stats[i]->MemUsed<<" "<<Stats[i]->CurrDataCnt<<" "
    <<Stats[i]->CurrEntryCnt<<" "<<sqrt(Stats[i]->CurFt)<<endl;
       
	if (Stats[i]->SplitBuffer!=NULL) Stats[i]->ScanSplitBuffer();
       	if (Stats[i]->OutlierQueue!=NULL) Stats[i]->ScanOutlierQueue();
	}

delete vectors;
delete [] entries;
}

