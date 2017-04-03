#ifndef __ITEMSET_H
#define __ITEMSET_H
#include <malloc.h>
using namespace std;
#include <iostream>

#include "pardhp.h"

#define DEFAULT -1

class Itemset {
public:
   
   Itemset (int = 1);
   ~Itemset();

   void clear();
   void copy(Itemset *);
   int subsequence(Itemset&, int *);
   int subsequence(Itemset&);
   int subsequence(char *, int);
   int compare(Itemset&);
   int compare(Itemset&, int);

   omp_lock_t lck;

   friend ostream& operator << (ostream&, Itemset&);

   inline int item(int pos){
      return theItemset[pos];
   }
   inline void add_item(int pos, int val)
   {
      theItemset[pos] = val;
   }
   
   inline int numitems(){
      return theNumel;
   }
   inline void set_numitems(int val)
   {
      theNumel = val;
   }
   
   inline int maxsize(){
      return theSize;
   }
   inline int sup(){
      return support;
   }
   inline float get_t_utility(){
      return t_utility;
   }
   inline void incr_sup()
   {
      support++;
   }
   inline void incr_t_utility(float val)
   {
      t_utility += val;
   }
#ifdef OPTIMAL
   inline float get_utility(){
      return utility;
   }
            
   inline void incr_utility(float val)
   {
       utility += val;
   }
#endif               
   inline void set_sup(int val)
   {
      support = val;
   }

   inline void set_t_utility(float val)
   {
      t_utility = val;
   }

   inline int tid(){
      return Tid;
   }
   inline void set_tid(int val)
   {
      Tid = val;
   }
   
private:
   int *theItemset;
   unsigned int theNumel;
   unsigned int theSize;
   int support;
   float t_utility;
   float utility;
   int Tid;
};

#endif //__ITEMSET_H

