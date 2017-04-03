/*----------------------------------------------------------------------
  File    : tbal.c
  Contents: program to balance value frequencies
  Author  : Christian Borgelt
  History : 13.02.1999 file created from file skel1.c
            17.04.1999 simplified using the new module 'io'
            14.07.2001 adapted to modified module tfscan
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
#define PRGNAME     "tbal"
#define DESCRIPTION "balance value frequencies"
#define VERSION     "version 1.5 (2003.08.16)         " \
                    "(c) 1999-2003   Christian Borgelt"

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
#define E_VALEXP    (-8)        /* value expected */
#define E_NUMEXP    (-9)        /* number expected */
#define E_NUMBER   (-10)        /* illegal number */
#define E_UNKNOWN  (-11)        /* unknown error */

#define TMFIELDS   "too many fields"

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
  /* E_VALEXP   -8 */  "file %s, record %d: value expected\n",
  /* E_NUMEXP   -9 */  "file %s, record %d: number expected\n",
  /* E_NUMBER  -10 */  "file %s, record %d: "
                          "illegal number \"%s\" in field %d\n",
  /* E_UNKNOWN -11 */  "unknown error\n"
};

/*----------------------------------------------------------------------
  Global Variables
----------------------------------------------------------------------*/
const  char   *prgname = NULL;  /* program name for error messages */
static ATTSET *attset  = NULL;  /* attribute set */
static TABLE  *table   = NULL;  /* table */
static FILE   *in      = NULL;  /* input  file */
static double *freqs   = NULL;  /* value frequency vector */
static char   buf[AS_MAXLEN+1]; /* read buffer */

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
  if (freqs)  free(freqs);      /* and close files */
  if (table)  tab_delete(table, 0);
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
  char   *fn_frq  = NULL;       /* name of frequency file */
  char   *fn_out  = NULL;       /* name of output file */
  char   *blanks  = NULL;       /* blank  characters */
  char   *fldseps = NULL;       /* field  separators */
  char   *recseps = NULL;       /* record separators */
  char   *uvchars = NULL;       /* unknown value characters */
  char   *clscol  = NULL;       /* name of class column to balance */
  int    inflags  = 0;                 /* table file read  flags */
  int    outflags = AS_ATT|AS_WEIGHT;  /* table file write flags */
  double wgtsum   = 0;          /* weight of tuples in output table */
  int    valcnt;                /* number of attribute values */
  int    clsid;                 /* id of class column */
  ATT    *att;                  /* class attribute */
  int    d;                     /* delimiter type */

  prgname = argv[0];            /* get program name for error msgs. */

  /* --- print startup/usage message --- */
  if (argc > 1) {               /* if arguments are given */
    fprintf(stderr, "%s - %s\n", argv[0], DESCRIPTION);
    fprintf(stderr, VERSION); } /* print a startup message */
  else {                        /* if no argument is given */
    printf("usage: %s [options] [-q frqfile] "
                     "[-d|-h hdrfile] tabfile outfile\n", argv[0]);
    printf("%s\n", DESCRIPTION);
    printf("%s\n", VERSION);
    printf("-c#      name of field to balance (default: last field)\n");
    printf("-s#      sum of tuple weights in output table "
                    "(default: as in input table)\n");
    printf("         (-2: lower, -1: boost, 0: shift weights)\n");
    printf("-q       adjust to relative frequencies stated in frqfile\n");
    printf("frqfile  file containing value/relative frequency pairs\n");
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
    printf("tabfile  table file to read "
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
          case 'c': optarg    = &clscol;       break;
          case 's': wgtsum    = strtod(s, &s); break;
          case 'q': optarg    = &fn_frq;       break;
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

  /* --- determine id of class column --- */
  if (!clscol)                  /* if no class column name given, */
    clsid = as_attcnt(attset) -1;     /* use last column as class */
  else {                        /* if class column name given */
    if ((clsid = as_attid(attset, clscol)) < 0) {
      s = (inflags & AS_ATT) ? fn_hdr : fn_tab;
      io_error(E_MISFLD, s, 1, clscol); error(1);
    }                           /* check whether class exists */
  }                             /* and abort on error */

  /* --- read table --- */
  table = io_bodyin(attset, in, fn_tab, inflags, "table", 1);
  in    = NULL;                 /* read the table and */
  if (!table) error(1);         /* check for an error */

  /* --- balance frequencies --- */
  if (fn_frq) {                 /* if frequencies are given */
    if (*fn_frq)                /* if a proper file name is given, */
      in = fopen(fn_frq, "rb"); /* open frequency file for reading */
    else {                      /* if no proper file name is given, */
      in = stdin; fn_frq = "<stdin>"; } /* read from standard input */
    fprintf(stderr, "reading %s ... ", fn_frq);
    if (!in) error(E_FOPEN, fn_frq);
    att    = as_att(attset, clsid);
    valcnt = att_valcnt(att);   /* get att. and number of values */
    freqs  = (double*)malloc(valcnt *sizeof(double));
    if (!freqs) error(E_NOMEM); /* allocate a frequency vector */
    for (i = valcnt; --i >= 0; ) freqs[i] = 1.0F;
    for (k = 0; 1; k++) {       /* frequency read loop */
      d = tfs_getfld(as_tfscan(attset), in, buf, AS_MAXLEN);
      if (d <= TFS_EOF) {       /* read next value */
        if (d < 0) error(E_FREAD, fn_frq); else break; }
      if (buf[0] == '\0') {     /* if name read is empty */
        if (d >= TFS_FLD) error(E_VALEXP, fn_frq, k+1);
        continue;               /* check for a missing name */
      }                         /* and skip empty lines */
      if (d < TFS_FLD) error(E_NUMEXP, fn_frq, k+1);
      i = att_valid(att, buf);  /* get the value identifier */
      if (i <  0) {             /* and check it */
        io_error(E_VALUE, fn_frq, k+1, buf, 1); error(1); }
      d = tfs_getfld(as_tfscan(attset), in, buf, AS_MAXLEN);
      if (d <  0) error(E_FREAD, fn_frq);
      if (d >= TFS_FLD) {       /* check the number of fields */
        io_error(E_FLDCNT, fn_frq, k+1, "", 3, 2); error(1); }
      freqs[i] = strtod(buf, &s);    /* read the value frequency */
      if ((s == buf) || *s || (freqs[i] < 0)) {
        io_error(E_NUMBER, fn_frq, k+1, buf, 2); error(1); }
    }                           /* convert frequency to a number */
    if (in != stdin) fclose(in);/* close the input file */
    in = NULL;                  /* and clear the variable */
    fprintf(stderr, "done.\n"); /* print a success message */
  }
  tab_reduce(table);            /* reduce and balance the table */
  tab_balance(table, clsid, wgtsum, freqs);

  /* --- write output table --- */
  if (io_tabout(table, fn_out, outflags, 1) != 0)
    error(1);                   /* write the balanced table */

  /* --- clean up --- */
  #ifndef NDEBUG
  if (freqs) free(freqs);       /* delete frequency vector, */
  tab_delete(table, 1);         /* table, and attribute set */
  #endif
  #ifdef STORAGE
  showmem("at end of program"); /* check memory usage */
  #endif
  return 0;                     /* return 'ok' */
}  /* main() */
