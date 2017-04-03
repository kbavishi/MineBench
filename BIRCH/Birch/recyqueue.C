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
#include "parameter.h"
#include "cfentry.h"
#include "status.h"
#include "cutil.h"
#include "recyqueue.h"

UnitQueue::UnitQueue(Stat *Stats) {
	queue=new Entry[UNIT_SIZE];
	for (int i=0; i<UNIT_SIZE; i++) 
		queue[i].Init(Stats->Dimension);
	next = NULL;
	}

UnitQueue::~UnitQueue() {
	delete [] queue;
	}

RecyQueueClass::RecyQueueClass(Stat *Stats) { 
	size=Stats->QueueSize*Stats->PageSize/Stats->EntrySize();
	head = tail = new UnitQueue(Stats);
	actsize = start = end = 0;
	}

RecyQueueClass::~RecyQueueClass() {
	UnitQueue *ptr;
	while (head!=tail) { 
		ptr=head; 
		head=head->next; 
		delete ptr;
		}
	delete tail;
	}

void RecyQueueClass::CF(Entry& tmpcf) const {
	UnitQueue *ptr=head; 
	tmpcf.Reset();
	for (int i=0; i<actsize; i++) {
		tmpcf += ptr->queue[(start+i)%UNIT_SIZE]; 
		if ((start+i+1)%UNIT_SIZE==0) ptr=ptr->next;
		}
	 }

int RecyQueueClass::Size() const {return size;}
short RecyQueueClass::Full() const {return actsize>size;}
short RecyQueueClass::Empty() const {return actsize==0;}

int RecyQueueClass::CountEntry() const {return actsize;}

int RecyQueueClass::CountTuple() const {
	UnitQueue *ptr=head;
	int tmpcnt=0;
	for (int i=0; i<actsize; i++) {
		tmpcnt += ptr->queue[(start+i)%UNIT_SIZE].n; 
		if ((start+i+1)%UNIT_SIZE==0) ptr=ptr->next;
		}
	 return tmpcnt;
	 }

void RecyQueueClass::AddEnt(Entry &ent,Stat* Stats) {
	UnitQueue *ptr;
	tail->queue[end]=ent; end++; actsize++;
	if (end==UNIT_SIZE) { 
			end=0;
			ptr=new UnitQueue(Stats);
			tail->next=ptr;
			tail=tail->next;
			}
	}

void RecyQueueClass::DeleteEnt(Entry &ent) {
	UnitQueue *ptr;
	ent=head->queue[start]; start++; actsize--;
	if (start==UNIT_SIZE) { 
			start=0;
		 	ptr=head;
			head=head->next;
			delete ptr;
			}
	}

ostream& operator<<(ostream &fo, RecyQueueClass *Queue)
{
fo << "Size: " << Queue->Size() << endl;
fo << "Entry: " << Queue->CountEntry() << endl;
fo << "Tuple: " << Queue->CountTuple() << endl;
return fo;
}

ofstream& operator<<(ofstream &fo, RecyQueueClass *Queue)
{
fo << "Size: " << Queue->Size() << endl;
fo << "Entry: " << Queue->CountEntry() << endl;
fo << "Tuple: " << Queue->CountTuple() << endl;
return fo;
}
