#include <stddef.h>
#include "HashTree.h"
#include "pardhp.h"
#include <malloc.h>
#include <omp.h>
HashTree::HashTree (int Depth_P, int hash, int thresh)
{
   Leaf = YES; 
   Count = 0;
   Hash_function = hash;
   Depth = Depth_P;
   List_of_itemsets = NULL;
   Hash_table = NULL;
   Threshold = thresh;
#if defined CCPD
   omp_init_lock(&nlck);
   omp_init_lock(&clck);
#endif
}

HashTree::~HashTree()
{
   clear();
}

ostream& operator << (ostream& outputStream, HashTree& hashtree){
      if (hashtree.Depth == 0)
         outputStream << " ROOT : C:" << hashtree.Count
                      << " H:" << hashtree.Hash_function << "\n";
      if (hashtree.Leaf){
         if (hashtree.List_of_itemsets != NULL){
            outputStream << " T:" << hashtree.Threshold
                         << " D:" << hashtree.Depth << "\n";
            outputStream << *(hashtree.List_of_itemsets) << flush;
         }
      }
      else{
         for(int i=0; i < hashtree.Hash_function; i++){
            if (hashtree.Hash_table[i]){
#if defined CCPD
               outputStream << "child = " << i << "\n";
#else
               outputStream << "child = " << i
                            << ", Count = " << hashtree.Count << "\n";
#endif
               outputStream << *hashtree.Hash_table[i] << flush;
            }
         }
      }
   
   return outputStream;
}

void HashTree::clear(){
   if (Leaf){
      if (List_of_itemsets)
         delete List_of_itemsets;
      List_of_itemsets = NULL;
   }
   else{
      if (Hash_table){
         for(int i=0; i < Hash_function; i++){
            if (Hash_table[i]) delete Hash_table[i];
         }
         delete [] Hash_table;
         Hash_table = NULL;
      }  
   }
   Leaf = YES; 
   Count = 0;
   Hash_function = 0;
   Threshold = 0;
   Depth = 0;
#if defined CCPD
   omp_unset_lock(&nlck);
   omp_unset_lock(&clck);
#endif
}

int HashTree::hash(int Value)
{
   if(Value != 0)
      return (Value%Hash_function);
   else
      return 0;
}

void HashTree::rehash()
{
   Leaf = NO;
   Hash_table = new HashTree_Ptr[Hash_function];
   for (int i=0; i < Hash_function; i++)
      Hash_table[i] = NULL;
   
   while(!(List_of_itemsets->first() == NULL)) { // iterate over current itemsets
      Itemset *temp = List_of_itemsets->remove();
#ifdef BALT
      int val = hash_indx[temp->item(Depth)];//according to current Depth
#else
      int val = hash(temp->item(Depth)); // according to current Depth
#endif
      if (Hash_table[val] == NULL){
         Hash_table[val] = new HashTree(Depth+1, Hash_function, Threshold);
      }
      Hash_table[val]->add_element(*temp);
   }

   delete List_of_itemsets;
   List_of_itemsets = NULL;
}


int HashTree::add_element(Itemset& Value)
{
#if defined CCPD
   int acquired = 0;
   if (Leaf){
      omp_set_lock(&nlck);
      acquired = 1;
   }
#endif
   
   if (Leaf){
      if (List_of_itemsets == NULL)
         List_of_itemsets = new ListItemset;
      
      List_of_itemsets->append(Value);
      if(List_of_itemsets->numitems() > Threshold){
         if (Depth+1 > Value.numitems())
            Threshold++;
         else rehash();     // if so rehash
      }
#if defined CCPD
      omp_unset_lock(&nlck);
#endif
   }
   else{
#if defined CCPD
      if (acquired) omp_unset_lock(&nlck);
      else while(1) {
             if (!omp_test_lock(&nlck)){}
             else {
               omp_unset_lock(&nlck);
               break;
             }
           }
#endif      
#ifdef BALT
      int val = hash_indx[Value.item(Depth)];
      //printf("in balt %d\n", val);
#else
      int val = hash(Value.theItemset[Depth]);
      //printf("NO balt %d\n", val);
#endif
      if (Hash_table[val] == NULL){
#if defined CCPD
         omp_set_lock(&nlck);
#endif
         if (Hash_table[val] == NULL)
            Hash_table[val] = new HashTree(Depth+1, Hash_function, Threshold);
#if defined CCPD
         omp_unset_lock(&nlck);
#endif
      }
      Hash_table[val]->add_element(Value);
   }
#if defined CCPD
   if (is_root()){
      int tt;
      omp_set_lock(&clck);
      Count++;
      tt = Count;
      omp_unset_lock(&clck);
      return tt;
   }
#else
   Count++;
   return Count;
#endif
}	

