/*----------------------------------------------------------------------
  File    : fbayes.c
  Contents: Full Bayes classifier management
  Author  : Christian Borgelt
  History : 26.11.2000 file created
            29.11.2000 first version completed
            15.07.2001 parser improved (global variables removed)
            16.07.2001 adapted to modified module scan
            17.07.2001 parser improved (conditional look ahead)
            15.09.2001 '*' instead of list of attributes made possible
            26.04.2003 function fbc_rand added
----------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#ifndef FBC_PARSE
#include "scan.h"
#endif
#include "fbayes.h"
#ifdef STORAGE
#include "storage.h"
#endif

/*----------------------------------------------------------------------
  Preprocessor Definitions
----------------------------------------------------------------------*/
#define EPSILON     1e-12       /* to handle roundoff errors */
#define BLKSIZE     16          /* block size for vectors */

#ifdef FBC_PARSE

/* --- error codes --- */
#define E_CHREXP    (-16)       /* character expected */
#define E_STREXP    (-17)       /* string expected */
#define E_PAREXP    (-18)       /* parameter expected */
#define E_ATTEXP    (-19)       /* attribute expected */
#define E_CLSEXP    (-20)       /* class value expected */
#define E_NUMEXP    (-21)       /* number expected */
#define E_ILLNUM    (-22)       /* illegal number */
#define E_UNKATT    (-23)       /* unknown attribute */
#define E_DUPATT    (-24)       /* duplicate attribute */
#define E_MISATT    (-25)       /* missing attribute */
#define E_UNKCLS    (-26)       /* unknown class value */
#define E_DUPCLS    (-27)       /* duplicate class value */
#define E_MISCLS    (-28)       /* missing class value */
#define E_ATTYPE    (-29)       /* attribute must be numeric */
#define E_CLSTYPE   (-30)       /* class attribute must be symbolic */
#define E_CLSCNT    (-31)       /* class attribute has too few values */

/* --- functions for parser --- */
#define ERROR(c)    return _paerr(scan, c,        -1, NULL)
#define XERROR(c,s) return _paerr(scan, c,        -1, s)
#define ERR_CHR(c)  return _paerr(scan, E_CHREXP,  c, NULL)
#define ERR_STR(s)  return _paerr(scan, E_STREXP, -1, s)
#define GET_TOK()   if (sc_next(scan) < 0) \
                      return sc_error(scan, sc_token(scan))
#define GET_CHR(c)  if (sc_token(scan) != (c)) ERR_CHR(c); \
                    else GET_TOK();

#endif  /* #ifdef FBC_PARSE */
/*----------------------------------------------------------------------
  Constants
----------------------------------------------------------------------*/
#ifdef FBC_PARSE
#ifdef GERMAN                   /* deutsche Texte */
static const char *errmsgs[] = {/* Fehlermeldungen */
  /* E_CHREXP  -16 */  "\"%c\" erwartet statt %s",
  /* E_STREXP  -17 */  "\"%s\" erwartet statt %s",
  /* E_PAREXP  -18 */  "Parameter erwartet statt %s",
  /* E_ATTEXP  -19 */  "Attribut erwartet statt %s",
  /* E_CLSEXP  -20 */  "Klassenattribut erwartet statt %s",
  /* E_NUMEXP  -21 */  "Zahl erwartet statt %s",
  /* E_ILLNUM  -22 */  "ungültige Zahl %s",
  /* E_UNKATT  -23 */  "unbekanntes Attribut %s",
  /* E_DUPATT  -24 */  "doppeltes Attribut %s",
  /* E_MISATT  -25 */  "fehlendes Attribut %s",
  /* E_UNKCLS  -26 */  "unbekannte Klasse %s",
  /* E_DUPCLS  -27 */  "doppelte Klasse %s",
  /* E_MISCLS  -28 */  "fehlende Klasse %s",
  /* E_ATTYPE  -29 */  "Attribut %s ist nicht numerisch",
  /* E_CLSTYPE -30 */  "Klassenattribut %s ist nicht symbolisch",
  /* E_CLSCNT  -31 */  "Klassenattribut %s hat zu wenige Werte",
};
#else                           /* English texts */
static const char *errmsgs[] = {/* error messages */
  /* E_CHREXP  -16 */  "\"%c\" expected instead of %s",
  /* E_STREXP  -17 */  "\"%s\" expected instead of %s",
  /* E_PAREXP  -18 */  "parameter expected instead of %s",
  /* E_ATTEXP  -19 */  "attribute expected instead of %s",
  /* E_CLSEXP  -20 */  "class attribute expected instead of %s",
  /* E_NUMEXP  -21 */  "number expected instead of %s",
  /* E_ILLNUM  -22 */  "illegal number %s",
  /* E_UNKATT  -23 */  "unknown attribute %s",
  /* E_DUPATT  -24 */  "duplicate attribute %s",
  /* E_MISATT  -25 */  "missing attribute %s",
  /* E_UNKCLS  -26 */  "unknown class value %s",
  /* E_DUPCLS  -27 */  "duplicate class value %s",
  /* E_MISCLS  -28 */  "missing class value %s",
  /* E_ATTYPE  -29 */  "attribute %s must be numeric",
  /* E_CLSTYPE -30 */  "class attribute %s must be symbolic",
  /* E_CLSCNT  -31 */  "class attribute %s has too few values",
};
#endif
#define MSGCNT  (int)(sizeof(errmsgs)/sizeof(const char*))

#endif  /* #ifdef FBC_PARSE */
/*----------------------------------------------------------------------
  Auxiliary Functions
----------------------------------------------------------------------*/
#ifdef FBC_INDUCE

static int _clsrsz (FBC *fbc, int clscnt)
{                               /* --- resize class dependent vectors */
  int    i;                     /* loop variable */
  int    clsvsz;                /* size of the class dep. vectors */
  double *frq;                  /* to traverse the frequency vectors */
  MVNORM **mvn;                 /* to traverse the distributions */

  assert(fbc && (clscnt >= 0)); /* check the function arguments */

  /* --- resize the class dependent vectors --- */
  clsvsz = fbc->clsvsz;         /* get the class dep. vector size */
  if (clscnt >= clsvsz) {       /* if the vectors are too small */
    clsvsz += (clsvsz > BLKSIZE) ? clsvsz >> 1 : BLKSIZE;
    if (clscnt >= clsvsz) clsvsz = clscnt;
    frq = (double*)realloc(fbc->frqs, clsvsz *3 *sizeof(double));
    if (!frq) return -1;        /* resize the frequencies vector */
    fbc->frqs   = frq;          /* and set the new vector */
    fbc->priors = fbc->frqs   +clsvsz;  /* organize the rest of the */
    fbc->posts  = fbc->priors +clsvsz;  /* allocated memory block */
    for (frq += clsvsz, i = clsvsz -fbc->clsvsz; --i >= 0; )
      *--frq = 0;               /* clear the new vector fields */
    mvn = (MVNORM**)realloc(fbc->mvns, clsvsz *sizeof(MVNORM*));
    if (!mvn) return -1;        /* resize the distribution vector */
    fbc->mvns   = mvn;          /* and set the new vector */
    fbc->clsvsz = clsvsz;       /* set the new size of the */
  }                             /* class dependent vectors */

  /* --- create new conditional distributions --- */
  mvn = fbc->mvns +clscnt;      /* traverse the normal distributions */
  for (i = clscnt -fbc->clscnt; --i >= 0; ) {
    *--mvn = mvn_create(fbc->numcnt);
    if (!*mvn) break;           /* create new normal distributions */
  }
  if (i >= 0) {                 /* if an error occurred */
    for (i = fbc->clscnt -i; --i >= 0; mvn++)
      mvn_delete(*mvn);         /* delete the newly created */
    return -1;                  /* multivariate normal distributions */
  }                             /* and abort the function */

  fbc->clscnt = clscnt;         /* set the new number of classes */
  return 0;                     /* return 'ok' */
}  /* _clsrsz() */

#endif
/*--------------------------------------------------------------------*/

static void _getvals (FBC *fbc, const TUPLE *tpl)
{                               /* --- get the attribute values */
  int    i;                     /* loop variable */
  const  INST *inst;            /* to traverse the instances */
  FBCID  *p;                    /* to traverse the attribute ids. */
  double *v;                    /* to traverse the att. value vector */

  assert(fbc);                  /* check the function argument */
  v = fbc->vals +fbc->numcnt;   /* get the attribute value vector */
  for (p = fbc->numids +(i = fbc->numcnt); --i >= 0; ) {
    --p;                        /* traverse the numeric attributes */
    inst = (tpl) ? tpl_colval(tpl, p->id) : att_inst(p->att);
    if (p->type == AT_FLT)      /* if the attribute is real-valued */
      *--v = (inst->f <= UV_FLT) ? MVN_UNKNOWN : (double)inst->f;
    else                        /* if the attribute is integer-valued */
      *--v = (inst->i <= UV_INT) ? MVN_UNKNOWN : (double)inst->i;
  }                             /* (collect attribute values) */
}  /* _getvals() */

/*----------------------------------------------------------------------
  Main Functions
----------------------------------------------------------------------*/

FBC* fbc_create (ATTSET *attset, int clsid)
{                               /* --- create a full Bayes classifier */
  int    i, t;                  /* loop variable, attribute type */
  FBC    *fbc;                  /* created classifier */
  FBCID  *p;                    /* to traverse the attribute ids. */
  ATT    *att;                  /* to traverse the attributes */
  double *frq;                  /* to traverse the frequency vector */
  MVNORM **mvn;                 /* to traverse the distributions */

  assert(attset && (clsid >= 0) /* check the function arguments */
      && (clsid < as_attcnt(attset))
      && (att_type(as_att(attset, clsid)) == AT_SYM));

  /* --- create the classifier body --- */
  i   = as_attcnt(attset);      /* get the number of attributes */
  fbc = (FBC*)malloc(sizeof(FBC) +(i-1) *sizeof(int));
  if (!fbc) return NULL;        /* allocate the classifier body */
  fbc->attset = attset;         /* and initialize the fields */
  fbc->attcnt = i;
  fbc->numcnt = 0;
  fbc->numids = NULL;
  fbc->clsid  = clsid;
  fbc->clsvsz = att_valcnt(as_att(attset, clsid));
  fbc->clscnt = fbc->clsvsz;
  fbc->total  = 0;
  fbc->lcorr  = 0;
  fbc->mode   = 0;
  fbc->frqs   = fbc->priors = fbc->posts = NULL;
  fbc->vals   = NULL;           /* clear pointers for a */
  fbc->mvns   = NULL;           /* proper cleanup on error */

  /* --- create the attribute information --- */
  fbc->numids = p = (FBCID*)malloc(fbc->attcnt *sizeof(FBCID));
  if (!p) { fbc_delete(fbc, 0); return NULL; }
  for (i = 0; i < fbc->attcnt; i++) {
    att = as_att(attset, i); t = att_type(att);
    if ((t != AT_INT) && (t != AT_FLT)) continue;
    p->id  = i; p->type = t;    /* count the numeric attributes */ 
    p->att = att; p++;          /* and note their identifications */
  }                             /* and types */
  fbc->numcnt = (int)(p -fbc->numids);
  fbc->vals = (double*)malloc(fbc->attcnt *sizeof(double));
  if (!fbc->vals) { fbc_delete(fbc, 0); return NULL; }
                                /* create an attribute value buffer */

  /* --- initialize the distributions --- */
  if (fbc->clscnt > 0) {        /* if there are classes, */
    fbc->frqs =                 /* allocate class vectors */
    frq = (double*)malloc(fbc->clsvsz *3 *sizeof(double));
    if (!frq) { fbc_delete(fbc, 0); return NULL; }
    fbc->priors = fbc->frqs   +fbc->clsvsz;
    fbc->posts  = fbc->priors +fbc->clsvsz;
    for (frq += i = fbc->clsvsz; --i >= 0; )
      *--frq = 0;               /* init. the class frequencies */
    fbc->mvns = mvn = (MVNORM**)calloc(fbc->clscnt, sizeof(MVNORM*));
    if (!mvn) { fbc_delete(fbc, 0); return NULL; }
    for (mvn += i = fbc->clscnt; --i >= 0; ) {
      *--mvn = mvn_create(fbc->numcnt);
      if (!*mvn) { fbc_delete(fbc, 0); return NULL; }
    }                           /* allocate a distribution vector and */
  }                             /* create multivariate normal dists. */

  return fbc;                   /* return the created classifier */
}  /* fbc_create() */

/*--------------------------------------------------------------------*/

FBC* fbc_dup (const FBC *fbc, int dupas)
{                               /* --- duplicate a full Bayes class. */
  int    i;                     /* loop variable */
  FBC    *dup;                  /* created classifier duplicate */
  ATTSET *attset;               /* duplicate of attribute set */
  double *df; const double *sf; /* to traverse the frequency vectors */
  FBCID  *di; const FBCID  *si; /* to traverse the attribute ids. */
  MVNORM **mvn;                 /* to traverse the distributions */

  assert(fbc);                  /* check the function argument */

  /* --- copy the classifier body --- */
  attset = fbc->attset;         /* get the attribute set */
  if (dupas) {                  /* if the corresp. flag is set, */
    attset = as_dup(attset);    /* duplicate the attribute set */
    if (!attset) return NULL;   /* of the original classifier, */
  }                             /* and then create a classifier */
  dup = (FBC*)malloc(sizeof(FBC) +(fbc->attcnt -1) *sizeof(int));
  if (!dup) { if (dupas) as_delete(attset); return NULL; }
  dup->attset = attset;         /* allocate a classifier body */
  dup->attcnt = fbc->attcnt;    /* and copy the fields */
  dup->numcnt = fbc->numcnt;
  dup->numids = NULL;
  dup->clsid  = fbc->clsid;
  dup->clsvsz = fbc->clscnt;
  dup->clscnt = fbc->clscnt;
  dup->total  = fbc->total;
  dup->lcorr  = fbc->lcorr;
  dup->mode   = fbc->mode;
  dup->frqs   = dup->priors = dup->posts = NULL;
  dup->vals   = NULL;           /* clear pointers for a */
  dup->mvns   = NULL;           /* proper cleanup on error */

  /* --- copy the attribute information --- */
  dup->numids = di = (FBCID*)malloc(dup->attcnt *sizeof(FBCID));
  if (!di) { fbc_delete(dup, dupas); return NULL; }
  si = fbc->numids +dup->numcnt;
  for (di += i = dup->numcnt; --i >= 0; )
    *--di = *--si;              /* copy the attribute identifications */
  dup->vals = (double*)malloc(dup->attcnt *sizeof(double));
  if (!dup->vals) { fbc_delete(dup, dupas); return NULL; }
                                /* create an attribute value buffer */

  /* --- copy the distributions --- */
  if (dup->clscnt > 0) {        /* if there are classes, */
    dup->frqs =                 /* allocate class vectors */
    df = (double*)malloc(dup->clsvsz *3 *sizeof(double));
    if (!df) { fbc_delete(dup, dupas); return NULL; }
    dup->priors = dup->frqs   +dup->clsvsz;
    dup->posts  = dup->priors +dup->clsvsz;
    sf = fbc->frqs +2 *dup->clscnt;
    for (df += i = 2 *dup->clscnt; --i >= 0; )
      *--df = *--sf;            /* copy the class frequencies */
    dup->mvns = mvn = (MVNORM**)calloc(dup->clscnt, sizeof(MVNORM*));
    if (!mvn) { fbc_delete(dup, dupas); return NULL; }
    for (mvn += i = dup->clscnt; --i >= 0; ) {
      *--mvn = mvn_dup(fbc->mvns[i]);
      if (!*mvn) { fbc_delete(dup, dupas); return NULL; }
    }                           /* allocate a distribution vector and */
  }                             /* copy multivariate normal distribs. */

  return dup;                   /* return the created duplicate */
}  /* fbc_dup() */

/*--------------------------------------------------------------------*/

void fbc_delete (FBC *fbc, int delas)
{                               /* --- delete a full Bayes classifier */
  int    i;                     /* loop variable */
  MVNORM **p;                   /* to traverse the distrib. vector */

  assert(fbc);                  /* check the function argument */
  if (fbc->mvns) {              /* if there is a distribution vector */
    for (p = fbc->mvns +(i = fbc->clscnt); --i >= 0; )
      if (*--p) mvn_delete(*p); /* delete the multivar. normal dists. */
    free(fbc->mvns);            /* and delete the vector itself, */
  }                             /* then delete the frequency vectors */
  if (fbc->frqs)   free(fbc->frqs);
  if (fbc->vals)   free(fbc->vals);
  if (fbc->numids) free(fbc->numids);
  if (delas)       as_delete(fbc->attset);
  free(fbc);                    /* delete the classifier body */
}  /* fbc_delete() */

/*--------------------------------------------------------------------*/

void fbc_clear (FBC *fbc)
{                               /* --- clear a full Bayes classifier */
  int    i;                     /* loop variables */
  double *frq;                  /* to traverse the frequency vectors */

  assert(fbc);                  /* check the function argument */
  fbc->total = 0;               /* clear the total number of cases */
  for (frq = fbc->frqs +(i = fbc->clscnt); --i >= 0; ) {
    *--frq = 0;                 /* clear the frequency distribution */
    mvn_clear(fbc->mvns[i]);    /* and the multivariate normal */
  }                             /* distributions */
}  /* fbc_clear() */

/*--------------------------------------------------------------------*/
#ifdef FBC_INDUCE

int fbc_add (FBC *fbc, const TUPLE *tpl)
{                               /* --- add an instantiation */
  int   cls;                    /* value of class attribute */
  float wgt;                    /* instantiation weight */

  assert(fbc);                  /* check the function argument */

  /* --- get class and weight --- */
  if (tpl) {                    /* if a tuple is given */
    cls = tpl_colval(tpl, fbc->clsid)->i;
    wgt = tpl_getwgt(tpl); }    /* get the class and the tuple weight */
  else {                        /* if no tuple is given */
    cls = att_inst(as_att(fbc->attset, fbc->clsid))->i;
    wgt = as_getwgt(fbc->attset);
  }                             /* get the class and the inst. weight */
  if (cls < 0) return 0;        /* if the class is unknown, abort */
  assert(wgt >= 0.0F);          /* check the tuple weight */

  /* --- update the class distribution --- */
  if ((cls >= fbc->clscnt)      /* if the class is a new one, */
  &&  (_clsrsz(fbc,cls+1) != 0))/* resize the class dependent vectors */
    return -1;                  /* (frequencies and distributions) */
  fbc->frqs[cls] += wgt;        /* update the class frequency */
  fbc->total     += wgt;        /* and the total frequency */

  /* --- update the conditional distributions --- */
  _getvals(fbc, tpl);           /* get the attribute value vector */
  mvn_add(fbc->mvns[cls], fbc->vals, wgt);
  return 0;                     /* add inst. to the cond. distrib. */
}  /* fbc_add() */              /* return 'ok' */

/*--------------------------------------------------------------------*/

FBC* fbc_induce (TABLE *table, int clsid, int mode, double lcorr)
{                               /* --- create a full Bayes classifier */
  int    i;                     /* loop variable */
  FBC    *fbc;                  /* created full Bayes classifier */
  ATTSET *attset;               /* attribute set of the classifier */

  assert(table                  /* check the function arguments */
      && (clsid >= 0) && (clsid < tab_colcnt(table))
      && (att_type(as_att(tab_attset(table), clsid)) == AT_SYM));

  /* --- create a classifier --- */
  attset = tab_attset(table);   /* get the attribute set of the table */
  if (mode & FBC_DUPAS) {       /* if the corresp. flag is set, */
    attset = as_dup(attset);    /* duplicate the attribute set */
    if (!attset) return NULL;   /* of the given data table, */
  }                             /* then create a classifier */
  fbc = fbc_create(attset, clsid);
  if (!fbc) { if (mode & FBC_DUPAS) as_delete(attset); return NULL; }

  /* --- build the classifier --- */
  for (i = tab_tplcnt(table); --i >= 0; )
    fbc_add(fbc, tab_tpl(table, i));        /* add all tuples */
  fbc_setup(fbc, mode, lcorr);   /* and set up the classifier */

  return fbc;                    /* return the created classifier */
}  /* fbc_induce() */

/*--------------------------------------------------------------------*/

int fbc_mark (FBC *fbc)
{                               /* --- mark selected attributes */
  int   i;                      /* loop variable, attibute counter */
  FBCID *p;                     /* to traverse the attribute ids. */

  assert(fbc);                  /* check the function argument */
  for (i = fbc->attcnt; --i >= 0; )   /* unmark all attributes */
    att_setmark(as_att(fbc->attset, i), -1);
  for (p = fbc->numids +(i = fbc->numcnt); --i >= 0; ) {
    --p; att_setmark(p->att, 1); }    /* mark all numeric attributes */
  att_setmark(as_att(fbc->attset, fbc->clsid), 0);
  return fbc->numcnt +1;        /* mark the class attribute and */
}  /* fbc_mark() */             /* return the number of marked atts. */

#endif
/*--------------------------------------------------------------------*/

void fbc_setup (FBC *fbc, int mode, double lcorr)
{                               /* --- set up a full Bayes classifier */
  int    i, n;                  /* loop variables */
  double cnt;                   /* number of cases, sum of priors */
  double *frq, *prb;            /* to traverse the value frqs./probs. */
  MVNORM **mvn;                 /* to traverse the distributions */

  assert(fbc && (lcorr >= 0));  /* check the function arguments */
  fbc->mode  = mode = mode & FBC_MAXLLH;
  fbc->lcorr = lcorr;           /* note estimation parameters */

  /* --- estimate class probabilities --- */
  n   = fbc->clscnt;            /* get the number of classes and */
  prb = fbc->priors +n;         /* traverse the class probabilities */
  cnt = fbc->total +lcorr *fbc->clscnt;
  if (cnt <= 0)                 /* if the denominator is invalid, */
    while (--n >= 0) *--prb = 0;       /* clear all probabilities */
  else {                        /* if the denominator is valid, */
    frq = fbc->frqs +n;         /* traverse the class frequencies */
    while (--n >= 0) *--prb = (*--frq +lcorr) /cnt;
  }                             /* estimate the class probabilities */

  /* --- estimate conditional probabilities --- */
  mode |= MVN_ALL;              /* set flags for all parameters */
  for (mvn = fbc->mvns +(i = fbc->clscnt); --i >= 0; )
    mvn_calc(*--mvn, mode);     /* calculate all parameters */
}  /* fbc_setup() */

/*--------------------------------------------------------------------*/

int fbc_exec (FBC *fbc, const TUPLE *tpl, double *conf)
{                               /* --- execute a full Bayes class. */
  int    i;                     /* loop variable */
  double *s, *d;                /* to traverse the probabilities */
  MVNORM **mvn;                 /* to traverse the distributions */
  double sum;                   /* sum of class probabilities */

  assert(fbc);                  /* check the function argument */
  _getvals(fbc, tpl);           /* get the attribute value vector */
  s = fbc->priors +fbc->clscnt; /* get the prior     distribution */
  d = fbc->posts  +fbc->clscnt; /* and the posterior distribution */
  for (mvn = fbc->mvns +(i = fbc->clscnt); --i >= 0; ) {
    --mvn;                      /* traverse the cond. distributions */
    *--d = *--s * mvn_eval(*mvn, fbc->vals);
  }                             /* compute the posterior probability */
  for (s = d, sum = *s, i = fbc->clscnt; --i > 0; ) {
    if (*++s > *d) d = s;       /* find the most probable class */
    sum += *s;                  /* and sum all probabilities */
  }                             /* (for the later normalization) */
  if (conf) *conf = (sum > 0) ? *d /sum : 0;
  return (int)(d -fbc->posts);  /* compute a confidence value and */
}  /* fbc_exec() */             /* return the classification result */

/*--------------------------------------------------------------------*/

double* fbc_rand (FBC *fbc, double drand (void))
{                               /* --- generate a random tuple */
  int    i;                     /* loop variable */
  double t, sum;                /* random number, sum of probs. */
  double *p = fbc->priors;      /* to access the class probabilities */
  FBCID  *q = fbc->numids;      /* to traverse the attributes */

  t = drand();                  /* generate a random number */
  for (sum = i = 0; i < fbc->clscnt; i++) {
    sum += p[i]; if (sum >= t) break; }
  if (i >= fbc->clscnt)         /* find the class that corresponds */
    i = fbc->clscnt -1;         /* to the generated random number */
  att_inst(as_att(fbc->attset, fbc->clsid))->i = i;
  p = mvn_rand(fbc->mvns[i], drand);   /* generate a random point */
  for (q = fbc->numids +(i = fbc->numcnt); --i >= 0; ) {
    --q; att_inst(q->att)->f = (float)p[i]; }
  return p;                     /* copy the point to the att. set */
}  /* fbc_rand() */             /* and return the generated point */

/*--------------------------------------------------------------------*/

int fbc_desc (FBC *fbc, FILE *file, int mode, int maxlen)
{                               /* --- describe a full Bayes class. */
  int   i, k;                   /* loop variables */
  int   pos, ind;               /* current position and indentation */
  int   len;                    /* length of a class value name */
  ATT   *att;                   /* to traverse the attributes */
  FBCID *p;                     /* to traverse the attribute ids. */
  char  buf[4*AS_MAXLEN+4];     /* output buffer */

  assert(fbc && file);          /* check the function arguments */

  /* --- print a header (as a comment) --- */
  if (mode & FBC_TITLE) {       /* if the title flag is set */
    i = k = (maxlen > 0) ? maxlen -2 : 70;
    fputs("/*", file); while (--i >= 0) fputc('-', file);
    fputs("\n  full Bayes classifier\n", file);
    while (--k >= 0) fputc('-', file); fputs("*/\n", file);
  }                             /* print a title header */
  if (maxlen <= 0) maxlen = INT_MAX;

  /* --- start description --- */
  att = as_att(fbc->attset, fbc->clsid);
  sc_format(buf, att_name(att), 0);
  fputs("fbc(", file);          /* get the class attribute name */
  fputs(buf, file);             /* and print it */
  fputs(") = {\n", file);       /* start the classifier */
  if ((fbc->lcorr > 0)          /* if estimation parameters */
  ||   fbc->mode) {             /* differ from default values */
    fprintf(file, "  params = %g", fbc->lcorr);
    if (fbc->mode & FBC_MAXLLH) fputs(", maxllh", file);
    fputs(";\n", file);         /* print Laplace correction */
  }                             /* and estimation mode */

  /* --- print class distribution --- */
  fputs("  prob(", file);       /* print a distribution indicator */
  fputs(buf, file);             /* print the class att. name and */
  fputs(") = {\n    ", file);   /* start the the class distribution */
  ind = att_valwd(att, 0) +4;   /* compute the indentation and */
  for (i = 0; i < fbc->clscnt; i++) {  /* traverse the classes */
    if (i > 0)                  /* if this is not the first class, */
      fputs(",\n    ", file);   /* start a new output line */
    len = sc_format(buf, att_valname(att, i), 0);
    fputs(buf, file);           /* get and print the class name */
    for (pos = len+4; pos < ind; pos++)
      putc(' ', file);          /* pad with blanks to equal width */
    fprintf(file, ": %g", fbc->frqs[i]);
    if (mode & FBC_REL)         /* print the absolute class frequency */
      fprintf(file, " (%.1f%%)", fbc->priors[i] *100);
  }                             /* print the relative class frequency */
  fputs(" };\n", file);         /* terminate the class distribution */

  /* --- print conditional distributions --- */
  if (fbc->numcnt > 0) {        /* if there are numeric attributes */
    fputs("  prob(", file);     /* print a distribution indicator */
    pos = ind = 7;              /* and traverse the num. attributes */
    for (p = fbc->numids, i = 0; i < fbc->numcnt; p++, i++) {
      if (i > 0) {              /* if this is not the first att., */
        fputc(',', file); pos++; }           /* print a separator */
      len = sc_format(buf, att_name(p->att), 0);
      if ((pos +len > maxlen-1) /* get the condition name and */
      &&  (pos      > ind)) {   /* if the line would get too long, */
        fputc('\n', file);      /* start a new line and indent */
        for (pos = 0; pos < ind; pos++) fputc(' ', file);
      }                         /* indent to the opening parenthesis */
      fputs(buf, file); pos += len;
    }                           /* print the name of the attribute */
    fputc('|', file);           /* print condition indicator */
    att = as_att(fbc->attset, fbc->clsid);
    sc_format(buf, att_name(att), 0);
    fputs(buf, file);           /* print the class attribute name */
    fputs(") = {\n    ", file); /* and start the distribution */
    ind = att_valwd(att, 0) +4; /* compute the indentation and */
    for (i = 0; i < fbc->clscnt; i++) {  /* traverse the classes */
      if (i > 0)                /* if this is not the first class, */
        fputs(",\n    ", file); /* start a new output line */
      len = sc_format(buf, att_valname(att, i), 0);
      fputs(buf, file);         /* get and print the class name */
      for (pos = len+4; pos < ind; pos++)
        putc(' ', file);        /* pad with blanks to equal width */
      fputs(": N(", file);      /* start a normal distribution */
      mvn_desc(fbc->mvns[i], file, -(ind+4), maxlen);
      fputc(')', file);         /* describe multivar. normal dists. */
    }                           /* and terminate the distribution */
    fputs(" };\n", file);       /* terminate the cond. distribution */
  }
  fputs("};\n", file);          /* terminate the classifier */

  return ferror(file) ? -1 : 0; /* return the write status */
}  /* fbc_desc() */

/*--------------------------------------------------------------------*/
#ifdef FBC_PARSE

static int _paerr (SCAN *scan, int code, int c, const char *s)
{                               /* --- report a parse error */
  char src[  AS_MAXLEN+1];      /* buffer for string to format */
  char dst[4*AS_MAXLEN+4];      /* buffer for fomatted string */

  if ((code == E_DUPCLS) || (code == E_MISCLS) || (code == E_MISATT))
    sc_format(dst, s,   1);     /* if class/attribute error message, */
  else {                        /* format the given name */
    strncpy(src, sc_value(scan), AS_MAXLEN); src[AS_MAXLEN] = '\0';
    sc_format(dst, src, 1);     /* if normal error message, */
  }                             /* copy and format the token value */
  if      (code == E_CHREXP) return sc_error(scan, code, c, dst);
  else if (code == E_STREXP) return sc_error(scan, code, s, dst);
  else                       return sc_error(scan, code,    dst);
}  /* _paerr() */               /* print an error message */

/*--------------------------------------------------------------------*/

static int _parse (ATTSET *attset, SCAN *scan, FBC **pfbc)
{                               /* --- parse a full Bayes classifier */
  int    i = -1, t;             /* loop variable, buffer */
  int    clsid, attid;          /* (class) attribute index */
  ATT    *att;                  /* class attribute */
  FBC    *fbc;                  /* created full Bayes classifier */
  double *p, f;                 /* to traverse the frequencies */
  int    *flags;                /* to traverse the attribute flags */
  FBCID  *ni;                   /* to traverse the numeric att. ids. */

  /* --- read start of description --- */
  if ((sc_token(scan) != T_ID)
  ||  (strcmp(sc_value(scan), "fbc") != 0))
    ERR_STR("fbc");             /* check for 'fbc' */
  GET_TOK();                    /* consume 'fbc' */
  GET_CHR('(');                 /* consume '(' */
  t = sc_token(scan);           /* check for a name */
  if ((t != T_ID) && (t != T_NUM)) ERROR(E_ATTEXP);
  clsid = as_attid(attset, sc_value(scan));
  if (clsid < 0)                   ERROR(E_UNKATT);
  att = as_att(attset, clsid);  /* get and check the class attribute */
  if (att_type(att) != AT_SYM)     ERROR(E_CLSTYPE);
  if (att_valcnt(att) < 1)         ERROR(E_CLSCNT);
  *pfbc = fbc = fbc_create(attset, clsid);
  if (!fbc) ERROR(E_NOMEM);     /* create a full Bayes classifier */
  GET_TOK();                    /* consume the class name */
  GET_CHR(')');                 /* consume '(' */
  GET_CHR('=');                 /* consume '=' */
  GET_CHR('{');                 /* consume '{' */

  /* --- read parameters --- */
  if ((sc_token(scan) == T_ID)  /* if 'params' follows */
  &&  (strcmp(sc_value(scan), "params") == 0)) {
    GET_TOK();                  /* consume 'params' */
    GET_CHR('=');               /* consume '=' */
    if (sc_token(scan) != T_NUM)  ERROR(E_NUMEXP);
    fbc->lcorr = atof(sc_value(scan));
    if (fbc->lcorr < 0)           ERROR(E_ILLNUM);
    GET_TOK();                  /* get Laplace correction */
    while (sc_token(scan) == ',') {
      GET_TOK();                /* read list of parameters */
      if (sc_token(scan) != T_ID) ERROR(E_PAREXP);
      if (strcmp(sc_value(scan), "maxllh") == 0)
        fbc->mode |= FBC_MAXLLH;/* use max. likelihood estimate */
      else ERROR(E_PAREXP);     /* abort on all other values */
      GET_TOK();                /* consume the estimator flag */
    }
    GET_CHR(';');               /* consume ';' */
  }

  /* --- read class distribution --- */
  if ((sc_token(scan) != T_ID)
  ||  ((strcmp(sc_value(scan), "prob") != 0)
  &&   (strcmp(sc_value(scan), "P")    != 0)))
    ERR_STR("prob");            /* check for 'prob' or 'P' */
  GET_TOK();                    /* consume   'prob' or 'P' */
  GET_CHR('(');                 /* consume '(' */
  t = sc_token(scan);           /* get the next token */
  if (((t != T_ID) && (t != T_NUM))
  ||  (strcmp(sc_value(scan), att_name(att)) != 0))
    ERROR(E_ATTEXP);            /* check for the class att. name */
  GET_TOK();                    /* consume the class att. name */
  GET_CHR(')');                 /* consume ')' */
  GET_CHR('=');                 /* consume '=' */
  GET_CHR('{');                 /* consume '{' */
  for (p = fbc->frqs +(i = fbc->clscnt); --i >= 0; )
    *--p = -1;                  /* clear the class frequencies */
  while (1) {                   /* class value read loop */
    t = sc_token(scan);         /* check for the class att. name */
    if ((t != T_ID) && (t != T_NUM)) ERROR(E_CLSEXP);
    if (t != T_NUM) t = ':';    /* if the token is no number, */
    else {                      /* the token must be a class, */
      GET_TOK();                /* otherwise consume the token, */
      t = sc_token(scan);       /* note the next token, and */
      sc_back(scan);            /* go back to the previous one */
    }                           /* (look ahead one token) */
    if (t != ':')               /* if no ':' follows, */
      i = (i+1) % fbc->clscnt;  /* get the cyclic successor id */
    else {                      /* if a  ':' follows */
      i = att_valid(att, sc_value(scan));
      if (i < 0) ERROR(E_UNKCLS);
      GET_TOK();                /* get and consume the class value */
      GET_CHR(':');             /* consume ':' */
    }
    if (sc_token(scan) != T_NUM) ERROR(E_NUMEXP);
    f = atof(sc_value(scan));   /* get and check */
    if (f < 0) ERROR(E_ILLNUM); /* the class frequency */
    if (fbc->frqs[i] >= 0)      /* check whether frequency is set */
      XERROR(E_DUPCLS, att_valname(att, i));
    fbc->frqs[i] = f;           /* set the class frequency */
    GET_TOK();                  /* consume the class frequency */
    if (sc_token(scan) == '('){ /* if a relative number follows, */
      GET_TOK();                /* consume '(' */
      if (sc_token(scan) != T_NUM)  ERROR(E_NUMEXP);
      if (atof(sc_value(scan)) < 0) ERROR(E_ILLNUM);
      GET_TOK();                /* consume the relative number */
      GET_CHR('%');             /* consume '%' */
      GET_CHR(')');             /* consume ')' */
    }
    if (sc_token(scan) != ',') break;
    GET_TOK();                  /* if at end of list, abort loop, */
  }                             /* otherwise consume ',' */
  GET_CHR('}');                 /* consume '}' (end of distribution) */
  for (f = 0, p = fbc->frqs +(i = fbc->clscnt); --i >= 0; ) {
    if (*--p < 0) *p = 0;       /* clear the unset frequencies */
    else          f += *p;      /* and sum all other frequancies */
  }                             /* to obtain the total frequency */
  fbc->total = f;               /* set the sum of the frequencies */
  GET_CHR(';');                 /* consume ';' */

  /* --- read conditional distributions --- */
  if ((sc_token(scan) != T_ID)
  ||  ((strcmp(sc_value(scan), "prob") != 0)
  &&   (strcmp(sc_value(scan), "P")    != 0)))
    ERR_STR("prob");            /* check for 'prob' or 'P' */
  GET_TOK();                    /* consume   'prob' or 'P' */
  GET_CHR('(');                 /* consume '(' */
  if (sc_token(scan) == '*') {  /* if a star follows, */
    GET_TOK(); }                /* simply consume it */
  else {                        /* if a list of attributes follows */
    for (flags = fbc->flags +(i = fbc->attcnt); --i >= 0; )
      *--flags = 0;             /* clear all attribute flags */
    for (ni = fbc->numids +(i = fbc->numcnt); --i >= 0; )
      flags[(--ni)->id] = -1;   /* set flags of numeric attributes */
    while (1) {                 /* attribute read loop */
      t = sc_token(scan);       /* check for a name */
      if ((t != T_ID) && (t != T_NUM)) ERROR(E_ATTEXP);
      attid = as_attid(attset, sc_value(scan));
      if (attid < 0)         ERROR(E_UNKATT);
      if (flags[attid] == 0) ERROR(E_DUPATT);
      flags[attid] = 0;         /* check and clear the attribute flag */
      GET_TOK();                /* consume the attribute name */
      if (sc_token(scan) != ',') break; 
      GET_TOK();                /* if at end of the list, abort loop, */
    }                           /* otherwise consume ',' */
    for (i = fbc->attcnt; --i >= 0; )
      if (flags[i]) XERROR(E_MISATT, att_name(as_att(attset, i)));
  }                             /* check the attribute flags */
  GET_CHR('|');                 /* consume '|' (condition indicator) */
  t = sc_token(scan);           /* get the next token */
  if (((t != T_ID) && (t != T_NUM))
  ||  (strcmp(sc_value(scan), att_name(att)) != 0))
    ERROR(E_CLSEXP);            /* check for a class name */
  GET_TOK();                    /* consume the class name */
  GET_CHR(')');                 /* consume ')' */
  GET_CHR('=');                 /* consume '=' */
  GET_CHR('{');                 /* consume '{' */

  for (p = fbc->posts +(i = fbc->clscnt); --i >= 0; )
    *--p = -1;                  /* mark all classes as unread */
  while (1) {                   /* class value read loop */
    t = sc_token(scan);         /* check for name, number, or 'N' */
    if ((t != T_ID) && (t != T_NUM)) ERROR(E_CLSEXP);
    if (t == T_NUM) t = ':';    /* if the token is a number, */
    else {                      /* the token must be a class, */
      GET_TOK();                /* otherwise consume the token, */
      t = sc_token(scan);       /* note the next token, and */
      sc_back(scan);            /* go back to the previous one */
    }                           /* (look ahead one token) */
    if (t != ':')               /* if no ':' follows, */
      i = (i+1) % fbc->clscnt;  /* get the cyclic successor id */
    else {                      /* if a  ':' follows */
      i = att_valid(att, sc_value(scan));
      if (i < 0) ERROR(E_UNKCLS);
      GET_TOK();                /* get and consume class value */
      GET_CHR(':');             /* consume ':' */
    }
    if (fbc->posts[i] >= 0) ERROR(E_DUPCLS);
    fbc->posts[i] = 1;          /* check and set the read marker */
    if ((sc_token(scan) != T_ID)
    ||  (strcmp(sc_value(scan), "N") != 0))
      ERR_STR("N");             /* check for an 'N' */
    GET_TOK();                  /* consume 'N' */
    GET_CHR('(');               /* consume '(' */
    i = mvn_parse(fbc->mvns[i], scan, fbc->frqs[i]);
    if (i != 0) return i;       /* parse a multivariate normal dist. */
    sc_errmsgs(scan, errmsgs, MSGCNT);     /* restore error messages */
    GET_CHR(')');               /* consume ')' */
    if (sc_token(scan) != ',') break;
    GET_TOK();                  /* if at end of list, abort loop, */
  }                             /* otherwise consume ',' */
  for (p = fbc->posts, i = 0; i < fbc->clscnt; p++, i++) {
    if ((*p < 0) && (fbc->frqs[i] > 0))
      XERROR(E_MISCLS, att_valname(att, i));
  }                             /* check for a complete classifier */
  GET_CHR('}');                 /* consume '}' */
  GET_CHR(';');                 /* consume ';' (end of distribution) */

  GET_CHR('}');                 /* consume '}' */
  GET_CHR(';');                 /* consume ';' (end of classifier) */
  return 0;                     /* return the error status */
}  /* _parse() */

/*--------------------------------------------------------------------*/

FBC* fbc_parse (ATTSET *attset, SCAN *scan)
{                               /* --- parse a full Bayes classifier */
  FBC *fbc = NULL;              /* created full Bayes classifier */

  assert(attset && scan);       /* check the function arguments */
  sc_errmsgs(scan, errmsgs, MSGCNT);
  if (_parse(attset, scan, &fbc) != 0) {
    if (fbc) fbc_delete(fbc,0); /* parse a full Bayes classifier */
    return NULL;                /* if an error occurred, */
  }                             /* delete the classifier and abort */
  fbc_setup(fbc, fbc->mode, fbc->lcorr);
  return fbc;                   /* set up the created classifier */
}  /* fbc_parse() */            /* and then return it */

#endif
