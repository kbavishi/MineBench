/*----------------------------------------------------------------------
  File    : solve.c
  Contents: solve linear equation systems
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
#define PRGNAME     "solve"
#define DESCRIPTION "solve linear equation system"
#define VERSION     "version 1.2 (2003.08.16)         " \
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
#define E_EQCNT     (-8)        /* number of unknowns/equations */
#define E_ALG       (-9)        /* unknown algorithm */
#define E_SING     (-10)        /* singular coefficient matrix */
#define E_POSDEF   (-11)        /* matrix is not positive definite */
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
  /* E_EQCNT    -8 */  "number of unknowns and "
                         "number of equations differ\n",
  /* E_ALG      -9 */  "unknown algorithm %d\n",
  /* E_SING    -10 */  "singular coefficient matrix\n",
  /* E_POSDEF  -11 */  "coefficient matrix is not positive definite\n",
  /*    -12 to -15 */  NULL, NULL, NULL, NULL,
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
static FILE   *in      = NULL;  /* input file */
static TFSCAN *tfscan  = NULL;  /* table file scanner */
static double *vec     = NULL;  /* vector to read equations */
static double *rhs     = NULL;  /* vector of right hand sides */
static MATRIX *mat     = NULL;  /* matrix of coefficients */

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
  if (mat)    mat_delete(mat);
  if (rhs)    free(vec);
  if (vec)    free(vec);           /* clean up memory */
  if (tfscan) tfs_delete(tfscan);  /* and close file */
  if (in && (in != stdin)) fclose(in);
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
  char   *fn_les  = NULL;       /* linear equation system file */
  char   *blanks  = NULL;       /* blanks */
  char   *fldseps = NULL;       /* field  separators */
  char   *recseps = NULL;       /* record separators */
  char   *fmt     = "%g";       /* format for number output */
  int    alg      = 0;          /* algorithm to use */
  int    mode     = MAT_PARTPIV;/* pivoting mode */
  int    cond     = 0;          /* flag for condensed output */
  int    cnt      = 0;          /* number of equations/unknows */
  char   seps[4]  = " \n";      /* separators for output */
  TFSERR *err;                  /* error information of scanner */

  prgname = argv[0];            /* get program name for error msgs. */

  /* --- print startup/usage message --- */
  if (argc > 1) {               /* if arguments are given */
    fprintf(stderr, "%s - %s\n", argv[0], DESCRIPTION);
    fprintf(stderr, VERSION); } /* print a startup message */
  else {                        /* if no argument given */
    printf("usage: %s [options] lesfile\n", argv[0]);
    printf("%s\n", DESCRIPTION);
    printf("%s\n", VERSION);
    printf("-a#      algorithm to use (default: %d)\n", alg);
    printf("         0: Gauss-Jordan elimination\n");
    printf("         1: LU       decomposition\n");
    printf("         2: Cholesky decomposition "
                    "(symmetric positive definite matrices)\n");
    printf("-p       full pivoting (Gauss-Jordan only, "
                    "default: partial pivoting)\n");
    printf("-o#      format for number output (default: %s)\n", fmt);
    printf("-c       condensed output "
                    "(print only a vector of numbers)\n");
    printf("-b/f/r#  blank characters, field and record separators\n"
           "         (default: \" \\t\\r\", \" \\t\", \"\\n\")\n");
    printf("lesfile  linear equation system file\n"
           "         (numbers only, right hand side in last field)\n");
    return 0;                   /* print a usage message */
  }                             /* and abort the program */

  /* --- evaluate arguments --- */
  for (i = 1; i < argc; i++) {  /* traverse the arguments */
    s = argv[i];                /* get option argument */
    if (optarg) { *optarg = s; optarg = NULL; continue; }
    if ((*s == '-') && *++s) {  /* -- if argument is an option */
      while (1) {               /* traverse characters */
        switch (*s++) {         /* evaluate option */
          case 'a': alg    = (int)strtol(s, &s, 0); break;
          case 'p': mode   = MAT_FULLPIV;           break;
          case 'o': optarg = &fmt;                  break;
          case 'c': cond   = 1;                     break;
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
        case  0: fn_les = s;      break;
        default: error(E_ARGCNT); break;
      }                         /* note filenames */
    }
  }
  if (optarg) error(E_OPTARG);  /* check option argument */
  if (k != 1) error(E_ARGCNT);  /* check number of arguments */
  if ((alg < 0) || (alg > 2)) error(E_ALG, alg);
  
  /* --- read the equation system --- */
  if (fn_les && *fn_les)        /* if a file name is given, */
    in = fopen(fn_les, "r");    /* open the file for reading */
  else {                        /* if no file name file is given, */
    fn_les = "<stdin>"; in = stdin; }     /* read from std. input */
  fprintf(stderr, "\nreading %s ... ", fn_les);
  if (!in) error(E_FOPEN, fn_les);
  tfscan = tfs_create();        /* create a table file scanner and */
  if (!tfscan) error(E_NOMEM);  /* set the separator characters */
  if (blanks)            tfs_chars(tfscan, TFS_BLANK,  blanks);
  if (fldseps) seps[0] = tfs_chars(tfscan, TFS_FLDSEP, fldseps);
  if (recseps) seps[1] = tfs_chars(tfscan, TFS_RECSEP, recseps);
  err = tfs_err(tfscan);        /* get the error information */
  vec = vec_readx(tfscan, in, &cnt); /* read the first vector */
  if (!vec || (cnt < 2)) {      /* and check for a read error */
    if (err->code >= 0) error(E_FREAD, fn_les);
    error(err->code, fn_les, 1, err->s, err->fld, err->exp);
  }
  rhs = (double*)malloc(--cnt *sizeof(double));
  mat = mat_create(cnt, cnt);   /* create the coefficient matrix and */
  if (!mat || !rhs) error(E_NOMEM);  /* a vector of right hand sides */
  i = 0;                        /* initialize the equation counter */
  do {                          /* and read the equations */
    if (i >= cnt) error(E_EQCNT);
    mat_rowset(mat, i, vec);    /* copy coefficients to the matrix */
    rhs[i++] = vec[cnt];        /* and set corresp. right hand side */
  } while (vec_read(vec, cnt+1, tfscan, in) == 0);
  if (in != stdin) {            /* if not read from standard input, */
    fclose(in); in = NULL; }    /* close the input file */
  if (err->code < 0)            /* check for a read error */
    error(err->code, fn_les, cnt+1, err->s, err->fld, err->exp);
  if (i != cnt) error(E_EQCNT); /* check the number of equations */
  fprintf(stderr, "[%d equations(s)] done.\n", cnt);

  /* --- solve the equation system --- */
  fprintf(stderr, "solving linear equation system ... ");
  switch (alg) {                /* evaluate the algorithm code */
    case 1:                     /* LU decomposition */
      if (mat_ludecom(mat, mat) != 0) error(E_SING);
      mat_lusubst(rhs, mat, rhs); break;
    case 2:                     /* Cholesky decomposition */
      if (mat_chdecom(mat, mat) != 0) error(E_POSDEF);
      mat_chsubst(rhs, mat, rhs); break;
    default:                    /* Gauss-Jordan elimination */
      if (mat_gjsol(&rhs, mat, &rhs, 1, mode, NULL) != 0)
        error(E_SING);            break;
  }                             /* (solution is in vector rhs) */
  fprintf(stderr, "done.\n");   /* print a success message */

  /* --- print the result --- */
  if (cond) {                   /* if condensed output, */
    printf(fmt, rhs[i-1]);      /* print the vector elements */
    for (i = 0; ++i < cnt; ) {  /* separated by field separators */
      putchar(seps[0]); printf(fmt, rhs[i-1]); }
    putchar(seps[1]); }         /* finally print a record separator */
  else {                        /* if normal output, */
    for (i = 0; ++i <= cnt; ) { /* traverse the result vector */
      printf("x_%d = ", i); printf(fmt, rhs[i-1]); printf("\n"); }
  }                             /* print variable/value pairs */

  /* --- clean up --- */
  #ifndef NDEBUG
  mat_delete(mat);              /* delete the coefficient matrix, */
  free(rhs); free(vec);         /* the right and side vector, and */
  tfs_delete(tfscan);           /* the table file scanner */
  #endif
  #ifdef STORAGE
  showmem("at end of program"); /* check memory usage */
  #endif
  return 0;                     /* return `ok' */
}  /* main() */
