#include <malloc.h>
#include <stdlib.h>

#include "GArray.h"
#include "Util.h"

#ifdef SGI
/////////Template Instantiation Stuff////////////
#pragma instantiate GArray<int>
#pragma instantiate GArray<GArray<int>*>
////////////////////////////////////////////////
#endif

template <class Items>
void GArray<Items>::Realloc (int newlen)
{
   totSz = Util<Items>::Realloc (newlen, sizeof(Items), theAry);
}
 
template <class Items>
GArray<Items>::GArray(int sz)
{
   totSz = sz;
   theSz=0;
   theAry = NULL;
   if (sz > 0) Util<Items>::Realloc(totSz,sizeof(Items), theAry);
}

template <class Items>
GArray<Items>::GArray(GArray<Items> *ary)
{
   totSz = ary->totSz;
   theSz=ary->theSz;
   theAry = NULL;
   if (theSz > 0){
      Util<Items>::Realloc(totSz,sizeof(Items), theAry);
      for (int i=0; i < theSz; i++)
         theAry[i] = ary->theAry[i];
   }
}

template <class Items>
GArray<Items>::~GArray()
{
   if (theAry) free(theAry);
}

template <class Items>
void GArray<Items>::copy (GArray<Items> *ary)
{
   theSz=ary->theSz;
   for (int i=0; i < theSz; i++) theAry[i] = ary->theAry[i]; 
}


template <class Items>
void GArray<Items>::add(Items it)
{
   if (theSz+1 > totSz){
      Realloc(2*totSz);
   }
   theAry[theSz++] = it;
}
 
template <class Items>
void GArray<Items>::compact(int nsz)
{
   if (nsz == -1)
      Realloc(theSz);
   else{
      Realloc(nsz);
      theSz = nsz;
   }
}

ostream& operator << (ostream& fout, GArray<int>& ary){
   for (int i=0; i < ary.theSz; i++)
      fout << ary.theAry[i] << " ";
   return fout;
}

template<class Items>
ostream& operator << (ostream& fout, GArray<Items>& ary){
   return fout;
}

#ifdef __GNUC__
/////////Template Instantiation Stuff////////////
template class GArray<int>;
template class GArray<GArray<int>*>;
#include "Graph.h"
template class GArray<GrItem *>;
template class GArray<GrNode *>;
class iterstat;
template class GArray<iterstat *>;

////////////////////////////////////////////////
#endif
