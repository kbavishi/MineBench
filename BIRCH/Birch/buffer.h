/****************************************************************
File Name:   buffer.h
Author: Tian Zhang, CS Dept., Univ. of Wisconsin-Madison, 1995

               Copyright(c) 1995 by Tian Zhang

                   All Rights Reserved

Permission to use, copy and modify this software must be granted
by the author and provided that the above copyright notice appear 
in all relevant copies and that both that copyright notice and this 
permission notice appear in all relevant supporting documentations. 

Comments and additions may be sent the author at zhang@cs.wisc.edu.

******************************************************************/

#ifndef BUFFER_H
#define BUFFER_H

class Stat;

class BufferClass {
private:
	Entry *buffer;
	int   size;
	int   head;
	int   tail;
public:
	BufferClass(Stat *Stats);
	~BufferClass();

	int Size() const {return size;}
	short Empty() const;
	short Full()  const;
	void CF(Entry& tmpcf) const;

	int CountEntry() const;
	int CountTuple() const;

	void AddEnt(const Entry &ent);
	void DeleteEnt(Entry &ent);

friend ostream& operator<<(ostream &fo, BufferClass *Buffer);
friend ofstream& operator<<(ofstream &fo, BufferClass *Buffer);
};

ostream& operator<<(ostream &fo, BufferClass *Buffer);
ofstream& operator<<(ofstream &fo, BufferClass *Buffer);

#endif BUFFER_H
