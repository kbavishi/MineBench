/*----------------------------------------------------------------------
  File    : attmap.h
  Contents: attribute map management (for numeric coding)
  Author  : Christian Borgelt
  History : 11.08.2003 file created
            12.08.2003 function am_type added
----------------------------------------------------------------------*/
#ifndef __ATTMAP__
#define __ATTMAP__
#include "table.h"

/*----------------------------------------------------------------------
  Type Definitions
----------------------------------------------------------------------*/
typedef struct {                /* --- attribute map element --- */
  ATT    *att;                  /* attribute to map */
  int    type;                  /* attribute type indicator */
  int    off;                   /* offset to the first dimension */
} AMEL;                         /* (attribute map element) */

typedef struct {                /* --- attribute map --- */
  ATTSET *attset;               /* underlying attribute set */
  int    attcnt;                /* number of attributes */
  int    dim;                   /* number of output dimensions */
  AMEL   amels[1];              /* attribute map elements */
} ATTMAP;                       /* (attribute map) */

/*----------------------------------------------------------------------
  Functions
----------------------------------------------------------------------*/
extern ATTMAP* am_create (ATTSET *attset, int marked);
extern void    am_delete (ATTMAP *map);
extern ATTSET* am_attset (ATTMAP *map);
extern int     am_attcnt (ATTMAP *map);
extern int     am_dim    (ATTMAP *map);
extern int     am_type   (ATTMAP *map, int attid);
extern int     am_off    (ATTMAP *map, int attid);
extern int     am_cnt    (ATTMAP *map, int attid);
extern void    am_exec   (ATTMAP *map, const TUPLE *tpl, double *vec);

/*----------------------------------------------------------------------
  Preprocessor Definitions
----------------------------------------------------------------------*/
#define am_attset(m)     ((m)->attset)
#define am_attcnt(m)     ((m)->attcnt)
#define am_dim(m)        ((m)->dim)
#define am_off(m,i)      ((m)->amels[i].off)
#define am_type(m,i)     ((m)->amels[i].type)

#endif
