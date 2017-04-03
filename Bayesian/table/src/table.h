/*----------------------------------------------------------------------
  File    : table.h
  Contents: tuple and table management
  Author  : Christian Borgelt
  History : 06.03.1996 file created
            07.03.1996 function tab_copy and tuple functions added
            23.07.1996 tuple and table management redesigned
            26.02.1997 functions tab_sort and tab_search added
            27.02.1997 function tab_reduce added
            28.03.1997 function tpl_weight added, INSTFN added
            29.03.1997 several functions added or extended
            07.08.1997 function tab_tplexg added
            29.01.1998 function tpl_fromas added
            03.02.1998 functions tab_sort and tab_secsort merged
            24.02.1998 function tab_shuffle added
            29.05.1998 get/set functions splitted
            22.06.1998 deletion function moved to function tab_create
            27.07.1998 function tab_tplmove added
            16.08.1998 several functions added
            25.09.1998 first step of major redesign completed
            01.10.1998 table names added
            29.11.1998 functions tpl_dup, tab_dup, and tab_filluv added
            04.02.1999 long int changed to int
            13.02.1999 function tab_balance added
            12.03.1999 function tab_opc added
            15.03.1999 one point coverage functions transf. from opc.c
            16.03.1999 function tpl_hash added
            03.04.1999 parameter 'dupas' added to function tab_dup
            17.04.1999 function tab_getwgt added
            21.10.1999 definition of flag TAB_NORM (for tab_opc) added
            16.01.2003 functions tpl_compat, tab_poss, tab_possx added
            22.07.2003 function tab_colnorm added
----------------------------------------------------------------------*/
#ifndef __TABLE__
#define __TABLE__
#include "attset.h"

/*----------------------------------------------------------------------
  Preprocessor Definitions
----------------------------------------------------------------------*/
#define UINT        unsigned int /* abbreviation */

/* --- cut/copy flags --- */
#define TAB_ALL     AS_ALL       /* cut/copy all      columns/tuples */
#define TAB_RANGE   AS_RANGE     /* cut/copy range of columns/tuples */
#define TAB_MARKED  AS_MARKED    /* cut/copy marked   columns/tuples */
#define TAB_SELECT  AS_SELECT    /* cut/copy selected columns/tuples */

/* --- one point coverage flags --- */
#define TAB_COND    0x0000       /* compute condensed form */
#define TAB_FULL    0x0001       /* fully expand unknown values */
#define TAB_NORM    0x0002       /* normalize one point coverages */

/*----------------------------------------------------------------------
  Type Definitions
----------------------------------------------------------------------*/
typedef struct {                /* --- tuple --- */
  ATTSET        *attset;        /* underlying attribute set */
  struct _table *table;         /* containing table (if any) */
  int           id;             /* identifier (index in table) */
  int           mark;           /* mark,   e.g. to indicate usage */
  float         weight;         /* weight, e.g. number of occurrences */
  INST          info;           /* additional information */
  INST          cols[1];        /* columns (holding attribute values) */
} TUPLE;                        /* (tuple) */

typedef void TPL_DELFN (TUPLE *tpl);
typedef void TPL_APPFN (TUPLE *tpl, void *data);
typedef int  TPL_SELFN (const TUPLE *tpl, void *data);
typedef int  TPL_CMPFN (const TUPLE *t1, const TUPLE *t2, void *data);

typedef struct _table {         /* --- table --- */
  char          *name;          /* table name */
  ATTSET        *attset;        /* underlying attribute set */
  int           tplvsz;         /* size of tuple vector */
  int           tplcnt;         /* number of tuples */
  TUPLE         **tpls;         /* tuple vector */
  TPL_DELFN     *delfn;         /* tuple deletion function */
  INST          info;           /* additional information */
  int           *marks;         /* marker buffer for internal use */
  TUPLE         **buf;          /* tuple  buffer for internal use */
} TABLE;                        /* (table) */

/*----------------------------------------------------------------------
  Tuple Functions
----------------------------------------------------------------------*/
extern TUPLE*  tpl_create  (ATTSET *attset, int fromas);
extern TUPLE*  tpl_dup     (TUPLE *tpl);
extern void    tpl_copy    (TUPLE *dst, const TUPLE *src);
extern void    tpl_delete  (TUPLE *tpl);
extern int     tpl_cmp     (const TUPLE *tpl1, const TUPLE *tpl2,
                            void *data);

extern ATTSET* tpl_attset  (TUPLE *tpl);
extern ATT*    tpl_col     (TUPLE *tpl, int colid);
extern INST*   tpl_colval  (TUPLE *tpl, int colid);
extern int     tpl_colcnt  (const TUPLE *tpl);
extern int     tpl_setmark (TUPLE *tpl, int mark);
extern int     tpl_getmark (const TUPLE *tpl);
extern float   tpl_setwgt  (TUPLE *tpl, float weight);
extern float   tpl_getwgt  (const TUPLE *tpl);
extern INST*   tpl_info    (TUPLE *tpl);
extern TABLE*  tpl_table   (TUPLE *tpl);
extern int     tpl_id      (const TUPLE *tpl);

extern void    tpl_toas    (TUPLE *tpl);
extern void    tpl_fromas  (TUPLE *tpl);
extern int     tpl_uvcnt   (const TUPLE *tpl);
extern int     tpl_isect   (TUPLE *res, TUPLE *tpl1, const TUPLE *tpl2);
extern int     tpl_compat  (const TUPLE *tpl1, const TUPLE *tpl2);
extern UINT    tpl_hash    (TUPLE *tpl);

#ifndef NDEBUG
extern void    tpl_show    (TUPLE *tpl, TPL_APPFN showfn, void *data);
#endif

/*----------------------------------------------------------------------
  Table Functions
----------------------------------------------------------------------*/
extern TABLE*  tab_create  (const char *name, ATTSET *attset,
                            TPL_DELFN delfn);
extern TABLE*  tab_dup     (const TABLE *tab, int dupas);
extern void    tab_delete  (TABLE *tab, int delas);
extern int     tab_rename  (TABLE *tab, const char *name);
extern int     tab_cmp     (const TABLE *tab1, const TABLE *tab2,
                            TPL_CMPFN cmpfn, void *data);

extern CCHAR*  tab_name    (TABLE *tab);
extern ATTSET* tab_attset  (TABLE *tab);
extern INST*   tab_info    (TABLE *tab);

extern void    tab_reduce  (TABLE *tab);
extern int     tab_opc     (TABLE *tab, int mode);
extern float   tab_poss    (TABLE *tab, TUPLE *tpl);
extern void    tab_possx   (TABLE *tab, TUPLE *tpl, double res[]);

extern int     tab_balance (TABLE *tab, int colid,
                            double wgtsum, double *fracs);
extern double  tab_getwgt  (TABLE *tab, int off, int cnt);
extern void    tab_shuffle (TABLE *tab, int off, int cnt,
                            double randfn(void));
extern void    tab_sort    (TABLE *tab, int off, int cnt,
                            TPL_CMPFN cmpfn, void *data);
extern int     tab_search  (TABLE *tab, int off, int cnt,
                            TUPLE *tpl, TPL_CMPFN cmpfn, void *data);
extern int     tab_group   (TABLE *tab, int off, int cnt,
                            TPL_SELFN selfn, void *data);
extern void    tab_apply   (TABLE *tab, int off, int cnt,
                            TPL_APPFN appfn, void *data);
extern void    tab_filluv  (TABLE *tab, int tploff, int tplcnt,
                                        int coloff, int colcnt);
extern int     tab_join    (TABLE *dst, TABLE *src,
                            int cnt, int *dcis, int *scis);
#ifndef NDEBUG
extern void    tab_show    (const TABLE *tab, int off, int cnt,
                            TPL_APPFN show, void *data);
#endif

/*----------------------------------------------------------------------
  Table Column Functions
----------------------------------------------------------------------*/
extern int     tab_coladd  (TABLE *tab, ATT *att, int filluv);
extern int     tab_coladdm (TABLE *tab, ATT **att, int cnt);
extern void    tab_colrem  (TABLE *tab, int colid);
extern int     tab_colconv (TABLE *tab, int colid, int type);
extern int     tab_colnorm (TABLE *tab, int colid,
                            double exp, double sdev);
extern void    tab_colexg  (TABLE *tab, int colid1, int colid2);
extern void    tab_colmove (TABLE *tab, int off, int cnt, int pos);
extern int     tab_colcut  (TABLE *dst, TABLE *src, int mode, ...);
extern int     tab_colcopy (TABLE *dst, const TABLE *src, int mode,...);
extern ATT*    tab_col     (TABLE *tab, int colid);
extern int     tab_colcnt  (const TABLE *tab);

/*----------------------------------------------------------------------
  Table Tuple Functions
----------------------------------------------------------------------*/
extern int     tab_tpladd  (TABLE *tab, TUPLE *tpl);
extern int     tab_tpladdm (TABLE *tab, TUPLE **tpls, int cnt);
extern TUPLE*  tab_tplrem  (TABLE *tab, int tplid);
extern void    tab_tplexg  (TABLE *tab, int tplid1, int tplid2);
extern void    tab_tplmove (TABLE *tab, int off, int cnt, int pos);
extern int     tab_tplcut  (TABLE *dst, TABLE *src, int mode, ...);
extern int     tab_tplcopy (TABLE *dst, const TABLE *src, int mode,...);
extern TUPLE*  tab_tpl     (TABLE *tab, int tplid);
extern int     tab_tplcnt  (const TABLE *tab);

/*----------------------------------------------------------------------
  Preprocessor Definitions
----------------------------------------------------------------------*/
#define tpl_attset(t)      ((t)->attset)
#define tpl_col(t,i)       as_att((t)->attset, i)
#define tpl_colval(t,i)    ((t)->cols +(i))
#define tpl_colcnt(t)      as_attcnt((t)->attset)
#define tpl_setmark(t,m)   ((t)->mark = (m))
#define tpl_getmark(t)     ((t)->mark)
#define tpl_setwgt(t,w)    ((t)->weight = (w))
#define tpl_getwgt(t)      ((t)->weight)
#define tpl_info(t)        (&(t)->info)
#define tpl_table(t)       ((t)->table)
#define tpl_id(t)          ((t)->id)

/*--------------------------------------------------------------------*/
#define tab_name(t)        ((t)->name)
#define tab_attset(t)      ((t)->attset)
#define tab_info(t)        (&(t)->info)

#define tab_coladd(t,c,i)  tab_coladdm(t, &(c), (i) ? -1 : 1)
#define tab_col(t,i)       as_att((t)->attset, i)
#define tab_colcnt(t)      as_attcnt((t)->attset)

#define tab_tpl(t,i)       ((t)->tpls[i])
#define tab_tplcnt(t)      ((t)->tplcnt)

#endif
