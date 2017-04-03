#include "assoc.h"
#include "memman.h"
#include "partition.h"
#include "Graph.h"

extern unsigned long int total_scan;
extern Graph *F2Graph;

void Memman::read_from_disk(Itemset *iset, int it, 
			    Dbase_Ctrl_Blk *DCB, int alloc_flag )
{
  int supsz, itx;
  //if (use_horizontal){
  //  itx = DCB->freqidx[it];
  //  if (itx == -1) return;
  //}
//else 
    itx = (*F2Graph)[it]->item();
  
   if (use_horizontal) supsz = DCB->tidlists[it]->size();
   else supsz = partition_get_idxsup(itx);
   total_scan += supsz;

   if (use_horizontal){
     iset->tidlist()->garray() = DCB->tidlists[it]->get_array();
   }
   else{
     if (alloc_flag){
       iset->tidlist() = new GArray<int>(supsz);
     }       
     partition_read_item(iset->tidlist()->garray(), itx);
   }
   iset->set_support(supsz);
   iset->set_max(1);   
   //cout << "SET SUP " << it << " " << itx << " " << supsz << endl;
}
