/*----------------------------------------------------------------------
  File    : tab4vis.h
  Contents: table utility functions for visualization programs
  Author  : Christian Borgelt
  History : 08.11.2001 file created from file lvq.h
----------------------------------------------------------------------*/
#ifndef __TAB4VIS__
#define __TAB4VIS__
#ifndef AS_RDWR
#define AS_RDWR
#endif
#include "table.h"

/*----------------------------------------------------------------------
  Preprocessor Definitions
----------------------------------------------------------------------*/
#define FMTCHRLEN     80        /* length of format character strings */

/* --- data formats --- */
#define FR_ATTS        0        /* attribute names in first record */
#define FR_DATA        1        /* data tuple in first record */
#define FR_COMMENT     2        /* comment in first record */

/* --- error codes --- */
#define E_NUMCNT     (-5)       /* no numeric attribute */

/*----------------------------------------------------------------------
  Type Definitions
----------------------------------------------------------------------*/
typedef struct {                /* -- data format description -- */
  int    first;                 /* contents of first record */
  char   blanks [FMTCHRLEN+1];  /* blank characters */
  char   fldseps[FMTCHRLEN+1];  /* field separators */
  char   recseps[FMTCHRLEN+1];  /* record separators */
  char   uvchars[FMTCHRLEN+1];  /* unknown value characters */
} DATAFMT;                      /* (data format description) */

typedef struct {                /* --- range of values --- */
  double min, max;              /* minimum and maximum value */
} RANGE;                        /* (range of values) */

typedef struct {                /* -- selected attributes -- */
  int    h_att;                 /* attribute for horizontal direction */
  RANGE  h_rng;                 /* horizontal range of values */
  int    v_att;                 /* attribute for vertical   direction */
  RANGE  v_rng;                 /* vertical range of values */
  int    c_att;                 /* class attribute (if any) */
} SELATT;                       /* (selected attributes) */

/*----------------------------------------------------------------------
  Global Variables
----------------------------------------------------------------------*/
/* --- attribute set variables --- */
extern ATTSET     *attset;      /* attribute set */
extern RANGE      *ranges;      /* ranges of attribute values */
extern const char **nms_num;    /* names of numeric attributes */
extern int        *map_num;     /* map num. attribs. to attset ids. */
extern int        numcnt;       /* number of numeric attributes */
extern const char **nms_sym;    /* names of symbolic attributs */
extern int        *map_sym;     /* map sym. attribs. to attset ids. */
extern int        symcnt;       /* number of symbolic attributes */
extern SELATT     selatt;       /* attribute selection information */

/* --- data table variables --- */
extern TABLE      *table;       /* data table */
extern int        recno;        /* record number for error messages */
extern DATAFMT    datafmt;      /* data format description */

/*----------------------------------------------------------------------
  Functions
----------------------------------------------------------------------*/
extern void tv_clean (void);
extern int  tv_load  (const char *fname, double addfrac);

#endif
