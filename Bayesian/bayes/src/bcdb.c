/*----------------------------------------------------------------------
  File    : bcdb.c
  Contents: generate a database from a Bayes classifier
  Author  : Christian Borgelt
  History : 26.04.2003 file created from file bcx.c
            16.08.2003 slight changes in error message output
----------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#ifndef SC_SCAN
#define SC_SCAN
#endif
#include "scan.h"
#ifndef AS_RDWR
#define AS_RDWR
#endif
#ifndef AS_PARSE
#define AS_PARSE
#endif
#include "attset.h"
#ifndef NBC_PARSE
#define NBC_PARSE
#endif
#include "nbayes.h"
#ifndef FBC_PARSE
#define FBC_PARSE
#endif
#include "fbayes.h"
#ifdef STORAGE
#include "storage.h"
#endif

/*----------------------------------------------------------------------
  Preprocessor Definitions
----------------------------------------------------------------------*/
#define PRGNAME     "bcdb"
#define DESCRIPTION "generate a database from a Bayes classifier"
#define VERSION     "version 1.1 (2003.08.16)         " \
                    "(c) 2003   Christian Borgelt"

/* --- error codes --- */
#define OK            0         /* no error */
#define E_NONE        0         /* no error */
#define E_NOMEM     (-1)        /* not enough memory */
#define E_FOPEN     (-2)        /* cannot open file */
#define E_FREAD     (-3)        /* read error on file */
#define E_FWRITE    (-4)        /* write error on file */
#define E_OPTION    (-5)        /* unknown option */
#define E_OPTARG    (-6)        /* missing option argument */
#define E_ARGCNT    (-7)        /* wrong number of arguments */
#define E_PARSE     (-8)        /* parse error */
#define E_NEGLC     (-9)        /* negative Laplace correction */
#define E_UNKNOWN  (-10)        /* unknown error */

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
  /* E_PARSE    -8 */  "parse error(s) on file %s\n",
  /* E_NEGLC    -9 */  "Laplace correction must not be negative\n",
  /* E_UNKNOWN -10 */  "unknown error\n"
};

/*----------------------------------------------------------------------
  Global Variables
----------------------------------------------------------------------*/
const  char   *prgname = NULL;  /* program name for error messages */
static SCAN   *scan    = NULL;  /* scanner */
static NBC    *nbc     = NULL;  /* naive Bayes classifier */
static FBC    *fbc     = NULL;  /* full  Bayes classifier */
static ATTSET *attset  = NULL;  /* attribute set */
static FILE   *out     = NULL;  /* output file */

/*----------------------------------------------------------------------
  Random Number Functions
----------------------------------------------------------------------*/
#ifdef DRAND48                  /* if library for drand48() available */
extern void   srand48 (long seed);
extern double drand48 (void);   /* use drand48 functions */
#define dseed(s) srand48((long)(s))
#define drand    drand48

#else                           /* if only standard rand() available */
#define dseed(s) srand((unsigned)(s))
static double drand (void)      /* compute value from rand() result */
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
    msg = errmsgs[-code];       /* get error message */
    if (!msg) msg = errmsgs[-E_UNKNOWN];
    fprintf(stderr, "\n%s: ", prgname);
    va_start(args, code);       /* get variable arguments */
    vfprintf(stderr, msg, args);/* print error message */
    va_end(args);               /* end argument evaluation */
  }
  #ifndef NDEBUG
  if (nbc)    nbc_delete(nbc, 0);
  if (fbc)    fbc_delete(fbc, 0);
  if (attset) as_delete(attset);   /* clean up memory */
  if (scan)   sc_delete(scan);     /* and close files */
  if (out && (out != stdout)) fclose(out);
  #endif
  #ifdef STORAGE
  showmem("at end of program"); /* check memory usage */
  #endif
  exit(code);                   /* abort programm */
}  /* error() */

/*--------------------------------------------------------------------*/

int main (int argc, char* argv[])
{                               /* --- main function */
  int    i, k = 0;              /* loop variables, buffer */
  char   *s;                    /* to traverse options */
  char   **optarg = NULL;       /* option argument */
  char   *fn_bc   = NULL;       /* name of classifier file */
  char   *fn_out  = NULL;       /* name of output file */
  char   *blank   = NULL;       /* blank */
  char   *fldsep  = NULL;       /* field  separator */
  char   *recsep  = NULL;       /* record separator */
  int    flags    = AS_ATT;     /* table file write flags */
  double lcorr    = -DBL_MAX;   /* Laplace correction value */
  int    distuv   = 0;          /* distribute weight of unknowns */
  int    maxllh   = 0;          /* max. likelihood est. of variance */
  int    tplcnt   = 1000;       /* number of tuples to generate */
  long   seed;                  /* seed for random number generator */
  int    mode;                  /* classifier setup mode */

  prgname = argv[0];            /* get program name for error msgs. */
  seed    = (long)time(NULL);   /* and get a default seed value */

  /* --- print startup/usage message --- */
  if (argc > 1) {               /* if arguments are given */
    fprintf(stderr, "%s - %s\n", argv[0], DESCRIPTION);
    fprintf(stderr, VERSION); } /* print a startup message */
  else {                        /* if no argument given */
    printf("usage: %s [options] bcfile "
                     "[-d|-h hdrfile] tabfile\n", argv[0]);
    printf("%s\n", DESCRIPTION);
    printf("%s\n", VERSION);
    printf("-n#      number of tuples to generate "
                    "(default: %d)\n", tplcnt);
    printf("-s#      seed for random number generator "
                    "(default: time)\n");
    printf("-L#      Laplace correction "
                    "(default: as specified in classifier)\n");
    printf("-v/V     (do not) distribute tuple weight "
                    "for unknown values\n");
    printf("-m/M     (do not) use maximum likelihood estimate "
                    "for the variance\n");
    printf("-a       align fields (default: do not align)\n");
    printf("-w       do not write field names to the output file\n");
    printf("-b/f/r#  blank character, field and record separator\n"
           "         (default: \" \", \" \", \"\\n\")\n");
    printf("bcfile   file containing classifier description\n");
    printf("tabfile  table file to write\n");
    return 0;                   /* print a usage message */
  }                             /* and abort the program */

  /* --- evaluate arguments --- */
  for (i = 1; i < argc; i++) {  /* traverse arguments */
    s = argv[i];                /* get option argument */
    if (optarg) { *optarg = s; optarg = NULL; continue; }
    if ((*s == '-') && *++s) {  /* -- if argument is an option */
      while (*s) {              /* traverse options */
        switch (*s++) {         /* evaluate option */
          case 'n': tplcnt  = (int)strtol(s, &s, 0); break;
          case 's': seed    =      strtol(s, &s, 0); break;
          case 'L': lcorr   = strtod(s, &s);         break;
          case 'v': distuv  = NBC_ALL;               break;
          case 'V': distuv |= NBC_DISTUV|NBC_ALL;    break;
          case 'm': maxllh  = NBC_ALL;               break;
          case 'M': maxllh |= NBC_MAXLLH|NBC_ALL;    break;
          case 'a': flags  |= AS_ALIGN;              break;
          case 'w': flags  &= ~AS_ATT;               break;
          case 'b': optarg  = &blank;                break;
          case 'f': optarg  = &fldsep;               break;
          case 'r': optarg  = &recsep;               break;
          default : error(E_OPTION, *--s);           break;
        }                       /* set option variables */
        if (!*s) break;         /* if at end of string, abort loop */
        if (optarg) { *optarg = s; optarg = NULL; break; }
      } }                       /* get option argument */
    else {                      /* if argument is no option */
      switch (k++) {            /* evaluate non-option */
        case  0: fn_bc  = s;      break;
        case  1: fn_out = s;      break;
        default: error(E_ARGCNT); break;
      }                         /* note filenames */
    }
  }
  if (optarg) error(E_OPTARG);  /* check the option argument */
  if (k != 2) error(E_ARGCNT);  /* and the number of arguments */
  if ((lcorr < 0) && (lcorr > -DBL_MAX))
    error(E_NEGLC);             /* check the Laplace correction */
  if ((flags & AS_ATT) && (flags & AS_ALIGN))
    flags |= AS_ALNHDR;         /* set align to header flag */

  /* --- read Bayes classifier --- */
  scan = sc_create(fn_bc);      /* create a scanner */
  if (!scan) error((!fn_bc || !*fn_bc) ? E_NOMEM : E_FOPEN, fn_bc);
  attset = as_create("domains", att_delete);
  if (!attset) error(E_NOMEM);  /* create an attribute set */
  fprintf(stderr, "\nreading %s ... ", sc_fname(scan));
  if ((sc_nexter(scan)   <  0)  /* start scanning (get first token) */
  ||  (as_parse(attset, scan, AT_ALL) != 0)
  ||  (as_attcnt(attset) <= 0)) /* parse attribute set */
    error(E_PARSE, sc_fname(scan));
  if ((sc_token(scan) == T_ID)  /* determine classifier type */
  &&  (strcmp(sc_value(scan), "fbc") == 0))
       fbc = fbc_parse(attset, scan);
  else nbc = nbc_parse(attset, scan);
  if ((!fbc && !nbc)            /* parse the Bayes classifier */
  ||   !sc_eof(scan))           /* and check for end of file */
    error(E_PARSE, sc_fname(scan));
  sc_delete(scan); scan = NULL; /* delete the scanner */
  fprintf(stderr, "[%d attribute(s)] done.\n", as_attcnt(attset));
  if ((lcorr >= 0) || distuv || maxllh) {
    if (lcorr < 0)              /* get the classifier's parameters */
      lcorr = (fbc) ? fbc_lcorr(fbc) : nbc_lcorr(nbc);
    mode    = (fbc) ? fbc_mode(fbc)  : nbc_mode(nbc);
    if (distuv) mode = (mode & ~NBC_DISTUV) | distuv;
    if (maxllh) mode = (mode & ~NBC_MAXLLH) | maxllh;
                                /* adapt the estimation parameters */
    if (fbc) fbc_setup(fbc, mode, lcorr);
    else     nbc_setup(nbc, mode, lcorr);
  }                             /* set up the classifier anew */

  /* --- generate database --- */
  if (fn_out && *fn_out)        /* if an output file name is given, */
    out = fopen(fn_out, "w");   /* open output file for writing */
  else {                        /* if no output file name is given, */
    out = stdout; fn_out = "<stdout>"; }    /* write to std. output */
  fprintf(stderr, "writing %s ... ", fn_out);
  if (!out) error(E_FOPEN, fn_out);
  if ((flags & AS_ATT)          /* if to write a table header */
  &&  (as_write(attset, out, flags) != 0))
    error(E_FWRITE, fn_out);    /* write the attributes names */
  flags = AS_INST | (flags & ~AS_ATT);
  dseed(seed);                  /* init. random number generator */
  for (i = tplcnt; --i >= 0;) { /* generate random tuples */
    if (fbc) fbc_rand(fbc, drand);   /* instantiate the */
    else     nbc_rand(nbc, drand);   /* attribute set */
    if (as_write(attset, out, flags) != 0)
      error(E_FWRITE,fn_out);   /* write the generated tuple */
  }                             /* to the output file */
  if (out != stdout) {          /* if not written to stdout */
    i = fclose(out); out = NULL;/* close the output file */
    if (i != 0) error(E_FWRITE, fn_out);
  }                             /* print a success message */
  fprintf(stderr, "[%d tuple(s)] done.\n", tplcnt);

  /* --- clean up --- */
  #ifndef NDEBUG
  if (fbc) fbc_delete(fbc, 1);  /* delete full  Bayes classifier */
  if (nbc) nbc_delete(nbc, 1);  /* or     naive Bayes classifier */
  #endif                        /* and underlying attribute set */
  #ifdef STORAGE
  showmem("at end of program"); /* check memory usage */
  #endif
  return 0;                     /* return 'ok' */
}  /* main() */
