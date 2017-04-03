/*----------------------------------------------------------------------
  File    : tmerge.c
  contents: program to merge tables / project a table / align a table
  Author  : Christian Borgelt
  History : 08.03.1996 file created as proj.c
            01.07.1996 alignment option removed
            22.11.1996 options -b, -f and -r added
            02.01.1998 column/field alignment (option -a) added
            07.01.1998 no header in projected table (option -c) added
            25.02.1998 table merging added and renamed to tmerge.c
            23.06.1998 extended to merging multiple tables
            12.09.1998 adapted to modified module attset
            25.09.1998 table reading simplified
            07.02.1999 input from stdin, output to stdout added
            12.02.1999 default header handling improved
            17.04.1999 simplified using the new module 'io'
            14.07.2001 adapted to modified module tfscan
            16.08.2003 slight changes in error message output
----------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>
#ifndef AS_RDWR
#define AS_RDWR
#endif
#include "io.h"
#ifdef STORAGE
#include "storage.h"
#endif

/*----------------------------------------------------------------------
  Preprocessor Definitions
----------------------------------------------------------------------*/
#define PRGNAME     "tmerge"
#define DESCRIPTION "merge tables / project / align a table"
#define VERSION     "version 1.9 (2003.08.16)         " \
                    "(c) 1996-2003   Christian Borgelt"

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
#define E_UNKNOWN   (-9)        /* unknown error */

/*----------------------------------------------------------------------
  Constants
----------------------------------------------------------------------*/
static const char *errmsgs[] = {    /* error messages */
  /* E_NONE     0 */  "no error\n",
  /* E_NOMEM   -1 */  "not enough memory\n",
  /* E_FOPEN   -2 */  "cannot open file %s\n",
  /* E_FREAD   -3 */  "read error on file %s\n",
  /* E_FWRITE  -4 */  "write error on file %s\n",
  /* E_OPTION  -5 */  "unknown option -%c\n",
  /* E_OPTARG  -6 */  "missing option argument\n",
  /* E_ARGCNT  -7 */  "wrong number of arguments\n",
  /* E_STDIN   -8 */  "double assignment of standard input\n",
  /* E_UNKNOWN -9 */  "unknown error\n"
};

/*----------------------------------------------------------------------
  Global Variables
----------------------------------------------------------------------*/
const  char   *prgname = NULL;  /* program name for error messages */
static ATTSET *attset  = NULL;  /* attribute set */
static FILE   *in      = NULL;  /* input  file */
static FILE   *out     = NULL;  /* output file */

/*----------------------------------------------------------------------
  Functions
----------------------------------------------------------------------*/

static void error (int code, ...)
{                               /* --- print an error message */
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
  if (attset) as_delete(attset);/* and close files */
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
  int    i, k = 0, r;           /* loop variables, counter */
  char   *s;                    /* to traverse options */
  char   **optarg = NULL;       /* option argument */
  char   *fn_hdr1 = NULL;       /* name of first  header file */
  char   *fn_hdr2 = NULL;       /* name of second header file */
  char   *blanks  = NULL;       /* blanks */
  char   *fldseps = NULL;       /* field  separators */
  char   *recseps = NULL;       /* record separators */
  char   *uvchars = "";         /* unknown value characters */
  char   *hdr     = NULL;       /* buffer for header file name */
  int    ignore   = 0;          /* ignore tuples of first table */
  int    inflags1 = 0;          /* first  table file read flags */
  int    inflags2 = 0;          /* second table file read flags */
  int    outflags = AS_ATT;     /* table file write flags */
  int    flags    = 0, f;       /* buffers for flags */
  int    verb     = 1;          /* flag for verbose message output */
  int    tplcnt, outcnt;        /* number of tuples read/written */
  double tplwgt, outwgt;        /* weight of tuples read/written */
  TFSERR *err;                  /* error information */

  prgname = argv[0];            /* get program name for error msgs. */

  /* --- print startup/usage message --- */
  if (argc > 1) {               /* if arguments are given */
    fprintf(stderr, "%s - %s\n", argv[0], DESCRIPTION);
    fprintf(stderr, VERSION); } /* print a startup message */
  else {                        /* if no argument is given */
    printf("usage: %s [options] [-d|-h hdr1] in1 "
                     "[-d|-h hdr2] [in2 [in3 ...]] out\n", argv[0]);
    printf("%s\n", DESCRIPTION);
    printf("%s\n", VERSION);
    printf("-i       ignore tuples of first input file "
                    "(use header only)\n");
    printf("-w       do not write field names to output file\n");
    printf("-a       align fields of output table "
                    "(default: do not align)\n");
    printf("-b/f/r#  blank characters, field and record separators\n"
           "         (default: \" \\t\\r\", \" \\t\", \"\\n\")\n");
    printf("-u#      unknown value characters (default: none)\n");
    printf("-n       number of tuple occurrences in last field\n");
    printf("-d       use default header "
                    "(field names = field numbers)\n");
    printf("-h       read table header (field names) from hdrfile\n");
    printf("hdr1/2   files containing table header (field names)\n");
    printf("in1/2/3  table files to read "
                    "(field names in first record)\n");
    printf("out      file to write merged tables/projected table to\n");
    return 0;                   /* print usage message */
  }                             /* and abort program */

  /* --- evaluate arguments --- */
  for (i = 1; i < argc; i++) {  /* traverse arguments */
    s = argv[i];                /* get option argument */
    if (optarg) { *optarg = s; optarg = NULL; continue; }
    if ((*s == '-') && *++s) {  /* -- if argument is an option */
      while (1) {               /* traverse characters */
        switch (*s++) {         /* evaluate option */
          case 'i': ignore    = 1;                              break;
          case 'w': outflags &= ~AS_ATT;                        break;
          case 'a': outflags |= AS_ALIGN;                       break;
          case 'b': optarg    = &blanks;                        break;
          case 'f': optarg    = &fldseps;                       break;
          case 'r': optarg    = &recseps;                       break;
          case 'u': optarg    = &uvchars;                       break;
          case 'n': outflags |= AS_WEIGHT;
                    if (k <= 0) inflags1 |= AS_WEIGHT;
                    else        inflags2 |= AS_WEIGHT;          break;
          case 'd': if (k <= 0) inflags1 |= AS_DFLT;
                    else        inflags2 |= AS_DFLT;            break;
          case 'h': optarg    = (k <= 0) ? &fn_hdr1 : &fn_hdr2; break;
          default : error(E_OPTION, *--s);                      break;
        }                       /* set option variables */
        if (!*s) break;         /* if at end of string, abort loop */
        if (optarg) { *optarg = s; optarg = NULL; break; }
      } }                       /* get option argument */
    else {                      /* -- if argument is no option */
      argv[k++] = s;            /* note filenames */
    }                           /* by reordering the */
  }                             /* argument vector */
  if (optarg)  error(E_OPTARG); /* check option argument */
  if (k-- < 2) error(E_ARGCNT); /* check number of arguments */
  if (fn_hdr1 && (strcmp(fn_hdr1, "-") == 0))
    fn_hdr1 = "";               /* convert "-" to "" */
  if (fn_hdr2 && (strcmp(fn_hdr2, "-") == 0))
    fn_hdr2 = "";               /* convert "-" to "" */
  r = ((inflags1 & AS_ATT) && (!fn_hdr1 || !*fn_hdr1)) ? 1 : 0;
  if  ((inflags2 & AS_ATT) && (!fn_hdr2 || !*fn_hdr2)) r++;
  for (i = k; --i >= 0; )  if (!argv[i] || !*argv[i])  r++;
  if (r > 1) error(E_STDIN);    /* stdin must not be used twice */
  if (fn_hdr1)                  /* set header flags */
    inflags1 = AS_ATT | (inflags1 & ~AS_DFLT);
  if (fn_hdr2)                  /* set header flags */
    inflags2 = AS_ATT | (inflags2 & ~AS_DFLT);
  if ((outflags & AS_ATT) && (outflags & AS_ALIGN))
    outflags |= AS_ALNHDR;      /* set align to header flag */

  /* --- create attribute set --- */
  attset = as_create("domains", att_delete);
  if (!attset) error(E_NOMEM);  /* create an attribute set */
  as_chars(attset, blanks, fldseps, recseps, uvchars);
  err = as_err(attset);         /* set delimiter characters and */
  fprintf(stderr, "\n");        /* get the error information */

  /* --- determine column widths --- */
  if (outflags & AS_ALIGN) {    /* if to align output columns */
    for (i = 0; i < k; i++) {   /* traverse input files */
      flags = (i > 0) ? inflags2 | AS_NOXATT : inflags1;
      hdr   = (i > 0) ? fn_hdr2              : fn_hdr1;
      if (((flags & AS_ATT) && !hdr) || !argv[i] || !*argv[i])
        continue;               /* skip standard input files */
      if (io_tab(attset, hdr, argv[i], flags, 1) != 0)
        error(1);               /* traverse the input table */
    }                           /* to determine the domains */
  }                             /* and check for an error */

  /* --- write output table --- */
  if (!argv[k] || !*argv[k])    /* if to write to stdout, */
    verb = 0;                   /* suppress log messages */
  in = io_hdr(attset, fn_hdr1, argv[0], inflags1, verb); 
  if (!in) error(1);            /* read the table header */
  if (argv[k] && *argv[k])      /* if an output file name is given, */
    out = fopen(argv[k], "w");  /* open output file for writing */
  else {                        /* if no output file name is given, */
    out = stdout; argv[k] = "<stdout>"; }        /* write to stdout */
  if (!out) error(E_FOPEN, argv[k]);
  if (outflags & AS_ATT) {      /* if to write table header */
    if (as_write(attset, out, outflags) != 0)
      error(E_FWRITE, argv[k]); /* write the attribute names */
  }                             /* to the output file */
  outflags = AS_INST | (outflags & ~AS_ATT);
  outwgt   = outcnt = 0;        /* clear tuple counter and weight sum */
  for (i = 0; i < k; i++) {     /* traverse the input files */
    flags = (i > 0) ? inflags2 | AS_NOXATT : inflags1;
    if (i > 0) {                /* if not the first table */
      in = io_hdr(attset, fn_hdr2, argv[i], flags, verb);
      if (!in) error(1);        /* read next table header */
    }                           /* and check for success */
    tplwgt = tplcnt = 0;        /* clear tuple counter and weight sum */
    if ((i > 0) || !ignore) {   /* if not 1st table or not to ignore */
      f =  flags & ~(AS_ATT|AS_DFLT);
      r = ((flags & AS_DFLT) && !(flags & AS_ATT))
        ? 0 : as_read(attset, in, f);
      while (r == 0) {          /* record read loop */
        if (as_write(attset, out, outflags) != 0)
          error(E_FWRITE, argv[i]);   /* read and write tuple */
        tplwgt += as_getwgt(attset);  /* sum the tuple weight and */
        tplcnt++;                     /* increment the tuple counter */
        r = as_read(attset, in, f);
      }                         /* try to read next record */
      if (err->code < 0) {      /* if an error occurred */
        if (!argv[i] || !*argv[i]) argv[i] = "<stdin>";
        tplcnt += (flags & (AS_ATT|AS_DFLT)) ? 1 : 2;
        io_error(err->code, argv[i], tplcnt, err->s,err->fld,err->exp);
        error(1);               /* print an error message */
      }                         /* and abort the program */
    }
    if (in != stdin) fclose(in);/* close the input file */
    in = NULL;                  /* and clear the variable */
    if (fflush(out) != 0) error(E_FWRITE, argv[k]);
    outcnt += tplcnt;           /* flush the output buffer and */
    outwgt += tplwgt;           /* sum the number of tuples */
    fprintf(stderr, "[%d/%g tuple(s)] done.\n", tplcnt, tplwgt);
  }                             /* print a success message */
  if (out != stdout) {          /* if not written to stdout */
    i = fclose(out); out = NULL;/* close the output file */
    if (i != 0) error(E_FWRITE, argv[k]);
  }                             /* print a success message */
  fprintf(stderr, "%d/%g tuple(s) written to %s.\n",
                  outcnt, outwgt, argv[k]);

  /* --- clean up --- */
  #ifndef NDEBUG
  as_delete(attset);            /* delete attribute set */
  #endif
  #ifdef STORAGE
  showmem("at end of program"); /* check memory usage */
  #endif
  return 0;                     /* return 'ok' */
}  /* main() */
