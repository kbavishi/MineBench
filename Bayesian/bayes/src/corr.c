/*----------------------------------------------------------------------
  File    : corr.c
  Contents: covariances/correlation coefficients computation program
  Author  : Christian Borgelt
  History : 07.04.1999 file created from file xmat.c
            10.04.1999 first version completed
            11.04.1999 options -x, -v, and -c added
            14.04.1999 option -k (expected values from known pairs)
            01.12.1999 bug in connection with option -d removed
            10.11.2000 adapted to new module mvnorm
            07.06.2002 TeX output option added
            16.08.2003 slight changes in error message output
----------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <float.h>
#include <math.h>
#include <assert.h>
#include "symtab.h"
#include "tfscan.h"
#include "mvnorm.h"
#ifdef STORAGE
#include "storage.h"
#endif

/*----------------------------------------------------------------------
  Preprocessor Definitions
----------------------------------------------------------------------*/
#define PRGNAME     "corr"
#define DESCRIPTION "compute covariance matrix/correlation coefficients"
#define VERSION     "version 2.5 (2003.08.16)         " \
                    "(c) 1999-2003   Christian Borgelt"

/* --- sizes --- */
#define BUFSIZE     512         /* size of read buffer */
#define BLKSIZE      32         /* block size for column vector */

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
#define E_STDIN     (-8)        /* double assignment of stdin */
#define E_FLDNAME   (-9)        /* illegal field name */
#define E_VALUE    (-10)        /* illegal value */
#define E_EMPFLD   (-11)        /* empty field name */
#define E_DUPFLD   (-12)        /* duplicate field name */
#define E_FLDCNT   (-13)        /* wrong number of fields */
#define E_UNKNOWN  (-14)        /* unknown error */

/*----------------------------------------------------------------------
  Type Definitions
----------------------------------------------------------------------*/
typedef struct {                /* --- set of table columns --- */
  int        colvsz;            /* size of column vector */
  int        colcnt;            /* number of matrix columns */
  int        vldcnt;            /* number of valid matrix columns */
  int        maxlen;            /* maximal length of a column name */
  const char **names;           /* table column names */
  double     *vals;             /* current values */
} COLSET;                       /* (set of table columns) */

/*----------------------------------------------------------------------
  Constants
----------------------------------------------------------------------*/
static const char *errmsgs[] = {   /* error messages */
  /* E_NONE      0 */  "no error\n",
  /* E_NOMEM    -1 */  "not enough memory\n",
  /* E_FOPEN    -2 */  "cannot open file %s\n",
  /* E_FREAD    -3 */  "read error on file %s\n",
  /* E_FWRITE   -4 */  "write error on file %s\n",
  /* E_OPTION   -5 */  "unknown option -%c\n",
  /* E_OPTARG   -6 */  "missing option argument\n",
  /* E_ARGCNT   -7 */  "wrong number of arguments\n",
  /* E_STDIN    -8 */  "double assignment of standard input\n",
  /* E_FLDNAME  -9 */  "illegal field name \"%s\"\n",
  /* E_VALUE   -10 */  "file %s, record %d: "
                         "illegal value \"%s\" in field %d\n",
  /* E_EMPFLD  -11 */  "file %s, record %d: "
                         "empty name%s in field %d\n",
  /* E_DUPFLD  -12 */  "file %s, record %d: "
                         "duplicate field name \"%s\"\n",
  /* E_FLDCNT  -13 */  "file %s, record %d: "
                         "%d field(s) expected\n",
  /* E_UNKNOWN -14 */  "unknown error\n"
};

/*----------------------------------------------------------------------
  Global Variables
----------------------------------------------------------------------*/
static char   *prgname;         /* program name for error messages */
static TFSCAN *tfscan = NULL;   /* table file scanner */
static SYMTAB *symtab = NULL;   /* symbol table */
static COLSET *colset = NULL;   /* column descriptions */
static MVNORM *mvnorm = NULL;   /* multivariate normal distribution */
static FILE   *in     = NULL;   /* input  file */
static FILE   *out    = NULL;   /* output file */
static char   rdbuf[BUFSIZE];   /* read buffer */
static char   fnbuf[BUFSIZE];   /* field name buffer */

/*----------------------------------------------------------------------
  Column Set Functions
----------------------------------------------------------------------*/

#define cs_create()         (COLSET*)calloc(1, sizeof(COLSET))
#define cs_colcnt(s)        ((s)->colcnt)
#define cs_process(s,m,w)   mvn_add(m, (s)->vals, w)

/*--------------------------------------------------------------------*/
#ifndef NDEBUG

static void cs_delete (COLSET *cset)
{                               /* --- delete a column set */
  if (cset->names)              /* if there is a names vector, */
    free((void*)cset->names);   /* delete it */
  if (cset->vals)               /* if there is a value vector, */
    free((void*)cset->vals);    /* delete it */
  free(cset);                   /* delete the column set body */
}  /* cs_delete() */

#endif
/*--------------------------------------------------------------------*/

static int cs_add (COLSET *cset, const char *name)
{                               /* --- add a column to a column set */
  int  n;                       /* new column vector size, buffer */
  void *p;                      /* new names/values vector */

  assert(cset && name);         /* check the function arguments */
  n = cset->colvsz;             /* get the column vector size and */
  if (cset->colcnt >= n) {      /* if the column vector is full */
    n += (n > BLKSIZE) ? (n >> 1) : BLKSIZE;
    p = realloc((void*)cset->names, n *sizeof(const char*));
    if (!p) return -1;          /* enlarge the names vector */
    cset->names = (const char**)p;
    p = realloc(cset->vals,  n *sizeof(double));
    if (!p) return -1;          /* enlarge the names vector */
    cset->vals  = (double*)p; cset->colvsz = n;
  }                             /* set the new vectors and their size */
  cset->names[cset->colcnt  ] = name;
  cset->vals [cset->colcnt++] = MVN_UNKNOWN;
  return 0;                     /* note the table column's name */
}  /* cs_add() */               /* and return 'ok' */

/*--------------------------------------------------------------------*/

static void cs_set (COLSET *cset, int colid, const char *name)
{                               /* --- set a column value */
  char   *s;                    /* end pointer for conversion */
  double *val;                  /* pointer to the value to set */

  assert(cset && (colid >= 0) && (colid < cset->colcnt));
  if (!cset->names[colid])      /* if the column name has been */
    return;                     /* deleted, abort the function */
  val = cset->vals +colid;      /* get the value buffer */
  if (!name) {                  /* if no value name is given, */
    *val = MVN_UNKNOWN; return; }     /* the value is unknown */
  *val = strtod(name, &s);      /* convert the attribute value */
  if (*s || (s == name)) {      /* if it is symbolic */
    *val = MVN_UNKNOWN; cset->names[colid] = NULL; }              
}  /* cs_set() */               /* invalidate the column */

/*--------------------------------------------------------------------*/

static void cs_prepare (COLSET *cset)
{                               /* --- prepare columns for output */
  int        i, len;            /* loop variable, length of a name */
  const char *name;             /* to traverse the column names */

  cset->vldcnt = 0;             /* init. the number of valid columns */
  cset->maxlen = 9;             /* and the maximal length of a name */
  for (i = 0; i < cset->colcnt; i++) {
    name = cset->names[i];      /* traverse the columns */
    if (!name) continue;        /* skip invalidated columns */
    cset->vldcnt++;             /* count the valid columns */
    len = (int)strlen(name);    /* determine the length of the name */
    if (len > cset->maxlen) cset->maxlen = len;
  }                             /* adapt the maximal length */
}  /* cs_prepare() */

/*----------------------------------------------------------------------
  Auxiliary Functions
----------------------------------------------------------------------*/

static void dblout (FILE *out, double num, int len)
{                               /* --- print a floating point number */
  int  n, d;                    /* number of characters/decimals */
  char m[16], e[8];             /* mantissa and exponent */

  if (num >= 0.0) {  m[0] = ' '; }
  else { num = -num; m[0] = '-'; }
  len--;                        /* determine and store the sign */
  n = (int)floor(log10(num));   /* calculate the decimal exponent */
  if ((n > len) || (n <= -3)) { /* if an exponent is needed, */
    num /= pow(10, n);          /* comp. mantissa and note exponent */
    d = len -2 -(n = sprintf(e, "e%d", n)); }
  else {                        /* if no exponent is needed, */
    d = len -((n > 0) ? n+2 : 2);      /* compute the number */
    e[0] = n = 0;               /* of decimal places possible */
  }                             /* and clear the exponent */
  n += sprintf(m+1, "%.*f", (d <= 0) ? 0 : d, num);
  while (n < len) {             /* format the mantissa, */
    fputc(' ', out); n++; }     /* fill to the requested length, */
  fputs(m, out); fputs(e, out); /* and print mantissa and exponent */
}  /* dblout() */

/*----------------------------------------------------------------------
The above function is needed, because a format string used with the
function fprintf does not yield appropriate results.
----------------------------------------------------------------------*/

static int isunk (TFSCAN *tfscan, const char *s)
{                               /* --- test for an unknown value */
  assert(tfscan && s);          /* check arguments */
  while (*s)                    /* while not at end of string */
    if (!tfs_istype(tfscan, TFS_OTHER, *s++))
      return 0;                 /* if a character is not marked, */
  return 1;                     /* return 'not an unknown value', */
}  /* isunk() */                /* otherwise return 'unknown value' */

/*----------------------------------------------------------------------
  Output Functions
----------------------------------------------------------------------*/

static void expvar (COLSET *cset, MVNORM *mvn, int tex, FILE *out)
{                               /* --- print exp. values/variances */
  int        i, k, n = 0;       /* loop variables */
  const char *name;             /* to traverse the column names */
  double     t;                 /* expected value and variance */
  char       *s1, *s2, *s3;     /* separators */
  char       *end;              /* end of line */

  assert(mvn && out);           /* check the function arguments */
  cs_prepare(cset);             /* prepare columns for output */
  if (tex) {                    /* if to generate TeX code */
    fputs("\\begin{tabular}{|r|l|r|r|r|}\\hline\n", out);
    fputs("\\multicolumn{5}{|l|}", out);
    fputs("{attribute statistics\\rule{0pt}{2.4ex}} \\\\\n", out);
    fputs("\\hline\\hline\\rule{0pt}{2.4ex}%\n", out);
    fputs("no & attribute", out);  /* print a header */
    for (k = 9; k <  cset->maxlen; k++) fputc(' ', out);
    fputs(" & exp. val. & variance  & std. dev.", out);
    fputs(" \\\\\n\\hline\\rule{0pt}{2.4ex}%\n", out);
    s1 = " & "; s2 = " &$"; s3 = "$&$"; end = "$ \\\\\n"; }
  else {                        /* if to generate normal text */
    fputs("attribute statistics\n", out);
    fputs("no | attribute", out);  /* print a header */
    for (k = 9; k < cset->maxlen; k++) fputc(' ', out);
    fputs(" | exp. val. | variance  | std. dev.\n---+-", out);
    for (k = 0; k < cset->maxlen; k++) fputc('-', out);
    fputs("-+-----------+-----------+----------\n", out);
    s1 = s2 = s3 = " | "; end = "\n";
  }
  for (i = 0; i < cset->colcnt; i++) {
    name = cset->names[i];      /* traverse the matrix columns, */
    if (!name) continue;        /* but skip invalidated columns */
    fprintf(out, "%2i", ++n);   /* print the column number, */
    fputs(s1,   out);           /* a separator, */
    fputs(name, out);           /* and the column name */
    for (k = (int)strlen(name); k < cset->maxlen; k++)
      fputc(' ', out);          /* fill the output field */
    fputs(s2, out);             /* retrieve and */
    t = mvn_exp(mvn, i);        /* print the expected value */
    if (t <= MVN_UNKNOWN) fputs("        ?", out);
    else                  dblout(out, t,       9);
    t = mvn_var(mvn, i);        /* retrieve and */
    fputs(s3, out);             /* print the variance */
    if (t < 0)            fputs("        ?", out);
    else                  dblout(out, t,       9);
    fputs(s3, out);             /* print the standard deviation */
    if (t < 0)            fputs("        ?", out);
    else                  dblout(out, sqrt(t), 9);
    fputs(end, out);            /* terminate the output line */
  }
  if (tex)                      /* if to generate TeX output */
    fputs("\\hline\n\\end{tabular}\n", out);
}  /* expvar() */

/*--------------------------------------------------------------------*/

static void covar (COLSET *cset, MVNORM *mvn, int tex, FILE *out)
{                               /* --- print covariance matrix */
  int        x, y, n = 0;       /* loop variables */
  const char *xname, *yname;    /* to traverse the column names */
  double     cov;               /* covariance */
  char       *s1, *s2, *s3;     /* separators */
  char       *end;              /* end of line */

  assert(cset && mvn && out);   /* check the function arguments */
  cs_prepare(cset);             /* prepare columns for output */
  if (tex) {                    /* if to generate TeX output */
    fprintf(out, "\\begin{tabular}{|r|l|*{%d}{r|}}", cset->vldcnt);
    fputs("\\hline\n", out);    /* print a header */
    fprintf(out, "\\multicolumn{%d}{|l|}", cset->vldcnt +2);
    fputs("{covariance matrix\\rule{0pt}{2.4ex}} \\\\\n", out);
    fputs("\\hline\\hline\\rule{0pt}{2.4ex}%\n", out);
    fputs("no & attribute", out);
    for (x = 9; x <  cset->maxlen; x++) fputc(' ', out);
    for (x = 1; x <= cset->vldcnt; x++) fprintf(out, " & %8i", x);
    fputs(" \\\\\n\\hline\\rule{0pt}{2.4ex}%\n", out);
    s1 = " & "; s2 = " &"; s3 = "$&$"; end = "$ \\\\\n"; }
  else {                        /* if to generate text output */
    fputs("covariance matrix\nno | attribute", out);
    for (x = 9; x <  cset->maxlen; x++) fputc(' ', out);
    fputs(" |", out);           /* print a header */
    for (x = 1; x <= cset->vldcnt; x++) fprintf(out, " %8i", x);
    fputs("\n---+-", out);
    for (x = 0; x <  cset->maxlen; x++) fputc('-', out);
    fputs("-+", out);
    for (x = 0; x <  cset->vldcnt; x++) fputs("---------", out);
    fputc('\n', out);           /* print a separating line */
    s1 = " | "; s2 = " |"; s3 = " "; end = "\n";
  }
  for (y = 0; y < cset->colcnt; y++) {
    yname = cset->names[y];     /* traverse the matrix columns, */
    if (!yname) continue;       /* but skip invalidated columns */
    fprintf(out, "%2i", ++n);   /* print the column number, */
    fputs(s1,    out);          /* a separator, */
    fputs(yname, out);          /* and the column name */
    for (x = (int)strlen(yname); x < cset->maxlen; x++)
      fputc(' ', out);          /* fill the output field */
    fputs(s2, out);             /* print a separator */
    for (x = 1; x < n; x++) {   /* skip lower left part of matrix */
      fputs("         ", out); if (tex) fputs(s2, out); }
    fputc(s3[0], out);          /* start a number */
    for (x = y; x < cset->colcnt; x++) {
      xname = cset->names[x];   /* traverse the remaining columns, */
      if (!xname) continue;     /* but skip invalidated columns */
      if (x > y) fputs(s3,out); /* print a separator */
      cov = mvn_cov(mvn, x, y); /* get and print the covariance */
      if (cov <= MVN_UNKNOWN) fputs("      ?", out);
      else                    dblout(out, cov, 8);
    }
    fputs(end, out);            /* terminate the output line */
  }
  if (tex)                      /* if to generate TeX output */
    fputs("\\hline\n\\end{tabular}\n", out);
}  /* covar() */

/*--------------------------------------------------------------------*/

static void correl (COLSET *cset, MVNORM *mvn, int tex, FILE *out)
{                               /* --- print correlation coefficients */
  int        x, y, n = 0;       /* loop variables */
  const char *xname, *yname;    /* to traverse the column names */
  double     r, t;              /* correlation coefficient, buffer */ 
  char       *s1, *s2, *s3;     /* separators */
  char       *end;              /* end of line */

  assert(cset && mvn && out);   /* check the function arguments */
  cs_prepare(cset);             /* prepare columns for output */
  if (tex) {                    /* if to generate TeX output */
    fprintf(out, "\\begin{tabular}{|r|l|*{%d}{r|}}", cset->vldcnt);
    fputs("\\hline\n", out);    /* print a header */
    fprintf(out, "\\multicolumn{%d}{|l|}", cset->vldcnt +2);
    fputs("{correlation coefficients\\rule{0pt}{2.4ex}} \\\\\n", out);
    fputs("\\hline\\hline\\rule{0pt}{2.4ex}%\n", out);
    fputs("no & attribute", out);
    for (x = 9; x <  cset->maxlen; x++) fputc(' ', out);
    for (x = 1; x <= cset->vldcnt; x++) fprintf(out, " & %5i", x);
    fputs(" \\\\\n\\hline\\rule{0pt}{2.4ex}%\n", out);
    s1 = " & "; s2 = " &"; s3 = "$&$"; end = "$ \\\\\n"; }
  else {                        /* if to generate text output */
    fputs("correlation coefficients\nno | attribute", out);
    for (x = 9; x <  cset->maxlen; x++) fputc(' ', out);
    fputs(" |", out);           /* print a header */
    for (x = 1; x <= cset->vldcnt; x++) fprintf(out, " %5i", x);
    fputs("\n---+-", out);
    for (x = 0; x <  cset->maxlen; x++) fputc('-', out);
    fputs("-+", out);
    for (x = 0; x <  cset->vldcnt; x++) fputs("------", out);
    fputc('\n', out);           /* print a separating line */
    s1 = " | "; s2 = " |"; s3 = " "; end = "\n";
  }
  for (y = 0; y < cset->colcnt; y++) {
    yname = cset->names[y];     /* traverse the matrix columns, */
    if (!yname) continue;       /* but skip invalidated columns */
    fprintf(out, "%2i", ++n);   /* print the column number, */
    fputs(s1,    out);          /* a separator, */
    fputs(yname, out);          /* and the column name */
    for (x = (int)strlen(yname); x < cset->maxlen; x++)
      fputc(' ', out);          /* fill the output field */
    fputs(s2, out);             /* print a separator */
    for (x = 1; x < n; x++) {   /* skip lower left part of matrix */
      fputs("      ", out); if (tex) fputs(s2, out); }
    fputc(s3[0], out);          /* start a number */
    fputs(" 1.00", out);        /* the diagonal is always 1 */
    for (x = y+1; x < cset->colcnt; x++) {
      xname = cset->names[x];   /* traverse the remaining columns, */
      if (!xname) continue;     /* but skip invalidated columns */
      fputs(s3, out);           /* print a separator */
      r = mvn_corr(mvn, x, y);  /* get and check the correl. coeff. */
      if (r < MVN_UNKNOWN) { fputs("     ?", out); continue; }
      t = fabs(r);              /* print the correlation coefficient */
      fputs(((r >= 0) || (t < 0.0005)) ? " " : "-", out);
      if (t >= 0.9995) fputs("1.00", out);
      else fprintf(out, ".%03.0f", t *1000);
    }
    fputs(end, out);            /* terminate the output line */
  }
  if (tex)                      /* if to generate TeX output */
    fputs("\\hline\n\\end{tabular}\n", out);
}  /* correl() */

/*----------------------------------------------------------------------
  Functions
----------------------------------------------------------------------*/

static void error (int code, ...)
{                               /* --- print error message */
  va_list    args;              /* list of variable arguments */
  const char *msg;              /* error message */

  if ((code > 0) || (code < E_UNKNOWN))
    code = E_UNKNOWN;           /* check error code */
  msg = errmsgs[-code];         /* get error message */
  if (!msg) msg = errmsgs[-E_UNKNOWN];
  fprintf(stderr, "\n%s: ", prgname);
  va_start(args, code);         /* get variable arguments */
  vfprintf(stderr, msg, args);  /* print error message */
  va_end(args);                 /* end argument evaluation */

  #ifndef NDEBUG
  if (mvnorm) mvn_delete(mvnorm);
  if (colset) cs_delete(colset);
  if (tfscan) tfs_delete(tfscan);  /* clean up memory */
  if (symtab) st_delete(symtab);   /* and close files */
  if (in  && (in  != stdin))  fclose(in);
  if (out && (out != stdout)) fclose(out);
  #endif
  #ifdef STORAGE
  showmem("at end of program"); /* check memory usage */
  #endif
  exit(code);                   /* abort the program */
}  /* error() */

/*--------------------------------------------------------------------*/

int main (int argc, char *argv[])
{                               /* --- main function */
  int    i, k = 0;              /* loop variables, counters */
  char   *s;                    /* to traverse options */
  char   **optarg = NULL;       /* option argument */
  char   *fn_hdr  = NULL;       /* name of table header file */
  char   *fn_tab  = NULL;       /* name of table file */
  char   *fn_mat  = NULL;       /* name of matrix file */
  char   *fname   = NULL;       /* buffer for file name */
  char   *blanks  = NULL;       /* blank  characters */
  char   *fldseps = NULL;       /* field  separators */
  char   *recseps = NULL;       /* record separators */
  char   *uvchars = NULL;       /* unknown value characters */
  int    header   = 0;          /* header type (table/default/file) */
  int    fldcnt   = 0;          /* number of fields/columns */
  int    wgtflg   = 0;          /* flag for weight in last field */
  int    flags    = 0;          /* flags for matrices to print */
  int    tex      = 0;          /* flag for TeX output */
  size_t maxlen   = 6, len;     /* (maximal) length of a value name */
  int    tplcnt   = 0;          /* number of tuples */
  double tplwgt   = 0.0;        /* weight of tuples */
  double weight   = 1.0;        /* weight of tuple */
  int    d;                     /* delimiter type */
  int    *p;                    /* pointer to symbol data */

  prgname = argv[0];            /* get program name for error msgs. */

  /* --- print startup/usage message --- */
  if (argc > 1) {               /* if arguments are given */
    fprintf(stderr, "%s - %s\n", argv[0], DESCRIPTION);
    fprintf(stderr, VERSION); } /* print a startup message */
  else {                        /* if no argument is given */
    printf("usage: %s [options] "
                     "[-d|-h hdrfile] tabfile [matfile]\n", argv[0]);
    printf("%s\n", DESCRIPTION);
    printf("%s\n", VERSION);
    printf("-x       print expected values and standard deviation\n");
    printf("-v       print covariance matrix\n");
    printf("-c       print correlation coefficients (default)\n");
    printf("-m       use maximum likelihood estimates "
                    "for the (co)variances\n");
    printf("-t       print output in TeX format\n");
    printf("-n       number of tuple occurrences in last field\n");
    printf("-b/f/r#  blank characters, field and record separators\n"
           "         (default: \" \\t\\r\", \" \\t\", \"\\n\")\n");
    printf("-u#      unknown value characters (default: \"?\")\n");
    printf("-d       use default header "
                    "(field names = field numbers)\n");
    printf("-h       read table header (field names) from hdrfile\n");
    printf("hdrfile  file containing table header (field names)\n");
    printf("tabfile  table file to read "
                    "(field names in first record)\n");
    printf("matfile  file to write matrix to (optional)\n");
    return 0;                   /* print a usage message */
  }                             /* and abort the program */

  /* --- evaluate arguments --- */
  for (i = 1; i < argc; i++) {  /* traverse arguments */
    s = argv[i];                /* get option argument */
    if (optarg) { *optarg = s; optarg = NULL; continue; }
    if ((*s == '-') && *++s) {  /* -- if argument is an option */
      while (1) {               /* traverse characters */
        switch (*s++) {         /* evaluate option */
          case 'x': flags |= MVN_EXPVAR;   break;
          case 'v': flags |= MVN_COVAR;    break;
          case 'c': flags |= MVN_CORREL;   break;
          case 'm': flags |= MVN_MAXLLH;   break;
          case 't': tex    = 1;            break;
          case 'n': wgtflg = 1;            break;
  	  case 'b': optarg = &blanks;      break;
          case 'f': optarg = &fldseps;     break;
          case 'r': optarg = &recseps;     break;
          case 'u': optarg = &uvchars;     break;
          case 'd': header = 1;            break;
          case 'h': optarg = &fn_hdr;      break;
          default : error(E_OPTION, *--s); break;
        }                       /* set option variables */
        if (!*s) break;         /* if at end of string, abort loop */
        if (optarg) { *optarg = s; optarg = NULL; break; }
      } }                       /* get option argument */
    else {                      /* -- if argument is no option */
      switch (k++) {            /* evaluate non-option */
        case  0: fn_tab = s;      break;
        case  1: fn_mat = s;      break;
        default: error(E_ARGCNT); break;
      }                         /* note filenames */
    }
  }
  if (optarg) error(E_OPTARG);  /* check option argument */
  if ((k < 1) || (k > 2)) error(E_ARGCNT);
  if (fn_hdr) {                 /* set the header file flag */
    header = 2; if (strcmp(fn_hdr, "-") == 0) fn_hdr = ""; }
  if (!flags) flags  = MVN_CORREL;

  /* --- create symbol table and table file scanner --- */
  symtab = st_create(0, 0, (HASHFN*)0, (SYMFN*)0);
  if (!symtab) error(E_NOMEM);  /* create symbol table */
  tfscan = tfs_create();        /* and table file scanner */
  if (!tfscan) error(E_NOMEM);  /* and set delimiter characters */
  tfs_chars(tfscan, TFS_BLANK,  blanks);
  tfs_chars(tfscan, TFS_FLDSEP, fldseps);
  tfs_chars(tfscan, TFS_RECSEP, recseps);
  colset = cs_create();         /* create a column set */
  if (!colset) error(E_NOMEM);  /* for the table reading */

  /* --- read table header/first record --- */
  fname = (fn_hdr) ? fn_hdr : fn_tab;
  if (!fname || !*fname) {      /* if no proper file name is given, */
    in = stdin; fname = "<stdin>"; }    /* read from standard input */
  else {                        /* if a proper file name is given */
    in = fopen(fname, "r");     /* open file for reading */
    if (!in) error(E_FOPEN, fname);
  }
  fprintf(stderr, "\nreading %s ... ", fname);
  do {                          /* read fields of table header */
    d = tfs_getfld(tfscan, in, rdbuf, BUFSIZE-1);
    if (d < 0) error(E_FREAD, fname);
    fldcnt++;                   /* read the next field name */
    if ((d <= TFS_REC) && wgtflg)
      break;                    /* skip a tuple weight field */
    if (header != 1) {          /* if to use a normal header */
      if (!rdbuf[0])   error(E_EMPFLD, fname, 1, rdbuf, fldcnt);
      p = (int*)st_insert(symtab, rdbuf, -1, sizeof(int));
      if (!p) error(E_NOMEM);   /* insert name into symbol table */
      if (p == EXISTS) error(E_DUPFLD, fname, 1, rdbuf, fldcnt);
      *p = fldcnt-1;            /* note the field number */
      if (cs_add(colset, st_name(p)) != 0)
        error(E_NOMEM); }       /* add a column to the column set */
    else {                      /* if to use a default header, */
      sprintf(fnbuf, "%d", fldcnt);     /* create a field name */
      p = (int*)st_insert(symtab, fnbuf, -1, sizeof(int));
      if (!p) error(E_NOMEM);   /* insert name into symbol table */
      *p = fldcnt-1;            /* note the field number */
      if (cs_add(colset, st_name(p)) != 0)
        error(E_NOMEM);         /* add a column to the column set */
      cs_set(colset, fldcnt-1, isunk(tfscan, rdbuf) ? NULL : rdbuf);
    }                           /* set the column value */
    len = strlen(st_name(p));   /* determine the maximal length */
    if (len > maxlen) maxlen = len;       /* of the field names */
  } while (d > TFS_REC);        /* while not at end of record */
  if (fn_hdr) {                 /* close the table header file */
    fclose(in); fprintf(stderr, "done."); }

  /* --- compute covariance matrix --- */
  mvnorm = mvn_create(colset->colcnt);
  if (!mvnorm) error(E_NOMEM);  /* create a normal distribution */
  if      (header > 1) {        /* if a table header file is given */
    if (fn_tab && *fn_tab)      /* if a proper table name is given, */
      in = fopen(fn_tab, "r");  /* open table file for reading */
    else {                      /* if no table file name is given, */
      in = stdin; fn_tab = "<stdin>"; }    /* read from std. input */
    fprintf(stderr, "reading %s ... ", fn_tab);
    if (!in) error(E_FOPEN, fn_tab); }
  else if (header > 0) {        /* if to use a default header */
    if (wgtflg) {               /* if a tuple weight is given, */
      weight = strtod(rdbuf,&s);       /* get the tuple weight */
      if (!*s || (s == rdbuf) || (weight < 0))
        error(E_VALUE, fname, 1, rdbuf, fldcnt);
    }
    cs_process(colset, mvnorm, weight);
    tplwgt += weight;           /* aggregate for the first tuple, */
    tplcnt++;                   /* sum the tuple weight and */
  }                             /* increment the tuple counter */
  do {                          /* read table records */
    d = TFS_FLD;                /* read fields in table record */
    for (i = 0; (i < fldcnt) && (d >= TFS_FLD); i++) {
      d = tfs_getfld(tfscan, in, rdbuf, BUFSIZE-1);
      if (d < 0) error(E_FREAD, fn_tab);
      if ((d <= TFS_EOF)        /* if at end of file */
      &&  (i <= 0)              /* and on first field */
      &&  (rdbuf[0] == '\0')) { /* and no value read */
        i = -1; break; }        /* (empty record), abort loop */
      if (i < fldcnt -wgtflg)   /* if this is a normal field */
        cs_set(colset, i, isunk(tfscan, rdbuf) ? NULL : rdbuf);
      else {                    /* if this is the weight field */
        weight = strtod(rdbuf, &s);
        if (!*s || (s == rdbuf) || (weight < 0))
          error(E_VALUE, fn_tab, tplcnt +((header > 0) ? 1 : 2), rdbuf);
      }                         /* get the tuple weight/counter */
    }                           /* and check for a valid number */
    if (i < 0) break;           /* if at end of file, abort loop */
    if (i != fldcnt)            /* check number of fields in record */
      error(E_FLDCNT, fn_tab, tplcnt +((header > 0) ? 1 : 2), fldcnt);
    cs_process(colset, mvnorm, weight);
    tplwgt += weight;           /* aggregate for the current tuple */
    tplcnt++;                   /* and sum the weight/count the tuple */
  } while (d > TFS_EOF);        /* while not at end of file */
  if (in != stdin) fclose(in);  /* close the input file and */
  in = NULL;                    /* clear the file variable */
  fprintf(stderr, "[%d/%g tuple(s)] done.\n", tplcnt, tplwgt);

  /* --- print matrix/matrices --- */
  k = flags;                    /* build the evaluation flags */
  if (k & MVN_CORREL) k |= MVN_COVAR;
  if (k & MVN_COVAR)  k |= MVN_EXPVAR;
  mvn_calc(mvnorm, k);          /* calculate parameters from data */
  if (fn_mat)                   /* if a matrix file name is given, */
    out = fopen(fn_mat, "w");   /* open correlation matrix file */
  else {                        /* if no matrix file name is given, */
    out = stdout; fn_mat = "<stdout>"; }         /* write to stdout */
  fprintf(stderr, "writing %s ... ", fn_mat);
  if (!out) error(E_FOPEN, fn_mat);
  if (flags & MVN_EXPVAR) {     /* print the expected values */
    expvar(colset, mvnorm, tex, out);
    if (flags & (MVN_COVAR|MVN_CORREL)) fputc('\n', out);
  }                             /* leave one line empty */
  if (flags & MVN_COVAR) {      /* print the covariance matrix */
    covar(colset, mvnorm, tex, out);
    if (flags & MVN_CORREL) fputc('\n', out);
  }                             /* leave one line empty */
  if (flags & MVN_CORREL)       /* print the correlation coefficients */
    correl(colset, mvnorm, tex, out);
  if (out != stdout) {          /* if not written to standard output */
    i = fclose(out); out = NULL;
    if (i) error(E_FWRITE, fn_mat);
  }                             /* close correlation matrix file */
  fprintf(stderr, "done.\n");   /* and print a success message */

  /* --- clean up --- */
  #ifndef NDEBUG                /* if debug version */
  mvn_delete(mvnorm);           /* delete normal distribution, */
  cs_delete(colset);            /* column set, */
  tfs_delete(tfscan);           /* table file scanner, */
  st_delete(symtab);            /* and symbol table */
  #endif
  #ifdef STORAGE
  showmem("at end of program"); /* check memory usage */
  #endif
  return 0;                     /* return 'ok' */
}  /* main() */
