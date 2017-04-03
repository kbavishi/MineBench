/*----------------------------------------------------------------------
  File    : attset3.c
  Contents: attribute set management, parser functions
  Author  : Christian Borgelt
  History : 26.10.1995 file created
            10.03.1998 attribute weights added to domain description
            31.05.1998 adapted to scanner changes
            01.09.1998 several assertions added
            06.09.1998 second major redesign completed
            04.02.1999 long int changed to int
            22.11.2000 functions sc_form and sc_len exported
            23.06.2001 module split into two files
            15.07.2001 parser adapted to modified module scan
            22.01.2002 parser functions moved to a separate file
            18.06.2002 full range no longer set for numeric attributes
----------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>
#ifndef AS_PARSE
#define AS_PARSE
#endif
#include "attset.h"
#ifdef STORAGE
#include "storage.h"
#endif

/*----------------------------------------------------------------------
  Preprocessor Definitions
----------------------------------------------------------------------*/
/* --- error codes --- */
#define E_CHREXP    (-16)       /* character expected */
#define E_NUMEXP    (-17)       /* number expected */
#define E_ILLNUM    (-18)       /* illegal number */
#define E_DOMAIN    (-19)       /* illegal attribute domain */
#define E_ATTEXP    (-20)       /* attribute expected */
#define E_UNKATT    (-21)       /* unknown attribute */
#define E_DUPATT    (-22)       /* duplicate attribute */
#define E_VALEXP    (-23)       /* attribute expected */
#define E_DUPVAL    (-24)       /* duplicate attribute value */
#define E_DUPWGT    (-25)       /* duplicate weight */

/* --- functions --- */
#define ERROR(c)    return _paerr(set, scan, c,       -1)
#define ERR_CHR(c)  return _paerr(set, scan, E_CHREXP, c)
#define GET_TOK()   if (sc_next(scan) < 0) \
                      return sc_error(scan, sc_token(scan))
#define GET_CHR(c)  if (sc_token(scan) != (c)) ERR_CHR(c); \
                    else GET_TOK();

/*----------------------------------------------------------------------
  Constants
----------------------------------------------------------------------*/
#ifdef GERMAN                   /* deutsche Texte */
static const char *errmsgs[] = {/* Fehlermeldungen */
  /* E_CHREXP  -16 */  "\"%c\" erwartet statt %s",
  /* E_NUMEXP  -17 */  "Zahl erwartet statt %s",
  /* E_ILLNUM  -18 */  "ungültige Zahl %s",
  /* E_DOMAIN  -19 */  "ungültiger Wertebereich %s",
  /* E_ATTEXP  -20 */  "Attributname erwartet statt %s",
  /* E_UNKATT  -21 */  "unbekanntes Attribut %s",
  /* E_DUPATT  -22 */  "doppeltes Attribut %s",
  /* E_VALEXP  -23 */  "Attributwert erwartet statt %s",
  /* E_DUPVAL  -24 */  "doppelter Attributwert %s",
  /* E_DUPWGT  -25 */  "doppelte Gewichtsangabe für Attribut %s\n",
};
#else                           /* English texts */
static const char *errmsgs[] = {/* error messages */
  /* E_CHREXP  -16 */  "\"%c\" expected instead of %s",
  /* E_NUMEXP  -17 */  "number expected instead of %s",
  /* E_ILLNUM  -18 */  "illegal number %s",
  /* E_DOMAIN  -19 */  "illegal attribute domain %s",
  /* E_ATTEXP  -20 */  "attribute name expected instead of %s",
  /* E_UNKATT  -21 */  "unknown attribute %s",
  /* E_DUPATT  -22 */  "duplicate attribute %s",
  /* E_VALEXP  -23 */  "attribute value expected instead of %s",
  /* E_DUPVAL  -24 */  "duplicate attribute value %s",
  /* E_DUPWGT  -25 */  "duplicate weight for attribute %s\n",
};
#endif
#define MSGCNT  (int)(sizeof(errmsgs)/sizeof(const char*))

/*----------------------------------------------------------------------
  Parser Functions
----------------------------------------------------------------------*/

static int _paerr (ATTSET *set, SCAN *scan, int code, int c)
{                               /* --- report a parse error */
  char src[  AS_MAXLEN+1];      /* buffer for string to format */
  char dst[4*AS_MAXLEN+4];      /* buffer for fomatted string */

  strncpy(src, sc_value(scan), AS_MAXLEN); src[AS_MAXLEN] = '\0';
  sc_format(dst, src, 1);       /* copy and format the token value */
  if (code == E_CHREXP) return sc_error(scan, code, c, dst);
  else                  return sc_error(scan, code,    dst);
}  /* _paerr() */               /* print an error message */

/*--------------------------------------------------------------------*/

static int _domains (ATTSET *set, SCAN *scan, int tflags)
{                               /* --- parse attribute domains */
  ATT        *att;              /* attribute read */
  int        type;              /* attribute type */
  int        t;                 /* temporary buffer */
  const char *v;                /* token value */

  while ((sc_token(scan) == T_ID) /* parse domain definitions */
  &&     ((strcmp(sc_value(scan), "dom")    == 0)
  ||      (strcmp(sc_value(scan), "domain") == 0))) {
    GET_TOK();                  /* consume 'dom' */
    GET_CHR('(');               /* consume '(' */
    t = sc_token(scan);         /* check next token for a valid name */
    if ((t != T_ID) && (t != T_NUM)) ERROR(E_ATTEXP);
    att = att_create(sc_value(scan), AT_SYM);
    if (!att) ERROR(E_NOMEM);   /* create an attribute and */
    t = as_attadd(set, att);    /* add it to the attribute set */
    if (t) { att_delete(att); ERROR((t > 0) ? E_DUPATT : E_NOMEM); }
    GET_TOK();                  /* consume attribute name */
    GET_CHR(')');               /* consume ')' */
    GET_CHR('=');               /* consume '=' */
    type = -1;                  /* init. attribute type to 'none' */
    t = sc_token(scan);         /* test next token */
    if      (t == '{')          /* if a set of values follows, */
      type = tflags & AT_SYM;   /* attribute is symbolic */
    else if (t == T_ID) {       /* if an identifier follows */
      v = sc_value(scan);       /* get it for simpler comparisons */
      if      ((strcmp(v, "ZZ")      == 0)
      ||       (strcmp(v, "Z")       == 0)
      ||       (strcmp(v, "int")     == 0)
      ||       (strcmp(v, "integer") == 0))
        type = tflags & AT_INT; /* attribute is integer-valued */
      else if ((strcmp(v, "IR")      == 0)
      ||       (strcmp(v, "R")       == 0)
      ||       (strcmp(v, "real")    == 0)
      ||       (strcmp(v, "float")   == 0))
        type = tflags & AT_FLT; /* attribute is real-valued */
    }                           /* (get and check attribute type) */
    if (type <= 0) ERROR(E_DOMAIN);
    att->type = type;           /* set attribute type */
    if (type != AT_SYM) {       /* if attribute is numeric */
      GET_TOK();                /* consume type indicator */
      if (type == AT_INT) {     /* if attribute is integer-valued */
        att->min.i =  INT_MAX;  /* initialize minimal */
        att->max.i = -INT_MAX;} /* and maximal value */
      else {                    /* if attribute is real-valued */
        att->min.f =  FLT_MAX;  /* initialize minimal */
        att->max.f = -FLT_MAX;  /* and maximal value */
      }
      if (sc_token(scan) == '[') { /* if a range of values is given */
        GET_TOK();              /* consume '[' */
        if (sc_token(scan) != T_NUM) ERROR(E_NUMEXP);
        if (att_valadd(att, sc_value(scan), NULL) != 0)
          ERROR(E_ILLNUM);      /* get and check lower bound */
        GET_TOK();              /* consume lower bound */
        GET_CHR(',');           /* consume ',' */
        if (sc_token(scan) != T_NUM) ERROR(E_NUMEXP);
        if (att_valadd(att, sc_value(scan), NULL) != 0)
          ERROR(E_ILLNUM);      /* get and check upper bound */
        GET_TOK();              /* consume upper bound */
        GET_CHR(']');           /* consume ']' */
      } }
    else {                      /* if attribute is symbolic */
      GET_CHR('{');             /* consume '{' */
      if (sc_token(scan) != '}') {
        while (1) {             /* read a list of values */
          t = sc_token(scan);   /* check for a name */
          if ((t != T_ID) && (t != T_NUM)) ERROR(E_VALEXP);
          t = att_valadd(att, sc_value(scan), NULL);
          if (t) ERROR((t > 0) ? E_DUPVAL : E_NOMEM);
          GET_TOK();            /* get and consume attribute value */
          if (sc_token(scan) != ',') break;
          GET_TOK();            /* if at end of list, abort loop, */
        }                       /* otherwise consume ',' */
      }
      GET_CHR('}');             /* consume '}' */
    }
    GET_CHR(';');               /* consume ';' */
  }  /* while ((sc_token(scan) == T_ID) .. */
  return 0;                     /* return 'ok' */
}  /* _domains() */

/*--------------------------------------------------------------------*/

static int _weights (ATTSET *set, SCAN *scan)
{                               /* --- parse attribute weights */
  ATT    *att;                  /* to traverse attributes */
  int    attid;                 /* attribute identifier */
  double wgt;                   /* buffer for attribute weight */
  int    t;                     /* temporary buffer */

  while ((sc_token(scan) == T_ID)  /* read weight definitions */
  &&     ((strcmp(sc_value(scan), "wgt")    == 0)
  ||      (strcmp(sc_value(scan), "weight") == 0))) {
    GET_TOK();                  /* consume 'wgt' */
    GET_CHR('(');               /* consume '(' */
    t = sc_token(scan);         /* check next token for a valid name */
    if ((t != T_ID) && (t != T_NUM)) ERROR(E_ATTEXP);
    attid = as_attid(set, sc_value(scan));
    if (attid < 0) ERROR(E_UNKATT); /* get attribute identifier */
    att = set->atts[attid];         /* and the corresp. attribute */
    if (att->read) ERROR(E_DUPWGT);
    att->read = -1;             /* mark and */
    GET_TOK();                  /* consume attribute */
    GET_CHR(')');               /* consume ')' */
    GET_CHR('=');               /* consume '=' */
    if (sc_token(scan) != T_NUM)            ERROR(E_NUMEXP);
    wgt = atof(sc_value(scan)); /* get attribute weight */
    if ((wgt <= UV_FLT) || (wgt > FLT_MAX)) ERROR(E_ILLNUM);
    att->weight = (float)wgt;   /* check and set attribute weight */
    GET_TOK();                  /* and consume the token */
    GET_CHR(';');               /* consume ';' */
  }
  return 0;                     /* return 'ok' */
}  /* _weights() */

/*--------------------------------------------------------------------*/

int as_parse (ATTSET *set, SCAN *scan, int types)
{                               /* --- parse att. set description */
  int i;                        /* loop variable */
  ATT **p;                      /* to traverse attributes */
  int r, err = 0;               /* result of function, error flag */

  assert(set);                  /* check the function argument */
  sc_errmsgs(scan, errmsgs, MSGCNT);
  if (types & AT_ALL) {         /* if at least one type flag is set */
    while (1) {                 /* read loop (with recovery) */
      r = _domains(set, scan, types);   /* read att. domains */
      if (r == 0) break;        /* if no error occurred, abort */
      err = r;                  /* otherwise set the error flag */
      if (r == E_NOMEM) break;  /* always abort on 'out of memory' */
      sc_recover(scan, ';', 0, 0, 0);
    }                           /* otherwise recover from the error */
    if (err) return -1;         /* read domain definitions */
  }                             /* and check for an error */
  if (types & AS_WEIGHT) {      /* if the weight flag is set */
    for (p = set->atts +(i = set->attcnt); --i >= 0; )
      (*--p)->read = 0;         /* clear all read flags */
    while (1) {                 /* read loop (with recovery) */
      r = _weights(set, scan);  /* read attribute weights */
      if (r == 0) break;        /* if no error occurred, abort */
      err = r;                  /* otherwise set the error flag */
      if (r == E_NOMEM) break;  /* always abort on 'out of memory' */
      sc_recover(scan, ';', 0, 0, 0);
    }                           /* otherwise recover from the error */
  }                             /* read attribute weights */
  return err;                   /* return error flag */
}  /* as_parse() */
