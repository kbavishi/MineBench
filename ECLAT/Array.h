#ifndef __ARRAY_H
#define __ARRAY_H
using namespace std;
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <iostream>
#include <sys/types.h>
#include <malloc.h>

class Array {
protected:   
   int *theArray;
   unsigned int theSize;
   unsigned int totSize;
   //unsigned int theIncr;
public:
   
   //Array (int sz, int incr);
   Array(int sz, int *ary);
   Array(int sz);
   ~Array();
   
   inline unsigned int size();
   inline void add (int);
   int subsequence(Array * ar);
   //inline void add (int, unsigned int);
   inline void add_ext(int val, int off, int *ary)
   {
      ary[off+theSize] = val;
      theSize++;
   }
   
   inline int item (unsigned int);

   int operator [] (unsigned int index)
   {
      return theArray[index];
   };
   
   inline void setitem(int pos, int val){
      theArray[pos] = val;
   };
   
   inline int totsize()
   {
      return totSize;
   }
   inline void set_totsize(int sz){
      totSize = sz;
   }
   inline void set_size(int sz){
      theSize = sz;
   }
   inline void reset()
   {
      theSize = 0;
   }

   inline int *get_array()
   {
      return theArray;
   }
   inline void set_array(int *ary){
      theArray = ary;
   }
   //int subsequence(Array&);
   //int compare(Array&);
   friend ostream& operator << (ostream& outputStream, Array& arr);
   static int Arraycompare(void * iset1, void *iset2)
   {
      Array *it1 = (Array *) iset1;
      Array *it2 = (Array *) iset2;
      return it1->compare(*it2);
   }
   int compare(Array& ar2);
};

inline int Array::item (unsigned int index) 
{
   return theArray[index];
}

inline unsigned int Array::size() 
{
   return theSize;
}

inline void Array::add (int item)
{
//    if (theSize+1 > totSize){
//       totSize += theIncr;
//       theArray = (int *)realloc(theArray, totSize*sizeof(int));
//       if (theArray == NULL){
//          cout << "MEMORY EXCEEDED\n";
//          exit(-1);
//       }
//    }
   theArray[theSize] = item;
   theSize++;
}

//inline void Array::add (int item, unsigned int index)
//{
//   theArray[index] = item;
//}

#endif //__ARRAY_H
// int Array::subsequence(Array& ar)
// {
//    if (theSize > ar.theSize) return 0;
//    int start = 0;
//    for(int i=0; i < theSize; i++){
//       for(int j=start; j < ar.theSize; j++)
//          if (theArray[i] == ar.theArray[j]){
//             start = j+1;
//             break;
//          }
//       if (j >= ar.theSize) return 0;
//    }
//    return 1;
// }

// int Array::compare(Array& ar2)
// {
//    int len = (theSize > ar2.theSize) ? ar2.theSize:theSize;
//    for(int i=0; i < len; i++){
//       if (theArray[i] > ar2.theArray[i]) return 1;
//       else if (theArray[i] < ar2.theArray[i]) return -1;
//    }
//    if (theSize > ar2.theSize) return 1;
//    else if (theSize < ar2.theSize) return -1;
//    else return 0;
// }




