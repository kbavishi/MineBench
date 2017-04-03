/*----------------------------------------------------------------------
  File    : tsplit.c
  Contents: program to split table into subtables
  Author  : Christian Borgelt
  History : 24.02.1998 file created
            12.09.1998 adapted to modified module attset
            25.09.1998 table reading simplified
            06.02.1999 arbitrary sample size made possible
            07.02.1999 input from stdin, output to stdout added
            12.02.1999 default header handling improved
            17.04.1999 simplified using the new module 'io'
            14.07.2001 adapted to modified module tfscan
            16.08.2003 slight changes in error message output
----------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <time.h>
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
#define PRGNAME     "tsplit"
#define DESCRIPTION "split a table into subtables"
#define VERSION     "version 1.6 (2003.08.16)         " \
                    "(c) 1998-2003   Christian Borgelt"

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
#define E_FLDNAME   (-8)        /* illegal field name */
#define E_EMPTAB    (-9)        /* empty table */
#define E_SMLTAB   (-10)        /* table too small for sample */
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
  /* E_FLDNAME  -8 */  "illegal field name \"%s\"\n",
  /* E_EMPTAB   -9 */  "table is empty\n",
  /* E_SMLTAB  -10 */  "table is too small for sample\n",
  /* E_UNKNOWN -11 */  "unknown error\n"
};

/*----------------------------------------------------------------------
  Global Variables
----------------------------------------------------------------------*/
const  char   *prgname = NULL;  /* program name for error messages */
static ATTSET *attset  = NULL;  /* attribute set */
static TABLE  *table   = NULL;  /* table */
static FILE   *in      = NULL;  /* input  file */
static FILE   *out     = NULL;  /* output file */
static char   fn_out[1024];     /* output file name */

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
  Comparison Function
----------------------------------------------------------------------*/

static int tplcmp (const TUPLE *tpl1, const TUPLE *tpl2, void *data)
{                               /* --- compare two tuples */
  const INST *col1, *col2;      /* buffer for column values */

  col1 = tpl_colval(tpl1, (int)data);   /* get column values */
  col2 = tpl_colval(tpl2, (int)data);   /* for both tuples */
  if (col1->i > col2->i) return  1;
  if (col1->i < col2->i) return -1;
  return 0;                     /* return sign of diff. of values */
}  /* tplcmp() */

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
    vfprintf(stderr, msg, args);/* print error message */
    va_end(args);               /* end argument evaluation */
  }
  #ifndef NDEBUG
  if (table)  tab_delete(table, 0); /* clean up memory */
  if (attset) as_delete(attset);    /* and close files */
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
  char   *blanks  = NULL;       /* blank  characters */
  char   *fldseps = NULL;       /* field  separators */
  char   *recseps = NULL;       /* record separators */
  char   *uvchars = NULL;       /* unknown value characters */
  char   *colname = NULL;       /* name of field to base split on */
  char   *pattern = "%i.tab";   /* output file name pattern */
  int    inflags  = 0;          /* table file read  flags */
  int    outflags = AS_ATT;     /* table file write flags */
  int    shuffle  = 0;          /* flag for tuple shuffling */
  int    sample   = 0;          /* flag for drawing a sample */
  int    colid    = -1;         /* column identifier */
  int    tplcnt   = 0;          /* number of tuples */
  double tplwgt   = 0;          /* weight of tuples */
  double tabcnt   = 0;          /* number of tables */
  int    tabid;                 /* table identifier */
  long   seed;                  /* random number seed */
  int    size, first, tplid;    /* table size and tuple index */
  int    prev, val;             /* (previous) column value */
  double off, wgt, tmp;         /* offset, tuple weight and buffer */
  TUPLE  *tpl;                  /* tuple to traverse table */
  int    one_in_n;              /* flag for one in n selection */
  int    done = 0;              /* completion flag */

  prgname = argv[0];            /* get program name for error msgs. */
  seed    = (long)time(NULL);   /* get a default seed value */

  /* --- print startup/usage message --- */
  if (argc > 1) {               /* if arguments are given */
    fprintf(stderr, "%s - %s\n", argv[0], DESCRIPTION);
    fprintf(stderr, VERSION); } /* print a startup message */
  else {                        /* if no argument is given */
    printf("usage: %s [options] [-d|-h hdrfile] tabfile\n", argv[0]);
    printf("%s\n", DESCRIPTION);
    printf("%s\n", VERSION);
    printf("-c#      name of field/column to base split on "
                    "(default: none)\n");
    printf("         (stratified sampling if the option -t "
                    "is also given)\n");
    printf("-x       shuffle tuples before operation\n");
    printf("-s#      seed value for random number generator "
                    "(default: time)\n");
    printf("-t#      number of subtables to split into\n");
    printf("-p#      draw a sample with # tuples (one output table)\n");
    printf("-o#      output file name pattern "
                    "(default: \"%s\")\n", pattern);
    printf("-a       align fields of output tables "
                    "(default: do not align)\n");
    printf("-w       do not write field names to output files\n");
    printf("-b/f/r#  blank characters, field and record separators\n"
           "         (default: \" \\t\\r\", \" \\t\", \"\\n\")\n");
    printf("-u#      unknown value characters (default: \"?\")\n");
    printf("-n       number of tuple occurences in last field\n");
    printf("-d       use default header "
                    "(field names = field numbers)\n");
    printf("-h       read table header (field names) from hdrfile\n");
    printf("hdrfile  file containing table header (field names)\n");
    printf("tabfile  table file to read "
                    "(field names in first record)\n");
    return 0;                   /* print a usage message */
  }                             /* and abort the program */

  /* --- evaluate arguments --- */
  for (i = 1; i < argc; i++) {  /* traverse arguments */
    s = argv[i];                /* get option argument */
    if (optarg) { *optarg = s; optarg = NULL; continue; }
    if ((*s == '-') && *++s) {  /* -- if argument is an option */
      while (1) {               /* traverse characters */
        switch (*s++) {         /* evaluate option */
          case 'c': optarg    = &colname;              break;
          case 'x': shuffle   = 1;                     break;
          case 's': seed      =      strtol(s, &s, 0); break;
          case 't': tabcnt    =      strtol(s, &s, 0); break;
          case 'p': sample    = (int)strtol(s, &s, 0); break;
   	  case 'o': optarg    = &pattern;              break;
          case 'a': outflags |= AS_ALIGN;              break;
          case 'w': outflags &= ~AS_ATT;               break;
  	  case 'b': optarg    = &blanks;               break;
          case 'f': optarg    = &fldseps;              break;
          case 'r': optarg    = &recseps;              break;
          case 'u': optarg    = &uvchars;              break;
          case 'n': outflags |= AS_WEIGHT;
                    inflags  |= AS_WEIGHT;             break;
          case 'd': inflags  |= AS_DFLT;               break;
          case 'h': optarg    = &fn_hdr;               break;
          default : error(E_OPTION, *--s);             break;
        }                       /* set option variables */
        if (!*s) break;         /* if at end of string, abort loop */
        if (optarg) { *optarg = s; optarg = NULL; break; }
      } }                       /* get option argument */
    else {                      /* -- if argument is no option */
      switch (k++) {            /* evaluate non-option */
        case  0: fn_tab = s;      break;
        default: error(E_ARGCNT); break;
      }                         /* note filenames */
    }
  }
  if (optarg) error(E_OPTARG);  /* check option argument */
  if (k != 1) error(E_ARGCNT);  /* check number of arguments */
  if (fn_hdr && (strcmp(fn_hdr, "-") == 0))
    fn_hdr = "";                /* convert "-" to "" */
  if (fn_hdr)                   /* set header flags */
    inflags = AS_ATT | (inflags & ~AS_DFLT);
  if ((outflags & AS_ALIGN) && (outflags & AS_ATT))
    outflags |= AS_ALNHDR;      /* set align to header flag */

  /* --- create attribute set and read table --- */
  attset = as_create("domains", att_delete);
  if (!attset) error(E_NOMEM);  /* create an attribute set */
  as_chars(attset, blanks, fldseps, recseps, uvchars);
  fprintf(stderr, "\n");        /* set delimiter characters */
  in = io_hdr(attset, fn_hdr, fn_tab, inflags, 1);
  if (!in) error(1);            /* read the table header */
  if (colname) {                /* if a field/column name is given */
    colid = as_attid(attset, colname);
    if (colid < 0) error(E_FLDNAME, colname);
  }                             /* get the column identifier */
  table = io_bodyin(attset, in, fn_tab, inflags, "table", 1);
  in    = NULL;                 /* read the table body */
  if (!table) error(1);         /* and check for an error */
  tplcnt = tab_tplcnt(table);   /* get and check the table size */
  if (tplcnt <= 0)     error(E_EMPTAB);
  if (sample > tplcnt) error(E_SMLTAB);

  /* --- split table --- */
  if (shuffle) {                /* if the shuffle flag is set, */
    dseed(seed);                /* init. random number generator */
    tab_shuffle(table, 0, INT_MAX, drand);
  }                             /* shuffle tuples in table */
  if (colid >= 0)               /* sort table w.r.t. given column */
    tab_sort(table, 0, INT_MAX, tplcmp, (void*)colid);
  if (sample > 0) tabcnt = (tplcnt/(double)sample) *(1 +1e-12);
  one_in_n = ((colid < 0) || (tabcnt > 0));
  if (tabcnt <= 0) tabcnt = 1;  /* get tuple selection mode */
  val   = UV_SYM;               /* clear current and get first value */
  prev  = (colid >= 0) ? tpl_colval(tab_tpl(table, 0), colid)->i : val;
  size  = tplcnt;               /* note number of tuples in table, */
  first = tplid = tabid = 0;    /* initialize tuple and table index */
  do {                          /* table write loop */
    if (!*pattern) {            /* if no file name pattern is given, */
      out = stdout; strcpy(fn_out, "<stdout>"); } /* write to stdout */
    else {                      /* if a file name pattern is given */
      sprintf(fn_out, pattern, tabid++);
      out = fopen(fn_out, "w"); /* open output file for writing */
    }
    fprintf(stderr, "writing %s ... ", fn_out);
    if (!out) error(E_FOPEN, fn_out);
    if ((outflags & AS_ATT)     /* if to write table header */
    &&  (as_write(attset, out, outflags) != 0))
      error(E_FWRITE, fn_out);  /* write field names to subtable */
    k = AS_INST | (outflags & ~AS_ATT);
    tplcnt = 0; tplwgt = 0;     /* initialize tuple counter */
    if (one_in_n) {             /* if to select every n-th tuple */
      tplid = 0; off = first;   /* get next tuple offset */
      if ((++first >= tabcnt) || sample)
        done = 1;               /* if last table, set done flag */
      while (tplid < size) {    /* while not at end of table */
        tpl = tab_tpl(table, tplid++);  /* get next tuple */
        wgt = floor(tpl_getwgt(tpl));   /* and its weight */
        if (wgt <= off) {       /* if offset is larger than weight, */
          off -= wgt; continue; }                /* skip this tuple */
        tpl_toas(tpl);          /* transfer tuple to attribute set */
        tplwgt += tmp = ceil((wgt -off) /tabcnt);
        as_setwgt(attset, tmp); /* set weight instantiation weight */
        if (as_write(attset, out, k) != 0)
          error(E_FWRITE, fn_out);  /* write instantiation (tuple) */
        off = fmod(off +tmp *tabcnt -wgt, tabcnt);
        tplcnt++;               /* compute next offset and */
      } }                       /* increment tuple counter */
    else {                      /* if to split according to values */
      while (tplid < size) {    /* while not all tuples processed */
        tpl = tab_tpl(table, tplid);      /* get next tuple and */
        val = tpl_colval(tpl, colid)->i;  /* its column value */
        if (val != prev) break; /* if next value reached, abort */
        tpl_toas(tpl);          /* transfer tuple to attribute set */
        if (as_write(attset, out, k) != 0)
          error(E_FWRITE, fn_out);  /* write instantiation (tuple) */
        tplwgt += as_getwgt(attset);
        tplcnt++; tplid++;      /* write instantiation (tuple) */
      }                         /* and increment tuple counter */
      if (tplid >= size) done = 1;
      prev = val;               /* check for completion and */
    }                           /* note the current column value */
    if (out == stdout) {        /* if written to standard output, */
      if (!done) printf("\n");} /* separate tables by an empty line */
    else {                      /* if not written to standard output */
      i = fclose(out); out = NULL;  /* close output file */
      if (i != 0) error(E_FWRITE, fn_out);
    }                           /* print a success message */
    fprintf(stderr, "[%d/%g tuple(s)] done.\n", tplcnt, tplwgt);
  } while (!done);              /* while not all tables written */

  /* --- clean up --- */
  #ifndef NDEBUG
  tab_delete(table, 1);         /* delete table and attribute set */
  #endif
  #ifdef STORAGE
  showmem("at end of program"); /* check memory usage */
  #endif
  return 0;                     /* return 'ok' */
}  /* main() */
