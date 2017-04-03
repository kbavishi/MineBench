#ifndef __ITEMSET_H
#define __ITEMSET_H
using namespace std;
#include <iostream>
#include <stdio.h>
#include <errno.h>
#include "Array.h"
#include "GArray.h"
#include "Lists.h"
//#include "Bitvec.h"

#define SETBIT(a,b) ((a) |= (1 << (b)))
#define UNSETBIT(a,b) ((a) &= ~(1 << (b)))
#define GETBIT(a,b) ((a) & (1 << (b)))

class Itemset{
protected:
   Array *theItemset;
   GArray<int> *theTidlist;
   //Bitvec *theNeighbors;
   int theSupport;
   int theDiff;
   Itemset **theSubsets;
   int theSubsetcnt;
   int theMaxflag;
   
public:
   Itemset(int it_sz, int list_sz);
   ~Itemset();

   friend ostream& operator << (ostream& outputStream, Itemset& itemset);
   void intersect_neighbors(Itemset *it1, Itemset *it2);
   int compare(Itemset& ar2, int len);
   int compare(Itemset& ar2);
   int compare(Array& ar2, int len);
   int compare(Itemset& ar2, int len, unsigned int);
   int subsequence(Itemset * ar);

   //inline void alloc_neighbors(int sz){
   //   theNeighbors = new Bitvec(sz);
   //};

   int isetsize(){
      if (theItemset) return theItemset->size();
      else return 0;
   }
                   
   int operator [] (int pos){
      return (*theItemset)[pos];
   };
   
   inline int item(int pos){
      return (*theItemset)[pos];
   };
   
   int litem(){
      return (*theItemset)[theItemset->size()-1];      
   }
   
   inline void setitem(int pos, int val){
      theItemset->setitem(pos, val);
   };
   
   inline int tid(int pos){
      return (*theTidlist)[pos];
   };

   //inline int is_neighbor(int nbor){
   //   return (theNeighbors->checkbit(nbor));
   //};
   
   inline int get_max(){
      return theMaxflag;
   };
   
   inline void set_max(int val){
      theMaxflag = val;
   }

   inline void set_itemset (Array *ary)
   {
      theItemset = ary;
   }
   
   inline Array * itemset(){
      return theItemset;
   };

   inline GArray<int> *& tidlist(){
      return theTidlist;
   };
   
   //inline Bitvec * neighbors(){
   //   return theNeighbors;
   //};

   inline Itemset ** subsets(){
      return theSubsets;
   };

   inline int num_subsets(){
      return theSubsetcnt;
   };
   
   void add_subset(Itemset *it){
      theSubsets[theSubsetcnt++] = it;
   };
   
   void add_item(int val){
      theItemset->add(val);
   };
   
   inline void add_tid(int val){
      theTidlist->optadd(val);
   };

   //inline void add_ext_tid(int val, int *ary)
   // {
   //   theTidlist->add_ext(val, theMaxflag, ary);
   //}
   //inline void add_neighbor(int val){
   //   theNeighbors->setbit(val);
   //};

//    inline void add_unique_neighbor(int val){
//       ListNodes<int> *hdr = theNeighbors->head();
//       int found = 0;
//       for (;hdr; hdr = hdr->next()){
//          if (hdr->item() == val){
//             found = 1;
//             break;
//          }
//       }
//       if (!found) theNeighbors->sortedAscend(val, intcmp);
//    };

   inline int size(){
      return theItemset->size();
   };

   inline int tidsize(){
      return theTidlist->size();
   };
   
   //inline int neighborsize(){
   //   return theNeighbors->size();
   //};

   int &diff(){ return theDiff;}

   int & support(){ 
      return theSupport;
   };

   inline void set_support(int sup)
   {
      theSupport = sup;
   }
   
   inline void increment_support(){
      theSupport++;
   };

   static int intcmp (void *it1, void *it2)
   {
      int i1 = *(int *) it1;
      int i2 = *(int *) it2;
      //printf("cmp %d %d\n", i1->theSupport, 
      if (i1 > i2) return 1;
      else if (i1 < i2) return -1;
      else return 0;
   }
   
   static int supportcmp (void *it1, void *it2)
   {
      Itemset * i1 = (Itemset *)it1;
      Itemset * i2 = (Itemset *)it2;
      //printf("cmp %d %d\n", i1->theSupport, 
      if (i1->theSupport > i2->theSupport) return 1;
      else if (i1->theSupport < i2->theSupport) return -1;
      else return 0;
   }
   
   static int Itemcompare(void * iset1, void *iset2)
   {
      Itemset *it1 = (Itemset *) iset1;
      Itemset *it2 = (Itemset *) iset2;
      return it1->compare(*it2);
   }
   
   //int find(int , int*);
   //int subsequence(Itemset &);
   //int compare(Itemset &);
};



#endif //__ITEMSET_H

// int Itemset::find(int val, int*pos)
// {
//    return theItemset->find(val, pos);
// }

// int Itemset::subsequence(Itemset& iset)
// {
//    return theItemset->subsequence(*(iset.theItemset));
// }

// int Itemset::compare(Itemset& iset)
// {
//    return theItemset->compare(*(iset.theItemset));
// }





