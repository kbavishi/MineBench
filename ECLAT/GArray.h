#ifndef __GARRAY_H
#define __GARRAY_H
using namespace std;
#include <iostream>

template <class Items>
class GArray{
private:
   int theSz;
   int totSz;
   Items *theAry;
public:
   GArray(int sz=2);
   GArray(GArray<Items> *ary);
   ~GArray();

   void copy (GArray<Items> *ary);
   void Realloc(int newlen);
   void compact(int nsz=-1);
   void add(Items it);

   void optadd(Items it){ theAry[theSz++] = it; }
   Items *& garray(){ return theAry; }
   void reset(){ theSz = 0; }
   int& size(){ return theSz; }
   int& totsize(){ return totSz; }
   Items& operator [] (unsigned int index){ return theAry[index]; }
   friend ostream& operator << (ostream& fout, GArray<Items>& ary);
};

#endif //__GArray_H
