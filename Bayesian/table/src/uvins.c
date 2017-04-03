/*----------------------------------------------------------------------
  File    : uvins.c
  Contents: insert unknown values into a table
  Authors : Christian Borgelt
  History : 20.12.2002 file created
            18.01.2003 option -x extended, options -i, -w added
            16.08.2003 slight changes in error message output
----------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <assert.h>
#ifndef TAB_RDWR
#define TAB_RDWR
#endif
#include "io.h"
#include "vecops.h"
#ifdef STORAGE
#include "storage.h"
#endif

/*----------------------------------------------------------------------
  Preprocessor Definitions
----------------------------------------------------------------------*/
#define PRGNAME     "uvins"
#define DESCRIPTION "insert unknown values into a table"
#define VERSION     "version 1.3 (2003.08.16)         " \
                    "(c) 2002-2003   Christian Borgelt"

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
#define E_PERCENT   (-8)        /* illegal percentage */
#define E_INEX      (-9)        /* both include and exclude used */
#define E_FLDNAME  (-10)        /* unknown field name */
#define E_UNKNOWN  (-11)        /* unknown error */

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
  /* E_PERCENT  -8 */  "illegal percentage %d\n",
  /* E_INEX     -9 */  "both include and exclude used\n",
  /* E_FLDNAME -10 */  "unknown field name \"%s\"\n",
  /* E_UNKNOWN -11 */  "unknown error\n"
};

/*----------------------------------------------------------------------
  Global Variables
----------------------------------------------------------------------*/
const  char   *prgname = NULL;  /* program name for error messages */
static ATTSET *attset  = NULL;  /* attribute set */
static TABLE  *table   = NULL;  /* table */
static int    *map     = NULL;  /* column map */
static INST   **insts  = NULL;  /* instance vector */

/*----------------------------------------------------------------------
  Random Number Functions
----------------------------------------------------------------------*/
#ifdef DRAND48                  /* if library for drand48() available */
extern void   srand48 (long seed);
extern double drand48 (void);   /* use drand48 functions */
#define dseed(s) srand48(s)
#define drand    drand48

#else                           /* if only standard rand() available */
#define dseed(s) srand((unsigned int)s)
static double drand (void)
{ return rand()/(RAND_MAX +1.0); }
#endif

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
  #ifndef NDEBUG                /* clean up memory */
  if (insts)  free(insts);
  if (map)    free(map);
  if (table)  tab_delete(table, 0);
  if (attset) as_delete(attset);
  #endif
  #ifdef STORAGE
  showmem("at end of program"); /* check memory usage */
  #endif
  exit(code);                   /* abort the program */
}  /* error() */

/*--------------------------------------------------------------------*/

int main (int argc, char *argv[])
{                               /* --- main function */
  int    i, k = 0, n;           /* loop variables, counters */
  char   *s;                    /* to traverse options */
  char   **optarg = NULL;       /* option argument */
  char   *fn_hdr  = NULL;       /* name of table header file */
  char   *fn_in   = NULL;       /* name of table file to read */
  char   *fn_out  = NULL;       /* name of table file to write */
  char   *blanks  = NULL;       /* blank  characters */
  char   *fldseps = NULL;       /* field  separators */
  char   *recseps = NULL;       /* record separators */
  char   *uvchars = NULL;       /* unknown value characters */
  int    clude    = 0;          /* flag for field in-/exclusion */
  int    inflags  = 0;          /* table file read  flags */
  int    outflags = AS_ATT;     /* table file write flags */
  double percent  = 10.0;       /* percent unknown values */
  int    colcnt   = 0;          /* number of fields/columns */
  int    csvmem   = 0;          /* flag for memory conservation */
  int    rowcnt;                /* number of tuples/rows */
  long   seed;                  /* random number seed */
  TUPLE  *tpl;                  /* to traverse the tuples */
  INST   *inst;                 /* a table field */

  prgname = argv[0];            /* get program name for error msgs. */
  seed    = (long)time(NULL);   /* get a default seed value */

  /* --- print startup/usage message --- */
  if (argc > 1) {               /* if arguments are given */
    fprintf(stderr, "%s - %s\n", argv[0], DESCRIPTION);
    fprintf(stderr, VERSION); } /* print a startup message */
  else {                        /* if no argument is given */
    printf("usage: %s [options] infile outfile\n", argv[0]);
    printf("%s\n", DESCRIPTION);
    printf("%s\n", VERSION);
    printf("-p#      percentage of unknown values to insert "
                    "(default: %g%%)\n", percent);
    printf("-i#      name of field to include "
                    "(multiple fields possible)\n");
    printf("-x#      name of field to exclude "
                    "(multiple fields possible)\n");
    printf("-s#      seed value for random number generator "
                    "(default: time)\n");
    printf("-m       conserve memory (may slow down operation)\n");
    printf("-a#      align fields of output table "
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
    printf("outfile  table file to write\n");
    return 0;                   /* print a usage message */
  }                             /* and abort the program */

  /* --- evaluate arguments --- */
  for (i = 1; i < argc; i++) {  /* traverse arguments */
    s = argv[i];                /* get option argument */
    if (optarg) { *optarg = s; optarg = NULL; continue; }
    if ((*s == '-') && *++s) {  /* -- if argument is an option */
      while (1) {               /* traverse characters */
        switch (*s++) {         /* evaluate option */
          case 'p': percent   = strtod(s, &s);    break;
          case 'i': if (clude < 0) error(E_INEX);
                    clude     = +1;
                    optarg    = argv +colcnt++;   break;
          case 'x': if (clude > 0) error(E_INEX);
                    clude     = -1;
                    optarg    = argv +colcnt++;   break;
          case 's': seed      = strtol(s, &s, 0); break;
          case 'm': csvmem    = 1;                break;
          case 'a': outflags |= AS_ALIGN;         break;
          case 'w': outflags &= ~AS_ATT;          break;
  	  case 'b': optarg    = &blanks;          break;
          case 'f': optarg    = &fldseps;         break;
          case 'r': optarg    = &recseps;         break;
          case 'u': optarg    = &uvchars;         break;
          case 'n': inflags  |= AS_WEIGHT;        break;
          case 'd': inflags  |= AS_DFLT;          break;
          case 'h': optarg    = &fn_hdr;          break;
          default : error(E_OPTION, *--s);        break;
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
  if (k != 2) error(E_ARGCNT);  /* check number of arguments */
  if (fn_hdr) {                 /* set header flags */
    inflags = AS_ATT | (inflags & ~AS_DFLT);
    if (strcmp(fn_hdr, "-") == 0) fn_hdr = "";
  }                             /* convert "-" to "" */
  if ((percent < 0) || (percent > 100))
    error(E_PERCENT, percent);  /* check the percentage */

  /* --- create attribute set and read table --- */
  attset = as_create("domains", att_delete);
  if (!attset) error(E_NOMEM);  /* create an attribute set */
  as_chars(attset, blanks, fldseps, recseps, uvchars);
  fprintf(stderr, "\n");        /* set delimiter characters */
  table = io_tabin(attset, fn_hdr, fn_in, inflags, "table", 1);
  if (!table) error(1);         /* read the table file */
  map = (int*)malloc(tab_colcnt(table) *sizeof(int));
  if (!map) error(E_NOMEM);     /* create a column map */

  /* --- find columns to work on --- */
  fprintf(stderr, "inserting unknown values ... ");
  if      (clude) {             /* if to in-/exclude certain columns */
    k = (clude > 0) ? 0 : 1;    /* initialize the column map */
    for (i = n = tab_colcnt(table); --i >= 0; ) map[i] = k;              
    for (i = colcnt; --i >= 0; ) {   /* traverse the field names */
      k = as_attid(attset, argv[i]);
      if (k < 0) error(E_FLDNAME, argv[i]);
      map[k] = clude;           /* mark/unmark the columns */
    }                           /* to include or exclude */
    for (colcnt = i = 0; i < n; i++) /* collect the column numbers */
      if (map[i] > 0) map[colcnt++] = i; }
  else {                        /* if to work on all columns */
    colcnt = tab_colcnt(table); /* get the number of columns */
    for (i = colcnt; --i >= 0; ) map[i] = i;
  }                             /* create an identity map */
  rowcnt = tab_tplcnt(table);   /* get the number of tuples */

  /* --- insert unknown values --- */
  dseed(seed);                  /* traverse the unknowns to insert */
  if (csvmem) {                 /* if to conserve memory */
    for (n = (int)(0.01 *percent *rowcnt *colcnt +0.4999); --n >= 0; ) {
      do {                      /* table field search loop */
        i = (int)(colcnt *drand());
        if (i <  0)      i = 0; /* compute a random column index */
        if (i >= colcnt) i = colcnt-1;
        k = (int)(rowcnt *drand());
        if (k <  0)      k = 0; /* compute a random row index */
        if (k >= rowcnt) k = rowcnt-1;
        inst = tpl_colval(tab_tpl(table, k), map[i]);
      } while (inst->i < 0);    /* find a known table field */
      inst->i = UV_SYM;         /* and replace its contents */
    } }                         /* with an unknown value */
  else {                        /* if to use an instance vector */
    insts = (INST**)malloc(colcnt *rowcnt *sizeof(INST*));
    if (!insts) error(E_NOMEM); /* create an instance vector */
    for (n = 0, k = rowcnt; --k >= 0; ) {
      tpl = tab_tpl(table, k);  /* traverse the tuples of the table */
      for (i = colcnt; --i >= 0; )
        insts[n++] = tpl_colval(tpl, map[i]);
    }                           /* collect the eligible instances */
    v_shuffle(insts, n, drand); /* and shuffle them */
    for (n = (int)(0.01 *percent *n +0.4999); --n >= 0; )
      insts[n]->i = UV_SYM;     /* set the first 'percent' instances */
  }                             /* to an unknown value */
  fprintf(stderr, "done.\n");   /* print a success message */

  /* --- write the output table --- */
  io_tabout(table, fn_out, outflags, 1);

  /* --- clean up --- */
  #ifndef NDEBUG
  if (insts) free(insts);       /* delete instances vector, */
  free(map);                    /* column map, */
  tab_delete(table, 1);         /* table and attribute set */
  #endif
  #ifdef STORAGE
  showmem("at end of program"); /* check memory usage */
  #endif
  return 0;                     /* return 'ok' */
}  /* main() */
