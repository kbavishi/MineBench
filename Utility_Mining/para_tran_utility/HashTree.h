#ifndef __HASHTREE_H
#define __HASHTREE_H
#include <omp.h>
#include "ListItemset.h"
#define CCPD
#define YES 1 
#define NO 0 

class HashTree {
public:

   void rehash(); // procedure for converting leaf node to a interior node
   int hash(int); // procedure to find out which node item hashes to

   HashTree(int, int, int);
   ~HashTree();
   
   int add_element(Itemset&);
   void clear();
   int is_root(){return (Depth == 0);};

   int Count;
   friend ostream& operator << (ostream&, HashTree&);

   inline int is_leaf()
   {
      return (Leaf == YES);
   }

   inline ListItemset * list()
   {
      return List_of_itemsets;
   }

   inline int hash_function()
   {
      return Hash_function;
   }

   inline int depth()
   {
      return Depth;
   }

   inline int hash_table_exists()
   {
      return (Hash_table != NULL);
   }
   
   inline HashTree *hash_table(int pos)
   {
      return Hash_table[pos];
   }
   
private:
   int Leaf;
   HashTree **Hash_table;
   int Hash_function;
   int Depth;
   ListItemset *List_of_itemsets;
   int Threshold;

#if defined CCPD
   omp_lock_t nlck; //node lock
   omp_lock_t clck; //count lock
#endif
};

typedef HashTree * HashTree_Ptr;
#endif //__HASHTREE_H
