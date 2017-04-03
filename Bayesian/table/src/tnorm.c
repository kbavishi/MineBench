/*----------------------------------------------------------------------
  File    : tnorm.c
  Contents: program to normalize numeric table columns
  Author  : Christian Borgelt
  History : 22.07.2003 file created from file tbal.c
            16.08.2003 slight changes in error message output
----------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>
#ifndef AS_RDWR
#define AS_RDWR
#endif
#ifndef TAB_RDWR
#define TAB_RDWR
#endif
#include "io.h"
#ifdef STORAGE
#include "storage.h"
#endif

/*----------------------------------------------------------------------
  Preprocessor Definitions
----------------------------------------------------------------------*/
#define PRGNAME     "tnorm"
#define DESCRIPTION "normalize numeric table columns"
#define VERSION     "version 1.1 (2003.08.16)         " \
                    "(c) 2003   Christian Borgelt"

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
#define E_TYPE      (-8)        /* wrong field type */
#define E_UNKNOWN   (-9)        /* unknown error */

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
  /* E_TYPE     -8 */  "wrong type (field must be numeric)\n",
  /* E_UNKNOWN  -9 */  "unknown error\n"
};

/*----------------------------------------------------------------------
  Global Variables
----------------------------------------------------------------------*/
const  char   *prgname = NULL;  /* program name for error messages */
static ATTSET *attset  = NULL;  /* attribute set */
static TABLE  *table   = NULL;  /* table */
static FILE   *in      = NULL;  /* input file */

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
  #ifndef NDEBUG                    /* clean up memory */
  if (table)  tab_delete(table, 0); /* and close files */
  if (attset) as_delete(attset);
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
  int    i, k = 0;              /* loop variables, counters */
  char   *s;                    /* to traverse options */
  char   **optarg = NULL;       /* option argument */
  char   *fn_hdr  = NULL;       /* name of table header file */
  char   *fn_tab  = NULL;       /* name of table file */
  char   *fn_out  = NULL;       /* name of output file */
  char   *blanks  = NULL;       /* blank  characters */
  char   *fldseps = NULL;       /* field  separators */
  char   *recseps = NULL;       /* record separators */
  char   *uvchars = NULL;       /* unknown value characters */
  char   *nrmcol  = NULL;       /* name of column to normalize */
  int    inflags  = 0;          /* table file read  flags */
  int    outflags = AS_ATT;     /* table file write flags */
  int    nrmid    = -1;         /* id of column to normalize */
  double exp      = 0;          /* desired expected value */
  double sdev     = 1;          /* desired standard deviation */

  prgname = argv[0];            /* get program name for error msgs. */

  /* --- print startup/usage message --- */
  if (argc > 1) {               /* if arguments are given */
    fprintf(stderr, "%s - %s\n", argv[0], DESCRIPTION);
    fprintf(stderr, VERSION); } /* print a startup message */
  else {                        /* if no argument is given */
    printf("usage: %s [options] "
                     "[-d|-h hdrfile] tabfile outfile\n", argv[0]);
    printf("%s\n", DESCRIPTION);
    printf("%s\n", VERSION);
    printf("-c#      name of field to normalize "
                    "(default: all numeric)\n");
    printf("-e#      desired expected value or minimum (default: 0)\n");
    printf("-s#      desired standard deviation (> 0) "
                    "or range (< 0) (default: 1)\n");
    printf("-a       align fields of output table "
                    "(default: do not align)\n");
    printf("-w       do not write field names to output file\n");
    printf("-b/f/r#  blank characters, field and record separators\n"
           "         (default: \" \\t\\r\", \" \\t\", \"\\n\")\n");
    printf("-u#      unknown value characters (default: \"?\")\n");
    printf("-n       number of tuple occurrences in last field\n");
    printf("-d       use default header "
                    "(field names = field numbers)\n");
    printf("-h       read table header (field names) from hdrfile\n");
    printf("hdrfile  file containing table header (field names)\n");
    printf("infile   table file to read "
                    "(field names in first record)\n");
    printf("outfile  file to write output table to\n");
    return 0;                   /* print a usage message */
  }                             /* and abort the program */

  /* --- evaluate arguments --- */
  for (i = 1; i < argc; i++) {  /* traverse arguments */
    s = argv[i];                /* get option argument */
    if (optarg) { *optarg = s; optarg = NULL; continue; }
    if ((*s == '-') && *++s) {  /* -- if argument is an option */
      while (1) {               /* traverse characters */
        switch (*s++) {         /* evaluate option */
          case 'c': optarg    = &nrmcol;       break;
          case 'e': exp       = strtod(s, &s); break;
          case 's': sdev      = strtod(s, &s); break;
          case 'a': outflags |= AS_ALIGN;      break;
          case 'w': outflags &= ~AS_ATT;       break;
  	  case 'b': optarg    = &blanks;       break;
          case 'f': optarg    = &fldseps;      break;
          case 'r': optarg    = &recseps;      break;
          case 'u': optarg    = &uvchars;      break;
          case 'n': inflags  |= AS_WEIGHT;     break;
          case 'd': inflags  |= AS_DFLT;       break;
          case 'h': optarg    = &fn_hdr;       break;
          default : error(E_OPTION, *--s);     break;
        }                       /* set option variables */
        if (!*s) break;         /* if at end of string, abort loop */
        if (optarg) { *optarg = s; optarg = NULL; break; }
      } }                       /* get option argument */
    else {                      /* -- if argument is no option */
      switch (k++) {            /* evaluate non-option */
        case  0: fn_tab = s;      break;
        case  1: fn_out = s;      break;
        default: error(E_ARGCNT); break;
      }                         /* note filenames */
    }
  }
  if (optarg) error(E_OPTARG);  /* check option argument */
  if (k != 2) error(E_ARGCNT);  /* check number of arguments */
  if (fn_hdr) {                 /* set header flags */
    inflags = AS_ATT | (inflags & ~AS_DFLT);
    if (strcmp(fn_hdr, "-") == 0) fn_hdr = "";
  }                             /* convert "-" to "" */

  /* --- read table header --- */
  attset = as_create("domains", att_delete);
  if (!attset) error(E_NOMEM);  /* create an attribute set */
  as_chars(attset, blanks, fldseps, recseps, uvchars);
  fprintf(stderr, "\n");        /* set delimiter characters */
  in = io_hdr(attset, fn_hdr, fn_tab, inflags, 1);
  if (!in) error(1);            /* read the table header */

  /* --- determine id of column to normalize --- */
  if (nrmcol) {                 /* if a column name is given */
    if ((nrmid = as_attid(attset, nrmcol)) < 0) {
      s = (inflags & AS_ATT) ? fn_hdr : fn_tab;
      io_error(E_MISFLD, s, 1, nrmcol); error(1);
    }                           /* check whether class exists */
  }                             /* and abort on error */

  /* --- read table --- */
  table = io_bodyin(attset, in, fn_tab, inflags, "table", 1);
  in    = NULL;                 /* read the table and */
  if (!table) error(1);         /* check for an error */

  /* --- normalize columns --- */
  if (nrmid >= 0) {             /* if a specific column is given */
    tab_colconv(table, nrmid, AT_AUTO);      /* determine the type */
    k = att_type(tab_col(table, nrmid));     /* and get it */
    if      (k == AT_INT) tab_colconv(table, nrmid, AT_FLT);
    else if (k != AT_FLT) error(E_TYPE);     /* convert to float */
    tab_colnorm(table, nrmid, exp, sdev); }  /* and normalize */
  else{                         /* if to convert all numeric columns */
    for (i = tab_colcnt(table); --i >= 0; ) {
      tab_colconv(table, i, AT_AUTO);        /* determine the type */
      k = att_type(tab_col(table, i));       /* and get it */
      if      (k == AT_INT) tab_colconv(table, i, AT_FLT);
      else if (k != AT_FLT) continue;        /* convert to float */
      tab_colnorm(table, i, exp, sdev);
    }                           /* normalize the numeric columns */
  }

  /* --- write output table --- */
  if (io_tabout(table, fn_out, outflags, 1) != 0)
    error(1);                   /* write the balanced table */

  /* --- clean up --- */
  #ifndef NDEBUG
  tab_delete(table, 1);         /* delete table and attribute set */
  #endif
  #ifdef STORAGE
  showmem("at end of program"); /* check memory usage */
  #endif
  return 0;                     /* return 'ok' */
}  /* main() */
