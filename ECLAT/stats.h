#ifndef __STATS_H
#define __STATS_H

using namespace std;
#include<iostream>
 
#include "GArray.h"

class iterstat{
public:
   iterstat(int nc=0, int nl=0, double tt=0.0,double aa=0.0):
      numcand(nc),numlarge(nl),time(tt),avgtid(aa){}
   int numcand;
   int numlarge;
   double time;
   double avgtid;
};

class Stats: public GArray<iterstat *>{
public:
   static double tottime;
   static int totcand;
   static int totlarge;  

   Stats(int sz=2): GArray<iterstat *>(sz){
      for (int i=0; i < sz; i++)
         (*this)[i] = NULL;
   }
   
   void add(iterstat *is){
      GArray<iterstat*>::add(is);
      tottime += is->time;
      totcand += is->numcand;
      totlarge += is->numlarge;      
   }
   void setcand(int pos, int cand){
      (*this)[pos]->numcand = cand;
      totcand += cand;
   }
   void incrcand(int pos, int cand=1){
      if (size() <= pos){
         for(int i = size(); i <= pos; i++){
            iterstat *is = new iterstat;
            add(is);
         }
      }
      
      (*this)[pos]->numcand+=cand;
      totcand += cand;
   }
   void setlarge(int pos, int lar){
      
      (*this)[pos]->numlarge = lar;
      totlarge += lar;
   }
   void incrlarge(int pos, int lar=1){
      if (size() <= pos){
         for(int i = size(); i <= pos; i++){
            iterstat *is = new iterstat;
            add(is);
         }
      }
      (*this)[pos]->numlarge+=lar;
      totlarge +=lar;
   }
   void settime(int pos, double time){
      (*this)[pos]->time = time;
      tottime += time;
   }
   void incrtime(int pos, double tt=0.0){
      if (size() <= pos){
         for(int i = size(); i <= pos; i++){
            iterstat *is = new iterstat;
            add(is);
         }
      }
      
      (*this)[pos]->time+=tt;
   }
   friend ostream& operator << (ostream& fout, Stats& stats){
      for (int i=0; i<stats.size(); i++){         
         fout << "[ " << i+1 << " " << stats[i]->numcand << " "
              << stats[i]->numlarge << " " << stats[i]->time << " "
              << stats[i]->avgtid << " ] ";
      }
      fout << "[ TOT " << totcand << " " << totlarge << " " 
           << tottime << " ]";      
      return fout;      
   }
};

extern Stats stats;
#endif

