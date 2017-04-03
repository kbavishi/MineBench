#include <errno.h>
#include "Itemset.h"

Itemset::Itemset(int it_sz, int list_sz){
   theItemset = new Array(it_sz);
   if (theItemset == NULL){
      perror("memory:: Itemset");
      exit(errno);
   }

   theTidlist = new GArray<int>(list_sz);
   if (theTidlist == NULL){
      perror("memory:: Itemset");
      exit(errno);
   }
   
   //theNeighbors = NULL;
   theSubsets = NULL;
   //if (it_sz > 2){
   //   theSubsets = new Itemset *[it_sz];
   //    if (theSubsets == NULL){
   //      perror("memory:: Itemset");
   //      exit(errno);
   //   }
   //}
   theSubsetcnt = 0;
   theSupport = 0;
   theDiff = 0;
   theMaxflag = 0;
}

Itemset::~Itemset(){
   if (theItemset) delete theItemset;
   if (theTidlist) delete theTidlist;
   if (theSubsets) {
      delete [] theSubsets;
   }
   theItemset = NULL;
   theTidlist = NULL;
   theSubsets = NULL;
   theDiff = 0;
   theSupport = 0;
   theMaxflag = 0;
}

ostream& operator << (ostream& outputStream, Itemset& itemset){
   //outputStream << "ISET: ";
   outputStream << *itemset.theItemset;
   //outputStream << "SUP: ";
   outputStream << "-1 ";

   outputStream << itemset.theSupport;
   //outputStream << " " << itemset.theDiff;

   //itemset.theNeighbors->print();
   //outputStream << " " << itemset.theNeighbors->size();
   //outputStream << "the MAX:" << itemset.theMaxflag;
   outputStream << "\n";
   return outputStream;
}

// void Itemset::intersect_neighbors(Itemset *it1, Itemset *it2){
//    theNeighbors->intersect(*it1->theNeighbors, *it2->theNeighbors);
//    int bitset = theNeighbors->count_setbits();
//    theNeighbors->set_size(bitset);
// };


int Itemset::compare(Itemset& ar2)
{
   int len;
   if (size() <= ar2.size()) len = size();
   else len = ar2.size();
   for(int i=0; i < len; i++){
      if ((*theItemset)[i] > (*ar2.theItemset)[i]) return 1;
      else if ((*theItemset)[i] < (*ar2.theItemset)[i]) return -1;
   }
   if (size() < ar2.size()) return -1;
   else if (size() > ar2.size()) return 1;
   else return 0;
}

//len must be less than length of both Itemsets
int Itemset::compare(Itemset& ar2, int len)
{
   for(int i=0; i < len; i++){
      if ((*theItemset)[i] > (*ar2.theItemset)[i]) return 1;
      else if ((*theItemset)[i] < (*ar2.theItemset)[i]) return -1;
   }
   return 0;
}
int Itemset::compare(Array& ar2, int len)
{
   for(int i=0; i < len; i++){
      if ((*theItemset)[i] > ar2[i]) return 1;
      else if ((*theItemset)[i] < ar2[i]) return -1;
   }
   return 0;
}

int Itemset::compare(Itemset& ar2, int len, unsigned int bvec)
{
   int pos = 0;
   int it;
   for(int i=0; i < len; i++){
      while (!GETBIT(bvec, pos)){
         pos ++;
      }
      it = (*theItemset)[pos++];
      if (it > (*ar2.theItemset)[i]) return 1;
      else if (it < (*ar2.theItemset)[i]) return -1;
   }
   return 0;
}

int Itemset::subsequence(Itemset * ar)
{
   int i,j;
   if (size() > ar->size()) return 0;
   int start = 0;
   for(i=0; i < size(); i++){
      for(j=start; j < ar->size(); j++){
         if ((*theItemset)[i] == (*ar->theItemset)[j]){
            start = j+1;
            break;
         }
      }
      if (j >= ar->size()) return 0;
   }
   return 1;
}





