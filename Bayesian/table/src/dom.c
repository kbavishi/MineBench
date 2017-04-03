/*----------------------------------------------------------------------
  File    : dom.c
  Contents: program to determine attribute domains
  Author  : Christian Borgelt
  History : 24.11.1995 file created
            26.11.1995 adapted to modified attset functions
            08.12.1995 variables in and out made global
            21.12.1995 sort of domains added
            17.01.1996 adapted to modified as_read
            23.02.1996 adapted to modified attset functions
            27.06.1996 minor improvements
            22.11.1996 options -b, -f, and -r added
            26.02.1997 tuple weights (option -n) added
            08.09.1997 minor improvements
            11.01.1998 unknown value characters (option -u) added
            12.09.1998 numerial/alphabetical sorting (option -S) added
            25.09.1998 table reading simplified
            07.02.1999 input from stdin, output to stdout added
            12.02.1999 default header handling improved
            17.04.1999 simplified using the new module 'io'
            14.07.2001 adapted to modified module tfscan
            18.06.2002 meaning of option -i (intervals) inverted
            16.08.2003 slight changes in error message output
----------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>
#include "io.h"
#ifdef STORAGE
#include "storage.h"
#endif

/*----------------------------------------------------------------------
  Preprocessor Definitions
----------------------------------------------------------------------*/
#define PRGNAME     "dom"
#define DESCRIPTION "determine attribute domains"
#define VERSION     "version 1.9 (2003.08.16)         " \
                    "(c) 1995-2003   Christian Borgelt"

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
#define E_UNKNOWN   (-8)        /* unknown error */

/*----------------------------------------------------------------------
  Constants
----------------------------------------------------------------------*/
static const char *errmsgs[] = {   /* error messages */
  /* E_NONE     0 */  "no error\n",
  /* E_NOMEM   -1 */  "not enough memory\n",
  /* E_FOPEN   -2 */  "cannot open file %s\n",
  /* E_FREAD   -3 */  "read error on file %s\n",
  /* E_FWRITE  -4 */  "write error on file %s\n",
  /* E_OPTION  -5 */  "unknown option -%c\n",
  /* E_OPTARG  -6 */  "missing option argument\n",
  /* E_ARGCNT  -7 */  "wrong number of arguments\n",
  /* E_UNKNOWN -8 */  "unknown error\n"
};

/*----------------------------------------------------------------------
  Global Variables
----------------------------------------------------------------------*/
const  char   *prgname = NULL;  /* program name for error messages */
static ATTSET *attset  = NULL;  /* attribute set */
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
  #ifndef NDEBUG                /* clean up memory */
  if (attset) as_delete(attset);/* and close files */
  if (out && (out != stdout)) fclose(out);
  #endif
  #ifdef STORAGE
  showmem("at end of program"); /* check memory usage */
  #endif
  exit(code);                   /* abort the program */
}  /* error() */

/*--------------------------------------------------------------------*/

static int numcmp (const char *name1, const char *name2)
{                               /* --- numerical comparison of names */
  int n1, n2;                   /* results of conversion */

  n1 = (int)strtol(name1, NULL, 10);  /* convert names */
  n2 = (int)strtol(name2, NULL, 10);  /* to integer numbers */
  if (n1 < n2) return -1;       /* compare numbers and */
  if (n1 > n2) return  1;       /* only if they are equal */
  return strcmp(name1, name2);  /* compare the names directly */
}  /* numcmp() */

/*--------------------------------------------------------------------*/

int main (int argc, char *argv[])
{                               /* --- main function */
  int  i, k = 0;                /* loop variables, counter */
  char *s;                      /* to traverse the options */
  char **optarg = NULL;         /* option argument */
  char *fn_hdr  = NULL;         /* name of table header file */
  char *fn_tab  = NULL;         /* name of table file */
  char *fn_dom  = NULL;         /* name of domains file */
  char *blanks  = NULL;         /* blanks */
  char *fldseps = NULL;         /* field  separators */
  char *recseps = NULL;         /* record separators */
  char *uvchars = NULL;         /* unknown value characters */
  int  flags    = 0;            /* table file read flags */
  int  sort     = 0;            /* flag for domain sorting */
  int  atdet    = 0;            /* flag for automatic type determ. */
  int  ivals    = AS_IVALS;     /* flag for numeric intervals */
  int  maxlen   = 0;            /* maximal output line length */
  int  attid;                   /* loop variable for attributes */
  ATT  *att;                    /* to traverse attributes */

  prgname = argv[0];            /* get program name for error msgs. */

  /* --- print startup/usage message --- */
  if (argc > 1) {               /* if arguments are given */
    fprintf(stderr, "%s - %s\n", argv[0], DESCRIPTION);
    fprintf(stderr, VERSION); } /* print a startup message */
  else {                        /* if no argument is given */
    printf("usage: %s [options] "
                     "[-d|-h hdrfile] tabfile domfile\n", argv[0]);
    printf("%s\n", DESCRIPTION);
    printf("%s\n", VERSION);
    printf("-s       sort domains alphabetically "
                    "(default: order of appearance)\n");
    printf("-S       sort domains numerically/alphabetically\n");
    printf("-a       automatic type determination "
                    "(default: all symbolic)\n");
    printf("-i       do not print intervals for numeric attributes\n");
    printf("-l#      output line length (default: no limit)\n");
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
    printf("domfile  file to write domain descriptions to\n");
    return 0;                   /* print a usage message */
  }                             /* and abort the program */

  /* --- evaluate arguments --- */
  for (i = 1; i < argc; i++) {  /* traverse the arguments */
    s = argv[i];                /* get option argument */
    if (optarg) { *optarg = s; optarg = NULL; continue; }
    if ((*s == '-') && *++s) {  /* -- if argument is an option */
      while (1) {               /* traverse characters */
        switch (*s++) {         /* evaluate option */
          case 's': sort   = 1;                     break;
          case 'S': sort   = 2;                     break;
          case 'a': atdet  = 1;                     break;
          case 'i': ivals  = 0;                     break;
          case 'l': maxlen = (int)strtol(s, &s, 0); break;
  	  case 'b': optarg = &blanks;               break;
          case 'f': optarg = &fldseps;              break;
          case 'r': optarg = &recseps;              break;
          case 'u': optarg = &uvchars;              break;
          case 'n': flags |= AS_WEIGHT;             break;
          case 'd': flags |= AS_DFLT;               break;
          case 'h': optarg = &fn_hdr;               break;
          default : error(E_OPTION, *--s);          break;
        }                       /* set option variables */
        if (!*s) break;         /* if at end of string, abort loop */
        if (optarg) { *optarg = s; optarg = NULL; break; }
      } }                       /* get option argument */
    else {                      /* -- if argument is no option */
      switch (k++) {            /* evaluate non-option */
        case  0: fn_tab = s;      break;
        case  1: fn_dom = s;      break;
        default: error(E_ARGCNT); break;
      }                         /* note filenames */
    }
  }
  if (optarg) error(E_OPTARG);  /* check option argument */
  if (k != 2) error(E_ARGCNT);  /* check number of arguments */
  if (fn_hdr && (strcmp(fn_hdr, "-") == 0))
    fn_hdr = "";                /* convert "-" to "" */
  if (fn_hdr)                   /* set header flags */
    flags = AS_ATT | (flags & ~AS_DFLT);
printf("here 1\n");
  /* --- determine attributes and domains --- */
  attset = as_create("domains", att_delete);
  if (!attset) error(E_NOMEM);  /* create an attribute set */
  as_chars(attset, blanks, fldseps, recseps, uvchars);
printf("here 2\n");
  fprintf(stderr, "\n");        /* set delimiter characters */
  if (io_tab(attset, fn_hdr, fn_tab, flags, 1) != 0)
    error(1);                   /* read the table */
printf("here 3\n");
  /* --- convert/sort domains --- */
  if (atdet) {                  /* if automatic type determination */
    for (attid = as_attcnt(attset); --attid >= 0; )
{
      att_conv(as_att(attset, attid), AT_AUTO, NULL);
printf("attid=%d\n", attid);
 }
 }                             /* try to convert attributes */
  if (sort) {                   /* if to sort domains (values) */
    for (attid = as_attcnt(attset); --attid >= 0; ) {
      att = as_att(attset, attid);
      if (att_type(att) != AT_SYM) continue;
      att_valsort(att, (sort > 1) ? numcmp : strcmp, NULL, 0);
    }                           /* traverse symbolic attributes */
  }                             /* and sort their domains */
printf("here 5\n");
  /* --- write output file --- */
  if (fn_dom && *fn_dom)        /* if a domain file name is given, */
    out = fopen(fn_dom, "w");   /* open domain file for writing */
  else {                        /* if no domain file name is given, */
    out = stdout; fn_dom = "<stdout>"; }         /* write to stdout */
  fprintf(stderr, "writing %s ... ", fn_dom);
  if (!out) error(E_FOPEN, fn_dom);
  if (as_desc(attset, out, AS_TITLE|ivals, maxlen) != 0)
    error(E_FWRITE, fn_dom);    /* write domain descriptions */
  if (out != stdout) {          /* if not written to stdout, */
    i = fclose(out); out = NULL;/* close the output file */
    if (i != 0) error(E_FWRITE, fn_dom);
  }                             /* print a success message */
  fprintf(stderr, "[%d attribute(s)] done.\n", as_attcnt(attset));

  /* --- clean up --- */
  #ifndef NDEBUG
  as_delete(attset);            /* delete attribute set */
  #endif
  #ifdef STORAGE
  showmem("at end of program"); /* check memory usage */
  #endif
  return 0;                     /* return 'ok' */
}  /* main() */
