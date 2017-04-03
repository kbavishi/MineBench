/*----------------------------------------------------------------------
  File    : table1.c
  Contents: tuple and table management,
            without one point coverage functions
  Author  : Christian Borgelt
  History : 06.03.1996 file created
            23.07.1996 tuple and table management redesigned
            26.02.1997 functions tab_sort and tab_search added
            27.02.1997 function tab_reduce added
            28.03.1997 several functions redesigned
            29.03.1997 several functions added or extended
            07.08.1997 function tab_tplexg added
            29.01.1998 function tpl_fromas added
            24.02.1998 function tab_shuffle added
            12.03.1998 tuple sort function improved
            09.06.1998 tuple vector enlargement modified
            22.06.1998 deletion function moved to function tab_create
            23.06.1998 adapted to changes of attset functions
            27.07.1998 function tab_tplmove added
            25.09.1998 first step of major redesign completed
            01.10.1998 function tab_rename added
            28.11.1998 functions tab_colcut and tab_colcopy finished
            29.11.1998 functions tpl_dup, tab_dup, and tab_filluv added
            04.02.1999 all occurences of long int changed to int
            15.03.1999 one point coverage functions transf. from opc.c
            17.03.1999 one point coverage functions redesigned
            23.03.1999 functions tab_colcut and tab_colcopy debugged
            03.04.1999 parameter 'dupas' added to function tab_dup
            17.04.1999 function tab_getwgt added
            21.10.1999 normalization of one point coverages added
            25.11.1999 bug in natural join (tab_join) removed
            24.06.2001 module split into two files
            26.02.2002 bug in tab_balance fixed (-2/-1 exchanged)
            22.07.2003 function tab_colnorm added
----------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <float.h>
#include <math.h>
#include <assert.h>
#include "vecops.h"
#include "table.h"
#ifdef STORAGE
#include "storage.h"
#endif

/*----------------------------------------------------------------------
  Preprocessor Definitions
----------------------------------------------------------------------*/
#define BLKSIZE    256          /* tuple vector block size */

/*----------------------------------------------------------------------
  Type Definitions
----------------------------------------------------------------------*/
typedef struct {                /* --- join comparison data --- */
  int cnt;                      /* number of columns to compare */
  int *cis1, *cis2;             /* vectors of column indices */
} JCDATA;                       /* (join comparison data) */

/*----------------------------------------------------------------------
  Auxiliary Functions
----------------------------------------------------------------------*/

int tab_resize (TABLE *tab, int size)
{                               /* --- resize tuple vector */
  int   n;                      /* current vector size */
  TUPLE **tmp;                  /* buffer for new tuple vector */

  assert(tab);                  /* check the function argument */
  n = tab->tplvsz;              /* get the current vector size */
  if (size > 0) {               /* if to enlarge the vector */
    if (n >= size) return 0;    /* if vector is large enough, abort */
    n += (n > BLKSIZE) ? n >> 1 : BLKSIZE;
    if (n >  size) size = n; }  /* compute the new vector size */
  else {                        /* if to shrink the vector */
    size = tab->tplcnt << 1;    /* get the maximal tolerable size */
    if (size < BLKSIZE) size = BLKSIZE;
    if (n <= size) return 0;    /* if vector is small enough, abort */
    size = tab->tplcnt +(tab->tplcnt >> 1);
    if (size < BLKSIZE) size = BLKSIZE;
  }                             /* compute the new vector size */
  tmp = (TUPLE**)realloc(tab->tpls, size *sizeof(TUPLE*));
  if (!tmp) return -1;          /* resize the tuple vector */
  tab->tpls   = tmp;            /* and set the new vector */
  tab->tplvsz = size;           /* set the new vector size */
  return 1;                     /* return 'ok' */
}  /* tab_resize() */

/*--------------------------------------------------------------------*/

static void _restore (TABLE *tab, int rszcnt, int addcnt)
{                               /* --- restore if expansion failed */
  int   cnt;                    /* number of attributes */
  int   size;                   /* old size of tuples */
  TUPLE **p = tab->tpls;        /* to traverse the tuples */

  assert(tab);                  /* check the function argument */
  cnt = as_attcnt(tab->attset); /* get the number of attributes */
  tab->marks = (int*)realloc(tab->marks, cnt *sizeof(int));
  size = (int)(sizeof(TUPLE) +(cnt-1) *sizeof(INST));
  while (--rszcnt >= 0) {       /* traverse the resized tuples */
    *p = (TUPLE*) realloc(*p, size); p++; }
  tab->tplcnt -= addcnt;        /* restore the number of tuples */
  while (--addcnt >= 0)         /* traverse the added tuples */
    free(*p++);                 /* and delete them */
}  /* _restore() */

/*--------------------------------------------------------------------*/

static int _expand (TABLE *tab, int tplcnt, int colcnt)
{                               /* --- expand a table */
  int   i, cnt;                 /* loop variable, num. of attributes */
  int   size = 0;               /* new tuple size (with add. columns) */
  TUPLE **p, *tpl;              /* to traverse the tuples */
  int   *marks;                 /* temporary buffer for marker vector */

  assert(colcnt >= 0);          /* check the function argument */
  cnt  = colcnt +as_attcnt(tab->attset);
  size = (int)(sizeof(TUPLE) +(cnt -1) *sizeof(INST));
                                /* get the (new) tuple size */
  if (colcnt > 0) {             /* if to add columns (expand tuples) */
    marks = (int*)realloc(tab->marks, cnt *sizeof(int));
    if (!marks) return -1;      /* resize the marker vector */
    tab->marks = marks;         /* and set the new vector */
    p = tab->tpls;              /* traverse the existing tuples */
    for (i = 0; i < tab->tplcnt; i++) {
      tpl = (TUPLE*)realloc(*p, size);
      if (!tpl) { _restore(tab, i, 0); return -1; }
      *p++ = tpl;               /* resize the tuple and */
    }                           /* set the new tuple */
  }
  if (tplcnt <= 0) return 0;    /* if no tuples to add, abort */
  if (tab_resize(tab, tab->tplcnt +tplcnt) < 0) {
    _restore(tab, tab->tplcnt, 0); return -1; }
  p = tab->tpls +tab->tplcnt;   /* get next field in tuple vector */
  for (i = 0; i < tplcnt; i++){ /* traverse the additional tuples */
    *p++ = tpl = (TUPLE*)malloc(size);
    if (!tpl) { _restore(tab, tab->tplcnt, i); return -1; }
    tpl->attset = tab->attset;  /* allocate a new tuple */
    tpl->table  = tab;          /* and initialize fields */
    tpl->id     = tab->tplcnt +i;
    tpl->weight = 1.0F; tpl->info.p = NULL;
  }
  tab->tplcnt += tplcnt;        /* compute new number of tuples */
  return 0;                     /* return 'ok' */
}  /* _expand() */

/*----------------------------------------------------------------------
  Tuple Functions
----------------------------------------------------------------------*/

TUPLE* tpl_create (ATTSET *attset, int fromas)
{                               /* --- create a tuple */
  TUPLE *tpl;                   /* created tuple */
  INST  *col;                   /* to traverse the columns */
  int   cnt;                    /* number of columns/loop variable */

  assert(attset);               /* check the function argument */
  cnt = as_attcnt(attset);      /* get the number of columns */
  tpl = (TUPLE*)malloc(sizeof(TUPLE) +(cnt-1) *sizeof(INST));
  if (!tpl) return NULL;        /* allocate memory */
  tpl->attset = attset;         /* note the attribute set */
  tpl->table  = NULL;           /* clear the reference to a table */
  tpl->id     = -1;             /* and the tuple identifier */
  if (!fromas) {                /* if not to initialize from attset, */
    tpl->weight = 1.0F;         /* set the tuple weight and */
    tpl->info.p = NULL; }       /* clear the additional information */
  else {                        /* if to initialize from att. set, */
    col = tpl->cols +cnt;       /* copy the attribute instances */
    while (--cnt >= 0) *--col = *att_inst(as_att(attset, cnt));
    tpl->weight = as_getwgt(attset);
    tpl->info   = *as_info(attset);
  }                             /* copy weight and add. information */
  return tpl;                   /* return the created tuple */
}  /* tpl_create() */

/*--------------------------------------------------------------------*/

TUPLE* tpl_dup (TUPLE *tpl)
{                               /* --- duplicate a tuple */
  int   i;                      /* loop variable, number of columns */
  TUPLE *dup;                   /* created duplicate */
  INST  *s, *d;                 /* to traverse the tuple columns */

  assert(tpl);                  /* check the function argument */
  i   = as_attcnt(tpl->attset); /* get the number of columns */
  dup = (TUPLE*)malloc(sizeof(TUPLE) +(i-1) *sizeof(INST));
  if (!dup) return NULL;        /* allocate memory */
  dup->attset = tpl->attset;    /* note the attribute set */
  dup->table  = NULL;           /* clear the reference to a table */
  dup->id     = -1;             /* and the tuple identifier */
  dup->mark   = tpl->mark;      /* copy the mark, */
  dup->weight = tpl->weight;    /* the weight, and */
  dup->info   = tpl->info;      /* the additional information */
  s = tpl->cols +i; d = dup->cols +i;
  while (--i >= 0) *--d = *--s; /* copy the tuple columns */
  return dup;                   /* return the created duplicate */
}  /* tpl_dup() */

/*--------------------------------------------------------------------*/

void tpl_copy (TUPLE *dst, const TUPLE *src)
{                               /* --- copy a tuple */
  int  i, k;                    /* loop variable, number of columns */
  INST const *s; INST *d;       /* to traverse the tuple columns */

  assert(src && dst);           /* check the function arguments */
  i = as_attcnt(src->attset);   /* get the number of columns */
  k = as_attcnt(dst->attset);   /* of source and destination */
  if (k < i) i = k;             /* and determine their minimum */
  s = src->cols +i; d = dst->cols +i;
  while (--i >= 0) *--d = *--s; /* copy the tuple columns */
  dst->mark   = src->mark;      /* copy the mark, */
  dst->weight = src->weight;    /* the weight, and */
  dst->info   = src->info;      /* the additional information */
}  /* tpl_copy() */

/*--------------------------------------------------------------------*/

void tpl_delete (TUPLE *tpl)
{                               /* --- delete a tuple */
  assert(tpl);                  /* check the function argument */
  if (tpl->table)               /* remove the tuple from cont. table */
    tab_tplrem(tpl->table, tpl->id);
  free(tpl);                    /* deallocate the memory */
}  /* tpl_delete() */

/*--------------------------------------------------------------------*/

int tpl_cmp (const TUPLE *tpl1, const TUPLE *tpl2, void *data)
{                               /* --- compare two tuples */
  const ATT  *att;              /* to traverse the attributes */
  const INST *col1, *col2;      /* to traverse the tuple columns */
  int        cnt, i;            /* number of columns, loop variable */
  int        r = 1;             /* return code (result of comparison) */

  if (!tpl1) { tpl1 = tpl2; tpl2 = NULL; r = -1; }
  assert(tpl1);                 /* if no first tuple, exchange */
  cnt = as_attcnt(tpl1->attset);/* traverse attributes and columns */
  for (col1 = tpl1->cols, i = 0; i < cnt; i++, col1++) {
    att  = as_att(tpl1->attset, i);
    col2 = (tpl2) ? tpl2->cols +i : att_inst(att);
    if (att_type(att) == AT_FLT) {
      if (col1->f < col2->f) return -r;    /* if float instance, */
      if (col1->f > col2->f) return  r; }  /* compare floats */
    else {                                 /* if integer or */
      if (col1->i < col2->i) return -r;    /* symbolic instance, */
      if (col1->i > col2->i) return  r;    /* compare integers */
    }                           /* (traverse columns and if they */
  }                             /* differ, return comparison result) */
  return 0;                     /* return 'equal' */
}  /* tpl_cmp() */

/*----------------------------------------------------------------------
The parameter data, which is not used in the above function, is needed
to make this function usable with the functions tab_sort and tab_search.
----------------------------------------------------------------------*/

void tpl_toas (TUPLE *tpl)
{                               /* --- copy tuple to attribute set */
  int  i;                       /* loop variable */
  INST *col;                    /* to traverse the tuple columns */

  assert(tpl);                  /* check the function argument */
  for (col = tpl->cols +(i = as_attcnt(tpl->attset)); --i >= 0; )
    *att_inst(as_att(tpl->attset, i)) = *--col;
  as_setwgt(tpl->attset, tpl->weight);  /* copy the tuple columns, */
  *as_info(tpl->attset) = tpl->info;    /* the tuple weight, and */
}  /* tpl_toas() */                     /* the additional information */

/*--------------------------------------------------------------------*/

void tpl_fromas (TUPLE *tpl)
{                               /* --- copy tuple from attribute set */
  int  i;                       /* loop variable */
  INST *col;                    /* to traverse the tuple columns */

  assert(tpl);                  /* check the function argument */
  for (col = tpl->cols +(i = as_attcnt(tpl->attset)); --i >= 0; )
    *--col = *att_inst(as_att(tpl->attset, i));
  tpl->weight = as_getwgt(tpl->attset); /* copy the att. instances, */
  tpl->info   = *as_info(tpl->attset);  /* the instance weight, and */
}  /* tpl_fromas() */                   /* the additional information */

/*--------------------------------------------------------------------*/

unsigned int tpl_hash (TUPLE *tpl)
{                               /* --- hash function for tuples */
  int  i, t;                    /* loop variable, buffer */
  int  e;                       /* binary exponent */
  INST *col;                    /* to traverse the tuple columns */
  UINT h = 0;                   /* hash value of the tuple */

  assert(tpl);                  /* check the function arguemnt */
  for (col = tpl->cols +(i = as_attcnt(tpl->attset)); --i >= 0; ) {
    if (att_type(as_att(tpl->attset, i)) == AT_FLT) {
      t = (int)(INT_MAX *(frexp((--col)->f, &e) -0.5));
      h ^= (h << 3) ^ t ^ e; }  /* split a float value into */
    else                        /* mantissa and exponent, */
      h ^= (h << 3) ^ (--col)->i;
  }                             /* use an integer value directly */
  return h;                     /* return the computed hash value */
}  /* tpl_hash() */

/*--------------------------------------------------------------------*/
#ifndef NDEBUG

void tpl_show (TUPLE *tpl, TPL_APPFN showfn, void *data)
{                               /* --- show a tuple */
  ATT  *att;                    /* to traverse attributes */
  INST *col;                    /* to traverse columns */
  int  cnt, i;                  /* number of columns/loop variable */

  assert(tpl);                  /* check the function argument */
  cnt = as_attcnt(tpl->attset); /* traverse the tuple columns */
  for (col = tpl->cols, i = 0; i < cnt; i++, col++) {
    att = as_att(tpl->attset,i);/* get the corresponding attribute */
    switch (att_type(att)) {    /* and evaluate its type */
      case AT_FLT: if (col->f <= UV_FLT) printf("? ");
                   else                  printf("%g ", col->f);  break;
      case AT_INT: if (col->i <= UV_INT) printf("? ");
                   else                  printf("%i ", col->i);  break;
      default    : if (col->i <= UV_SYM) printf("? ");
                   else printf("%s ", att_valname(att, col->i)); break;
    }                           /* print the column value */
  }                             /* w.r.t. the attribute type */
  printf(": %g", tpl->weight);  /* print the tuple weight */
  if (showfn) showfn(tpl,data); /* show the additional information */
  printf("\n");                 /* terminate the output line */
}  /* tpl_show() */

#endif
/*----------------------------------------------------------------------
  Table Functions
----------------------------------------------------------------------*/

TABLE* tab_create (const char *name, ATTSET *attset, TPL_DELFN delfn)
{                               /* --- create a table */
  TABLE *tab;                   /* created table */

  assert(attset && delfn && name && *name);    /* check arguments */
  tab = (TABLE*)malloc(sizeof(TABLE));
  if (!tab) return NULL;        /* allocate memory for table body */
  tab->marks = (int*)malloc(as_attcnt(attset) *sizeof(int));
  if (!tab->marks) {                   free(tab); return NULL; }
  tab->name = (char*)malloc((strlen(name) +1) *sizeof(char));
  if (!tab->name)  { free(tab->marks); free(tab); return NULL; }
  strcpy(tab->name, name);      /* copy the table name and */
  tab->attset = attset;         /* initialize the fields */
  tab->tplvsz = tab->tplcnt = 0;
  tab->tpls   = NULL;
  tab->info.p = NULL;
  tab->delfn  = delfn;
  return tab;                   /* return the created table */
}  /* tab_create() */

/*--------------------------------------------------------------------*/

TABLE* tab_dup (const TABLE *tab, int dupas)
{                               /* --- duplicate a table */
  ATTSET *attset;               /* duplicate of attribute set */
  TABLE  *dup;                  /* created duplicate of table */
  int    i;                     /* loop variable */
  TUPLE  *const*s, **d;         /* to traverse the tuples */

  assert(tab);                  /* check the function argument */
  attset = tab->attset;         /* get the table's attribute set */
  if (dupas) {                  /* if the duplicate flag is set, */
    attset = as_dup(attset);    /* duplicate the attribute set */
    if (!attset) return NULL;   /* and check for an error, */
  }                             /* then create a new table */
  dup = tab_create(tab->name, attset, tab->delfn);
  if (!dup) { if (dupas) as_delete(attset); return NULL; }
  dup->info = tab->info;        /* copy the add. table information */
  if (tab->tplcnt <= 0)         /* if there are no tuples, */
    return dup;                 /* abort the function */
  dup->tpls = (TUPLE**)malloc(tab->tplcnt *sizeof(TUPLE*));
  if (!dup->tpls) { tab_delete(dup, dupas); return NULL; }
  s = tab->tpls +tab->tplcnt;   /* allocate a tuple vector */
  d = dup->tpls +tab->tplcnt;   /* and traverse the tuples */
  for (i = tab->tplcnt; --i >= 0; ) {
    *--d = tpl_dup(*--s);       /* duplicate a tuple */
    if (!*d) break;             /* and check for success */
    (*d)->table = dup; (*d)->id = i;
  }                             /* set table ref. and identifier */
  if (i >= 0) {                 /* if an error occured */
    for (i = tab->tplcnt -i; --i > 0; ) free(*++d);
    tab_delete(dup, dupas); return NULL;
  }                             /* delete the table and abort */
  dup->tplvsz = dup->tplcnt = tab->tplcnt;
  return dup;                   /* set the number of tuples and */
}  /* tab_dup() */              /* return the created duplicate */

/*--------------------------------------------------------------------*/

void tab_delete (TABLE *tab, int delas)
{                               /* --- delete a table */
  int   i;                      /* loop variable */
  TUPLE **p;                    /* to traverse the tuples */

  assert(tab);                  /* check the function argument */
  if (tab->tpls) {              /* if there are tuples */
    for (p = tab->tpls +(i = tab->tplcnt); --i >= 0; ) {
      (*--p)->table = NULL; tab->delfn(*p); }
    free(tab->tpls);            /* delete tuples, vector, */
  }                             /* and the attribute set */
  if (delas) as_delete(tab->attset);
  free(tab->name);              /* delete the table name, */
  free(tab->marks);             /* the column marker vector, */
  free(tab);                    /* and the table body */
}  /* tab_delete() */

/*--------------------------------------------------------------------*/

int tab_rename (TABLE *tab, const char *name)
{                               /* --- rename a table */
  char *tmp;                    /* temporary buffer for table name */

  assert(tab && name && *name); /* check the function arguments */
  tmp = (char*)realloc(tab->name, (strlen(name) +1) *sizeof(char));
  if (!tmp) return -1;          /* reallocate memory for new name */
  tab->name = strcpy(tmp,name); /* copy the new table name */
  return 0;                     /* and return 'ok' */
}  /* tab_rename() */

/*--------------------------------------------------------------------*/

int tab_cmp (const TABLE *tab1, const TABLE *tab2,
             TPL_CMPFN cmpfn, void *data)
{                               /* --- compare two tables */
  int   n;                      /* loop variable, number of tuples */
  TUPLE **p1, **p2;             /* to traverse the tuples */

  assert(tab1 && tab2 && cmpfn);/* check the function arguments */
  n  = (tab1->tplcnt < tab2->tplcnt) ? tab1->tplcnt : tab2->tplcnt;
  p1 = tab1->tpls +n;           /* get the number of tuples and */
  p2 = tab2->tpls +n;           /* traverse und compare the leading */
  while (--n >= 0)              /* tuples of the two tables */
    if (cmpfn(*--p1, *--p2, data) != 0)
      return -1;                /* if the tuples differ, abort */
  return 0;                     /* otherwise return 'equal' */
}  /* tab_cmp() */

/*--------------------------------------------------------------------*/

void tab_reduce (TABLE *tab)
{                               /* --- reduce a table */
  int   i;                      /* loop variable */
  TUPLE **p1, **p2;             /* to traverse the tuples */

  assert(tab);                  /* check the function argument */
  if (tab->tplcnt <= 0) return; /* check whether table is empty */
  v_sort(tab->tpls, tab->tplcnt, (VCMPFN*)tpl_cmp, NULL);
  p1 = tab->tpls; p2 = p1+1;    /* sort and traverse the tuple vector */
  for (i = tab->tplcnt, tab->tplcnt = 1; --i > 0; p2++) {
    if (tpl_cmp(*p1, *p2, NULL) != 0) {
      *++p1 = *p2;              /* if the next tuple differs, keep it */
      (*p1)->id = tab->tplcnt++; } /* and increment the tuple counter */
    else {                      /* if the next tuple is identical */
      (*p1)->weight += (*p2)->weight;
      (*p2)->table = NULL;      /* sum the counters (weights), */
      tab->delfn(*p2);          /* remove the tuple from the table, */
    }                           /* and call the deletion function */
  }
  tab_resize(tab, 0);           /* try to shrink the tuple vector */
}  /* tab_reduce() */

/*--------------------------------------------------------------------*/

int tab_balance (TABLE *tab, int colid, double wgtsum, double *freqs)
{                               /* --- balance a table w.r.t. a column*/
  int    i, k;                  /* loop variable, buffer */
  int    valcnt;                /* number of attribute values */
  TUPLE  **p;                   /* to traverse the tuples */
  double *facts, *f;            /* weighting factors, buffer */
  double sum, tmp;              /* weight sum, temporary buffer */

  assert(tab                    /* check the function arguments */
      && (colid >= 0) && (colid <= tab_colcnt(tab))
      && (att_type(tab_col(tab, colid)) == AT_SYM));
  
  /* --- initialize --- */
  valcnt = att_valcnt(tab_col(tab, colid));
  facts  = (double*)calloc(valcnt, sizeof(double));
  if (!facts) return -1;        /* allocate a factor vector */
  p = tab->tpls +tab->tplcnt;   /* traverse the tuples in table */
  for (sum = 0.0F, i = tab->tplcnt; --i >= 0; ) {
    sum += tmp = (*--p)->weight;/* sum the tuple weights */
    k = (*p)->cols[colid].i;    /* get the class identifier */
    if (k >= 0) facts[k] += tmp;/* and sum the tuple weights */
  }                             /* determine the value frequencies */
  if (sum <= 0) return 0;       /* check the tuple weight sum */

  /* --- compute weighting factors --- */
  f = facts +(i = valcnt);      /* traverse the computed frequencies */
  if      (wgtsum <= -2.0F) {   /* if to lower the tuple weights */
    for (tmp = FLT_MAX; --i >= 0; ) { if (*--f < tmp) tmp = *f; }
    wgtsum = valcnt *tmp; }     /* find the minimal frequency */
  else if (wgtsum <= -1.0F) {   /* if to boost the tuple weights */
    for (tmp = 0.0F;    --i >= 0; ) { if (*--f > tmp) tmp = *f; }
    wgtsum = valcnt *tmp; }     /* find the maximal frequency */
  else if (wgtsum <=  0.0F)     /* if to shift the tuple weights, */
    wgtsum = sum;               /* use the sum of the tuple weights */
  if (!freqs) {                 /* if no relative freqs. requested */
    tmp = wgtsum /valcnt;       /* compute the weighting factors */
    for (i = valcnt; --i >= 0; ) facts[i] = tmp / facts[i]; }
  else {                        /* if relative freqs. requested */
    f = freqs +(i = valcnt);    /* sum the requested frequencies */
    for (sum = 0.0F; --i >= 0; ) sum += *--f;
    tmp = wgtsum /sum; f = freqs +(i = valcnt);
    while (--i >= 0) facts[i] = tmp *(*--f / facts[i]);
  }                             /* compute the weighting factors */

  /* --- weight tuples --- */
  for (p = tab->tpls +(i = tab->tplcnt); --i >= 0; ) {
    k = (*--p)->cols[colid].i;  /* get the class identifier */
    if (k >= 0) (*p)->weight = (float)((*p)->weight *facts[k]);
    else        (*p)->weight = 0.0F;
  }                             /* adapt the tuple weight */

  free(facts);                  /* delete the factor vector */
  return 0;                     /* and return 'ok' */
}  /* tab_balance() */

/*--------------------------------------------------------------------*/

double tab_getwgt (TABLE *tab, int off, int cnt)
{                               /* --- get the tuple weight sum */
  TUPLE  **p;                   /* to traverse the tuples */
  double sum = 0;               /* sum of the tuple weights */
  
  assert(tab && (off >= 0));    /* check the function arguments */
  if (cnt > tab->tplcnt -off) cnt = tab->tplcnt -off;
  assert(cnt >= 0);             /* check and adapt number of tuples */
  for (p = tab->tpls +off +cnt; --cnt >= 0; )
    sum += (*--p)->weight;      /* sum the tuple weights */
  return sum;                   /* and return the result */
}  /* tab_getwgt() */

/*--------------------------------------------------------------------*/

void tab_shuffle (TABLE *tab, int off, int cnt, double randfn(void))
{                               /* --- shuffle a table section */
  int   i;                      /* tuple index */
  TUPLE **p, *tpl;              /* to traverse the tuples, buffer */

  assert(tab && (off >= 0) && randfn);  /* check function arguments */
  if (cnt > tab->tplcnt -off) cnt = tab->tplcnt -off;
  assert(cnt >= 0);             /* check and adapt number of tuples */
  for (p = tab->tpls +off; --cnt > 0; ) {
    i = (int)((cnt+1) *randfn());
    if      (i > cnt) i = cnt;  /* compute a random index in the */
    else if (i < 0)   i = 0;    /* remaining table section */
    tpl = p[i]; p[i] = *p; *p++ = tpl;
    tpl->id = off++;            /* exchange first and i-th tuple */
  }                             /* and adapt the tuple identifier */
  if (cnt >= 0) (*p)->id = off; /* adapt the identifier */
}  /* tab_shuffle() */          /* of the last tuple */

/*--------------------------------------------------------------------*/

void tab_sort (TABLE *tab, int off, int cnt,
               TPL_CMPFN cmpfn, void *data)
{                               /* --- sort a table section */
  TUPLE **p;                    /* to traverse the tuples */

  assert(tab && (off >= 0) && cmpfn);   /* check function arguments */
  if (cnt > tab->tplcnt -off) cnt = tab->tplcnt -off;
  assert(cnt >= 0);             /* check and adapt number of tuples */
  v_sort(tab->tpls +off, cnt, (VCMPFN*)cmpfn, data);
  p = tab->tpls +off;           /* sort tuples and adapt identifiers */
  while (--cnt >= 0) (*p++)->id = off++;
}  /* tab_sort() */

/*--------------------------------------------------------------------*/

int tab_search (TABLE *tab, int off, int cnt,
                TUPLE *tpl, TPL_CMPFN cmpfn, void *data)
{                               /* --- search a tuple in a table sec. */
  int mid, last;                /* tuple indices */
  int r;                        /* result of comparison */

  assert(tab && (off >= 0) && cmpfn);  /* check function arguments */
  if (cnt > tab->tplcnt -off) cnt = tab->tplcnt -off;
  if (cnt <= 0) return -1;      /* if the section is empty, abort */
  last = off +cnt -1;           /* get index of last tuple in section */
  do {                          /* binary search loop */
    mid = (off +last) >> 1;     /* compare the tuple in the middle */
    r = cmpfn(tpl, tab->tpls[mid], data);    /* with the given one */
    if (r > 0) off  = mid +1;   /* if smaller, continue with right, */
    else       last = mid;      /* otherwise continue with left half */
  } while (off < last);         /* repeat until the section is empty */
  if (off > last) return -1;    /* if no tuple left, abort function */
  if (off != mid)               /* if last test on different tuple, */
    r = cmpfn(tpl, tab->tpls[off], data);      /* retest last tuple */
  return (r == 0) ? off : -1;   /* return the index of the tuple */
}  /* tab_search() */

/*--------------------------------------------------------------------*/

int tab_group (TABLE *tab, int off, int cnt,
               TPL_SELFN selfn, void *data)
{                               /* -- group tuples in a table section */
  TUPLE **src, **dst;           /* source and destination */
  TUPLE *t;                     /* exchange buffer */
  int   i;                      /* tuple identifier */

  assert(tab && (off >= 0) && selfn);   /* check function arguments */
  if (cnt > tab->tplcnt -off) cnt = tab->tplcnt -off;
  assert(cnt >= 0);             /* check and adapt number of tuples */
  for (src = dst = tab->tpls +cnt; --cnt >= 0; ) {
    if (!selfn(*--src, data)) { /* traverse the tuples in the section */
      t = *src;  *src  = *--dst;     *dst       = t;
      i = t->id; t->id = (*src)->id; (*src)->id = i;
    }                           /* swap the qualifying tuples */
  }                             /* to the head of the section */
  return (int)(dst -src);       /* and return their number */
}  /* tab_group() */

/*--------------------------------------------------------------------*/

void tab_apply (TABLE *tab, int off, int cnt,
                TPL_APPFN appfn, void *data)
{                               /* --- apply a function to all tuples */
  TUPLE **p;                    /* to traverse the tuples */

  assert(tab && (off >= 0) && appfn);  /* check function arguments */
  if (cnt > tab->tplcnt -off) cnt = tab->tplcnt -off;
  assert(cnt >= 0);             /* check and adapt number of tuples */
  p = tab->tpls +off;           /* apply the function to the tuples */
  while (--cnt >= 0) appfn(*p++, data);
}  /* tab_apply() */

/*--------------------------------------------------------------------*/

void tab_filluv (TABLE *tab, int tploff, int tplcnt,
                             int coloff, int colcnt)
{                               /* --- fill table part with unknowns */
  int   k;                      /* loop variable, buffer */
  TUPLE **p;                    /* to traverse the tuples */
  INST  *src = NULL, *dst;      /* to traverse the tuple columns */

  assert(tab && (tploff >= 0) && (coloff >= 0));  /* check arguments */
  k = tab->tplcnt;              /* get the total number of tuples */
  if (tplcnt > k -tploff) tplcnt = k -tploff;
  assert(tplcnt >= 0);          /* get and check the num. of tuples */
  k = as_attcnt(tab->attset);   /* get the total number of columns */
  if (colcnt > k -coloff) colcnt = k -coloff;
  assert(colcnt >= 0);          /* get and check the num. of columns */
  for (p = tab->tpls +tploff +tplcnt; --tplcnt >= 0; ) {
    dst = (*--p)->cols +coloff +colcnt;     /* traverse the tuples */
    if (src) {                  /* if this is not the first tuple, */
      src += colcnt;            /* copy unknown values from 1st tuple */
      for (k = colcnt; --k >= 0; ) *--dst = *--src; }
    else {                      /* if this is the first tuple, */
      for (k = colcnt; --k >= 0; ) {   /* traverse the columns */
        switch (att_type(as_att(tab->attset, coloff +k))) {
          case AT_FLT: (--dst)->f = UV_FLT; break;
          case AT_INT: (--dst)->i = UV_INT; break;
          default    : (--dst)->i = UV_SYM; break;
        }                       /* set the columns to unknown values */
      }                         /* w.r.t. to the column type */
      src = dst;                /* note the first tuple */
    }                           /* to copy the values from it */
  }                             /* for the next tuples */
}  /* tab_filluv() */

/*--------------------------------------------------------------------*/

static int _joincmp (const TUPLE *tpl1, const TUPLE *tpl2, JCDATA *jcd)
{                               /* --- compare tuples for join */
  int   i;                      /* loop variable */
  int   *p1,   *p2;             /* to traverse the column indices */
  CINST *col1, *col2;           /* columns to compare */

  p1 = jcd->cis1; p2 = jcd->cis2;
  for (i = jcd->cnt; --i >= 0; ) {
    col1 = tpl1->cols +*p1++;   /* traverse the column indices */
    col2 = tpl2->cols +*p2;     /* and get the columns to compare */
    if (att_type(as_att(tpl1->attset, *p2++)) == AT_FLT) {
      if (col1->f < col2->f) return -1;
      if (col1->f > col2->f) return  1; }
    else {
      if (col1->i < col2->i) return -1;
      if (col1->i > col2->i) return  1;
    }                           /* compare the tuple columns */
  }                             /* and if they differ, abort */
  return 0;                     /* otherwise return 'equal' */
}  /* _joincmp() */

/*--------------------------------------------------------------------*/

static int _joinsel (const ATT *att, void *data)
{                               /* --- attribute selection for join */
  return ((int*)data)[att_id(att)];
}  /* _joinsel() */

/*--------------------------------------------------------------------*/

static int _joinerr (TUPLE **src, TUPLE **res, int cnt)
{                               /* --- clean up if join failed */
  if (res) {                    /* if a (partial) result exists */
    while (--cnt >= 0) free(res[cnt]);
    free(res);                  /* delete all result tuples */
  }                             /* and the result vector */
  if (src) free(src);           /* delete the source tuple buffer */
  return -1;                    /* return an error code */
}  /* _joinerr() */

/*--------------------------------------------------------------------*/

int tab_join (TABLE *dst, TABLE *src, int cnt, int *dcis, int *scis)
{                               /* --- join two tables */
  int    i, k;                  /* loop variables, buffers */
  int    ncs, ncd, ncr;         /* number of columns in src/dst/res */
  int    rescnt, resvsz;        /* number of tuples in result vector */
  TUPLE  *tpl;                  /* created (joined) tuple */
  TUPLE  **d, **s, **r, **t;    /* to traverse the tuples */
  INST   *dc, *sc;              /* to traverse the tuple columns */
  int    *cis;                  /* non-join column index vector */
  JCDATA jcd;                   /* column data for sorting */

  assert(dst && src             /* check the function arguments */
      && ((cnt <= 0) || (dcis && scis)));

  /* --- build column index vectors --- */
  ncs = as_attcnt(src->attset); /* get the number of columns */
  ncd = as_attcnt(dst->attset); /* in source and destination */
  if (cnt > 0) {                /* if to do a normal join, */
    cis = src->marks;           /* build index vector for copying */
    for (i = ncs; --i >= 0; ) cis[i] = i;
    for (i = cnt; --i >= 0; ) cis[scis[i]] = -1;
    for (i = k = 0; i < ncs; i++)
      if (cis[i] >= 0) cis[k++] = cis[i];
    ncr = ncd +k; }             /* get the number of result columns */
  else {                        /* if to do a natural join, */
    scis = src->marks;          /* use the marker vectors to */
    dcis = dst->marks;          /* store the column indices */
    for (cnt = i = 0; i < ncs; i++) {
      k = as_attid(dst->attset, att_name(as_att(src->attset, i)));
      if (k >= 0) { dcis[cnt] = k; scis[cnt++] = i; }
    }                           /* collect columns with same names */
    k = cnt -1;                 /* traverse the source columns again */
    for (cis = src->marks +(i = ncs); --i >= 0; ) {
      if ((k < 0) || (scis[k] < i)) *--cis = i;
      else --k;                 /* collect the source columns */
    }                           /* that are not in the destination */
    ncr = ncd +(ncs -cnt);      /* (i.e. the non-join columns) and */
  }                             /* get the number of result columns */

  /* --- sort tuple vectors --- */
  k = src->tplcnt +dst->tplcnt; /* get the total number of tuples */
  d = src->buf = (TUPLE**)malloc((k+2) *sizeof(TUPLE*));
  if (!d) return -1;            /* allocate a tuple buffer */
  for (s = src->tpls, i = src->tplcnt; --i >= 0; ) *d++ = *s++;
  *d++ = NULL;                  /* copy source tuples to the buffer */
  for (s = dst->tpls, i = dst->tplcnt; --i >= 0; ) *d++ = *s++;
  *d   = NULL;                  /* copy dest.  tuples to the buffer */
  d   -= dst->tplcnt;           /* (place a sentinel behind the */
  s    = src->buf;              /* source and the dest. tuples) */
  jcd.cnt  = cnt;               /* get the number of join columns */
  jcd.cis1 = jcd.cis2 = scis;   /* and sort the source tuples */
  v_sort(s, src->tplcnt, (VCMPFN*)_joincmp, &jcd);
  jcd.cis1 = jcd.cis2 = dcis;   /* sort the destination tuples */
  v_sort(d, dst->tplcnt, (VCMPFN*)_joincmp, &jcd);
  jcd.cis2 = scis;              /* prepare for join comparisons */

  /* --- join tuples --- */
  resvsz   = rescnt = 0;        /* initialize the counters and */
  dst->buf = NULL;              /* clear the result tuple vector */
  while (*d && *s) {            /* while not at end of vectors */
    k = _joincmp(*d, *s, &jcd); /* compare the current tuples */
    if (k < 0) { d++; continue; }  /* and find next pair */
    if (k > 0) { s++; continue; }  /* of joinable tuples */
    r = s;                      /* get the next source tuple */
    do {                        /* tuple join loop */
      if (rescnt >= resvsz) {   /* if the result vector is full */
        resvsz += (resvsz > BLKSIZE) ? resvsz >> 1 : BLKSIZE;
        t = (TUPLE**)malloc(resvsz *sizeof(TUPLE*));
        if (!t) return _joinerr(src->buf, dst->buf, rescnt);
        dst->buf = t;           /* resize the result vector */
      }                         /* and set the new vector */
      tpl = (TUPLE*)malloc(sizeof(TUPLE) +(ncr-1) *sizeof(INST));
      if (!tpl) return _joinerr(src->buf, dst->buf, rescnt);
      dc = tpl->cols +ncr;      /* create a new tuple and */
      sc = (*r)->cols;          /* copy the source columns */
      for (i = ncs -cnt; --i >= 0; ) *--dc = sc[cis[i]];
      sc = (*d)->cols +ncd;     /* copy the destination columns */
      for (i = ncd;      --i >= 0; ) *--dc = *--sc;
      tpl->weight = (*s)->weight *(*d)->weight;
      tpl->mark   = (*d)->mark; /* compute weight of joined tuple */
      tpl->info   = (*d)->info; /* and copy other fields from the */
      tpl->attset = dst->attset;/* dest. tuple to the created one */
      tpl->table  = dst;
      tpl->id     = rescnt;
      dst->buf[rescnt++] = tpl; /* insert the created (joined) tuple */
    } while (*++r               /* while more joins are possible */
    &&       (_joincmp(*d, *r, &jcd) == 0));
    d++;                        /* go to the next tuple */
  }                             /* in the destination vector */
  free(src->buf);               /* delete the tuple buffer */

  /* --- replace the destination table --- */
  dcis = (int*)realloc(dst->marks, ncr *sizeof(int));
  if (!dcis) return _joinerr(NULL, dst->buf, rescnt);
  dst->marks = dcis;            /* adapt the column marker vector */
  if (scis == src->marks) {     /* if a natural join has been done */
    for (--cnt, cis = src->marks +(i = ncs); --i >= 0; ) {
      if ((cnt < 0) || (scis[cnt] < i)) *--cis = 1;
      else {                     --cnt; *--cis = 0; }
    } }                         /* mark join columns for copying */
  else {                        /* if a normal join has been done */
    for (cis = src->marks +(i = ncs); --i >= 0; ) *--cis = 0;
    for (i = cnt; --i >= 0; ) cis[scis[i]] = 1;
  }                             /* mark join columns for copying */
  i = as_attcopy(dst->attset, src->attset, AS_SELECT, _joinsel, cis);
  if (i != 0) {                 /* copy attributes of added columns */
    dst->marks = (int*)realloc(dst->marks, ncd *sizeof(int));
    return _joinerr(NULL, dst->buf, rescnt);
  }                             /* on error restore old table */
  tab_tplrem(dst, -1);          /* delete all destination tuples and */
  dst->tplvsz = resvsz;         /* set the created (joined) tuples */
  dst->tplcnt = rescnt;         /* as the new destination tuples */
  dst->tpls   = dst->buf;       /* (replace the tuple vector) */
  return 0;                     /* return 'ok' */
}  /* tab_join() */

/*--------------------------------------------------------------------*/
#ifndef NDEBUG

void tab_show (const TABLE *tab, int off, int cnt,
               TPL_APPFN showfn, void *data)
{                               /* --- show a table */
  int    i, colcnt;             /* loop variable, number of columns */
  TUPLE  *const*p;              /* to traverse tuples */
  double wgt = 0.0F;            /* sum of tuple weights */

  assert(tab && (off >= 0));    /* check the function arguments */
  if (cnt > tab->tplcnt -off) cnt = tab->tplcnt -off;
  assert(cnt >= 0);             /* check and adapt number of tuples */
  colcnt = tab_colcnt(tab);     /* get the number of columns */
  for (i = 0; i < colcnt; i++)  /* and traverse the columns */
    printf("%s ", att_name(tab_col(tab, i)));
  printf(": #\n");              /* print the table header */
  for (p = tab->tpls +off; --cnt >= 0; ) {
    tpl_show(*p, showfn, data); /* show the tuple and */
    wgt += (*p++)->weight;      /* sum the tuple weights */
  }
  printf("%d/%g tuple(s)\n", tab->tplcnt, wgt);
}  /* tab_show() */

#endif
/*----------------------------------------------------------------------
  Table Column Functions
----------------------------------------------------------------------*/

int tab_coladdm (TABLE *tab, ATT **atts, int cnt)
{                               /* --- add multiple columns */
  assert(tab && atts);          /* check the function arguments */
  if (_expand(tab, 0, abs(cnt)) != 0)  /* expand the table */
    return -1;                  /* by adding 'cnt' columns */
  if (as_attaddm(tab->attset, atts, abs(cnt)) != 0) {
    _restore(tab, tab->tplcnt, 0);
    return -1;                  /* add the attributes */
  }                             /* to the attribute set */
  if (cnt < 0)                  /* fill new columns with unknowns */
    tab_filluv(tab, 0, tab->tplcnt, as_attcnt(tab->attset) +cnt, -cnt);
  return 0;                     /* return 'ok' */
}  /* tab_coladdm() */

/*--------------------------------------------------------------------*/

void tab_colrem (TABLE *tab, int colid)
{                               /* --- remove a column from a table */
  int   i, k;                   /* loop variables */
  int   cnt;                    /* number of columns (to shift) */
  int   size;                   /* new tuple size */
  TUPLE **p;                    /* to traverse the tuples */
  INST  *col;                   /* to traverse the tuple columns */

  assert(tab && (colid >= 0) && (colid < as_attcnt(tab->attset)));
  as_attcut(NULL, tab->attset, AS_RANGE, colid, 1);
  cnt  = as_attcnt(tab->attset);/* remove attribute from att. set */
  size = (int)(sizeof(TUPLE) +(cnt-1) *sizeof(INST));
  tab->marks = (int*)realloc(tab->marks, (cnt-1) *sizeof(int));
  cnt -= colid;                 /* compute new number of columns */
  for (p = tab->tpls +(i = tab->tplcnt); --i >= 0; ) {
    col = (*--p)->cols +colid;  /* traverse tuples and shift columns */
    for (k = cnt; --k >= 0; ) { *col = col[1]; col++; }
    *p = (TUPLE*) realloc(*p, size);     /* shrink the tuple */
  }                             /* (remove the last column) */
}  /* tab_colrem() */

/*--------------------------------------------------------------------*/

int tab_colconv (TABLE *tab, int colid, int type)
{                               /* --- convert a table column */
  int   i;                      /* loop variable */
  TUPLE **p;                    /* to traverse the tuples */
  int   old;                    /* old attribute type */
  ATT   *att;                   /* attribute to convert */
  INST  *col;                   /* tuple column to convert */
  INST  *map = NULL;            /* conversion map */
  INST  unk;                    /* type specific unknown value */
  INST  min, max, curr;         /* buffers for attribute values */
  char  s[20];                  /* buffer for value name */

  assert(tab && (colid >= 0) && (colid < as_attcnt(tab->attset)));
  att = as_att(tab->attset, colid);
  old = att_type(att);          /* get attribute and note its type */
  if (old == AT_SYM) {          /* if the attribute is symbolic */
    map = (INST*)malloc(att_valcnt(att) *sizeof(INST));
    if (!map) return -1;        /* allocate memory */
  }                             /* for a value map */
  if (att_conv(att, type, map) != 0) {
    if (map) free(map); return 1; }
  type = att_type(att);         /* convert attribute and get new type */
  if ((old  == AT_INT)          /* if to convert an integer column */
  &&  (type == AT_FLT)) {       /* to a real/float column */
    for (p = tab->tpls +(i = tab->tplcnt); --i >= 0; ) {
      col = (*--p)->cols+colid; /* get a pointer to the column */
      col->f = (col->i <= UV_INT) ? UV_FLT : (float)col->i;
    }                           /* convert the column value */
    return 0;                   /* return 'ok' */
  }
  if ((old  == AT_FLT)          /* if to convert a real/float column */
  &&  (type == AT_INT)) {       /* to an integer column */
    for (p = tab->tpls +(i = tab->tplcnt); --i >= 0; ) {
      col = (*--p)->cols+colid; /* get a pointer to the column */
      if      (col->f <= UV_FLT)  col->i =  UV_INT;
      else if (col->f < -INT_MAX) col->i = -INT_MAX;
      else if (col->f >  INT_MAX) col->i =  INT_MAX;
      else                        col->i = (int)col->f;
    }                           /* convert the column value */
    return 0;                   /* return 'ok' */
  }
  if (old == AT_SYM) {          /* if to convert symbolic to numeric */
    if (type == AT_FLT) unk.f = UV_FLT;  /* set the instance to */
    else                unk.i = UV_INT;  /* an unknown value */
    for (p = tab->tpls +(i = tab->tplcnt); --i >= 0; ) {
      col = (*--p)->cols+colid; /* get a pointer to the column */
      *col = (col->i < 0) ? unk : map[col->i];
    }                           /* convert the column value */
    free(map); return 0;        /* delete the value map */
  }                             /* and return 'ok' */
  min  = *att_valmin(att);      /* note the minimal */
  max  = *att_valmax(att);      /* and the maximal value */
  curr = *att_inst(att);        /* and the instance (current value) */
  if (old == AT_INT) {          /* if to convert integer to symbolic */
    for (p = tab->tpls +(i = tab->tplcnt); --i >= 0; ) {
      col = (*--p)->cols +colid;/* get a pointer to the column */
      if (col->i <= UV_INT) {   /* skip unknown values */
        (*p)->id = UV_SYM; continue; }
      sprintf(s, "%d", col->i); /* format integer value */
      if (att_valadd(att, s, NULL) < 0) break;
      (*p)->id = att_inst(att)->i;
    } }                         /* add the value and note its ident. */
  else {                        /* if to convert float to symbolic */
    for (p = tab->tpls +(i = tab->tplcnt); --i >= 0; ) {
      col = (*--p)->cols +colid;/* get a pointer to the column */
      if (col->f <= UV_FLT) {   /* skip unknown values */
        (*p)->id = UV_SYM; continue; }
      sprintf(s, "%g", col->f); /* format real/float value */
      if (att_valadd(att, s, NULL) < 0) break;
      (*p)->id = att_inst(att)->i;
    }                           /* add the value and note its ident. */
  }
  if (i >= 0) {                 /* if an error occurred */
    while (++i < tab->tplcnt) (*++p)->id = i;
    att_conv(att, old, NULL);   /* restore the tuple identifiers, */
    att_valadd(att, NULL, &min);/* the old attribute type, */
    att_valadd(att, NULL, &max);/* the minimal and maximal value, */
    *att_inst(att) = curr;      /* and the instance (current value) */
    return -1;                  /* return an error code */
  }
  for (p = tab->tpls +(i = tab->tplcnt); --i >= 0; ) {
    --p; (*p)->cols[colid].i = (*p)->id;
    (*p)->id = i;               /* set the value identifier and */
  }                             /* restore the tuple identifier */
  return 0;                     /* return 'ok' */
}  /* tab_colconv() */

/*--------------------------------------------------------------------*/

int tab_colnorm (TABLE *tab, int colid, double exp, double sdev)
{                               /* --- convert a table column */
  int    i;                     /* loop variable */
  TUPLE  **p;                   /* to traverse the tuples */
  INST   *col;                  /* tuple column to convert */
  double min, max;              /* minimum and maximum value */
  double sum, sqr;              /* sum of values/squared values */
  double scl, off;              /* scaling factor and offset */
  double cnt;                   /* sum of tuple weights */

  assert(tab && (colid >= 0) && (colid < as_attcnt(tab->attset)));
  if (att_type(as_att(tab->attset, colid)) != AT_FLT)
    return -1;                  /* check for a real valued column */
  if (sdev < 0) {               /* if minimum and range are given */
    min = DBL_MAX; max = -DBL_MAX; /* clear the minimum and maximum */
    for (p = tab->tpls +(i = tab->tplcnt); --i >= 0; ) {
      col = (*--p)->cols+colid; /* traverse the tuples */
      if (col->f <= UV_FLT) continue;
      if (col->f < min) min = col->f;
      if (col->f > max) max = col->f;
    }                           /* determine range of values */
    if (min > max) {            /* if no values found, */
      off = 0; scl = 1; }       /* set the identity scaling */
    else {                      /* if there are values in the column */
      scl = (max > min) ? sdev /(min -max) : 1;
      off = exp -min *scl;      /* scaling factor to new range and */
    } }                         /* offset to new minimum value */
  else {                        /* if exp. value and std. dev. given */
    sum = sqr = cnt = 0;        /* clear the sums and tuple weight */
    for (p = tab->tpls +(i = tab->tplcnt); --i >= 0; ) {
      col = (*--p)->cols+colid; /* traverse the tuples */
      if (col->f <= UV_FLT) continue;
      sqr += col->f *(double)col->f;
      sum += col->f;            /* sum the column values */
      cnt += (*p)->weight;      /* and their squares */
    }                           /* and the tuple weights */
    if (cnt <= 0) {             /* if no values found, */
      off = 0; scl = 1; }       /* set the identity scaling */
    else {                      /* if there are values in the column */
      off = sum /cnt;           /* compute exp. value and std. dev. */
      scl = sqrt((sqr -off *sum) /cnt);
      scl = (scl > 0) ? sdev/scl : 1;
      off = exp -off *scl;      /* compute scaling factor and offset */
    }                           /* for a linear transformation */
  }                             /* of the column values */
  for (p = tab->tpls +(i = tab->tplcnt); --i >= 0; ) {
    col = (*--p)->cols +colid;  /* traverse the tuples */
    if (col->f > UV_FLT) col->f = scl *col->f +off;
  }                             /* scale the column values */
  return 0;                     /* return 'ok' */
}  /* tab_colnorm() */

/*--------------------------------------------------------------------*/

void tab_colexg (TABLE *tab, int colid1, int colid2)
{                               /* --- exchange two table columns */
  int   i;                      /* loop variable */
  TUPLE **p;                    /* to traverse the tuples */
  INST  *cols, t;               /* column vector and exchange buffer */

  assert(tab                    /* check the function arguments */
      && (colid1 >= 0) && (colid1 < as_attcnt(tab->attset))
      && (colid2 >= 0) && (colid2 < as_attcnt(tab->attset)));
  as_attexg(tab->attset, colid1, colid2);  /* exchange attributes */
  for (p = tab->tpls +(i = tab->tplcnt); --i >= 0; ) {
    cols = (*--p)->cols;        /* traverse the tuples */
    t = cols[colid1]; cols[colid1] = cols[colid2]; cols[colid2] = t;
  }                             /* exchange the column values */
}  /* tab_colexg() */

/*--------------------------------------------------------------------*/

void tab_colmove (TABLE *tab, int off, int cnt, int pos)
{                               /* --- move table columns */
  int   i;                      /* loop variable */
  TUPLE **p;                    /* to traverse the tuples */

  assert(tab);                  /* check function arguments */
  i = as_attcnt(tab->attset);   /* check and adapt the insert pos. */
  if (pos > i)      pos = i;    /* and the number of columns */
  if (cnt > i -off) cnt = i -off;
  assert((cnt >= 0) && (off  >= 0)
      && (pos >= 0) && ((pos <= off) || (pos >= off +cnt)));
  as_attmove(tab->attset, off, cnt, pos);  /* move attributes */
  for (p = tab->tpls +(i = tab->tplcnt); --i >= 0; )
    v_move((*--p)->cols, off, cnt, pos, (int)sizeof(INST));
}  /* tab_colmove() */          /* shift columns in tuples */

/*--------------------------------------------------------------------*/

int tab_colcut (TABLE *dst, TABLE *src, int mode, ...)
{                               /* --- cut some table columns */
  va_list   args;               /* list of variable arguments */
  int       i, k, n = 0;        /* loop variables, buffers */
  int       old = 0;            /* old number of tuples in dest. */
  int       add = 0;            /* number of tuples added to dest. */
  int       off, cnt, end, rem; /* range of columns */
  TUPLE     **s, **d;           /* to traverse the tuples */
  INST      *sc, *dc;           /* to traverse the tuple columns */
  int       *mark;              /* to traverse the column markers */
  ATT_SELFN *selfn = 0;         /* attribute selection function */
  void      *data  = NULL;      /* attribute selection data */
  ATT       *att;               /* to traverse the attributes */

  assert(src);                  /* check the function arguments */

  /* --- get range of columns --- */
  va_start(args, mode);         /* start variable argument evaluation */
  if (mode & TAB_RANGE) {       /* if an index range is given */
    off = va_arg(args, int);    /* get the offset to the first column */
    cnt = va_arg(args, int);    /* and the number of columns */
    i = as_attcnt(src->attset) -off;
    if (cnt > i) cnt = i;       /* check and adapt number of tuples */
    assert((off >= 0) && (cnt >= 0)); }
  else {                        /* if no index range given */
    off = 0; cnt = as_attcnt(src->attset);
  }                             /* get the full index range */
  if (mode & TAB_SELECT) {      /* if to select cols. by a function */
    selfn = va_arg(args, ATT_SELFN*);
    data  = va_arg(args, void*);/* get the column selection function */
  }                             /* and the column selection data */
  va_end(args);                 /* end variable argument evaluation */
  end = off +cnt;               /* compute index of last column +1 */
  rem = as_attcnt(src->attset) -end;  /* and the remaining columns */

  /* --- prepare destination --- */
  if (mode & (TAB_MARKED|TAB_SELECT)) {
    mark = src->marks;          /* if to cut marked/selected columns, */
    for (k = off; k < end; k++){/* traverse the range of columns */
      att = as_att(src->attset, k);
      if (mode & TAB_MARKED) {  /* if to cut marked columns */
        if (att_getmark(att) >= 0) *mark++ = 1;
        else {              cnt--; *mark++ = 0; } }
      else {                    /* if to cut selected columns */
        if (selfn(att, data) != 0) *mark++ = 1;
        else {              cnt--; *mark++ = 0; }
      }                         /* determine number of marked columns */
    }                           /* or of selected columns, resp., and */
  }                             /* set the markers in the buffer */
  if (dst) {                    /* if there is a destination table */
    n = as_attcnt(dst->attset); /* note the old number of attributes */
    old = dst->tplcnt;          /* and the number of tuples in dest. */
    add = src->tplcnt -old;     /* and get the number of add. tuples */
    if (_expand(dst, add, cnt) != 0) return -1;
  }                             /* expand the destination table */
  
  /* --- cut source columns --- */
  i = as_attcut((dst) ? dst->attset : NULL, src->attset,
                mode|AS_RANGE, off, end-off, selfn, data);
  if (i != 0) {                 /* copy attributes to destination */
    if (dst) _restore(dst, old, dst->tplcnt-old); return -1; }
  if (mode & (TAB_MARKED|TAB_SELECT)) {
    mark = src->marks;          /* if to cut marked/selected columns, */
    for (k = off; k < end; k++){/* traverse the range of columns */
      if (*mark++) {            /* if a column is marked/selected */
        if (!dst) continue;     /* if there is no dest., skip column */
        d = dst->tpls +src->tplcnt; /* otherwise traverse the tuples */
        for (s = src->tpls +(i = src->tplcnt); --i >= 0; )
          (*--d)->cols[n] = (*--s)->cols[k];
        n++; }                  /* copy marked columns to the dest. */
      else {                    /* if a column is not marked/selected */
        for (s = src->tpls +(i = src->tplcnt); --i >= 0; ) {
          --s; (*s)->cols[off] = (*s)->cols[k]; }
        off++;                  /* shift the unmarked columns */
      }                         /* to close the gap in the tuple */
    } }                         /* left by the cut columns */
  else {                        /* if to cut a range of columns */
    if (dst) {                  /* if there is a destination table, */
      s = src->tpls +src->tplcnt;  /* traverse the tuples and the */
      d = dst->tpls +src->tplcnt;  /* column range of each tuple */
      for (i = src->tplcnt; --i >= 0; ) {
        sc = (*--s)->cols +off +cnt;
        dc = (*--d)->cols +n   +cnt;
        for (k = cnt; --k >= 0; ) *--dc = *--sc;
      }                         /* copy the source columns */
    }                           /* to the destination tuple */
    if (rem > 0) {              /* if columns remain in source, */
      s = src->tpls;            /* traverse the source tuples */
      for (i = src->tplcnt; --i >= 0; s++) {
        sc = (*s)->cols +end; dc = (*s)->cols +off;
        for (k = rem; --k >= 0; ) *dc++ = *sc++;
      }                         /* copy remaining columns to close */
    }                           /* the gap left by the cut columns */
  }
  if (!dst) return 0;           /* if there is no dest. table, abort */

  /* --- fill uncopied table section --- */
  n = as_attcnt(dst->attset) -cnt;
  if (add > 0) {                /* if tuples were added, */
    s = src->tpls +src->tplcnt; /* copy the weights of these tuples */
    d = dst->tpls +src->tplcnt; /* and fill uncopied table section */
    for (i = add; --i >= 0; ) (*--d)->weight = (*--s)->weight;
    tab_filluv(dst, old,          add, 0, n); }
  else                          /* if no tuples were added */
    tab_filluv(dst, src->tplcnt, -add, n, cnt);
  return 0;                     /* fill uncopied table section */
}  /* tab_colcut() */           /* and return 'ok' */

/*--------------------------------------------------------------------*/

int tab_colcopy (TABLE *dst, const TABLE *src, int mode, ...)
{                               /* --- copy some table columns */
  va_list    args;              /* list of variable arguments */
  int        i, k, n;           /* loop variables, buffers */
  int        old;               /* old number of tuples in dest. */
  int        add;               /* number of tuples to add to dest. */
  int        off, cnt, end;     /* range of columns */
  TUPLE      *const*s, **d;     /* to traverse the tuples */
  const INST *sc;               /* to traverse the source columns */
  INST       *dc;               /* to traverse the dest.  columns */
  int        *mark;             /* to traverse the column markers */
  ATT_SELFN  *selfn = 0;        /* attribute selection function */
  void       *data  = NULL;     /* attribute selection data */
  const ATT  *att;              /* to traverse the attributes */

  assert(src && dst);           /* check the function arguments */

  /* --- get range of columns --- */
  va_start(args, mode);         /* start variable argument evaluation */
  if (mode & TAB_RANGE) {       /* if an index range is given */
    off = va_arg(args, int);    /* get the offset to the first column */
    cnt = va_arg(args, int);    /* and the number of columns */
    i = as_attcnt(src->attset) -off;
    if (cnt > i) cnt = i;       /* check and adapt number of tuples */
    assert((off >= 0) && (cnt >= 0)); }
  else {                        /* if no index range given */
    off = 0; cnt = as_attcnt(src->attset);
  }                             /* get the full index range */
  if (mode & TAB_SELECT) {      /* if to select cols. by a function */
    selfn = va_arg(args, ATT_SELFN*);
    data  = va_arg(args, void*);/* get the column selection function */
  }                             /* and the column selection data */
  va_end(args);                 /* end variable argument evaluation */
  end = off +cnt;               /* compute index of last column +1 */

  /* --- prepare destination --- */
  if (mode & (TAB_MARKED|TAB_SELECT)) {
    mark = src->marks;          /* if to cut marked/selected columns, */
    for (k = off; k < end; k++){/* traverse the range of columns */
      att = as_att(src->attset, k);
      if (mode & TAB_MARKED) {  /* if to cut marked columns */
        if (att_getmark(att) >= 0) *mark++ = 1;
        else {              cnt--; *mark++ = 0; } }
      else {                    /* if to cut selected columns */
        if (selfn(att, data) != 0) *mark++ = 1;
        else {              cnt--; *mark++ = 0; }
      }                         /* determine number of marked columns */
    }                           /* or of selected columns, resp., and */
  }                             /* set the markers in the buffer */
  old = dst->tplcnt;            /* note the number of tuples in dest. */
  add = src->tplcnt -old;       /* and get the number of add. tuples */
  if (_expand(dst, add, cnt) != 0)
    return -1;                  /* expand the destination table */

  /* --- copy source columns --- */
  n = as_attcnt(dst->attset);   /* note the old number of attributes */
  i = as_attcopy(dst->attset,   /* and copy the attributes */
                 src->attset, mode|AS_RANGE, off, end-off, selfn, data);
  if (i != 0) { _restore(dst, old, src->tplcnt -old); return -1; }
  if (mode & (TAB_MARKED|TAB_SELECT)) {
    mark = src->marks;          /* if to copy marked/selected columns */
    for (k = off; k < end; k++){/* traverse the range of columns */
      if (*mark++) {            /* if column is marked/selected */
        s = src->tpls +src->tplcnt;
        d = dst->tpls +src->tplcnt;
        for (i = src->tplcnt; --i >= 0; )
          (*--d)->cols[n] = (*--s)->cols[k];
        n++;                    /* traverse the source columns */
      }                         /* and copy the marked columns */
    } }                         /* to the destination table */
  else {                        /* if to copy a range of columns */
    s = src->tpls +src->tplcnt; /* traverse the tuples and the */
    d = dst->tpls +src->tplcnt; /* column range of each tuple */
    for (i = src->tplcnt; --i >= 0; ) {
      sc = (*--s)->cols +off +cnt;
      dc = (*--d)->cols +n   +cnt;
      for (k = cnt; --k >= 0; ) *--dc = *--sc;
    }                           /* copy the source columns */
  }                             /* to the destination tuple */

  /* --- fill uncopied table section --- */
  n = as_attcnt(dst->attset) -cnt;
  if (add > 0) {                /* if tuples were added, */
    s = src->tpls +src->tplcnt; /* copy the weights of these tuples */
    d = dst->tpls +src->tplcnt; /* and fill uncopied table section */
    for (i = add; --i >= 0; ) (*--d)->weight = (*--s)->weight;
    tab_filluv(dst, old,          add, 0, n); }
  else                          /* if no tuples were added */
    tab_filluv(dst, src->tplcnt, -add, n, cnt);
  return 0;                     /* fill uncopied table section */
}  /* tab_colcopy() */          /* and return 'ok' */

/*----------------------------------------------------------------------
  Table Tuple Functions
----------------------------------------------------------------------*/

int tab_tpladd (TABLE *tab, TUPLE *tpl)
{                               /* --- add one tuple to a table */
  int  i;                       /* loop variable */
  INST *col;                    /* to traverse the tuple columns */

  assert(tab);                  /* check the function argument */
  if (tab_resize(tab, tab->tplcnt +1) < 0)
    return -1;                  /* resize the tuple vector */
  if (tpl) {                    /* if a tuple is given, */
    if (tpl->table)             /* remove it from the old table */
      tab_tplrem(tpl->table, tpl->id); }
  else {                        /* if no tuple is given */
    i = as_attcnt(tab->attset); /* get number of columns */
    tpl = (TUPLE*)malloc(sizeof(TUPLE) +(i-1) *sizeof(INST));
    if (!tpl) return -1;        /* allocate memory */
    col = tpl->cols +i;         /* copy instances (set columns) */
    while (--i >= 0) *--col = *att_inst(as_att(tab->attset, i));
    tpl->weight = as_getwgt(tab->attset);
    tpl->info   = *as_info(tab->attset);
    tpl->attset = tab->attset;  /* note the weight, the additional */
  }                             /* information, and the attribute set */
  tpl->table = tab;             /* set the table reference */
  tpl->id    = tab->tplcnt;     /* and the tuple identifier */
  tab->tpls[tab->tplcnt++] = tpl;  /* insert the tuple into the table */
  return 0;                     /* return 'ok' */
}  /* tab_tpladd() */

/*--------------------------------------------------------------------*/

int tab_tpladdm (TABLE *tab, TUPLE **tpls, int cnt)
{                               /* --- add several tuples */
  TUPLE *tpl;                   /* to traverse new tuples */

  assert(tab && ((cnt >= 0) || !tpls));  /* check function arguments */

  /* --- add empty tuples --- */
  if (!tpls) {                  /* if no tuples are given */
    if (_expand(tab, abs(cnt), 0) != 0) return -1;
    if (cnt < 0)                /* expand the table by 'cnt' tuples */
      tab_filluv(tab, tab->tplcnt +cnt,-cnt, 0, as_attcnt(tab->attset));
    return 0;                   /* fill the tuples with unknown */
  }                             /* values and abort the function */

  /* --- add given tuples --- */
  if (tab_resize(tab, tab->tplcnt +cnt) < 0)
    return -1;                  /* resize the tuple vector */
  while (--cnt >= 0) {          /* traverse the new tuples */
    tpl = *tpls++;              /* get the next tuple and */
    if (tpl->table)             /* remove it from old table */
      tab_tplrem(tpl->table, tpl->id);
    tpl->table = tab;           /* set the table reference */
    tpl->id    = tab->tplcnt;   /* and the tuple identifier */
    tab->tpls[tab->tplcnt++] = tpl;
  }                             /* insert the tuple into the table */
  return 0;                     /* return 'ok' */
}  /* tab_tpladdm() */

/*--------------------------------------------------------------------*/

TUPLE* tab_tplrem (TABLE *tab, int tplid)
{                               /* --- remove a tuple from a table */
  TUPLE **p;                    /* to traverse the tuples */
  TUPLE *tpl;                   /* removed tuple */

  assert(tab && (tplid < tab->tplcnt));  /* check function arguments */

  /* --- remove all tuples --- */
  if (tplid < 0) {              /* if no tuple identifier given */
    if (!tab->tpls) return NULL;/* if there are no tuples, abort */
    for (p = tab->tpls +(tplid = tab->tplcnt); --tplid >= 0; ) {
      (*--p)->table = NULL; tab->delfn(*p); }
    free(tab->tpls);            /* delete all tuples */
    tab->tpls = NULL;           /* and the tuple vector */
    tab->tplcnt = 0; return NULL;
  }                             /* abort the function */

  /* --- remove one tuple --- */
  tpl = tab->tpls[tplid];       /* get the tuple to remove */
  p   = tab->tpls +tplid;       /* traverse the remaining tuples */
  for (tplid = --tab->tplcnt -tplid; --tplid >= 0; ) {
    *p = p[1]; (*p++)->id--; }  /* shift tuples and adapt identifiers */
  tpl->table = NULL;            /* clear the table reference */
  tpl->id    = -1;              /* and the tuple identifier */
  tab_resize(tab, 0);           /* try to shrink the tuple vector */
  return tpl;                   /* return the removed tuple */
}  /* tab_tplrem() */

/*--------------------------------------------------------------------*/

void tab_tplexg (TABLE *tab, int tplid1, int tplid2)
{                               /* --- exchange two tuples */
  TUPLE **p1, **p2, *tpl;       /* temporary buffers */

  assert(tab && (tplid1 >= 0) && (tplid1 < tab->tplcnt)
             && (tplid2 >= 0) && (tplid2 < tab->tplcnt));
  p1  = tab->tpls +tplid1;      /* get pointers to the tuples, */
  p2  = tab->tpls +tplid2;      /* exchange them, and */
  tpl = *p1;                    /* set the new identifiers */
  *p1 = *p2; (*p1)->id = tplid1;
  *p2 = tpl; tpl->id   = tplid2;
}  /* tab_tplexg() */

/*--------------------------------------------------------------------*/

void tab_tplmove (TABLE *tab, int off, int cnt, int pos)
{                               /* --- move tuples */
  int   i;                      /* loop variable */
  TUPLE **p;                    /* to traverse the tuples */

  assert(tab);                  /* check for a valid table */
  i = tab->tplcnt; if (pos > i) pos = i;  /* check and adapt */
  i -= off;        if (cnt > i) cnt = i;  /* the insert position and */
  assert((cnt >= 0) && (off  >= 0)        /* the number of attributes */
      && (pos >= 0) && ((pos <= off) || (pos >= off +cnt)));
  v_move(tab->tpls, off, cnt, pos, (int)sizeof(TUPLE*));
  if (pos <= off) {             /* move the tuples in the vector */
    cnt += off; off = pos; pos = cnt; }
  p = tab->tpls +off;           /* set the new tuple identifiers */
  while (off < pos) (*p++)->id = off++;
}  /* tab_tplmove() */

/*--------------------------------------------------------------------*/

int tab_tplcut (TABLE *dst, TABLE *src, int mode, ...)
{                               /* --- cut some tuples */
  va_list   args;               /* list of variable arguments */
  int       i;                  /* loop variable */
  int       off, cnt;           /* range of tuples */
  TUPLE     **s, **d, **r;      /* to traverse the tuples */
  TPL_SELFN *selfn = 0;         /* tuple selection function */
  void      *data  = NULL;      /* tuple selection data */

  assert(src);                  /* check the function arguments */

  /* --- get range of tuples --- */
  va_start(args, mode);         /* start variable argument evaluation */
  if (mode & TAB_RANGE) {       /* if an index range is given, */
    off = va_arg(args, int);    /* get the offset to the first tuple */
    cnt = va_arg(args, int);    /* and the number of tuples */
    i   = src->tplcnt -off;     /* check and adapt */
    if (cnt > i) cnt = i;       /* the number of tuples */
    assert((off >= 0) && (cnt >= 0)); }
  else {                        /* if no index range given, */
    off = 0; cnt = src->tplcnt; /* get the full index range */
  }
  if (mode & TAB_SELECT) {      /* if to select tuples by a function */
    selfn = va_arg(args, TPL_SELFN*);
    data  = va_arg(args, void*);/* get the tuple selection function */
  }                             /* and the tuple selection data */
  va_end(args);                 /* end variable argument evaluation */
  if (cnt <= 0) return 0;       /* if the range is empty, abort */
  if (dst && (tab_resize(dst, dst->tplcnt +cnt) < 0))
    return -1;                  /* resize the dest. tuple vector */

  /* --- cut source tuples --- */
  d = (dst) ? dst->tpls +dst->tplcnt : NULL;
  s = r = src->tpls +off;       /* get destination and source */
  for (i = cnt; --i >= 0; s++){ /* traverse the tuples in range */
    if (((mode & TAB_MARKED)    /* if in marked mode */
    &&   ((*s)->mark < 0))      /* and the tuple is not marked */
    ||  ((mode & TAB_SELECT)    /* or in selection mode */
    &&   (!selfn(*s, data)))) { /* and the tuple does not qualify, */
      (*s)->id = (int)(r -src->tpls);   /* set the new tuple id. */
      *r++ = *s; }              /* and shift the tuple down/left */
    else {                      /* if to cut the tuple, */
      (*s)->table = dst;        /* set/clear the table reference */
      if (dst) { (*s)->id = dst->tplcnt++; *d++ = *s; }
      else     { src->delfn(*s); }
    }                           /* store the tuple in the */
  }                             /* destination or delete it */
  for (i = src->tplcnt -off -cnt; --i >= 0; ) {
    (*s)->id = (int)(r -src->tpls); *r++ = *s++;
  }                             /* shift down/left remaining tuples */
  src->tplcnt = (int)(r -src->tpls);  /* set new number of tuples */
  tab_resize(src, 0);           /* try to shrink the tuple vector */
  if (dst) tab_resize(dst, 0);  /* of the source and the destination */
  return 0;                     /* return 'ok' */
}  /* tab_tplcut() */

/*--------------------------------------------------------------------*/

int tab_tplcopy (TABLE *dst, const TABLE *src, int mode, ...)
{                               /* --- copy some tuples */
  va_list   args;               /* list of variable arguments */
  int       i, n;               /* loop variable, number of columns */
  int       off, cnt;           /* range of tuples */
  TUPLE     *const*s, **d;      /* to traverse the tuples */
  TPL_SELFN *selfn = 0;         /* tuple selection function */
  void      *data  = NULL;      /* tuple selection data */

  assert(src && dst);           /* check the function arguments */

  /* --- get range of tuples --- */
  va_start(args, mode);         /* start variable argument evaluation */
  if (mode & TAB_RANGE) {       /* if an index range is given, */
    off = va_arg(args, int);    /* get the offset to the first tuple */
    cnt = va_arg(args, int);    /* and the number of tuples */
    i   = src->tplcnt -off;     /* check and adapt */
    if (cnt > i) cnt = i;       /* the number of tuples */
    assert((off >= 0) && (cnt >= 0)); }
  else {                        /* if no index range given, */
    off = 0; cnt = src->tplcnt; /* get the full index range */
  }
  if (mode & TAB_SELECT) {      /* if to select tuples by a function */
    selfn = va_arg(args, TPL_SELFN*);
    data  = va_arg(args, void*);/* get the tuple selection function */
  }                             /* and the tuple selection data */
  va_end(args);                 /* end variable argument evaluation */
  if (cnt <= 0) return 0;       /* if the range is empty, abort */
  if (tab_resize(dst, dst->tplcnt +cnt) < 0)
    return -1;                  /* resize the dest. tuple vector */

  /* --- copy source tuples --- */
  n = as_attcnt(src->attset);   /* get the number of columns */
  d = dst->tpls +dst->tplcnt;   /* and the destination and */
  s = src->tpls +off;           /* source tuple pointers */
  for (i = cnt; --i >= 0; s++){ /* traverse the tuples in the range */
    if (((mode & TAB_MARKED)    /* if in marked mode */
    &&   ((*s)->mark < 0))      /* an the tuple is not marked */
    ||  ((mode & TAB_SELECT)    /* or in selection mode */
    &&   (!selfn(*s, data))))   /* and the tuple does not qualify, */
      continue;                 /* skip this tuple */
    *d = (TUPLE*)malloc(sizeof(TUPLE) +(n-1) *sizeof(INST));
    if (!*d) break;             /* create a new tuple */
    (*d)->attset = dst->attset;
    (*d)->id     = (int)(d -dst->tpls);
    (*d)->table  = dst;         /* store the tuple in the destination */
    tpl_copy(*d++, *s);         /* (set identifier and table ref.) */
  }                             /* and copy the source tuple */
  if (i >= 0) {                 /* if an error occurred */
    for (i = (int)(d -(dst->tpls +dst->tplcnt)); --i > 0; )
      free(*--d);               /* delete all copied tuples */
    return -1;                  /* and abort the function */
  }
  dst->tplcnt = (int)(d -dst->tpls);  /* set new number of tuples */
  tab_resize(dst, 0);           /* try to shrink the tuple vector */
  return 0;                     /* return 'ok' */
}  /* tab_tplcopy() */
