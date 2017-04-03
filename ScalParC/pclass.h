
#ifndef	_PCLASS_H
#define _PCLASS_H

#include "common.h"
#include <omp.h>
#include <sys/time.h>
#include <time.h>

extern struct timeval tp;
#define seconds(tm) gettimeofday(&tp,(struct timezone *)0);\
   tm=tp.tv_sec+tp.tv_usec/1000000.0

/*** TYPE DEFINITIONS ***/

typedef struct v{
  int val;
  int rid;
  int cid;
} VR;

#define VR_nTypes 	2

typedef struct cnt {
  int attid; 
  VR *valsrids;
  float max,min;
} Continuous;

#define Cval(p,i,j)	p[i].valsrids[j].val
#define Crid(q,i,j)	q[i].valsrids[j].rid
#define Ccid(q,i,j)	q[i].valsrids[j].cid

#define min(a, b) ((a) < (b) ? (a) : (b))
typedef struct tnd 	*pTNode;
typedef struct tnd {
  int 		Aid;			/* Attribute Id */
  float 	SplitVal;		/* if continuous atr */
  boolean	MoreThanOneClasses;  	/* 0: only one class */
  pTNode 	left; 			/* left kid */
  pTNode 	right;			/* right kid */
  int           index;
} TreeNode;

typedef struct bfsq	*pQueue;
typedef struct bfsq {
  pTNode node;
  int 	*CatrPointer;		/* Pointer into list for this node */
  int 	*CatrSize;		/* Size of the list for this node */
  int	**CatrCabove;		/* Cabove for the attribute */
  pQueue left;
  pQueue right;
  pQueue next;
  int id;
} Queue;


/*** FUNCTION PROTOTYPES ***/


/*** GLOBAL VARIABLES ***/

#ifdef MAIN_FILE 

string *ClassNames;
Continuous *catr;
int *atype;
int nrec;
int nclass;

pTNode TreeRoot;

FILE *fpout;

#else

extern int nclass;
extern string *ClassNames;
extern Continuous *catr;
extern int *atype;
extern int nrec;

extern pTNode TreeRoot;

extern FILE *fpout;

#endif

#endif
