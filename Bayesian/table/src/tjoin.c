/*----------------------------------------------------------------------
  File    : tjoin.c
  Contents: program to join two tables
  Author  : Christian Borgelt
  History : 02.02.1999 file created
            05.02.1999 first version completed
            07.02.1999 field name options (-1,-2) added
            12.02.1999 default header handling improved
            17.04.1999 simplified using the new module 'io'
            25.11.1999 bug in join comparison information fixed
            14.07.2001 adapted to modified module tfscan
            24.02.2002 check for multiple assignment of stdin added
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
#define MEMJOIN

/*----------------------------------------------------------------------
  Preprocessor Definitions
----------------------------------------------------------------------*/
#define PRGNAME     "tjoin"
#define DESCRIPTION "join two tables"
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
#define E_STDIN     (-8)        /* double assignment of stdin */
#define E_JOINCNT   (-9)        /* number of join columns differs */
#define E_UNKFLD   (-10)        /* unknown field name */
#define E_FLDNAME  (-11)        /* duplicate field name in output */
#define E_UNKNOWN  (-12)        /* unknown error */

/*----------------------------------------------------------------------
  Type Definitions
----------------------------------------------------------------------*/
typedef struct {                /* --- join comparison data --- */
  int cnt;                      /* number of columns to compare */
  int *cis1, *cis2;             /* vectors of column indices */
} JCDATA;                       /* (join comparison data) */

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
  /* E_JOINCNT  -9 */  "number of join columns differ\n",
  /* E_UNKFLD  -10 */  "unknown field name %s\n",
  /* E_FLDNAME -11 */  "duplicate field name in output table\n",
  /* E_UNKNOWN -12 */  "unknown error\n"
};

/*----------------------------------------------------------------------
  Global Variables
----------------------------------------------------------------------*/
const  char   *prgname;         /* program name for error messages */
static ATTSET *attsets[3] = { NULL, NULL, NULL }; /* attribute sets */
static TABLE  *tables [2] = { NULL, NULL };       /* tables */
static int    *cis     = NULL;  /* column index vector */
static char   **flds   = NULL;  /* field names vector */
#ifndef MEMJOIN
static FILE   *out     = NULL;  /* output file */
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
  #ifndef NDEBUG
  if (tables[0])  tab_delete(tables[0], 0);
  if (tables[1])  tab_delete(tables[1], 0);
  if (attsets[0]) as_delete(attsets[0]);
  if (attsets[1]) as_delete(attsets[1]);
  if (attsets[2]) as_delete(attsets[2]);
  if (cis)        free(cis);    /* clean up memory */
  if (flds)       free(flds);   /* and close files */
  #ifndef MEMJOIN
  if (out && (out != stdout)) fclose(out);
  #endif
  #endif
  #ifdef STORAGE
  showmem("at end of program"); /* check memory usage */
  #endif
  exit(code);                   /* abort the program */
}  /* error() */

/*--------------------------------------------------------------------*/
#ifndef MEMJOIN

static int joincmp (const TUPLE *tpl1, const TUPLE *tpl2, JCDATA *jcd)
{                               /* --- compare tuples for join */
  int    i;                     /* loop variable */
  int    *p1,   *p2;            /* to traverse column indices */
  CINST  *col1, *col2;          /* columns to compare */
  ATTSET *attset;               /* underlying attribute set */

  attset = tpl_attset(tpl1);    /* note attribute set */
  p1 = jcd->cis1; p2 = jcd->cis2;
  for (i = jcd->cnt; --i >= 0; ) {
    col1 = tpl_colval(tpl1, *p1++); /* traverse the column indices */
    col2 = tpl_colval(tpl2, *p2);   /* and get columns to compare */
    if (att_type(as_att(attset, *p2++)) == AT_FLT) {
      if (col1->f < col2->f) return  1;
      if (col1->f > col2->f) return -1; }
    else {
      if (col1->i < col2->i) return  1;
      if (col1->i > col2->i) return -1;
    }                           /* compare the tuple columns */
  }                             /* and if they differ, abort */
  return 0;                     /* otherwise return 'equal' */
}  /* joincmp() */

#endif
/*--------------------------------------------------------------------*/

int main (int argc, char *argv[])
{                               /* --- main function */
  int    i, k = 0;              /* loop variables, counters */
  char   *s;                    /* to traverse options */
  char   **optarg = NULL;       /* option argument */
  char   *fn_hdr1 = NULL;       /* name of table header file 1 */
  char   *fn_hdr2 = NULL;       /* name of table header file 2 */
  char   *fn_in1  = NULL;       /* name of table file 1 */
  char   *fn_in2  = NULL;       /* name of table file 2 */
  char   *fn_out  = NULL;       /* name of output table */
  char   *blanks  = NULL;       /* blank  characters */
  char   *fldseps = NULL;       /* field  separators */
  char   *recseps = NULL;       /* record separators */
  char   *uvchars = NULL;       /* unknown value characters */
  int    inflags1 = 0;          /* first  table file read flags */
  int    inflags2 = 0;          /* second table file read flags */
  int    outflags = AS_ATT;     /* table file write flags */
  int    ncs = 0, ncd = 0;      /* number of (join) columns */
  int    *scis, *dcis;          /* column index vector */
#ifndef MEMJOIN
  int    tplcnt;                /* number of tuples written */
  double tplwgt;                /* weight of tuples written */
  int    si, di, ri;            /* tuple indices */
  TUPLE  *ts, *td;              /* to traverse the tuples */
  ATT    *att;                  /* to traverse attributes */
  JCDATA jcd;                   /* column data for sorting */
#endif

  prgname = argv[0];            /* get program name for error msgs. */

  /* --- print startup/usage message --- */
  if (argc > 1) {               /* if arguments are given */
    fprintf(stderr, "%s - %s\n", argv[0], DESCRIPTION);
    fprintf(stderr, VERSION); } /* print a startup message */
  else {                        /* if no argument is given */
    printf("usage: %s [options] [-d|-h hdr1] in1 "
                     "[-d|-h hdr2] in2 out\n", argv[0]);
    printf("%s\n", DESCRIPTION);
    printf("%s\n", VERSION);
    printf("-1#      join column in table 1 "
                    "(may appear several times)\n");
    printf("-2#      join column in table 2 "
                    "(may appear several times)\n");
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
    printf("hdr1/2   file containing table header (field names)\n");
    printf("in1/2    table files to read "
                    "(field names in first record)\n");
    printf("out      file to write the result to\n");
    return 0;                   /* print a usage message */
  }                             /* and abort the program */

  /* --- evaluate arguments --- */
  flds = (char**)malloc((argc-1) *(int)sizeof(char*));
  if (!flds) error(E_NOMEM);    /* allocate a field names vector */
  for (i = 1; i < argc; i++) {  /* traverse arguments */
    s = argv[i];                /* get option argument */
    if (optarg) { *optarg = s; optarg = NULL; continue; }
    if ((*s == '-') && *++s) {  /* -- if argument is an option */
      while (1) {               /* traverse characters */
        switch (*s++) {         /* evaluate option */
          case '1': optarg    = argv +ncd++;                    break;
          case '2': optarg    = flds +ncs++;                    break;
          case 'a': outflags |= AS_ALIGN;                       break;
          case 'w': outflags &= ~AS_ATT;                        break;
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
      switch (k++) {            /* evaluate non-option */
        case  0: fn_in1 = s;      break;
        case  1: fn_in2 = s;      break;
        case  2: fn_out = s;      break;
        default: error(E_ARGCNT); break;
      }                         /* note filenames */
    }
  }
  if (optarg)     error(E_OPTARG);  /* check option argument */
  if (k   != 3)   error(E_ARGCNT);  /* check number of arguments */
  if (ncs != ncd) error(E_JOINCNT); /* check join columns */
  if (fn_hdr1 && (strcmp(fn_hdr1, "-") == 0))
    fn_hdr1 = "";               /* convert "-" to "" */
  if (fn_hdr2 && (strcmp(fn_hdr2, "-") == 0))
    fn_hdr2 = "";               /* convert "-" to "" */
  i = (!fn_in1 || !*fn_in1) ? 1 : 0;
  if  (!fn_in2 || !*fn_in2)  i++;
  if  (fn_hdr1 && !*fn_hdr1) i++;
  if  (fn_hdr2 && !*fn_hdr2) i++;/* check assignments of stdin: */
  if (i > 1) error(E_STDIN);    /* stdin must not be used twice */
  if (fn_hdr1)                  /* set header flags */
    inflags1 = AS_ATT | (inflags1 & ~AS_DFLT);
  if (fn_hdr2)                  /* set header flags */
    inflags2 = AS_ATT | (inflags2 & ~AS_DFLT);
  if ((outflags & AS_ALIGN) && (outflags & AS_ATT))
    outflags |= AS_ALNHDR;      /* set align to header flag */
  fprintf(stderr, "\n");        /* terminate the startup message */

  /* --- read input tables --- */
  attsets[0] = as_create("domains 0", att_delete);
  if (!attsets[0]) error(1);    /* create first attribute set */
  as_chars(attsets[0], blanks, fldseps, recseps, uvchars);
  tables[0] = io_tabin(attsets[0], fn_hdr1,
                       fn_in1, inflags1, "table 0", 1);
  if (!tables[0]) error(1);     /* read the first table */
  attsets[1] = as_create("domains 1", att_delete);
  if (!attsets[1]) error(1);    /* create second attribute set */
  as_chars(attsets[1], blanks, fldseps, recseps, uvchars);
  tables[1] = io_tabin(attsets[1], fn_hdr2,
                       fn_in2, inflags2, "table 1", 1);
  if (!tables[1]) error(1);     /* read the second table */

#ifdef MEMJOIN
  /* --- join input tables --- */
  if (ncd <= 0)                 /* if to do a natural join, */
    scis = dcis = NULL;         /* clear the column index vectors */
  else {                        /* if to do a normal join */
    cis = scis = (int*)malloc((ncd +ncd) *(int)sizeof(int));
    if (!cis) error(E_NOMEM);   /* allocate a column index vector */
    dcis = scis +ncd;           /* and organize it (2 in 1) */
    for (i = 0; i < ncd; i++) { /* traverse join column names */
      dcis[i] = as_attid(attsets[0], argv[i]);
      if (dcis[i] < 0) error(E_UNKFLD, argv[i]);
      scis[i] = as_attid(attsets[1], flds[i]);
      if (scis[i] < 0) error(E_UNKFLD, flds[i]);
    }                           /* build column index vectors */
  }                             /* for source and destination */
  if (tab_join(tables[0], tables[1], ncd, scis, dcis) != 0)
    error(E_NOMEM);             /* join the two tables */
  
  /* --- write output table --- */
  if (io_tabout(tables[0], fn_out, outflags, 1) != 0)
    error(1);                   /* write the join result */

#else  /* #ifdef MEMJOIN */

  /* --- join attribute sets --- */
  jcd.cnt = ncd;                /* note number of join columns */
  ncs = as_attcnt(attsets[1]);  /* get number of columns */
  ncd = as_attcnt(attsets[0]);  /* of source and destination */
  if (jcd.cnt <= 0) {           /* -- if to do a natural join */
    k   = (ncs < ncd) ? ncs : ncd;
    cis = (int*)malloc((ncs +k +k) *(int)sizeof(int));
    if (!cis) error(E_NOMEM);   /* allocate column index vectors */
    scis = cis +ncs; dcis = scis +k;
    for (jcd.cnt = 0, i = ncs; --i >= 0; ) {
      att = as_att(attsets[1],i);  /* traverse source columns */
      k   = as_attid(attsets[0], att_name(att));
      if (k < 0) continue;      /* skip non-join columns */
      dcis[jcd.cnt] = k; scis[jcd.cnt++] = i;
      att_setmark(att, -1);     /* collect join columns and */
    } }                         /* mark them in the source */
  else {                        /* -- if to do a normal join */
    cis = (int*)malloc((ncs +jcd.cnt +jcd.cnt) *(int)sizeof(int));
    if (!cis) error(E_NOMEM);   /* allocate column index vector and */
    scis = cis +ncs; dcis = scis +jcd.cnt;  /* organize it (3 in 1) */
    for (i = 0; i < jcd.cnt; i++) {   /* traverse join column names */
      dcis[i] = as_attid(attsets[0], argv[i]);
      if (dcis[i] < 0) error(E_UNKFLD, argv[i]);
      scis[i] = as_attid(attsets[1], flds[i]);
      if (scis[i] < 0) error(E_UNKFLD, flds[i]);
      att_setmark(as_att(attsets[1], scis[i]), -1);
    }                           /* build column index vectors */
  }                             /* for source and destination */
  attsets[2] = as_dup(attsets[0]);
  if (!attsets[2]               /* duplicate dest. attribute set */
  ||  (as_attcopy(attsets[2], attsets[1], AS_MARKED) != 0))
    error(E_NOMEM);             /* add columns to source att. set */
  if (as_attcnt(attsets[2]) != ncd +ncs -jcd.cnt)
    error(E_FLDNAME);           /* check result of att. set join */
  for (i = ncs;     --i >= 0; ) cis[i] = i;
  for (i = jcd.cnt; --i >= 0; ) cis[scis[i]] = -1;
  for (i = k = 0; i < ncs; i++) /* build index vector for copying */
    if (cis[i] >= 0) cis[k++] = cis[i];

  /* --- sort input tables --- */
  jcd.cis1 = jcd.cis2 = dcis;   /* sort the destination tuples */
  tab_sort(tables[0], 0, INT_MAX, (TPL_CMPFN*)joincmp, &jcd);
  jcd.cis1 = jcd.cis2 = scis;   /* sort the source tuples */
  tab_sort(tables[1], 0, INT_MAX, (TPL_CMPFN*)joincmp, &jcd);
  jcd.cis1 = dcis;              /* prepare for join comparisons */

  /* --- write output table --- */
  if (fn_out && *fn_out)        /* if an output file name is given, */
    out = fopen(fn_out, "w");   /* open output file for writing */
  else {                        /* if no output file name is given, */
    out = stdout; fn_out = "<stdout>"; }         /* write to stdout */
  fprintf(stderr, "writing %s ...", fn_out);
  if (!out) error(E_FOPEN, fn_out);
  if (outflags & AS_ATT) {      /* if to write table header */
    if (as_write(attsets[2], out, outflags) != 0)
      error(E_FWRITE, fn_out);  /* write the attribute names */
  }                             /* to the output file */
  outflags = AS_INST | (outflags & ~AS_ATT);
  tplwgt = tplcnt = 0;          /* clear tuple counter and weight sum */
  si = tab_tplcnt(tables[1])-1; /* get the index of the */
  di = tab_tplcnt(tables[0])-1; /* last tuple in each table */
  while ((di >= 0)              /* while there are still */
  &&     (si >= 0)) {           /* tuples left in both tables */
    td = tab_tpl(tables[0], di);/* get the next destination tuple */
    ts = tab_tpl(tables[1], si);/* and the current source tuple */
    k = joincmp(td, ts, &jcd);  /* compare the two tuples */
    if (k > 0) { di--; continue; }  /* and find next pair */
    if (k < 0) { si--; continue; }  /* of joinable tuples */
    ri = si;                    /* get current source index */
    for (i = ncd; --i >= 0; )   /* copy dest. tuple to attribute set */
      *att_inst(as_att(attsets[2], i)) = *tpl_colval(td, i);
    do {                        /* tuple join loop */
      for (i = ncs -jcd.cnt; --i >= 0; )  /* copy source columns */
        *att_inst(as_att(attsets[2], ncd +i)) = *tpl_colval(ts, cis[i]);
      as_setwgt(attsets[2], tpl_getwgt(ts) *tpl_getwgt(td));
                                /* compute and set the tuple weight */
      if (as_write(attsets[2], out, outflags) != 0)
        error(E_FWRITE,fn_out); /* write the joined tuple */
      tplwgt += as_getwgt(attsets[0]);
      tplcnt++;                 /* sum tuple weight and count tuple */
      if (--ri < 0) break;      /* if source is empty, abort loop, */
      ts = tab_tpl(tables[1], ri); /* otherwise get next source tuple */
    } while (joincmp(td, ts, &jcd) == 0);
    di--;                       /* if tuples cannot be joined, abort */
  }                             /* and go to the next dest. tuple */
  if (out != stdout) {          /* if not written to stdout */
    i = fclose(out); out = NULL;/* close the output file */
    if (i) error(E_FWRITE, fn_out);
  }                             /* print a success message */
  fprintf(stderr, "[%d/%g tuple(s)] done.\n", tplcnt, tplwgt);
#endif  /* #ifdef MEMJOIN .. #else .. */

  /* --- clean up --- */
  #ifndef NDEBUG
  #ifndef MEMJOIN
  as_delete(attsets[2]);        /* delete joined attribute set */
  #endif
  tab_delete(tables[0], 1);     /* delete tables, */
  tab_delete(tables[1], 1);     /* attribute sets, */
  if (cis) free(cis);           /* column index vector, */
  free(flds);                   /* and field names vector */
  #endif
  #ifdef STORAGE
  showmem("at end of program"); /* check memory usage */
  #endif
  return 0;                     /* return 'ok' */
}  /* main() */
