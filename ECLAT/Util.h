#ifndef _UTIL_H
#define _UTIL_H

#define boolean char
typedef int (*CMPFUNC) (const void * a, const void * b);

template <class Items>
class Util{
public:
   static boolean Bsearch(int min, int max, Items *itary, Items it,
                          CMPFUNC cfunc, int& ret);
   static int Realloc (int newlen, int elsz, Items*&ary);
};

int Choose(int n, int k);
#endif //_UTIL_H
