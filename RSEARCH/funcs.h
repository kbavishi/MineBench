#include "squid.h"
#include "msa.h"
#include "structs.h"
#include "rnamat.h"

/* from alphabet.c
 */
extern char   SymbolIndex(char sym);
extern void   SingletCount(float *counters, char symidx, float wt);
extern void   PairCount(float *counters, char syml, char symr, float wt);
extern float  DegeneratePairScore(float *esc, char syml, char symr);
extern float  DegenerateSingletScore(float *esc, char sym);
extern char  *DigitizeSequence(char *seq, int L);
extern void   Digitize_Sequence(char *seq, int L, char *d_seq);
extern char **DigitizeAlignment(char **aseq, int nseq, int alen);

/* from cm.c
 */
extern CM_t *CreateCM(int nnodes, int nstates);
extern CM_t *CreateCMShell(void);
extern void  CreateCMBody(CM_t *cm, int nnodes, int nstates);
extern void  CMZero(CM_t *cm);
extern void  CMRenormalize(CM_t *cm);
extern void  FreeCM(CM_t *cm);
extern void  CMSetDefaultNullModel(CM_t *cm);
extern void  CMSimpleProbify(CM_t *cm);
extern void  CMLogoddsify(CM_t *cm);
extern void  CMHackInsertScores(CM_t *cm);
extern int   CMCountStatetype(CM_t *cm, char type);
extern int   CMSegmentCountStatetype(CM_t *cm, int r, int z, char type);
extern int   CMSubtreeCountStatetype(CM_t *cm, int v, char type);
extern int   CMSubtreeFindEnd(CM_t *cm, int v);
extern int   CalculateStateIndex(CM_t *cm, int node, char utype);
extern int   TotalStatesInNode(int ndtype);
extern int   SplitStatesInNode(int ndtype);
extern int   InsertStatesInNode(int ndtype);
extern void  PrintCM(FILE *fp, CM_t *cm);
extern void  SummarizeCM(FILE *fp, CM_t *cm);
extern char *Statetype(int type);
extern int   StateCode(char *s);
extern char *Nodetype(int type);
extern int   NodeCode(char *s);
extern char *UniqueStatetype(int type);
extern int   UniqueStateCode(char *s);
extern int   DeriveUniqueStateCode(int ndtype, int sttype);
extern CM_t *CMRebalance(CM_t *cm);

/* from cmio.c
 */
extern CMFILE *CMFileOpen(char *cmfile, char *env);
extern int     CMFileRead(CMFILE *cmf, CM_t **ret_cm);
extern void    CMFileClose(CMFILE *cmf);
extern void    CMFileRewind(CMFILE *cmf);
extern int     CMFilePositionByIndex(CMFILE *cmf, int idx);
extern int     CMFilePositionByKey(CMFILE *cmf, char *key);
extern void    CMFileWrite(FILE *fp, CM_t *cm, int do_binary);

/* from display.c
 */
extern Fancyali_t    *CreateFancyAli(Parsetree_t *tr, CM_t *cm, CMConsensus_t *cons, char *dsq);
extern void           PrintFancyAli(FILE *fp, Fancyali_t *ali);
extern void           FreeFancyAli(Fancyali_t *ali);
extern CMConsensus_t *CreateCMConsensus(CM_t *cm, float pthresh, float sthresh);
extern void           FreeCMConsensus(CMConsensus_t *con);

/* from modelconfig.c
 */
extern void ConfigLocal(CM_t *cm, float p_internal_start, float p_internal_exit);


/* from modelmaker.c
 */
extern void HandModelmaker(MSA *msa, char **dsq, int use_rf, float gapthresh, 
			   CM_t **ret_cm, Parsetree_t **ret_mtr);
extern Parsetree_t *Transmogrify(CM_t *cm, Parsetree_t *gtr, 
				 char *dsq, char *aseq, int alen);


/* from parsetree.c
 */
extern Parsetree_t *CreateParsetree(void);
extern void         GrowParsetree(Parsetree_t *tr);
extern void         FreeParsetree(Parsetree_t *tr);
extern int          InsertTraceNode(Parsetree_t *tr, int y, int whichway, 
				    int emitl, int emitr, int state);
extern void         ParsetreeCount(CM_t *cm, Parsetree_t *tr, char *seq, float wgt);
extern float        ParsetreeScore(CM_t *cm, Parsetree_t *tr, char *dsq);
extern void         PrintParsetree(FILE *fp, Parsetree_t *tr);
extern void         ParsetreeDump(FILE *fp, Parsetree_t *tr, CM_t *cm, char *dsq);
extern void         ParsetreeCompare(Parsetree_t *t1, Parsetree_t *t2);
extern void         SummarizeMasterTrace(FILE *fp, Parsetree_t *tr);
extern void         MasterTraceDisplay(FILE *fp, Parsetree_t *mtr, CM_t *cm);


/* from rna_ops.c
 */
extern int KHS2ct(char *ss, int len, int allow_pseudoknots, int **ret_ct);
extern int IsCompensatory(float *pij, int symi, int symj);


/* from smallcyk.c
 */
extern float CYKDivideAndConquer(CM_t *cm, char *dsq, int L, int r0, int i0, int j0,
				      Parsetree_t **ret_tr);
extern float CYKInside(CM_t *cm, char *dsq, int L, int r, int i0, int j0, Parsetree_t **ret_tr);
extern float CYKInsideScore(CM_t *cm, char *dsq, int r, int i0, int j0, int L);
extern void  CYKDemands(CM_t *cm, int L);
extern int   CYKDeckCount(CM_t *cm);



/* from buildcm.c
 */
extern CM_t *build_cm (MSAFILE *queryfp, fullmat_t *fullmat, int *querylen,
		       float alpha, float beta, float alphap, float betap,
		       float beginsc, float endsc);
extern CM_t *read_cm (char *queryfile);

/* from scancyk.c 
 */
#define INIT_RESULTS 100

extern scan_results_t *CreateResults (int size);
extern void ExpandResults (scan_results_t *r, int additional);
extern void FreeResults (scan_results_t *r);
extern void SFree_Results (scan_results_t *r);
extern void sort_results (scan_results_t *results);
extern void report_hit (int i, int j, int bestr, float score, scan_results_t *results);
extern void remove_overlapping_hits (scan_results_t *results, int L);
extern void  remove_extremely_overlapping_hits (scan_results_t *results);
extern float CYKScan (CM_t *cm, char *dsq, int L, float cutoff, int D,
		      scan_results_t *results);
