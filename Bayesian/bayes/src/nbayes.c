/*----------------------------------------------------------------------
  File    : nbayes.c
  Contents: Naive Bayes classifier management
  Author  : Christian Borgelt
  History : 07.12.1998 file created
            08.12.1998 nbc_create, nbc_dup, nbc_delete, nbc_add prog.
            10.12.1998 function nbc_desc completed
            11.12.1998 function nbc_exec completed
            12.12.1998 function nbc_parse completed
            16.12.1998 all functions debugged
            13.02.1999 tuple parameters added to nbc_add and nbc_exec
            10.01.1999 execution for one att. made a separate function
            11.03.1999 function nbc_induce added
            25.03.1999 distrib. of tuple weight for unknown values added
            27.03.1999 functions nbc_exp und nbc_var added
            15.05.1999 automatic frequency vector resizing added
            10.11.2000 function nbc_exec adapted
            18.11.2000 function nbc_setup added, nbc_exec adapted
            21.11.2000 redesign completed
            11.02.2001 bug in function nbc_mark (> instead of >=) fixed
            15.07.2001 parser improved (global variables removed)
            16.07.2001 adapted to modified module scan
            17.07.2001 parser improved (conditional look ahead)
            26.04.2003 function nbc_rand added
----------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#ifndef NBC_PARSE
#include "scan.h"
#endif
#include "nbayes.h"
#ifdef STORAGE
#include "storage.h"
#endif

/*----------------------------------------------------------------------
  Preprocessor Definitions
----------------------------------------------------------------------*/
#define	M_PI        3.14159265358979323846  /* \pi */
#define EPSILON     1e-12       /* to handle roundoff errors */
#define BLKSIZE     16          /* block size for vectors */

#ifdef NBC_PARSE

/* --- error codes --- */
#define E_CHREXP    (-16)       /* character expected */
#define E_STREXP    (-17)       /* string expected */
#define E_PAREXP    (-18)       /* parameter expected */
#define E_ATTEXP    (-19)       /* attribute expected */
#define E_VALEXP    (-20)       /* attribute value expected */
#define E_NUMEXP    (-21)       /* number expected */
#define E_ILLNUM    (-22)       /* illegal number */
#define E_UNKATT    (-23)       /* unknown attribute */
#define E_DUPATT    (-24)       /* duplicate attribute */
#define E_MISATT    (-25)       /* missing attribute */
#define E_ILLATT    (-26)       /* illegal attribute */
#define E_UNKVAL    (-27)       /* unknown attribute value */
#define E_DUPVAL    (-28)       /* duplicate attribute value */
#define E_MISVAL    (-29)       /* missing attribute value */
#define E_CLSTYPE   (-30)       /* class attribute must be symbolic */
#define E_CLSCNT    (-31)       /* class attribute has too few values */
#define E_CLSEXP    (-32)       /* class attribute expected */

/* --- functions for parser --- */
#define ERROR(c)    return _paerr(scan, c,        -1, NULL)
#define XERROR(c,s) return _paerr(scan, c,        -1, s)
#define ERR_CHR(c)  return _paerr(scan, E_CHREXP,  c, NULL)
#define ERR_STR(s)  return _paerr(scan, E_STREXP, -1, s)
#define GET_TOK()   if (sc_next(scan) < 0) \
                      return sc_error(scan, sc_token(scan))
#define GET_CHR(c)  if (sc_token(scan) != (c)) ERR_CHR(c); \
                    else GET_TOK();

#endif  /* #ifdef NBC_PARSE */
/*----------------------------------------------------------------------
  Type Definitions
----------------------------------------------------------------------*/
typedef struct {                /* --- selectable attribute --- */
  int    attid;                 /* attribute identifier */
  double errs;                  /* number of misclassifications */
} SELATT;                       /* (selectable attribute) */

/*----------------------------------------------------------------------
  Constants
----------------------------------------------------------------------*/
#ifdef NBC_PARSE
#ifdef GERMAN                   /* deutsche Texte */
static const char *errmsgs[] = {/* Fehlermeldungen */
  /* E_CHREXP  -16 */  "\"%c\" erwartet statt %s",
  /* E_STREXP  -17 */  "\"%s\" erwartet statt %s",
  /* E_PAREXP  -18 */  "Parameter erwartet statt %s",
  /* E_ATTEXP  -19 */  "Attribut erwartet statt %s",
  /* E_VALEXP  -20 */  "Attributwert erwartet statt %s",
  /* E_NUMEXP  -21 */  "Zahl erwartet statt %s",
  /* E_ILLNUM  -22 */  "ungültige Zahl %s",
  /* E_UNKATT  -23 */  "unbekanntes Attribut %s",
  /* E_DUPATT  -24 */  "doppeltes Attribut %s",
  /* E_MISATT  -25 */  "fehlendes Attribut %s",
  /* E_ILLATT  -26 */  "ungültiges Attribut %s",
  /* E_UNKVAL  -27 */  "unbekannter Attributwert %s",
  /* E_DUPVAL  -28 */  "doppelter Attributwert %s",
  /* E_MISVAL  -29 */  "fehlender Attributwert %s",
  /* E_CLSTYPE -30 */  "Klassenattribut %s ist nicht symbolisch",
  /* E_CLSCNT  -31 */  "Klassenattribut %s hat zu wenige Werte",
  /* E_CLSEXP  -32 */  "Klassenattribut erwartet statt %s",
};
#else                           /* English texts */
static const char *errmsgs[] = {/* error messages */
  /* E_CHREXP  -16 */  "\"%c\" expected instead of %s",
  /* E_STREXP  -17 */  "\"%s\" expected instead of %s",
  /* E_PAREXP  -18 */  "parameter expected instead of %s",
  /* E_ATTEXP  -19 */  "attribute expected instead of %s",
  /* E_VALEXP  -20 */  "attribute value expected instead of %s",
  /* E_NUMEXP  -21 */  "number expected instead of %s",
  /* E_ILLNUM  -22 */  "illegal number %s",
  /* E_UNKATT  -23 */  "unknown attribute %s",
  /* E_DUPATT  -24 */  "duplicate attribute %s",
  /* E_MISATT  -25 */  "missing attribute %s",
  /* E_ILLATT  -26 */  "illegal attribute %s",
  /* E_UNKVAL  -27 */  "unknown attribute value %s",
  /* E_DUPVAL  -28 */  "duplicate attribute value %s",
  /* E_MISVAL  -29 */  "missing attribute value %s",
  /* E_CLSTYPE -30 */  "class attribute %s must be symbolic",
  /* E_CLSCNT  -31 */  "class attribute %s has too few values",
  /* E_CLSEXP  -32 */  "class attribute expected instead of %s",
};
#endif
#define MSGCNT  (int)(sizeof(errmsgs)/sizeof(const char*))

#endif  /* #ifdef DT_PARSE */
/*----------------------------------------------------------------------
  Auxiliary Functions
----------------------------------------------------------------------*/
#ifdef NBC_INDUCE

static int _clsrsz (NBC *nbc, int clscnt)
{                               /* --- resize class dependent vectors */
  int    i, k, n;               /* loop variables, buffer */
  int    clsvsz;                /* size of the class dep. vectors */
  DVEC   *dvec;                 /* to traverse the distrib. vectors */
  NORMD  *normd;                /* to traverse the normal   distribs. */
  DISCD  *discd;                /* to traverse the discrete distribs. */
  double *frq;                  /* to traverse the frequency vectors */

  assert(nbc && (clscnt >= 0)); /* check the function arguments */

  /* --- resize the class dependent vectors --- */
  clsvsz = nbc->clsvsz;         /* get the class dep. vector size */
  if (clscnt >= clsvsz) {       /* if the vectors are too small */
    clsvsz += (clsvsz > BLKSIZE) ? clsvsz >> 1 : BLKSIZE;
    if (clscnt >= clsvsz) clsvsz = clscnt;
    frq = (double*)realloc(nbc->frqs, clsvsz *4 *sizeof(double));
    if (!frq) return -1;        /* resize the frequencies vector */
    nbc->frqs   = frq;          /* and set the new vector */
    nbc->priors = nbc->frqs   +clsvsz;  /* organize the rest */
    nbc->posts  = nbc->priors +clsvsz;  /* of the allocated */
    nbc->cond   = nbc->posts  +clsvsz;  /* memory block */
    n = clsvsz -nbc->clsvsz;    /* calc. number of new vector fields */
    for (frq += clsvsz, k = n; --k >= 0; )
      *--frq = 0;               /* clear the new vector fields */
    for (dvec = nbc->dvecs +(i = nbc->attcnt); --i >= 0; ) {
      if ((--dvec)->type == 0)  /* traverse all attributes */
        continue;               /* except the class attribute */
      if (dvec->type == AT_SYM){/* if the attribute is symbolic */
        discd = (DISCD*)realloc(dvec->discds, clsvsz *sizeof(DISCD));
        if (!discd) return -1;  /* resize the discrete dists. vector */
        dvec->discds = discd;   /* and set the new vector */
        for (discd += clsvsz, k = n; --k >= 0; ) {
          (--discd)->cnt = 0; discd->frqs = NULL;
        } }                     /* clear the new vector fields */
      else {                    /* if the attribute is numeric */
        normd = (NORMD*)realloc(dvec->normds, clsvsz *sizeof(NORMD));
        if (!normd) return -1;  /* resize the normal dists. vector */
        dvec->normds = normd;   /* and set the new vector */
        for (normd += clsvsz, k = n; --k >= 0; ) {
          (--normd)->cnt = 0; normd->sv = normd->sv2 = 0; }
      }                         /* clear the new vector fields */
    }  /* for (dvec = ... */
    nbc->clsvsz = clsvsz;       /* set new size of the class vectors */
  }  /* if (clscnt >= clsvsz) ... */

  /* --- create new value frequency vectors --- */
  for (dvec = nbc->dvecs +(i = nbc->attcnt); --i >= 0; ) {
    if ((--dvec)->type != AT_SYM)
      continue;                 /* traverse all symbolic attributes */
    discd = dvec->discds +clscnt;
    for (k = clscnt -nbc->clscnt; --k >= 0; ) {
      (--discd)->frqs =         /* allocate a value frequency vector */
      frq = (double*)malloc(dvec->valvsz *2 *sizeof(double));
      if (!frq) break;          /* set the probabilities vector */
      discd->probs = frq +dvec->valvsz;
      for (frq += n = dvec->valvsz; --n >= 0; )
        *--frq = 0;             /* traverse the frequency vectors */
    }                           /* and init. the value frequencies */
    if (k >= 0) break;          /* on error abort the loop */
  }
  if (i >= 0) {                 /* if an error occurred */
    for (i = nbc->attcnt -i; --i >= 0; dvec++) {
      if ((--dvec)->type != AT_SYM) continue;
      discd = dvec->discds +clscnt;
      for (k = clscnt -nbc->clscnt; --k >= 0; )
        if ((--discd)->frqs) { free(discd->frqs); discd->frqs = NULL; }
    }                           /* delete the newly created value */
    return -1;                  /* frequency vectors of the symbolic */
  }                             /* attributes and abort the function */

  nbc->clscnt = clscnt;         /* set the new number of classes */
  return 0;                     /* return 'ok' */
}  /* _clsrsz() */

/*--------------------------------------------------------------------*/

static int _valrsz (DVEC *dvec, int clscnt, int valcnt)
{                               /* --- resize the value freq. vectors */
  int    i, k, n;               /* loop variables, num. of new elems. */
  int    valvsz;                /* size of the value freq. vectors */
  int    bsz;                   /* size of vector in bytes */
  DISCD  *discd;                /* to traverse the discrete distribs. */
  double *frq;                  /* to traverse the frequency vectors */

  assert(dvec                   /* check the function argument */
     && (dvec->type == AT_SYM) && (clscnt >= 0) && (valcnt >= 0));
  valvsz = dvec->valvsz;        /* get the value freq. vector size */
  if (valcnt > valvsz) {        /* if the vectors are too small */
    valvsz += (valvsz > BLKSIZE) ? valvsz >> 1 : BLKSIZE;
    if (valcnt > valvsz) valvsz = valcnt;
    n   = valvsz -dvec->valcnt; /* get the number of new elements */
    bsz = valvsz *2 *sizeof(double);                     
    for (discd = dvec->discds +(i = clscnt); --i >= 0; ) {
      --discd;                  /* traverse the discrete distribs. */
      frq = (double*)realloc(discd->frqs, bsz);
      if (!frq) break;          /* resize the value freq. vector */
      discd->frqs  = frq;       /* and the probabilities vector */
      discd->probs = frq +valvsz;    /* and set the new vectors */
      for (frq += valvsz, k = n; --k >= 0; )
        *--frq = 0;             /* clear the new vector elements */
    }
    if (i < 0) {                /* if an error occurred */
      bsz = dvec->valvsz *2 *sizeof(double);
      for (i = clscnt -i -1; --i >= 0; ) {
        ++discd;                /* traverse the processed distribs. */
        discd->frqs  = (double*)realloc(discd->frqs, bsz);
        discd->probs = discd->frqs +dvec->valvsz;
      }                         /* shrink all value freq. vectors */
      return -1;                /* to their old size */
    }                           /* and then abort */
  }
  dvec->valcnt = valcnt;        /* set the new number of values */
  return 0;                     /* return 'ok' */
}  /* _valrsz() */

#endif
/*--------------------------------------------------------------------*/

static int _exec (const NBC *nbc, int attid, const INST *inst)
{                               /* --- execute for one attribute */
  int         i, k;             /* loop variable, buffer */
  const DVEC  *dvec;            /* to traverse the distrib. vectors */
  const NORMD *normd;           /* to traverse the normal   distribs. */
  const DISCD *discd;           /* to traverse the discrete distribs. */
  double      *prob;            /* to traverse the class probs. */
  double      v, d, s;          /* temporary buffers */

  assert(nbc && inst            /* check the function arguments */
      && (attid >= 0) && (attid < nbc->attcnt));
  dvec = nbc->dvecs +attid;     /* get the distribution vector */
  assert(dvec->type != 0);      /* and check the attribute type */
  if (dvec->type == AT_SYM) {   /* --- if the attribute is symbolic */
    k = inst->i;                /* get and check the attribute value */
    if ((k < 0) || (k >= dvec->valcnt)) return -1;
    discd = dvec->discds +nbc->clscnt;
    prob  = nbc->cond    +nbc->clscnt;
    for (i = nbc->clscnt; --i >= 0; ) {
      d = (--discd)->probs[k];  /* traverse the discrete distribs. */
      *--prob = (d > 0) ? d : EPSILON;
    } }                         /* copy the class probabilities */
  else {                        /* --- if the attribute is numeric */
    if (dvec->type == AT_FLT) { /* if the attribute is real valued */
      if (inst->f <= UV_FLT) return -1;
      v = (double)inst->f; }    /* check and get the attribute value */
    else {                      /* if the attribute is integer valued */
      if (inst->i <= UV_INT) return -1;
      v = (double)inst->i;      /* check and get the attribute value */
    }                           /* (convert it to double) */
    normd = dvec->normds +nbc->clscnt;
    prob  = nbc->cond    +nbc->clscnt;
    for (i = nbc->clscnt; --i >= 0; ) {
      d = v -(--normd)->exp;    /* traverse the normal distributions */
      s = 2 *normd->var;        /* and get their parameters */
      *--prob = (s > 0) ? exp(-d*d/s) /sqrt(M_PI*s) : EPSILON;
    }                           /* compute the probability density */
  }                             /* at the value of the attribute */
  return 0;                     /* return 'ok' */
}  /* _exec() */

/*--------------------------------------------------------------------*/

static double _normd (double drand (void))
{                               /* --- compute N(0,1) distrib. number */
  static double b;              /* buffer for random number */
  double x, y, r;               /* coordinates and radius */

  if (b != 0.0) {               /* if the buffer is full, */
    x = b; b = 0; return x; }   /* return the buffered number */
  do {                          /* pick a random point */
    x = 2.0*drand()-1.0;        /* in the unit square [-1,1]^2 */
    y = 2.0*drand()-1.0;        /* and check whether it lies */
    r = x*x +y*y;               /* inside the unit circle */
  } while ((r > 1) || (r == 0));
  r = sqrt(-2*log(r)/r);        /* factor for Box-Muller transform */
  b = x *r;                     /* save one of the random numbers */
  return y *r;                  /* and return the other */
}  /* _normd() */

/*--------------------------------------------------------------------*/
#ifdef NBC_INDUCE

static int _eval (NBC *nbc, TABLE *table, int mode,
                  SELATT *savec, int cnt)
{                               /* --- evaluate selectable attributes */
  int    i, k, n;               /* loop variables, buffers */
  SELATT *sa;                   /* to traverse the selectable atts. */
  TUPLE  *tpl;                  /* to traverse the tuples */
  double *s, *d;                /* to traverse the probabilities */
  double max, tmp;              /* maximum of probabilities, buffer */
  int    old, new;              /* old and new predicted class */
  int    cls;                   /* actual class of a tuple */

  assert(nbc && table && savec  /* check the function arguments */
     && (cnt > 0) && (mode & (NBC_ADD|NBC_REMOVE)));
  for (n = tab_tplcnt(table); --n >= 0; ) {
    tpl = tab_tpl(table, n);    /* traverse the tuples in the table */
    cls = tpl_colval(tpl, nbc->clsid)->i;
    if (cls < 0) continue;      /* skip tuples with an unknown class */
    old = nbc_exec(nbc, tpl, NULL);
    for (sa = savec +(i = cnt); --i >= 0; ) {
      --sa;                     /* traverse the selectable attributes */
      if (_exec(nbc, sa->attid, tpl_colval(tpl, sa->attid)) != 0)
        new = old;              /* evaluate the classifier and */
      else {                    /* on failure use the old class */
        s = nbc->cond;          /* if a probability distribution */
        d = nbc->posts;         /* could be determined, traverse it */
        if (mode & NBC_ADD) {   /* if to add attributes, */
          max = *d * *s;        /* multiply with cond. probability */
          for (new = 0, k = 1; k < nbc->clscnt; k++) {
            tmp = *++d * *++s;  /* compute new probability */
            if (tmp > max) { max = tmp; new = k; }
          } }                   /* find the most probable class */
        else {                  /* if to remove attributes, */
          max = *d / *s;        /* divide by cond. probability */
          for (new = 0, k = 1; k < nbc->clscnt; k++) {
            tmp = *++d / *++s;  /* compute new probability */
            if (tmp > max) { max = tmp; new = k; }
          }                     /* find the most probable class */
        }                       /* for the current tuple */
      }                         /* (det. new classification result) */
      if (new != cls) sa->errs += tpl_getwgt(tpl);
    }                           /* count the misclassifications */
  }                             /* of the modified classifier */
  return 0;                     /* return 'ok' */
}  /* _eval() */

#endif
/*----------------------------------------------------------------------
  Main Functions
----------------------------------------------------------------------*/

NBC* nbc_create (ATTSET *attset, int clsid)
{                               /* --- create a naive Bayes class. */
  int    i, k, n;               /* loop variables */
  NBC    *nbc;                  /* created classifier */
  ATT    *att;                  /* to traverse the attributes */
  DVEC   *dvec;                 /* to traverse the distrib. vectors */
  DISCD  *discd;                /* to traverse the discrete distribs. */
  NORMD  *normd;                /* to traverse the normal   distribs. */
  double *frq;                  /* to traverse the frequency vectors */

  assert(attset && (clsid >= 0) /* check the function arguments */
      && (clsid < as_attcnt(attset))
      && (att_type(as_att(attset, clsid)) == AT_SYM));

  /* --- create the classifier body --- */
  i   = as_attcnt(attset);      /* get the number of attributes */
  nbc = (NBC*)malloc(sizeof(NBC) +(i-1) *sizeof(DVEC));
  if (!nbc) return NULL;        /* allocate the classifier body */
  for (dvec = nbc->dvecs +(k = i); --k >= 0; ) {
    (--dvec)->discds = NULL; dvec->normds = NULL;
  }                             /* clear the distribution vectors */
  nbc->attset = attset;         /* (for a proper clean up on error) */
  nbc->attcnt = i;              /* and initialize the other fields */
  nbc->clsid  = clsid;
  nbc->clsvsz = att_valcnt(as_att(attset, clsid));
  nbc->clscnt = nbc->clsvsz;
  nbc->total  = 0;
  nbc->lcorr  = 0;
  nbc->mode   = 0;

  /* --- initialize the class distributions --- */
  if (nbc->clscnt <= 0) {       /* if there are no classes, */
    nbc->frqs   =               /* no class vectors are needed */
    nbc->priors = nbc->posts = nbc->cond = NULL; }
  else {                        /* if there are classes, */
    nbc->frqs =                 /* allocate class vectors */
    frq = (double*)malloc(nbc->clsvsz *4 *sizeof(double));
    if (!frq) { nbc_delete(nbc, 0); return NULL; }
    nbc->priors = frq         +nbc->clsvsz;
    nbc->posts  = nbc->priors +nbc->clsvsz;
    nbc->cond   = nbc->posts  +nbc->clsvsz;
    for (frq += k = nbc->clsvsz; --k >= 0; )
      *--frq = 0;               /* traverse the frequency vector */
  }                             /* and init. the class frequencies */

  /* --- initialize the conditional distributions --- */
  for (dvec = nbc->dvecs +(i = nbc->attcnt); --i >= 0; ) {
    (--dvec)->mark = -1;        /* traverse and unmark all attributes */
    if (i == clsid) {           /* if this is the class attribute, */
      dvec->type = 0; continue;}/* clear the type for easier recogn. */
    att = as_att(attset, i);    /* get the next attribute */
    dvec->type = att_type(att); /* and its type */
    if (dvec->type == AT_SYM) { /* -- if the attribute is symbolic */
      dvec->valcnt =            /* set the number of att. values */
      dvec->valvsz = att_valcnt(att);
      if (nbc->clscnt <= 0)     /* if there are no classes, */
        continue;               /* there is nothing else to do */
      dvec->discds =            /* create a vector of discrete dists. */
      discd = (DISCD*)calloc(nbc->clsvsz, sizeof(DISCD));
      if (!discd) { nbc_delete(nbc, 0); return NULL; }
      if (dvec->valcnt <= 0)    /* if the attribute has no values, */
        continue;               /* there is nothing else to do */
      for (discd += k = nbc->clscnt; --k >= 0; ) {
        (--discd)->frqs =       /* create a value frequency vector */
        frq = (double*)malloc(dvec->valvsz *2 *sizeof(double));
        if (!frq) { nbc_delete(nbc, 0); return NULL; }
        discd->probs = frq +dvec->valvsz;
        for (frq += n = dvec->valvsz; --n >= 0; )
          *--frq = 0;           /* traverse the frequency vectors */
      } }                       /* and init. the value frequencies */
    else {                      /* -- if the attribute is numeric */
      dvec->valcnt = dvec->valvsz = 0;
      if (nbc->clscnt <= 0)     /* if there are no classes, */
        continue;               /* there is nothing else to do */
      dvec->normds =            /* create a vector of normal dists. */
      normd = (NORMD*)malloc(nbc->clsvsz *sizeof(NORMD));
      if (!normd) { nbc_delete(nbc, 0); return NULL; }
      for (normd += k = nbc->clsvsz; --k >= 0; ) {
        (--normd)->cnt = 0; normd->sv = normd->sv2 = 0; }
    }                           /* clear the sums from which expected */
  }                             /* value and variance are computed */

  return nbc;                   /* return the created classifier */
}  /* nbc_create() */

/*--------------------------------------------------------------------*/

NBC* nbc_dup (NBC *nbc, int dupas)
{                               /* --- duplicate a naive Bayes class. */
  NBC    *dup;                  /* created classifier duplicate */
  ATTSET *attset;               /* duplicate of attribute set */
  int    i, k, n;               /* loop variables */
  DVEC   *dv; const DVEC   *sv; /* to traverse the distrib. vectors */
  NORMD  *dn; const NORMD  *sn; /* to traverse the normal   distribs. */
  DISCD  *dd; const DISCD  *sd; /* to traverse the discrete distribs. */
  double *df; const double *sf; /* to traverse the frequency vectors */

  assert(nbc);                  /* check the function argument */

  /* --- copy the classifier body --- */
  attset = nbc->attset;         /* get the attribute set */
  if (dupas) {                  /* if the corresp. flag is set, */
    attset = as_dup(attset);    /* duplicate the attribute set */
    if (!attset) return NULL;   /* of the original classifier, */
  }                             /* and then create a classifier */
  dup = (NBC*)malloc(sizeof(NBC) +(nbc->attcnt-1) *sizeof(DVEC));
  if (!dup) { if (dupas) as_delete(attset); return NULL; }
  for (dv = dup->dvecs +(i = nbc->attcnt); --i >= 0; ) {
    (--dv)->discds = NULL; dv->normds = NULL;
  }                             /* clear the distribution vectors */
  dup->attset = attset;         /* (for a proper clean up on error) */
  dup->attcnt = nbc->attcnt;    /* and copy the other fields */
  dup->clsid  = nbc->clsid;
  dup->clsvsz = nbc->clscnt;
  dup->clscnt = nbc->clscnt;
  dup->total  = nbc->total;
  dup->lcorr  = nbc->lcorr;
  dup->mode   = nbc->mode;

  /* --- copy the class distributions --- */
  if (nbc->clscnt <= 0)         /* if there are no classes, */
    dup->frqs   =               /* no class vectors are needed */
    dup->priors = dup->posts = dup->cond = NULL;
  else {                        /* if there are classes, */
    dup->frqs =                 /* allocate class vectors */
    df = (double*)malloc(dup->clsvsz *4 *sizeof(double));
    if (!df) { nbc_delete(dup, dupas); return NULL; }
    dup->priors = dup->frqs   +dup->clsvsz;
    dup->posts  = dup->priors +dup->clsvsz;
    dup->cond   = dup->posts  +dup->clsvsz;
    sf = nbc->frqs +2 *dup->clscnt;
    for (df += k = 2 *dup->clscnt; --k >= 0; )
      *--df = *--sf;            /* traverse the frequency vector */
  }                             /* and copy the class frequencies */

  /* --- copy the conditional distributions --- */
  sv = nbc->dvecs +nbc->attcnt; /* get pointers to the */
  dv = dup->dvecs +nbc->attcnt; /* distribution vectors */
  for (i = nbc->attcnt; --i >= 0; ) {
    --sv; --dv;                 /* traverse the distribution vectors */
    dv->mark   = sv->mark;      /* copy the attribute mark, */
    dv->type   = sv->type;      /* the attribute type, */
    dv->valvsz = sv->valcnt;    /* the value vector size, and */
    dv->valcnt = sv->valcnt;    /* the number of attribute values */
    if ((sv->type    == 0)      /* if this is the class attribute */
    ||  (nbc->clscnt <= 0))     /* or if there are no classes, */
      continue;                 /* there is nothing else to do */
    if (sv->type == AT_SYM) {   /* -- if the attribute is symbolic */
      dv->discds =              /* create a vector of discrete dists. */
      dd = (DISCD*)calloc(dup->clsvsz, sizeof(DISCD));
      if (!dd) { nbc_delete(dup, dupas); return NULL; }
      if (sv->valcnt <= 0)      /* if the attribute has no values, */
        continue;               /* there is nothing else to do */
      sd = sv->discds +nbc->clscnt;
      for (dd += (k = nbc->clscnt); --k >= 0; ) {
        --dd; --sd;             /* traverse the discrete distribs. */
        dd->cnt  = sd->cnt;     /* copy the total frequency and */
        dd->frqs =              /* create a value frequency vector */
        df = (double*)malloc(dv->valvsz *2 *sizeof(double));
        if (!df) { nbc_delete(dup, dupas); return NULL; }
        dd->probs = df +dv->valvsz;
        sf = sd->frqs +2 *dv->valvsz;
        for (df += n = 2 *dv->valvsz; --n >= 0; )
          *--df = *--sf;        /* traverse the frequency vectors */
      } }                       /* and copy the value frequencies */
    else {                      /* -- if the attribute is numeric */
      dv->normds =              /* create a vector of normal dists. */
      dn = (NORMD*)malloc(dup->clsvsz *sizeof(NORMD));
      if (!dn) { nbc_delete(dup, dupas); return NULL; }
      sn = sv->normds +dup->clsvsz;
      for (dn += k = dup->clsvsz; --k >= 0; )
        *--dn = *--sn;          /* copy the normal distributions */
    }                           /* (including computed estimates) */
  }
  return dup;                   /* return the created duplicate */
}  /* nbc_dup() */

/*--------------------------------------------------------------------*/

void nbc_delete (NBC *nbc, int delas)
{                               /* --- delete a naive Bayes class. */
  int   i, k;                   /* loop variables */
  DVEC  *dvec;                  /* to traverse the distrib. vectors */
  DISCD *discd;                 /* to traverse the discrete distribs. */

  assert(nbc);                  /* check the function argument */
  for (dvec = nbc->dvecs +(i = nbc->attcnt); --i >= 0; ) {
    if ((--dvec)->discds) {     /* traverse the attributes */
      for (discd = dvec->discds +(k = nbc->clscnt); --k >= 0; )
        if ((--discd)->frqs) free(discd->frqs);
      free(dvec->discds);       /* delete all frequency vectors */
    }                           /* and the distribution vectors */
    if (dvec->normds) free(dvec->normds);
  }                             /* delete the normal distributions */
  if (nbc->frqs) free(nbc->frqs);
  if (delas)     as_delete(nbc->attset);
  free(nbc);                    /* delete the classifier body */
}  /* nbc_delete() */

/*--------------------------------------------------------------------*/

void nbc_clear (NBC *nbc)
{                               /* --- clear a naive Bayes classifier */
  int    i, k, n;               /* loop variables */
  DVEC   *dvec;                 /* to traverse the distrib. vectors */
  DISCD  *discd;                /* to traverse the discrete distribs. */
  NORMD  *normd;                /* to traverse the normal   distribs. */
  double *frq;                  /* to traverse the frequency vectors */

  assert(nbc);                  /* check the function argument */
  nbc->total = 0;               /* clear the total number of cases */
  for (frq = nbc->frqs +(i = nbc->clscnt); --i >= 0; )
    *--frq = 0;                 /* clear the frequency distribution */
  for (dvec = nbc->dvecs +(i = nbc->attcnt); --i >= 0; ) {
    if ((--dvec)->type == 0)    /* traverse all attributes */
      continue;                 /* except the class attribute */
    if (dvec->type == AT_SYM) { /* if the attribute is symbolic */
      for (discd = dvec->discds +(k = nbc->clscnt); --k >= 0; ) {
        (--discd)->cnt = 0;     /* traverse the distributions */
        for (frq = discd->frqs +(n = dvec->valcnt); --n >= 0; )
          *--frq = 0;           /* traverse the frequency vector */
      } }                       /* and clear the value frequencies */
    else {                      /* if the attribute is numeric */
      for (normd = dvec->normds +(k = nbc->clscnt); --k >= 0; ) {
        (--normd)->cnt = 0; normd->sv = normd->sv2 = 0; }
    }                           /* clear the sums from which expected */
  }                             /* value and variance are computed */
}  /* nbc_clear() */

/*--------------------------------------------------------------------*/
#ifdef NBC_INDUCE

int nbc_add (NBC *nbc, const TUPLE *tpl)
{                               /* --- add an instantiation */
  int    i;                     /* loop variable */
  int    cls;                   /* value of class attribute */
  float  wgt;                   /* instantiation weight */
  const  INST *inst;            /* to traverse the instances */
  DVEC   *dvec;                 /* to traverse the distrib. vectors */
  NORMD  *normd;                /* to access normal   distributions */
  DISCD  *discd;                /* to access discrete distributions */
  double v;                     /* buffer (for an attribute value) */

  assert(nbc);                  /* check the function argument */

  /* --- get class and weight --- */
  if (tpl) {                    /* if a tuple is given */
    cls = tpl_colval(tpl, nbc->clsid)->i;
    wgt = tpl_getwgt(tpl); }    /* get the class and the tuple weight */
  else {                        /* if no tuple is given */
    cls = att_inst(as_att(nbc->attset, nbc->clsid))->i;
    wgt = as_getwgt(nbc->attset);
  }                             /* get the class and the inst. weight */
  if (cls < 0) return 0;        /* if the class is unknown, abort */
  assert(wgt >= 0.0F);          /* check the tuple weight */

  /* --- update class distribution --- */
  if ((cls >= nbc->clscnt)      /* if the class is a new one, */
  &&  (_clsrsz(nbc,cls+1) != 0))/* resize the class dependent vectors */
    return -1;                  /* (frequencies and distributions) */
  nbc->frqs[cls] += wgt;        /* update the class frequency */
  nbc->total     += wgt;        /* and the total frequency */

  /* --- update conditional distributions --- */
  for (dvec = nbc->dvecs +(i = nbc->attcnt); --i >= 0; ) {
    if ((--dvec)->type == 0)    /* traverse all attributes */
      continue;                 /* except the class attribute */
    inst = (tpl)                /* get the attribute instantiation */
         ? tpl_colval(tpl, i)   /* from the tuple or the att. set */
         : att_inst(as_att(nbc->attset, i));
    if (dvec->type == AT_SYM) { /* -- if the attribute is symbolic */
      if (inst->i < 0)          /* if the att. value is unknown, */
        continue;               /* there is nothing to do */
      if ((inst->i >= dvec->valcnt)
      &&  (_valrsz(dvec, nbc->clscnt, inst->i+1) != 0))
        return -1;              /* resize the value freq. vectors */
      discd = dvec->discds +cls;   /* get the proper distribution */
      discd->frqs[inst->i] += wgt; /* and update the value frequency */
      discd->cnt += wgt; }         /* and the total frequency */
    else {                      /* -- if the attribute is numeric */
      if (dvec->type == AT_FLT){/* if the attribute is real valued */
        if (inst->f <= UV_FLT) continue;
        v = (double)inst->f;}   /* check and get the attribute value */
      else {                    /* if the attribute is integer valued */
        if (inst->i <= UV_INT) continue;
        v = (double)inst->i;    /* check and get the attribute value */
      }                         /* (convert it to double) */
      normd = dvec->normds +cls;/* get the proper distribution */
      normd->cnt += wgt;        /* update the case counter */
      normd->sv  += wgt *v;     /* the sum of the values, and */
      normd->sv2 += wgt *v*v;   /* the sum of their squares */
    }                           /* (expected value and variance */
  }                             /*  are computed in nbc_setup) */
  return 0;                     /* return 'ok' */
}  /* nbc_add() */

/*--------------------------------------------------------------------*/

NBC* nbc_induce (TABLE *table, int clsid, int mode, double lcorr)
{                               /* --- induce a naive Bayes class. */
  int    i, r = 0;              /* loop variable, buffer */
  int    cnt;                   /* number of selectable attributes */
  int    cls;                   /* predicted class */
  NBC    *nbc;                  /* created classifier */
  ATTSET *attset;               /* attribute set of the classifier */
  SELATT *savec;                /* vector of selectable attributes */
  SELATT *sa, *best;            /* to traverse the selectable atts. */
  TUPLE  *tpl;                  /* to traverse the tuples */
  DVEC   *dvec;                 /* to traverse the distrib. vectors */
  double *p;                    /* to traverse the class probs. */
  double max;                   /* maximum of class probabilities */
  double errs;                  /* weight sum of misclassified tuples */

  assert(table                  /* check the function arguments */
      && (clsid >= 0) && (clsid < tab_colcnt(table))
      && (att_type(as_att(tab_attset(table), clsid)) == AT_SYM));

  /* --- create a classifier --- */
  attset = tab_attset(table);   /* get the attribute set of the table */
  if (mode & NBC_DUPAS) {       /* if the corresp. flag is set, */
    attset = as_dup(attset);    /* duplicate the attribute set */
    if (!attset) return NULL;   /* of the given data table, */
  }                             /* then create a classifier */
  nbc = nbc_create(attset, clsid);
  if (!nbc) { if (mode & NBC_DUPAS) as_delete(attset); return NULL; }

  /* --- build initial classifier --- */
  for (i = tab_tplcnt(table); --i >= 0; )
    nbc_add(nbc, tab_tpl(table, i));    /* start with a */
  nbc_setup(nbc, mode|NBC_ALL, lcorr);  /* full classifier */
  if (!(mode & (NBC_ADD|NBC_REMOVE)))
    return nbc;                 /* if no simp. is requested, abort */

  /* --- evaluate initial classifier --- */
  if (mode & NBC_ADD) {         /* if to add attributes, */
    p = nbc->frqs; errs = max = *p;
    for (i = nbc->clscnt; --i > 0; ) {
      errs += *++p;             /* traverse the class frequencies, */
      if (*p > max) max = *p;   /* sum them, and determine their */
    }                           /* maximum (find the majority class) */
    errs -= max; }              /* compute the number of prior errors */
  else {                        /* if to remove attributes */
    for (errs = 0, i = tab_tplcnt(table); --i >= 0; ) {
      tpl = tab_tpl(table,i);   /* traverse the tuples in the table */
      cls = tpl_colval(tpl, clsid)->i;
      if (cls < 0) continue;    /* skip tuples with an unknown class */
      assert(cls < nbc->clscnt);/* check the class value */
      if (nbc_exec(nbc, tpl, NULL) != cls)
        errs += tpl_getwgt(tpl);/* classify the current tuple */
    }                           /* and determine the number */
  }                             /* of misclassifications */
  #ifndef NDEBUG
  fprintf(stderr, "\n%8g errors initially\n", errs);
  #endif                        /* print a counter for debugging */

  /* --- collect selectable attributes --- */
  savec = malloc(nbc->attcnt *sizeof(SELATT));
  if (!savec) { nbc_delete(nbc, mode & NBC_DUPAS); return NULL; }
  sa = savec;                   /* create vector of selectable atts. */
  for (dvec = nbc->dvecs +(i = nbc->attcnt); --i >= 0; ) {
    (--dvec)->mark = -1;        /* traverse all attributes */
    if ( (dvec->type == 0)      /* except the class attribute */
    ||  ((dvec->type == AT_SYM) /* and all symbolic attributes */
    &&   (dvec->valcnt <= 0)))  /* that do not have any values */
      continue;
    if (mode & NBC_REMOVE)      /* if to remove attributes, */
      dvec->mark = i;           /* mark all selectable attributes */
    sa->attid = i;              /* note selectable attributes and */
    sa->errs  = 0; sa++;        /* initialize the number of errors */
  }
  cnt = (int)(sa -savec);       /* compute the number of attributes */
  nbc->dvecs[nbc->clsid].mark = nbc->clsid;   /* and mark the class */

  /* --- select attributes --- */
  while ((cnt  > 0)             /* while there are selectable atts. */
  &&     (errs > 0)) {          /* and the classifier is not perfect */
    for (sa = savec +(i = cnt); --i >= 0; )
      (--sa)->errs = 0;         /* clear the numbers of errors */
    r = _eval(nbc, table, mode, savec, cnt);
    if (r < 0) break;           /* evaluate selectable attributes */
    best = sa = savec;          /* traverse the selectable attributes */
    for (i = cnt; --i > 0; ) {  /* in order to find the best */
      if (((++sa)->errs <  best->errs)
      ||  ((  sa ->errs <= best->errs) && (mode & NBC_ADD)))
        best = sa;              /* find least number of errors and */
    }                           /* note the corresponding attribute */
    if ( (best->errs >  errs)   /* if more tuples were misclassified */
    ||  ((best->errs >= errs) && (mode & NBC_ADD)))
      break;                    /* abort the selection loop */
    errs = best->errs;          /* note the new number of errors */
    #ifndef NDEBUG
    fprintf(stderr, "%8g errors %s\n", best->errs,
            att_name(as_att(attset, best->attid)));
    #endif                      /* print a counter for debugging */
    nbc->dvecs[best->attid].mark = (mode & NBC_ADD)
      ? best->attid : -1;       /* mark/unmark the selected attribute */
    for (--cnt; best < sa; best++)  /* remove the selected */
      best->attid = best[1].attid;  /* attribute from the  */
  }                                 /* list of attributes  */
  free(savec);                  /* delete the selectable atts. vector */

  if (r < 0) {                  /* if an error occurred, abort */
    nbc_delete(nbc, mode & NBC_DUPAS); return NULL; }
  return nbc;                   /* return the created classifier */
}  /* nbc_induce() */

/*--------------------------------------------------------------------*/

int nbc_mark (NBC *nbc)
{                               /* --- mark selected attributes */
  int  i, m;                    /* loop variable, buffer for marker */
  DVEC *dvec;                   /* to traverse the distrib. vectors */
  int  cnt = 0;                 /* attribute counter */

  assert(nbc);                  /* check the function argument */
  for (dvec = nbc->dvecs +(i = nbc->attcnt); --i >= 0; ) {
    if ((--dvec)->mark < 0) m = -1;
    else           { cnt++; m =  1; }
    att_setmark(as_att(nbc->attset, i), m);
  }                             /* transfer marker to attribute set */
  att_setmark(as_att(nbc->attset, nbc->clsid), 0);
  return cnt;                   /* return number of marked atts. */
}  /* nbc_mark() */

#endif
/*--------------------------------------------------------------------*/

void nbc_setup (NBC *nbc, int mode, double lcorr)
{                               /* --- set up a naive Bayes class. */
  int    i, k, n;               /* loop variables */
  DVEC   *dvec;                 /* to traverse the distrib. vectors */
  NORMD  *normd;                /* to traverse the normal   distribs. */
  DISCD  *discd;                /* to traverse the discrete distribs. */
  double *frq, *prb;            /* to traverse the value frqs./probs. */
  double cnt, sp;               /* number of cases, sum of priors */
  double add;                   /* Laplace corr. + distributed weight */

  assert(nbc && (lcorr >= 0));  /* check the function arguments */
  nbc->mode  = mode & (NBC_DISTUV|NBC_MAXLLH);
  nbc->lcorr = lcorr;           /* note estimation parameters */

  /* --- estimate class probabilities --- */
  cnt = nbc->total +lcorr *nbc->clscnt;
  n   = nbc->clscnt;            /* get the number of classes and */
  prb = nbc->priors +n;         /* traverse the class probabilities */
  if (cnt <= 0)                 /* if the denominator is invalid, */
    while (--n >= 0) *--prb = 0;       /* clear all probabilities */
  else {                        /* if the denominator is valid, */
    frq = nbc->frqs +n;         /* traverse the class frequencies */
    while (--n >= 0) *--prb = (*--frq +lcorr) /cnt;
  }                             /* estimate the class probabilities */

  /* --- estimate conditional probabilities --- */
  for (dvec = nbc->dvecs +(i = nbc->attcnt); --i >= 0; ) {
    if ((--dvec)->type == 0) {  /* traverse all attributes */
      dvec->mark = 0; continue;}/* except the class attribute */
    if      (mode & NBC_ALL)    /* if to use all attributes, */
      dvec->mark = 1;           /* mark attribute as used */
    else if (mode & NBC_MARKED) /* if to use only marked atts. */
      dvec->mark = (att_getmark(as_att(nbc->attset, i)) >= 0)
                 ? 1 : -1;      /* get the attribute mark */
    if (dvec->mark < 0)         /* otherwise keep the */
      continue;                 /* selection of attributes */
    if (dvec->type == AT_SYM) { /* -- if the attribute is symbolic */
      if (dvec->valcnt <= 0) {  /* if the attribute has no values, */
        dvec->mark = -1; continue; }     /* there is nothing to do */
      sp = dvec->valcnt *lcorr; /* compute the sum of the priors */
      for (discd = dvec->discds +(k = nbc->clscnt); --k >= 0; ) {
        --discd;                /* traverse the distributions */
        n   = dvec->valcnt;     /* get the number of att. values */
        prb = discd->probs +n;  /* and the probability vector */
        add = (mode & NBC_DISTUV) ? nbc->frqs[k] : discd->cnt;
        cnt = sp +add;          /* compute denominator of estimator */
        if (cnt <= 0)           /* if the estimator is invalid, */
          while (--n >= 0) *--prb = 0;      /* clear all probs. */
        else {                  /* if the estimator is valid */
          add = lcorr +(add -discd->cnt) /n;
          for (frq = discd->frqs +n; --n >= 0; )
            *--prb = (*--frq +add) /cnt;
        }                       /* traverse the value frequencies */
      } }                       /* and estimate the probabilities */
    else {                      /* -- if the attribute is numeric */
      for (normd = dvec->normds +(k = nbc->clscnt); --k >= 0; ) {
        cnt = (--normd)->cnt;   /* traverse the distributions */
        normd->exp = (cnt > 0) ? normd->sv /cnt : 0;
        if (!(mode & NBC_MAXLLH)) cnt -= 1;
        normd->var = (cnt > 0)
                   ? (normd->sv2 -normd->exp *normd->sv) /cnt : 0;
      }                         /* estimate the expected value and */
    }                           /* the variance (either with unbiased */
  }                             /* or with max. likelihood estimator) */
  nbc->dvecs[nbc->clsid].mark = 1;  /* mark the class attribute */
}  /* nbc_setup() */

/*----------------------------------------------------------------------
Point estimator for the expected value of a normal distribution:
  \hat{\mu} = \frac{1}{n} \sum_{i=1}^n x_i
(unbiased, consistent, and efficient)

Point estimator for the variance of a normal distribution:
  \hat{\sigma}^2 = \frac{1}{n-1} \sum_{i=1}^n (x_i - \hat{\mu})^2
                 = \frac{1}{n-1} (\sum_{i=1}^n x_i^2 - n\hat{\mu}^2)
(unbiased and consistent)

Maximum likelihood estimator for the variance of a normal distribution:
  \hat{\sigma}^2 = \frac{1}{n} \sum_{i=1}^n (x_i - \hat{\mu})^2
                 = \frac{1}{n} (\sum_{i=1}^n x_i^2 - n\hat{\mu}^2)
(consistent)

Source: R.L. Larsen and M.L. Marx.
        An Introduction to Mathematical Statistics and Its Applications.
        Prentice Hall, Englewood Cliffs, NJ, USA 1986, pp. 312 & 314
----------------------------------------------------------------------*/

int nbc_exec (NBC *nbc, const TUPLE *tpl, double *conf)
{                               /* --- execute a naive Bayes class. */
  int        i, k;              /* loop variables */
  DVEC       *dvec;             /* to traverse the distrib. vectors */
  const INST *inst;             /* to traverse the instances */
  double     *s, *d;            /* to traverse the probabilities */
  double     sum;               /* sum of class probabilities */

  assert(nbc);                  /* check the function argument */

  /* --- initialize --- */
  s = nbc->priors +nbc->clscnt; /* init. the posterior distribution */
  d = nbc->posts  +nbc->clscnt; /* with  the prior     distribution */
  for (k = nbc->clscnt; --k >= 0; ) *--d = *--s;

  /* --- process attribute values --- */
  for (dvec = nbc->dvecs +(i = nbc->attcnt); --i >= 0; ) {
    if (((--dvec)->type == 0)   /* traverse all attributes */
    ||  (   dvec ->mark <  0))  /* except the class attribute */
      continue;                 /* and all unmarked attributes */
    inst = (tpl)                /* get the attribute instantiation */
         ? tpl_colval(tpl, i)   /* from the tuple or the att. set */
         : att_inst(as_att(nbc->attset, i));
    if (_exec(nbc, i, inst) < 0)/* execute the classifier */
      continue;                 /* for the current attribute */
    s = nbc->cond +nbc->clscnt; /* traverse the cond. probabilities */
    d = nbc->posts+nbc->clscnt; /* and the posterior distribution */
    for (sum = 0, k = nbc->clscnt; --k >= 0; )
      sum += *--d *= *--s;      /* multiply with cond. probabilities */
    if ((sum > 1e-24) && (sum < 1e24))
      continue;                 /* if the sum is ok, continue */
    if (sum <= 0) break;        /* if the sum is fubar, abort */
    for (d += k = nbc->clscnt; --k >= 0; )
      *--d /= sum;              /* otherwise renormalize in order */
  }                             /* to avoid an over- or underflow */
  s = d = nbc->posts;           /* traverse the final distribution */
  for (sum = *s, k = nbc->clscnt; --k > 0; ) {
    if (*++s > *d) d = s;       /* find the most probable class */
    sum += *s;                  /* and sum all probabilities */
  }                             /* (for the later normalization) */
  if (conf) *conf = (sum > 0) ? *d /sum : 0;
  return (int)(d -nbc->posts);  /* compute a confidence value and */
}  /* nbc_exec() */             /* return the classification result */

/*--------------------------------------------------------------------*/

void nbc_rand (NBC *nbc, double drand (void))
{                               /* --- generate a random tuple */
  int    i, k, n;               /* loop variables */
  double t, sum;                /* random number, sum of probs. */
  double *p;                    /* to access the probabilities */
  DVEC   *dvec;                 /* to traverse the distrib. vectors */
  INST   *inst;                 /* to traverse the instances */
  NORMD  *nd;                   /* normal distribution */

  p = nbc->priors;              /* get the prior probabilities */
  t = drand();                  /* generate a random number */
  for (sum = i = 0; i < nbc->clscnt; i++) {
    sum += p[i]; if (sum >= t) break; }
  if (i >= nbc->clscnt)         /* find the class that corresponds */
    i = nbc->clscnt -1;         /* to the generated random number */
  att_inst(as_att(nbc->attset, nbc->clsid))->i = i;
  for (dvec = nbc->dvecs +(n = nbc->attcnt); --n >= 0; ) {
    if (((--dvec)->type == 0)   /* traverse all attributes */
    ||  (   dvec ->mark <  0))  /* except the class attribute */
      continue;                 /* and all unmarked attributes */
    inst = att_inst(as_att(nbc->attset, n));
    if (dvec->type == AT_SYM) { /* --- if the attribute is symbolic */
      p = dvec->discds[i].probs;/* get the conditional distribution */
      t = drand();              /* generate a random number */
      for (sum = k = 0; k < dvec->valcnt; k++) {
        sum += p[k]; if (sum >= t) break; }
      if (k >= dvec->valcnt)    /* find the value that corresponds */
        k = dvec->valcnt -1;    /* to the generated random number */
      inst->i = k; }            /* and set the attribute instance */
    else {                      /* --- if the attribute is numeric */
      nd = dvec->normds +i;     /* get the conditional distribution */
      t  = sqrt(nd->var) *_normd(drand) +nd->exp;
      if (dvec->type == AT_FLT) inst->f = (float)t;
      else                      inst->i = (int)(t +0.5);
    }                           /* sample from the normal distrib. */
  }                             /* and transform the result */
}  /* nbc_rand() */

/*--------------------------------------------------------------------*/

int nbc_desc (NBC *nbc, FILE *file, int mode, int maxlen)
{                               /* --- describe a naive Bayes class. */
  int         i, k, n;          /* loop variables */
  int         pos, ind;         /* current position and indentation */
  int         len, l;           /* length of class/value name/number */
  const char  *clsname;         /* name of class attribute */
  ATT         *att, *clsatt;    /* to traverse the attributes */
  const DVEC  *dvec;            /* to traverse the distrib. vectors */
  const NORMD *normd;           /* to traverse the normal   distribs. */
  const DISCD *discd;           /* to traverse the discrete distribs. */
  char  name[4*AS_MAXLEN+4];    /* output buffer for names */
  char  num[64];                /* output buffer for numbers */

  assert(nbc && file);          /* check the function arguments */

  /* --- print a header (as a comment) --- */
  if (mode & NBC_TITLE) {       /* if the title flag is set */
    i = k = (maxlen > 0) ? maxlen -2 : 70;
    fputs("/*", file); while (--i >= 0) fputc('-', file);
    fputs("\n  naive Bayes classifier\n", file);
    while (--k >= 0) fputc('-', file); fputs("*/\n", file);
  }                             /* print a title header */
  if (maxlen <= 0) maxlen = INT_MAX;

  /* --- start description --- */
  clsatt  = as_att(nbc->attset, nbc->clsid);
  clsname = att_name(clsatt);   /* note the class attribute name */
  fputs("nbc(", file);          /* (is needed repeatedly below) */
  sc_format(name, clsname, 0);  /* format and print */
  fputs(name, file);            /* the class attribute name */
  fputs(") = {\n", file);       /* and start the description */
  if ((nbc->lcorr > 0)          /* if estimation parameters */
  ||   nbc->mode) {             /* differ from default values */
    fprintf(file, "  params = %g", nbc->lcorr);
    if (nbc->mode & NBC_DISTUV) fputs(", distuv", file);
    if (nbc->mode & NBC_MAXLLH) fputs(", maxllh", file);
    fputs(";\n", file);         /* print Laplace correction */
  }                             /* and estimation mode */

  /* --- print the class distribution --- */
  fputs("  prob(", file);       /* print a distribution indicator */
  fputs(name, file);            /* and the class attribute name and */
  fputs(") = {\n    ", file);   /* start the class distribution */
  pos = 4;                      /* initialize the output position */
  ind = att_valwd(clsatt,0) +2; /* compute position and indentation */
  for (i = 0; i < nbc->clscnt; i++) {   /* traverse the classes */
    if (i > 0)                  /* if this is not the first class, */
      fputs(",\n    ", file);   /* start a new output line */
    len = sc_format(name, att_valname(clsatt, i), 0);
    fputs(name, file);          /* get and print the class name */
    for (pos = len+2; pos < ind; pos++)
      putc(' ', file);          /* pad with blanks to equal width */
    fprintf(file, ": %g", nbc->frqs[i]);
    if (mode & NBC_REL)         /* print the absolute class frequency */
      fprintf(file, " (%.1f%%)", nbc->priors[i] *100);
  }                             /* print the relative class frequency */
  fputs(" };\n", file);         /* terminate the class distribution */

  /* --- print the (conditional) distributions --- */
  for (dvec = nbc->dvecs, n = 0; n < nbc->attcnt; dvec++, n++) {
    if ((dvec->type == 0)       /* traverse all attributes, */
    ||  ((mode & NBC_MARKED)    /* but skip the class attribute */
    &&   (dvec->mark < 0)))     /* and in marked mode also */
      continue;                 /* all unmarked attributes */
    fputs("  prob(", file);     /* print a distribution indicator */
    att = as_att(nbc->attset,n);/* and get the next attribute */
    sc_format(name, att_name(att), 0);
    fputs(name, file);          /* print the attribute name */
    putc('|', file);            /* and the condition separator */
    sc_format(name, clsname,0); /* format and print */
    fputs(name, file);          /* the class attribute name */
    fputs(") = {\n    ", file); /* and start the cond. distribution */
    if (dvec->discds) {         /* if the attribute is symbolic, */
      discd = dvec->discds;     /* traverse the discrete distribs. */
      for (i = 0; i < nbc->clscnt; discd++, i++) {
        if (i > 0)              /* if this is not the first class, */
          fputs(",\n    ", file);       /* start a new output line */
        len = sc_format(name, att_valname(clsatt, i), 0);
        fputs(name, file);      /* get and print the class name */
        for (pos = len+2; pos < ind; pos++)
          putc(' ', file);      /* pad with blanks to equal width */
        fputs(":{", file);      /* start the value distribution and */
        pos += 3;               /* traverse the attribute values */
        for (k = 0; k < dvec->valcnt; k++) {
          if (k > 0) {          /* if this is not the first value, */
            putc(',', file); pos++; }         /* print a separator */
          len  = sc_format(name, att_valname(att, k), 0);
          len += l = sprintf(num, ": %g", discd->frqs[k]);
          if (mode & NBC_REL)   /* format value frequency */
            len += sprintf(num +l, " (%.1f%%)", discd->probs[k]*100);
          if ((pos      > ind)  /* if the line would get too long */
          &&  (pos +len > maxlen -4)) {
            putc('\n', file);   /* start a new line and indent */
            for (pos = 0; pos < ind; pos++) putc(' ', file); }
          else {                /* if there is enough space left, */
            putc(' ', file); pos++; }   /* only print a separator */
          fputs(name, file); fputs(num, file);
          pos += len;           /* print value and its frequency */
        }                       /* and update the output position */
        fputs(" }", file);      /* terminate the value distribution */
      } }
    else {                      /* if the attribute is numeric, */
      normd = dvec->normds;     /* traverse the normal distributions */
      for (i = 0; i < nbc->clscnt; normd++, i++) {
        if (i > 0)              /* if this is not the first class, */
          fputs(",\n    ", file);       /* start a new output line */
        len = sc_format(name, att_valname(clsatt, i), 0);
        fputs(name, file);      /* get and print the class name */
        for (pos = len+2; pos < ind; pos++)
          putc(' ', file);      /* pad with blanks to equal width */
        fprintf(file, ": N(%g, %g) [%g]",
                normd->exp, normd->var, normd->cnt);
      }                         /* print the normal distribution */
      putc(' ', file);          /* with expected value and variance */
    }  /* if (dvec->discds) .. else .. */
    fputs("};\n", file);        /* terminate the distributions */
  }  /* for (n = 0; .. */
  fputs("};\n", file);          /* terminate the classifier */

  return ferror(file) ? -1 : 0; /* return the write status */
}  /* nbc_desc() */

/*--------------------------------------------------------------------*/
#ifdef NBC_PARSE

static int _paerr (SCAN *scan, int code, int c, const char *s)
{                               /* --- report a parse error */
  char src[  AS_MAXLEN+1];      /* buffer for string to format */
  char dst[4*AS_MAXLEN+4];      /* buffer for fomatted string */

  assert(scan);                 /* check the function arguments */
  if ((code == E_DUPVAL) || (code == E_MISATT))
    sc_format(dst, s,   1);     /* if attribute error message, */
  else {                        /* format the given name */
    strncpy(src, sc_value(scan), AS_MAXLEN); src[AS_MAXLEN] = '\0';
    sc_format(dst, src, 1);     /* if normal error message, */
  }                             /* copy and format the token value */
  if      (code == E_CHREXP) return sc_error(scan, code, c, dst);
  else if (code == E_STREXP) return sc_error(scan, code, s, dst);
  else                       return sc_error(scan, code,    dst);
}  /* _paerr() */               /* print an error message */

/*--------------------------------------------------------------------*/

static int _distin (SCAN *scan, ATT *att, double *frqs, double *sum)
{                               /* --- read a distribution */
  int    i, cnt;                /* loop variable, number of values */
  double *p, f;                 /* to traverse the frequencies */
  int    t;                     /* buffer for token */

  assert(scan && att && frqs && sum); /* check the function arguments */
  GET_CHR('{');                 /* consume '{' (start of distrib.) */
  cnt = att_valcnt(att);        /* get the number of att. values */
  for (p = frqs +(i = cnt); --i >= 0; )
    *--p = -1;                  /* clear the value frequencies */
  while (1) {                   /* attribute value read loop */
    t = sc_token(scan);         /* check for a name */
    if ((t != T_ID) && (t != T_NUM)) ERROR(E_VALEXP);
    if (t != T_NUM) t = ':';    /* if the token is no number, */
    else {                      /* the token must be an att. value, */
      GET_TOK();                /* otherwise consume the token, */
      t = sc_token(scan);       /* note the next token, and */
      sc_back(scan);            /* go back to the previous one */
    }                           /* (look ahead one token) */
    if (t != ':')               /* if no ':' follows, */
      i = (i+1) % cnt;          /* get the cyclic successor id */
    else {                      /* if a  ':' follows */
      i = att_valid(att, sc_value(scan));
      if (i < 0) ERROR(E_UNKVAL);
      GET_TOK();                /* get and consume the value */
      GET_CHR(':');             /* consume ':' */
    }
    if (frqs[i] >= 0)           /* check whether value has been read */
      XERROR(E_DUPVAL, att_valname(att, i));
    if (sc_token(scan) != T_NUM) ERROR(E_NUMEXP);
    f = atof(sc_value(scan));   /* get and check */
    if (f < 0) ERROR(E_ILLNUM); /* the value frequency */
    frqs[i] = f;                /* set the value frequency */
    GET_TOK();                  /* consume the value frequency */
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
  for (f = 0, p = frqs +(i = cnt); --i >= 0; ) {
    if (*--p < 0) *p = 0;       /* clear the unset frequencies */
    else          f += *p;      /* and sum all other frequencies */
  }                             /* to obtain the total frequency */
  *sum = f;                     /* set the sum of the frequencies */
  return 0;                     /* return 'ok' */
}  /* _distin() */

/*--------------------------------------------------------------------*/

static int _discdin (NBC *nbc, SCAN *scan,
                     ATT *clsatt, ATT *att, DVEC *dvec)
{                               /* --- read discrete distributions */
  int   i = -1, t;              /* class identifier, buffer */
  DISCD *discd;                 /* to access discrete distribution */

  assert(nbc && clsatt && att && dvec); /* check function arguments */
  for (discd = dvec->discds +(i = nbc->clscnt); --i >= 0; )
    (--discd)->cnt = -1;        /* unmark all distributions */
  while (1) {                   /* distribution read loop */
    if (sc_token(scan) == '{')  /* if no class name is given, */
      i = (i+1) % nbc->clscnt;  /* get the cyclic successor */
    else {                      /* if a class name is given, */
      t = sc_token(scan);       /* check for a name */
      if ((t != T_ID) && (t != T_NUM)) ERROR(E_VALEXP);
      i = att_valid(clsatt, sc_value(scan));
      if (i < 0) ERROR(E_UNKVAL);
      GET_TOK();                /* get and consume the value */
      GET_CHR(':');             /* consume ':' */
    }
    discd = dvec->discds +i;    /* get and check the distribution */
    if (discd->cnt >= 0) XERROR(E_DUPVAL, att_valname(clsatt, i));
    discd->cnt = 0;             /* clear the counter as a flag */
    t = _distin(scan, att, discd->frqs, &discd->cnt);
    if (t) return t;            /* read distribution */
    if (sc_token(scan) != ',') break;
    GET_TOK();                  /* if at end of list, abort loop */
  }                             /* otherwise consume ',' */
  for (discd = dvec->discds +(i = nbc->clscnt); --i >= 0; )
    if ((--discd)->cnt < 0) discd->cnt = 0;
                                /* clear the unset counters */
  return 0;                     /* return 'ok' */
}  /* _discdin() */

/*--------------------------------------------------------------------*/

static int _contdin (NBC *nbc, SCAN *scan,
                     ATT *clsatt, ATT *att, DVEC *dvec)
{                               /* --- read continuous distributions */
  int    i = -1;                /* class identifier, buffer */
  NORMD  *normd;                /* to access normal distribution */
  double t;                     /* temporary buffer */

  assert(nbc && clsatt && att && dvec); /* check function arguments */
  for (normd = dvec->normds +(i = nbc->clscnt); --i >= 0; )
    (--normd)->cnt = -1;        /* unmark all distributions */
  while (1) {                   /* distribution read loop */
    t = sc_token(scan);         /* check for a name */
    if ((t != T_ID) && (t != T_NUM)) ERROR(E_VALEXP);
    if (t == T_NUM) t = ':';    /* if the token is a number, */
    else {                      /* the token must be a class */
      GET_TOK();                /* otherwise consume the token, */
      t = sc_token(scan);       /* note the next token, and */
      sc_back(scan);            /* go back to the previous one */
    }                           /* (look ahead one token) */
    if (t != ':')               /* if no class name is given, */
      i = (i+1) % nbc->clscnt;  /* get the cyclic successor id */
    else {                      /* if a  class name is given */
      i = att_valid(clsatt, sc_value(scan));
      if (i < 0) ERROR(E_UNKVAL);
      GET_TOK();                /* get and consume the class */
      GET_CHR(':');             /* consume ':' */
    }
    normd = dvec->normds +i;    /* get the normal distribution and */
    if (normd->cnt >= 0)        /* check whether it is already set */
      XERROR(E_DUPVAL, att_valname(clsatt, i));
    normd->cnt = 0;             /* clear the counter as a flag */
    if ((sc_token(scan) != T_ID)
    ||  (strcmp(sc_value(scan), "N") != 0))
      ERR_STR("N");             /* check for an 'N' */
    GET_TOK();                  /* consume 'N' */
    GET_CHR('(');               /* consume '(' */
    if (sc_token(scan) != T_NUM) ERROR(E_NUMEXP);
    normd->exp = atof(sc_value(scan));
    GET_TOK();                  /* get and consume the exp. value */
    GET_CHR(',');               /* consume ',' */
    if (sc_token(scan) != T_NUM) ERROR(E_NUMEXP);
    normd->var = atof(sc_value(scan));
    if (normd->var < 0)          ERROR(E_ILLNUM);
    GET_TOK();                  /* get and consume the variance */
    GET_CHR(')');               /* consume ')' */
    if (sc_token(scan) != '['){ /* if no number of cases follows, */
      normd->cnt = nbc->frqs[i];/* get the class frequencies */
      if (normd->cnt <= 1) normd->cnt = 2; }
    else {                      /* if a number of cases follows, */
      GET_TOK();                /* consume '[' and */
      if (sc_token(scan) != T_NUM) ERROR(E_NUMEXP);
      normd->cnt = atof(sc_value(scan));
      if (normd->cnt < 0)          ERROR(E_ILLNUM);
      GET_TOK();                /* consume the number of cases */
      GET_CHR(']');             /* consume ']' */
    }                           /* then compute the sums */
    normd->sv  = normd->exp *(t = normd->cnt);
    if (!(nbc->mode & NBC_MAXLLH)) t -= 1;
    normd->sv2 = normd->var *t +normd->exp *normd->sv;
    if (sc_token(scan) != ',') break;
    GET_TOK();                  /* if at end of list, abort loop, */
  }                             /* otherwise consume ',' */
  for (normd = dvec->normds +(i = nbc->clscnt); --i >= 0; )
    if ((--normd)->cnt < 0) normd->cnt = 0;
                                /* clear the unset counters */
  return 0;                     /* return 'ok' */
}  /* _contdin() */

/*--------------------------------------------------------------------*/

static int _dvecsin (NBC *nbc, SCAN *scan, ATT *clsatt)
{                               /* --- read distribution vectors */
  int  t;                       /* temporary buffer */
  int  attid;                   /* attribute identifier */
  ATT  *att;                    /* current attribute */
  DVEC *dvec;                   /* to traverse the distrib. vectors */

  assert(nbc && scan && clsatt);   /* check the function arguments */
  while ((sc_token(scan) == T_ID)  /* while another dist. follows */
  &&     ((strcmp(sc_value(scan), "prob") == 0)
  ||      (strcmp(sc_value(scan), "P")    == 0))) {
    GET_TOK();                  /* consume 'prob' or 'P' */
    GET_CHR('(');               /* consume '(' */
    t = sc_token(scan);         /* check for a name */
    if ((t != T_ID) && (t != T_NUM)) ERROR(E_ATTEXP);
    attid = as_attid(nbc->attset, sc_value(scan));
    if (attid < 0)                   ERROR(E_UNKATT);
    att  = as_att(nbc->attset, attid);
    dvec = nbc->dvecs +attid;   /* get and check the attribute */
    if (dvec->type == 0) ERROR(E_ILLATT);
    if (dvec->mark >= 0) ERROR(E_DUPATT);
    dvec->mark = 1;             /* set the read flag */
    GET_TOK();                  /* consume the attribute name */
    GET_CHR('|');               /* consume '|' (condition indicator) */
    t = sc_token(scan);         /* get the next token */
    if (((t != T_ID) && (t != T_NUM))
    ||  (strcmp(sc_value(scan), att_name(clsatt)) != 0))
      ERROR(E_CLSEXP);          /* check for the class att. name */
    GET_TOK();                  /* consume the class att. name */
    GET_CHR(')');               /* consume ')' */
    GET_CHR('=');               /* consume '=' */
    GET_CHR('{');               /* consume '{' */
    if (sc_token(scan) != '}'){ /* if a distribution vector follows */
      t = (dvec->type == AT_SYM)
        ? _discdin(nbc, scan, clsatt, att, dvec)
        : _contdin(nbc, scan, clsatt, att, dvec);
      if (t) return t;          /* read conditional distributions */
    }                           /* and check for an error */
    GET_CHR('}');               /* consume '}' */
    GET_CHR(';');               /* consume ';' */
  }  /* while ((sc_token == T_ID) .. */

  return 0;                     /* return 'ok' */
}  /* _dvecsin() */

/*--------------------------------------------------------------------*/

static int _parse (ATTSET *attset, SCAN *scan, NBC **pnbc)
{                               /* --- parse a naive Bayes classifier */
  int  i, t;                    /* loop variable, buffer */
  int  err = 0;                 /* error flag */
  int  clsid;                   /* class attribute index */
  ATT  *att;                    /* class attribute */
  NBC  *nbc;                    /* created naive Bayes classifier */
  DVEC *dvec;                   /* to traverse the distrib. vectors */

  /* --- read header --- */
  if ((sc_token(scan) != T_ID)
  ||  (strcmp(sc_value(scan), "nbc") != 0))
    ERR_STR("nbc");             /* check for 'nbc' */
  GET_TOK();                    /* consume 'nbc' */
  GET_CHR('(');                 /* consume '(' */
  t = sc_token(scan);           /* check for a name */
  if ((t != T_ID) && (t != T_NUM)) ERROR(E_ATTEXP);
  clsid = as_attid(attset, sc_value(scan));
  if (clsid < 0)                   ERROR(E_UNKATT);
  att = as_att(attset, clsid);  /* get and check the class attribute */
  if (att_type(att) != AT_SYM)     ERROR(E_CLSTYPE);
  if (att_valcnt(att) < 1)         ERROR(E_CLSCNT);
  *pnbc = nbc = nbc_create(attset, clsid);
  if (!nbc) ERROR(E_NOMEM);     /* create a naive Bayes classifier */
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
    nbc->lcorr = atof(sc_value(scan));
    if (nbc->lcorr < 0)           ERROR(E_ILLNUM);
    GET_TOK();                  /* get Laplace correction */
    while (sc_token(scan) == ',') {
      GET_TOK();                /* read list of parameters */
      if (sc_token(scan) != T_ID) ERROR(E_PAREXP);
      if      (strcmp(sc_value(scan), "distuv") == 0)
        nbc->mode |= NBC_DISTUV;/* distribute weight for unknowns */
      else if (strcmp(sc_value(scan), "maxllh") == 0)
        nbc->mode |= NBC_MAXLLH;/* use max. likelihood estimate */
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
    ERROR(E_CLSEXP);            /* check for a class name */
  GET_TOK();                    /* consume the class name */
  GET_CHR(')');                 /* consume ')' */
  GET_CHR('=');                 /* consume '=' */
  t = _distin(scan, att, nbc->frqs, &nbc->total);
  if (t) return t;              /* read the class distribution */
  GET_CHR(';');                 /* consume ';' */
  nbc->dvecs[clsid].mark = 0;   /* mark the class attribute */

  /* --- read conditional distributions --- */
  do {                          /* read the distributions, */
    t = _dvecsin(nbc,scan,att); /* trying to recover on errors */
    if (t) { err = t; sc_recover(scan, ';', 0, 0, 0); }
  } while (t);                  /* while not all dists. read */
  if (err) return err;          /* if an error occurred, abort */
  for (dvec = nbc->dvecs, i = 0; i < nbc->attcnt; dvec++, i++) {
    if ((dvec->mark < 0) && (nbc->frqs[i] > 0))
      XERROR(E_MISATT, att_name(as_att(attset, i)));
  }                             /* check for a complete classifier */

  GET_CHR('}');                 /* consume '}' */
  GET_CHR(';');                 /* consume ';' */
  return 0;                     /* return 'ok' */
}  /* _parse() */

/*--------------------------------------------------------------------*/

NBC* nbc_parse (ATTSET *attset, SCAN *scan)
{                               /* --- parse a naive Bayes class. */
  NBC *nbc = NULL;              /* created naive Bayes classifier */

  assert(attset && scan);       /* check the function arguments */
  sc_errmsgs(scan, errmsgs, MSGCNT);
  if (_parse(attset, scan, &nbc) != 0) {
    if (nbc) nbc_delete(nbc,0); /* parse a naive Bayes classifier */
    return NULL;                /* if an error occurred, */
  }                             /* delete the classifier and abort */
  nbc_setup(nbc, nbc->mode, nbc->lcorr);
  return nbc;                   /* set up the created classifier */
}  /* nbc_parse() */            /* and then return it */

#endif
