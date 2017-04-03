/*
  ========================================================================
  DEVise Data Visualization Software
  (c) Copyright 1992-1995
  By the DEVise Development Group
  Madison, Wisconsin
  All Rights Reserved.
  ========================================================================

  Under no circumstances is this software to be copied, distributed,
  or altered in any way without prior permission from the DEVise
  Development Group.
*/

/*
  $Id: DList.h 2396 2014-03-13 21:13:27Z wkliao $

  $Log: DList.h,v $
  Revision 1.9  1996/11/26 16:47:42  ssl
  Added support for Stacked Opaque and Transparent views

  Revision 1.8  1996/08/04 20:58:52  beyer
  Insertion with iterators is now ok.

  Revision 1.7  1996/07/25 14:33:48  guangshu
  Added member function InsertOrderly

  Revision 1.6  1996/04/16 20:06:54  jussi
  Replaced assert() calls with DOASSERT macro.

  Revision 1.5  1996/04/09 18:06:34  jussi
  Added error checking.

  Revision 1.4  1996/04/08 22:13:18  jussi
  Added assertions along the way.

  Revision 1.3  1995/12/28 18:45:12  jussi
  Added copyright notice and made small fixes to remove compiler
  warnings.

  Revision 1.2  1995/09/05 21:12:37  jussi
  Added/updated CVS header.
*/

#ifndef DList_h
#define DList_h

#include "Exit.h"

const int MaxIterators = 5;		/* max # of concurrent iterators */

#define DefineDList(listName, valType)\
class listName {\
\
	struct ListElement {\
		ListElement *next, *prev;\
		valType val;\
	};\
\
	struct IteratorData {\
		ListElement *current;	/* == NULL if not used */\
		int backwards; /* TRUE if iterate backwards */\
	};\
\
public:\
	listName();\
	~listName();\
\
	int Size();\
\
	void Insert(valType v);\
	void InsertOrderly(valType v, int order);\
	void Append(valType v);\
\
	/* Return true if v is in the list */\
	int Find(valType v);\
\
	/* Delete v from list. Return true if v is found */\
	int Delete(valType v);\
\
	/* Return the first element */\
	valType GetFirst();\
\
	/* Return the last element */\
	valType GetLast();\
\
	/* swap elements a and b */\
	void Swap(valType a, valType b);\
\
	/* iterator for the list. This implementation supports\
	multiple iterators, and also  supports \
	Deletecurrent(), which deletes the current element being looked at. \
	Note:  DeleteCurrent() is an error when more than 1 iterator is in \
        effect.\
	*/\
\
	/* Init and return index of iterator */\
	int InitIterator(int backwards=0);\
\
	/* Init iterator to return the last N records */\
	int InitIteratorLastN(int n=1);\
	int More(int index);\
	valType Next(int index); \
	void DeleteCurrent(int index) ;\
	void InsertAfterCurrent(int index, valType v); \
	void InsertBeforeCurrent(int index, valType v); \
	void DoneIterator(int index);\
	void DeleteAll();\
	void PrintIterators();\
\
protected:\
	/* insert node2 after node 1 */\
	void _Insert(ListElement *node1, ListElement *node2);\
\
	/* delete node */\
	void _DListDelete(ListElement *node);\
\
private:\
	void ClearIterators();\
	ListElement *_head;\
	int _size;		/* size of the list */\
\
	IteratorData _iterators[MaxIterators];\
	int _numIterators ;\
};

#define ImplementDList(listName,valType)\
\
listName::listName(){\
	_head = new ListElement;\
	_head->next = _head->prev = _head;\
	_size=0;\
	ClearIterators();\
\
}\
\
listName::~listName() { DeleteAll(); delete _head; }\
\
int listName::Size() { return _size; }\
\
void listName::Insert(valType v){\
	ListElement *node = new ListElement;\
	node->val = v;\
	_Insert(_head, node);\
}\
\
/* insert v in increasing order if order = 1, in decreasing order if order = 0 */\
void listName::InsertOrderly(valType v, int order){\
	ListElement *new_node = new ListElement;\
	ListElement *node;\
	new_node->val = v;\
	if(order == 1) {\
	    for (node = this->_head->next; v >= node->val && node != this->_head; node = node->next){}\
   	} else if(order == 0) {\
	    for (node = this->_head->next; v <= node->val && node != this->_head; node = node->next){}\
	} else {\
	    DOASSERT(0, "Invalid order");\
	    return;\
	}\
	_Insert(node->prev, new_node);\
}\
\
void listName::Append(valType v){\
	ListElement *node = new ListElement;\
	node->val = v;\
	_Insert(_head->prev, node);\
}\
\
/* Return the first element */\
valType listName::GetFirst() {\
	DOASSERT(_head->next != _head, "Empty list");\
	return _head->next->val;\
}\
\
/* Return the last element */\
valType listName::GetLast() {\
	DOASSERT(_head->prev != _head, "Empty list");\
	return _head->prev->val;\
}\
\
void listName::Swap(valType val1, valType val2) {\
	ListElement *node;\
	ListElement *node1=NULL;\
	ListElement *node2=NULL;\
	for (node = this->_head->next; node != this->_head; node = node->next){\
		if (node->val == val1) {\
			node1 = node;\
		} else if (node->val == val2) {\
			node2 = node;\
		}\
	}\
	DOASSERT(node1 && node2, "Empty lists");\
	node1->val = val2;\
	node2->val = val1;\
}\
\
\
/* Init and return index of iterator */\
int listName::InitIterator(int backwards) {\
	/* find an empty slot in the array of active iterators */\
	for(int i = 0; i < MaxIterators; i++) {\
		if (_iterators[i].current == NULL){\
			/* found one */\
			_iterators[i].backwards = backwards;\
			if (backwards)\
				_iterators[i].current = _head->prev;\
			else\
				_iterators[i].current = _head->next; \
			_numIterators++;\
			return i;\
		}\
	}\
	DOASSERT(0, "No more space for iterators");\
	return -1; /* to keep compiler happy */\
}\
\
/* DEBUGGING FUNCTION : Print Iterators */ \
void listName::PrintIterators() { \
    printf("printing active iterators (%d)",_numIterators );\
    for (int i = 0; i < MaxIterators; i++) { \
	if (_iterators[i].current != NULL) {\
	   printf("iterator open %p\n", _iterators[i].current);\
	}\
    }\
}\
\
/* Init iterator to return the last N records */\
int listName::InitIteratorLastN(int n){\
	DOASSERT(n >= 1, "Invalid parameter");\
	/* find an empty slot in the array of active iterators */\
	for(int i = 0; i < MaxIterators; i++) {\
		if (_iterators[i].current == NULL){\
			/* found one */\
			_iterators[i].backwards = 0;\
			if (Size() >= n){\
				/* more than enough entries. Scan backwards n records */\
				_iterators[i].current = _head->prev;\
				for (int j=1; j < n; j++)\
					_iterators[i].current = _iterators[i].current->prev;\
			}\
			else\
				_iterators[i].current = _head->next; \
			_numIterators++;\
			return i;\
		}\
	}\
	DOASSERT(0, "No more space for iterators");\
	return -1; /* to keep compiler happy */\
}\
\
int listName::More(int index) {\
	DOASSERT(index >= 0 && index < MaxIterators, "Invalid iterator");\
	return _iterators[index].current != _head;\
}\
\
valType listName::Next(int index) { \
	DOASSERT(index >= 0 && index < MaxIterators, "Invalid iterator");\
	IteratorData *iData = &_iterators[index];\
	valType v = iData->current->val;\
	if (iData->backwards)\
		iData->current = iData->current->prev;\
	else iData->current = iData->current->next; \
	return v; \
}\
\
void listName::DeleteCurrent(int index) {\
	DOASSERT(index >= 0 && index < MaxIterators, "Invalid iterator");\
	DOASSERT(_numIterators <= 1, "Cannot delete with iterator");\
	if (_iterators[index].backwards)\
		_DListDelete(_iterators[index].current->next);\
	else\
		_DListDelete(_iterators[index].current->prev);\
}\
void listName::InsertAfterCurrent(int index, valType v){ \
	DOASSERT(index >= 0 && index < MaxIterators, "Invalid iterator");\
	IteratorData *iData = &_iterators[index];\
	ListElement *node = new ListElement;\
	node->val = v;\
	if (iData->backwards) \
		_Insert(iData->current, node);\
	else \
		_Insert(iData->current->prev, node);\
} \
void listName::InsertBeforeCurrent(int index, valType v){ \
	DOASSERT(index >= 0 && index < MaxIterators, "Invalid iterator");\
	IteratorData *iData = &_iterators[index];\
	ListElement *node = new ListElement;\
	node->val = v;\
	if (iData->backwards) \
		_Insert(iData->current->prev, node);\
	else \
		_Insert(iData->current->prev->prev, node);\
} \
void listName::DoneIterator(int index){\
	DOASSERT(index >= 0 && index < MaxIterators\
		 && _iterators[index].current != NULL, "Invalid iterator");\
	_iterators[index].current = NULL;\
	--_numIterators;\
	DOASSERT(_numIterators >= 0, "Invalid iterator count");\
}\
\
/* insert node2 after node 1 */\
void listName::_Insert(ListElement *node1, ListElement*node2){\
	node2->next = node1->next;\
	node2->prev = node1;\
	node1->next->prev = node2;\
	node1->next = node2;\
	_size++;\
}\
\
/* delete node */\
void listName::_DListDelete(ListElement *node){\
	DOASSERT(node != _head, "Cannot delete head of list");\
	node->next->prev = node->prev;\
	node->prev->next = node->next;\
	delete node;\
	_size--;\
}\
\
void listName::ClearIterators(){\
	for(int i = 0; i < MaxIterators; i++)\
		_iterators[i].current = NULL;\
	_numIterators = 0;\
}\
\
int listName::Find(valType v) {\
	ListElement *node;\
	for (node = this->_head->next; node != this->_head; node = node->next){\
		if (node->val == v){\
			return 1; /* true */\
		}\
	}\
	return 0;	/* false */\
}\
\
int listName::Delete(valType v){\
	DOASSERT(!_numIterators, "Cannot delete with iterator");\
	ListElement *node;\
	for (node = this->_head->next; node != this->_head; node = node->next){\
		if (node->val == v){\
			_DListDelete(node);\
			return 1; /* true */\
		}\
	}\
	return 0;	/* false */\
}\
\
void listName::DeleteAll(){\
	while (_head->next != _head)\
		_DListDelete(_head->next);\
	_size = 0;\
}\

/* define a void list */
DefineDList(VoidList, void *)

/* template to convert pointer list to use VoidList. 
valType must be a pointer. */
#define DefinePtrDList(listName, valType)\
class listName {\
public:\
	int Size() { return _voidList.Size(); }\
\
	void Insert(valType v) { _voidList.Insert((void *)v);}\
	void Append(valType v) { _voidList.Append((void *)v);}\
\
	/* Return true if v is in the list */\
	int Find(valType v) { return _voidList.Find((void *)v);}\
\
	/* Delete v from list. Return true if v is found */\
	int Delete(valType v) { return _voidList.Delete((void *)v); }\
\
	/* Return the first element */\
	valType GetFirst() { return (valType)_voidList.GetFirst();} \
\
	/* Return the last element */\
	valType GetLast() { return (valType)_voidList.GetLast();}\
\
	/* Swap two list elements */\
	void Swap(valType val1, valType val2) { \
		_voidList.Swap((void *)val1,(void *)val2);\
	}\
\
	/* Init and return index of iterator */\
	int InitIterator(int backwards=0) {\
		return _voidList.InitIterator(backwards);}\
\
	/* Init iterator to return the last N records */\
	int InitIteratorLastN(int n=1){ return _voidList.InitIteratorLastN(n);}\
	int More(int index){return _voidList.More(index);}\
	valType Next(int index){ return (valType)_voidList.Next(index);} \
	void DeleteCurrent(int index){_voidList.DeleteCurrent(index);} \
	void InsertAfterCurrent(int indx, valType v) { \
		_voidList.InsertAfterCurrent(indx,v);\
	}\
	void InsertBeforeCurrent(int indx, valType v) {\
		_voidList.InsertBeforeCurrent(indx,v);\
	}\
	void DoneIterator(int index){_voidList.DoneIterator(index);}\
	void PrintIterators() {_voidList.PrintIterators();}\
private:\
	VoidList _voidList; \
};

#endif
