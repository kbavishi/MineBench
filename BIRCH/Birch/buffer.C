/****************************************************************
File Name:   buffer.C
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
#include "buffer.h"

BufferClass::BufferClass(Stat *Stats) { 
	int n=Stats->BufferSize*Stats->PageSize/Stats->EntrySize();
	buffer=new Entry[n+1];
	for (int i=0; i<n+1; i++) 
		buffer[i].Init(Stats->Dimension);
	size = n; head = 0; tail = 0; 
	}

BufferClass::~BufferClass() {
	if (buffer!=NULL) delete [] buffer;
	}

short BufferClass::Empty() const
	{ return (tail-head==0); }

short BufferClass::Full()  const
	{ return ((tail-head+size+1)%(size+1)==size); }

void BufferClass::CF(Entry& tmpcf) const
	{int i=head; 
	 tmpcf.Reset();
	 while (i!=tail) 
		{ tmpcf+=buffer[i]; i=(i+1)%(size+1); }
	 }

int BufferClass::CountEntry() const 
	{return (tail-head+size+1)%(size+1);}

int BufferClass::CountTuple() const
	{int i=head; int tmp=0;
	 while (i!=tail) { tmp+=buffer[i].n; i=(i+1)%(size+1); }
	 return tmp;
	}

void BufferClass::AddEnt(const Entry &ent) {
	buffer[tail]=ent;
	tail=(tail+1)%(size+1);
	}

void BufferClass::DeleteEnt(Entry &ent) {
	ent=buffer[head];
	head=(head+1)%(size+1);
	}

ostream& operator<<(ostream &fo, BufferClass *Buffer)
{
fo << "Size: " << Buffer->Size() << endl;
fo << "Entry: " << Buffer->CountEntry() << endl;
fo << "Tuple: " << Buffer->CountTuple() << endl;
return fo;
}

ofstream& operator<<(ofstream &fo, BufferClass *Buffer)
{
fo << "Size: " << Buffer->Size() << endl;
fo << "Entry: " << Buffer->CountEntry() << endl;
fo << "Tuple: " << Buffer->CountTuple() << endl;
return fo;
}
