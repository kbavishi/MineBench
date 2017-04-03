/****************************************************************
File Name:   contree.h
Author: Tian Zhang, CS Dept., Univ. of Wisconsin-Madison, 1995

               Copyright(c) 1995 by Tian Zhang

                   All Rights Reserved

Permission to use, copy and modify this software must be granted
by the author and provided that the above copyright notice appear 
in all relevant copies and that both that copyright notice and this 
permission notice appear in all relevant supporting documentations. 

Comments and additions may be sent the author at zhang@cs.wisc.edu.

******************************************************************/

#ifndef CONTREE_H
#define CONTREE_H

class Para;
class Stat;

extern Para *Paras;

class ConNode {

public:	
	int actsize;
	Entry *entry;
	ConNode **child;

	ConNode(int size, Stat* Stats);
	~ConNode();

	void Free();
	int ActSize() const {return actsize;}
	Entry* TheEntry(int i) {return &(entry[i]);}
	ConNode* TheChild(int i) {return child[i];}

	int N() const;
	void SX(Vector& tmpsx) const;
	double SXX() const;
	void CF(Entry& tmpcf) const;
#ifdef RECTANGLE
	void Rect(Rectangle& tmprect) const;
#endif RECTANGLE

	double Radius() const;
	double Diameter() const;
	double Fitness(short ftype) const;
	
	int Size() const;
	int Depth() const;
	int NumEntry() const;
	int LeafNum() const;
	int NonleafNum() const;
	int NumLeafEntry() const;
	int NumNonleafEntry() const;
	double Occupancy(Stat *Stats) const;

	void Print_Tree(short ind, ostream &fo) const;
	void Print_Tree(short ind, ofstream &fo) const;
	void Print_Summary(ostream &fo) const;
	void Print_Summary(ofstream &fo) const;

	void Connect(Stat *Stats);

	friend class Stat;
	friend class Node;
	friend class Leaf;
	friend class Nonleaf;

};

#endif CONTREE_H

