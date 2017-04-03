/*----------------------------------------------------------------------
  File    : io.h
  Contents: input/output utility functions for attribute sets and tables
  Authors : Christian Borgelt
  History : 17.04.1999 file created
            14.07.2001 function io_verb replaced by a parameter
            15.07.2001 function io_asin removed
            23.07.2001 function msg removed
----------------------------------------------------------------------*/
#ifndef __IOUTIL__
#define __IOUTIL__
#ifndef AS_RDWR
#define AS_RDWR
#endif
#ifdef TAB_RDWR
#include "table.h"
#else
#include "attset.h"
#endif

/*----------------------------------------------------------------------
  Externals
----------------------------------------------------------------------*/
extern const char *prgname;     /* program name for error messages */
/* This variable, which is used by the error function (io_error)      */
/* of this module, must be defined somewhere in the main program.     */
/* It should be initialized to the name of the program (e.g. by       */
/* assigning argv[0] to it), so that error messages can be qualified. */

/*----------------------------------------------------------------------
  Error Output Function
----------------------------------------------------------------------*/
extern int     io_error  (int code, ...);

/*----------------------------------------------------------------------
  Attribute Set Functions
----------------------------------------------------------------------*/
extern FILE*   io_hdr    (ATTSET *attset, const char *fn_hdr,
                          const char *fn_tab, int flags, int verbose);
extern int     io_body   (ATTSET *attset, FILE *in,
                          const char *fn_tab, int flags, int verbose);
extern int     io_tab    (ATTSET *attset, const char *fn_hdr,
                          const char *fn_tab, int flags, int verbose);

/*----------------------------------------------------------------------
  Table Functions
----------------------------------------------------------------------*/
#ifdef TAB_RDWR
extern TABLE*  io_bodyin (ATTSET *attset, FILE *in, const char *fn_tab,
                          int flags, const char *tabname, int verbose);
extern TABLE*  io_tabin  (ATTSET *attset,
                          const char *fn_hdr, const char *fn_tab,
                          int flags, const char *tabname, int verbose);
extern int     io_tabout (TABLE *table, const char *fname,
                          int flags, int verbose);
#endif
#endif
