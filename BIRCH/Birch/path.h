/****************************************************************
File Name: path.h  
Author: Tian Zhang, CS Dept., Univ. of Wisconsin-Madison, 1995

               Copyright(c) 1995 by Tian Zhang

                   All Rights Reserved

Permission to use, copy and modify this software must be granted
by the author and provided that the above copyright notice appear 
in all relevant copies and that both that copyright notice and this 
permission notice appear in all relevant supporting documentations. 

Comments and additions may be sent the author at zhang@cs.wisc.edu.

******************************************************************/

#ifndef PATH_H
#define PATH_H

class Node;

class Path {
public: 

	int height;
	int stacktop;
	int *indexstack;
	Node **nodestack;

	explicit Path(int size);
	~Path();

	void Push(int index, Node* node);
	void Pop(int &index, Node** node);
	Entry *TopLeafEntry() const;
	Node  *TopLeaf() const;

	void Reset() {stacktop=-1;}
	short Exists() const {return stacktop==height-1;}

	void operator=(const Path& path);
	short operator==(const Path& path);
	short operator>(const Path& path);
	short operator<(const Path& path);


	short NextRightPath();
	Node* NextRightLeafFreeSpace(Stat *Stats);

	short CollectSpace(Stat *Stats);
	void TakeoffPath(const Entry &ent);

	void InsertLeaf(Stat *Stats, Node* Root);
	void AddonPath(Stat *Stats, const Entry &ent, Node* Root);
	void AddonLeaf(Stat *Stats, const Entry &ent, Node* Root);


};

ostream& operator<<(ostream &fo,const Path& node);
ofstream& operator<<(ofstream &fo,const Path& node);

#endif PATH_H



