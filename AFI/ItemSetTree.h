#ifndef ITEM_SET_TREE_H
#define ITEM_SET_TREE_H

#include <iostream>
#include <string.h>

using namespace std;

struct ItemSetTreeNode
{
  int lastIndex;
  int *items;
  int itemCount;
  struct ItemSetTreeNode **successors;
  int *supportingTransactions;
  int supportCount;
  bool meetsThreshold;
}; 

class ItemSetTree
{
 public:
  ItemSetTree(int items);

  ItemSetTreeNode * GetNode(int *items, int itemCount, int skip)
    const;
  
  ItemSetTreeNode * CreateNode(int *items, int itemCount,
			       int *supportTrans, int supportCount,
			       bool meetsThreshold);

 private:
  ItemSetTreeNode treeRoot;
  int itemCount;

  int SuccessorCount(int lastIndex, int depth) const;
};

#endif
