/*----------------------------------------------------------------------
  File    : attset.h
  Contents: attribute set management
  Author  : Christian Borgelt
  History : 25.10.1995 file created
            03.11.1995 functions as_write and as_read added
            21.12.1995 function att_valsort added
            17.03.1996 attribute types added
            01.07.1996 functions att_valmin and att_valmax added
            04.07.1996 attribute weights added
            24.07.1996 definitions of unknown values added
            22.11.1996 function as_chars added
            25.02.1997 function as_info added
            12.03.1997 attribute marks added
            28.03.1997 function as_weight added
            01.08.1997 restricted read/write/describe added
            02.08.1997 additional information output added
            09.09.1997 function as_scform added
            26.09.1997 error code added to as_err structure
            04.01.1998 separators made attribute set dependent
            06.01.1998 read/write functions made optional
            10.01.1998 variable unknown value characters added
            08.02.1998 function as_parse transferred from parse.h
            18.03.1998 function att_info added
            22.06.1998 deletion function moved to function as_create
            23.06.1998 major redesign, attribute functions introduced
            16.08.1998 lock functions removed, several functions added
            19.08.1998 typedef for attribute values (VAL) added
            22.08.1998 attribute set names and some functions added
            30.08.1998 parameters map and dir added to att_valsort
            02.09.1998 instance (current value) moved to attribute
            06.09.1998 second major redesign completed
            12.09.1998 deletion function parameter changed to ATT
            14.09.1998 attribute selection in as_write/as_desc improved
            17.09.1998 attribute selection improvements completed
            24.09.1998 parameter map added to function att_conv
            25.09.1988 function as_attaddm added
            25.11.1998 fucntions att_valcopy and as_attcopy added
            29.11.1998 functions att_dup and as_dup added
            04.02.1999 long int changed to int
            17.04.1999 definitions of AS_NOXATT and AS_NOXVAL added
            22.11.2000 function sc_form moved to module scan
            13.05.2001 definition of AS_NOUNKS added
            14.07.2001 global variable as_err replaced by a function
----------------------------------------------------------------------*/
#ifndef __ATTSET__
#define __ATTSET__
#include <stdio.h>
#include <limits.h>
#include <float.h>
#ifdef AS_RDWR
#include "tfscan.h"
#endif
#if defined AS_PARSE && !defined SC_SCAN
#define SC_SCAN
#endif
#include "scan.h"

/*----------------------------------------------------------------------
  Preprocessor Definitions
----------------------------------------------------------------------*/
#define CCHAR      const char   /* abbreviation */
#define CINST      const INST   /* ditto */

/* --- attribute types --- */
#define AT_SYM     0x0001       /* symbolic   valued */
#define AT_INT     0x0002       /* integer    valued */
#define AT_FLT     0x0004       /* real/float valued */
#define AT_ALL     0x0007       /* all types (for function as_parse) */
#define AT_AUTO    (-1)         /* automatic type conversion */

/* --- unknown values --- */
#define UV_SYM     (-1)         /* unknown symbolic   value */
#define UV_INT     INT_MIN      /* unknown integer    value */
#define UV_FLT     (-FLT_MAX)   /* unknown real/float value */

/* --- cut/copy flags --- */
#define AS_ALL     0x0000       /* cut/copy all      atts./values */
#define AS_RANGE   0x0010       /* cut/copy range of atts./values */
#define AS_MARKED  0x0020       /* cut/copy marked   atts./values */
#define AS_SELECT  0x0040       /* cut/copy selected attributes */

/* --- read/write flags --- */
#define AS_INST    0x0000       /* read/write instances */
#define AS_ATT     0x0001       /* read/write attributes */
#define AS_DFLT    0x0002       /* create default attribute names */
#define AS_NOXATT  0x0004       /* do not extend set of attributes */
#define AS_NOXVAL  0x0008       /* do not extend set of values */
#define AS_NOEXT   (AS_NOXATT|AS_NOXVAL)   /* do not extend either */
#define AS_NOUNKS  0x0100       /* do not accept unknown values */
#define AS_RDORD   0x0200       /* write fields in read order */
#define AS_ALIGN   0x0400       /* align fields (pad with blanks) */
#define AS_ALNHDR  0x0800       /* align fields respecting a header */
#define AS_WEIGHT  0x1000       /* last field contains inst. weight */
#define AS_INFO1   0x2000       /* write add. info. 1 (before weight) */
#define AS_INFO2   0x4000       /* write add. info. 2 (after  weight) */
/* also applicable: AS_RANGE, AS_MARKED */

/* --- describe flags --- */
#define AS_TITLE   0x0001       /* title with att.set name (comment) */
#define AS_IVALS   0x0002       /* intervals for numeric attributes */
/* also applicable: AS_RANGE, AS_MARKED, AS_WEIGHT */

/* --- sizes --- */
#define AS_MAXLEN     255       /* maximal name length */

/* --- error codes --- */
#ifndef OK
#define OK            0         /* no error */
#define E_NONE        0         /* no error */
#define E_NOMEM     (-1)        /* not enough memory */
#define E_FOPEN     (-2)        /* file open failed */
#define E_FREAD     (-3)        /* file read failed */
#define E_FWRITE    (-4)        /* file write failed */
#endif
#ifndef E_VALUE
#define E_VALUE    (-16)        /* illegal field value */
#define E_FLDCNT   (-17)        /* wrong number of fields */
#define E_EMPFLD   (-18)        /* empty field name */
#define E_DUPFLD   (-19)        /* duplicate field name */
#define E_MISFLD   (-20)        /* missing field name */
#endif

/*----------------------------------------------------------------------
  Type Definitions
----------------------------------------------------------------------*/
typedef union {                 /* --- instance --- */
  int    i;                     /* identifier or integer number */
  float  f;                     /* floating point number */
  char   *s;                    /* pointer to string (unused) */
  void   *p;                    /* arbitrary pointer (unused) */
} INST;                         /* (instance) */

typedef struct _val {           /* --- attribute value --- */
  int          id;              /* identifier (index in attribute) */
  unsigned int hval;            /* hash value of value name */
  struct _val *succ;            /* successor in hash bucket */
  char         name[1];         /* value name */
} VAL;                          /* (attribute value) */

typedef int VAL_CMPFN (const char *name1, const char *name2);

typedef struct _att {           /* --- attribute --- */
  char   *name;                 /* attribute name */
  int    type;                  /* attribute type, e.g. AT_SYM */
  int    read;                  /* read flag (used e.g. in as_read) */
  int    valvsz;                /* size of value vector */
  int    valcnt;                /* number of values in vector */
  VAL    **vals;                /* value vector (symbolic attributes) */
  VAL    **htab;                /* hash table for values */
  INST   min, max;              /* minimal and maximal value/id */
  int    attwd[2];              /* attribute name widths */
  int    valwd[2];              /* maximum of value name widths */
  int    mark;                  /* mark,   e.g. to indicate usage */
  float  weight;                /* weight, e.g. to indicate relevance */
  INST   inst;                  /* instance (current value) */
  INST   info;                  /* additional attribute information */
  struct _attset *set;          /* containing attribute set (if any) */
  int    id;                    /* identifier (index in set) */
  unsigned int hval;            /* hash value of attribute name */
  struct _att *succ;            /* successor in hash bucket */
} ATT;                          /* (attribute) */

typedef void ATT_DELFN (ATT *att);
typedef void ATT_APPFN (ATT *att, void *data);
typedef int  ATT_SELFN (const ATT *att, void *data);

typedef struct _attset {        /* --- attribute set --- */
  char   *name;                 /* name of attribute set */
  int    attvsz;                /* size of attribute vector */
  int    attcnt;                /* number of attributes in vector */
  ATT    **atts;                /* attribute vector */
  ATT    **htab;                /* hash table for attributes */
  float  weight;                /* weight (of current instantiation) */
  INST   info;                  /* info. (for current instantiation) */
  ATT_DELFN *delfn;             /* attribute deletion function */
  #if defined AS_RDWR || defined AS_FLDS
  int    fldvsz;                /* size of field vector */
  int    fldcnt;                /* number of fields */
  int    *flds;                 /* field vector for as_read() */
  #endif                        /* and as_write() (flag AS_RDORD) */
  #ifdef AS_RDWR
  char   chars[8];              /* special characters */
  TFSCAN *tfscan;               /* table file scanner */
  TFSERR *err;                  /* error information */
  char   buf[4*AS_MAXLEN+4];    /* buffer for error information */
  #endif                        /* for function as_read() */
} ATTSET;                       /* (attribute set) */

typedef void AS_DELFN (ATTSET *set);
typedef void INFOUTFN (ATTSET *set, FILE *file, int mode, CCHAR *chars);

/*----------------------------------------------------------------------
  Attribute Functions
----------------------------------------------------------------------*/
extern ATT*    att_create  (const char *name, int type);
extern ATT*    att_dup     (const ATT *att);
extern void    att_delete  (ATT *att);
extern int     att_rename  (ATT *att, const char *name);
extern int     att_conv    (ATT *att, int type, INST *map);
extern int     att_cmp     (const ATT *att1, const ATT *att2);

extern CCHAR*  att_name    (const ATT *att);
extern int     att_type    (const ATT *att);
extern int     att_width   (const ATT *att, int scform);
extern int     att_setmark (ATT *att, int mark);
extern int     att_getmark (const ATT *att);
extern float   att_setwgt  (ATT *att, float weight);
extern float   att_getwgt  (const ATT *att);
extern INST*   att_inst    (ATT *att);
extern INST*   att_info    (ATT *att);
extern ATTSET* att_attset  (ATT *att);
extern int     att_id      (const ATT *att);

/*----------------------------------------------------------------------
  Attribute Value Functions
----------------------------------------------------------------------*/
extern int     att_valadd  (ATT *att, CCHAR *name, INST *inst);
extern void    att_valrem  (ATT *att, int valid);
extern void    att_valexg  (ATT *att, int valid1, int valid2);
extern void    att_valmove (ATT *att, int off, int cnt, int pos);
extern int     att_valcut  (ATT *dst, ATT *src, int mode, ...);
extern int     att_valcopy (ATT *dst, const ATT *src, int mode, ...);
extern void    att_valsort (ATT *att, VAL_CMPFN cmpfn,
                            int *map, int dir);

extern int     att_valid   (const ATT *att, const char *name);
extern CCHAR*  att_valname (const ATT *att, int valid);
extern int     att_valcnt  (const ATT *att);
extern int     att_valwd   (ATT *att, int scform);
extern CINST*  att_valmin  (const ATT *att);
extern CINST*  att_valmax  (const ATT *att);

/*----------------------------------------------------------------------
  Attribute Set Functions
----------------------------------------------------------------------*/
extern ATTSET* as_create   (const char *name, ATT_DELFN delfn);
extern ATTSET* as_dup      (const ATTSET *set);
extern void    as_delete   (ATTSET *set);
extern int     as_rename   (ATTSET *set, const char *name);
extern int     as_cmp      (const ATTSET *set1, const ATTSET *set2);

extern CCHAR*  as_name     (const ATTSET *set);
extern float   as_setwgt   (ATTSET *set, float weight);
extern float   as_getwgt   (const ATTSET *set);
extern INST*   as_info     (ATTSET *set);

extern int     as_attadd   (ATTSET *set, ATT *att);
extern int     as_attaddm  (ATTSET *set, ATT **att, int cnt);
extern ATT*    as_attrem   (ATTSET *set, int attid);
extern void    as_attexg   (ATTSET *set, int attid1, int attid2);
extern void    as_attmove  (ATTSET *set, int off, int cnt, int pos);
extern int     as_attcut   (ATTSET *dst, ATTSET *src, int mode, ...);
extern int     as_attcopy  (ATTSET *dst, const ATTSET *src,
                            int mode, ...);
extern int     as_attid    (const ATTSET *set, const char *name);
extern ATT*    as_att      (ATTSET *set, int attid);
extern int     as_attcnt   (const ATTSET *set);

extern void    as_apply    (ATTSET *set, ATT_APPFN appfn, void *data);
extern int     as_save     (const ATTSET *set, FILE *file);
extern int     as_load     (ATTSET *set, FILE *file);
#ifdef AS_RDWR
extern CCHAR*  as_chars    (ATTSET *set,    CCHAR *blanks,
                            CCHAR *fldseps, CCHAR *recseps,
                            CCHAR *uvchars);
extern TFSCAN* as_tfscan   (ATTSET *set);
extern TFSERR* as_err      (ATTSET *set);
extern int     as_read     (ATTSET *set, FILE *file, int mode, ...);
extern int     as_write    (ATTSET *set, FILE *file, int mode, ...);
#endif
extern int     as_desc     (ATTSET *set, FILE *file, int mode,
                            int maxlen, ...);
#ifdef AS_PARSE
extern int     as_parse    (ATTSET *set, SCAN *scan, int types);
#endif

#ifndef NDEBUG
extern void    as_stats    (const ATTSET *set);
#endif

/*----------------------------------------------------------------------
  Preprocessor Definitions
----------------------------------------------------------------------*/
#define att_name(a)        ((CCHAR*)(a)->name)
#define att_type(a)        ((a)->type)
#define att_width(a,s)     ((a)->attwd[(s) ? 1 : 0])
#define att_setmark(a,m)   ((a)->mark = (m))
#define att_getmark(a)     ((a)->mark)
#define att_setwgt(a,w)    ((a)->weight = (w))
#define att_getwgt(a)      ((a)->weight)
#define att_inst(a)        (&(a)->inst)
#define att_info(a)        (&(a)->info)
#define att_attset(a)      ((a)->set)
#define att_id(a)          ((a)->id)

/*--------------------------------------------------------------------*/
#define att_valname(a,i)   ((CCHAR*)(a)->vals[i]->name)
#define att_valcnt(a)      ((a)->valcnt)
#define att_valmin(a)      ((CINST*)&(a)->min)
#define att_valmax(a)      ((CINST*)&(a)->max)

/*--------------------------------------------------------------------*/
#define as_name(s)         ((CCHAR*)(s)->name)
#define as_info(s)         (&(s)->info)
#define as_getwgt(s)       ((s)->weight)
#define as_setwgt(s,w)     ((s)->weight = (w))

#define as_att(s,i)        ((s)->atts[i])
#define as_attcnt(s)       ((s)->attcnt)

#ifdef AS_RDWR
#define as_tfscan(s)       ((s)->tfscan)
#define as_err(s)          ((s)->err)
#endif

#endif
