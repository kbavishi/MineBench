#include "ItemSetTree.h"

ItemSetTree::ItemSetTree(int items):
  itemCount(items)
{
  treeRoot.lastIndex = -1;
  treeRoot.items = NULL;
  treeRoot.itemCount = 0;
  treeRoot.successors = new ItemSetTreeNode * [itemCount];
  treeRoot.supportingTransactions = NULL;
  treeRoot.supportCount = 0;
  treeRoot.meetsThreshold = true;
}

int ItemSetTree::SuccessorCount(int lastIndex, int depth) const
{
  if (depth == 0)
    return itemCount;
  else
    return itemCount - lastIndex - 1;
}

ItemSetTreeNode * ItemSetTree::CreateNode(int *items, int itemsCount,
					  int *supportTrans, int supportCount,
					  bool meetsThreshold)
{
  // traverse to the correct place in the tree
  ItemSetTreeNode *currentNode = &treeRoot;
  for (int i = 0; i < itemsCount - 1; i++) {
    currentNode = currentNode->successors[items[i] - currentNode->lastIndex - 1];
  }

  // create the appropriate successor
  ItemSetTreeNode *newNode = new ItemSetTreeNode;
  newNode->lastIndex = items[itemsCount - 1];
  int successorCount = SuccessorCount(newNode->lastIndex, itemsCount);
  if (successorCount == 0) {
    newNode->successors = NULL;
  } else {
    newNode->successors = new ItemSetTreeNode * [successorCount];
    for (int i = 0; i < successorCount; i++)
      newNode->successors[i] = NULL;
  }  
  newNode->supportingTransactions = new int[supportCount];
  memcpy(newNode->supportingTransactions, supportTrans,
	 sizeof (int) * supportCount);
  newNode->items = new int[itemCount];
  memcpy(newNode->items, items, sizeof(int) * itemCount);
  newNode->itemCount = itemsCount;
  newNode->supportCount = supportCount;
  newNode->meetsThreshold = meetsThreshold;

  
  // add this new node to the tree
  int index =  newNode->lastIndex - currentNode->lastIndex - 1;
  currentNode->successors[index] = newNode;

  if (newNode == NULL)
    cerr << "NULL node" << endl;

  return newNode;
}


ItemSetTreeNode * ItemSetTree::GetNode(int *items, int itemCount, int skip) const
{
  ItemSetTreeNode *currentNode = (ItemSetTreeNode *) &treeRoot;

  // traverse the tree
  for (int i = 0; i < itemCount; i++) {
    if (currentNode == NULL)
      break;

    if (i != skip) {
      int index = items[i] - currentNode->lastIndex - 1;
      currentNode = currentNode->successors[index];
    }  
  }		     

  // if (currentNode == NULL)
  //  cerr << "NULL node" << endl;

  return currentNode;
}
