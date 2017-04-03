/*----------------------------------------------------------------------
  File    : attmap.c
  Contents: attribute map management (for symbolic to numeric coding)
  Author  : Christian Borgelt
  History : 11.08.2003 file created
            12.08.2003 function am_cnt added
----------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "attmap.h"

/*----------------------------------------------------------------------
  Functions
----------------------------------------------------------------------*/

ATTMAP* am_create (ATTSET *attset, int marked)
{                               /* --- create an attribute map */
  int    i, k, cnt;             /* loop variable, buffer */
  int    dim = 0;               /* number of dimensions */
  ATTMAP *map;                  /* created attribute map */
  ATT    *att;                  /* to traverse the attributes */
  AMEL   *p;                    /* to traverse the map elements */

  assert(attset);               /* check the function argument */
  for (i = cnt = as_attcnt(attset); --i >= 0; ) {
    att = as_att(attset, i);    /* traverse the (marked) attributes */
    if (marked && (att_getmark(att) < 0)) continue;
    if (att_type(att) != AT_SYM)/* if the attribute is numeric, */
      dim++;                    /* use only one dimension */
    else {                      /* if the attribute is symbolic, */
      k = att_valcnt(att);      /* get its number of values */
      dim += (k > 2) ? k : 1;   /* use one dim. for binary attribs. */
    }                           /* and otherwise as many dims. as */
  }                             /* are needed for a 1-in-n coding */
  map = (ATTMAP*)malloc(sizeof(ATTMAP) +(dim-1) *sizeof(AMEL));
  if (!map) return NULL;        /* create an attribute map */
  map->attset = attset;         /* and initialize it */
  map->attcnt = cnt;
  map->dim    = dim;
  for (p = map->amels, i = dim = 0; i < cnt; p++, i++) {
    p->att = as_att(attset, i); /* traverse the attributes */
    switch (att_type(p->att)) { /* note the input attribute */
      case AT_FLT: p->type = -2; p->off = dim++; break;
      case AT_INT: p->type = -1; p->off = dim++; break;
      default    : p->type = att_valcnt(p->att); p->off = dim;
                   dim += (p->type > 2) ? p->type : 1; break;
    }                           /* note the type/number of values */
  }                             /* (num. atts. always have one unit) */
  return map;                   /* return the created attribute map */
}  /* am_create() */

/*--------------------------------------------------------------------*/

void am_delete (ATTMAP *map)
{ free(map); }                  /* --- delete an attribute map */

/*--------------------------------------------------------------------*/

int am_cnt (ATTMAP *map, int attid)
{                               /* --- get number of attribute dim. */
  int n = map->amels[attid].type;
  return (n > 2) ? n : 1;       /* check the attribute type */
}  /* am_cnt() */

/*--------------------------------------------------------------------*/

void am_exec (ATTMAP *map, const TUPLE *tpl, double *vec)
{                               /* --- execute an attribute map */
  int        k, n, v;           /* loop variables, buffer */
  AMEL       *p;                /* to traverse the map elements */
  const INST *inst;             /* to traverse the instantiations */

  assert(map && vec);           /* check the function arguments */
  p = map->amels;               /* traverse the attributes */
  for (k = map->attcnt; --k >= 0; ) {
    inst = (tpl) ? tpl_colval(tpl, att_id(p->att)) : att_inst(p->att);
    n    = (p++)->type;         /* get the attribute instantiation */
    if      (n <  0) {          /* if the attribute is numeric */
      *vec++ = (n < -1) ? inst->f : inst->i; }
    else if (n <= 2) {          /* if the attribute is binary, */
      v = inst->i;              /* set the value directly */
      *vec++ = ((v >= 0) && (v < n)) ? v : 0.5; }
    else {                      /* if the attribute is symbolic */
      for (v = n; --v >= 0; )  vec[v] = 0;
      v = inst->i;              /* clear all fields */
      if ((v >= 0) && (v < n)) vec[v] = 1;
      vec += n;                 /* get the symbolic value and */
    }                           /* set the corresponding field */
  }                             /* set input and target values */
}  /* am_exec() */
