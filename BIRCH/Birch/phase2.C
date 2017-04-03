/****************************************************************
File Name: phase2.C  
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
#include "recyqueue.h"
#include "phase2.h"

extern Para *Paras;

void BirchPhase2(Stat **Stats) {

int   i;
short res;

Entry *entries=new Entry[Paras->ntrees];
for (i=0; i<Paras->ntrees; i++)
	entries[i].Init(Stats[i]->Dimension);

for (i=0; i<Paras->ntrees; i++) {

   if (Stats[i]->CurrEntryCnt>Stats[i]->Range) {

	Stats[i]->Phase = 2;
	Stats[i]->Passi = 0;
	Stats[i]->RestLeafPtr=Stats[i]->NewLeafHead;

	Stats[i]->AvgDensity=(1.0*Stats[i]->NewRoot->N())/(1.0*Stats[i]->CurrEntryCnt);

	Stats[i]->SelectInitFt2();
	Stats[i]->StartNewTree();
	Stats[i]->CurrDataCnt=0;

	// while scanning leaf entries
	res=TRUE;
        while (res!=FALSE) {
	  res=Stats[i]->NextEntryFreeRestLeafPtr(Stats[i]->RestLeafK,entries[i]);
	  if (res==TRUE) {
	     if (entries[i].n<Stats[i]->NoiseRate*Stats[i]->AvgDensity &&
		 Stats[i]->OutlierQueue!=NULL) 
		  Stats[i]->OutlierQueue->AddEnt(entries[i],Stats[i]);
	     else Stats[i]->Accept2(entries[i]);
	     }
	  }

	// after scanning leaf entries

	if (Stats[i]->OutlierQueue!=NULL) Stats[i]->ScanOutlierQueue();

cout<<"#"<<Stats[i]->name<<" "
    <<Stats[i]->Phase<<" "<<Stats[i]->Passi<<" "
    <<Stats[i]->MemUsed<<" "<<Stats[i]->CurrDataCnt<<" "
    <<Stats[i]->CurrEntryCnt<<" "<<sqrt(Stats[i]->CurFt)<<endl;

	Stats[i]->RebuiltTree2(0);

cout<<"#"<<Stats[i]->name<<" "
    <<Stats[i]->Phase<<" "<<Stats[i]->Passi<<" "
    <<Stats[i]->MemUsed<<" "<<Stats[i]->CurrDataCnt<<" "
    <<Stats[i]->CurrEntryCnt<<" "<<sqrt(Stats[i]->CurFt)<<endl;

	if (Stats[i]->OutlierQueue!=NULL) Stats[i]->ScanOutlierQueue();
	}
   }

delete [] entries;
}

