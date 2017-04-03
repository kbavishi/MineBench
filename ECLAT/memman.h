#ifndef __MEMMAN_H
#define __MEMMAN_H

#include "Itemset.h"
#include "calcdb.h"

class Memman{
private:
public:
   static void read_from_disk(Itemset *iset, int it, 
			      Dbase_Ctrl_Blk *DCB, int alloc_flag=0);
};

#endif //__MEMMAN_H
