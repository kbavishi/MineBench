/*----------------------------------------------------------------------
  File    : fbayes.h
  Contents: Full Bayes classifier management
  Author  : Christian Borgelt
  History : 10.11.2000 file created
            29.11.2000 first version completed
            16.07.2001 adapted to modified module scan
            26.04.2003 function fbc_rand added
----------------------------------------------------------------------*/
#ifndef __FBAYES__
#define __FBAYES__
#ifdef FBC_PARSE
#ifndef SC_SCAN
#define SC_SCAN
#endif
#include "scan.h"
#ifndef MVN_PARSE
#define MVN_PARSE
#endif
#endif
#include "mvnorm.h"
#include "table.h"

/*----------------------------------------------------------------------
  Preprocessor Definitions
----------------------------------------------------------------------*/
/* --- induction modes --- */
#define FBC_DUPAS   0x0001      /* duplicate attribute set */
#define FBC_ADD     0x0010      /* greedily add attributes */
#define FBC_REMOVE  0x0020      /* greedily remove attributes */

/* --- setup/induction modes --- */
#define FBC_MAXLLH  0x0080      /* use max. likelihood est. of var. */

/* --- description modes --- */
#define FBC_TITLE   0x0001      /* print a title (as a comment) */
#define FBC_REL     0x0002      /* print relative numbers */

/*----------------------------------------------------------------------
  Type Definitions
----------------------------------------------------------------------*/
typedef struct {                /* --- attribute identification --- */
  int    id;                    /* attribute identifier */
  int    type;                  /* attribute type */
  ATT    *att;                  /* the attribute itself */
} FBCID;                        /* (attribute identification) */

typedef struct {                /* --- full Bayes classifier --- */
  ATTSET *attset;               /* underlying attribute set */
  int    attcnt;                /* number of attributes */
  int    numcnt;                /* number of numeric attributes */
  FBCID  *numids;               /* identifications of numeric atts. */
  int    clsid;                 /* identifier of class attribute */
  int    clsvsz;                /* size of class dependent vectors */
  int    clscnt;                /* number of classes */
  int    mode;                  /* estimation mode (e.g. FBC_MAXLLH) */
  double lcorr;                 /* Laplace correction */
  double total;                 /* total number of cases */
  double *frqs;                 /* class frequencies */
  double *priors;               /* prior     class probabilities */
  double *posts;                /* posterior class probabilities */
  double *vals;                 /* vector of attribute values */
  MVNORM **mvns;                /* multivariate normal distributions */
  int    flags[1];              /* attribute flags */
} FBC;                          /* (full Bayes classifier) */

/*----------------------------------------------------------------------
  Functions
----------------------------------------------------------------------*/
extern FBC*    fbc_create (ATTSET *attset, int clsid);
extern FBC*    fbc_dup    (const FBC *fbc, int dupas);
extern void    fbc_delete (FBC *fbc, int delas);
extern void    fbc_clear  (FBC *fbc);

extern ATTSET* fbc_attset (const FBC *fbc);
extern int     fbc_attcnt (const FBC *fbc);
extern int     fbc_numcnt (const FBC *fbc);
extern int     fbc_clsid  (const FBC *fbc);
extern int     fbc_clscnt (const FBC *fbc);
extern double  fbc_total  (const FBC *fbc);

#ifdef FBC_INDUCE
extern int     fbc_add    (FBC *fbc, const TUPLE *tpl);
extern FBC*    fbc_induce (TABLE *table, int clsid,
                           int mode, double lcorr);
extern int     fbc_mark   (FBC *fbc);
#endif

extern void    fbc_setup  (FBC *fbc, int mode, double lcorr);
extern double  fbc_lcorr  (const FBC *fbc);
extern int     fbc_mode   (const FBC *fbc);

extern double  fbc_prior  (const FBC *fbc, int clsid);
extern MVNORM* fbc_mvnorm (FBC *fbc, int clsid);
extern int     fbc_exec   (FBC *fbc, const TUPLE *tpl, double *conf);
extern double* fbc_rand   (FBC *fbc, double drand (void));

extern int     fbc_desc   (FBC *fbc, FILE *file, int mode, int maxlen);
#ifdef FBC_PARSE
extern FBC*    fbc_parse  (ATTSET *attset, SCAN *scan);
#endif

/*----------------------------------------------------------------------
  Preprocessor Definitions
----------------------------------------------------------------------*/
#define fbc_attset(b)       ((b)->attset)
#define fbc_attcnt(b)       ((b)->attcnt)
#define fbc_numcnt(b)       ((b)->numcnt)
#define fbc_clsid(b)        ((b)->clsid)
#define fbc_clscnt(b)       ((b)->clscnt)
#define fbc_total(b)        ((b)->total)

#define fbc_lcorr(b)        ((b)->lcorr)
#define fbc_mode(b)         ((b)->mode)

#define fbc_prior(b,c)      ((b)->priors[c])
#define fbc_mvnorm(b,c)     ((b)->mvns[c])

#endif
