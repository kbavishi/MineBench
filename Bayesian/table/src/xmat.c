/*----------------------------------------------------------------------
  File    : xmat.c
  Contents: program to determine a confusion matrix for two table fields
  Author  : Christian Borgelt
  History : 04.01.1998 file created
            10.01.1998 first version completed
            20.06.1998 adapted to modified st_create() function
            03.08.1998 output format of numbers improved
            07.02.1999 input from stdin, output to stdout added
            12.02.1999 default header handling improved
            27.03.1999 bugs in matrix (re)allocation removed
            14.07.2001 adapted to modified module tfscan
            16.08.2003 slight changes in error message output
----------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "symtab.h"
#include "tfscan.h"
#ifdef STORAGE
#include "storage.h"
#endif

/*----------------------------------------------------------------------
  Preprocessor Definitions
----------------------------------------------------------------------*/
#define PRGNAME     "xmat"
#define DESCRIPTION "determine a confusion matrix"
#define VERSION     "version 1.6 (2003.08.16)         " \
                    "(c) 1998-2003   Christian Borgelt"

#define CCHAR      const char   /* abbreviation */

/* --- sizes --- */
#define BUFSIZE     512         /* size of read buffer */
#define BLKSIZE       4         /* block size for value vector */

/* --- error codes --- */
#define OK            0         /* no error */
#define E_NONE        0         /* no error */
#define E_NOMEM     (-1)        /* not enough memory */
#define E_FOPEN     (-2)        /* cannot open file */
#define E_FREAD     (-3)        /* read error on file */
#define E_FWRITE    (-4)        /* write error on file */
#define E_OPTION    (-5)        /* unknown option */
#define E_OPTARG    (-6)        /* missing option argument */
#define E_ARGCNT    (-7)        /* too few/many arguments */
#define E_STDIN     (-8)        /* double assignment of stdin */
#define E_FLDNAME   (-9)        /* illegal field name */
#define E_VALUE    (-10)        /* illegal value */
#define E_FLDCNT   (-11)        /* wrong number of fields */
#define E_UNKNOWN  (-12)        /* unknown error */

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
  /* E_FLDNAME  -9 */  "illegal field name \"%s\"\n",
  /* E_VALUE   -10 */  "file %s, record %d: "
                         "illegal value \"%s\" in field %d\n",
  /* E_FLDCNT  -11 */  "file %s, record %d: "
                         "%d field(s) expected\n",
  /* E_UNKNOWN -12 */  "unknown error\n"
};

/*----------------------------------------------------------------------
  Global Variables
----------------------------------------------------------------------*/
static CCHAR  *prgname;         /* program name for error messages */
static int    **vals  = NULL;   /* value vector (symtab entries) */
static int    valvsz  = 0;      /* size of value vector */
static int    valcnt  = 0;      /* number of values */
static TFSCAN *tfscan = NULL;   /* table file scanner */
static SYMTAB *symtab = NULL;   /* symbol table */
static double **xmat  = NULL;   /* confusion matrix */
static FILE   *in     = NULL;   /* input  file */
static FILE   *out    = NULL;   /* output file */
static char   rdbuf[BUFSIZE];   /* read buffer */
static char   fnbuf[BUFSIZE];   /* field name buffer */

/*----------------------------------------------------------------------
  Auxiliary Functions
----------------------------------------------------------------------*/
#ifndef NDEBUG

static void _delmat (void)
{                               /* --- delete confusion matrix */
  int    x;                     /* loop variable */
  double **p;                   /* to traverse the matrix columns */

  if (!xmat) return;            /* if no matrix exists, abort */
  for (p = xmat +(x = valvsz+1); --x >= 0; )
    if (*--p) free(*p);         /* delete matrix columns */
  free(xmat);                   /* and column vector */
}  /* _delmat() */

#endif
/*--------------------------------------------------------------------*/

static int _valcmp (const void *p1, const void *p2)
{                               /* --- compare values */
  return strcmp(st_name(*(const void**)p1), st_name(*(const void**)p2));
}  /* _valcmp() */

/*----------------------------------------------------------------------
  Main Functions
----------------------------------------------------------------------*/

static void error (int code, ...)
{                               /* --- print error message */
  va_list    args;              /* list of variable arguments */
  const char *msg;              /* error message */

  if ((code > 0) || (code < E_UNKNOWN))
    code = E_UNKNOWN;           /* check index */
  msg = errmsgs[-code];         /* and error message text */
  fprintf(stderr, "\n%s: ", prgname);
  va_start(args, code);         /* get variable arguments */
  vfprintf(stderr, msg, args);  /* print error message */
  va_end(args);                 /* end variable argument evaluation */

  #ifndef NDEBUG                /* if debug version */
  if (tfscan) tfs_delete(tfscan);
  if (symtab) st_delete(symtab);
  if (xmat)   _delmat();        /* clean up memory */
  if (vals)   free(vals);       /* and close files */
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
  int    i, k = 0;              /* loop variables, counter */
  char   *s;                    /* to traverse options, buffer */
  CCHAR  **optarg = NULL;       /* option argument */
  CCHAR  *fn_tab  = NULL;       /* name of table  file */
  CCHAR  *fn_hdr  = NULL;       /* name of header file */
  CCHAR  *fn_mat  = NULL;       /* name of matrix file */
  CCHAR  *fname   = NULL;       /* buffer for file name */
  CCHAR  *blanks  = NULL;       /* blanks */
  CCHAR  *fldseps = NULL;       /* field  separators */
  CCHAR  *recseps = NULL;       /* record separators */
  CCHAR  *xname   = NULL;       /* name for x-direction */
  CCHAR  *yname   = NULL;       /* name for y-direction */
  int    header   = 0;          /* header type (tyble/default/file) */
  int    wgtflg   = 0;          /* flag for weight in last field */
  int    relnum   = 0;          /* flag for relative numbers */
  int    sort     = 0;          /* flag for sorted values */
  int    maxlen   = 6, len;     /* (maximal) length of a value name */
  int    fldcnt   = 0, cnt;     /* number of fields */
  int    tplcnt   = 0;          /* number of records */
  double tplwgt   = 0.0;        /* weight of tuples */
  double weight   = 1.0;        /* weight of tuple */
  int    d;                     /* delimiter type */
  int    x, y;                  /* field indices, loop variables */
  int    *px = NULL, *py = NULL, *p;  /* pointers to symbol data */
  double *c1, *c2;              /* to traverse matrix columns */
  void   *tmp;                  /* temporary buffer */
  CCHAR  *fmt;                  /* output format for numbers */

  prgname = argv[0];            /* get program name for error msgs. */

  /* --- print usage message --- */
  if (argc > 1) {               /* if arguments are given */
    fprintf(stderr, "%s - %s\n", argv[0], DESCRIPTION);
    fprintf(stderr, VERSION); } /* print a startup message */
  else {                        /* if no arguments is given */
    printf("usage: %s [options] "
                     "[-d|-h hdrfile] tabfile [matfile]\n", argv[0]);
    printf("%s\n", DESCRIPTION);
    printf("%s\n", VERSION);
    printf("-x#      field name for x-direction of matrix\n");
    printf("-y#      field name for y-direction of matrix\n");
    printf("-p       compute relative numbers (in percent)\n");
    printf("-s       sort values alphabetically "
                    "(default: order of appearance)\n");
    printf("-n       number of tuple occurrences in last field\n");
    printf("-b/f/r#  blank characters, field and record separators\n"
           "         (default: \" \\t\\r\", \" \\t\", \"\\n\")\n");
    printf("-d       use default header "
                    "(field names = field numbers)\n");
    printf("-h       read table header (field names) from hdrfile\n");
    printf("hdrfile  file containing table header (field names)\n");
    printf("tabfile  table file to read "
                    "(field names in first record)\n");
    printf("matfile  file to write confusion matrix to (optional)\n");
    return 0;                   /* print a usage message */
  }                             /* and abort the program */

  /* --- evaluate arguments --- */
  for (i = 1; i < argc; i++) {  /* traverse arguments */
    s = argv[i];                /* get option argument */
    if (optarg) { *optarg = s; optarg = NULL; continue; }
    if ((*s == '-') && *++s) {  /* -- if argument is an option */
      while (1) {               /* traverse characters */
        switch (*s++) {         /* evaluate options */
          case 'x': optarg  = &xname;      break;
          case 'y': optarg  = &yname;      break;
	  case 'p': relnum  = 1;           break;
          case 's': sort    = 1;           break;
          case 'n': wgtflg  = 1;           break;
          case 'h': optarg  = &fn_hdr;     break;
          case 'b': optarg  = &blanks;     break;
          case 'f': optarg  = &fldseps;    break;
          case 'r': optarg  = &recseps;    break;
          case 'd': header  = 1;           break;
          default : error(E_OPTION, *--s); break;
        }                       /* set option variables */
        if (!*s) break;         /* if at end of string, abort loop */
        if (optarg) { *optarg = s; optarg = NULL; break; }
      } }                       /* get option argument */
    else {                      /* -- if argument is no option */
      switch (k++) {            /* evaluate non-options */
        case  0: fn_tab = s;      break;
        case  1: fn_mat = s;      break;
        default: error(E_ARGCNT); break;
      }                         /* note filenames */
    }
  }
  if (optarg) error(E_OPTARG);  /* check option argument */
  if ((k < 1) || (k > 2)) error(E_ARGCNT);
  if (fn_hdr && (strcmp(fn_hdr, "-") == 0))
    fn_hdr = "";                /* convert "-" to "" */
  if (fn_hdr) header = 2;       /* set header flag */

  /* --- create symbol table and table file scanner --- */
  symtab = st_create(0, 0, (HASHFN*)0, (SYMFN*)0);
  if (!symtab) error(E_NOMEM);  /* create symbol table */
  tfscan = tfs_create();        /* and table file scanner */
  if (!tfscan) error(E_NOMEM);  /* and set delimiter characters */
  tfs_chars(tfscan, TFS_BLANK,  blanks);
  tfs_chars(tfscan, TFS_FLDSEP, fldseps);
  tfs_chars(tfscan, TFS_RECSEP, recseps);

  /* --- read table header/first record --- */
  fname = (fn_hdr) ? fn_hdr : fn_tab;
  if (fname && *fname)          /* if a proper filename is given, */
    in = fopen(fname, "rb");    /* open file for reading */
  else {                        /* if no proper file name is given, */
    in = stdin; fname = "<stdin>"; }    /* read from standard input */
  fprintf(stderr, "\nreading %s ... ", fname);
  if (!in) error(E_FOPEN, fname);
  do {                          /* read fields of table header */
    d = tfs_getfld(tfscan, in, rdbuf, BUFSIZE-1);
    if (d < 0) error(E_FREAD, fname);   /* read next table field */
    if (header != 1) {          /* if to use a normal header */
      p = (int*)st_insert(symtab, rdbuf, -1, sizeof(int));
      if (!p) error(E_NOMEM);   /* insert name into symbol table */
      if (p == EXISTS) p = (int*) st_lookup(symtab, rdbuf, -1);
      else            *p = fldcnt; }     /* note field number */
    else {                      /* if to use a default header */
      sprintf(fnbuf, "%d", fldcnt +1); /* create a field name */
      i = (int)(strlen(rdbuf) +1 +sizeof(int));
      p = (int*)st_insert(symtab, fnbuf, -1, i);
      if (!p) error(E_NOMEM);   /* insert name into symbol table */
      if (p == EXISTS) p = (int*) st_lookup(symtab, fnbuf, -1);
      else {          *p = fldcnt; strcpy((char*)(p+1), rdbuf); }
    }                           /* note field number and value name */
    if (fldcnt >= valvsz) {     /* if the field vector is full */
      valvsz += (valvsz > BLKSIZE) ? (valvsz >> 1) : BLKSIZE;
      tmp = realloc(vals, valvsz *sizeof(int*));
      if (!tmp) error(E_NOMEM); /* allocate and set */
      vals = (int**)tmp;        /* a (new) field name vector, */
    }                           /* then insert the name read */
    vals[fldcnt++] = p;         /* into the field name vector */
  } while (d > TFS_REC);        /* while not at end of record */
  cnt = fldcnt -wgtflg;         /* compute number of data fields */
  if (cnt < 1) error(E_FLDCNT, fname, 1, wgtflg +1);
  if (fn_hdr) {                 /* if a table header file is given, */
    fclose(in); fprintf(stderr, "done.");} /* close the header file */

  /* --- determine field indices --- */
  x = y = -1;                   /* initialize field indices */
  if (xname) {                  /* if field name for x-dir. given */
    px = (int*) st_lookup(symtab, xname, -1);
    if (!px || (*px >= cnt)) error(E_FLDNAME, xname);
    x = *px;                    /* get the symbol data for name */
  }                             /* and read the field index from it */
  if (yname) {                  /* if field name for y-dir. given */
    py = (int*) st_lookup(symtab, yname, -1);
    if (!py || (*py >= cnt)) error(E_FLDNAME, yname);
    y = *py;                    /* get the symbol data for name */
  }                             /* and read the field index from it */
  if (y < 0) y = cnt -((x >= 0) ? 1 : 2);
  if (y < 0) y = 0;             /* if field indices have not been */
  if (x < 0) x = cnt -1;        /* determined, set default indices */
  if (x < 0) x = 0;             /* (last field/last field but one) */
  xname = st_name(vals[x]);     /* get names of the fields */
  yname = st_name(vals[y]);     /* to compute confusion matrix of */

  /* --- create a confusion matrix --- */
  if (valvsz > BLKSIZE) valvsz = BLKSIZE;
  xmat = (double**)calloc(valvsz+1, sizeof(double*));
  if (!xmat) error(E_NOMEM);    /* allocate column vector */
  for (i = valvsz +1; --i >= 0; ) {
    xmat[i] = (double*)calloc(valvsz+1, sizeof(double));
    if (!xmat[i]) error(E_NOMEM);
  }                             /* allocate matrix columns */

  /* --- compute confusion matrix --- */
  if      (header > 1) {        /* if a table header file is given */
    if (fn_tab && *fn_tab)      /* if a proper table name is given, */
      in = fopen(fn_tab, "rb"); /* open table file for reading */
    else {                      /* if no table file name is given, */
      in = stdin; fn_tab = "<stdin>"; }         /* read from stdin */
    fprintf(stderr, "\nreading %s ... ", fn_tab);
    if (!in) error(E_FOPEN, fn_tab); }
  else if (header > 0) {        /* if to use a default header */
    px = vals[x]; py = vals[y]; /* get the two values already read */
    s  = (char*)(px +1);        /* and the name of the first value */
    len = (int)strlen(s);       /* determine the length of the name */
    if (len > maxlen) maxlen = len;
    px = (int*) st_insert(symtab, s, 0, sizeof(int));
    if (!px) error(E_NOMEM);    /* insert value into symbol table, */
    vals[*px = valcnt++] = px;  /* note its name and set its index */
    s  = (char*)(py +1);        /* get the name of the second value */
    py = (int*) st_insert(symtab, s, 0, sizeof(int));
    if (!py) error(E_NOMEM);    /* insert value into symbol table */
    if (py == EXISTS) py = px;  /* if value is the same, get pointer */
    else {                      /* if value differs from the first, */
      vals[*py = valcnt++] = py;/* note its name and set its index */
      len = (int)strlen(s);     /* determine the length of the name */
      if (len > maxlen) maxlen = len;
    }                           /* update maximal length */
    if (wgtflg) {               /* if tuple weight in last field, */
      weight = strtod(rdbuf,&s);/* convert last field to a number */
      if (*s || (s == rdbuf) || (weight < 0))
        error(E_VALUE, fn_tab, tplcnt +((header > 0) ? 1 : 2), rdbuf);
    }                           /* get tuple weight/counter */
    xmat[*px][*py] += weight;   /* add weight to confusion matrix */
    tplwgt += weight;           /* and to the tuple weight */
    tplcnt++;                   /* increment the tuple counter */
  }                             /* update max. length, if necessary */
  do {                          /* read table records */
    d = TFS_FLD;                /* read fields in table record */
    for (cnt = 0; (cnt < fldcnt) && (d >= TFS_FLD); cnt++) {
      d = tfs_getfld(tfscan, in, rdbuf, BUFSIZE-1);
      if (d < 0) error(E_FREAD, fn_tab);
      if ((d        <= TFS_EOF) /* if at end of file */
      &&  (cnt      <= 0)       /* and on first field */
      &&  (rdbuf[0] == '\0')) { /* and no value read */
        cnt = -1; break; }      /* (empty record), abort loop */
      if ((cnt != x) && (cnt != y))
        continue;               /* skip irrelevant fields */
      p = (int*)(tmp = st_insert(symtab, rdbuf, 0, sizeof(int)));
      if (!p) error(E_NOMEM);   /* insert value into symbol table */
      if (tmp == EXISTS) p = (int*) st_lookup(symtab, rdbuf, 0);
      else              *p = valcnt;
      if (cnt == x) px = p;     /* note pointers to */
      if (cnt == y) py = p;     /* the symbol data */
      if (tmp == EXISTS) continue; /* skip known values */
      if (valcnt >= valvsz) {   /* if the  value vector is full */
        i   = valvsz + ((valvsz > BLKSIZE) ? (valvsz >> 1) : BLKSIZE);
        tmp = realloc(vals, i *sizeof(int*));
        if (!tmp) error(E_NOMEM);  /* allocate and set */
        vals = (int**)tmp;         /* a new value vector */
        tmp = realloc(xmat, (i+1) *sizeof(double*));
        if (!tmp) error(E_NOMEM);  /* allocate and set */
        xmat = (double**)tmp;      /* a new column vector */
        for (valvsz = i++; --i > valcnt; )
          xmat[i] = NULL;       /* invalidate new columns */
        for (i = valvsz+1; --i >= 0; ) {
          c1 = (double*)realloc(xmat[i], (valvsz+1) *sizeof(double));
          if (!c1) error(E_NOMEM);
          xmat[i] = c1;         /* (re)allocate matrix columns */
          for (k = (i > valcnt) ? 0 : valcnt+1; k <= valvsz; k++)
            c1[k] = 0.0;        /* clear (new part of) the */
        }                       /* allocated matrix column */
      }
      vals[valcnt++] = p;       /* note symbol data pointer and */
      len = (int)strlen(rdbuf); /* determine the length of the name */
      if (len > maxlen) maxlen = len;
    }                           /* update max. length, if necessary */
    if (cnt < 0) break;         /* if at end of file, abort loop */
    if (cnt != fldcnt)          /* check number of fields in record */
      error(E_FLDCNT, fn_tab, tplcnt +((header > 0) ? 1 : 2), fldcnt);
    if (wgtflg) {               /* if tuple weight in last field */
      weight = strtod(rdbuf, &s); /* convert last field to a number */
      if (*s || (s == rdbuf) || (weight < 0))
        error(E_VALUE, fn_tab, tplcnt +((header > 0) ? 1 : 2), rdbuf);
    }                           /* get tuple weight/counter */
    xmat[*px][*py] += weight;   /* add weight to confusion matrix */
    tplwgt += weight;           /* and to the tuple weight */
    tplcnt++;                   /* increment the tuple counter */
  } while (d > TFS_EOF);        /* while not at end of file */
  if (in != stdin) fclose(in);  /* close the input file and */
  in = NULL;                    /* clear the file variable */
  fprintf(stderr, "[%d/%g tuple(s)] done.\n", tplcnt, tplwgt);

  /* --- compute number/percentage of errors --- */
  for (c2 = xmat[valcnt] +(y = valcnt+1); --y >= 0; )
    *--c2 = 0;                  /* clear error column (last column) */
  for (x = 0; x < valcnt; x++){ /* traverse the matrix columns */
    c1 = xmat[x];               /* get pointer to current */
    c2 = xmat[valcnt];          /* and to last column */
    for (weight = 0, y = 0; y < valcnt; c1++, c2++, y++)
      if (y != x) { weight += *c1; *c2 += *c1; }
    *c1 = weight;               /* sum column errors and */
  }                             /* note them in last row */
  weight = 0;                   /* traverse error column */
  for (c2 = xmat[valcnt] +(y = valcnt); --y >= 0; )
    weight += *--c2;            /* sum error column and */
  c2[valcnt] = weight;          /* set total errors in last row */
  if (relnum) {                 /* if to compute relative numbers, */
    weight = tplcnt *0.01;      /* determine weight factor */
    for (x = valcnt +1; --x >= 0; ) {
      for (c1 = xmat[x] +(y = valcnt+1); --y >= 0; )
        *--c1 /= weight;        /* traverse all matrix fields */
    }                           /* (including error row/column) and */
  }                             /* divide by the number of tuples */

  /* --- print confusion matrix --- */
  if (fn_mat && *fn_mat)        /* if a matrix file name is given, */
    out = fopen(fn_mat, "w");   /* open confusion matrix file */
  else {                        /* if no matrix file name is given, */
    out = stdout; fn_mat = "<stdout>"; }         /* write to stdout */
  fprintf(stderr, "writing %s ... ", fn_mat);
  if (!out) error(E_FOPEN, fn_mat);
  fmt = (relnum) ? " %5.1f" : " %5g";
  fprintf(out, "confusion matrix for \"%s\" vs. \"%s\":\n",
          yname, xname);        /* print the matrix title */
  if (sort)                     /* sort values alphabetically */
    qsort(vals, valcnt, sizeof(int*), _valcmp);
  fprintf(out, "no | value");   /* print start of header */
  for (i = maxlen -5; --i >= 0; ) putc(' ', out);
  fprintf(out, " | ");          /* fill value column */
  for (x = 0; x < valcnt; x++) fprintf(out, "%5d ", x+1);
  fprintf(out, "| errors\n");   /* print end of header */
  fprintf(out, "---+-");        /* print start of separating line */
  for (i = maxlen; --i >= 0; ) putc('-', out);
  fprintf(out, "-+");           /* fill value column */
  for (x = valcnt; --x >= 0; ) fprintf(out, "------");
  fprintf(out, "-+-------\n");  /* print end of separating line */

  for (y = 0; y < valcnt; y++){ /* traverse the rows of the matrix */
    fprintf(out, "%2d | ",y+1); /* print row number */
    py = vals[y]; yname = st_name(py);
    fprintf(out, yname);        /* print value corresp. to row */
    for (i = maxlen -(int)strlen(yname); --i >= 0; )
      putc(' ', out);           /* fill value column */
    fprintf(out, " |");         /* and print a separator */
    for (x = 0; x < valcnt; x++)
      fprintf(out, fmt, xmat[*(vals[x])][*py]);
    fprintf(out, " |"); fprintf(out, fmt, xmat[valcnt][*py]);
    fprintf(out, "\n");         /* print columns of matrix row */
  }                             /* using the appropriate format */

  fprintf(out, "---+-");        /* print start of separating line */
  for (i = maxlen; --i >= 0; ) putc('-', out);
  fprintf(out, "-+");           /* fill value column */
  for (x = valcnt; --x >= 0; ) fprintf(out, "------");
  fprintf(out, "-+-------\n");  /* print end of separating line */

  fprintf(out, "   | errors");  /* print start of error row */
  for (i = maxlen -6; --i >= 0; ) putc(' ', out);
  fprintf(out, " |");           /* fill the value column */
  for (x = 0; x < valcnt; x++)  /* and print the errors */
    fprintf(out, fmt, xmat[*(vals[x])][valcnt]);
  fprintf(out, " |"); fprintf(out, fmt, xmat[valcnt][valcnt]);
  fprintf(out, "\n");           /* print columns of error row */
  if (out != stdout) {          /* if not written to standard output */
    i = fclose(out); out = NULL;
    if (i) error(E_FWRITE, fn_mat);
  }                             /* close confusion matrix file */
  fprintf(stderr, "done.\n");   /* and print a success message */

  /* --- clean up --- */
  #ifndef NDEBUG                /* if debug version */
  _delmat();                    /* delete confusion matrix, */
  free(vals);                   /* value data vector, */
  tfs_delete(tfscan);           /* table file scanner, */
  st_delete(symtab);            /* and symbol table */
  #endif
  #ifdef STORAGE
  showmem("at end of program"); /* check memory usage */
  #endif
  return 0;                     /* return 'ok' */
}  /* main() */
