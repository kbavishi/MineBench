/*----------------------------------------------------------------------
  File    : transp.c
  Contents: transpose a data matrix
  Author  : Christian Borgelt
  History : 17.10.2001 file created
            16.08.2003 slight changes in error message output
----------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>
#include <math.h>
#include "tfscan.h"
#ifndef MAT_READ
#define MAT_READ
#endif
#include "matrix.h"
#ifdef STORAGE
#include "storage.h"
#endif

/*----------------------------------------------------------------------
  Preprocessor Definitions
----------------------------------------------------------------------*/
#define PRGNAME     "transp"
#define DESCRIPTION "transpose a data matrix"
#define VERSION     "version 1.1 (2003.08.16)         " \
                    "(c) 2001-2003   Christian Borgelt"

/* --- error codes --- */
#define OK            0         /* no error */
#define E_NONE        0         /* no error */
#define E_NOMEM     (-1)        /* not enough memory */
#define E_FOPEN     (-2)        /* file open failed */
#define E_FREAD     (-3)        /* file read failed */
#define E_FWRITE    (-4)        /* file write failed */
#define E_OPTION    (-5)        /* unknown option */
#define E_OPTARG    (-6)        /* missing option argument */
#define E_ARGCNT    (-7)        /* wrong number of arguments */
#define E_UNKNOWN  (-18)        /* unknown error */

/*----------------------------------------------------------------------
  Constants
----------------------------------------------------------------------*/
static const char *errmsgs[] = {   /* error messages */
  /* E_NONE      0 */  "no error\n",
  /* E_NOMEM    -1 */  "not enough memory\n",
  /* E_FOPEN    -2 */  "cannot open file `%s'\n",
  /* E_FREAD    -3 */  "read error on file `%s'\n",
  /* E_FWRITE   -4 */  "write error on file `%s'\n",
  /* E_OPTION   -5 */  "unknown option -%c\n",
  /* E_OPTARG   -6 */  "missing option argument\n",
  /* E_ARGCNT   -7 */  "wrong number of arguments\n",
  /*     -8 to -15 */  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  /* E_VALUE   -16 */  "file %s, record %d: "
                         "illegal value %s in field %d\n",
  /* E_FLDCNT  -17 */  "file %s, record %d: "
                         "%s%d field(s) instead of %d\n",
  /* E_UNKNOWN -18 */  "unknown error\n"
};

/*----------------------------------------------------------------------
  Global Variables
----------------------------------------------------------------------*/
const  char   *prgname = NULL;  /* program name for error messages */
static FILE   *file    = NULL;  /* input/output file */
static TFSCAN *tfscan  = NULL;  /* table file scanner */
static MATRIX *mat     = NULL;  /* data matrix */

/*----------------------------------------------------------------------
  Functions
----------------------------------------------------------------------*/

static void error (int code, ...)
{                               /* --- print error message */
  va_list    args;              /* list of variable arguments */
  const char *msg;              /* error message */

  assert(prgname);              /* check the program name */
  if (code < E_UNKNOWN) code = E_UNKNOWN;
  if (code < 0) {               /* if to report an error, */
    msg = errmsgs[-code];       /* get the error message */
    if (!msg) msg = errmsgs[-E_UNKNOWN];
    fprintf(stderr, "\n%s: ", prgname);
    va_start(args, code);       /* get variable arguments */
    vfprintf(stderr, msg, args);/* print the error message */
    va_end(args);               /* end argument evaluation */
  }
  #ifndef NDEBUG
  if (mat)    mat_delete(mat);     /* clean up memory */
  if (tfscan) tfs_delete(tfscan);  /* and close file */
  if (file && (file != stdin) && (file != stdout)) fclose(file);
  #endif
  #ifdef STORAGE
  showmem("at end of program"); /* check memory usage */
  #endif
  exit(code);                   /* abort the program */
}  /* error() */

/*--------------------------------------------------------------------*/

int main (int argc, char *argv[])
{                               /* --- main function */
  int    i, k = 0;              /* loop variables, counter */
  char   *s;                    /* to traverse options */
  char   **optarg = NULL;       /* option argument */
  char   *fn_in   = NULL;       /* name of file to read */
  char   *fn_out  = NULL;       /* name of file to write */
  char   *blanks  = NULL;       /* blanks */
  char   *fldseps = NULL;       /* field  separators */
  char   *recseps = NULL;       /* record separators */
  char   *fmt     = "%g";       /* format for number output */
  char   seps[4]  = " \n";      /* separators for output */
  int    rowcnt, colcnt;        /* number of rows/columns */
  TFSERR *err;                  /* error information of scanner */

  prgname = argv[0];            /* get program name for error msgs. */

  /* --- print startup/usage message --- */
  if (argc > 1) {               /* if arguments are given */
    fprintf(stderr, "%s - %s\n", argv[0], DESCRIPTION);
    fprintf(stderr, VERSION); } /* print a startup message */
  else {                        /* if no argument given */
    printf("usage: %s [options] infile [outfile]\n", argv[0]);
    printf("%s\n", DESCRIPTION);
    printf("%s\n", VERSION);
    printf("-o#      format for number output (default: %s)\n", fmt);
    printf("-b/f/r#  blank characters, field and record separators\n"
           "         (default: \" \\t\\r\", \" \\t\", \"\\n\")\n");
    printf("infile   matrix file to read (numbers only)\n");
    printf("outfile  file to write transposed matrix to\n");
    return 0;                   /* print a usage message */
  }                             /* and abort the program */

  /* --- evaluate arguments --- */
  for (i = 1; i < argc; i++) {  /* traverse the arguments */
    s = argv[i];                /* get option argument */
    if (optarg) { *optarg = s; optarg = NULL; continue; }
    if ((*s == '-') && *++s) {  /* -- if argument is an option */
      while (1) {               /* traverse characters */
        switch (*s++) {         /* evaluate option */
          case 'o': optarg = &fmt;                  break;
          case 'b': optarg = &blanks;               break;
          case 'f': optarg = &fldseps;              break;
          case 'r': optarg = &recseps;              break;
          default : error(E_OPTION, *--s);          break;
        }                       /* set option variables */
        if (!*s) break;         /* if at end of string, abort loop */
        if (optarg) { *optarg = s; optarg = NULL; break; }
      } }                       /* get option argument */
    else {                      /* -- if argument is no option */
      switch (k++) {            /* evaluate non-option */
        case  0: fn_in  = s;      break;
        case  1: fn_out = s;      break;
        default: error(E_ARGCNT); break;
      }                         /* note filenames */
    }
  }
  if (optarg) error(E_OPTARG);  /* check option argument */
  if ((k != 1) && (k != 2)) error(E_ARGCNT);
  
  /* --- read the matrix file --- */
  if (fn_in && *fn_in)          /* if a file name is given, */
    file = fopen(fn_in, "r");   /* open the file for reading */
  else {                        /* if no file name is given, */
    file = stdin; fn_in = "<stdin>"; }     /* use std. input */
  fprintf(stderr, "\nreading %s ... ", fn_in);
  if (!file) error(E_FOPEN, fn_in);
  tfscan = tfs_create();        /* create a table file scanner and */
  if (!tfscan) error(E_NOMEM);  /* set the separator characters */
  if (blanks)            tfs_chars(tfscan, TFS_BLANK,  blanks);
  if (fldseps) seps[0] = tfs_chars(tfscan, TFS_FLDSEP, fldseps);
  if (recseps) seps[1] = tfs_chars(tfscan, TFS_RECSEP, recseps);
  err = tfs_err(tfscan);        /* get the error information */
  mat = mat_readx(tfscan, file, -1, -1);
  if (!mat) {                   /* read the data matrix */
    if (err->code >= 0) error(E_FREAD, fn_in);
    error(err->code, fn_in, 1, err->s, err->fld, err->exp);
  }
  if (file != stdin) fclose(file);
  file = NULL;                  /* close the input file */
  rowcnt = mat_rowcnt(mat);     /* get the number of rows/columns */
  colcnt = mat_colcnt(mat);     /* and print a success message */
  fprintf(stderr, "[%d row(s), %d column(s)] done.\n", rowcnt, colcnt);

  /* --- write transposed matrix --- */
  if (fn_out && *fn_out)        /* if an output file name is given, */
    file = fopen(fn_out, "w");  /* open the output file */
  else {                        /* if no output file name is given, */
    file = stdout; fn_out = "<stdout>"; }   /* write to std. output */
  fprintf(stderr, "writing %s ... ", fn_out);
  if (!file) error(E_FOPEN, fn_out);
  for (i = 0; i < colcnt; i++){ /* traverse the columns */
    fprintf(file, fmt, mat_get(mat, 0, i));
    for (k = 0; ++k < rowcnt;){ /* traverse the rows */
      fputc(seps[0], file); fprintf(file, fmt, mat_get(mat, k, i)); }
    fputc(seps[1], file);       /* print matrix with */
  }                             /* columns and rows exchanged */
  if ((file         != stdout)  /* if not written to stdout, */
  &&  (fclose(file) != 0))      /* close the output file */
    error(E_FWRITE, fn_out);    /* and check for a write error */
  fprintf(stderr, "done.\n");   /* print a success message */

  /* --- clean up --- */
  #ifndef NDEBUG
  mat_delete(mat);              /* delete the data matrix */
  tfs_delete(tfscan);           /* and the table file scanner */
  #endif
  #ifdef STORAGE
  showmem("at end of program"); /* check memory usage */
  #endif
  return 0;                     /* return `ok' */
}  /* main() */
