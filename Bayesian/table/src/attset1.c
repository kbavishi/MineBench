/*----------------------------------------------------------------------
  File    : attset1.c
  Contents: attribute set management, base functions
  Author  : Christian Borgelt
  History : 26.10.1995 file created
            21.12.1995 function att_valsort added
            17.03.1996 attribute types added
            04.07.1996 attribute weights added
            26.02.1997 default attribute name generation added
            12.03.1997 attribute marks added
            27.05.1997 function att_conv added
            30.08.1997 removal of field vector made possible
            22.06.1998 deletion function moved to function as_create
            23.06.1998 major redesign, attribute functions introduced
            23.08.1998 attribute creation and deletion functions added
            30.08.1998 parameters map and dir added to att_valsort
            01.09.1998 several assertions added
            06.09.1998 second major redesign completed
            12.09.1998 deletion function parameter changed to ATT
            24.09.1998 parameter map added to function att_conv
            25.09.1988 function as_attaddm added
            25.11.1998 functions att_valcopy and as_attcopy added
            29.11.1998 functions att_dup and as_dup added
            04.02.1999 long int changed to int
            22.11.2000 functions sc_format and sc_fmtlen exported
            23.06.2001 module split into two files
            16.07.2001 return code of as_attadd set to 1 if att. exists
----------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>
#include "vecops.h"
#include "attset.h"
#include "scan.h"
#ifdef STORAGE
#include "storage.h"
#endif

/*----------------------------------------------------------------------
  Preprocessor Definitions
----------------------------------------------------------------------*/
#define BLKSIZE       16        /* block size for vectors */

/*----------------------------------------------------------------------
  Auxiliary Functions
----------------------------------------------------------------------*/

static int _length (const char *s)
{                               /* --- compute bounded string length */
  register int n = AS_MAXLEN;   /* character counter */
  while (*s++ && (--n > 0));    /* determine the bounded length */
  return AS_MAXLEN -n;          /* of the string (max. AS_MAXLEN) */
}  /* _length() */

/*--------------------------------------------------------------------*/

static char* _copy (char *d, const char *s)
{                               /* --- copy string bounded */
  register int   n = AS_MAXLEN; /* character counter */
  register char *p = d;         /* to traverse the destination string */
  while (*s && (--n >= 0)) *p++ = *s++;
  *p = '\0';                    /* copy string (max. AS_MAXLEN ch.) */
  return d;                     /* and return the destination */
}  /* _copy() */

/*--------------------------------------------------------------------*/

static unsigned int _hash (const char *s)
{                               /* --- hash function */
  register unsigned int h = 0;  /* hash value */
  register int n = AS_MAXLEN;   /* character counter */

  while ((--n >= 0) && *s) h ^= (h << 3) ^ (unsigned int)(*s++);
  return h;                     /* compute and return hash value */
}  /* _hash() */

/*--------------------------------------------------------------------*/

static int _valcmp (const void *p1, const void *p2, void *data)
{                               /* --- compare two attribute values */
  return ((VAL_CMPFN*)data)(((const VAL*)p1)->name,
                            ((const VAL*)p2)->name);
}  /* _valcmp() */

/*--------------------------------------------------------------------*/
#if defined AS_RDWR || defined AS_FLDS

static void _delflds (ATTSET *set)
{                               /* --- delete field map */
  if (set->flds) { free(set->flds); set->flds = NULL; }
  set->fldcnt = set->fldvsz = 0;/* delete field vector */
}  /* _delflds() */             /* and clear counters */

#else
#define _delflds(s)
#endif
/*----------------------------------------------------------------------
  Attribute Functions
----------------------------------------------------------------------*/

static int att_resize (ATT *att, int size)
{                               /* --- resize value vector */
  int i;                        /* loop variable */
  VAL **p;                      /* to traverse values */
  VAL **hb;                     /* to traverse hash bucket */

  assert(att);                  /* check the function argument */
  i = att->valvsz;              /* get current vector size */
  if (size > 0) {               /* if to enlarge the vector */
    if (i >= size) return 0;    /* if vector is large enough, abort */
    i += (i > BLKSIZE) ? i >> 1 : BLKSIZE;
    if (i >  size) size = i; }  /* compute new vector size */
  else {                        /* if to shrink the vector */
    size = att->valcnt << 1;    /* get max. tolerable size */
    if (size < BLKSIZE) size = BLKSIZE;
    if (i <= size) return 0;    /* if vector is small enough, abort */
    size = att->valcnt +(att->valcnt >> 1);
    if (size < BLKSIZE) size = BLKSIZE;
  }                             /* compute new vector size */
  p  = (VAL**)realloc(att->vals, size *sizeof(VAL*));
  if (!p)  return -1;           /* resize value vector */
  att->vals = p;                /* and set new vector */
  hb = (VAL**)realloc(att->htab, size *sizeof(VAL*));
  if (!hb) return -1;           /* resize hash table */
  att->htab = hb;               /* and set new hash table */
  att->valvsz = size;           /* set new vector size */
  for (i = size; --i >= 0; )    /* clear hash table */
    hb[i] = NULL;               /* and traverse values */
  for (i = att->valcnt; --i >= 0; ) {
    hb = att->htab +(*p)->hval % size;
    (*p)->succ = *hb;           /* get pointer to hash bucket and */
    *hb = *p++;                 /* insert value into hash table */
  }                             /* (at the head of the bucket list) */
  return 0;                     /* return 'ok' */
}  /* att_resize() */

/*--------------------------------------------------------------------*/

ATT* att_create (const char *name, int type)
{                               /* --- create an attribute */
  ATT *att;                     /* created attribute */

  assert(name && *name);        /* check the function argument */
  att = (ATT*)malloc(sizeof(ATT));
  if (!att) return NULL;        /* allocate memory for att. and name */
  att->name = (char*)malloc((_length(name) +1) *sizeof(char));
  if (!att->name) { free(att); return NULL; }
  _copy(att->name, name);       /* copy attribute name and */
  att->hval = _hash(att->name); /* compute its hash value */
  att->attwd[0] = sc_fmtlen(att->name, att->attwd +1);
  att->vals     = att->htab     = NULL;
  att->valcnt   = att->valvsz   = att->mark = 0;
  att->valwd[0] = att->valwd[1] = 0;
  att->weight   = 1.0F;         /* initialize fields */
  att->succ     = NULL;
  att->info.p   = NULL;
  att->set      = NULL; att->id = -1;
  if      (type == AT_INT) {    /* if attribute is integer valued, */
    att->type   =  AT_INT;      /* note type */
    att->min.i  =  INT_MAX;     /* initialize minimal */
    att->max.i  = -INT_MAX;     /* and maximal value */
    att->inst.i =  UV_INT; }    /* and clear instance */
  else if (type == AT_FLT) {    /* if attribute is real valued, */
    att->type   =  AT_FLT;      /* note type */
    att->min.f  =  FLT_MAX;     /* initialize minimal */
    att->max.f  = -FLT_MAX;     /* and maximal value */
    att->inst.f =  UV_FLT; }    /* and clear instance */
  else {                        /* if attribute is symbolic, */
    att->type   =  AT_SYM;      /* note type */
    att->min.i  =  0;           /* initialize minimal */
    att->max.i  = -1;           /* and maximal value */
    att->inst.i =  UV_SYM;      /* and clear instance */
  }
  return att;                   /* return created attribute */
}  /* att_create() */

/*--------------------------------------------------------------------*/

ATT* att_dup (const ATT *att)
{                               /* --- duplicate an attribute */
  ATT *dup;                     /* created duplicate */

  assert(att);                  /* check the function arguments */
  dup = att_create(att->name, att->type);
  if (!dup) return NULL;        /* create a new attribute */
  if (att_valcopy(dup, att, 0) != 0) { att_delete(dup); return NULL; }
  dup->mark   = att->mark;      /* copy all attribute values */
  dup->weight = att->weight;    /* and all other information */
  dup->inst   = att->inst;      /* into the duplicate */
  dup->info   = att->info;
  return dup;                   /* return the created duplicate */
}  /* att_dup() */

/*--------------------------------------------------------------------*/

void att_delete (ATT *att)
{                               /* --- delete an attribute */
  int i;                        /* loop variable */
  VAL **p;                      /* to traverse the value vector */

  assert(att && att->name);     /* check the function argument */
  if (att->set)                 /* if there is a containing set, */
    as_attrem(att->set,att->id);/* remove the attribute from it */
  if (att->vals) {              /* if there are values */
    for (p = att->vals +(i = att->valcnt); --i >= 0; )
      free(*--p);               /* traverse and delete valeus */
    free(att->vals);            /* delete the value vector */
  }                             /* and the hash table */
  if (att->htab) free(att->htab);
  free(att->name);              /* delete attribute name */
  free(att);                    /* and attribute body */
}  /* att_delete() */

/*--------------------------------------------------------------------*/

int att_rename (ATT *att, const char *name)
{                               /* --- rename an attribute */
  int  hval;                    /* hash value of new attribute name */
  char *tmp;                    /* temporary buffer for name */
  ATT  **p, **hb = NULL;        /* buffers for hash buckets */  
  ATT  *t;                      /* to traverse a hash bucket */

  assert(att && name && *name); /* check the function arguments */
  hval = _hash(name);           /* and compute its hash value */
  if (att->set) {               /* if attribute is contained in a set */
    hb = att->set->htab +hval % att->set->attvsz;
    for (t = *hb; t; t = t->succ)
      if ((t != att) && strncmp(name, t->name, AS_MAXLEN) == 0)
        return -2;              /* check for another attribute */
  }                             /* with the same name */
  tmp = (char*)realloc(att->name, (_length(name) +1) *sizeof(char));
  if (!tmp) return -1;          /* reallocate memory for the new name */
  att->name = _copy(tmp, name); /* copy name and determine its widths */
  att->attwd[0] = sc_fmtlen(att->name, att->attwd +1);
  if (att->set) {               /* if attribute is contained in a set */
    p = att->set->htab +att->hval % att->set->attvsz;
    while (att != *p) p = &(*p)->succ;
    *p = (*p)->succ;            /* remove the attribute from the */
    att->succ = *hb; *hb = att; /* hash table and reinsert it */
  }                             /* into another hash bucket */
  att->hval = hval;             /* set the new hash value */
  return 0;                     /* return 'ok' */
}  /* att_rename() */

/*--------------------------------------------------------------------*/

int att_conv (ATT *att, int type, INST *map)
{                               /* --- convert attribute to new type */
  int    i, k;                  /* loop variables, buffers */
  double f;                     /* buffer for a floating point value */
  VAL    **p;                   /* to traverse the attribute values */
  char   *s;                    /* end pointer for conversion */

  assert(att);                  /* check the function argument */
  if ((att->type == AT_INT)     /* if to convert integer */
  &&  (type      == AT_FLT)) {  /* to real/float */
    att->type   = AT_FLT;       /* set new attribute type */
    att->min.f  = (float)att->min.i;  /* adapt minimal */
    att->max.f  = (float)att->max.i;  /* and maximal value */
    att->inst.f = (att->inst.i <= UV_INT)
                ? UV_FLT : (float)att->inst.i;
    return 0;                   /* adapt attribute instance */
  }                             /* and return 'ok' */
  if ((att->type == AT_FLT)     /* if to convert real/float */
  &&  (type      == AT_INT)) {  /* to integer */
    att->type  = AT_INT;        /* set new attribute type */
    att->min.i = (att->min.f > -INT_MAX)      /* adapt minimal */
               ? (int)att->min.f : -INT_MAX; /* and */
    att->max.i = (att->max.f <  INT_MAX)      /* maximal value */
               ? (int)att->max.f :  INT_MAX;
    if      (att->inst.f <= UV_FLT)  att->inst.i =  UV_INT;
    else if (att->inst.f < -INT_MAX) att->inst.i = -INT_MAX;
    else if (att->inst.f >  INT_MAX) att->inst.i =  INT_MAX;
    else                             att->inst.i = (int)att->inst.f;
    return 0;                   /* adapt attribute instance */
  }                             /* and return 'ok' */
  if (att->type != AT_SYM) {    /* if attribute is not symbolic */
    if (type != AT_SYM) return -1;
    att->type     = AT_SYM;     /* if to convert to symbolic */
    att->min.i    =  0;         /* clear minimal */
    att->max.i    = -1;         /* and maximal value, */
    att->valwd[0] = -1;         /* value widths, and */
    att->inst.i   = UV_SYM;     /* attribute instance */
    return 0;                   /* return 'ok' */
  }
  if (type == AT_AUTO) {        /* if automatic type determination */
    type = (att->valcnt > 0) ? AT_INT : AT_SYM;
    p    = att->vals;           /* traverse attribute values */
    for (k = att->valcnt; --k >= 0; p++) {
      i = (int)strtol((*p)->name, &s, 0); /* try to convert to int */
      if ((s == (*p)->name) || (*s != '\0') || (i <= UV_INT)) {
        type = AT_FLT; break; } /* if conversion was not successful, */
    }                           /* try to convert to float */
    for (k++; --k >= 0; p++) {  /* traverse remaining values */
      f = strtod((*p)->name, &s);    /* try to convert to float */
      if ((s == (*p)->name) || (*s != '\0')
      ||  (f <= UV_FLT) || (f > FLT_MAX)) {
        type = AT_SYM; break; } /* if conversion was not successful, */
    }                           /* attribute must stay symbolic */
  }                             /* (determine most specific type) */
  if      (type == AT_INT) {    /* if to convert to integer */
    att->min.i =  INT_MAX;      /* initialize */
    att->max.i = -INT_MAX;      /* range of values */
    for (p = att->vals +(k = att->valcnt); --k >= 0; ) {
      i = (int)strtol((*--p)->name, &s, 0); /* convert value to int */
      if (s == (*p)->name) {    /* skip unconvertable names */
        if (map) map[k].i = UV_INT; continue; }
      if (map) map[k].i = i;    /* if a map is requested, set it */
      if (i < att->min.i) att->min.i = i;
      if (i > att->max.i) att->max.i = i;
    }                           /* adapt range of values */
    if (att->min.i < -INT_MAX) att->min.i = -INT_MAX;
    if (att->inst.i < 0)        /* if the current value is unknown, */
      att->inst.i = UV_INT;     /* set instance to unknown integer */
    else {                      /* if the current value is known */
      p = att->vals +att->inst.i;
      i = (int)strtol((*p)->name, &s, 10);
      att->inst.i = (s == (*p)->name) ? UV_INT : i;
    } }                         /* adapt attribute instance */
  else if (type == AT_FLT) {    /* if to convert to float/real */
    att->min.f =  FLT_MAX;      /* initialize */
    att->max.f = -FLT_MAX;      /* range of values */
    for (p = att->vals +(k = att->valcnt); --k >= 0; ) {
      f = strtod((*--p)->name, &s); /* convert value to float */
      if (s == (*p)->name) {    /* skip unconvertable names */
        if (map) map[k].f = UV_FLT; continue; }
      if (map) map[k].f = (float)f; /* if a map is requested, set it */
      if (f < att->min.f) att->min.f = (float)f;
      if (f > att->max.f) att->max.f = (float)f;
    }                           /* adapt range of values */
    if (att->inst.i < 0)        /* if the current value is unknown, */
      att->inst.f = UV_FLT;     /* set instance to unknown float */
    else {                      /* if the current value is known */
      p = att->vals +att->inst.i;
      f = strtod((*p)->name, &s);
      att->inst.f = (s == (*p)->name) ? UV_FLT : (float)f;
    } }                         /* adapt attribute instance */
  else                          /* if no correct new type given or */
    return -1;                  /* no conversion possible, abort */
  att->type = type;             /* set new attribute type */
  if (att->vals) {              /* if there are attribute values, */
    for (p = att->vals +(i = att->valcnt); --i >= 0; )
      free(*--p);               /* delete attribute values */
    free(att->vals); free(att->htab);
    att->vals = att->htab = NULL;
  }                             /* delete value vector and hash table */
  att->valvsz = att->valcnt = 0;/* clear vector size and counter */
  return 0;                     /* return 'ok' */
}  /* att_conv() */

/*--------------------------------------------------------------------*/

int att_cmp (const ATT *att1, const ATT *att2)
{                               /* --- compare two attributes */
  int i;                        /* loop variable */
  VAL *const*p, *const*q;       /* to traverse values */

  assert(att1 && att2);         /* check the function arguments */
  if (att1->type != att2->type) /* compare attribute types and */
    return 1;                   /* if they are not equal, abort */
  if (att1->type == AT_INT) {   /* if attribute is integer valued */
    return ((att1->min.i != att2->min.i)
    ||      (att1->max.i != att2->max.i));
  }                             /* compare range of values */
  if (att1->type == AT_FLT) {   /* if attribute is real/float valued */
    return ((att1->min.f != att2->min.f)
    ||      (att1->max.f != att2->max.f));
  }                             /* compare range of values */
  if (att1->valcnt != att2->valcnt)
    return 1;                   /* compare number of values */
  p = att1->vals +att1->valcnt; /* traverse attribute values */
  q = att2->vals +att2->valcnt; /* and compare them */
  for (i = att1->valcnt; --i >= 0; )
    if (strcmp((*--p)->name, (*--q)->name) != 0)
      return 1;                 /* if a value differs, abort */
  return 0;                     /* otherwise return 'equal' */
}  /* att_cmp() */

/*----------------------------------------------------------------------
  Attribute Value Functions
----------------------------------------------------------------------*/

int att_valadd (ATT *att, const char *name, INST *inst)
{                               /* --- add a value to an attribute */
  int    i;                     /* buffer for value, loop variable */
  double f;                     /* buffer for value */
  char   *s;                    /* end pointer for conversion */
  VAL    *val;                  /* created symbolic value */
  VAL    **p;                   /* to traverse values */
  int    len;                   /* length of value name */
  int    w, sw;                 /* value name widths */
  unsigned int h;               /* hash value of value name */

  assert(att);                  /* check the function arguments */

  /* --- integer attribute --- */
  if (att->type == AT_INT) {    /* if attribute is integer valued */
    if (!name) {                /* if no value name given, */
      if (!inst) {              /* if no instance given */
        att->min.i = -INT_MAX;  /* otherwise set the */
        att->max.i =  INT_MAX;  /* maximal range of values */
        return 0;               /* and abort the function */
      }                         /* if an instance is given, */
      i = inst->i; len = 0; }   /* get the value from the instance */
    else {                      /* if a value name is given, */
      i = (int)strtol(name, &s, 10);  /* convert name to int */
      if ((s == name) || (*s != '\0'))
        return -2;              /* if the conversion failed, abort */
      len = (int)((const char*)s -name);
    }                           /* get the length of the name */
    if (i <= UV_INT) return -2; /* check for an unknown value */
    if (name && inst && ((i < att->min.i) || (i > att->max.i)))
      return -3;                /* check for a new value */
    if (i < att->min.i) att->min.i = i;  /* update minimal */
    if (i > att->max.i) att->max.i = i;  /* and maximal value */
    att->inst.i = i;                     /* and set instance */
    if (len > att->valwd[0]) att->valwd[0] = att->valwd[1] = len;
    return 0;                   /* adapt value widths */
  }                             /* and return 'ok' */

  /* --- real/float attribute --- */
  if (att->type == AT_FLT) {    /* if attribute is real valued */
    if (!name) {                /* if no value name given */
      if (!inst) {              /* if no instance given */
        att->min.f = -FLT_MAX;  /* otherwise set the */
        att->max.f =  FLT_MAX;  /* maximal range of values */
        return 0;               /* and abort the function */
      }                         /* if an instance is given, */
      f = inst->f; len = 0; }   /* get the value from the instance */
    else {                      /* if a value name is given, */
      f = strtod(name, &s);     /* convert name to float value */
      if ((s == name) || (*s != '\0') || (f > FLT_MAX))
        return -2;              /* if the conversion failed, abort */
      len = (int)((const char*)s -name);
    }                           /* get the length of the name */
    if (f <= UV_FLT) return -2; /* check for an unknown value */
    if (name && inst            /* check for a new value */
    && (((float)f < att->min.f)    /* (the conversion to float is */
    ||  ((float)f > att->max.f)))  /* necessary to avoid problems */
      return -3;                   /* of representational accuracy */ 
    if (f < att->min.f) att->min.f = (float)f;  /* update minimal */
    if (f > att->max.f) att->max.f = (float)f;  /* and maximal value */
    att->inst.f = (float)f;                     /* and set instance */
    if (len > att->valwd[0]) att->valwd[0] = att->valwd[1] = len;
    return 0;                   /* adapt value widths */
  }                             /* and return 'ok' */

  /* --- symbolic attribute --- */
  assert(name && *name);        /* check for a valid name */
  if (att_resize(att, att->valcnt +1) != 0)
    return -1;                  /* resize the value vector */
  h = _hash(name);              /* compute the name's hash value */
  p = att->htab +h % att->valvsz;
  for (val = *p; val; val = val->succ) {
    if (strncmp(name, val->name, AS_MAXLEN) == 0) {
      att->inst.i = val->id; return 1; }
  }                             /* if name already exists, abort */
  if (inst) return -3;          /* if not to extend the domain, abort */
  val = (VAL*)malloc(sizeof(VAL) +_length(name) *sizeof(char));
  if (!val) return -1;          /* allocate memory for a value */
  _copy(val->name, name);       /* copy name and set hash value */
  val->hval = h;                /* set value identifier and instance */
  val->id   = att->inst.i = att->valcnt;
  val->succ = *p; *p = val;     /* insert value into the hash table */
  att->vals[att->valcnt++] = val;           /* and the value vector */
  att->max.i++;                 /* adapt maximal value identifier */
  if (att->valwd[0] >= 0) {     /* if value name widths are valid */
    w = sc_fmtlen(val->name, &sw); /* determine value name widths */
    if (w  > att->valwd[0]) att->valwd[0] = w;
    if (sw > att->valwd[1]) att->valwd[1] = sw;
  }                             /* update maximal value widths */
  return 0;                     /* return 'ok' */
}  /* att_valadd() */

/*--------------------------------------------------------------------*/

void att_valrem (ATT *att, int valid)
{                               /* --- remove an attribute value */
  int i;                        /* loop variable */
  VAL **p;                      /* to traverse value vector */
  VAL **hb;                     /* to traverse hash bucket */

  assert(att                    /* check the function arguments */
      && (att->type == AT_SYM) && (valid < att->valcnt));

  /* --- remove all attribute values --- */
  if (valid < 0) {              /* if no value identifier given */
    if (!att->vals) return;     /* if there are no values, abort */
    for (p = att->vals +(i = att->valcnt); --i >= 0; )
      free(*--p);               /* delete attribute values */
    free(att->vals); free(att->htab);
    att->vals   = att->htab   = NULL;
    att->valcnt = att->valvsz = 0;
    att->max.i  = 0;            /* delete vector and hash table, */
    att->inst.i = UV_SYM;       /* clear value counter, maximal id, */
    return;                     /* and instance (current value) */
  }                             /* and abort the function */

  /* --- remove one attribute value --- */
  p  = att->vals +valid;        /* get value to remove */
  hb = att->htab +(*p)->hval % att->valvsz;
  while (*hb != *p) hb = &(*hb)->succ;
  *hb = (*hb)->succ;            /* remove value from hash table */
  free(*p);                     /* and delete it */
  for (i = --att->valcnt -valid; --i >= 0; ) {
    *p = p[1]; (*p++)->id--; }  /* shift values and adapt identifiers */
  att->max.i--;                 /* adapt maximal value identifier */
  att->valwd[0] = -1;           /* invalidate value widths */
  if      (att->inst.i >  valid) att->inst.i--;
  else if (att->inst.i == valid) att->inst.i = UV_SYM;
  att_resize(att, 0);           /* adapt instance (current value) */
}  /* att_valrem() */           /* and try to shrink the value vector */

/*--------------------------------------------------------------------*/

void att_valexg (ATT *att, int valid1, int valid2)
{                               /* --- exchange two attribute values */
  VAL **p1, **p2, *val;         /* temporary buffers */

  assert(att && (att->type == AT_SYM)
      && (valid1 >= 0) && (valid1 < att->valcnt)
      && (valid2 >= 0) && (valid2 < att->valcnt));
  p1 = att->vals +valid1; val = *p1;
  p2 = att->vals +valid2;       /* get pointers to values, */
  *p1 = *p2; (*p1)->id = valid1;/* exchange them, and */
  *p2 = val; val->id   = valid2;/* set new identifiers */
  if      (att->inst.i == valid1) att->inst.i = valid2;
  else if (att->inst.i == valid2) att->inst.i = valid1;
}  /* att_valexg() */           /* adapt instance (current value) */

/*--------------------------------------------------------------------*/

void att_valmove (ATT *att, int off, int cnt, int pos)
{                               /* --- move some attribute values */
  int n;                        /* temporary buffer */
  VAL **p;                      /* to traverse values */
  VAL *curr;                    /* current value (instance) */

  assert(att && (att->type == AT_SYM));  /* check function arguments */
  curr = (att->inst.i >= 0)     /* note instance (current value) */
       ? att->vals[att->inst.i] : NULL;
  n = att->valcnt;              /* check and adapt insert position */
  if (pos > n)      pos = n;    /* and number of values */
  if (cnt > n -off) cnt = n -off;
  assert((cnt >= 0) && (off >= 0)
      && (pos >= 0) && ((pos <= off) || (pos >= off +cnt)));
  v_move(att->vals, off, cnt, pos, (int)sizeof(VAL*));
  if (pos <= off) {             /* move values in vector */
    cnt += off; off = pos; pos = cnt; }
  p = att->vals +off;           /* set new value identifiers */
  while (off < pos) (*p++)->id = off++;
  if (curr) att->inst.i = curr->id;
}  /* att_valmove() */          /* adapt instance (current value) */

/*--------------------------------------------------------------------*/

int att_valcut (ATT *dst, ATT *src, int mode, ...)
{                               /* --- cut some attribute values */
  va_list args;                 /* list of variable arguments */
  int     i;                    /* loop variable, buffer */
  int     off, cnt;             /* range of values */
  VAL     **s;                  /* to traverse source values */
  VAL     **d, **p, *hb;        /* to traverse dest.  values */

  assert(src                    /* check the function arguments */
      && (!dst || (dst->type == src->type)));

  /* --- numeric attributes --- */
  if (src->type != AT_SYM) {    /* if attribute is numeric */
    if (dst) {                  /* if there is a destination */
      if (src->type == AT_INT){ /* if integer attribute */
        if (src->min.i < dst->min.i) dst->min.i = src->min.i;
        if (src->max.i > dst->max.i) dst->max.i = src->max.i; }
      else {                    /* if real/float attribute */
        if (src->min.f < dst->min.f) dst->min.f = src->min.f;
        if (src->max.f > dst->max.f) dst->max.f = src->max.f;
      }                         /* adapt range of values */
      if (src->valwd[0] > dst->valwd[0])
        dst->valwd[0] = dst->valwd[1] = src->valwd[0];
    }                           /* adapt value widths */
    if (src->type == AT_INT) {  /* if integer attribute */
      src->min.i  = INT_MAX; src->max.i = -INT_MAX;
      src->inst.i = UV_INT; }   /* clear range of values and instance */
    else {                      /* if real/float attribute */
      src->min.f  = FLT_MAX;  src->max.f = -FLT_MAX;
      src->inst.f = UV_FLT;     /* clear range of values and */
    }                           /* instance (current value) */
    src->valwd[0] = src->valwd[1] = 0;
    return 0;                   /* clear value widths */
  }                             /* and return 'ok' */

  /* --- symbolic attributes: get range of values --- */
  if (mode & AS_RANGE) {        /* if an index range is given */
    va_start(args, mode);       /* start variable argument evaluation */
    off = va_arg(args, int);    /* get offset to first value */
    cnt = va_arg(args, int);    /* and number of values */
    va_end(args);               /* end variable argument evaluation */
    i = src->valcnt -off;       /* check and adapt */
    if (cnt > i) cnt = i;       /* number of values */
    assert((off >= 0) && (cnt >= 0)); }
  else {                        /* if no index range given */
    off = 0; cnt = src->valcnt; /* get full index range */
  }
  if (cnt <= 0) return 0;       /* if range is empty, abort */
  if (dst && (att_resize(dst, dst->valcnt +cnt) != 0))
    return -1;                  /* resize vector and hash table */

  /* --- cut source values --- */
  d = (dst) ? dst->vals +dst->valcnt : NULL;
  s = src->vals +off;           /* get destination and source */
  for (i = cnt; --i >= 0; s++){ /* traverse the source values */
    p = src->htab +(*s)->hval % src->valvsz;
    while (*p != *s) p = &(*p)->succ;
    *p = (*p)->succ;            /* remove value from the hash table */
    if (dst) {                  /* if there is a destination */
      hb = dst->htab[(*s)->hval % dst->valvsz];
      for (; hb; hb = hb->succ) /* search value in destination */
        if (strcmp((*s)->name, hb->name) == 0) break;
      if (!hb) { *d++ = *s; continue; }
    }                           /* store value in destination or */
    free(*s);                   /* delete it (if there is no dest. */
  }                             /* or the value is already present) */
  p = src->vals +off;           /* traverse rear part of the vector */
  for (i = src->valcnt -off -cnt; --i >= 0; ) {
    (*s)->id -= cnt; *p++ = *s++;
  }                             /* shift left/down remaining values */
  src->valcnt  -= cnt;          /* adapt number of values */
  src->max.i   -= cnt;          /* and maximal identifier */
  src->valwd[0] = -1;           /* invalidate value widths */
  i = src->inst.i;              /* adapt instance (current value) */
  if (i >= off) src->inst.i = (i -cnt >= off) ? i -cnt : UV_SYM;
  att_resize(src, 0);           /* try to shrink the value vector */

  /* --- insert values into destination --- */
  if (dst) {                    /* if there is a destination */
    s = dst->vals +dst->valcnt; /* get pointer to first new value */
    dst->valcnt += i = (int)(d -s);   /* adapt number of values and */
    dst->max.i  += i;                 /* maximal value identifier */
    while (--i >= 0) {                /* traverse inserted values */
      (*s)->id = (int)(s -dst->vals); /* set value identifier */
      p = dst->htab +(*s)->hval % dst->valvsz;
      (*s)->succ = *p; *p = *s++;
    }                           /* insert value into the hash table */
    dst->valwd[0] = -1;         /* invalidate value widths */
    att_resize(dst, 0);         /* try to shrink the value vector */
  }
  return 0;                     /* return 'ok' */
}  /* att_valcut() */

/*--------------------------------------------------------------------*/

int att_valcopy (ATT *dst, const ATT *src, int mode, ...)
{                               /* --- copy some attribute values */
  va_list args;                 /* list of variable arguments */
  int     i;                    /* loop variable, buffer */
  int     off, cnt;             /* range of values */
  VAL     *const *s;            /* to traverse source values */
  VAL     **d, **p, *hb;        /* to traverse destination values */

  assert(src && dst             /* check the function arguments */
      && (dst->type == src->type));

  /* --- numeric attributes --- */
  if (src->type != AT_SYM) {    /* if attribute is numeric */
    if (src->type == AT_INT) {  /* if integer attribute */
      if (src->min.i < dst->min.i) dst->min.i = src->min.i;
      if (src->max.i > dst->max.i) dst->max.i = src->max.i; }
    else {                      /* if real/float attribute */
      if (src->min.f < dst->min.f) dst->min.f = src->min.f;
      if (src->max.f > dst->max.f) dst->max.f = src->max.f;
    }                           /* adapt range of values */
    if (src->valwd[0] > dst->valwd[0])
      dst->valwd[0] = dst->valwd[1] = src->valwd[0];
    return 0;                   /* adapt value widths */
  }                             /* and return 'ok' */

  /* --- symbolic attributes: get range of values --- */
  if (mode & AS_RANGE) {        /* if an index range is given */
    va_start(args, mode);       /* start variable argument evaluation */
    off = va_arg(args, int);    /* get offset to first value */
    cnt = va_arg(args, int);    /* and number of values */
    va_end(args);               /* end variable argument evaluation */
    i = src->valcnt -off;       /* check and adapt */
    if (cnt > i) cnt = i;       /* number of values */
    assert((off >= 0) && (cnt >= 0)); }
  else {                        /* if no index range given */
    off = 0; cnt = src->valcnt; /* get full index range */
  }
  if (cnt <= 0) return 0;       /* if range is empty, abort */
  if (att_resize(dst, dst->valcnt +cnt) != 0)
    return -1;                  /* resize vector and hash table */

  /* --- cut/copy source values --- */
  d = dst->vals +dst->valcnt;   /* get destination */
  s = src->vals +off;           /* and source pointers */
  for (i = cnt; --i >= 0; s++, d++) {
    hb = dst->htab[(*s)->hval % dst->valvsz];
    for ( ; hb; hb = hb->succ)  /* search value in destination */
      if (strcmp((*s)->name, hb->name) == 0) break;
    if (hb) continue;           /* if value already exists, skip it */
    *d = (VAL*)malloc(sizeof(VAL) +strlen((*s)->name) *sizeof(char));
    if (!*d) break;             /* allocate memory for a new value */
    strcpy((*d)->name, (*s)->name);
    (*d)->hval = (*s)->hval;    /* copy value name and hash value */
  }                             /* (the identifier is set later) */
  if (i >= 0) {                 /* if an error occured */
    for (i = (int)(d -(dst->vals +dst->valcnt)); --i > 0; )
      free(*--d);               /* delete all copied values */
    return -1;                  /* and abort the function */
  }

  /* --- insert values into destination --- */
  p = dst->vals +dst->valcnt;   /* get pointer to first new value */
  dst->valcnt += i = (int)(d-p);/* adapt number of values and */
  dst->max.i  += i;             /* maximal value identifier */
  while (--i >= 0) {            /* traverse inserted values */
    (*p)->id = (int)(p -dst->vals); /* set value identifier */
    d = dst->htab +(*p)->hval % dst->valvsz;
    (*p)->succ = *d; *d = *p++; /* insert value into the hash table */
  }
  dst->valwd[0] = -1;           /* invalidate value widths */
  att_resize(dst, 0);           /* try to shrink the value vector */
  return 0;                     /* return 'ok' */
}  /* att_valcopy() */

/*--------------------------------------------------------------------*/

void att_valsort (ATT *att, VAL_CMPFN cmpfn, int *map, int dir)
{                               /* --- sort attribute values */
  VAL *curr;                    /* buffer for current value */
  int i;                        /* loop variable */
  VAL **p;                      /* to traverse the value vector */

  assert(att && (att->type == AT_SYM));  /* check function arguments */
  curr = (att->inst.i >= 0)     /* note instance (current value) */
       ? att->vals[att->inst.i] : NULL;
  v_sort(att->vals, att->valcnt, _valcmp, (void*)cmpfn);
  if (map) {                    /* if an identifier map is requested, */
    p = att->vals +(i = att->valcnt);  /* traverse sorted vector */
    if (dir < 0)                /* if backward map (i.e. new -> old) */
      while (--i >= 0) map[i] = (*--p)->id;
    else                        /* if forward  map (i.e. old -> new) */
      while (--i >= 0) map[(*--p)->id] = i;
  }                             /* (build identifier map) */
  for (p = att->vals +(i = att->valcnt); --i >= 0; )
    (*--p)->id = i;             /* set new value identifiers */
  if (curr) att->inst.i = curr->id;
}  /* att_valsort() */          /* adapt instance (current value) */

/*--------------------------------------------------------------------*/

int att_valwd (ATT *att, int scform)
{                               /* --- determine widths of values */
  int   i;                      /* loop variable */
  VAL   **p;                    /* to traverse values */
  int   w, sw;                  /* value name widths */

  assert(att);                  /* check the function argument */
  if (att->valwd[0] < 0) {      /* if the value widths are invalid */
    att->valwd[0] = att->valwd[1] = 0;
    p = att->vals +att->valcnt; /* traverse attribute values */
    for (i = att->valcnt; --i >= 0; ) {
      w = sc_fmtlen((*--p)->name, &sw);
      if (w  > att->valwd[0]) att->valwd[0] = w;
      if (sw > att->valwd[1]) att->valwd[1] = sw;
    }                           /* determine maximal widths and */
  }                             /* return width of widest value */
  return att->valwd[(scform) ? 1 : 0];
}  /* att_valwd() */

/*--------------------------------------------------------------------*/

int att_valid (const ATT *att, const char *name)
{                               /* --- get the identifier of a value */
  VAL *val;                     /* to traverse hash bucket */

  assert(att                    /* check the function arguments */
      && name && (att->type == AT_SYM));
  if (att->valcnt <= 0) return UV_SYM;
  val = att->htab[_hash(name) % att->valvsz];
  for ( ; val; val = val->succ) /* traverse hash bucket list */
    if (strncmp(name, val->name, AS_MAXLEN) == 0)
      break;                    /* if value found, abort loop */
  return (val) ? val->id : UV_SYM;
}  /* att_valid() */            /* return value identifier */

/*----------------------------------------------------------------------
  Attribute Set Functions
----------------------------------------------------------------------*/

static int as_resize (ATTSET *set, int size)
{                               /* --- resize attribute vector */
  int i;                        /* loop variable */
  ATT **p;                      /* to traverse attributes */
  ATT **hb;                     /* to traverse hash bucket */

  assert(set);                  /* check the function argument */
  i = set->attvsz;              /* get current vector size */
  if (size > 0) {               /* if to enlarge the vector */
    if (i >= size) return 0;    /* if vector is large enough, abort */
    i += (i > BLKSIZE) ? i >> 1 : BLKSIZE;
    if (i >  size) size = i; }  /* compute new vector size */
  else {                        /* if to shrink the vector */
    size = set->attcnt << 1;    /* get maximal tolerable size */
    if (size < BLKSIZE) size = BLKSIZE;
    if (i <= size) return 0;    /* if vector is small enough, abort */
    size = set->attcnt +(set->attcnt >> 1);
    if (size < BLKSIZE) size = BLKSIZE;
  }                             /* compute new vector size */
  p  = (ATT**)realloc(set->atts, size *sizeof(ATT*));
  if (!p)  return -1;           /* resize attribute vector */
  set->atts = p;                /* and set new vector */
  hb = (ATT**)realloc(set->htab, size *sizeof(ATT*));
  if (!hb) return -1;           /* resize hash table */
  set->htab = hb;               /* and set new hash table */
  set->attvsz = size;           /* set new vector size */
  for (i = size; --i >= 0; )    /* clear hash table */
    hb[i] = NULL;               /* and traverse attributes */
  for (i = set->attcnt; --i >= 0; ) {
    hb = set->htab +(*p)->hval % size;
    (*p)->succ = *hb;           /* get pointer to hash bucket and */
    *hb = *p++;                 /* insert attribute into hash table */
  }                             /* (at the head of the bucket list) */
  return 0;                     /* return 'ok' */
}  /* as_resize() */

/*--------------------------------------------------------------------*/

ATTSET* as_create (const char *name, ATT_DELFN delfn)
{                               /* --- create an attribute set */
  ATTSET *set;                  /* created attribute set */

  assert(name && *name && delfn);  /* check the function arguments */
  set = (ATTSET*)malloc(sizeof(ATTSET));
  if (!set) return NULL;        /* allocate memory for an att. set */
  set->name = (char*)malloc((_length(name) +1) *sizeof(char));
  if (!set->name) { free(set); return NULL; }
  _copy(set->name, name);       /* copy attribute set name */
  set->atts     = set->htab   = NULL;
  set->attcnt   = set->attvsz = 0;
  set->delfn    = delfn;        /* initialize fields */
  set->weight   = 1.0F;
  #if defined AS_FLDS || defined AS_RDWR
  set->fldvsz   = set->fldcnt = 0;
  set->flds     = NULL;         /* clear field map */
  #endif
  #ifdef AS_RDWR                /* if to compile read/write functions */
  set->chars[0] = ' ';          /* blank  character */
  set->chars[1] = ' ';          /* field  separator */
  set->chars[2] = '\n';         /* record separator */
  set->chars[3] = '?';          /* unknown value character */
  set->tfscan   = tfs_create(); /* create table file scanner */
  if (!set->tfscan) { as_delete(set); return NULL; }
  set->err = tfs_err(set->tfscan);
  set->err->s = set->buf;       /* initialize the error information */
  tfs_chars(set->tfscan, TFS_OTHER, "?");
  #endif                        /* set '?' as unknown value character */
  return set;                   /* return created attribute set */
}  /* as_create() */

/*--------------------------------------------------------------------*/

ATTSET* as_dup (const ATTSET *set)
{                               /* --- duplicate an attribute set */
  ATTSET *dup;                  /* created duplicate */
  #if defined AS_FLDS || defined AS_RDWR
  int    i;                     /* loop variable */
  int    *d; const int *s;      /* to traverse the field vector */
  #endif

  assert(set);                  /* check the function argument */
  dup = as_create(set->name, set->delfn);
  if (!dup) return NULL;        /* create a new attribute set */
  if (as_attcopy(dup, set, 0) != 0) { as_delete(dup); return NULL; }
  dup->weight = set->weight;    /* copy all attributes and */
  dup->info   = set->info;      /* all other information */
  #if defined AS_FLDS || defined AS_RDWR
  if (set->flds) {              /* if there is a field vector */
    dup->flds = (int*)malloc(set->fldvsz *sizeof(int));
    if (!dup->flds) { as_delete(dup); return NULL; }
    s = set->flds +set->fldcnt; /* create a field vector and */
    d = dup->flds +set->fldcnt; /* copy the source field vector */
    for (i = set->fldcnt; --i >= 0; ) *--d = *--s;
  }
  #endif
  #ifdef AS_RDWR                /* if to compile read/write functions */
  for (i = 4; --i >= 0; ) dup->chars[i] = set->chars[i];
  tfs_copy(dup->tfscan, set->tfscan);
  #endif                        /* copy chars. and table file scanner */
  return dup;                   /* return created duplicate */
}  /* as_dup() */

/*--------------------------------------------------------------------*/

void as_delete (ATTSET *set)
{                               /* --- delete an attribute set */
  int i;                        /* loop variable */
  ATT **p;                      /* to traverse the attribute vector */

  assert(set);                  /* check the function argument */
  if (set->atts) {              /* if there are attributes, */
    for (p = set->atts +(i = set->attcnt); --i >= 0; ) {
      (*--p)->set = NULL; (*p)->id = -1; set->delfn(*p); }
    free(set->atts);            /* delete attributes, vector, */
  }                             /* and hash table */
  if (set->htab)   free(set->htab);
  #if defined AS_FLDS || defined AS_RDWR
  if (set->flds)   free(set->flds);
  #endif                        /* delete field map */
  #ifdef AS_RDWR                /* if to compile read/write functions */
  if (set->tfscan) tfs_delete(set->tfscan);
  #endif                        /* delete table file scanner */
  free(set->name);              /* delete attribute set name */
  free(set);                    /* and attribute set body */
}  /* as_delete() */

/*--------------------------------------------------------------------*/

int as_rename (ATTSET *set, const char *name)
{                               /* --- rename an attribute set */
  char *t;                      /* temporary buffer for name */

  assert(set && name && *name); /* check the function arguments */
  t = (char*)realloc(set->name, (_length(name) +1) *sizeof(char));
  if (!t) return -1;            /* reallocate memory block */
  set->name = _copy(t, name);   /* and copy the new name */
  return 0;                     /* return 'ok' */
}  /* as_rename() */

/*--------------------------------------------------------------------*/

int as_cmp (const ATTSET *set1, const ATTSET *set2)
{                               /* --- compare two attribute sets */
  int i;                        /* loop variable, attribute index */
  ATT **p1, **p2;               /* to traverse attributes */

  assert(set1 && set2);         /* check the function arguments */
  if (set1->attcnt != set2->attcnt)
    return 1;                   /* compare number of attributes */
  p1 = set1->atts +set1->attcnt;/* traverse all attributes */
  p2 = set2->atts +set1->attcnt;/* and compare them */
  for (i = set1->attcnt; --i >= 0; )
    if (att_cmp(*--p1, *--p2) != 0)
      return 1;                 /* if an attribute differs, abort */
  return 0;                     /* otherwise return 'equal' */
}  /* as_cmp() */

/*--------------------------------------------------------------------*/

int as_attadd (ATTSET *set, ATT *att)
{                               /* --- add one attribute */
  ATT **p, *hb;                 /* to traverse the hash bucket */

  assert(set && att);           /* check the function arguments */
  if (as_resize(set, set->attcnt +1) != 0)
    return -1;                  /* resize the attribute vector */
  p = set->htab +att->hval % set->attvsz;
  for (hb = *p; hb; hb = hb->succ) /* traverse hash bucket list */
    if (strcmp(att->name, hb->name) == 0)
      return 1;                 /* if name already exists, abort */
  if (att->set)                 /* remove attribute from old set */
    as_attrem(att->set, att->id);
  att->set  = set;              /* set containing attribute set */
  att->id   = set->attcnt;      /* and attribute identifier */
  att->succ = *p; *p = att;     /* insert attribute into hash table */
  set->atts[set->attcnt++] = att;           /* and attribute vector */
  return 0;                     /* return 'ok' */
}  /* as_attadd() */

/*--------------------------------------------------------------------*/

int as_attaddm (ATTSET *set, ATT **atts, int cnt)
{                               /* --- add several attributes */
  int i;                        /* loop variable */
  ATT **p, *hb;                 /* to traverse the hash bucket */
  ATT *att;                     /* buffer for attribute */

  assert(set && atts && (cnt >= 0));  /* check function arguments */
  if (as_resize(set, set->attcnt +cnt) != 0)
    return -1;                  /* resize the attribute vector */
  for (i = cnt; --i >= 0; ) {   /* traverse new attributes */
    att = atts[i];              /* get next attribute */
    hb  = (ATT*)set->htab[att->hval % set->attvsz];
    for ( ; hb; hb = hb->succ)  /* traverse hash bucket list */
      if (strcmp(att->name, hb->name) == 0)
        return -2;              /* if name already exists, */
  }                             /* abort the function */
  for (i = cnt; --i >= 0; ) {   /* traverse new attributes again */
    att = *atts++;              /* get next attribute and */
    if (att->set)               /* remove it from old set */
      as_attrem(att->set, att->id);
    att->set = set;             /* set containing attribute set */
    att->id  = set->attcnt;     /* and attribute identifier */
    set->atts[set->attcnt++] = att;
    p = set->htab +att->hval % set->attvsz;
    att->succ = *p; *p = att;   /* insert attribute into */
  }                             /* attribute vector and hash table */
  return 0;                     /* return 'ok' */
}  /* as_attaddm() */

/*--------------------------------------------------------------------*/

ATT* as_attrem (ATTSET *set, int attid)
{                               /* --- remove an attribute */
  ATT **p;                      /* to traverse attribute vector */
  ATT **hb;                     /* to traverse hash bucket */
  ATT *att;                     /* buffer for removed attribute */

  assert(set && (attid < set->attcnt));  /* check function arguments */

  /* --- remove all attributes --- */
  if (attid < 0) {              /* if no attribute identifier given */
    if (!set->atts) return NULL;/* if there are no attributes, abort */
    for (p = set->atts +(attid = set->attcnt); --attid >= 0; ) {
      (*--p)->set = NULL; (*p)->id = -1;
      set->delfn(*p);           /* delete all attributes */
    }
    free(set->atts); free(set->htab);
    set->atts   = set->htab   = NULL;
    set->attcnt = set->attvsz = 0;
    return NULL;                /* delete att. vector and hash table, */
  }                             /* clear size and counter and abort */

  /* --- remove one attribute --- */
  p = set->atts +attid; att = *p;  /* get the attribute to remove */
  hb  = set->htab +att->hval % set->attvsz;
  while (*hb != att) hb = &(*hb)->succ;
  *hb = att->succ;              /* remove att. from the hash table */
  att->set = NULL;              /* clear reference to containing set */
  att->id  = -1;                /* and attribute identifier */
  for (attid = --set->attcnt -attid; --attid >= 0; ) {
    *p = p[1]; (*p++)->id--; }  /* shift atts. and adapt identifiers */
  as_resize(set, 0);            /* try to shrink attribute vector */
  _delflds(set);                /* and delete the field map */
  return att;                   /* return removed attribute */
}  /* as_attrem() */

/*--------------------------------------------------------------------*/

void as_attexg (ATTSET *set, int attid1, int attid2)
{                               /* --- exchange two attributes */
  ATT **p1, **p2, *att;         /* temporary buffers */

  assert(set                    /* check the function arguments */
      && (attid1 >= 0) && (attid1 < set->attcnt)
      && (attid2 >= 0) && (attid2 < set->attcnt));
  p1 = set->atts +attid1; att = *p1;
  p2 = set->atts +attid2;       /* get pointers to attributes, */
  *p1 = *p2; (*p1)->id = attid1;/* exchange them, and */
  *p2 = att; att->id   = attid2;/* set new identifiers */
  _delflds(set);                /* delete field map */
}  /* as_attexg() */

/*--------------------------------------------------------------------*/

void as_attmove (ATTSET *set, int off, int cnt, int pos)
{                               /* --- move some attributes */
  int n;                        /* temporary buffer */
  ATT **p;                      /* to traverse attributes */

  assert(set);                  /* check the function argument */
  n = set->attcnt;              /* check and adapt insert position */
  if (pos > n)      pos = n;    /* and number of attributes */
  if (cnt > n -off) cnt = n -off;
  assert((cnt >= 0) && (off  >= 0)
      && (pos >= 0) && ((pos <= off) || (pos >= off +cnt)));
  v_move(set->atts, off, cnt, pos, (int)sizeof(ATT*));
  if (pos <= off) {             /* move attributes in vector */
    cnt += off; off = pos; pos = cnt; }
  p = set->atts +off;           /* set new attribute identifiers */
  while (off < pos) (*p++)->id = off++;
  _delflds(set);                /* delete field map */
}  /* as_attmove() */

/*--------------------------------------------------------------------*/

int as_attcut (ATTSET *dst, ATTSET *src, int mode, ...)
{                               /* --- cut/copy attributes */
  va_list   args;               /* list of variable arguments */
  int       i;                  /* loop variable, buffer */
  int       off, cnt;           /* range of attributes */
  ATT       **s, **r;           /* to traverse source attributes */
  ATT       **d, **p;           /* to traverse dest.  attributes */
  ATT       *hb;                /* to traverse hash bucket list */
  ATT_SELFN *selfn = 0;         /* attribute selection function */
  void      *data  = NULL;      /* attribute selection data */

  assert(src);                  /* check the function arguments */

  /* --- get range of attributes --- */
  va_start(args, mode);         /* start variable argument evaluation */
  if (mode & AS_RANGE) {        /* if an index range is given */
    off = va_arg(args, int);    /* get offset to first attribute */
    cnt = va_arg(args, int);    /* and number of attributes */
    i = src->attcnt -off;       /* check and adapt */
    if (cnt > i) cnt = i;       /* number of attributes */
    assert((off >= 0) && (cnt >= 0)); }
  else {                        /* if no index range given */
    off = 0; cnt = src->attcnt; /* get full index range */
  }
  if (mode & AS_SELECT) {       /* if to select atts. by a function */
    selfn = va_arg(args, ATT_SELFN*);
    data  = va_arg(args, void*);/* get attribute selection function */
  }                             /* and attribute selection data */
  va_end(args);                 /* end variable argument evaluation */
  if (cnt <= 0) return 0;       /* if range is empty, abort */
  if (dst && (as_resize(dst, dst->attcnt +cnt) != 0))
    return -1;                  /* resize vector and hash table */

  /* --- cut source attributes --- */
  d = (dst) ? dst->atts +dst->attcnt : NULL;
  s = r = src->atts +off;       /* get destination and source */
  for (i = cnt; --i >= 0; s++){ /* traverse the values in range */
    if (((mode & AS_MARKED)     /* if in marked mode */
    &&   ((*s)->mark < 0))      /* and attribute is not marked */
    ||  ((mode & AS_SELECT)     /* or in selection mode */
    &&   (!selfn(*s, data)))) { /* and attribute does not qualify, */
      (*s)->id = (int)(r -src->atts);   /* set new attribute id. */
      *r++ = *s; continue;      /* and shift down/left attribute */
    }                           /* otherwise (if to cut attribute) */
    p = src->htab +(*s)->hval % src->attvsz;
    while (*p != *s) p = &(*p)->succ;
    *p = (*p)->succ;            /* remove att. from the hash table */
    if (dst) {                  /* if there is a destination */
      hb = dst->htab[(*s)->hval % dst->attvsz];
      for (; hb; hb = hb->succ) /* search attribute in destination */
        if (strcmp((*s)->name, hb->name) == 0) break;
      if (!hb) { *d++ = *s; continue; }
    }                           /* if attribute does not exist, */
    (*s)->set = NULL; (*s)->id = -1;               /* store it, */
    src->delfn(*s);             /* otherwise delete the attribute */
  }
  for (i = src->attcnt -off -cnt; --i >= 0; ) {
    (*s)->id = (int)(r -src->atts); *r++ = *s++;
  }                             /* shift down/left rem. attributes */
  src->attcnt = (int)(r -src->atts);   /* set new number of values */
  as_resize(src, 0);            /* try to shrink the attribute vector */
  _delflds(src);                /* and delete the field map */

  /* --- insert attributes into destination --- */
  if (dst) {                    /* if there is a destination */
    s = dst->atts +dst->attcnt;     /* get first new attribute and */
    dst->attcnt += i = (int)(d -s); /* adapt number of attributes */
    while (--i >= 0) {          /* traverse inserted attributes */
      p = dst->htab +(*s)->hval % dst->attvsz;
      (*s)->succ = *p; *p = *s; /* insert attribute into hash table */
      (*s)->id   = (int)(s -dst->atts); /* set attribute identifier */
      (*s)->set  = dst; s++;    /* and attribute set reference */
    }
    as_resize(dst, 0);          /* try to shrink the attribute vector */
  }
  return 0;                     /* return 'ok' */
}  /* as_attcut() */

/*--------------------------------------------------------------------*/

int as_attcopy (ATTSET *dst, const ATTSET *src, int mode, ...)
{                               /* --- cut/copy attributes */
  va_list   args;               /* list of variable arguments */
  int       i;                  /* loop variable, buffer */
  int       off, cnt;           /* range of attributes */
  ATT       *const*s;           /* to traverse source attributes */
  ATT       **d, **p;           /* to traverse dest.  attributes */
  ATT       *hb;                /* to traverse hash bucket list */
  ATT_SELFN *selfn = 0;         /* attribute selection function */
  void      *data  = NULL;      /* attribute selection data */

  assert(src && dst);           /* check the function arguments */

  /* --- get range of attributes --- */
  va_start(args, mode);         /* start variable argument evaluation */
  if (mode & AS_RANGE) {        /* if an index range is given */
    off = va_arg(args, int);    /* get offset to first attribute */
    cnt = va_arg(args, int);    /* and number of attributes */
    i = src->attcnt -off;       /* check and adapt */
    if (cnt > i) cnt = i;       /* number of attributes */
    assert((off >= 0) && (cnt >= 0)); }
  else {                        /* if no index range given */
    off = 0; cnt = src->attcnt; /* get full index range */
  }
  if (mode & AS_SELECT) {       /* if to select atts. by a function */
    selfn = va_arg(args, ATT_SELFN*);
    data  = va_arg(args, void*);/* get attribute selection function */
  }                             /* and attribute selection data */
  va_end(args);                 /* end variable argument evaluation */
  if (cnt <= 0) return 0;       /* if range is empty, abort */
  if (as_resize(dst, dst->attcnt +cnt) != 0)
    return -1;                  /* resize vector and hash table */

  /* --- copy source attributes --- */
  d = dst->atts +dst->attcnt;   /* get destination and */
  s = src->atts +off;           /* source attributes */
  for (i = cnt; --i >= 0; s++){ /* traverse the values in range */
    if (((mode & AS_MARKED)     /* if in marked mode */
    &&   ((*s)->mark < 0))      /* and attribute is not marked */
    ||  ((mode & AS_SELECT)     /* or in selection mode */
    &&   (!selfn(*s, data))))   /* and attribute does not qualify, */
      continue;                 /* skip this attribute */
    hb = dst->htab[(*s)->hval % dst->attvsz];
    for ( ; hb; hb = hb->succ)  /* search attribute in destination */
      if (strcmp((*s)->name, hb->name) == 0) break;
    if (hb) continue;           /* if attribute exists, skip it */
    *d = att_dup(*s);           /* otherwise duplicate */
    if (!*d) break; d++;        /* the source attribute */
  }
  if (i >= 0) {                 /* if an error occured */
    for (i = (int)(d -(dst->atts +dst->attcnt)); --i > 0; )
      att_delete(*--d);         /* delete all copied attributes */
    return -1;                  /* and abort the function */
  }

  /* --- insert attributes into destination --- */
  p = dst->atts +dst->attcnt;   /* get first new attribute and */
  dst->attcnt += i = (int)(d-p);/* adapt number of attributes */
  while (--i >= 0) {            /* traverse inserted attributes */
    d = dst->htab +(*p)->hval % dst->attvsz;
    (*p)->succ = *d; *d = *p;   /* insert attribute into hash table */
    (*p)->id   = (int)(p -dst->atts);   /* set attribute identifier */
    (*p)->set  = dst; p++;           /* and attribute set reference */
  }
  as_resize(dst, 0);            /* try to shrink the attribute vector */
  return 0;                     /* return 'ok' */
}  /* as_attcopy() */

/*--------------------------------------------------------------------*/

int as_attid (const ATTSET *set, const char *name)
{                               /* --- get the id of an attribute */
  ATT *att;                     /* to traverse hash bucket list */

  assert(set && name);          /* check the function arguments */
  if (set->attcnt <= 0) return -1;
  att = set->htab[_hash(name) % set->attvsz];
  for ( ; att; att = att->succ) /* traverse hash bucket list */
    if (strncmp(name, att->name, AS_MAXLEN) == 0)
      break;                    /* if attribute found, abort loop */
  return (att) ? att->id : -1;  /* return attribute identifier */
}  /* as_attid() */

/*--------------------------------------------------------------------*/

void as_apply (ATTSET *set, ATT_APPFN appfn, void *data)
{                               /* --- apply a function to all atts. */
  int i;                        /* loop variable */
  ATT  **p;                     /* to traverse attribute vector */

  assert(set && appfn);         /* check the function arguments */
  for (p = set->atts +(i = set->attcnt); --i >= 0; )
    appfn(*--p, data);          /* apply function to all attributes */
}  /* as_apply() */
