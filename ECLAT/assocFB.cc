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
ofstream summary("summary.out", ios::app);
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
int sep_cliques = 1;
double ts, te;
FILE *out;
int maxiter = 1;
long tmpflen=0;
int use_clique = 0;
char use_char_extl2=0;
char ext_l2_pass=0;
char use_simple_l2=0;
char use_diff_f2=0; //use diff while calculating f2; original tidlist 
char use_diff=0; //use diffs after f2
char diff_input=0; //original difflists instead of tidlists
char print_output=0;
char use_chain = 0;
boolean sort_ascend = TRUE;
boolean sort_F2 = TRUE;
boolean use_horizontal=FALSE;

double cliq_time;
double exttime=0;
double L2TIME=0;

double WEAK_MAXIMAL_PER = 1.0;
int bron_kerbosch=0;
int getmem=0;

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
   
   sprintf(tempf, "/tmp/tmp");
   if (argc < 2)
      cout << "usage: assocFB -i<infile> -o<outfile> -s<support>\n";
   else{
      while ((c=getopt(argc,argv,"abcdDCe:fhi:loO:rRs:St:uw:mx:zZ"))!=-1){
         switch(c){
         case 'a':
            use_auto_maxthr = 0;
            break;
         case 'b':
            bron_kerbosch = 1;
            break;
         case 'c':
            use_clique = 1;
            break;
         case 'C':
            use_char_extl2 = 1;
            break;
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
         case 'f':
            sep_cliques = 0;
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
         case 'm':
            getmem = 1;
            break;
         case 'o':
            print_output = 2;
            break;
         case 'O': //give output file name
            OUTF.open(optarg,ios::out);
            print_output = 1;
            break;
         case 'r':
            rhybrid=1;
            break;
         case 'R':
            F1rhybrid=1;
            break;
         case 's':
            MINSUP_PER = atof(optarg);
            break;
         case 'S':
            use_simple_l2=1;
            break;
         case 't':
            MAXIMAL_THRESHOLD = atof(optarg);
            break;
         case 'u':
            use_chain =1;
            break;
         case 'w':
            //collapse cliques when they share "weak_maximal" % of items
            WEAK_MAXIMAL_PER = atof(optarg);
            break;
         case 'x':
            sprintf(tempf, "%s", optarg);
            break;
         case 'z':
            sort_ascend = FALSE;
            break;           
         case 'Z':
            sort_F2 = FALSE;
            break;           
         }
      }
   }
   
   if (use_diff_f2) use_diff = 1;
   if (diff_input){
      use_diff = 1;
      use_diff_f2 = 0;      
   }
   
   if (!use_clique) WEAK_MAXIMAL_PER = 1.0;
   if (WEAK_MAXIMAL_PER > 1.0) WEAK_MAXIMAL_PER = 1.0;
   else if (WEAK_MAXIMAL_PER <= 0.0){
      // indicates that collapse all cliques into one, i.e, equ class
      use_clique = 0;
   }
   
   //if cliques are not used then turn of sep_cliq flag
   if (!use_clique) sep_cliques = 1;
   
   c= open(conf, O_RDONLY);
   if (c < 0){
      perror("ERROR: invalid conf file\n");
      exit(errno);
   }
   read(c,(char *)&DBASE_NUM_TRANS,ITSZ);
   MINSUPPORT = (int)(MINSUP_PER*DBASE_NUM_TRANS+0.5);
   //ensure that support is at least 2
   if (MINSUPPORT < 2) MINSUPPORT = 2;
   read(c,(char *)&DBASE_MAXITEM,ITSZ);
   read(c,(char *)&DBASE_AVG_TRANS_SZ,sizeof(float));
   read(c,(char *)&DBASE_MINTRANS,ITSZ);   
   read(c,(char *)&DBASE_MAXTRANS,ITSZ);
   close(c);
   cout << "CONF " << DBASE_NUM_TRANS << " " << DBASE_MAXITEM << " "
        << DBASE_AVG_TRANS_SZ << flush << endl;
}

int choose(int n, int k)
{
   int i;
   long long val = 1;

   if (k >= 0 && k <= n){
      for (i=n; i > n-k; i--)
         val *= i;
      for (i=2; i <= k; i++)
         val /= i;
   }
   
   return (int) val;
}



void mark_subsets(Itemset *it, int flg)
{
   it->set_max(flg);
   if (it->subsets())
      for (int i=0; i < it->num_subsets(); i++){
         if ((it->subsets())[i]->get_max() != flg)
            mark_subsets((it->subsets())[i], flg);
      }
}

void add_mark_subsets(Itemset *join, Itemset *it1, Itemset *it2, int flg)
{
   join->add_subset(it1);
   join->add_subset(it2);
   it1->set_max(flg);
   mark_subsets(it2, flg);
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

void get_union(Itemset *join, Itemset *it1, Itemset *it2)
{
   int i,j;
   
   num_intersect++;

   int dc2 = it1->support()-MINSUPPORT;

   for (i=0,j=0; i < it2->diff() && j < it1->diff() && join->diff() <= dc2;){
      if (it2->tid(i) > it1->tid(j)){
         join->add_tid(it1->tid(j));
	 join->diff()++;
         j++;
      }
      else if (it2->tid(i) == it1->tid(j)){
         join->add_tid(it2->tid(i));
	 join->diff()++;
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
   for (; j < it1->diff() && join->diff() <= dc2; j++){
     join->add_tid(it1->tid(j));
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

      //cout << "Doing " << it << " " << it2 << " " << item1->support() << " " 
      //   << MINSUPPORT << endl << flush;

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
      //cout << "LArge " << it << " " << it2 << " "
      //     << join->support() << " " << join->diff() << " " 
      //   << item1->support() << " " << item2->support() << endl << flush; 
      if (print_output && !rhybrid) 
         (print_output==1) ? OUTF << *join : cout << *join;
      //if (use_clique) L2->append(join);
      //else if (MAXIMAL_THRESHOLD==1.0) 
      L2->sortedDescend(join, Itemset::supportcmp);
      //else L2->sortedAscend(join, Itemset::supportcmp);
   }
   seconds(t2);
   exttime += (t2-t1);
   
   return L2;
}

void extract_relevant_items(Lists<Itemset *> *l2it, Array *cliq,
                            Lists<Itemset *> *&pmax, Lists<Itemset *> *&oth)
{
   int i;
   double ratio;
   pmax = new Lists<Itemset *>;
   oth = new Lists<Itemset *>;
   ListNodes<Itemset *> *ln;
   
   // cout << "LATT " << l2it->size() << " " << cliq->size() << endl << flush;
   //  ln= l2it->head();
   // cout << "L2 " << *cliq;
   // for (;ln; ln=ln->next()) cout << *ln->item();
   // cout << endl << flush;
   
   ln= l2it->head();
   for (;ln; ln=ln->next()){ 
     for (i=0; i < cliq->size()-1; i++){
       if ((*ln->item())[1] == (*cliq)[i+1]){
         if (use_auto_maxthr){
	   ratio = ((double)ln->item()->support())/MINSUPPORT;
	   if (ratio >= MAXIMAL_THRESHOLD)
	     pmax->sortedDescend(ln->item(), Itemset::supportcmp);
	   else oth->append(ln->item());
	   //cout << "PRRROR " << *ln->item();
         }
         else{
	   ratio = ((double)ln->item()->support())/MINSUPPORT;
	   if (ratio >= MAXIMAL_THRESHOLD)
	     pmax->append(ln->item());
	   else oth->append(ln->item());
         }
	 break;
       }
     }
   }
   //ln = pmax->head();
   //for (; ln; ln=ln->next())
   //   cout << "PMAX " << *ln->item() << flush;
   // ln = oth->head();
   //for (; ln; ln=ln->next())
   //   cout << "OTH " << *ln->item() << flush;
}

void delete_eq_list(Lists<Eqclass *> *eqlist)
{
   ListNodes<Eqclass *> *eqhd = eqlist->head();

   for (;eqhd; eqhd=eqhd->next()){
      delete eqhd->item()->list();
      eqhd->item()->set_list(NULL);
      delete eqhd->item();
   }
   delete eqlist;
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

void process_max(Lists<Itemset *> *pmax, Lists<Itemset *> *oth, int it)
{
   Itemset *join, *it1, *it2, *maxit=NULL;
   int i;
   int orig_pmax_sz = pmax->size();
   ListNodes<Itemset *> *phd, *prev, *chd;
   Lists<Itemset *> chain;
   Itemset *tt1 = new Itemset(1,0);  
   
   phd = pmax->head();
   if (phd && phd->next()){
      it1 = phd->item();
      int initsz = it1->size();
      if (it1->size() == 1 && !it1->get_max())
         Memman::read_from_disk(it1,(*it1)[0],DCB,1);      
      if (use_chain) chain.append(it1);
      prev = phd;
      phd = phd->next();
      for (; phd; ){
         it2 = phd->item();
         if (it2->size() == 1 && !it2->get_max())
            Memman::read_from_disk(it2,(*it2)[0],DCB,1);         
         
         if (it2->size() == 1){           
            int fine = 1;            
            for (i=0; i < it1->size(); i++){                 
               if (!F2Graph->connected(it2->litem(),(*it1)[i])){
                  fine = 0;
                  break;                  
               }
            }
            if (!fine){               
               oth->sortedAscend(it2, Itemset::supportcmp);
               pmax->remove(prev, phd);               
               phd = prev->next();  
               continue;              
            }
         }
         
         
         /*
         if (it2->size() == 1){
            int fine = 0; 
            for (i=0; i < it1->size(); i++){               
               fine = 0;               
               tt1->setitem(0,(*it1)[i]);              
               if (eqgraph[(*it1)[i]] && 
                   eqgraph[(*it1)[i]]->find_element(it2))
                  fine=1;
               else if (eqgraph[(*it2)[0]] &&
                        eqgraph[(*it2)[0]]->find_element(tt1))
                  fine = 1;
               if (!fine){                 
                  break;
               }
            }
            if (!fine){              
               oth->sortedAscend(it2, Itemset::supportcmp);
               pmax->remove(prev, phd);               
               phd = prev->next();  
               continue;              
            }            
         }
         */

         if (use_diff_f2 || diff_input || initsz >= 3) {
            if (use_diff){
               if (!use_chain){
                  join=new Itemset(it1->size()+1,it1->support()-MINSUPPORT+1);
                  get_union(join, it1, phd->item());
               }               
               else{
                  it1 = phd->item();
                  for (chd=chain.head(); chd; chd=chd->next()){
                     if (it1->size() == 1){
                        join = new Itemset
                           (chd->item()->size()+1,
                            chd->item()->support()-MINSUPPORT+1);
                        get_2_diff(join, chd->item(), it1);
                     }
                     else{
                        join = new Itemset 
                           (chd->item()->size()+1, 
                            min(chd->item()->support()-MINSUPPORT+1, 
                                it1->diff()));
                        get_diff(join, chd->item(), it1);
                     }
                     
                     stats.incrcand(chd->item()->size());                 
                     if (join->support() < MINSUPPORT) break;
                     if (chd->next() == NULL)fill_join(join, chd->item(), 
                                                       phd->item());
                     if (it1 != phd->item()) delete it1;
                     if (chd->next() != NULL) it1 = join;
                  }
               }
            }
            else{
               join = new Itemset (it1->size()+1, 
                                   min (it1->support(),
                                        phd->item()->support()));
               get_intersect(join, it1, phd->item());
            }
            
         }
         else{
            if (use_diff){
              if (!use_chain){
                 Itemset *h = pmax->head()->item();                 
                 Itemset *tt = new Itemset(h->size()+1,
                                           h->support()-MINSUPPORT+1);
                 get_2_diff(tt,h,phd->item());
                 if (it1 != h && tt->support() >= MINSUPPORT){
                    join=new Itemset(it1->size()+1,
                                     it1->support()-MINSUPPORT+1);
                    get_union(join, it1, tt);
                    delete tt;                    
                 }
                 else{
                    join = tt;                   
                 }                 
              }              
              else {
                 it1 = phd->item();
                 for (chd=chain.head(); chd; chd=chd->next()){
                    if (chd == chain.head()){                       
                       join = new Itemset 
                          (chd->item()->size()+1, 
                           chd->item()->support()-MINSUPPORT+1);
                       get_2_diff(join, chd->item(), it1);
                    }                    
                    else{
                       join = new Itemset 
                          (chd->item()->size()+1, 
                           min(chd->item()->support()-MINSUPPORT+1,
                               it1->diff()));
                       get_diff(join, chd->item(), it1);
                    }                    
                    stats.incrcand(chd->item()->size());
                    if (join->support() < MINSUPPORT) break;
                    if (chd->next() == NULL)fill_join(join, chd->item(), 
                                                      phd->item());
                    if (it1 != phd->item()) delete it1;
                    if (chd->next() != NULL) it1 = join;
                 }	
              }
	   }
	   else{
              join = new Itemset (it1->size()+1, 
                                  min (it1->support(),
                                       phd->item()->support()));
              get_intersect(join, it1, phd->item());
           }            
         }
         if (!use_chain) stats.incrcand(it1->size());
        if (join->support() >= MINSUPPORT){
           if (use_auto_maxthr){
              if (!use_chain) fill_join(join,it1, phd->item());
           }
           else{
              if (!use_chain){                
                 for (i=0; i < it1->size(); i++)
                    join->add_item((*it1)[i]);
                 join->add_item(phd->item()->litem());
              }              
           }
           //if (print_output)
           //   (print_output==1) ? OUTF << *join : cout << *join;
           //cout << "LER " << *join;
           //NumLargeItemset[join->size()-1]++;
           //if (maxiter < join->size()) maxiter = join->size();
           maxit = join;
           if (use_chain) chain.append(join);
           //eqgraph[it]->largelist()->append(join);
           prev = phd;
           phd = phd->next();
           if (!use_chain && it1->size() > initsz) delete it1;
           it1 = join;           
        }
        else{
           delete join;
           if (orig_pmax_sz == 2) break;
           oth->sortedAscend(phd->item(), Itemset::supportcmp);
           pmax->remove(prev, phd);
           phd = prev->next();
           //if (!rhybrid)
           if (use_auto_maxthr){
              while(phd){
                 oth->sortedAscend(phd->item(), Itemset::supportcmp);
                 pmax->remove(prev, phd);
                 phd = prev->next();
              }
           }
        }
      }
   }
   if (maxit){
      //cout << "MAXIT " << *maxit;
      if (print_output)
         (print_output==1) ? OUTF << *maxit : cout << *maxit;
      if (maxiter < maxit->size()) maxiter = maxit->size();
      NumLargeItemset[maxit->size()-1]++;
      stats.incrlarge(maxit->size()-1);
      stats[maxit->size()-1]->avgtid += maxit->tidsize();
      
      if (!rhybrid && !F1rhybrid){
         for (i=maxit->size()-1; i >= 3; i--){
            NumLargeItemset[i-1] += choose(maxit->size()-1, i-1);
            //stats.incrlarge(i-1,choose(maxit->size()-1, i-1));
         }
         
      }
      delete maxit;
   }
}

Eqclass * get_cand(Lists<Itemset *> *pmax, Itemset *it)
{
   if (it->size() == 1 && !it->get_max())
         Memman::read_from_disk(it,(*it)[0],DCB,1);
   
   //Lists<Eqclass *> *EQ = new Lists<Eqclass *>;
   Eqclass * EQ = new Eqclass;
   Itemset *join;
   ListNodes<Itemset *> *phd = pmax->head();
   TOTMEM = 0;
   for (;phd; phd = phd->next()){
      
      if (phd->item()->size() == 1 && !phd->item()->get_max())
         Memman::read_from_disk(phd->item(),(*phd->item())[0],DCB,1);
     
      
      if (it->size() == 1){         
         if (!F2Graph->connected(it->litem(),phd->item()->litem()))
            continue;         
         /*
         int f = 0;         
         if (eqgraph[(*it)[0]] && eqgraph[(*it)[0]]->find_element(phd->item()))
            f = 1;
         else if ( eqgraph[(*phd->item())[0]] && 
                   eqgraph[(*phd->item())[0]]->find_element(it))
            f = 1;
         if (!f){           
            continue;               
         } 
         */
      }
      
      if (use_diff){
         if (it->size() == 1){
            join = new Itemset (it->size()+1,it->support()-MINSUPPORT+1);
            get_2_diff(join, it, phd->item());             
         }         
         else if ((!use_diff_f2 && !diff_input) && it->size() ==2 ) {
            join = new Itemset (it->size()+1,it->support()-MINSUPPORT+1);
            get_2_diff(join, it, phd->item());
         }         
         else{
            join = new Itemset (it->size()+1, min(it->support()-MINSUPPORT+1,
                                                  phd->item()->diff()));
            get_diff(join, it, phd->item());
         }
      }
      else{
         join = new Itemset (it->size()+1, min(it->support(), 
                                               phd->item()->support()));
         get_intersect(join, it, phd->item()); 
      }
      
      // get_diff(join, it, phd->item());
      stats.incrcand(it->size());
      if (join->support() >= MINSUPPORT){
         TOTMEM += join->tidlist()->totsize();
         fill_join(join, it, phd->item());
         if (print_output && !rhybrid) 
            (print_output==1) ? OUTF << *join : cout << *join;
                 
         //for (int i=0; i < it->size(); i++)
         //   join->add_item((*it)[i]);         
         //join->add_item((*phd->item())[it->size()-1]);
         

         //cout << "\tCANDL " << *join;
         NumLargeItemset[it->size()]++;
         stats.incrlarge(it->size());
         stats[it->size()]->avgtid += join->tidsize();
         //EQ->append(join);
         EQ->sortedDescend(join, Itemset::supportcmp);
         
      }
      else {
         delete join;
      }
   }
   
   if (getmem && TOTMEM>0) cout << TOTMEM*4 << endl;
   return EQ;
}



inline void print_as_iset(Lists<Itemset *> *cand, Itemset *is, int MS)
{
   
   ListNodes<Itemset *> *hd = cand->head();
   
   int sz = hd->item()->size()+cand->size()-1;
   
   NumLargeItemset[sz-1]++;
   stats.incrlarge(sz-1);
   int i, j, eq;
   
   //cout << "LB ";
   
   //for(i=0; i < hd->item()->size(); i++)
   //   cout << " " << (*hd->item())[i];
   //for (; hd; hd=hd->next()){
   //   for (i=0; i < hd->item()->size(); i++){
   //      eq = 0;
   //      for (j=0; j < is->size(); j++)
   //         if ((*hd->item())[i] == (*is)[j]) eq = 1;
   //      if (!eq) cout << " " << (*hd->item())[i];
   //   }  
   //}
   //cout << sz << " " << "SUP " << MS << endl;
   
}

inline int compute_LB(Lists<Itemset *> *cand, Itemset *is)
{
   
   int drop = 0;
   ListNodes<Itemset *> *hd = cand->head();
   Itemset *gg = hd->item();
   ListNodes<Itemset *> *hd2 = hd->next();
   for (; hd2 ; hd2=hd2->next()){
      //if (use_diff) drop += hd2->item()->diff();      
      //else 
      drop += (is->support() - hd2->item()->support());
     //drop += hd2->item()->diff();
   }
   int val = gg->support() - drop;
   //cout << "DROP " << val << endl << flush;
   if (val >= MINSUPPORT)
      print_as_iset(cand, is, val);
   return val;
   
}

void find_rhybrid (Lists<Itemset *> *pmax, Lists<Itemset *> *oth, int it)
{
   int clustersz = pmax->size()+oth->size();
   Eqclass *Candidate;

   //ListNodes<Itemset *> *ln = pmax->head();
   //for (; ln; ln=ln->next())
   //   cout << "PMAX " << *ln->item() << flush;

   process_max(pmax, oth, it);

   //ln = oth->head();
   //for (; ln; ln=ln->next())
   //   cout << "OTH " << *ln->item() << flush;

   if (oth->size() > 0 && clustersz > 2){
      ListNodes <Itemset *> *ohd = oth->head();
      for (;ohd; ohd = ohd->next()){
         Candidate = get_cand(pmax, ohd->item());
         //ln = pmax->head();
         // for (; ln; ln=ln->next())
         //   cout << "PMAX2 " << *ln->item() << flush;
         //pmax->sortedDescend(ohd->item(), Itemset::supportcmp);
         pmax->append(ohd->item());
         //ln = pmax->head();
         //for (; ln; ln=ln->next())
         //   cout << "PMAX3 " << *ln->item() << flush;
         if (Candidate->list()->size() > 1){
            if (compute_LB(Candidate->list(), ohd->item()) < MINSUPPORT){
               Lists<Itemset *> *oth2 = new Lists<Itemset *>;
               find_rhybrid(Candidate->list(), oth2, it);
               delete oth2;
            }
         }        
         //Candidate->clear();
         delete Candidate;
      } 
   }
}

void find_large(Lists<Itemset *> *pmax, Lists<Itemset *> *oth, int it)
{
   double te,ts;
   ListNodes<Itemset *> *hdr1, *hdr2;
   Lists<Eqclass *> *LargeL, *Candidate;
   ListNodes<Eqclass *> *chd, *beg;
   Eqclass *EQ, *tempeq, *eql;
   Itemset *join;
   int *joinary;
   int i, k, iter;
   int LargelistSum=0;
   int more, hpos;
   int first;
   Lists<Itemset *> *pmax2;
   
   //if (MAXIMAL_THRESHOLD == 1.0) 
   process_max(pmax, oth, it);
//    ListNodes<Itemset *> *hh = pmax->head();
//    for (; hh; hh = hh->next())
//       cout << "PMAX " << *hh->item();
//    hh = oth->head();
//    for (; hh; hh = hh->next())
//       cout << "OTH " << *hh->item() << flush;      
   //    if (use_auto_maxthr){
//       //sort the pmax list!!!!!
//       pmax2 = pmax;
//       pmax = new Lists<Itemset *>;
//       hdr1 = pmax2->head();
//       for (; hdr1; hdr1=hdr1->next()){
//          pmax->sortedAscend(hdr1->item(), Itemset::Itemcompare);
//       }
//       delete pmax2;
//    }
   
   iterstat *is;
   
   ListNodes <Itemset *> *ohd = oth->head();
   for (;ohd; ohd = ohd->next()){
      //if (MAXIMAL_THRESHOLD == 1.0){
      seconds(ts);
     Candidate = new Lists<Eqclass*>;
     eql = get_cand(pmax, ohd->item());
     if (eql->list()->head())
       Candidate->append(eql);
      pmax->sortedAscend(ohd->item(), Itemset::supportcmp);
      //   iter=4;
      // }
      //else{
      //tempeq = new Eqclass(pmax);
      //Candidate->append(tempeq);
      //iter = 3;
      //}
      seconds (te);
      //cout << "SSS " << stats.size() << endl;

      stats.incrtime(2,te-ts);
      
      more = 1;
      for (iter=4; more; iter++){
         TOTMEM = 0;
         seconds(ts);
         LargeL = new Lists<Eqclass *>;
         chd = Candidate->head();
         for (; chd; chd=chd->next()){
            hdr1 = chd->item()->list()->head();
            for (; hdr1->next(); hdr1 = hdr1->next()){
               first = 1;
               EQ = NULL;
               hdr2 = hdr1->next();
               for (; hdr2; hdr2=hdr2->next()){
                  join = new Itemset (iter, 0);
                  if (join == NULL){
                     perror("memory exceeded");
                     exit(errno);
                  }
                  fill_join(join, hdr1->item(), hdr2->item());
                  if (sep_cliques) hpos = -1;
                  else hpos = Htab->find(join);
                  //cout << "HPOS " << hpos << " " << *joinary << endl;
                  if (hpos == -1){
                     if (use_diff) 
                        joinary = new int[min(hdr1->item()->support()-
                                              MINSUPPORT+1, 
                                              hdr2->item()->diff())];
		    else joinary = new int[min(hdr1->item()->support(),
					       hdr2->item()->support())];
                     if (joinary == NULL){
                        perror("memory exceeded");
                        exit(errno);
                     }
                     join->tidlist()->garray() = joinary;
		     if (use_diff)
		       join->tidlist()->totsize() = 
                          min(hdr1->item()->support()-MINSUPPORT+1,
                              hdr2->item()->diff());
		     else
                        join->tidlist()->totsize() = 
                           min(hdr1->item()->support(),
                                hdr2->item()->support());
 		       
                     //join->add_subset(hdr1->item());
                     //join->add_subset(hdr2->item());
                     
                     if (use_diff) get_diff(join, hdr1->item(), hdr2->item());
		     else get_intersect(join, hdr1->item(), hdr2->item());

                     stats.incrcand(hdr1->item()->size());
                     //if small then delete the tidlist
                     if (join->support() < MINSUPPORT){
                        delete join->tidlist()->garray();
                        join->tidlist()->garray() = NULL;
                     }
                     else{
                        if (print_output)
                           (print_output==1) ? OUTF << *join : cout << *join;
                        //cout << "LL " << *join;
                        //eqgraph[it]->largelist()->append(join);
                        TOTMEM += min(hdr1->item()->support(),
                                      hdr2->item()->support());
                        NumLargeItemset[iter-1]++;
                        stats.incrlarge(iter-1);
                        stats[iter-1]->avgtid += join->tidsize();
                     }
                     
                     if (!sep_cliques) Htab->add(join);
                  }
                  else{
                     delete join;
                     join = Htab->get_cell(hpos);
                  }
                  if (join->support() >= MINSUPPORT){
                     if (first){
                        first = 0;
                        EQ = new Eqclass();
                        if (EQ == NULL){
                           perror("memory exceeded");
                           exit(errno);
                        }
                     }
                     //EQ->append(join);
		     EQ->sortedAscend(join, Itemset::supportcmp);
                  }
               }
               if (EQ){
                  LargeL->append(EQ);
               }
            }
         }
         if (maxiter < iter) maxiter = iter;
         if (getmem && TOTMEM > 0) cout << TOTMEM*4 << endl;
         beg = LargeL->head();
         LargelistSum = 0;
         for (;beg; beg=beg->next()){
            LargelistSum += beg->item()->list()->size();
         }
         
         if (LargeL->size() > 0)
            more = (((double)LargelistSum/LargeL->size()) > 1.0);
         else more = 0;
         
         if (sep_cliques){
            Candidate->clear();
            delete Candidate;
            Candidate = LargeL;
            if (!more) {
               LargeL->clear();
               delete LargeL;
            }         
         }
         else{
            delete_eq_list (Candidate);
            Candidate = LargeL;
            if (!more) delete_eq_list (LargeL);
         }
         seconds(te);
         //cout << "ITTT " << iter-1 << " " << te-ts << endl;
         stats.incrtime(iter-1, te-ts);
      }
   }
}


void process_class(int it)
{
   Lists<Itemset *> *pmax_l2it, *other_l2it;
   
   //construct new hash_table for large/small itemsets. let c_i by the size
   //of clique i, then max hash table size = sum (2^(c_i)).
   int htabsz = 0;
   ListNodes<Array *> *clhd = eqgraph[it]->clique()->head();
   for (;clhd; clhd=clhd->next()){
      htabsz += (int) pow(2,clhd->item()->size());
   }
   
   if (!sep_cliques)
      Htab = new HashTable(htabsz);
   
   //from 2-itemsets from ext disk
   Lists<Itemset *> *large2it = get_ext_eqclass(it);

   if (use_clique){
      clhd = eqgraph[it]->clique()->head();   
      //process each clique
      for (;clhd; clhd=clhd->next()){
         //cout << "processing clique " << *clhd->item() << endl;
         //construct large k-itemsets, k > 2
         extract_relevant_items(large2it, clhd->item(), pmax_l2it, other_l2it);
         if (getmem){
            TOTMEM = 0;      
            ListNodes<Itemset *> *hh = pmax_l2it->head();
            for (; hh; hh = hh->next())
               TOTMEM += hh->item()->tidlist()->totsize();
            hh = other_l2it->head();
            for (; hh; hh = hh->next())
               TOTMEM += hh->item()->tidlist()->totsize();      
            if (TOTMEM > 0) cout << TOTMEM*4 << endl;
         }
         //if (rhybrid) find_rhybrid(pmax_l2it, other_l2it, it);
         //else find_large(pmax_l2it, other_l2it, it);

         if (rhybrid) find_rhybrid(pmax_l2it, other_l2it, it);
	 else if (MAXIMAL_THRESHOLD!=1.0)find_large(other_l2it, pmax_l2it, it);
	 else find_large(pmax_l2it, other_l2it, it);

         delete pmax_l2it;
         delete other_l2it;
      }
      if (!sep_cliques){
         //delete all itemsets in the hashtable
         Htab->clear_cells();
         delete Htab;
      }
   }
   else{
      other_l2it = new Lists<Itemset *>;
      if (rhybrid) find_rhybrid(large2it, other_l2it, it);
      else if (MAXIMAL_THRESHOLD!=1.0)find_large(other_l2it, large2it, it);
      else find_large(large2it, other_l2it, it);
      delete other_l2it;
   }
   
   //delete all 2-itemsets
   large2it->clear();
   delete large2it;
}

void newApriori()
{
   int i, it;

   if (F1rhybrid){
      Lists<Itemset *> *pmax, *other;       
      pmax = new Lists<Itemset *>;
      other = new Lists<Itemset *>;
      for(i=0; i < F2Graph->size(); i++){
         Itemset *F1= new Itemset(1,0);
         //it = (*F2Graph)[i]->item();        
         it = i;
         
         F1->add_item(it);
         
         F1->support() = partition_get_idxsup((*F2Graph)[i]->item());         
         //cout << "XX " << *F1;
         //pmax->append(F1);   
         pmax->prepend(F1);         
         //pmax->sortedDescend(F1,Itemset::supportcmp);          
      }
      find_rhybrid(pmax, other, it);
      delete other;      
      pmax->clear();
      delete pmax;     
   }
   else{        
      //form large itemsets for each eqclass
      for (i=0; i < DBASE_MAXITEM-1; i++){
         if (eqgraph[i]){
            //cout << "PROCESS " << i << endl;        
            process_class(i);
         }
      }
   }   
}

//builds cliques via dynamic programming. recursively process each covering item
//once the cliques for the cover are formed, simply join the current item "it"
//to each relevant clique of the cover
void process_clique(int it, int *newcov)
{
   int i, csz, clcnt, res;
   Array *cliq;
   ListNodes<Array *> *clhd, *clhd2, *prev;
   
   ListNodes<int *> *cover = eqgraph[it]->cover()->head();
   // for each covering class, build clique
   //if (it == 596) cout << "processing 596 " << *eqgraph[it] << endl;
   for (;cover; cover = cover->next()){
      if (eqgraph[*cover->item()]){
         //if (it == 596) cout << "doing cover " << *cover->item() << endl;
         clcnt = 0;
         clhd = eqgraph[*cover->item()]->clique()->head();
         for (;clhd; clhd = clhd->next()){
            bzero((char *)newcov, eqgraph[it]->num_elements()*sizeof(int));
            //the cover cliques may be larger. extract common subseq bewteen
            //the clique and EQ[it]->elements.
            csz = eqgraph[it]->get_common(clhd->item(), newcov);
            cliq = NULL;
            if (csz > 1){
               clcnt++;
               cliq = new Array(csz+1);
               if (cliq == NULL)
               {
                  perror("memory exceeded : cliq ");
                  exit (errno);
               }
               cliq->add(it);
               for (i=0; i < eqgraph[it]->num_elements(); i++)
                  if (newcov[i])
                     cliq->add(eqgraph[it]->get_element(i));
               //if (it == 596) cout << "cliqqq " << *cliq << endl;
               //eliminate duplicate cliques
               clhd2 = eqgraph[it]->clique()->head();
               prev = NULL;
               for (;clhd2;){
                  res = cliq->subsequence(clhd2->item());
                  if (res == 1)
                  {
                     //cliq is subset of clhd2->item(), remove cliq
                     delete cliq;
                     cliq = NULL;
                     //prev= clhd2;
                     //clhd2=clhd2->next();
                     break;
                  }
                  else if (res == -1){
                     //remove clhd2->item()
                     //if (it == 596) cout << "remove " << *clhd2->item() << endl;
                     eqgraph[it]->clique()->remove(prev,clhd2);
                     delete clhd2->item();
                     delete clhd2;
                     //if (prev == NULL) clhd2 = eqgraph[it]->clique()->head();
                     //else clhd2 = prev->next();
                     break;
                  }
                  else{
                     prev = clhd2;
                     clhd2=clhd2->next();
                  }
               }
               if (cliq){
                  eqgraph[it]->clique()->sortedAscend(cliq, Array::Arraycompare);
                  //if (it == 596) cout << "asppend " << *cliq << endl;
               }
            }
         }
         //if none of the cliques from cover had any elements in common
         //then join it with cover->item() to form a 2 item clique
         if (!clcnt){
            cliq = new Array(2);
            if (cliq == NULL)
            {
               perror("memory exceeded : cliq");
               exit (errno);
            }
            cliq->add(it);
            cliq->add(*cover->item());
            eqgraph[it]->clique()->append(cliq);
         }
         //if (it == 596) cout << *cover->item() << " inter" << *eqgraph[it] << endl;
      }
      else{
         //EQ[cover->item()] doesn't exist, form 2item clique
         cliq = new Array(2);
         if (cliq == NULL)
         {
            perror("memory exceeded : cliq");
            exit (errno);
         }
         cliq->add(it);
         cliq->add(*cover->item());
         eqgraph[it]->clique()->append(cliq);
      }
   }
   //if (it == 596) cout << "FINAL " << *eqgraph[it] << endl;
   
   //eqgraph[it]->setflg(1);
}


void get_common(Array *ar1, Array *ar2, int &com, int &dist, Array *res)
{
   int i,j;

   com = 0;
   for (i=0, j = 0; i < ar1->size() && j < ar2->size();){
      if ((*ar1)[i] > (*ar2)[j]){
         if (res) res->add((*ar2)[j]);
         j++;
      }
      else if ((*ar1)[i] < (*ar2)[j]){
         if (res) res->add((*ar1)[i]);
         i++;
      }
      else{
         if (res) res->add((*ar2)[j]);
         else com++;
         i++;
         j++;
      }
   }
   for (; i < ar1->size(); i++)
      if (res) res->add((*ar1)[i]);
   for (; j < ar2->size(); j++)
      if (res) res->add((*ar2)[j]);
   
   if (res == NULL) dist = ar1->size()+ar2->size()-com;
}

void collapse_cliques(Lists<Array *> *cliql)
{
   ListNodes<Array *> *hd, *hd2, *prv2;
   Array *newcliq;
   int com_el, dist_el;


//    cout << "BEFORE ";
//    hd = cliql->head();
//    for (;hd; hd=hd->next())
//       cout << *hd->item() << "; ";
//    cout << endl;
   
   hd = cliql->head();
   for (;hd; hd=hd->next()){
      prv2 = hd;
      hd2 = hd->next();
      for (; hd2; ){
         get_common(hd->item(), hd2->item(), com_el, dist_el, NULL);
         //cout << "COMM " << com_el << " " << dist_el << " " <<
         //   *hd->item() << *hd2->item() << endl;
         if (((double)com_el)/dist_el >= WEAK_MAXIMAL_PER){
            newcliq = new Array(dist_el);
            get_common(hd->item(), hd2->item(), com_el, dist_el, newcliq);
            cliql->remove(prv2, hd2);
            delete hd2;
            hd2 = prv2->next();
            delete hd->item();
            hd->set_item(newcliq);
         }
         else{
            prv2 = hd2;
            hd2 = hd2->next();
         }
      }
   }
   
//    cout << "AFTER ";
//    hd = cliql->head();
//    for (;hd; hd=hd->next())
//       cout << *hd->item() << "; ";
//    cout << endl;
}


void build_cliques(EqGrNode **eqgraph)
{
   int i,j,k;
   int *newcov = new int [maxeqsize];
   ListNodes<Array *> *clhd, *prev;
   // form maximal cliques for each class
   for (i=DBASE_MAXITEM-1; i >= 0; i--){
      if (eqgraph[i] ){
         //cout << "PROCESS " << i << endl;
         process_clique(i, newcov);
         //apply weak maximality to collapse related cliques 
         if (WEAK_MAXIMAL_PER < 1.0)
            collapse_cliques(eqgraph[i]->clique());
      }
   }

   //delete classes with max cliques of only size 2
   for (i=0; i < DBASE_MAXITEM-1; i++){
      if (eqgraph[i]){
         prev = NULL;
         clhd = eqgraph[i]->clique()->head();
         for (;clhd; ){
            //cout << "CLIQ " << *clhd->item() << endl;
            if (clhd->item()->size() == 2){
               eqgraph[i]->clique()->remove(prev, clhd);
               delete clhd;
               if (prev == NULL) clhd = eqgraph[i]->clique()->head();
               else clhd = prev->next();
            }
            else{
               prev = clhd;
               clhd = clhd->next();
            }
         }
         
         //if there are no cliques left delete class
         clhd = eqgraph[i]->clique()->head();
         if (!clhd){
            delete eqgraph[i];
            eqgraph[i] = NULL;
         }
      }

      // delete elements not contained in any clique
      if (eqgraph[i]){
         bzero((char *)newcov, eqgraph[i]->num_elements()*sizeof(int));
         clhd = eqgraph[i]->clique()->head();
         // for each clique, mark items contained in it
         for (;clhd; clhd=clhd->next()){
            for (j=0,k=0;j<clhd->item()->size() && k<eqgraph[i]->num_elements();){
               if ((*clhd->item())[j] > eqgraph[i]->get_element(k)) k++;
               else if ((*clhd->item())[j] < eqgraph[i]->get_element(k)) j++;
               else{
                  newcov[k]=1;
                  j++;
                  k++;
               }
            }
         }

         //remove unmarked items
         k = eqgraph[i]->num_elements();
         int deln = 0;
         for (j=0; j < k; j++){
            if (!newcov[j]){
               eqgraph[i]->remove_el(j-deln);
               deln++;
            }
         }
      }
   }
   delete [] newcov;
}


//returns cnt > 0 if Child is a cover for Parent.
int get_cover(EqGrNode *Parent, EqGrNode *Child, int *cover, int pos, int val)
{
   int cnt = 0;
   int chdr = 0;
   int phdr = pos;
   int oldval=-1;

   // set the cover for current item (Child).
   for (;phdr < Parent->num_elements() && chdr < Child->num_elements();){
      if (Child->get_element(chdr) < Parent->get_element(phdr))
         chdr++;
      else if (Child->get_element(chdr) > Parent->get_element(phdr)){
         phdr++;
      }
      else{
         //need this to take care of the case EQ[1] = 2 3 4 5 6 7 8,
         //EQ[2] = 3 5 7 8 , EQ[3] = 4 5 6, EQ[4] = 5 6, EQ[5] = 6 8,
         //EQ[6] = 8, EQ[7] = 8. Covers of EQ[1] are 2 3 and 5, ie,
         //the clique are 1235, 1258, 1278, 13456, 1568 :-
         //2 covers 3 5 7 8, and covers 4 5 6, but 5 covers 6 8, both of
         //which are individaully covered by 2 and 3, but 568 is not a
         //subset of 3578 or 456. 
         if (oldval == -1) oldval = cover[phdr];
         else if (oldval != cover[phdr]) cnt++;

         //if not covered already increase cnt.
         if (!cover[phdr] || !cover[pos-1]){
            cnt++;
         }
         cover[phdr] = val;
         chdr++;
         phdr++;
      }
   }

   return cnt;
}

//build the cover graph for each eqclass
void build_eq_cover(EqGrNode **eqgraph)
{
   int i, cnt;
   int gridx, pos;
   int *cover = new int [maxeqsize];
   for (i=0; i < DBASE_MAXITEM-1; i++){
      if (eqgraph[i]){
         bzero((char *)cover, eqgraph[i]->num_elements()*sizeof(int));
         for (pos=0;pos < eqgraph[i]->num_elements(); pos++){
            gridx = eqgraph[i]->get_element(pos);
            //                 cout << i << " GRIDX : " << gridx << " POS :" << pos << " :: ";
            //                 for (int l = 0; l < eqgraph[i]->num_elements(); l++)
            //                    cout << l << "=" << cover[l] << " ";
            //                 cout << endl;
            if (eqgraph[gridx]){
               cnt = get_cover(eqgraph[i], eqgraph[gridx], cover, pos+1, gridx);
               if (cnt > 0 || cover[pos] == 0){
                  int *newpos = new int;
                  *newpos = gridx;
                  if (!cover[pos]) cover[pos]=gridx;
                  eqgraph[i]->add_cover(newpos);
               }
            }
            else if (cover[pos] == 0){
               int *newpos = new int;
               *newpos = gridx;
               cover[pos]=gridx;
               eqgraph[i]->add_cover(newpos);
            }
            
         }
      }
   }
   
   delete [] cover;
}

int connected(int i, int j, int *offsets)
{
   int idx;
   i--;
   j--;
   if (i == j) return 1;
   else if (i < j){
      idx = offsets[i]-i-1;
      if (it2_cnt[idx+j] >= MINSUPPORT){
         cout << "IDX " << idx << " " << i << " " << j << endl << flush;
         return 1;
      }
      else return 0;
   }
   else{
      idx = offsets[j]-j-1;
      if (it2_cnt[idx+i] >= MINSUPPORT){
         cout << "I2DX " << idx << " " << i <<  " " << j << endl << flush;
         return 1;
      }
      else return 0;      
   }
}

void bron_kerbosch_extend(int *old, int ne, int ce, int &c,
                          int *compsub, int *offsets)
{
   int nod, fixp;
   int newne, newce, i, j, count, pos, p, s, sel, minnod;
   int *newary = new int [ce];
   
   minnod = ce;
   i=0;
   nod=0;

   for (i++; i <= ce && minnod != 0; i++){
      p = old[i];
      count =0;
      j=ne;
      for (j++; j <= ce && count < minnod; j++){
         if (!connected(p, old[j], offsets))
         {
            count++;
            pos=j;
         }
      }
      if (count < minnod){
         fixp = p;
         minnod = count;
         if (i <= ne) s = pos;
         else{
            s=i;
            nod=1;
         }
      }
   }
   for(nod=minnod+nod; nod >= 1; nod--){
      p = old[s];
      old[s] = old[ne+1];
      sel = old[ne+1] = p;
      newne = i = 0;
      for (i++; i <= ne; i++){
         if (connected(sel, old[i], offsets)){
            newne++;
            newary[newne] = old[i];
         }
      }
      newce = newne;
      i = ne+1;
      for (i++; i <= ce; i++){
         if (connected(sel, old[i], offsets)){
            newce++;
            newary[newce] = old[i];
         }
      }
      c++;
      compsub[c] = sel;
      if (newce == 0){
         int loc;
         cout << "CLIQUE = ";
         for (loc=1; loc <= c; loc++)
            cout << compsub[loc] << ", ";
         cout << endl;
      }
      else{
         if (newne < newce)
            bron_kerbosch_extend(newary, newne, newce, c, compsub, offsets);
         c--;
         ne++;
         if (nod > 1){
            s = ne;
            look : s++;
            if (connected(fixp, old[s], offsets)) goto look;
         }
      }
   }
   delete newary;
}

void bron_kerbosch_cliq(int *offsets)
{
   int i;
   
   int *ALL = new int[DBASE_MAXITEM+1];
   int *compsub = new int [DBASE_MAXITEM+1];
   
   for (i=1; i <= DBASE_MAXITEM; i++) ALL[i]=i;
   i=0;
   bron_kerbosch_extend(ALL, 0, DBASE_MAXITEM, i, compsub, offsets);
}

void read_files()
{
   int i,j;
   double t1,t2;

   seconds(t1);
   NumLargeItemset = new int [(int)(DBASE_AVG_TRANS_SZ*100)];
   bzero((char *)NumLargeItemset, (int)(sizeof(int)*DBASE_AVG_TRANS_SZ*100));
   
   maxitemsup = make_l1_pass(*DCB);
   NumLargeItemset[0] = Graph::numF1;
   
   if (use_horizontal) maxitemsup =0;

   item1 = new Itemset(1,maxitemsup);
   item2 = new Itemset(1,maxitemsup);
   

   seconds(t2);
   iterstat *is = new iterstat(DBASE_MAXITEM, Graph::numF1, t2-t1);
   stats.add(is);   
   //stats[0]->time = t2-t1;
   //stats.incrcand(0,DBASE_MAXITEM);
   //stats.incrlarge(Graph::numF1);
      
   t1=t2;
   int l2cnt = make_l2_pass(ext_l2_pass,it2f,*DCB);
   seconds(t2);
   L2TIME = t2-t1;
   
   is = new iterstat(DBASE_MAXITEM*(DBASE_MAXITEM-1)/2,l2cnt,t2-t1);
   stats.add(is);   
   
   /*
   it2_fd = open(it2f, O_RDWR);
   if (it2_fd < 1){
      perror("can't open it2 file");
      exit(errno);
   }
   it2flen = lseek(it2_fd,0,SEEK_END);
   lseek(it2_fd,0,SEEK_SET);
   it2_cnt = (int *) mmap((char *)NULL, it2flen, PROT_WRITE|PROT_READ,
                          (MAP_PRIVATE),
                          it2_fd, 0);
   if (it2_cnt == (int *)-1){
      perror("MMAP ERROR");
      exit(errno);
   }

   offsets = new int [DBASE_MAXITEM];
   int offt = 0;
   for (i=DBASE_MAXITEM-1; i >= 0; i--){
      offsets[DBASE_MAXITEM-i-1] = offt;
      offt += i;
   }
   
   seconds(t1);
   eqgraph = new EqGrNode *[DBASE_MAXITEM];
   bzero((char *)eqgraph, DBASE_MAXITEM*sizeof(EqGrNode *));   

   // build eqgraph -- large 2-itemset relations
   int lcnt = 0;
   int idx;
   for (i=0; i < DBASE_MAXITEM-1; i++){
      //cout << "ITEM " << i;
      idx = offsets[i]-i-1;
      lcnt = 0;
      for (j=i+1; j < DBASE_MAXITEM; j++){
         //cout << " " << j << "=" << it2_cnt[idx+j];
        if (it2_cnt[idx+j] >= MINSUPPORT) lcnt++;
      }
      //cout << endl;
      NumLargeItemset[1] += lcnt;
      if (maxeqsize < lcnt) maxeqsize = lcnt;
      if (lcnt > 0){
         eqgraph[i] = new EqGrNode(lcnt);
         lcnt = 0;
         for (j=i+1; j < DBASE_MAXITEM; j++){
           if (it2_cnt[idx+j] >= MINSUPPORT){
               eqgraph[i]->add_element(j,lcnt++);
            }
         }         
      }
   }
   */

   seconds(t1);
   eqgraph = new EqGrNode *[DBASE_MAXITEM];
   bzero((char *)eqgraph, DBASE_MAXITEM*sizeof(EqGrNode *));   
   
   // build eqgraph -- large 2-itemset relations
   int lcnt = 0;
   int idx;
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

   if (use_clique){
      if (bron_kerbosch){
         bron_kerbosch_cliq(offsets);
      }
      else{
         build_eq_cover(eqgraph);   
         //for (i=0; i < DBASE_MAXITEM-1; i++){
         //   if (eqgraph[i]) cout << "COVER" << i << " " << *eqgraph[i];
         //}
         build_cliques(eqgraph);
      //for (i=0; i < DBASE_MAXITEM-1; i++){
         //   if (eqgraph[i]) cout << "CLIQ " << i << " " << *eqgraph[i];
         //}
      }
   }
   else{
      Array *cliq;
      //insert one big clique the same as the eqclass
      for (i=0; i < DBASE_MAXITEM-1; i++){
         if (eqgraph[i]){
            cliq = new Array(eqgraph[i]->num_elements()+1);
            if (cliq == NULL)
            {
               perror("memory exceeded : cliq ");
               exit (errno);
            }
            cliq->add(i);
            for (j=0; j < eqgraph[i]->num_elements(); j++)
               cliq->add(eqgraph[i]->get_element(j));
            eqgraph[i]->clique()->append(cliq);
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
   
   if (MAXIMAL_THRESHOLD < 100.0){
      summary << "MAX";
   }

   if (use_clique) summary << "CLIQ ";
   else if (rhybrid) summary << "RHYBRID ";
   else if (F1rhybrid) summary << "F1RHYBRID ";
   else summary <<  "ECLAT ";
   if (!sep_cliques) summary << "NSEPCLIQ ";
   if (!use_auto_maxthr) summary << "NAUTO ";
   if (diff_input) summary << "DIFFIN ";
   else if (use_diff_f2) summary << "DIFF2 ";
   else if (use_diff) summary << "DIFF ";
   if (use_chain) summary << "CHAIN ";
   
   summary << dataf << " " << MINSUP_PER << " "
          << DBASE_NUM_TRANS << " " << MINSUPPORT << " ";


   if (use_clique) cout << "CLIQ ";
   else if (rhybrid) cout << "RHYBRID ";
   else if (F1rhybrid) cout << "F1RHYBRID ";
   else cout <<  "ECLAT ";
   if (!sep_cliques) cout << "NSEPCLIQ ";
   if (!use_auto_maxthr) cout << "NAUTO ";
   if (diff_input) cout << "DIFFIN ";
   else if (use_diff_f2) cout << "DIFF2 ";
   else if (use_diff) cout << "DIFF ";
   if (use_chain) cout << "CHAIN ";
   
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

   //close(it2_fd);
   //munmap((char *)it2_cnt, it2flen);

   //cout << "CLK RATE " <<  sysconf(_SC_CLK_TCK) << endl;

   if (use_horizontal) DCB->delete_tidbuf();
   else partition_dealloc();
   exit(0);
}



