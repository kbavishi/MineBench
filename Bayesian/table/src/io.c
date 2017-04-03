/*----------------------------------------------------------------------
  File    : io.c
  Contents: input/output utility functions for attribute sets and tables
  Authors : Christian Borgelt
  History : 17.04.1999 file created
            14.07.2001 adapted to new function as_err
            15.07.2001 function io_asin removed
            23.07.2001 function msg removed
            16.08.2003 slight changes in error message output
----------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>
#include "io.h"
#ifdef STORAGE
#include "storage.h"
#endif

/*----------------------------------------------------------------------
  Preprocessor Definitions
----------------------------------------------------------------------*/
/* --- error codes --- */
#define OK            0         /* no error */
#define E_NONE        0         /* no error */
#define E_NOMEM     (-1)        /* not enough memory */
#define E_FOPEN     (-2)        /* cannot open file */
#define E_FREAD     (-3)        /* read error on file */
#define E_FWRITE    (-4)        /* write error on file */
#define E_STDIN     (-5)        /* double assignment of stdin */
/* codes  -6 to -15 not used */
/* codes -16 to -20 defined in attset.h */
#define E_UNKNOWN  (-21)        /* unknown error */

/*----------------------------------------------------------------------
  Constants
----------------------------------------------------------------------*/
static const char *errmsgs[] = {   /* error messages */
  /* E_NONE      0 */  "no error\n",
  /* E_NOMEM    -1 */  "not enough memory\n",
  /* E_FOPEN    -2 */  "cannot open file %s\n",
  /* E_FREAD    -3 */  "read error on file %s\n",
  /* E_FWRITE   -4 */  "write error on file %s\n",
  /* E_STDIN    -5 */  "double assignment of standard input\n",
  /*     -6 to -15 */  NULL, NULL, NULL, NULL, NULL,
                       NULL, NULL, NULL, NULL, NULL,
  /* E_VALUE   -16 */  "file %s, record %d: "
                         "illegal value %s in field %d\n",
  /* E_FLDCNT  -17 */  "file %s, record %d: "
                         "%s%d field(s) instead of %d\n",
  /* E_EMPFLD  -18 */  "file %s, record %d: "
                         "empty name%s in field %d\n",
  /* E_DUPFLD  -19 */  "file %s, record %d: "
                         "duplicate field name %s in field %d\n",
  /* E_MISFLD  -20 */  "file %s, record %d: "
                         "missing field %s\n",
  /* E_UNKNOWN -21 */  "unknown error\n"
};

/*----------------------------------------------------------------------
  Auxiliary Functions
----------------------------------------------------------------------*/

int io_error (int code, ...)
{                               /* --- print an error message */
  va_list    args;              /* list of variable arguments */
  const char *msg;              /* error message */

  if ((code > 0) || (code < E_UNKNOWN))
    code = E_UNKNOWN;           /* check the error code and */
  msg = errmsgs[-code];         /* get the error message */
  if (!msg) msg = errmsgs[-E_UNKNOWN];
  if (prgname) fprintf(stderr, "\n%s: ", prgname);
  else         fprintf(stderr, "\n");
  va_start(args, code);         /* get variable arguments */
  vfprintf(stderr, msg, args);  /* print the error message */
  va_end(args);                 /* end argument evaluation */
  return code;                  /* return the error code */
}  /* io_error() */

/*----------------------------------------------------------------------
  Attribute Set Functions
----------------------------------------------------------------------*/

FILE* io_hdr (ATTSET *attset, const char *fn_hdr,
              const char *fn_tab, int flags, int verbose)
{                               /* --- read a table header */
  FILE   *in;                   /* input file to read */
  TFSERR *err;                  /* error information */
  int    r;                     /* buffer for result of as_read */
  assert(attset);               /* check the function arguments */
  if (flags & AS_ATT) {         /* if to use a table header file */
    if      (fn_hdr && *fn_hdr) /* if a proper file name is given, */
      in = fopen(fn_hdr, "rb"); /* open header file for reading */
    else if (fn_tab && *fn_tab){/* if a proper table name is given, */
      in = stdin; fn_hdr = "<stdin>"; }          /* read from stdin */
    else { io_error(E_STDIN); return NULL; }
    if (verbose) fprintf(stderr, "reading %s ... ", fn_hdr);
    if (!in) { io_error(E_FOPEN, fn_hdr); return NULL; }
    r = as_read(attset, in, flags & ~AS_DFLT);
    if (in != stdin) fclose(in);/* read the table header */
    if (r  != 0) {              /* if an error occurred, */
      err = as_err(attset);     /* get the error information */
      io_error(r, fn_hdr, 1, err->s, err->fld, err->exp);
      return NULL;              /* print an error message */
    }                           /* and abort the function */
    if (verbose) fprintf(stderr, "[%d attribute(s)] done.\n",
                                 as_attcnt(attset));
  }                             /* print a success message */
  if (fn_tab && *fn_tab)        /* if a table file name is given, */
    in = fopen(fn_tab, "rb");   /* open table file for input */
  else {                        /* if no table file is given, */
    in = stdin; fn_tab = "<stdin>"; }      /* read from stdin */
  if (verbose) fprintf(stderr, "reading %s ... ", fn_tab);
  if (!in) { io_error(E_FOPEN, fn_tab); return NULL; }
  if (!(flags & AS_ATT)         /* if not to use a table header file */
  &&  (as_read(attset, in, flags|AS_ATT) != 0)) {
    err = as_err(attset);       /* get the error information */
    io_error(err->code, fn_tab, 1, err->s, err->fld, err->exp);
    if (in != stdin) fclose(in); return NULL;
  }                             /* read att. names from table file */
  return in;                    /* return the file to read table from */
}  /* io_hdr() */

/*--------------------------------------------------------------------*/

int io_body (ATTSET *attset, FILE *in, const char *fn_tab,
             int flags, int verbose)
{                               /* --- traverse a table body */
  int    cnt = 0;               /* number of tuples */
  double wgt = 0;               /* weight of tuples */
  TFSERR *err;                  /* error information */
  int    f;                     /* flags for reading records */
  int    r;                     /* buffer for result of as_read */
  assert(attset && in);         /* check the function arguments */
  if (!fn_tab || !*fn_tab) fn_tab = "<stdin>";
  f = AS_INST | (flags & ~(AS_ATT|AS_DFLT));
  r = ((flags & AS_DFLT) && !(flags & AS_ATT))
    ? 0 : as_read(attset, in, f);
  while (r == 0) {              /* record read loop */
    cnt++;                      /* increment the tuple counter */
    wgt += as_getwgt(attset);   /* and sum the tuple weight */
    r = as_read(attset, in, f); /* try to read the next tuple */
  }                             /* from the table file */
  if (in != stdin) fclose(in);  /* close the input file */
  if (r < 0) {                  /* if an error occurred, */
    err  = as_err(attset);      /* get the error information */
    cnt += (flags & (AS_ATT|AS_DFLT)) ? 1 : 2;
    return io_error(r, fn_tab, cnt, err->s, err->fld, err->exp);
  }                             /* print an error message */
  if (verbose) fprintf(stderr, "[%d/%g tuple(s)] done.\n", cnt, wgt);
  return 0;                     /* return 'ok' */
}  /* io_body() */

/*--------------------------------------------------------------------*/

int io_tab (ATTSET *attset, const char *fn_hdr,
            const char *fn_tab, int flags, int verbose)
{                               /* --- traverse a table */
  FILE *in;                     /* input file to read */

  assert(attset);               /* check the function arguments */
  in = io_hdr(attset, fn_hdr, fn_tab, flags, verbose);
  if (!in) return E_FREAD;      /* read the table header */
  return io_body(attset, in, fn_tab, flags, verbose);
}  /* io_tab() */               /* and  the table body */

/*----------------------------------------------------------------------
  Table Functions
----------------------------------------------------------------------*/
#ifdef TAB_RDWR

TABLE* io_bodyin (ATTSET *attset, FILE *in, const char *fn_tab,
                  int flags, const char *tabname, int verbose)
{                               /* --- read a table body */
  TABLE  *table;                /* created table */
  int    cnt = 0;               /* number of tuples */
  double wgt = 0;               /* weight of tuples */
  TFSERR *err;                  /* error information */
  int    f;                     /* flags for reading records */
  int    r;                     /* buffer for result of as_read */
printf("here\n");
  assert(attset && in);         /* check the function arguments */
  if (!fn_tab || !*fn_tab) fn_tab = "<stdin>";
  table = tab_create(tabname, attset, tpl_delete);
  if (!table) {                 /* create a table */
    if (in != stdin) fclose(in);
    io_error(E_NOMEM); return NULL;
  }
  f = AS_INST | (flags & ~(AS_ATT|AS_DFLT));
  r = ((flags & AS_DFLT) && !(flags & AS_ATT))
    ? 0 : as_read(attset, in, f);
  while (r == 0) {              /* record read loop */
    if (tab_tpladd(table, NULL) != 0) {
      r = E_NOMEM; break; }     /* store the current tuple */
    cnt++;                      /* count the tuple read */
    wgt += as_getwgt(attset);   /* and sum the tuple weight */
    r = as_read(attset, in, f); /* try to read the next tuple */
  }                             /* from the table file */
  if (in != stdin) fclose(in);  /* close the input file */
  if (r < 0) {                  /* if an error occurred, */
    err  = as_err(attset);      /* get the error information */
    cnt += (flags & (AS_ATT|AS_DFLT)) ? 1 : 2;
    io_error(r, fn_tab, cnt, err->s, err->fld, err->exp);
    tab_delete(table, 0); return NULL;
  }                             /* print an error message */
  if (verbose) fprintf(stderr, "[%d/%g tuple(s)] done.\n", cnt, wgt);
  return table;                 /* return the table read */
}  /* io_bodyin() */

/*--------------------------------------------------------------------*/

TABLE* io_tabin (ATTSET *attset, const char *fn_hdr,
                 const char *fn_tab,  int flags,
                 const char *tabname, int verbose)
{                               /* --- read a table from a file */
  FILE *in;                     /* input file to read */

  assert(attset && tabname);    /* check the function arguments */
  in = io_hdr(attset, fn_hdr, fn_tab, flags, verbose);
  if (!in) return NULL;         /* read the table header */
  return io_bodyin(attset, in, fn_tab, flags, tabname, verbose);
}  /* io_tabin() */             /* read the table body */

/*--------------------------------------------------------------------*/

int io_tabout (TABLE *table, const char *fname, int flags, int verbose)
{                               /* --- write a table to a file */  
  int    i, k;                  /* loop variable, buffer */
  int    cnt;                   /* number of tuples */
  ATTSET *attset;               /* attribute set of table */
  FILE   *out;                  /* output file to write */
  double wgt = 0;               /* weight of tuples */

  assert(table);                /* check the function argument */
  attset = tab_attset(table);   /* get the table's attribute set */
  if (fname && *fname)          /* if a output file name is given, */
    out = fopen(fname, "w");    /* open output file for writing */
  else {                        /* if no output file name is given, */
    out = stdout; fname = "<stdout>"; } /* write to standard output */
  if (verbose) fprintf(stderr, "writing %s ... ", fname);
  if (!out) return io_error(E_FOPEN, fname);
  if (flags & AS_ATT) {         /* if to write a table header */
    if (flags & AS_ALIGN)       /* if to align the fields, */
      flags |= AS_ALNHDR;       /* align them to the header */
    if (as_write(attset, out, flags) != 0) {
      if (out != stdout) fclose(out);
      return io_error(E_FWRITE, fname);
    }                           /* write the attribute names */
  }                             /* to the output file */
  flags = AS_INST | (flags & ~(AS_ATT|AS_INST));
  wgt   = 0;                    /* clear weight sum */
  cnt   = tab_tplcnt(table);    /* get the number of tuples */
  for (i = 0; i < cnt; i++) {   /* and traverse the tuples */
    tpl_toas(tab_tpl(table,i)); /* copy tuple to its attribute set */
    if (as_write(attset, out, flags) != 0) {
      i = -1; break; }          /* write the tuple */
    wgt += as_getwgt(attset);   /* to the output file */
  }                             /* and sum its weight */
  k = (out != stdout)           /* if not written to stdout, */
    ? fclose(out) : 0;          /* close the output file */
  if ((i < 0) || (k != 0)) return io_error(E_FWRITE, fname);
  if (verbose) fprintf(stderr, "[%d/%g tuple(s)] done.\n", cnt, wgt);
  return 0;                     /* return 'ok' */
}  /* io_tabout() */

#endif
