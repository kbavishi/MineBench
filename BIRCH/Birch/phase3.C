/****************************************************************
File Name: phase3.C  
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
#include "hierarchy.h"
#include "clarans.h"
#include "lloyd.h"
#include "kmeans.h"
#include "phase3.h"

void BirchPhase3(Stat **Stats)
{

char     tmpname[MAX_NAME];
char     tmpname1[MAX_NAME];
ofstream tmpfile;
ofstream tmpfile1;

int      i,j,k;

Entry    *tmpents = new Entry[Paras->ntrees]; 
for (i=0; i<Paras->ntrees; i++) 
	tmpents[i].Init(Stats[i]->Dimension);

for (i=0; i<Paras->ntrees; i++) {

	sprintf(tmpname,"%s-cluster",Stats[i]->name);
	sprintf(tmpname1,"%s-tmp-cluster",Stats[i]->name);
	tmpfile.open(tmpname);
	tmpfile1.open(tmpname1);

        Stats[i]->Phase=3;
        Stats[i]->NewRoot->free_nonleaf(Stats[i]);
	Stats[i]->NewRoot=NULL;

	Stats[i]->Entries=new Entry[Stats[i]->CurrEntryCnt];
	for (j=0; j<Stats[i]->CurrEntryCnt; j++)
		Stats[i]->Entries[j].Init(Stats[i]->Dimension);

	j=k=0;
        Stats[i]->OldLeafHead=Stats[i]->NewLeafHead;
        while (Stats[i]->NextEntryFreeOldLeafHead(j,tmpents[i])!=FALSE) 
			{
        	Stats[i]->Entries[k]=tmpents[i];
			tmpfile1 << tmpents[i].n << "   ";
			for (int l=0; l < tmpents[i].Dim(); l++)
				tmpfile1 << tmpents[i].sx.value[l]/tmpents[i].n << "   " ;
			tmpfile1 << tmpents[i].sxx << endl;
			k++;
			}
	tmpfile1.close();
	Stats[i]->OldLeafHead=Stats[i]->NewLeafHead=NULL;

        if (k!=Stats[i]->CurrEntryCnt) 
	    print_error(Stats[i]->name,"Entry number not matching");

        if (Stats[i]->K==0 && Stats[i]->CurFt>Stats[i]->Ft)
	   {
        	Paras->logfile<<Stats[i]->name<<":"<<"CurFt>EndFt"<<endl;
           }
        else if (Stats[i]->K!=0 && Stats[i]->K>Stats[i]->CurrEntryCnt)
	{
	Paras->logfile<<Stats[i]->name<<":"<<"K>CurEntryCnt"<<endl;
        }  
      else {
            Paras->logfile<<Stats[i]->name<<":"
			  <<"CurFt<EndFt or K<CurrEntryCnt"<<endl;
	    switch (Stats[i]->Gtype) {
		case HIERARCHY0:Hierarchy0(Stats[i]->CurrEntryCnt,Stats[i]->K,&(Stats[i]->Entries),Stats[i]->GDtype,Stats[i]->Ftype,Stats[i]->Ft);
	      			break;
		case HIERARCHY1:Hierarchy1(Stats[i]->CurrEntryCnt,Stats[i]->K,&(Stats[i]->Entries),Stats[i]->GDtype,Stats[i]->Ftype,Stats[i]->Ft);
	      			break;
	    	case CLARANS0: 	Clarans0(Stats[i]->CurrEntryCnt,Stats[i]->K,Stats[i]->Entries);
			      	break;
	    	case CLARANS1: 	Clarans1(Stats[i]->CurrEntryCnt,Stats[i]->K,Stats[i]->Entries,Stats[i]->GDtype, Stats[i]->Qtype);
	   	 	  	break;
		case LLOYD: 	Lloyd(Stats[i]->CurrEntryCnt,&(Stats[i]->Entries));
	     			break;
		case KMEANS:    Kmeans(Stats[i]->CurrEntryCnt,&(Stats[i]->Entries));
				break;
		default: print_error("BirchPhase3","Invalid global algorithm");
			 break;
		}
	     }

	Paras->logfile<<Stats[i]->name<<":"<<"Quality of Phase3 "
		      <<Quality(Stats[i]->Qtype,Stats[i]->CurrEntryCnt,Stats[i]->Entries)<<endl;

	for (j=0; j<Stats[i]->CurrEntryCnt; j++)
	tmpfile<<Stats[i]->Entries[j]<<endl;

        tmpfile.close();
	}
delete [] tmpents;
}

