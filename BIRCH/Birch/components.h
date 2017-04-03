/****************************************************************
File Name: component.h  
Author: Tian Zhang, CS Dept., Univ. of Wisconsin-Madison, 1995

               Copyright(c) 1995 by Tian Zhang

                   All Rights Reserved

Permission to use, copy and modify this software must be granted
by the author and provided that the above copyright notice appear 
in all relevant copies and that both that copyright notice and this 
permission notice appear in all relevant supporting documentations. 

Comments and additions may be sent the author at zhang@cs.wisc.edu.

******************************************************************/

#ifndef COMPONENT_H
#define COMPONENT_H

class Component;
class Components;

class Unit1 {
private:
	int 		*unit;
	Unit1	 	*next;
public:
	Unit1();
	~Unit1();

friend class Component;
friend ostream& operator<<(ostream &fo, Component *Compo);
friend ofstream& operator<<(ofstream &fo, Component *Compo);
};

class Component {
private:
	Unit1 *orig, *head, *tail;
	int start, end, size;
public:
	Component();
	~Component();

	int Size() const;

	void AddVertex(int i);
	int  CurVertex();
	void ResetVertex();
	int  TupleCnt(Entry *entry);
	void EntryChild(Stat* Stats, Entry *entry, ConNode** child, Entry &newentry, ConNode* &newchild);

friend ostream& operator<<(ostream &fo, Component *Compo);
friend ofstream& operator<<(ofstream &fo, Component *Compo);
};

ostream& operator<<(ostream &fo, Component *Compo);
ofstream& operator<<(ofstream &fo, Component *Compo);

class Unit2 {
private:
	Component 	**unit;
	Unit2 		*next;
public:
	Unit2();
	~Unit2();

friend class Components;
friend ostream& operator<<(ostream &fo, Components *Compos);
friend ofstream& operator<<(ofstream &fo, Components *Compos);
};

class Components {
private:
	Unit2 *orig, *head, *tail;
	int start, end, ncluster, noutlier;
public:
	Components();
	~Components();

	int Size() const;
	int NumCluster() const;
	int NumOutlier() const;

	void AddComponent(Component *Compo);
	Component* CurComponent();
	void ResetComponent();

friend ostream& operator<<(ostream &fo, Components *Compos);
friend ofstream& operator<<(ofstream &fo, Components *Compos);
};

ostream& operator<<(ostream &fo, Components *Compos);
ofstream& operator<<(ofstream &fo, Components *Compos);

class Graph {
private:
	int    size;
	short  *flag;
	short  *matrix;
public:

	Graph(int n, Entry* entries);
	Graph(int n, Entry* entries, short ftype, double Ft, double density);

	~Graph();
	Components *Connected_Components();
};

#endif COMPONENT_H
