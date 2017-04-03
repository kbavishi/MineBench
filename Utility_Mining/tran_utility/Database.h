#ifndef __DATABASE_H
#define __DATABASE_H

#include <fstream>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include "ListItemset.h"
#include "pardhp.h"
#include "HashTree.h"

#define ITSZ sizeof(int)

struct Dbase_Ctrl_Blk
{
   int fd;     
   int buf_size;
   int * buf;
   int cur_blk_size; 
   int cur_buf_pos;  
};

int Database_readfrom(char *,itemset2 *&);
void reset_database(Dbase_Ctrl_Blk&);
void get_next_trans_ext(Dbase_Ctrl_Blk& DCB, int &numitem, int &tid);
void close_DCB(Dbase_Ctrl_Blk &DCB);
void init_DCB(Dbase_Ctrl_Blk &DCB, int infile);

inline void get_first_blk(Dbase_Ctrl_Blk& DCB)
{
   DCB.cur_blk_size = (read(DCB.fd,(char *)DCB.buf,
                            (DCB.buf_size*ITSZ)))/ITSZ;
   DCB.cur_buf_pos = 2;
}

inline void get_next_trans(Dbase_Ctrl_Blk& DCB, int * &buf, int &numitem, int &tid)
{
   numitem = DCB.buf[DCB.cur_buf_pos-1];
   tid = DCB.buf[DCB.cur_buf_pos-2];
   if ((DCB.cur_buf_pos + numitem*2 + 2) > DCB.cur_blk_size)
   {
      // Need to get more items from file
      get_next_trans_ext(DCB, numitem, tid);
   }
   buf = DCB.buf + DCB.cur_buf_pos;

   DCB.cur_buf_pos += numitem*2 + 2;
}

inline void make_Itemset(Itemset *it, int *buf, int numitem, int tid)
{
   int j, i;
   
   it->set_tid(tid);
   it->set_numitems(numitem);
   for (j=0, i=0; j < numitem; j++, i=i+2) {
      it->add_item(j, buf[i]);
   }
}
#endif //__DATABASE_H

