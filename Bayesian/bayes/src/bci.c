/*----------------------------------------------------------------------
  File    : bci.c
  Contents: naive and full Bayes classifier induction
  Author  : Christian Borgelt
  History : 08.12.1998 file created from file dti.c
            13.02.1999 input from stdin, output to stdout added
            25.03.1999 weight distribution (option -w) added
            17.04.1999 simplified using the new module 'io'
            15.05.1999 check of class count moved behind table reading
            21.11.2000 adapted to redesigned module nbayes
            30.11.2000 full Bayes classifier induction added
            11.02.2001 bug in attribute output (full Bayes) fixed
            16.07.2001 adapted to modified module scan
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
#ifndef AS_PARSE
#define AS_PARSE
#endif
#ifndef TAB_RDWR
#define TAB_RDWR
#endif
#include "io.h"
#ifndef NBC_INDUCE
#define NBC_INDUCE
#endif
#include "nbayes.h"
#ifndef FBC_INDUCE
#define FBC_INDUCE
#endif
#include "fbayes.h"
#ifdef STORAGE
#include "storage.h"
#endif

/*----------------------------------------------------------------------
  Preprocessor Definitions
----------------------------------------------------------------------*/
#define PRGNAME     "bci"
#define DESCRIPTION "naive and full Bayes classifier induction"
#define VERSION     "version 2.5 (2002.06.18)         " \
                    "(c) 1998-2002   Christian Borgelt"

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
#define E_PARSE     (-9)        /* parse error */
#define E_BALANCE  (-10)        /* unknown balancing mode */
#define E_SIMP     (-11)        /* unknown simplification mode */
#define E_CLASS    (-12)        /* missing class */
#define E_CLSTYPE  (-13)        /* class is not symbolic */
#define E_UNKNOWN  (-14)        /* unknown error */

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
  /* E_PARSE    -9 */  "parse error(s) on file %s\n",
  /* E_BALANCE -10 */  "unknown balancing mode %c\n",
  /* E_SIMP    -11 */  "unknown simplification mode %c\n",
  /* E_CLASS   -12 */  "missing class \"%s\" in file %s\n",
  /* E_CLSTYPE -13 */  "class \"%s\" is not symbolic\n",
  /* E_UNKNOWN -14 */  "unknown error\n"
};

/*----------------------------------------------------------------------
  Global Variables
----------------------------------------------------------------------*/
const  char   *prgname = NULL;  /* program name for error messages */
static SCAN   *scan    = NULL;  /* scanner */
static ATTSET *attset  = NULL;  /* attribute set */
static TABLE  *table   = NULL;  /* table */
static NBC    *nbc     = NULL;  /* naive Bayes classifier */
static FBC    *fbc     = NULL;  /* full  Bayes classifier */
static FILE   *in      = NULL;  /* input  file */
static FILE   *out     = NULL;  /* output file */

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
  if (nbc)    nbc_delete(nbc, 0);
  if (fbc)    fbc_delete(fbc, 0);
  if (attset) as_delete(attset);
  if (table)  tab_delete(table, 0);  /* clean up memory */
  if (scan)   sc_delete(scan);       /* and close files */
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
  int    i, k = 0;              /* loop variables, counter, buffer */
  char   *s;                    /* to traverse options */
  char   **optarg = NULL;       /* option argument */
  char   *fn_hdr  = NULL;       /* name of table header file */
  char   *fn_tab  = NULL;       /* name of table file */
  char   *fn_dom  = NULL;       /* name of domain file */
  char   *fn_bc   = NULL;       /* name of classifier file */
  char   *blanks  = NULL;       /* blanks */
  char   *fldseps = NULL;       /* field  separators */
  char   *recseps = NULL;       /* record separators */
  char   *uvchars = NULL;       /* unknown value characters */
  char   *clscol  = NULL;       /* class column name */
  int    full     = 0;          /* flag for a full Bayes classifier */
  int    flags    = AS_NOXATT;  /* table file read flags */
  int    balance  = 0;          /* flag for balancing class freqs. */
  int    simp     = 0;          /* flag for classifier simplification */
  double lcorr    = 0;          /* Laplace correction value */
  int    maxlen   = 0;          /* maximal output line length */
  int    setup    = 0;          /* setup/induction mode */
  int    desc     = 0;          /* description mode */
  int    attcnt   = 0;          /* number of attributes */
  int    tplcnt   = 0;          /* number of tuples */
  double tplwgt   = 0.0;        /* weight of tuples */
  int    clsid;                 /* id of class column */
  ATT    *att;                  /* to traverse attributes */
  TFSERR *err;                  /* error information */

  prgname = argv[0];            /* get program name for error msgs. */

  /* --- print startup/usage message --- */
  if (argc > 1) {               /* if arguments are given */
    fprintf(stderr, "%s - %s\n", argv[0], DESCRIPTION);
    fprintf(stderr, VERSION); } /* print a startup message */
  else {                        /* if no argument given */
    printf("usage: %s [options] domfile "
                     "[-d|-h hdrfile] tabfile bcfile\n", argv[0]);
    printf("%s\n", DESCRIPTION);
    printf("%s\n", VERSION);
    printf("-F       induce a full Bayes classifier "
                    "(default: naive Bayes)\n");
    printf("-c#      class field name (default: last field)\n");
    printf("-w#      balance class frequencies (weight tuples)\n");
    printf("         l: lower, b: boost, s: shift weights\n");
    printf("-s#      simplify classifier (naive Bayes only)\n"
           "         a: by adding, r: by removing attributes\n");
    printf("-L#      Laplace correction (default: %g)\n", lcorr);
    printf("-t       distribute tuple weight for unknown values\n");
    printf("-m       use maximum likelihood estimate "
                    "for the variance\n");
    printf("-p       print relative frequencies (in percent)\n");
    printf("-l#      output line length (default: no limit)\n");
    printf("-b/f/r#  blank characters, field and record separators\n"
           "         (default: \" \\t\\r\", \" \\t\", \"\\n\")\n");
    printf("-u#      unknown value characters (default: \"?\")\n");
    printf("-n       number of tuple occurrences in last field\n");
    printf("domfile  file containing domain descriptions\n");
    printf("-d       use default table header "
                    "(field names = field numbers)\n");
    printf("-h       read table header (field names) from hdrfile\n");
    printf("hdrfile  file containing table header (field names)\n");
    printf("tabfile  table file to read "
                    "(field names in first record)\n");
    printf("bcfile   file to write Bayes classifier to\n");
    return 0;                   /* print a usage message */
  }                             /* and abort the program */

  /* --- evaluate arguments --- */
  for (i = 1; i < argc; i++) {  /* traverse arguments */
    s = argv[i];                /* get option argument */
    if (optarg) { *optarg = s; optarg = NULL; continue; }
    if ((*s == '-') && *++s) {  /* -- if argument is an option */
      while (1) {               /* traverse characters */
        switch (*s++) {         /* evaluate option */
          case 'F': full    = 1;                     break;
          case 'c': optarg  = &clscol;               break;
          case 'w': balance = (*s) ? *s++ : 0;       break;
          case 's': simp    = (*s) ? *s++ : 0;       break;
          case 'L': lcorr   =      strtod(s, &s);    break;
          case 't': setup  |= NBC_DISTUV;            break;
          case 'm': setup  |= NBC_MAXLLH;            break;
          case 'p': desc   |= NBC_REL;               break;
          case 'l': maxlen  = (int)strtol(s, &s, 0); break;
          case 'b': optarg  = &blanks;               break;
          case 'f': optarg  = &fldseps;              break;
          case 'r': optarg  = &recseps;              break;
          case 'u': optarg  = &uvchars;              break;
          case 'n': flags  |= AS_WEIGHT;             break;
          case 'd': flags  |= AS_DFLT;               break;
          case 'h': optarg  = &fn_hdr;               break;
          default : error(E_OPTION, *--s);           break;
        }                       /* set option variables */
        if (!*s) break;         /* if at end of string, abort loop */
        if (optarg) { *optarg = s; optarg = NULL; break; }
      } }                       /* get option argument */
    else {                      /* -- if argument is no option */
      switch (k++) {            /* evaluate non-option */
        case  0: fn_dom = s;      break;
        case  1: fn_tab = s;      break;
        case  2: fn_bc  = s;      break;
        default: error(E_ARGCNT); break;
      }                         /* note filenames */
    }
  }
  if (optarg) error(E_OPTARG);  /* check the option argument */
  if (k != 3) error(E_ARGCNT);  /* and the number of arguments */
  if (fn_hdr && (strcmp(fn_hdr, "-") == 0))
    fn_hdr = "";                /* convert "-" to "" */
  i = (!fn_dom || !*fn_dom) ? 1 : 0;
  if  (!fn_tab || !*fn_tab) i++;
  if  ( fn_hdr && !*fn_hdr) i++;/* check assignments of stdin: */
  if (i > 1) error(E_STDIN);    /* stdin must not be used twice */
  if      (simp == 'a') setup |= NBC_ADD;
  else if (simp == 'r') setup |= NBC_REMOVE;
  else if (simp !=  0 )         /* check simplification mode */
    error(E_SIMP, simp);        /* (must be 'add' or 'remove') */
  if ((balance !=  0)  && (balance != 'l')
  &&  (balance != 'b') && (balance != 's'))
    error(E_BALANCE, balance);  /* check balancing mode */
  if (fn_hdr)                   /* set the header file flag */
    flags = AS_ATT | (flags & ~AS_DFLT);

  /* --- read attribute set --- */
  scan = sc_create(fn_dom);     /* create a scanner */
  if (!scan) error((!fn_dom || !*fn_dom) ? E_NOMEM : E_FOPEN, fn_dom);
  attset = as_create("domains", att_delete);
  if (!attset) error(E_NOMEM);  /* create an attribute set */
  fprintf(stderr, "\nreading %s ... ", sc_fname(scan));
  if ((sc_nexter(scan)   <  0)  /* start scanning (get first token) */
  ||  (as_parse(attset, scan, AT_ALL) != 0)
  ||  (as_attcnt(attset) <= 0)  /* parse attribute set and */
  ||  !sc_eof(scan))            /* check for end of file */
    error(E_PARSE, sc_fname(scan));
  sc_delete(scan); scan = NULL; /* delete the scanner */
  attcnt = as_attcnt(attset);   /* get and print the number of atts. */
  fprintf(stderr, "[%d attribute(s)] done.\n", attcnt);

  /* --- determine id of class column --- */
  if (!clscol)                  /* if no class column name given, */
    clsid = attcnt -1;          /* use the last column as the class */
  else {                        /* if a class column name given */
    clsid = as_attid(attset, clscol);
    if (clsid < 0) error(E_CLASS, clscol, fn_dom);
  }                             /* check whether class exists */
  att = as_att(attset, clsid);  /* get the class attribute */
  if (att_type(att) != AT_SYM)  /* check the type of the class */
    error(E_CLSTYPE, att_name(att));     /* (must be symbolic) */

  /* --- read table header --- */
  as_chars(attset, blanks, fldseps, recseps, uvchars);
  in = io_hdr(attset, fn_hdr, fn_tab, flags, 1);
  if (!in) error(1);            /* read the table header */

  /* --- construct naive Bayes classifier --- */
  if (!full                     /* if to induce a naive Bayes class. */
  &&  (balance                  /* and to balance class frequencies */
  ||   simp)) {                 /* or to simplify the classifier */
    table = io_bodyin(attset, in, fn_tab, flags, "table", 1);
    if (!table) error(1);       /* read the table body */
    fprintf(stderr, "reducing%s table ... ",
                   (balance) ? " and balancing" : "");
    tab_reduce(table);          /* reduce table for speed up */
    if (balance) {              /* if the balance flag is set */
      tab_balance(table, clsid, (balance == 'l') ? -2.0F
                              : (balance == 'b') ? -1.0F : 0.0F, NULL);
    }                           /* balance the class frequencies */
    tplwgt = tab_getwgt(table, 0, INT_MAX);
    fprintf(stderr, "[%d/", tab_tplcnt(table));
    fprintf(stderr, "%g tuple(s)] done.\n", tplwgt);
    fprintf(stderr, "building classifier ... ");
    nbc = nbc_induce(table, clsid, setup, lcorr);
    if (!nbc) error(E_NOMEM);   /* induce a classifier and */
    attcnt = nbc_mark(nbc);     /* mark the selected attributes */
    fprintf(stderr, "done.\n");}/* print a success message */
  else {                        /* if to build a normal classifier */
    if (full) fbc = fbc_create(attset, clsid);
    else      nbc = nbc_create(attset, clsid);
    if (!fbc && !nbc)           /* create either a full or */
      error(E_NOMEM);           /* a naive Bayes classifier */
    k = AS_INST | (flags & ~(AS_ATT|AS_DFLT));
    i = ((flags & AS_DFLT) && !(flags & AS_ATT))
      ? 0 : as_read(attset, in, k);
    while (i == 0) {            /* record read loop */
      if (((fbc) ? fbc_add(fbc, NULL) : nbc_add(nbc, NULL)) != 0)
        error(E_NOMEM);         /* process tuple and count it */
      tplcnt++; tplwgt += as_getwgt(attset);
      i = as_read(attset, in, k);
    }                           /* try to read the next record */
    if (i < 0) {                /* if an error occurred, */
      err = as_err(attset);     /* get the error information */
      tplcnt += (flags & (AS_ATT|AS_DFLT)) ? 1 : 2;
      io_error(i, fn_tab, tplcnt, err->s, err->fld, err->exp);
      error(1);                 /* print an error message */
    }                           /* and abort the program */
    if (in != stdin) fclose(in);/* close the input file */
    in = NULL;                  /* and set up the classifier */
    if (fbc) { fbc_setup(fbc, setup, lcorr); attcnt = fbc_mark(fbc); }
    else     { nbc_setup(nbc, setup|NBC_ALL, lcorr); }
    fprintf(stderr, "[%d/%g tuple(s)] done.\n", tplcnt, tplwgt);
  }                             /* print a success message */

  /* --- describe created classifier --- */
  if (fn_bc && *fn_bc)          /* if an output file name is given, */
    out = fopen(fn_bc, "w");    /* open output file for writing */
  else {                        /* if no output file name is given, */
    out = stdout; fn_bc = "<stdout>"; }     /* write to std. output */
  fprintf(stderr, "writing %s ... ", fn_bc);
  if (!out) error(E_FOPEN, fn_bc);
  k = (full || simp)            /* print only the class and */
    ? AS_MARKED : 0;            /* the marked attributes */
  if (as_desc(attset, out, k|AS_TITLE|AS_IVALS, maxlen) != 0)
    error(E_FWRITE, fn_bc);     /* describe attribute domains */
  fputc('\n', out);             /* leave one line empty */
  k = (simp) ? NBC_MARKED : 0;  /* print only marked attributes */
  if (((fbc) ? fbc_desc(fbc,  out, desc  |FBC_TITLE, maxlen)
  :            nbc_desc(nbc,  out, desc|k|NBC_TITLE, maxlen)) != 0)
    error(E_FWRITE, fn_bc);     /* describe Bayes classifier */
  if (maxlen <= 0) maxlen = 72; /* determine maximal line length */
  fputs("\n/*", out);           /* append additional information */
  for (k = maxlen -2; --k >= 0; ) fputc('-', out);
  fprintf(out, "\n  number of attributes: %d",   attcnt);
  fprintf(out, "\n  number of tuples    : %g\n", tplwgt);
  for (k = maxlen -2; --k >= 0; ) fputc('-', out);
  fputs("*/\n", out);           /* terminate additional information */
  if (out != stdout) {          /* if not written to stdout, */
    k = fclose(out); out = NULL;/* close the output file */
    if (k) error(E_FWRITE, fn_bc);
  }                             /* print a success message */
  fprintf(stderr, "[%d attribute(s)] done.\n", attcnt);

  /* --- clean up --- */
  #ifndef NDEBUG
  if (table) tab_delete(table, 0);  /* delete table, */
  if (nbc)   nbc_delete(nbc, 1);    /* naive Bayes classifier, */
  if (fbc)   fbc_delete(fbc, 1);    /* full Bayes classifier, */
  #endif                            /* and underlying attribute set */
  #ifdef STORAGE
  showmem("at end of program"); /* check memory usage */
  #endif
  return 0;                     /* return 'ok' */
}  /* main() */
