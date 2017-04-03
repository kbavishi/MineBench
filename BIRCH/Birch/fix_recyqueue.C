/****************************************************************
File Name:   recyqueue.C
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
#include "status.h"
#include "recyqueue.h"

RecyQueueClass::RecyQueueClass(Stat *Stats) { 
	int n;

#ifdef RECTANGLE
	n=Stats->QueueSize*Stats->PageSize/
  	  (sizeof(int)+sizeof(double)*(3*Stats->Dimension+1));
#else 
	n=Stats->QueueSize*Stats->PageSize/
          (sizeof(int)+sizeof(double)*(Stats->Dimension+1));
#endif

	queue=new Entry[n+1];
	for (int i=0; i<n+1; i++) 
		queue[i].Init(Stats->Dimension);
	size = n; head = 0; tail = 0; 
	}

RecyQueueClass::~RecyQueueClass() {
	if (queue!=NULL) delete [] queue;
	}

short RecyQueueClass::Empty() const
	{ return (tail-head==0); }

short RecyQueueClass::Full()  const
	{ return ((tail-head+size+1)%(size+1)==size); }

void RecyQueueClass::CF(Entry& tmpcf) const
	{int i=head; 
	 tmpcf.Reset();
	 while (i!=tail) 
		{ tmpcf+=queue[i]; i=(i+1)%(size+1); }
	 }

int RecyQueueClass::CountEntry() const 
	{return (tail-head+size+1)%(size+1);}

int RecyQueueClass::CountTuple() const
	{int i=head; int tmp=0;
	 while (i!=tail) { tmp+=queue[i].n; i=(i+1)%(size+1); }
	 return tmp;
	}

void RecyQueueClass::AddEnt(const Entry &ent) {
	queue[tail]=ent;
	tail=(tail+1)%(size+1);
	}

void RecyQueueClass::DeleteEnt(Entry &ent) {
	ent=queue[head];
	head=(head+1)%(size+1);
	}

ostream& operator<<(ostream &fo, RecyQueueClass *Queue) {
fo << "    Size: " << Queue->size << endl;
fo << "    Entry: " << Queue->CountEntry() << endl;
fo << "    Tuple: " << Queue->CountTuple() << endl;
return fo;
}

ofstream& operator<<(ofstream &fo, RecyQueueClass *Queue) { 
fo << "    Size: " << Queue->size << endl;
fo << "    Entry: " << Queue->CountEntry() << endl;
fo << "    Tuple: " << Queue->CountTuple() << endl;
return fo;
}
