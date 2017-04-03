/****************************************************************
File Name:   main.C
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
#include "timeutil.h"
#include "vector.h"
#include "rectangle.h"
#include "cfentry.h"
#include "cutil.h"
#include "parameter.h"
#include "status.h"
#include "cftree.h"
#include "components.h"
#include "phase12.h"
#include "phase3.h"
#include "phase4.h"
#include "density.h"
#include "ApInit.h"

Para *Paras;

int main(int argc,char **argv) {

int 	 i;
char     tmpname[MAX_NAME];

if (argc!=5) 
print_error("Usage:", 
	    "birch parafile schefile projfile datafile"); 

ifstream tmpfile;
tmpfile.open("tmp");
Entry* ents=new Entry[56];
for (i=0;i<56;i++) {
	ents[i].Init(2);
	tmpfile>=ents[i];
	}
Graph *g=new Graph(56,ents,1,1);
Components *comp=g->Connected_Components();
//cout << comp <<endl;
tmpfile.close();


Timer t0("Init");
Timer t1("Phase1");
Timer t2("Phase2");
Timer td("Density");
Timer t3("Phase3");
Timer t4("Phase4");

// cout << "Doing Init\n";

t0.Start();

/* devise-kind initialization */

Init::DoInit();

/* new Para includes new AttrProj */

Paras = new Para(argv[1],argv[2],argv[3],argv[4]);

/* load overall parameters: including new AttrProj */

Paras->parafile>>Paras;
//Paras->logfile<<Paras;

/* set up for ntrees */

Stat **Stats;
Stats = new Stat*[Paras->ntrees];

for (i=0; i<Paras->ntrees; i++) {

	// init for each tree
	sprintf(tmpname,"%s+%s+%s+%s-%d",argv[1],argv[2],argv[3],argv[4],i);
	Stats[i]=new Stat(tmpname);
	Stats[i]->Dimension=Paras->attrcnts[i];

	// allocate space in bytes based on dimensions or sizes for each tree
	Stats[i]->MemSize=Paras->TotalMemSize*
			  Paras->attrcnts[i]/
			  Paras->total_attrcnt;
        Stats[i]->BufferSize=Paras->TotalBufferSize*
			     Paras->attrcnts[i]/
			     Paras->total_attrcnt;
	Stats[i]->QueueSize=Paras->TotalQueueSize*
			    Paras->attrcnts[i]/
			    Paras->total_attrcnt;
	Stats[i]->OutlierTreeSize=Paras->TotalOutlierTreeSize*
				  Paras->attrcnts[i]/
				  Paras->total_attrcnt;

	// read parameters for each tree
	Paras->parafile>>Stats[i];
//	Paras->logfile<<Stats[i];

	// start each tree
	Stats[i]->SelectInitFt1();
	Stats[i]->StartNewTree();

	// start each outlier tree
	if (Stats[i]->OStats) {
		Stats[i]->OStats->SelectInitFt1();
		Stats[i]->OStats->StartNewTree();
		}
	}

t0.Stop();

// Paras->logfile<<Stats;

#ifdef LOGGING
//Paras->logfile<<"******************Init******************\n";
#endif LOGGING

#ifdef LOGGING
//Paras->logfile<<t0<<endl;
#endif LOGGING

//cout << "Doing Phase1\n";

#ifdef LOGGING
//Paras->logfile<<"******************Phase 1***************\n";
#endif LOGGING

t1.Start();
BirchPhase1(Stats);
t1.Stop();

//Paras->logfile<<Stats;

#ifdef LOGGING
//Paras->logfile<<t1<<endl;
#endif LOGGING

//cout << "Doing Phase2\n";

#ifdef LOGGING
//Paras->logfile<<"******************Phase 2******************\n";
#endif LOGGING

t2.Start();
BirchPhase2(Stats);
t2.Stop();

//Paras->logfile<<Stats;

#ifdef LOGGING
//Paras->logfile<<t2<<endl;
#endif LOGGING

if (Paras->CorD==1) { // do density

//cout << "Doing Density\n";

#ifdef LOGGING
//Paras->logfile<<"******************Density****************\n";
#endif LOGGING

td.Start();
BirchDensity(Stats);
td.Stop();

// Paras->logfile<<Stats;

#ifdef LOGGING
//Paras->logfile<<td<<endl;
//Paras->logfile<<"Phase1-2+Density "<<t1+t2+td<<endl;
#endif LOGGING
}

else { // do clustering 

//cout << "Doing Phase3\n";

#ifdef LOGGING
//Paras->logfile<<"******************Phase 3******************\n";
#endif LOGGING

t3.Start();
BirchPhase3(Stats);
t3.Stop();

// Paras->logfile<<Stats;

#ifdef LOGGING
//Paras->logfile<<t3<<endl;
//Paras->logfile<<"Phase1-3 "<<t1+t2+t3<<endl;
#endif LOGGING

//cout << "Doing Phase4\n";

#ifdef LOGGING
//Paras->logfile<<"******************Phase 4****************\n";
#endif LOGGING

t4.Start();
BirchPhase4(Stats);
t4.Stop();

// Paras->logfile<<Stats;

#ifdef LOGGING
//Paras->logfile<<t4<<endl;
//Paras->logfile<<"Phase1-4 "<<t1+t2+t3+t4 <<endl;
#endif LOGGING
}

// free space fro all trees
for (i=0; i<Paras->ntrees; i++) delete Stats[i];
delete [] Stats;
delete Paras;
}

