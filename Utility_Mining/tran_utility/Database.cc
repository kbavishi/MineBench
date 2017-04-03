#include <fstream>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/time.h>
#include <time.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <unistd.h>
#include <memory.h>
#include <fcntl.h>

#include "pardhp.h"
#include "Database.h"

int DBASE_BUFFER_SIZE = 8192*8;

void init_DCB(Dbase_Ctrl_Blk &DCB, int infile)
{
   DCB.fd = infile;
   DCB.buf_size = DBASE_BUFFER_SIZE;
   DCB.buf = new int [DCB.buf_size];
   DCB.cur_buf_pos = 0;
   DCB.cur_blk_size = 0;
}

void close_DCB(Dbase_Ctrl_Blk &DCB)
{
   delete [] DCB.buf;
}

void reset_database(Dbase_Ctrl_Blk& DCB)
{
   lseek(DCB.fd, 3*ITSZ, SEEK_SET);
   DCB.cur_buf_pos = 0;
   DCB.cur_blk_size = 0;
}

void get_next_trans_ext(Dbase_Ctrl_Blk& DCB, int &numitem, int &tid)
{
   // Need to get more items from file
   int res = DCB.cur_blk_size - DCB.cur_buf_pos+2;
   if (res > 0)
   {
      // First copy partial transaction to beginning of buffer
      //DCB.cur_blk_size -= DCB.cur_buf_pos;
      memcpy(DCB.buf,
              (char *)(DCB.buf + DCB.cur_buf_pos - 2),
              res * ITSZ);
      DCB.cur_blk_size = res;
   }
   else
   {
      // No partial transaction in buffer
      DCB.cur_blk_size = 0;
   }

   DCB.cur_blk_size +=
      (read(DCB.fd,
            (char *)(DCB.buf + DCB.cur_blk_size),
            ((DCB.buf_size - DCB.cur_blk_size)*ITSZ)))/ITSZ;
   
   if (DCB.cur_blk_size > 0)
   {
      tid = DCB.buf[0];
      numitem = DCB.buf[1];
      DCB.cur_buf_pos = 2;   
   }   
}

int Database_readfrom(char *infile, itemset2 * &t_utilitycnt)
{
   int i,j,k,m;
   
   int max_trans_sz = 1;
   int tid, numitem;
   Dbase_Ctrl_Blk DCB;
   int *buf;
   int transaction_file;
   double total_tran_utility = 0, tran_utility=0;
   transaction_file = open(infile,O_RDONLY);
   int maxsize;
   read(transaction_file,buf,ITSZ);
   read(transaction_file,buf,ITSZ);
   read(transaction_file,buf,ITSZ);

   item_t_utility = (double *)calloc(maxitem, sizeof(double));
   int blk = num_trans;
   int lb = 0;
   int ub = num_trans;
   init_DCB(DCB, transaction_file);
   reset_database(DCB);
   get_first_blk(DCB);
   for (i=lb; i < ub; i++){
      get_next_trans(DCB, buf, numitem, tid);
      if (numitem > max_trans_sz) max_trans_sz = numitem;
   
      for (j=0; j < numitem*2-1; j=j+2)
      {
        tran_utility +=buf[j+1]*profit_array[buf[j]]; 
      }
      total_tran_utility += tran_utility;
      transaction_utility_array[tid] = tran_utility;
      for (j=0; j < numitem*2-1; j=j+2)
        item_t_utility[buf[j]] +=tran_utility;
      tran_utility = 0;
   }
   MIN_UTILITY = MIN_UTILITY_PER *total_tran_utility;
 
   int count=0;
   for (i=0; i<maxitem; i++) {
      if (item_t_utility[i] >= MIN_UTILITY) 
          count++;
   }

   printf("level 1 >MIN_UTILITY=%d, %f\n", count, total_tran_utility);

   if (count > 0)  max_pattern_length =1;
   else max_pattern_length = 0;

   int idx=0;
   for (i=0; i< maxitem-1; i++)
      for (j=i+1; j<maxitem; j++) {
         if ((item_t_utility[i] >= MIN_UTILITY) && (item_t_utility[j] >= MIN_UTILITY))
          tot_cand++;
      }

   maxsize = min(maxitem, 1000);
   t_utilitycnt = (itemset2 *)calloc(maxitem, sizeof(itemset2));
   for (i=0; i< maxitem-1; i++) {
      t_utilitycnt[i].size = maxsize;
      t_utilitycnt[i].t2 = (item_2 *)calloc(maxsize, sizeof(item_2));
   }

   reset_database(DCB);
   get_first_blk(DCB);
   for (i=lb; i < ub; i++){
      get_next_trans(DCB, buf, numitem, tid);
      for (j=0; j < numitem*2-1; j=j+2){
         if (item_t_utility[buf[j]] >= MIN_UTILITY) {
            for (k=j+2; k < numitem*2-1; k=k+2){
               if (item_t_utility[buf[k]] >= MIN_UTILITY) {
                  for (m=0; m < t_utilitycnt[buf[j]].count; m++)
                     if (t_utilitycnt[buf[j]].t2[m].item2==buf[k]) {
                        t_utilitycnt[buf[j]].t2[m].t_utility  += transaction_utility_array[tid];
                        break;
                     }
                  if (m==t_utilitycnt[buf[j]].count) {
                     t_utilitycnt[buf[j]].count++;
                     if (t_utilitycnt[buf[j]].count > t_utilitycnt[buf[j]].size) {
                        t_utilitycnt[buf[j]].size += maxsize;
                        t_utilitycnt[buf[j]].t2 = (item_2 *)realloc(t_utilitycnt[buf[j]].t2, t_utilitycnt[buf[j]].size*sizeof(item_2));
                     }
                     t_utilitycnt[buf[j]].t2[m].item2 = buf[k];
                     t_utilitycnt[buf[j]].t2[m].t_utility = transaction_utility_array[tid];
                  }
               }      
            }
         }
      }
   }

//   for(i=0; i < maxitem-1; i++){
//      printf("i=%d, count =%d, size=%d\n", i, t_utilitycnt[i].count, t_utilitycnt[i].size);
//      for (j=0; j < t_utilitycnt[i].count; j++)
//         printf("i=%d, j=%d, t_utility = %f\n", i, t_utilitycnt[i].t2[j].item2, t_utilitycnt[i].t2[j].t_utility);
//   }

   printf("MIN_UTILITY=%f\n", MIN_UTILITY);
   printf("level 2 tot=%d\n", tot_cand);
   return max_trans_sz;
}








