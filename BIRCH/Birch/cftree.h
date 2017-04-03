/****************************************************************
File Name:   cftree.h
Author: Tian Zhang, CS Dept., Univ. of Wisconsin-Madison, 1995

               Copyright(c) 1995 by Tian Zhang

                   All Rights Reserved

Permission to use, copy and modify this software must be granted
by the author and provided that the above copyright notice appear 
in all relevant copies and that both that copyright notice and this 
permission notice appear in all relevant supporting documentations. 

Comments and additions may be sent the author at zhang@cs.wisc.edu.

******************************************************************/

#ifndef CFTREE_H
#define CFTREE_H

class Para;
class Stat;
class Path;
class ConNode;

extern Para *Paras;

typedef enum {
	LEAF, NONLEAF
} NodeType;

class Node {

public:	
	int actsize;
	Entry *entry;

	int ActSize() const {return actsize;}
	void AssignActSize(int size) {actsize=size;}
	Entry* TheEntry(int i) { return &(entry[i]); }
	Entry* Entries() const {return entry;}

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
	
	virtual int MaxSize(Stat *Stats) const=0;
	virtual NodeType Type() const=0;
	virtual int Size() const=0;
	virtual int Depth() const=0;
	virtual int NumEntry() const=0;
	virtual int LeafNum() const=0;
	virtual int NonleafNum() const=0;
	virtual int NumLeafEntry() const=0;
	virtual int NumNonleafEntry() const=0;
	virtual double Occupancy(Stat *Stats) const=0;

	virtual double AbsVofLevel(int i, short ftype, short dim) const=0;

	void Print_Summary(Stat *Stats, ostream &fo) const;
	void Print_Summary(Stat *Stats, ofstream &fo) const;

	short NotEmpty() const {return actsize>0;} 
	short NotFull(Stat *Stats) const {return actsize<MaxSize(Stats);}

	// inline functions: for convenience
	virtual void free_leaf(Stat *Stats)=0;
	virtual void free_nonleaf(Stat *Stats)=0;
	virtual void NewLeafChildI(Stat *Stats, int i)=0;
	virtual void NewNonleafChildI(Stat *Stats, int i)=0;
	virtual void assign(Node *node1, int i1, Node *node2, int i2)=0;
	virtual void swap(Node *node1, int i1, Node *node2, int i2)=0;

	void AddEntry(int i, const Entry &ent) {
		entry[i]+=ent;
		}
	void SubEntry(int i, const Entry &ent) {
		entry[i]-=ent;
		}

	virtual void AttachEntry(const Entry &ent, Node *ptr)=0;
	virtual void DeleteEntry(int i)=0;

	void AssignEntry(int i, const Entry &ent) { entry[i]=ent; }
	virtual Node *TheChild(int i) const=0;
	virtual void AssignChild(int i, Node *ptr)=0;

	virtual void Print_Tree(short ind, ostream &fo) const=0;
	virtual void Print_Tree(short ind, ofstream &fo) const=0;

	virtual void print_leaf_entries_topdown(ofstream &fo) const=0;

	virtual int DensestEntry() const=0;
	virtual Node* DenseNode()=0;

	virtual void AssignNextPrev(Stat *Stats)=0;
	virtual void ChainNextPrev(Stat *Stats)=0;

	virtual short FreeEmptyNode(Stat *Stats)=0;

virtual int ClosestOne(Stat *Stats, const Entry &ent) const=0;
virtual double ClosestTwo(Stat *Stats, int &i, int &j) const=0;
virtual void FarthestTwo(Stat *Stats, int &i, int &j) const=0;
virtual double ClosestDiffTwo(Stat *Stats, int &i, int &j) const=0;
virtual void FarthestTwoOut(Stat *Stats, Node *node1, Node *node2,
			    short &samegroup, int &i, int &j) const=0;

virtual short BestFitPath1(Stat *Stats, const Entry &ent, Path& BestPath)=0;
virtual short BestFitPath2(Stat *Stats, const Entry &ent, Path& BestPath)=0;
virtual short AbsorbEntry1(Stat *Stats, const Entry &ent)=0;
virtual short AbsorbEntry2(Stat *Stats, const Entry &ent)=0;
virtual Node* AdjustTree(Stat *Stats, const Entry &ent)=0;
virtual Node* InsertMightSplit(Stat *Stats, const Entry &ent, Node *ptr)=0;

virtual ConNode* Copy(Stat *Stats) const=0;

	friend class Stat;
	friend class Leaf;
	friend class Nonleaf;
	friend class Path;

};

class Leaf : public Node {

public:

	Leaf* next;
	Leaf* prev;

	Leaf(Stat *Stats) { 
		actsize=0;
	 	int leafsize=MaxSize(Stats);
		if (leafsize<1) 
			print_error("Leaf","less than 1 entry per page");
	 	entry = new Entry[leafsize];
	 	for (int i=0; i<leafsize; i++)
			entry[i].Init(Stats->Dimension);

		 next = NULL;
		 prev = NULL;
	       }

	virtual ~Leaf() {
		delete [] entry;
		}

	virtual int MaxSize(Stat *Stats) const {
#ifdef RECTANGLE
	return (Stats->PageSize-2*sizeof(int))/
	       (sizeof(int)+sizeof(double)*(3*Stats->Dimension+1));
#else
	return (Stats->PageSize-2*sizeof(int))/
	       (sizeof(int)+sizeof(double)*(Stats->Dimension+1));
#endif
	}

	virtual NodeType Type() const {return LEAF;}
	virtual int Size() const;
	virtual int Depth() const;
	virtual int NumEntry() const;
	virtual int LeafNum() const;
	virtual int NonleafNum() const;
	virtual int NumLeafEntry() const;
	virtual int NumNonleafEntry() const;
	virtual double Occupancy(Stat *Stats) const;

	virtual double AbsVofLevel(int i, short ftype, short dim) const;

	virtual void free_leaf(Stat *Stats) {
		if (this->next!=NULL)
			this->next->free_leaf(Stats);
		delete this; Stats->MemUsed--;
		}

	virtual void free_nonleaf(Stat *Stats) {}

	virtual void NewLeafChildI(Stat *Stats, int i) {}
	virtual void NewNonleafChildI(Stat *Stats, int i) {}

	virtual void assign(Node *node1, int i1, Node *node2, int i2) 
	{
	if (!(node1==node2 && i1==i2))
    		node1->entry[i1]=node2->entry[i2];
    	}

	virtual void swap(Node *node1, int i1, Node *node2, int i2) {
	Entry tmpent;
	tmpent.Init(node1->entry[0].sx.dim);
	if (!(node1==node2 && i1==i2)) {
		tmpent=node1->entry[i1];
		node1->entry[i1]=node2->entry[i2];
		node2->entry[i2]=tmpent;
		}
	}

	virtual void AttachEntry(const Entry& ent, Node* ptr) {
		entry[actsize]=ent; 
		// ignore ptr
		actsize++;
		}

	virtual void DeleteEntry(int i) {
		actsize--; 
		entry[i]=entry[actsize];
		}

	virtual Node* TheChild(int i) const {return NULL;}
	virtual void AssignChild(int i, Node *ptr) { }

	virtual void Print_Tree(short ind, ostream &fo) const;
	virtual void Print_Tree(short ind, ofstream &fo) const;

	virtual void print_leaf_entries_topdown(ofstream &fo) const;

	virtual int DensestEntry() const;
	virtual Node* DenseNode();

	virtual void AssignNextPrev(Stat *Stats) {
		if (this==Stats->NewLeafHead) 
			Stats->NewLeafHead=this->next;
	        if (this->prev!=NULL)
			{this->prev->next=this->next;
			}
		if (this->next!=NULL)
			{this->next->prev=this->prev;
			}
		}

	virtual void ChainNextPrev(Stat *Stats) {
		Stats->OldLeafHead->next=this;
		this->prev=Stats->OldLeafHead;
		Stats->OldLeafHead=this;
		}

	virtual short FreeEmptyNode(Stat *Stats);

virtual int ClosestOne(Stat *Stats, const Entry &ent) const;
virtual double ClosestTwo(Stat *Stats, int &i, int &j) const;
virtual void FarthestTwo(Stat *Stats, int &i, int &j) const;
virtual double ClosestDiffTwo(Stat *Stats, int &i, int &j) const;
virtual void FarthestTwoOut(Stat *Stats, Node *node1, Node *node2, 
		            short &samegroup, int &i, int &j) const;

virtual short BestFitPath1(Stat *Stats, const Entry &ent, Path& BestPath);
virtual short BestFitPath2(Stat *Stats, const Entry &ent, Path& BestPath);
virtual short AbsorbEntry1(Stat *Stats, const Entry &ent);
virtual short AbsorbEntry2(Stat *Stats, const Entry &ent);
virtual Node* AdjustTree(Stat *Stats, const Entry &ent);
virtual Node* InsertMightSplit(Stat *Stats, const Entry &ent, Node *ptr);

virtual ConNode* Copy(Stat *Stats) const;

	// only for leaf nodes
	Leaf* NextLeaf() const {return next;}
	Leaf* PrevLeaf() const {return prev;}

	void print_leaf_entries_bychain(ofstream &fo) const;
	void print_leaf_entries_bychain(ostream &fo) const;

	void visualize_leaf_entries(Stat *Stats, ofstream &fo);
	void visualize_leaf_entries(Stat *Stats, ostream &fo);

	int count_leaf_nodes() const;
	int count_leaf_entries() const;
	int count_leaf_tuples() const;

	friend class Stat;
};

class Nonleaf : public Node {

public:

	Node **child;

	Nonleaf(Stat *Stats) { 
	actsize = 0;
        int nonleafsize=MaxSize(Stats);
        entry = new Entry[nonleafsize];
	child = new Node*[nonleafsize];
        for (int i=0;i<nonleafsize;i++) {
		entry[i].Init(Stats->Dimension);
		child[i]=NULL;
		}
	}

	virtual ~Nonleaf() { 
		delete [] entry; 
		delete [] child;
		}

	virtual int MaxSize(Stat *Stats) const {
#ifdef RECTANGLE
	return Stats->PageSize/(2*sizeof(int)+sizeof(double)*(3*Stats->Dimension+1));
#else
	return Stats->PageSize/(2*sizeof(int)+sizeof(double)*(Stats->Dimension+1));
#endif
	}

	virtual NodeType Type() const {return NONLEAF;}
	virtual int Size() const;
	virtual int Depth() const;
	virtual int NumEntry() const;
	virtual int LeafNum() const;
	virtual int NonleafNum() const;
	virtual int NumLeafEntry() const;
	virtual int NumNonleafEntry() const;
	virtual double Occupancy(Stat *Stats) const;

	virtual double AbsVofLevel(int i, short ftype, short dim) const;

	virtual void free_leaf(Stat *Stats) {}

	virtual void free_nonleaf(Stat *Stats) {
		for (int i=0; i<actsize; i++)
			child[i]->free_nonleaf(Stats);
		delete this; Stats->MemUsed--;
		}

	virtual void NewLeafChildI(Stat *Stats, int i) { 
		child[i]=new Leaf(Stats);
		}
	virtual void NewNonleafChildI(Stat *Stats, int i) {
		child[i]=new Nonleaf(Stats); 
		}

	virtual void assign(Node *node1, int i1, Node *node2, int i2) 
	{
	if (!(node1==node2 && i1==i2)) {
   	  	node1->entry[i1]=node2->entry[i2];
   		((Nonleaf*)node1)->child[i1]=((Nonleaf *)node2)->child[i2];
   		}
   	}

	virtual void swap(Node *node1, int i1, Node *node2, int i2) {
	Entry tmpent;
	tmpent.Init(node1->entry[0].sx.dim);
	Node  *tmpptr;
	if (!(node1==node2 && i1==i2)) {
		tmpent=node1->entry[i1];
		tmpptr=((Nonleaf *)node1)->child[i1];
		node1->entry[i1]=node2->entry[i2];
		((Nonleaf *)node1)->child[i1]=((Nonleaf *)node2)->child[i2];
		node2->entry[i2]=tmpent;
		((Nonleaf *)node2)->child[i2]=tmpptr;
		}
	}
	
	virtual void AttachEntry(const Entry &ent, Node *ptr) {
		entry[actsize]=ent,
		child[actsize]=ptr;
		actsize++;
		}

	virtual void DeleteEntry(int i) {
		actsize--;
		entry[i]=entry[actsize];
		child[i]=child[actsize];
		}

	virtual Node* TheChild(int i) const { return child[i]; }
	virtual void AssignChild(int i, Node *ptr) { child[i]=ptr; }

	virtual void Print_Tree(short ind, ostream &fo) const;
	virtual void Print_Tree(short ind, ofstream &fo) const;

	virtual void print_leaf_entries_topdown(ofstream &fo) const;

	virtual int DensestEntry() const;
	virtual Node* DenseNode();

	virtual void AssignNextPrev(Stat *Stats) {}
	virtual void ChainNextPrev(Stat *Stats) {}

	virtual short FreeEmptyNode(Stat *Stats);

virtual int ClosestOne(Stat *Stats, const Entry &ent) const;
virtual double ClosestTwo(Stat *Stats, int &i, int &j) const;
virtual void FarthestTwo(Stat *Stats, int &i, int &j) const;
virtual double ClosestDiffTwo(Stat *Stats, int &i, int &j) const;
virtual void FarthestTwoOut(Stat *Stats, Node *node1, Node *node2, 
		            short &samegroup, int &i, int &j) const;

virtual short BestFitPath1(Stat *Stats, const Entry &ent, Path& BestPath);
virtual short BestFitPath2(Stat *Stats, const Entry &ent, Path& BestPath);
virtual short AbsorbEntry1(Stat *Stats, const Entry &ent);
virtual short AbsorbEntry2(Stat *Stats, const Entry &ent);
virtual Node* AdjustTree(Stat *Stats, const Entry &ent);
virtual Node* InsertMightSplit(Stat *Stats, const Entry &ent, Node *ptr);
void MergeMightResplit(Stat *Stats, int i, int j);

virtual ConNode* Copy(Stat *Stats) const;

	friend class Stat;
};

#endif CFTREE_H



