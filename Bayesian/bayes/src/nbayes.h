/*----------------------------------------------------------------------
  File    : nbayes.h
  Contents: Naive Bayes classifier management
  Author  : Christian Borgelt
  History : 07.12.1998 file created
            16.12.1998 definition of type DVEC changed
            13.02.1999 tuple parameters added to nbc_add and nbc_exec
            10.03.1999 definition of NBC_MARKED added
            25.03.1999 definition of NBC_DISTUV added
            27.03.1999 some enquiry functions added
            05.04.1999 Laplace correction parameter added
            23.04.1999 parameter 'mode' added to function nbc_parse
            15.05.1999 function nbc_mark added
            13.11.2000 parameter 'dupas' added to function nbc_dup
            18.11.2000 function nbc_setup added, nbc_exec adapted
            21.11.2000 functions nbc_lcorr and nbc_mode added
            16.07.2001 adapted to modified module scan
            26.04.2003 function nbc_rand added
----------------------------------------------------------------------*/
#ifndef __NBAYES__
#define __NBAYES__
#ifdef NBC_PARSE
#ifndef SC_SCAN
#define SC_SCAN
#endif
#include "scan.h"
#endif
#include "table.h"

/*----------------------------------------------------------------------
  Preprocessor Definitions
----------------------------------------------------------------------*/
/* --- induction modes --- */
#define NBC_DUPAS   0x0001      /* duplicate attribute set */
#define NBC_ADD     0x0002      /* greedily add attributes */
#define NBC_REMOVE  0x0004      /* greedily remove attributes */

/* --- setup/induction modes --- */
#define NBC_ALL     0x0010      /* set up for all attributes */
#define NBC_MARKED  0x0020      /* set up only for marked attributes */
#define NBC_DISTUV  0x0040      /* distrib. weight for unknown values */
#define NBC_MAXLLH  0x0080      /* max. likelihood estim. of variance */

/* --- description modes --- */
#define NBC_TITLE   0x0001      /* print a title (as a comment) */
#define NBC_REL     0x0002      /* print relative numbers */

/*----------------------------------------------------------------------
  Type Definitions
----------------------------------------------------------------------*/
typedef struct {                /* --- discrete distribution --- */
  double cnt;                   /* number of cases (total frequency) */
  double *frqs;                 /* value frequency   vector */
  double *probs;                /* value probability vector */
} DISCD;                        /* (discrete distribution) */

typedef struct {                /* --- normal distribution --- */
  double cnt;                   /* number of cases (total frequency) */
  double sv;                    /* sum of values */
  double sv2;                   /* sum of squared values */
  double exp;                   /* expected value */
  double var;                   /* variance */
} NORMD;                        /* (normal distribution) */

typedef struct {                /* --- distribution vector --- */
  int    mark;                  /* whether read or to be processed */
  int    type;                  /* attribute type (0: class) */
  int    valvsz;                /* size of value frequency vectors */
  int    valcnt;                /* number of attribute values */
  DISCD  *discds;               /* vector of discrete distributions */
  NORMD  *normds;               /* vector of normal distributions */
} DVEC;                         /* (distribution vector) */

typedef struct {                /* --- naive Bayes classifier --- */
  ATTSET *attset;               /* underlying attribute set */
  int    attcnt;                /* number of attributes */
  int    clsid;                 /* identifier of class attribute */
  int    clsvsz;                /* size of class dependent vectors */
  int    clscnt;                /* number of classes */
  int    mode;                  /* estimation mode (e.g. NBC_MAXLLH) */
  double lcorr;                 /* Laplace correction */
  double total;                 /* total number of cases */
  double *frqs;                 /* class frequencies */
  double *priors;               /* prior     class probabilities */
  double *posts;                /* posterior class probabilities */
  double *cond;                 /* buffer for conditional probs. */
  DVEC   dvecs[1];              /* vector of distribution vectors */
} NBC;                          /* (naive Bayes classifier) */

/*----------------------------------------------------------------------
  Functions
----------------------------------------------------------------------*/
extern NBC*    nbc_create (ATTSET *attset, int clsid);
extern NBC*    nbc_dup    (NBC *nbc, int dupas);
extern void    nbc_delete (NBC *nbc, int delas);
extern void    nbc_clear  (NBC *nbc);

extern ATTSET* nbc_attset (const NBC *nbc);
extern int     nbc_attcnt (const NBC *nbc);
extern int     nbc_valcnt (const NBC *nbc, int attid);
extern int     nbc_clsid  (const NBC *nbc);
extern int     nbc_clscnt (const NBC *nbc);
extern double  nbc_total  (const NBC *nbc);

#ifdef NBC_INDUCE
extern int     nbc_add    (NBC *nbc, const TUPLE *tpl);
extern NBC*    nbc_induce (TABLE *table, int clsid,
                           int mode, double lcorr);
extern int     nbc_mark   (NBC *nbc);
#endif

extern void    nbc_setup  (NBC *nbc, int mode, double lcorr);
extern double  nbc_lcorr  (const NBC *nbc);
extern int     nbc_mode   (const NBC *nbc);

extern double  nbc_prior  (const NBC *nbc, int clsid);
extern double  nbc_prob   (const NBC *nbc, int clsid, int attid,
                           int valid);
extern double  nbc_exp    (const NBC *nbc, int clsid, int attid);
extern double  nbc_var    (const NBC *nbc, int clsid, int attid);
extern int     nbc_exec   (NBC *nbc, const TUPLE *tpl, double *conf);
extern void    nbc_rand   (NBC *nbc, double drand (void));

extern int     nbc_desc   (NBC *nbc, FILE *file, int mode, int maxlen);
#ifdef NBC_PARSE
extern NBC*    nbc_parse  (ATTSET *attset, SCAN *scan);
#endif

/*----------------------------------------------------------------------
  Preprocessor Definitions
----------------------------------------------------------------------*/
#define nbc_attset(b)       ((b)->attset)
#define nbc_attcnt(b)       ((b)->attcnt)
#define nbc_valcnt(b,a)     ((b)->dvecs[a].valcnt)
#define nbc_clsid(b)        ((b)->clsid)
#define nbc_clscnt(b)       ((b)->clscnt)
#define nbc_total(b)        ((b)->total)

#define nbc_lcorr(b)        ((b)->lcorr)
#define nbc_mode(b)         ((b)->mode)

#define nbc_prior(b,c)      ((b)->priors[c])
#define nbc_prob(b,c,a,v)   ((b)->dvecs[a].discds[c].probs[v])
#define nbc_exp(b,c,a)      ((b)->dvecs[a].normds[c].exp)
#define nbc_var(b,c,a)      ((b)->dvecs[a].normds[c].var)

#endif
