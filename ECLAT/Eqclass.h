#ifndef _EQCLASS_H
#define _EQCLASS_H

using namespace std;
#include <iostream>
#include <stdio.h>
#include <errno.h>
#include "Lists.h"
#include "Itemset.h"

class EqGrNode;

class Eqclass {
private:
   Lists<Itemset *> *theList;
 public:
   Eqclass(){
      theList = new Lists<Itemset *>;
      if (theList == NULL){
         perror("memory :: Eqclass");
         exit(errno);
      }
   }
   Eqclass(Lists<Itemset *> *ll){
      theList = ll;
   }

   ~Eqclass(){
      if (theList){
         theList->clear();
         delete theList;
      }
      theList = NULL;
   }

   inline Lists<Itemset *> * list()
   {
      return theList;
   }
   inline void set_list(Lists<Itemset *> * ll)
   {
      theList = ll;
   }
   inline void append(Itemset *it)
   {
      theList->append(it);
   }

   inline void remove(ListNodes<Itemset *> *prev, ListNodes<Itemset *> *it)
   {
      theList->remove(prev, it);
   }

   inline void sortedAscend(Itemset *it, CMP_FUNC func)
   {
      theList->sortedAscend(it, func);
   }
   inline void sortedDescend(Itemset *it, CMP_FUNC func)
   {
      theList->sortedDescend(it, func);
   }

   Itemset * uniqsorted(Itemset *it, CMP_FUNC func)
   {
      Itemset *rval;
      //ListNodes<Itemset *> * prev = NULL;
      //if (!theList->find_ascend(prev, it, func))
      //   theList->insert(prev, it);
      if (!(rval = theList->find(it, Itemset::Itemcompare))){
         theList->sortedAscend(it, func);
         //  return 0;
         //}
         //else return 1;
      }
      return rval;
   }
   
   int subseq(Itemset *it)
   {
      ListNodes<Itemset *> *hd = theList->head();
      for (;hd; hd=hd->next()){
         if (it->subsequence(hd->item())){
            return 1;
         }
      }
      return 0;
   }
   
   friend ostream& operator << (ostream& outputStream, Eqclass& EQ){
      outputStream << "EQ : ";
      EQ.theList->print();
      return outputStream;
   };
};

class EqGrNode {
private:
   int *theElements;
   int numElements;
   Lists<int *> *theCoverL;
   Lists<Array *> *theCliqueL;
   Lists<Itemset *> *theLargeL;
   int theFlg; //indicates if class is in memory
public:
   EqGrNode(int sz)
   {
      numElements = sz;
      theElements = new int[sz];
      theCoverL = new Lists<int *>;
      theLargeL = new Lists<Itemset *>;
      theCliqueL = new Lists<Array *>;
      theFlg = 0;
   }
   
   ~EqGrNode()
   {
      delete [] theElements;
      delete theCoverL;
      delete theCliqueL;
      theElements = NULL;
      theCoverL = NULL;
      theFlg = 0;
   }

   inline int getflg()
   {
      return theFlg;
   }
   inline void setflg(int val)
   {
      theFlg=val;
   }

   inline Lists<Array *> *clique()
   {
      return theCliqueL;
   }
   
   inline Lists<Itemset *> *largelist()
   {
      return theLargeL;
   }

   inline Lists<int *> * cover()
   {
      return theCoverL;
   }
   inline int * elements()
   {
      return theElements;
   }
   inline int num_elements()
   {
      return numElements;
   }
   inline void add_element(int el, int pos)
   {
      theElements[pos] = el;
   }
   inline int get_element(int pos)
   {
      return theElements[pos];
   }
   inline void remove_el(int pos)
   {
      for (int i=pos; i < numElements-1; i++)
         theElements[i] = theElements[i+1];
      numElements--;
   }
   inline void add_cover(int *eq)
   {
      theCoverL->append(eq);
   }

   inline int find_element(Itemset *it)
   {
      int i,j;
      int cnt =0;
      for (i=0, j=0; i < numElements && j < it->size();){
         if (theElements[i] < (*it)[j]) i++;
         else if (theElements[i] == (*it)[j]){
            i++;
            j++;
            cnt++;
         }
         else return 0;
      }
      if (cnt == it->size()) return 1;
      else return 0;
   }
   inline int find_element(Array *it)
   {
      int i,j;
      int cnt =0;
      for (i=0, j=0; i < numElements && j < it->size();){
         if (theElements[i] < (*it)[j]) i++;
         else if (theElements[i] == (*it)[j]){
            i++;
            j++;
            cnt++;
         }
         else return 0;
      }
      if (cnt == it->size()) return 1;
      else return 0;
   }
   
   inline int get_common(Array *arr, int *newcov)
   {
      int i,j;
      int cnt=0;
      
      for (i=0, j=0; i < numElements && j < arr->size();){
         if (theElements[i] < (*arr)[j]) i++;
         else if (theElements[i] > (*arr)[j]) j++;
         else{
            newcov[i] = 1;
            i++;
            j++;
            cnt++;
         }
      }
      return cnt;
   }
   
   friend ostream& operator << (ostream& outputStream, EqGrNode& EQ){
      cout << "ELEMENTS : ";
      for (int i = 0; i < EQ.numElements; i++){
         cout << i << "=" << EQ.theElements[i] << " ";
      }
      cout << endl;
      ListNodes<int *> *hd = EQ.theCoverL->head();
      cout << "COVER : ";
      for (; hd; hd=hd->next()){
         cout << *(hd->item()) << " ";
      }
      cout << endl;
      ListNodes<Array *> *clhd = EQ.theCliqueL->head();
      cout << "CLIQUE : ";
      for (; clhd; clhd=clhd->next()){
         cout << *(clhd->item()) << ", ";
      }
      cout << endl;
      
      return outputStream;
   };
   
};

extern void eq_insert(Lists<Eqclass *> &EQC, Itemset *it);
extern void eq_print(Eqclass *LargeEqclass);
#endif

