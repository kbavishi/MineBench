using namespace std;
#include <errno.h>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <fcntl.h>
#include <sys/stat.h>
//#include <sys/mode.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>
#include <sys/types.h>
#include <string.h>
#include <strings.h>
#include <sys/mman.h>
#include <values.h>

#ifdef SGI
//#include <bstring.h>
#endif

#include "Eqclass.h"
#include "Itemset.h"
#include "Lists.h"
#include "HashTable.h"
#include "partition.h"
#include "memman.h"
#include "extl2.h"
#include "assoc.h"
#include "calcdb.h"

#define NONMAXFLG -2
struct timeval tp;
#define min(a, b) ((a) < (b) ? (a) : (b))

//--------------------------------------
#include "stats.h" //MJZ
double Stats::tottime = 0;
int Stats::totcand = 0;
int Stats::totlarge = 0;
Stats stats;
ofstream summary("eclat.summary.out", ios::app);
//--------------------------------------

char hdataf[300];
char dataf[300];
char idxf[300];
char conf[300];
char tempf[300];
char it2f[300];
long TOTMEM=0;

int data_fd, idx_fd, it2_fd;
int idxflen, it2flen;

unsigned long int total_scan=0;
unsigned long int DB_size=0;

#include "Graph.h"
extern Graph *F2Graph;

int *NumLargeItemset;
HashTable *Htab;
int *it2_cnt; //counts for 2-itemsets
int maxitemsup;
int *offsets;
Itemset *item1, *item2; // for use in reading external dbase
int maxeqsize = 1;
EqGrNode** eqgraph;

//default is eclat code
double MAXIMAL_THRESHOLD = MAXFLOAT;


int use_auto_maxthr = 1;
char rhybrid=0;
char F1rhybrid =0;

int num_intersect=0;
double ts, te;
FILE *out;
int maxiter = 1;
long tmpflen=0;
char use_char_extl2=0;
char ext_l2_pass=0;
char use_simple_l2=0;
char use_diff_f2=0; //use diff while calculating f2; original tidlist 
char use_diff=0; //use diffs after f2
char diff_input=0; //original difflists instead of tidlists
char print_output=0;
boolean sort_ascend = TRUE;
boolean sort_F2 = TRUE;
boolean use_horizontal=FALSE;

double cliq_time;
double exttime=0;
double L2TIME=0;

int DBASE_NUM_TRANS;
int DBASE_MAXITEM;
float DBASE_AVG_TRANS_SZ;
int DBASE_MINTRANS; //works only with 1 partition
int DBASE_MAXTRANS;
double MINSUP_PER;
int MINSUPPORT;

extern int num_partitions;

ofstream OUTF;
Dbase_Ctrl_Blk *DCB=NULL;



void parse_args(int argc, char **argv)
{
   extern char * optarg;
   int c;
   FILE *conf_file;
   
   sprintf(tempf, "/tmp/tmp");
   if (argc < 2)
      cout << "usage: assocFB -i<infile> -o<outfile> -s<support>\n";
   else{
      while ((c=getopt(argc,argv,"abcdDCe:fhi:loO:rRs:St:uw:mx:zZ"))!=-1){
         switch(c){
	 case 'd':
	   use_diff = 1;
	   break;
         case 'D':
            diff_input = 1;
            break;            
         case 'e':
            num_partitions = atoi(optarg);
            ext_l2_pass = 1;
            break;            
         case 'h':
            use_horizontal = TRUE;
            break;
         case 'i':
            sprintf(dataf,"%s.tpose", optarg);
            sprintf(idxf,"%s.idx", optarg);
            sprintf(conf,"%s.conf", optarg);
            sprintf(it2f,"%s.2it", optarg);
            sprintf(hdataf,"%s.data", optarg);
            break;
         case 'l':
            use_diff_f2 = 1;
            break;
         case 'o':
            print_output = 2;
            break;
         case 'O': //give output file name
            OUTF.open(optarg,ios::out);
            print_output = 1;
            break;
         case 's':
            MINSUP_PER = atof(optarg);
            break;
         case 'x':
            sprintf(tempf, "%s", optarg);
            break;
         }
      }
   }
   
   if (use_diff_f2) use_diff = 1;
   if (diff_input){
      use_diff = 1;
      use_diff_f2 = 0;      
   }
   
   conf_file= fopen(conf, "r");
   if (conf_file < 0){
      perror("ERROR: invalid conf file\n");
      exit(errno);
   }
   fread(&DBASE_NUM_TRANS,ITSZ, 1, conf_file);
   MINSUPPORT = (int)(MINSUP_PER*DBASE_NUM_TRANS+0.5);
   //ensure that support is at least 2
   if (MINSUPPORT < 2) MINSUPPORT = 2;
   fread(&DBASE_MAXITEM,1,ITSZ,conf_file);
   fread(&DBASE_AVG_TRANS_SZ,sizeof(float), 1, conf_file);
   fread(&DBASE_MINTRANS,ITSZ, 1,conf_file);   
   fread(&DBASE_MAXTRANS,ITSZ,1,conf_file);
   fclose(conf_file);
   cout << "CONF " << DBASE_NUM_TRANS << " " << DBASE_MAXITEM << " "
        << DBASE_AVG_TRANS_SZ << flush << endl;
}

void get_intersect(Itemset *join, Itemset *it1, Itemset *it2)
{
   int i,j;

   num_intersect++;

   
   int dc1 = it1->support()-MINSUPPORT;
   int dc2 = it2->support()-MINSUPPORT;
   int df1=0;
   int df2=0;

   for (i=0,j=0; i < it1->support() && j < it2->support();){
     if (df1 > dc1 || df2 > dc2) break;
      if (it1->tid(i) > it2->tid(j)){
         j++;
         df2++;
      }
      else if (it1->tid(i) == it2->tid(j)){
         join->add_tid(it1->tid(i));
         join->support()++;
         j++;
         i++;                               
      }
      else{
	df1++;
         i++;
      }
   }
}                           

void get_2_diff(Itemset *join, Itemset *it1, Itemset *it2)
{
   int i,j;
   
   num_intersect++;
   int dc2 = it1->support()-MINSUPPORT;
   for (i=0,j=0; i < it1->support() && j < it2->support() 
	  && join->diff() <= dc2;){
      if (it1->tid(i) > it2->tid(j)){
         j++;
      }
      else if (it1->tid(i) == it2->tid(j)){
         j++;
         i++;
      }
      else{
         join->add_tid(it1->tid(i));
	 join->diff()++;
         i++;
      }
   }   
 
   for (; i < it1->support() && join->diff() <= dc2; i++){
     join->add_tid(it1->tid(i));
     join->diff()++;
   }
   join->support() = it1->support() - join->diff();
   //cout << "TT " << join->support() << " " << join->diff() << endl;
   join->tidlist()->compact();  
}


void get_diff(Itemset *join, Itemset *it1, Itemset *it2)
{
   int i,j;
   
   num_intersect++;

   int dc2 = it1->support()-MINSUPPORT;

   for (i=0,j=0; i < it2->diff() && j < it1->diff() && join->diff() <= dc2;){
      if (it2->tid(i) > it1->tid(j)){
         j++;
      }
      else if (it2->tid(i) == it1->tid(j)){
         j++;
         i++;
      }
      else{
         join->add_tid(it2->tid(i));
	 join->diff()++;
         i++;
      }
   }   
   for (; i < it2->diff() && join->diff() <= dc2; i++){
     join->add_tid(it2->tid(i));
     join->diff()++;
   }
   join->support() = it1->support() - join->diff();
   join->tidlist()->compact();  
}

//construct the next set of eqclasses from external disk
Lists<Itemset *>* get_ext_eqclass(int it)
{
   double t1, t2;
   seconds(t1);
  
   int i, it2;
   Itemset *join;
   Lists<Itemset *> *L2 = new Lists<Itemset *>;
   if (L2 == NULL)
   {
      perror("memory exceeded : ext_class ");
      exit (errno);
   }
   Memman::read_from_disk(item1,it,DCB);
   stats[0]->avgtid += item1->support();
   if (diff_input){
      item1->diff() = item1->support();
      item1->support() = DBASE_NUM_TRANS - item1->diff();      
   }
   
   for (i=0; i < eqgraph[it]->num_elements(); i++){
      it2 = eqgraph[it]->get_element(i);
      Memman::read_from_disk(item2,it2,DCB);
      if (diff_input){
         item2->diff() = item2->support();
         item2->support() = DBASE_NUM_TRANS - item2->diff();      
      }

      if (diff_input){
         join = new Itemset(2,min(item1->support()-MINSUPPORT+1,
                                  item2->diff()));
         get_diff(join, item1, item2);
      }      
      else if (use_diff_f2){
         join = new Itemset(2,item1->support()-MINSUPPORT+1);
         get_2_diff(join, item1, item2);
      }      
      else{
         join = new Itemset(2,min(item1->support(),item2->support()));
         get_intersect(join, item1, item2);
      }
      
      join->add_item(it);
      join->add_item(it2);

      stats[1]->avgtid += join->tidsize();

      if (print_output) 
         (print_output==1) ? OUTF << *join : cout << *join;

      L2->sortedDescend(join, Itemset::supportcmp);
   }
   seconds(t2);
   exttime += (t2-t1);
   
   return L2;
}

void fill_join(Itemset *join,  Itemset *hdr1, Itemset *hdr2)
{
   int i,j;
   //cout << "\t" << *hdr1 << flush;
   //cout << *hdr2 << flush;

   for (i=0, j =0; i < hdr1->size() && j < hdr2->size();){
      if ((*hdr1)[i] == (*hdr2)[j]){
         join->add_item((*hdr1)[i]);
         i++;
         j++;
      }
      else if ((*hdr1)[i] < (*hdr2)[j]){
         join->add_item((*hdr1)[i]);
         i++;
      }
      else{
         join->add_item((*hdr2)[j]);
         j++;
      }
   }
   while (i < hdr1->size()){
      join->add_item((*hdr1)[i]);
      i++;
   }
   while (j < hdr2->size()){
      join->add_item((*hdr2)[j]);
      j++;
   }
}

void find_large(Lists<Itemset *> *large2it, int it)
{
   double te,ts;
   ListNodes<Itemset *> *hdr1, *hdr2;
   Lists<Eqclass *> *LargeL, *Candidate;
   ListNodes<Eqclass *> *chd, *beg;
   Eqclass *EQ;
   Itemset *join;
   int iter;
   int LargelistSum=0;
   int more;
   
   Candidate = new Lists<Eqclass*>;
   EQ = new Eqclass(large2it);
   Candidate->append(EQ);
   more = 1;
   
   for (iter=3; more; iter++){
      seconds(ts);
      LargeL = new Lists<Eqclass *>;
      chd = Candidate->head();
      for (; chd; chd=chd->next()){
         hdr1 = chd->item()->list()->head();
         for (; hdr1->next(); hdr1 = hdr1->next()){
            EQ = NULL;
            hdr2 = hdr1->next();
            for (; hdr2; hdr2=hdr2->next()){
               if (use_diff){
                  if (iter == 3 && !use_diff_f2){
                     join = new Itemset (iter, hdr1->item()->support()-
                                         MINSUPPORT+1);
                     get_2_diff(join, hdr1->item(), hdr2->item());
                  }
                  else{
                     join = new Itemset (iter, min(hdr1->item()->support()-
                                                   MINSUPPORT+1, 
                                                   hdr2->item()->diff()));
                     get_diff(join, hdr1->item(), hdr2->item());
                  }
               }
               
               else{
                  join = new Itemset(iter, min(hdr1->item()->support(),
                                               hdr2->item()->support()));   
                  get_intersect(join, hdr1->item(), hdr2->item());
               }
               
               
               stats.incrcand(hdr1->item()->size());

               //if small then delete the tidlist
               if (join->support() < MINSUPPORT){
                  delete join;
               }
               else{
                  fill_join(join, hdr1->item(), hdr2->item());
                  if (print_output)
                     (print_output==1) ? OUTF << *join : cout << *join;
                  NumLargeItemset[iter-1]++;
                  stats.incrlarge(iter-1);
                  if (EQ == NULL) EQ = new Eqclass();
                  EQ->sortedAscend(join, Itemset::supportcmp);
               }
            }
            if (EQ) LargeL->append(EQ);
         }
         
      }

      if (maxiter < iter) maxiter = iter;
      beg = LargeL->head();
      LargelistSum = 0;
      for (;beg; beg=beg->next()){
         LargelistSum += beg->item()->list()->size();
      }
      
      if (LargeL->size() > 0)
         more = (((double)LargelistSum/LargeL->size()) > 1.0);
      else more = 0;
      
      Candidate->clear();
      delete Candidate;
      Candidate = LargeL;
      if (!more) {
         LargeL->clear();
         delete LargeL;
      }         
      seconds(te);
      //cout << "ITTT " << iter-1 << " " << te-ts << endl;
      stats.incrtime(iter-1, te-ts);
   }
}


void process_class(int it)
{
   //from 2-itemsets from ext disk
   Lists<Itemset *> *large2it = get_ext_eqclass(it);
   find_large(large2it, it);
}

void newApriori()
{
   int i;

   //form large itemsets for each eqclass
   for (i=0; i < DBASE_MAXITEM-1; i++){
      if (eqgraph[i]){
         //cout << "PROCESS " << i << endl;        
         process_class(i);
      }
   }   
}


void read_files()
{
   int i,j;
   double t1,t2;
   seconds(t1);
   NumLargeItemset = (int *) calloc((int)(DBASE_AVG_TRANS_SZ*100), sizeof(int));
//   NumLargeItemset = new int [(int)(DBASE_AVG_TRANS_SZ*100)];
//   bzero((char *)NumLargeItemset, (int)(sizeof(int)*DBASE_AVG_TRANS_SZ*100));
   maxitemsup = make_l1_pass(*DCB);
   NumLargeItemset[0] = Graph::numF1;
   if (use_horizontal) maxitemsup =0;

   item1 = new Itemset(1,maxitemsup);
   item2 = new Itemset(1,maxitemsup);
   seconds(t2);
   iterstat *is = new iterstat(DBASE_MAXITEM, Graph::numF1, t2-t1);
   stats.add(is);   
   t1=t2;
   int l2cnt = make_l2_pass(ext_l2_pass,it2f,*DCB);
   seconds(t2);
   L2TIME = t2-t1;
   is = new iterstat(DBASE_MAXITEM*(DBASE_MAXITEM-1)/2,l2cnt,t2-t1);
   stats.add(is);   
   seconds(t1);
   eqgraph = (EqGrNode **) calloc(DBASE_MAXITEM, sizeof(EqGrNode *));
//   eqgraph = new EqGrNode *[DBASE_MAXITEM];
//   bzero((char *)eqgraph, DBASE_MAXITEM*sizeof(EqGrNode *));   
   // build eqgraph -- large 2-itemset relations
   int lcnt = 0;
   int it1, it2;
   GrNode *grn;  
   for (i=0; i < Graph::numF1; i++){
      grn = (*F2Graph)[i];     
      //it1 = grn->item();     
      it1 = i;      
      lcnt = grn->size();
      NumLargeItemset[1] += lcnt;      
      if (maxeqsize < lcnt) maxeqsize = lcnt;
      if (lcnt > 0){
         eqgraph[it1] = new EqGrNode(lcnt);
         lcnt = 0;
         for (j=0; j < grn->size(); j++){
	   //it2 = (*F2Graph)[(*grn)[j]->adj()]->item();           
            it2 = (*grn)[j]->adj();            
            //cout << "ADD " << it1 << " "<< it2 << endl;
            
            eqgraph[it1] -> add_element(it2,lcnt++); 
         }         
      }
   }

   seconds(t2);
   cliq_time = t2-t1;
   cout << "MAXEQSZIE " << maxeqsize << " " << t2-ts << endl;
}

int main(int argc, char **argv)
{
   int i;
   
   seconds(ts);
   parse_args(argc, argv);
   if (use_horizontal) DCB =  new Dbase_Ctrl_Blk(hdataf);
   else{
     partition_alloc(dataf, idxf);
   }
   read_files();
   newApriori();
   summary <<  "ECLAT ";
   if (diff_input) summary << "DIFFIN ";
   else if (use_diff_f2) summary << "DIFF2 ";
   else if (use_diff) summary << "DIFF ";
   
   summary << dataf << " " << MINSUP_PER << " "
          << DBASE_NUM_TRANS << " " << MINSUPPORT << " ";


   cout <<  "ECLAT ";
   if (diff_input) cout << "DIFFIN ";
   else if (use_diff_f2) cout << "DIFF2 ";
   else if (use_diff) cout << "DIFF ";
   
   cout << dataf << " " << MINSUP_PER << " "
          << DBASE_NUM_TRANS << " " << MINSUPPORT << endl;

   //fprintf(out, "%s %f %d %f %d %f %f %f : ", dataf, MINSUP_PER,
   //        MINSUPPORT, MAXIMAL_THRESHOLD, num_intersect, cliq_time, 
   //        exttime, L2TIME);

   for (i=0; i < maxiter; i++){
      cout << "ITER " <<  i+1 << " " << NumLargeItemset[i] << endl;
   }
   seconds(te);
   cout << "Total elapsed time " << te-ts
        << ", NumIntersect " << num_intersect <<"\n";

   //fprintf(out, "%f \n", te-ts);
   for (i=0; i < stats.size(); i++)
      stats[i]->avgtid /= stats[i]->numlarge;

   stats.tottime = te-ts;
   summary << stats << " " << num_intersect << " " 
           << MAXIMAL_THRESHOLD << " " << cliq_time << " " 
           << DB_size << " " << total_scan;
   struct rusage ruse;  
   getrusage(RUSAGE_SELF,&ruse);
   summary << " " << getsec(ruse.ru_utime) << " " << getsec(ruse.ru_stime) << endl;
  
   summary.close();

   if (use_horizontal) DCB->delete_tidbuf();
   else partition_dealloc();
   exit(0);
}



