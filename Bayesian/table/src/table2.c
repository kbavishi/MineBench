/*----------------------------------------------------------------------
  File    : table2.c
  Contents: tuple and table management, one point coverage functions
  Author  : Christian Borgelt
  History : 06.03.1996 file created
            25.09.1998 first step of major redesign completed
            04.02.1999 all occurences of long int changed to int
            15.03.1999 one point coverage functions transf. from opc.c
            17.03.1999 one point coverage functions redesigned
            24.06.2001 module split into two files
            11.07.2001 bug in function _opc_cond removed
            16.01.2003 functions tpl_compat, tab_poss, tab_possx added
----------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include <assert.h>
#include "table.h"
#ifdef STORAGE
#include "storage.h"
#endif

extern int tab_resize (TABLE *tab, int size);

/*----------------------------------------------------------------------
  Preprocessor Definitions
----------------------------------------------------------------------*/
#define BLKSIZE    256          /* tuple vector block size */

/*----------------------------------------------------------------------
  Type Definitions
----------------------------------------------------------------------*/
typedef struct {                /* --- one point coverage data --- */
  TABLE *table;                 /* table to work on */
  TUPLE **buf;                  /* tuple buffer */
  TUPLE **htab;                 /* hash table */
  TUPLE *curr;                  /* current tuples */
  float *wgts;                  /* weights buffer */
} OPCDATA;                      /* (one point coverage data) */

/*----------------------------------------------------------------------
  Tuple Functions
----------------------------------------------------------------------*/

int tpl_isect (TUPLE *res, TUPLE *tpl1, const TUPLE *tpl2)
{                               /* --- compute a tuple intersection */
  ATT   *att;                   /* to traverse the attributes */
  INST  *inst;                  /* to traverse the instances */
  CINST *col1, *col2;           /* to traverse the tuple columns */
  int   cnt;                    /* number of columns/loop variable */
  int   type;                   /* type of current column */
  int   unkval;                 /* type specific unknown value */
  int   ident = 3;              /* 'identical to intersection' flags */

  assert(tpl1 && tpl2           /* check the function arguments */
      && (as_attcnt(tpl1->attset) == as_attcnt(tpl2->attset)));
  cnt  = as_attcnt(tpl1->attset); 
  col1 = tpl1->cols +cnt;       /* get the number of columns and */
  col2 = tpl2->cols +cnt;       /* the column vectors of the tuples */
  while (--cnt >= 0) {          /* traverse the tuple columns */
    col1--; col2--;             /* advance the column pointers */
    att  = as_att(tpl1->attset, cnt);
    inst = (res) ? res->cols +cnt : att_inst(att);
    type = att_type(att);       /* get the instance and the att. type */
    if (type == AT_FLT) {       /* if real/float valued column */
      if      (col1->f == col2->f)   /* if the values are identical, */
        inst->f = col1->f;           /* just copy any of them */
      else if (col1->f <= UV_FLT) {  /* if first value is unknown, */
        inst->f = col2->f;           /* copy second value and */
        ident &= ~1; }               /* clear first tuple flag */
      else if (col2->f <= UV_FLT) {  /* if second value is unknown, */
        inst->f = col1->f;           /* copy first value and */
        ident &= ~2; }               /* clear second tuple flag */
      else return -1; }              /* if columns differ, abort */
    else {                      /* if integer or symbolic column */
      unkval = (type == AT_INT)      /* get appropriate */
             ? UV_INT : UV_SYM;      /* unknown value */
      if      (col1->i == col2->i)   /* if values are identical, */
        inst->i = col1->i;           /* just copy any of them */
      else if (col1->i <= unkval) {  /* if first value is unknown, */
        inst->i = col2->i;           /* copy second value and */
        ident &= ~1; }               /* clear first tuple flag */
      else if (col2->i <= unkval) {  /* if second value is unknown, */
        inst->i = col1->i;           /* copy first value and */
        ident &= ~2; }               /* clear second tuple flag */
      else return -1;                /* if columns differ, abort */
    }                           /* (ident indicates which tuple is */
  }                             /* identical to their intersection) */
  return ident;                 /* return ident flags */
}  /* tpl_isect() */

/*----------------------------------------------------------------------
Result of tpl_isect:
bit 0 is set: tpl1 is identical to intersection
bit 1 is set: tpl2 is identical to intersection
----------------------------------------------------------------------*/

int tpl_compat (const TUPLE *tpl1, const TUPLE *tpl2)
{                               /* --- check tuples for compatibility */
  ATT   *att;                   /* to traverse the attributes */
  int   i1, i2, k;              /* integer values, loop variable */
  float f1, f2;                 /* float values */

  if (!tpl1) { tpl1 = tpl2; tpl2 = NULL; }
  if (!tpl1) return -1;         /* check for at least one tuple */
  for (k = as_attcnt(tpl1->attset); --k >= 0; ) {
    att = as_att(tpl1->attset, k);    /* traverse the columns and */
    switch (att_type(att)) {    /* get the attribute and its type */
      case AT_FLT:              /* if real/float valued column */
        f1 = tpl1->cols[k].f;   /* check for different known values */
        f2 = (tpl2) ? tpl2->cols[k].f : att_inst(att)->f;
        if ((f1 != f2) && (f1 > UV_FLT) && (f2 > UV_FLT)) return 0;
        break;
      case AT_INT:              /* if integer valued column */
        i1 = tpl1->cols[k].i;   /* check for different known values */
        i2 = (tpl2) ? tpl2->cols[k].i : att_inst(att)->i;
        if ((i1 != i2) && (i1 > UV_INT) && (i2 > UV_INT)) return 0;
        break;
      default:                  /* if symbolic valued column */
        i1 = tpl1->cols[k].i;   /* check for different known values */
        i2 = (tpl2) ? tpl2->cols[k].i : att_inst(att)->i;
        if ((i1 != i2) && (i1 > UV_SYM) && (i2 > UV_SYM)) return 0;
        break;                  /* if different known values exist, */
    }                           /* return 'tuples are incompatible' */
  }
  return -1;                    /* return 'tuples are compatible' */
}  /* tpl_compat() */

/*--------------------------------------------------------------------*/

int tpl_uvcnt (const TUPLE *tpl)
{                               /* --- count unknown values */
  CINST *col;                   /* to traverse the columns */
  int   i;                      /* loop variable */
  int   cnt = 0;                /* number of unknown columns */

  assert(tpl);                  /* check the function argument */
  for (col = tpl->cols +(i = as_attcnt(tpl->attset)); --i >= 0; ) {
    switch (att_type(as_att(tpl->attset, i))) {
      case AT_FLT: if ((--col)->f <= UV_FLT) cnt++; break;
      case AT_INT: if ((--col)->i <= UV_INT) cnt++; break;
      default    : if ((--col)->i <= UV_SYM) cnt++; break;
    }                           /* traverse the tuple columns */
  }                             /* and if a column is unknown, */
  return cnt;                   /* increment the column counter; */
}  /* tpl_uvcnt() */            /* return number of unknown columns */

/*----------------------------------------------------------------------
  Table Functions
----------------------------------------------------------------------*/

static int _htab (OPCDATA *opc, int init)
{                               /* --- create/resize hash table */
  int   i;                      /* loop variable */
  TABLE *tab = opc->table;      /* table to work on */
  TUPLE *tpl, **p, **hb;        /* to traverse tuples/hash buckets */

  assert(opc && tab);           /* check the function arguments */
  if (opc->htab) free(opc->htab);      /* delete old hash table */
  opc->htab = (TUPLE**)calloc(tab->tplvsz, sizeof(TUPLE*));
  if (!opc->htab) return -1;    /* allocate a new hash table */
  for (p = tab->tpls +(i = tab->tplcnt); --i >= 0; ) {
    tpl = *--p;                 /* traverse the tuples in the table */
    if (init) tpl->id = tpl_hash(tpl);
    hb  = opc->htab +(unsigned int)tpl->id % tab->tplvsz;
    tpl->table = (TABLE*)(*hb); /* compute the hash value and get */
    *hb = tpl;                  /* the corresponding bucket, then */
  }                             /* insert the tuple into the bucket */
  return 0;                     /* return 'ok' */
}  /* _htab() */

/*----------------------------------------------------------------------
The one point coverage functions below use a hash table to find tuples
quickly. This hash table is stored in the field tab->htab and is created
and resized with the above function. Within a hash bucket the tuples are
chained together with the (abused) tpl->table pointers. For a faster
reorganization of the hash table, the hash value of a tuple is stored in
the tuple's (abused) tpl->id field, so that it need not be recomputed.
----------------------------------------------------------------------*/

static int _opcerr (OPCDATA *opc, int addcnt)
{                               /* --- clean up after o.p.c. error */
  int   i;                      /* loop variable */
  TABLE *tab = opc->table;      /* table to work on */
  TUPLE **p;                    /* to traverse the tuples */
  float *w;                     /* to traverse the tuple weights */

  assert(opc && tab);           /* check the function arguments */
  tab->tplcnt -= addcnt;        /* recompute old number of tuples */
  for (p = tab->tpls +tab->tplcnt +addcnt; --addcnt >= 0; )
    free(*--p);                 /* delete the added tuples and */
  tab_resize(tab, 0);           /* try to shrink the tuple vector */
  if (opc->wgts) {              /* if there is a weight vector, */
    w = opc->wgts +tab->tplcnt; /* traverse it and the tuples */
    for (p = tab->tpls +(i = tab->tplcnt); --i >= 0; )
      (*--p)->weight = *--w;    /* copy back the tuple weights */
    free(opc->wgts);            /* and delete the weight buffer */
  }                             /* then delete the work buffers */
  if (opc->htab) free(opc->htab);
  if (opc->buf)  free(opc->buf);
  if (opc->curr) free(opc->curr);
  for (p = tab->tpls +(i = tab->tplcnt); --i >= 0; ) {
    (*--p)->table = tab; (*p)->id = i; }
  return -1;                    /* restore the table ref. and the */
}  /* _opcerr() */              /* tuple id and return an error code */

/*--------------------------------------------------------------------*/

static int _opc_full (TABLE *tab)
{                               /* --- det. full one point coverages */
  int     i, k;                 /* loop variables, temporary buffers */
  int     addcnt = 0;           /* number of added tuples */
  int     attcnt;               /* number of attributes */
  int     valcnt;               /* number of attribute values */
  TUPLE   *curr;                /* current expansion of a tuple */
  TUPLE   *tpl, **p;            /* to traverse the tuples */
  TUPLE   *tmp, **hb;           /* to traverse a hash bucket */
  float   *w;                   /* to traverse the tuple weights */
  ATT     *att;                 /* to traverse the attributes */
  INST    *col;                 /* to traverse the tuple columns */
  int     uvcnt;                /* number of unknown columns */
  int     *unk;                 /* to traverse unknown columns vector */
  UINT    hval;                 /* hash value of the current tuple */
  OPCDATA opc;                  /* one point coverage data */

  /* --- initialize --- */
  opc.table = tab;              /* note the table and */
  opc.htab  = opc.buf = NULL;   /* clear the buffer variables */
  opc.curr  = curr = tpl_create(tab->attset, 0);
  opc.wgts  = (float*)malloc(tab->tplcnt *sizeof(float));
  if (!curr || !opc.wgts        /* create a tuple buffer, */
  ||  (_htab(&opc, 1) != 0))    /* a tuple weight buffer, */
    return _opcerr(&opc, 0);    /* and a hash table for the tuples */
  w = opc.wgts +tab->tplcnt;    /* traverse all tuples in the table */
  for (p = tab->tpls +(i = tab->tplcnt); --i >= 0; )
    *--w = (*--p)->weight;      /* note the weights of the tuples */

  /* --- expand tuples with unknown values --- */
  attcnt = as_attcnt(tab->attset);
  for (p = tab->tpls +(i = tab->tplcnt); --i >= 0; ) {
    tpl = tab->tpls[i];         /* traverse the tuples in the table */

    /* -- find an expandable tuple --- */
    unk = tab->marks;           /* collect the unknown columns */
    for (col = tpl->cols +(k = attcnt); --k >= 0; ) {
      att = as_att(tab->attset, k);     /* get the corresp. attribute */
      valcnt = att_valcnt(att); /* and the number of attribute values */
      if (((--col)->i >= 0)     /* if the column value is known */
      ||  (valcnt     <= 0))    /* or the attribute is not symbolic, */
        curr->cols[k] = *col;   /* simply copy the attribute value */
      else {                    /* if the column value is unknown */
        curr->cols[k].i = valcnt -1;
        *unk++ = k;             /* impute the last value of */
      }                         /* the attribute domain and note */
    }                           /* the index of the unknown column */
    uvcnt = (int)(unk -tab->marks);
    if (uvcnt <= 0) continue;   /* check the number of unknown columns*/
    curr->weight = tpl->weight; /* get the tuple weight for expansion */

    /* -- generate and add expansions -- */
    do {                        /* tuple creation loop */
      hval = tpl_hash(curr);    /* search the current expansion */
      hb   = opc.htab +hval % tab->tplvsz;
      for (tmp = *hb; tmp; tmp = (TUPLE*)tmp->table)
        if (tpl_cmp(curr, tmp, NULL) == 0) break;
      if (tmp)                  /* if an identical tuple exists, */
        tmp->weight += tpl->weight;     /* sum the tuple weights */
      else {                    /* if no identical tuple exists, */
        k = tab_resize(tab, tab->tplcnt +1);
        if (k < 0)              /* resize the tuple vector */
          return _opcerr(&opc, addcnt);
        if (k > 0) {            /* if the tuple vector was resized, */
          if (_htab(&opc, 0) != 0)    /* resize the hash table, too */
            return _opcerr(&opc, addcnt);
          hb = opc.htab +hval % tab->tplvsz;
        }                       /* recompute the hash bucket */
        tmp = tpl_dup(curr);    /* duplicate the current expansion */
        if (!tmp) return _opcerr(&opc, addcnt);
        tab->tpls[tab->tplcnt++] = tmp;
        addcnt++;               /* add the new tuple to the table */
        tmp->id    = hval;      /* note the tuple's hash value */
        tmp->table = (TABLE*)*hb;
        *hb = tmp;              /* insert the new tuple */
      }                         /* into the hash table */
      for (unk = tab->marks +(k = uvcnt); --k >= 0; ) {
        col = curr->cols + *--unk;    /* traverse the unknown columns */
        if (col->i > 0) { col->i--; break; }
        col->i = att_valcnt(as_att(tab->attset, *unk)) -1;
      }                         /* compute the next value combination */
    } while (k >= 0);           /* while there is another combination */

    /* -- remove the expanded tuple -- */
    hb = opc.htab +(unsigned int)tpl->id % tab->tplvsz;
    while (*hb != tpl) hb = (TUPLE**)&(*hb)->table;
    *hb = (TUPLE*)tpl->table;   /* remove the expanded tuple from the */
    tpl->table = tab;           /* hash table and mark it for deletion*/
  }  /* for (p = tab->tpls + .. */

  /* --- clean up --- */
  free(opc.wgts); free(opc.curr);
  free(opc.htab);               /* delete the work buffers */
  p = hb = tab->tpls;           /* traverse the tuples in the table */
  for (i = tab->tplcnt; --i >= 0; p++) {
    if ((*p)->table == tab) {   /* if the tuple has been expanded */
      (*p)->table = NULL; tab->delfn(*p);
      continue;                 /* remove and delete the tuple */
    }                           /* and continue with the next tuple */
    (*p)->table = tab; (*p)->id = (int)(hb -tab->tpls);
    *hb++ = *p;                 /* restore the table reference and */
  }                             /* set the new tuple identifier */
  tab->tplcnt = (int)(hb -tab->tpls);
  tab_resize(tab, 0);           /* set the new number of tuples, */
  return 0;                     /* try to shrink the tuple vector, */
}  /* _opc_full() */            /* and return 'ok' */

/*--------------------------------------------------------------------*/

static int _opc_cond (TABLE *tab)
{                               /* --- det. condensed one point cov. */
  int     i, k, r;              /* loop variables, temporary buffers */
  int     addcnt = 0;           /* number of added tuples */
  int     tuvvsz;               /* size of the tuple vector */
  int     tuvcnt;               /* number of tuples with unk. values */
  int     initcnt;              /* initial number of tuples with u.v. */
  TUPLE   *curr;                /* current intersection of tuples */
  TUPLE   **s, **d, *tpl;       /* to traverse the tuples */
  TUPLE   **hb, *tmp;           /* to traverse a hash bucket, buffer */
  UINT    hval;                 /* hash value of a tuple */
  OPCDATA opc;                  /* one point coverage data */

  /* --- initialize --- */
  opc.table = tab;              /* note the table and */
  opc.htab  = NULL;             /* clear the buffer variables */
  opc.curr  = NULL; opc.wgts = NULL;
  tuvvsz    = tab->tplcnt;      /* get the number of tuples */
  opc.buf   = d = (TUPLE**)malloc(tuvvsz *sizeof(TUPLE*));
  if (!d) return -1;            /* allocate a tuple vector */
  for (s = tab->tpls, i = tab->tplcnt; --i >= 0; s++) {
    if (tpl_uvcnt(*s) > 0)      /* collect in the tuple vector */
      *d++ = *s;                /* all tuples with unknown values and */
  }                             /* compute the number of these tuples */
  /* The tuple vector tab->tpls must be traversed in this order */
  /* (forward) because of the way the weights are distributed below. */
  tuvcnt = initcnt = (int)(d -opc.buf);
  if (tuvcnt <= 0) {            /* if no tuples contain unk. values, */
    _opcerr(&opc,0); return 0;} /* there is nothing to do, so abort */
  opc.curr = curr = tpl_create(tab->attset, 0);
  if (!curr || (_htab(&opc, 1) != 0))     /* create a tuple buffer */
    return _opcerr(&opc, 0);    /* and a hash table for the tuples */
  curr->weight = 0.0F;          /* clear the current tuple weight */

  /* --- compute closure under tuple intersection --- */
  for (i = 0; i < tuvcnt; i++){ /* traverse all tuples */
    tpl = opc.buf[i];           /* with unknown values */
    for (k = i; --k >= 0; ) {   /* traverse the preceding tuples */
      if (tpl_isect(curr, tpl, opc.buf[k]) != 0)
        continue;               /* skip tuples with no intersection */
      hval = tpl_hash(curr);    /* compute hash value of intersection */
      hb   = opc.htab +hval % tab->tplvsz;          /* and look it up */
      for (tmp = *hb; tmp; tmp = (TUPLE*)tmp->table)
        if (tpl_cmp(curr, tmp, NULL) == 0) break;
      if (tmp) continue;        /* skip existing intersections */
      r = tab_resize(tab, tab->tplcnt +1);
      if (r < 0)                /* resize the tuple vector */
        return _opcerr(&opc, addcnt);
      if (r > 0) {              /* if the tuple vector was resized, */
        if (_htab(&opc, 0) != 0)      /* resize the hash table, too */
          return _opcerr(&opc, addcnt);
        hb = opc.htab +hval % tab->tplvsz;
      }                         /* recompute the hash bucket */
      tmp = tpl_dup(curr);      /* duplicate the current expansion */
      if (!tmp) return _opcerr(&opc, addcnt);
      tab->tpls[tab->tplcnt++] = tmp;
      addcnt++;                 /* add the new tuple to the table */
      tmp->id    = hval;        /* note the tuple's hash value */
      tmp->table = (TABLE*)*hb; /* and insert the new tuple */
      *hb = tmp;                /* into the hash table */
      if (tpl_uvcnt(tmp) <= 0)  /* if the new tuple contains */
        continue;               /* no unknown values, continue */
      if (tuvcnt >= tuvvsz) {   /* if the tuple vector is full */
        tuvvsz += (tuvvsz > BLKSIZE) ? (tuvvsz >> 1) : BLKSIZE;
        hb = (TUPLE**)realloc(opc.buf, tuvvsz *sizeof(TUPLE*));
        if (!hb) return _opcerr(&opc, addcnt);
        opc.buf = hb;           /* resize the tuple vector */
      }                         /* and set the new vector */
      opc.buf[tuvcnt++] = tmp;  /* add the new tuple */
    }  /* for (k = i; ... */    /* to the tuple vector */
  }  /* for (i = 0; ... */

  /* --- distribute weights --- */
  /* This way of distributing the weights presupposes that the tuples */
  /* with unknown values are in the same order in the tuple buffer as */
  /* in the complete table. The weight distribution process may be */
  /* improvable by exploiting a sorted table. */
  for (d = tab->tpls +(k = tab->tplcnt); --k >= 0; ) {
    --d;                        /* traverse all tuples in the table */
    for (s = opc.buf +(i = initcnt); --i >= 0; ) {
      if (*--s == *d) {         /* traverse all initial u.v. tuples, */
        initcnt--; continue; }  /* but skip the tuple itself */
      if (tpl_isect(curr, *s, *d) >= 2)
        (*d)->weight += (*s)->weight;
    }                           /* if a table tuple is more specific, */
  }                             /* add the weight of the u.v. tuple */

  /* --- clean up --- */
  free(opc.curr); free(opc.buf);
  free(opc.htab);               /* delete the work buffers and */
  tab_resize(tab, 0);           /* try to shrink the tuple vector */
  return 0;                     /* return 'ok' */
}  /* _opc_cond() */

/*--------------------------------------------------------------------*/

int tab_opc (TABLE *tab, int mode)
{                               /* --- compute one point coverages */
  int    i, r;                  /* loop variable, return code */
  double norm = 0;              /* normalization factor */
  TUPLE  **p;                   /* to traverse the tuples */

  assert(tab);                  /* check the function argument */
  if (mode & TAB_NORM) {        /* if to normalize the one point cov. */
    for (p = tab->tpls +(i = tab->tplcnt); --i >= 0; )
      norm += (*--p)->weight;   /* traverse all tuples */
    if (norm <= 0) return 0;    /* and sum the tuple weights */
    norm = 1/norm;              /* compute the normalization factor */
  }
  r = (mode & TAB_FULL) ? _opc_full(tab) : _opc_cond(tab);
  if (r != 0) return r;         /* compute one point coverages */
  if (mode & TAB_NORM) {        /* if to normalize the one point cov. */
    for (p = tab->tpls +(i = tab->tplcnt); --i >= 0; )
      (*--p)->weight *= norm;   /* traverse all tuples and */
  }                             /* normalize the tuple weights */
  return 0;                     /* return 'ok' */
}  /* tab_opc() */              

/*----------------------------------------------------------------------
The function tab_opc requires the argument table to be reduced (and
sorted, but a call to tab_reduce automatically sorts the table).
----------------------------------------------------------------------*/

float tab_poss (TABLE *tab, TUPLE *tpl)
{                               /* --- det. degrees of possibility */
  int   i;                      /* loop variable */
  TUPLE **p;                    /* to traverse the tuples */
  float poss = 0;               /* degree of possibility */

  assert(tab);                  /* check the function arguments */
  for (p = tab->tpls +(i = tab->tplcnt); --i >= 0; ) {
    if (tpl_compat(*--p, tpl)   /* traverse the tuples in the table */
    &&  ((*p)->weight > poss))  /* and if the tuple is compatible */
      poss = (*p)->weight;      /* with the tuple in the table, */
  }                             /* update the degree of possibility */
  return poss;                  /* return the degree of possibility */
}  /* tab_poss() */

/*--------------------------------------------------------------------*/

void tab_possx (TABLE *tab, TUPLE *tpl, double res[])
{                               /* --- det. degrees of possibility */
  int    i, n = 0;              /* loop variable, counter */
  INST   *col, *inst;           /* to traverse the columns */
  ATT    *att;                  /* to traverse the attributes */
  double p;                     /* degree of possibility */

  assert(tab && tpl && res);    /* check the function arguments */
  for (col = tpl->cols +(i = tpl_colcnt(tpl)); --i >= 0; ) {
    att  = as_att(tab->attset, i);
    inst = att_inst(att);       /* traverse the tuple columns */
    if ((--col)->i > UV_SYM) { att_inst(att)->i = col->i; }
    else                { n++; att_inst(att)->i = att_valcnt(att)-1; }
  }                             /* copy the tuple to the att. set */
  if (n <= 0) {                 /* if precise tuple, no loop is nec. */
    res[0] = res[1] = res[2] = tab_poss(tab, tpl); return; }
  res[2] = res[0] = n = 0;      /* init. the aggregates to compute */
  res[1] = DBL_MAX;             /* and the instantiation counter */
  do {                          /* loop over compat. precise tuples */
    p = tab_poss(tab, NULL);    /* determine the possibility */
    if (p < res[1]) res[1] = p; /* update minimum, */
    if (p > res[2]) res[2] = p; /* maximum, and sum */
    res[0] += p; n++;           /* and count the tuple */
    for (col = tpl->cols +(i = tab_colcnt(tpl)); --i >= 0; ) {
      if ((--col)->i > UV_SYM) continue;
      att  = as_att(tab->attset, i);
      inst = att_inst(att);     /* traverse the unknown columns */
      if (--inst->i >= 0) break;
      inst->i = att_valcnt(att)-1;
    }                           /* compute next value combination */
  } while (i >= 0);             /* while there is another combination */
  if (n      >  0) res[0] /= n; /* determine average value and */
  if (res[2] <= 0) res[1]  = 0; /* correct minimum if maximum is zero */
}  /* tab_possx() */
