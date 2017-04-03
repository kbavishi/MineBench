// Compiler options:-
// -DBALT     = make trees balanced

#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <sys/time.h>
#include <time.h>
#include <sys/types.h>
#include <math.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>
#include <memory.h>
#include <fcntl.h>

#include "Itemset.h"
#include "ListItemset.h"
#include "HashTree.h"
#include "Database.h"
#include "pardhp.h"

#define MAXITER 30

struct timeval tp;
double MIN_UTILITY;
int NUM_INSERT;
int NUM_ACTUAL_INSERT;

char offset_file_name[100],transaction_file_name[100], profit_file_name[100];
float *profit_array, *transaction_utility_array;
int num_trans;
int maxitem;
int avg_trans_sz;

int *max_trans_sz;

float MIN_UTILITY_PER;
FILE *summary;
int *file_offset;

ListItemset *Largelist =NULL;
ListItemset *Total_Cand_Largelist;
double *item_t_utility, **local_item_t_utility, *local_tran_utility;
double total_tran_utility = 0, tran_utility=0;

int nthreads;
int more =1, threshold = 2, tot_cand =0, num_db_scan=2, max_pattern_length;
int *local_tot_cand;
int **hash_pos, **start, **enda;

HashTree *Candidate = NULL;
int *hash_indx = NULL;

#ifdef BALT
void form_hash_indx(int hash_function)
{
   int i, cnt;
   i=0;

   //printf("HASH_FUNCTION = %d\n", hash_function);
   if (hash_function == 1){
      return;
   }

   while(i < maxitem){
      for(cnt = 0; i < maxitem && cnt < hash_function; i++)
         if (hash_indx[i] == 0){
             hash_indx[i] = cnt;
             cnt++;
         }
      for(cnt = hash_function-1;i < maxitem && cnt >= 0; i++)
         if (hash_indx[i] == 0){
            hash_indx[i] = cnt;
            cnt--;
         }
   }
}
#endif

int init_subsets(int *starts, int *endas, int num_item, int k_item)
{
   int i;
     
   if (num_item < k_item) return 0;
         
   for (i=0; i < k_item; i++){
       starts[i] = i;
       endas[i] = num_item - k_item + 1 + i;
   }
   return 1;
}

int get_next_subset(int *starts, int *endas, int k_item)
{
   int i,j;
     
   for (i=k_item-1; i >= 0; i--){
       starts[i]++;
       if (starts[i] < endas[i]){
          for (j=i+1; j < k_item; j++)
              starts[j] = starts[j-1]+1;
          return 1;
       }
   }
   return 0;
}


ListElement * find_in_list_2(Itemset *item, ListElement *head, int pid)
{
   for(;head; head = head->next()){
      Itemset *curr = head->item();
      if (item->item(1) ==curr->item(0)) {
         if (item->item(2) == curr->item(1)) return head;
      }
      else if (item->item(1) < curr->item(0)) return NULL;
   }
   return NULL;
}          

ListElement * find_in_list(Itemset *item, int sz, ListElement *head, int pid)
{
   for(;head; head = head->next()){
      Itemset *curr = head->item();
      for(int i=0; i < sz; i++){
         int it = item->item(start[pid][i]);
         if (curr->item(i) < it)
            break;
         else if (curr->item(i) > it)
            return NULL;
         else if (i==sz-1) return head;
      }
   }
   return NULL;
}

int not_prune(Itemset *curr, int k, ListElement *beg, int pid)
{
   if (k+1 == 3){
      start[pid][0] = 1;
      start[pid][1] = 2;
      if ((beg = find_in_list_2(curr, beg, pid)) == NULL) return 0;
   }
   else{
      int res = init_subsets(start[pid], enda[pid], curr->numitems(), k);
      start[pid][k-2] = curr->numitems()-2;
      start[pid][k-1] = curr->numitems()-1;
      while (res){
         if ((beg = find_in_list(curr, k, beg,pid)) == NULL) return 0;
         res = get_next_subset(start[pid], enda[pid], k);
      }
   }
   return 1;
}

int choose(int n, int k)
{
   int i;
   int val = 1;
   if (k >= 0 && k <= n){
      for (i=n; i > n-k; i--)
          val *= i;
      for (i=2; i <= k; i++)
          val /= i;
   }
   return val;
}
                                             
int get_hash_function(int num, int k){
   int hash = (int)ceil(pow(num/threshold, 1.0/k));
   if (hash < 1) hash = 1;
      return hash;
}

int apriori_gen(int k, int pid){
   int tt;
   long blk = (Largelist->numitems() + nthreads -1)/nthreads;
   int lb = pid*blk;
   int ub = min((pid+1)*blk, Largelist->numitems());

   ListElement *L1iter = Largelist->node(lb);
   for (int i=lb; i < ub && L1iter; i++, L1iter = L1iter->next()){
      Itemset *temp = L1iter->item();
      ListElement *L2iter = L1iter->next();
      for(;L2iter; L2iter = L2iter->next()){
         Itemset *temp2 = L2iter->item();
         if (temp->compare(*temp2,k-2) < 0) break;
         else{
            int k1 = temp->item(k-2);
            int k2 = temp2->item(k-2);
            if (k1 > k2){ 
                int t=k1;
                k1=k2;
                k2=t;
            }
//            printf("k0=%d, k1=%d, k2=%d\n", temp->item(0),k1, k2);
            Itemset *it = new Itemset(k);
            it->set_numitems(k);
            for (int l=0; l < temp->numitems()-1; l++)
               it->add_item(l,temp->item(l));
            it->add_item(k-2, k1);
            it->add_item(k-1, k2);
            ListElement *beg = Largelist->first();
            if(k==2 || not_prune(it, k-1, beg,pid)){
//               printf("i=%d, j=%d, k=%d\n", it->item(0), it->item(1),it->item(2)); 
               tt = Candidate->add_element(*it);
               local_tot_cand[pid]++;
            }
         }
      }
   }
#pragma omp barrier  
}

void increment(Itemset *trans, ListItemset *Clist, int pid, char *tbitvec)
{
   if (Clist->first()){
      ListElement *head = Clist->first();
      for(;head; head = head->next()){
         Itemset *temp = head->item();
         if (temp->subsequence(tbitvec, trans->numitems())){
            omp_set_lock(&(temp->lck));
            temp->incr_t_utility(transaction_utility_array[trans->tid()]);
            omp_unset_lock(&(temp->lck));
         }
      }
   }
}   

void subset(Itemset *trans, int st, int en, int final,
            HashTree* node, int k, int level, int pid,
            char *tbitvec, int *vist, int hash_function)
{
   int i;

   (*vist)++;
   int myvist = *vist;

   if (node == Candidate
      && node->is_leaf() && node->list())
      increment(trans, node->list(), pid, tbitvec);
   else{
      for(i=st; i < en; i++){
#ifdef BALT
         int val = trans->item(i);
         int hashval = hash_indx[val];
         if (hashval == -1) continue;
#else
         int val = trans->item(i);
         if (hash_indx[val] == -1) continue;
         int hashval = node->hash(val);
#endif
         if ((hash_pos[pid])[level*hash_function+hashval] != myvist){
            (hash_pos[pid])[level*hash_function+hashval] = myvist;
            if (node->hash_table_exists() && node->hash_table(hashval)){
               if (node->hash_table(hashval)->is_leaf() &&
                  node->hash_table(hashval)->list())
                  increment(trans, node->hash_table(hashval)->list()
                            , pid,tbitvec);
               else if (en+1 <= final)
                  subset(trans, i+1, en+1, final,node->hash_table(hashval),
                         k, level+1, pid, tbitvec, vist, hash_function);
            }
         }
      }
   }
}

void form_large(HashTree *node,int k)
{
   if (node->is_leaf()){
      ListItemset *list = node->list();
      if(list && list->first()){
         ListElement *iter = list->first();
         for(;iter;iter = iter->next()){
            if (iter->item()->get_t_utility() >=MIN_UTILITY) {
//               printf("item=%d, item=%d, item=%d, %f\n", iter->item()->item(0), iter->item()->item(1), iter->item()->item(2), iter->item()->get_t_utility());
               Largelist->sortedInsert(iter->item());
               for (int j=0; j < iter->item()->numitems(); j++)
               {
                   hash_indx[iter->item()->item(j)] = 0;
               }
            }
         }
      }
   }
   else{
      for(int i=0; i < node->hash_function(); i++)
         if (node->hash_table(i))
            form_large(node->hash_table(i), k);
   }
}


void alloc_mem_for_var(int pid)
{
   start[pid] = new int [MAXITER];
   enda[pid] = new int [MAXITER];
}
      
void main_proc()
{
   itemset2 *t_utilitycnt;
   Itemset *trans;
   int pid, lb, ub,blk,i, j, idx,k, vist, numitem, tid, hash_function;
   char *trans_bitvec;
   itemset2 *cnt_ary;
   HashTree *oldCand=NULL; 
   int last_tot_cand;
   int transaction_file;
   
   item_t_utility = (double *)calloc(maxitem, sizeof(double));
   local_item_t_utility = (double **)calloc(nthreads, sizeof(double *));
   local_tran_utility = (double *)calloc(nthreads, sizeof(double));

   local_tot_cand=(int *) calloc(nthreads, sizeof(int));
   
   omp_set_num_threads(nthreads);
                   
#pragma omp parallel private(k,vist, lb, ub, pid, numitem, tid, i, j, trans_bitvec)
{
   pid = omp_get_thread_num();
   
   max_trans_sz[pid] = Database_readfrom(transaction_file_name, t_utilitycnt, pid, file_offset[pid], file_offset[pid+1]- file_offset[pid]);  
   
   alloc_mem_for_var(pid);

//   fprintf(summary, "Database %s utility_threshold %f num_trans %d min_utility %d thr %d\n", transaction_file_name, MIN_UTILITY_PER, num_trans,MIN_UTILITY, threshold);

#pragma omp barrier
   if (pid==0){
   
      hash_indx = new int [maxitem];
      cnt_ary = t_utilitycnt;
 
      for(i=0; i < maxitem; i++)
         hash_indx[i] = -1;
      for(i=0; i < maxitem-1; i++){
         for (j=0; j < cnt_ary[i].count; j++){
            if (cnt_ary[i].t2[j].t_utility >= MIN_UTILITY){
               hash_indx[i] = 0;
               hash_indx[cnt_ary[i].t2[j].item2] = 0;
               Itemset *it = new Itemset(2);
               it->set_numitems(2);
               it->add_item(0,i);
               it->add_item(1,cnt_ary[i].t2[j].item2);
               it->set_t_utility(cnt_ary[i].t2[j].t_utility);
               Largelist->append(*it);
            }
         }
      }
     
      NUM_INSERT = choose(Largelist->numitems(),2);
//   fprintf(summary, "(2)it = %d ", Largelist->numitems());
printf("k=%d, Largelist->numitems=%d\n", 2, Largelist->numitems());

      if (Largelist->numitems() > 0) max_pattern_length = 2;

      Itemset *temp_2 = new Itemset(2);
      ListElement *L1iter = Largelist->node(0);
      for (int j=0; j <  Largelist->numitems(); j++, L1iter = L1iter->next()){
         Itemset *temp = L1iter->item();
         Itemset *temp_2 = new Itemset(2);
         temp_2->copy(temp);
         Total_Cand_Largelist->append(*temp_2);
      }
                                   
      hash_function = get_hash_function(NUM_INSERT,2);
      Candidate = new HashTree(0, hash_function, threshold);

#ifdef BALT
      more = (NUM_INSERT > 0);
      form_hash_indx(hash_function);
#endif
   }
   
   for (i=0; i<nthreads; i++)
      tot_cand +=local_tot_cand[i];

#pragma omp barrier

//   fprintf (summary," %f ", te-ts);

   blk = (num_trans + nthreads -1)/nthreads;
   lb = pid*blk;
   ub = min((pid+1)*blk, num_trans);

   hash_function = get_hash_function(NUM_INSERT,2);

   more = (Largelist->numitems() > 1);
   last_tot_cand = tot_cand;

   for(k=3;more;k++){
      if (hash_pos[pid]) delete [] hash_pos[pid];
      hash_pos[pid] = new int [(k+1)*hash_function];
      for (i=0; i<(k+1)*hash_function; i++)
         hash_pos[pid][i]=0;
                
      //form new candidates

       local_tot_cand[pid]=0;
       apriori_gen(k,pid);
      
       NUM_ACTUAL_INSERT = Candidate->Count;
             
printf("last=%d, tot_cand=%d, NUM_ACTUAL_INSERT=%d\n", last_tot_cand, tot_cand,NUM_ACTUAL_INSERT);
      if (pid==0) {
         for (i=0; i<nthreads; i++)
            tot_cand +=local_tot_cand[i];
      }
                                    
#pragma omp barrier
      if (pid==0) {
#ifndef EQ
         Largelist->clearlist();
#endif
         if (oldCand) delete oldCand;
      }
#pragma omp barrier      
      more = (last_tot_cand < tot_cand);
      
      trans = new Itemset(max_trans_sz[pid]);
      trans_bitvec = new char[maxitem];
      for (i=0; i<maxitem; i++)
         trans_bitvec[i]=0;

      Dbase_Ctrl_Blk DCB;
      int *buf;
      transaction_file = open(transaction_file_name, O_RDONLY); 

      vist = 1;
      if (more){
         if (pid==0) num_db_scan++;
         init_DCB(DCB, transaction_file);
         reset_database(DCB,file_offset[pid]);
         get_first_blk(DCB);
         for(i=lb; i < ub;i++){
            get_next_trans(DCB, buf, numitem, tid);
            make_Itemset(trans, buf, numitem, tid);
            for (j=0; j < numitem; j++)
               trans_bitvec[trans->item(j)] = 1;
            subset(trans, 0, trans->numitems()-k+1, trans->numitems(),
                   Candidate,
                   k, 0, pid, trans_bitvec, &vist, hash_function);
            for (j=0; j < trans->numitems(); j++)
               trans_bitvec[trans->item(j)] = 0;
         }
      }
#pragma omp barrier

      if (pid==0) {
         for (i=0; i < maxitem; i++) hash_indx[i] = -1;
         form_large(Candidate,k);
         printf("k=%d, Largelist->numitems=%d\n", k, Largelist->numitems());
       
         Itemset *temp_2 = new Itemset(k);
         ListElement *L1iter = Largelist->node(0);
         for (int j=0; j <  Largelist->numitems(); j++, L1iter = L1iter->next()){
            Itemset *temp = L1iter->item();
            Itemset *temp_2 = new Itemset(k);
            temp_2->copy(temp);
            Total_Cand_Largelist->append(*temp_2);
         }
//      fprintf(summary, "(%d)it = %d ", k, Largelist->numitems());
      
         more = (Largelist->numitems() > 1);
         last_tot_cand = tot_cand;

         if (Largelist->numitems() > 0) max_pattern_length = k;

         NUM_INSERT = choose(Largelist->numitems(),2);
#ifdef BALT
         form_hash_indx(get_hash_function(NUM_INSERT,k+1));
#endif
         oldCand = Candidate;
         Candidate = new HashTree(0, get_hash_function(NUM_INSERT,k+1),
                               threshold);
         hash_function = get_hash_function(NUM_INSERT,k+1);
//      fprintf(summary," %f ", t4-t3);
      }
#pragma omp barrier
   }
}    

//   delete []trans_bitvec;
//   delete trans;
   for (i=0; i<maxitem; i++)
      free(t_utilitycnt[i].t2);
   free(t_utilitycnt);
//   close_DCB(DCB);
//   fprintf(summary,"MAIN %f ", t2-t1);
}

void init_var()
{
   int i;
   Largelist = new ListItemset;
   Total_Cand_Largelist = new ListItemset;
        
   start = new int *[nthreads];
   enda = new int *[nthreads];
   hash_pos = new int *[nthreads];
   for (i=0; i < nthreads; i++) hash_pos[i] = NULL;

   max_trans_sz = new int[nthreads];
   for (i=0; i < nthreads; i++) max_trans_sz[i] = 0;

   more = 1;
}

void clean_up(){
   int i;

   delete Candidate;
   if (Largelist) delete Largelist;
   delete [] hash_indx;

   for (i=0; i < nthreads; i++){
      delete start[i];
      delete enda[i];
      delete hash_pos[i];
   }
   delete [] start;
   delete [] enda;
   delete [] hash_pos;
}

int main(int argc, char **argv)
{
    int profit_file;
    summary = fopen("out", "a+");
    int lb, ub, pid, blk,indx, i, k, j, m, n, *buf, numitem, tid;
    float val;
    ListElement *L1iter;
    int total_real_high;
    Itemset *trans=NULL;
    char *tbitvec;
    double t1, t2, t3;
    int  global_max_trans_sz;
    int transaction_file;
    Dbase_Ctrl_Blk DCB;    
    FILE *off_fp; 
    
    if (argc < 6){
        cout << "usage: <transaction_file> <offset_file> <profit_file> <utility_threshold> <number_of_threads>\n";
        exit(3);
    }
     
     strcpy(transaction_file_name, argv[1]);
     strcpy(offset_file_name, argv[2]);
     strcpy(profit_file_name, argv[3]);
     nthreads = atoi(argv[5]);

     transaction_file = open(transaction_file_name, O_RDONLY);
seconds(t1);     
     read(transaction_file, &num_trans, ITSZ);
     read(transaction_file, &maxitem, ITSZ);
     read(transaction_file, &avg_trans_sz, ITSZ);
    
     transaction_utility_array = (float *)calloc(num_trans, sizeof(float));
     profit_array = (float *)calloc(maxitem, sizeof(float));

     profit_file = open(profit_file_name, O_RDONLY);
     read(profit_file, profit_array,sizeof(float)*maxitem);

     file_offset = (int *) calloc(nthreads+1, sizeof(int));
     off_fp = fopen(offset_file_name, "r");

     for (i=0; i<nthreads+1; i++)
        fscanf(off_fp,"%d\n", &(file_offset[i]));
               
     close(profit_file);
     
     MIN_UTILITY_PER = atof(argv[4]);
     
     init_var();
 
     if (more) main_proc();

seconds(t2);     
#ifdef OPTIMAL
     
     for (i=0; i<maxitem; i++) {
        if (item_t_utility[i] >= MIN_UTILITY) {
           Itemset *it = new Itemset(1);
           it->set_numitems(1);
           it->add_item(0,i);
           Total_Cand_Largelist->append(*it);
        }
     }

     global_max_trans_sz = max_trans_sz[0];
     for (i=1; i<nthreads; i++)
        if (global_max_trans_sz < max_trans_sz[i])
           global_max_trans_sz =  max_trans_sz[i];
           
     if (max_pattern_length > 0) {
        num_db_scan++;   
   
   printf("max_pattern_length =%d\n", max_pattern_length);  

        omp_set_num_threads(nthreads);
#pragma omp parallel private(m,i,k,j,n,L1iter,buf,tid, numitem,transaction_file, lb, ub, pid, blk, DCB, trans, tbitvec)
{
        pid = omp_get_thread_num();

        trans = new Itemset(global_max_trans_sz);
        tbitvec = (char *)calloc(maxitem, sizeof(char));
        for (i=0; i < maxitem; i++){
           tbitvec[i] = 0;
        }

        blk = (num_trans + nthreads -1)/nthreads;
        lb = pid*blk;
        ub = min((pid+1)*blk, num_trans);;
        
        transaction_file = open(transaction_file_name, O_RDONLY);
             
        init_DCB(DCB, transaction_file);   
        reset_database(DCB,file_offset[pid]);
        get_first_blk(DCB);

        for(i=lb; i < ub;i++){
           get_next_trans(DCB, buf, numitem, tid);
           make_Itemset(trans, buf, numitem, tid);
           for (j=0; j < trans->numitems(); j++) 
              tbitvec[trans->item(j)] = 1;
           L1iter = Total_Cand_Largelist->node(0);
           for (k=0; k<Total_Cand_Largelist->numitems(); k++, L1iter = L1iter->next()){
              Itemset *temp = L1iter->item();
              if (temp->subsequence(tbitvec, trans->numitems())){
                 for (n=0; n<temp->numitems(); n++) {
                    m=0;
                    while (buf[m]!=temp->item(n))  m=m+2;
                    omp_set_lock(&(temp->lck));
                    temp->incr_utility(profit_array[temp->item(n)] * buf[m+1]);
                    omp_unset_lock(&(temp->lck));
                 }
              }
           }
           for (j=0; j < trans->numitems(); j++)
              tbitvec[trans->item(j)] = 0;
        }     
     #pragma omp barrier
}
     }
    
     n=2;
     total_real_high=0;
     int seg_total_real_high =0;
     L1iter = Total_Cand_Largelist->node(0);
     for (k=0; k<Total_Cand_Largelist->numitems(); k++, L1iter = L1iter->next()){
        if(L1iter->item()->get_utility() >= MIN_UTILITY) {
           if (n==L1iter->item()->numitems()) 
              seg_total_real_high++;
           else {
              printf("length=%d, real_high=%d\n", n, seg_total_real_high);
              seg_total_real_high=1;
              n = L1iter->item()->numitems();
           }
           total_real_high++;
           for (m=0; m<L1iter->item()->numitems(); m++)
               printf("%d, ",L1iter->item()->item(m));

            printf("%f\n", L1iter->item()->get_utility());
        }
     } 
     printf("length=%d, real_high=%d\n", 1, seg_total_real_high);
#endif
    
seconds(t3);    
//     fprintf(summary, "Cands %d ", tot_cand);
       printf("Final tot_cand=%d, total_real_high=%d, num_db_scan=%d\n", tot_cand, total_real_high, num_db_scan);
       printf("Phase I time=%f, total time = %f\n", t2-t1, t3-t1); 
//     fprintf(summary, "Cands %d Total = %f ", tot_cand, te-ts);
     fprintf(summary, "\n");
     fflush(summary);
     fflush(stdout);
//     close_DCB(DCB);
     clean_up();

     return(0);
}


