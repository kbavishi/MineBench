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

class Stat;

class UnitQueue {
private:
	Entry 		*queue;
	UnitQueue 	*next;
public:
	friend class RecyQueueClass;
	UnitQueue(Stat* Stats);
	~UnitQueue();
};

class RecyQueueClass {
public:
	UnitQueue *head, *tail;
	int start, end, size, actsize;
public:
	RecyQueueClass(Stat* Stats);
	~RecyQueueClass();

	void CF(Entry& tmpcf) const;

	int Size() const;
	short Empty() const;
	short Full() const;

	int CountEntry() const;
	int CountTuple() const;

	void AddEnt(Entry &ent,Stat *Stats);
	void DeleteEnt(Entry &ent);

friend ostream& operator<<(ostream &fo, RecyQueueClass *Queue);
friend ofstream& operator<<(ofstream &fo, RecyQueueClass *Queue);
};

ostream& operator<<(ostream &fo, RecyQueueClass *Queue);
ofstream& operator<<(ofstream &fo, RecyQueueClass *Queue);

#endif RECYQUEUE_H
