/****************************************************************
File Name:   recyqueue.h
Author: Tian Zhang, CS Dept., Univ. of Wisconsin-Madison, 1995

               Copyright(c) 1995 by Tian Zhang

                   All Rights Reserved

Permission to use, copy and modify this software must be granted
by the author and provided that the above copyright notice appear 
in all relevant copies and that both that copyright notice and this 
permission notice appear in all relevant supporting documentations. 

Comments and additions may be sent the author at zhang@cs.wisc.edu.

******************************************************************/

#ifndef RECYQUEUE_H
#define RECYQUEUE_H

class RecyQueueClass {
private:
	Entry *queue;
	int   size;
	int   head;
	int   tail;
public:
	RecyQueueClass(Stat *Stats);
	~RecyQueueClass();
	short Empty() const;
	short Full()  const;
	void CF(Entry& tmpcf) const;
	int CountEntry() const;
	int CountTuple() const;
	void AddEnt(const Entry &ent);
	void DeleteEnt(Entry &ent);

friend ostream& operator<<(ostream &fo, RecyQueueClass *Queue);
friend ofstream& operator<<(ofstream &fo, RecyQueueClass *Queue);
};

ostream& operator<<(ostream &fo, RecyQueueClass *Queue);
ofstream& operator<<(ofstream &fo, RecyQueueClass *Queue);

#endif RECYQUEUE_H
