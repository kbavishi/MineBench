#include <errno.h>
#include "Array.h"


Array::Array (int sz, int *ary){
  totSize = sz;
  theSize = 0;
  theArray = ary;
}

Array::Array (int sz){
   totSize = sz;
   theSize = 0;
   //theIncr = incr;
   theArray = NULL;
   if (sz > 0){
      theArray =  new int[totSize];
      if (theArray == NULL){
         perror("memory:: Array");
         exit(errno);
      }
   }
}

Array::~Array(){
   if (theArray) delete [] theArray;
   theArray = NULL;
}

ostream& operator << (ostream& outputStream, Array& arr){
   for (int i=0; i < arr.theSize; i++)
      outputStream << arr[i] << " ";
   return outputStream;
}

// int Array::subsequence(Array * ar)
// {
//    int i,j;
//    if (theSize > ar->theSize) return 0;
//    int start = 0;
//    for(i=0; i < theSize; i++){
//       for(j=start; j < ar->theSize; j++){
//          if (theArray[i] == (ar->theArray)[j]){
//             start = j+1;
//             break;
//          }
//       }
//       if (j >= ar->theSize) return 0;
//    }
//    return 1;
// }
//return 0 if none is a subseq of other, 1 is this is subseq of ar, and -1 otw.
int Array::subsequence(Array * ar)
{
   int i,j;
   int sz1, sz2;
   Array *ar1, *ar2;
   int retval;
   
   if (theSize <= ar->theSize){
      sz1 = theSize;
      sz2 = ar->theSize;
      ar1 = this;
      ar2 = ar;
      retval = 1;
   }
   else{
      sz1 = ar->theSize;
      sz2 = theSize;
      ar1 = ar;
      ar2 = this;
      retval = -1;
   }
   int start = 0;
   for(i=0; i < sz1; i++){
      for(j=start; j < sz2; j++){
         if ((ar1->theArray)[i] == (ar2->theArray)[j]){
            start = j+1;
            break;
         }
      }
      if (j >= ar2->theSize) return 0;
   }
   return retval;
}


int Array::compare(Array& ar2)
{
   int len;
   if (size() <= ar2.size()) len = size();
   else len = ar2.size();
   for(int i=0; i < len; i++){
      if ((theArray)[i] > (ar2.theArray)[i]) return 1;
      else if ((theArray)[i] < (ar2.theArray)[i]) return -1;
   }
   if (size() < ar2.size()) return -1;
   else if (size() > ar2.size()) return 1;
   else return 0;
}
