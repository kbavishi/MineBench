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

char transaction_file_name[100], profit_file_name[100];
float *profit_array, *transaction_utility_array;
int num_trans;
int maxitem;
int avg_trans_sz;

int max_trans_sz;

float MIN_UTILITY_PER;
FILE *summary;

ListItemset *Largelist =NULL;
ListItemset *Total_Cand_Largelist;
double *item_t_utility;

int more =1, threshold = 2, tot_cand =0, num_db_scan=2, max_pattern_length;
int *hash_pos, *start, *enda;

HashTree *Candidate = NULL;
int *hash_indx = NULL;

int transaction_file;
Dbase_Ctrl_Blk DCB;

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


ListElement * find_in_list_2(Itemset *item, ListElement *head)
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

ListElement * find_in_list(Itemset *item, int sz, ListElement *head)
{
   for(;head; head = head->next()){
      Itemset *curr = head->item();
      for(int i=0; i < sz; i++){
         int it = item->item(start[i]);
         if (curr->item(i) < it)
            break;
         else if (curr->item(i) > it)
            return NULL;
         else if (i==sz-1) return head;
      }
   }
   return NULL;
}

int not_prune(Itemset *curr, int k, ListElement *beg)
{
   if (k+1 == 3){
      start[0] = 1;
      start[1] = 2;
      if ((beg = find_in_list_2(curr, beg)) == NULL) return 0;
   }
   else{
      int res = init_subsets(start, enda, curr->numitems(), k);
      start[k-2] = curr->numitems()-2;
      start[k-1] = curr->numitems()-1;
      while (res){
         if ((beg = find_in_list(curr, k, beg)) == NULL) return 0;
         res = get_next_subset(start, enda, k);
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

int apriori_gen(int k){
   int tt;
   long blk = Largelist->numitems();
   int lb = 0;
   int ub = blk;
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
            if(k==2 || not_prune(it, k-1, beg)){
//               printf("i=%d, j=%d, k=%d\n", it->item(0), it->item(1),it->item(2)); 
               tt = Candidate->add_element(*it);
               tot_cand++;
            }
         }
      }
   }
   printf(" after tot=%d\n", tot_cand);
   return tt;
}

void increment(Itemset *trans, ListItemset *Clist, char *tbitvec)
{
   if (Clist->first()){
      ListElement *head = Clist->first();
      for(;head; head = head->next()){
         Itemset *temp = head->item();
         if (temp->subsequence(tbitvec, trans->numitems())){
            temp->incr_t_utility(transaction_utility_array[trans->tid()]);
         }
      }
   }
}   

void subset(Itemset *trans, int st, int en, int final,
            HashTree* node, int k, int level,
            char *tbitvec, int *vist, int hash_function)
{
   int i;

   (*vist)++;
   int myvist = *vist;

   if (node == Candidate
      && node->is_leaf() && node->list())
      increment(trans, node->list(), tbitvec);
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
         if (hash_pos[level*hash_function+hashval] != myvist){
            hash_pos[level*hash_function+hashval] = myvist;
            if (node->hash_table_exists() && node->hash_table(hashval)){
               if (node->hash_table(hashval)->is_leaf() &&
                  node->hash_table(hashval)->list())
                  increment(trans, node->hash_table(hashval)->list()
                            , tbitvec);
               else if (en+1 <= final)
                  subset(trans, i+1, en+1, final,node->hash_table(hashval),
                         k, level+1, tbitvec, vist, hash_function);
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

void main_proc()
{
   itemset2 *t_utilitycnt;
   Itemset *trans=NULL;
   int i, j, idx,k, vist, *buf, numitem, tid, hash_function;
   char *trans_bitvec = new char[maxitem];
   itemset2 *cnt_ary;
   HashTree *oldCand=NULL; 
   Dbase_Ctrl_Blk DCB;
   int last_tot_cand;

   max_trans_sz = Database_readfrom(transaction_file_name, t_utilitycnt);  
   trans = new Itemset(max_trans_sz);
 
   transaction_file = open(transaction_file_name, O_RDONLY);
   init_DCB(DCB, transaction_file);
   for (i=0; i < maxitem; i++){
       trans_bitvec[i] = 0;
   }

//   fprintf(summary, "Database %s utility_threshold %f num_trans %d min_utility %d thr %d\n", transaction_file_name, MIN_UTILITY_PER, num_trans,MIN_UTILITY, threshold);
  
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

//ListElement *L1iter = Largelist->node(0);
//for (int j=0; j <  Largelist->numitems(); j++, L1iter = L1iter->next())
//  printf("i=%d, j=%d\n",L1iter->item()->item(0), L1iter->item()->item(1));

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
//   fprintf (summary," %f ", te-ts);

   int blk = num_trans;
   int lb = 0;
   int ub = num_trans;

   hash_function = get_hash_function(NUM_INSERT,2);

   more = (Largelist->numitems() > 1);
   last_tot_cand = tot_cand;
   
   for(k=3;more;k++){
      if (hash_pos) delete [] hash_pos;
      hash_pos = (int *) calloc((k+1)*hash_function, sizeof(int));
      vist = 1;

      //form new candidates

      NUM_ACTUAL_INSERT = apriori_gen(k);

      printf("k=%d, NUM_ACTUAL_INSERT=%d\n", k, NUM_ACTUAL_INSERT);
#ifndef EQ
      Largelist->clearlist();
#endif
      if (oldCand) delete oldCand;
      
      more = (last_tot_cand < tot_cand);
printf("more =%d, last_tot_cand=%d, tot_cand=%d\n", more, last_tot_cand, tot_cand);
      if (more){
         num_db_scan++;
         reset_database(DCB);
         get_first_blk(DCB);
         for(i=lb; i < ub;i++){
            get_next_trans(DCB, buf, numitem, tid);
            make_Itemset(trans, buf, numitem, tid);
            for (j=0; j < trans->numitems(); j++)
                trans_bitvec[trans->item(j)] = 1;
            subset(trans, 0, trans->numitems()-k+1, trans->numitems(),
                   Candidate,
                   k, 0, trans_bitvec, &vist, hash_function);
            for (j=0; j < trans->numitems(); j++)
               trans_bitvec[trans->item(j)] = 0;
         }
      }
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
 
   delete [] trans_bitvec;
   delete trans;
   for (i=0; i<maxitem; i++)
      free(t_utilitycnt[i].t2);
   free(t_utilitycnt);
//   close_DCB(DCB);
//   fprintf(summary,"MAIN %f ", t2-t1);
}

int main(int argc, char **argv)
{
    int transaction_file, profit_file;
    summary = fopen("out", "a+");
    int indx, i, k, j, m, n, *buf, numitem, tid;
    float val;
    ListElement *L1iter;
    int total_real_high;
    Itemset *trans=NULL;
    char *tbitvec;
    double t1, t2, t3;

    if (argc < 4){
        cout << "usage: <transaction_file> <profit_file> <utility_threshold>\n";
        exit(3);
    }
                      
     strcpy(transaction_file_name, argv[1]);
     strcpy(profit_file_name, argv[2]);
    
     transaction_file = open(transaction_file_name, O_RDONLY);
seconds(t1);     
     read(transaction_file, &num_trans, ITSZ);
     read(transaction_file, &maxitem, ITSZ);
     read(transaction_file, &avg_trans_sz, ITSZ);
     
     transaction_utility_array = (float *)calloc(num_trans, sizeof(float));
     profit_array = (float *)calloc(maxitem, sizeof(float));

     profit_file = open(profit_file_name, O_RDONLY);
     
     read(profit_file, profit_array,sizeof(float)*maxitem);
     close(profit_file);
     MIN_UTILITY_PER = atof(argv[3]);
     
     Largelist = new ListItemset;

     Total_Cand_Largelist = new ListItemset;
     
     more =1;

     start = new int [MAXITER];
     enda = new int [MAXITER];
     main_proc();
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
     if (max_pattern_length > 0) {
        num_db_scan++;   
        trans = new Itemset(max_trans_sz);
        tbitvec = (char *)calloc(maxitem, sizeof(char));
        for (i=0; i < maxitem; i++){
           tbitvec[i] = 0;
        }
   printf("max_pattern_length =%d\n", max_pattern_length);  
        init_DCB(DCB, transaction_file);   
        reset_database(DCB);
        get_first_blk(DCB);
        for(i=0; i < num_trans;i++){
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
                    temp->incr_utility(profit_array[temp->item(n)] * buf[m+1]);
                 }
              }
           }
           for (j=0; j < trans->numitems(); j++)
              tbitvec[trans->item(j)] = 0;
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
     printf("length=%d, real_high=%d\n", n, seg_total_real_high);
#endif
    
seconds(t3);    
//     fprintf(summary, "Cands %d ", tot_cand);
printf("Final tot_cand=%d, total_real_high=%d, num_db_scan=%d\n", tot_cand, total_real_high, num_db_scan);
printf("Phase I time=%f, total time = %f\n", t2-t1, t3-t1); 
//     fprintf(summary, "Cands %d Total = %f ", tot_cand, te-ts);
     fprintf(summary, "\n");
     //printf("testing\n");
     fflush(summary);
     fflush(stdout);
     close_DCB(DCB);
     delete Candidate;
     if (Largelist) delete Largelist;
        delete [] hash_indx;

     delete [] start;
     delete [] enda;
     delete [] hash_pos;
     return(0);
}


