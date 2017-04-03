/*----------------------------------------------------------------------
  File    : t1inn.c
  Contents: program to convert symbolic attributes to 1-in-n coding
  Author  : Christian Borgelt
  History : 11.08.2003 file created
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
#include "io.h"
#include "attmap.h"
#ifdef STORAGE
#include "storage.h"
#endif

/*----------------------------------------------------------------------
  Preprocessor Definitions
----------------------------------------------------------------------*/
#define PRGNAME     "t1inn"
#define DESCRIPTION "convert symbolic attributes to 1-in-n coding"
#define VERSION     "version 1.0 (2003.08.11)         " \
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
#define E_STDIN     (-8)        /* double assignment of stdin */
#define E_PARSE     (-9)        /* parse error(s) */
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
  /* E_STDIN    -8 */  "double assignment of standard input\n",
  /* E_PARSE    -9 */  "parse error(s) on file %s\n",
  /* E_UNKNOWN -10 */  "unknown error\n"
};

/*----------------------------------------------------------------------
  Global Variables
----------------------------------------------------------------------*/
const  char   *prgname = NULL;  /* program name for error messages */
static SCAN   *scan    = NULL;  /* scanner */
static ATTSET *attset  = NULL;  /* attribute set */
static ATTMAP *attmap  = NULL;  /* attribute map */
static double *vec     = NULL;  /* vector of mapped values */
static FILE   *in      = NULL;  /* input  file */
static FILE   *out     = NULL;  /* output file */

/*----------------------------------------------------------------------
  Main Functions
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
  if (scan)   sc_delete(scan);  /* and close files */
  if (attset) as_delete(attset);
  if (attmap) am_delete(attmap);
  if (vec)    free(vec);
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
  int    i, k = 0, n, f;        /* loop variables, counter */
  char   *s;                    /* to traverse the options */
  char   **optarg = NULL;       /* option argument */
  char   *fn_dom  = NULL;       /* name of domains file */
  char   *fn_hdr  = NULL;       /* name of table header file */
  char   *fn_in   = NULL;       /* name of input  file */
  char   *fn_out  = NULL;       /* name of output file */
  char   *blanks  = NULL;       /* blanks characters */
  char   *fldseps = NULL;       /* field  separators */
  char   *recseps = NULL;       /* record separators */
  char   *uvchars = NULL;       /* unknown value characters */
  int    inflags  = AS_NOXATT;  /* table file read flags */
  int    outflags = AS_ATT;     /* table file write flags */
  int    tplcnt   = 0;          /* number of tuples */
  double tplwgt   = 0.0;        /* weight of tuples */
  char   *fmt     = "%g";       /* output format for numbers */
  float  wgt;                   /* tuple/instantiation weight */
  CCHAR  *seps;                 /* separator characters */
  CCHAR  *name;                 /* attribute name */
  TFSERR *err;                  /* error information */

  prgname = argv[0];            /* get program name for error msgs. */

  /* --- print startup/usage message --- */
  if (argc > 1) {               /* if arguments are given */
    fprintf(stderr, "%s - %s\n", argv[0], DESCRIPTION);
    fprintf(stderr, VERSION); } /* print a startup message */
  else {                        /* if no argument is given */
    printf("usage: %s [options] domfile "
                     "[-d|-h hdrfile] infile outfile\n", argv[0]);
    printf("%s\n", DESCRIPTION);
    printf("%s\n", VERSION);
    printf("-o#      number output format (default: \"%s\")\n", fmt);
    printf("-w       do not write field names to output file\n");
    printf("-b/f/r#  blank characters, field and record separators\n"
           "         (default: \" \\t\\r\", \" \\t\", \"\\n\")\n");
    printf("-u#      unknown value characters (default: \"?\")\n");
    printf("-n       number of tuple occurrences in last field\n");
    printf("-d       use default header "
                    "(field names = field numbers)\n");
    printf("domfile  file containing domain descriptions\n");
    printf("-h       read table header (field names) from hdrfile\n");
    printf("hdrfile  file containing table header (field names)\n");
    printf("infile   table file to read "
                    "(field names in first record)\n");
    printf("outfile  table file to write\n");
    return 0;                   /* print a usage message */
  }                             /* and abort the program */

  /* --- evaluate arguments --- */
  for (i = 1; i < argc; i++) {  /* traverse the arguments */
    s = argv[i];                /* get option argument */
    if (optarg) { *optarg = s; optarg = NULL; continue; }
    if ((*s == '-') && *++s) {  /* -- if argument is an option */
      while (1) {               /* traverse characters */
        switch (*s++) {         /* evaluate option */
          case 'o': optarg    = &fmt;      break;
          case 'w': outflags &= ~AS_ATT;   break;
  	  case 'b': optarg    = &blanks;   break;
          case 'f': optarg    = &fldseps;  break;
          case 'r': optarg    = &recseps;  break;
          case 'u': optarg    = &uvchars;  break;
          case 'n': outflags |= AS_WEIGHT;
                    inflags  |= AS_WEIGHT; break;
          case 'd': inflags  |= AS_DFLT;   break;
          case 'h': optarg    = &fn_hdr;   break;
          default : error(E_OPTION, *--s); break;
        }                       /* set option variables */
        if (!*s) break;         /* if at end of string, abort loop */
        if (optarg) { *optarg = s; optarg = NULL; break; }
      } }                       /* get option argument */
    else {                      /* -- if argument is no option */
      switch (k++) {            /* evaluate non-option */
        case  0: fn_dom = s;      break;
        case  1: fn_in  = s;      break;
        case  2: fn_out = s;      break;
        default: error(E_ARGCNT); break;
      }                         /* note filenames */
    }
  }
  if (optarg) error(E_OPTARG);  /* check option argument */
  if (k != 3) error(E_ARGCNT);  /* check number of arguments */
  if (fn_hdr && (strcmp(fn_hdr, "-") == 0))
    fn_hdr = "";                /* convert "-" to "" */
  i = (!fn_dom || !*fn_dom) ? 1 : 0;
  if (fn_in  && !*fn_in)  i++;
  if (fn_hdr && !*fn_hdr) i++;  /* check assignments of stdin: */
  if (i > 1) error(E_STDIN);    /* stdin must not be used twice */
  if (fn_hdr)                   /* set the header file flag */
    inflags = AS_ATT | (inflags & ~AS_DFLT);
  if ((outflags & AS_ATT) && (outflags & AS_ALIGN))
    outflags |= AS_ALNHDR;      /* set align to header flag */

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
  attmap = am_create(attset, 0);
  if (!attset) error(E_NOMEM);  /* create an attribute map */
  vec = (double*)malloc(am_dim(attmap) *sizeof(double));
  if (!vec)    error(E_NOMEM);  /* create an output vector */
  fprintf(stderr, "[%d attribute(s)] done.\n", as_attcnt(attset));

  /* --- read table header --- */
  seps = as_chars(attset, blanks, fldseps, recseps, uvchars);
  in = io_hdr(attset, fn_hdr, fn_in, inflags|AS_MARKED, 1);
  if (!in) error(1);            /* read the table header */
  if ((outflags & AS_ALIGN)     /* if to align output file */
  &&  (in != stdin)) {          /* and not to read from stdin */
    i = AS_INST | (inflags & ~(AS_ATT|AS_DFLT));
    while (as_read(attset, in, i) == 0);
    fclose(in);                 /* determine the column widths */
    in = io_hdr(attset, fn_hdr, fn_in, inflags|AS_MARKED, 1);
    if (!in) error(1);          /* reread the table header */
  }                             /* (necessary because of first tuple) */

  /* --- write output file --- */
  if (fn_out && *fn_out)        /* if an output file name is given, */
    out = fopen(fn_out, "w");   /* open output file for writing */
  else {                        /* if no output file name is given, */
    out = stdout; fn_out = "<stdout>"; }         /* write to stdout */
  if (!out) error(E_FOPEN, fn_out);
  if (outflags & AS_ATT) {      /* if to write table header */
    for (i = 0; i < am_attcnt(attmap); i++) {
      if (i > 0) fputc(seps[1], out);  /* print a separator */
      name = att_name(as_att(attset, i));
      n = am_cnt(attmap, i);    /* get name and column counter */
      if (n <= 1) {             /* single column: print only name */
        fputs(name, out); continue; }
      for (k = 1; k <= n; k++){ /* multiple column: */
        if (k > 1) fputc(seps[1], out);
        fprintf(out, "%s_%d", name, k);
      }                         /* print a separator and */
    }                           /* name and column counter */
    if (outflags & AS_WEIGHT) { /* print a weight indicator */
      fputc(seps[1], out); fputc('#', out); }
    fputc(seps[2], out);        /* terminate the output line */
  }
  n = am_dim(attmap);           /* initialize the read flags */
  f = AS_INST|(inflags & ~(AS_ATT|AS_DFLT));
  i = ((inflags & AS_DFLT) && !(inflags & AS_ATT))
    ? 0 : as_read(attset, in, f);
  while (i == 0) {              /* record read loop */
    wgt = as_getwgt(attset);    /* get the tuple weight, count */
    tplwgt += wgt; tplcnt++;    /* the tuple, and sum its weight */
    am_exec(attmap, NULL, vec); /* execute the attribute map */
    for (k = 0; k < n; k++) {   /* traverse the output vector */
      if (k > 0) fputc(seps[1], out);
      fprintf(out, fmt,vec[k]); /* print a field separator */
    }                           /* and the vector element */
    fputc(seps[2], out);        /* terminate the output line */
    i = as_read(attset, in, f); /* try to read the next record */
  }
  if (i < 0) {                  /* if an error occurred, */
    err = as_err(attset);       /* get the error information */
    tplcnt += (inflags & (AS_ATT|AS_DFLT)) ? 1 : 2;
    io_error(i, fn_in, tplcnt, err->s, err->fld, err->exp);
    error(1);                   /* print an error message */
  }                             /* and abort the program */
  if (in != stdin) fclose(in);  /* close the table file and */
  in = NULL;                    /* clear the file variable */
  if (out != stdout) {          /* if not written to stdout, */
    i = fclose(out); out = NULL;/* close the output file */
    if (i != 0) error(E_FWRITE, fn_out);
  }
  fprintf(stderr, "[%d/%g tuple(s)] done.\n", tplcnt, tplwgt);

  /* --- clean up --- */
  #ifndef NDEBUG
  as_delete(attset);            /* delete attribute set, */
  am_delete(attmap);            /* attribute map, */
  free(vec);                    /* and output vector */
  #endif
  #ifdef STORAGE
  showmem("at end of program"); /* check memory usage */
  #endif
  return 0;                     /* return 'ok' */
}  /* main() */
