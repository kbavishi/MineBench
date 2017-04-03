/*
 * scancyk.c
 *
 * Scanning CYK algorithm using INFERNAL code base.  Based on scanning
 * algorithm described in Durbin, et al., Biological Sequence Analysis,
 * pp. 289-293
 *
 * Robert J. Klein
 * March 24, 2002
 */

#include <stdio.h>
#include <stdlib.h>
#include "squid.h"
#include "sqfuncs.h"
#include "structs.h"
#include "funcs.h"
#include "histogram.h"

#ifdef _OPENMP
#include <omp.h>
#endif

/*
 * Function: CreateResults ()
 * Date:     RJK, Mon Apr 1 2002 [St. Louis]
 * Purpose:  Creates a results type of specified size
 */
scan_results_t *CreateResults (int size) {
  scan_results_t *results;
  results = MallocOrDie (sizeof(scan_results_t));
  results->num_results = 0;
  results->num_allocated = size;
  results->data = MallocOrDie(sizeof(scan_result_node_t)*size);
  return (results);
}

/* Function: ExpandResults ()
 * Date:     RJK, Mon Apr 1 2002 [St. Louis]
 * Purpose:  Expans a results structure by specified amount
 */
void ExpandResults (scan_results_t *results, int additional) {
  results->data = ReallocOrDie (results->data, 
				sizeof(scan_result_node_t)*
				(results->num_allocated+additional));
  results->num_allocated+=additional;
}

/*
 * Function: FreeResults()
 * Date:     RJK, Mon Apr 1 2002 [St. Louis]
 * Purpose:  Frees a results structure
 */
void FreeResults (scan_results_t *r) {
  int i;
  if (r != NULL) {
    for (i=0; i<r->num_results; i++) {
      if (r->data[i].tr != NULL) {
	FreeParsetree(r->data[i].tr);
      }
    }
    free (r->data);
    free(r);
  }
}


void SFree_Results (scan_results_t *r) {
  int i;
  if (r != NULL) {
    free (r->data);
    free(r);
  }
}
/*
 * Function: compare_results()
 * Date:     RJK, Wed Apr 10, 2002 [St. Louis]
 * Purpose:  Compares two scan_result_node_ts based on score and returns -1
 *           if first is higher score than second, 0 if equal, 1 if first
 *           score is lower.  This results in sorting by score, highest
 *           first.
 */
int compare_results (const void *a_void, const void *b_void) {
  scan_result_node_t *a, *b;
 
  a = (scan_result_node_t *)a_void;
  b = (scan_result_node_t *)b_void;

  if (a->score < b->score)
    return (1);
  else if (a->score > b->score)
    return (-1);
  else if (a->start < b->start)
    return (1);
  else if (a->start > b->start)
    return (-1);
  else
    return (0);
}

/*
 * Function: sort_results()
 * Date:    RJK,  Sun Mar 31, 2002 [AA Flight 2869 LGA->STL]
 * Purpose: Given a results array, sorts it with a call to qsort
 *
 */
void sort_results (scan_results_t *results) {
  qsort (results->data, results->num_results, sizeof(scan_result_node_t), compare_results);
}

/*
 * Function: report_hit()
 * Date:     RJK, Sun Mar 31, 2002 [LGA Gate D7]
 *
 * Given j,d, coordinates, a score, and a scan_results_t data type,
 * adds result into the set of reportable results.  Naively adds hit.
 *
 * Non-overlap algorithm is now done in the scanning routine by Sean's
 * Semi-HMM code.  I've just kept the hit report structure for convenience.
 */
void report_hit (int i, int j, int bestr, float score, scan_results_t *results) {
  if (results->num_results == results->num_allocated) {
    ExpandResults (results, INIT_RESULTS);
  }
  results->data[results->num_results].score = score;
  results->data[results->num_results].start = i;
  results->data[results->num_results].stop = j;
  results->data[results->num_results].bestr = bestr;
  results->data[results->num_results].tr = NULL;
  results->num_results++;
}

/*
 * Function: remove_overlapping_hits ()
 * Date:     RJK, Sun Mar 31, 2002 [LGA Gate D7]
 *
 * Purpose:  Given a list of hits, removes overlapping hits to produce
 * a list consisting of at most one hit covering each nucleotide in the
 * sequence.  Works as follows:
 * 1.  Bubble sort hits (I know this is ridiculously slow, but I want
 *     to get something working.  I can replace this later with a faster
 *     algorithm.)  (O(N^2))
 * 2.  For each hit, sees if any nucleotide covered yet
 *     If yes, remove hit
 *     If no, mark each nt as covered
 */
void remove_overlapping_hits (scan_results_t *results, int L) {
  char *covered_yet;
  int x,y;
  int covered;
  scan_result_node_t swap;

  if (results == NULL)
    return;

  if (results->num_results == 0)
    return;

  covered_yet = MallocOrDie (sizeof(char)*(L+1));
  for (x=0; x<=L; x++)
    covered_yet[x] = 0;

  sort_results (results);

  for (x=0; x<results->num_results; x++) {
    covered = 0;
    for (y=results->data[x].start; y<=results->data[x].stop && !covered; y++) {
      if (covered_yet[y] != 0) {
	covered = 1;
      } 
    }
    if (covered == 1) {
      results->data[x].start = -1;        /* Flag -- remove later to keep sorted */
    } else {
      for (y=results->data[x].start; y<=results->data[x].stop; y++) {
	covered_yet[y] = 1;
      }
    }
  }
  free (covered_yet);

  for (x=0; x<results->num_results; x++) {
    while (results->num_results > 0 &&
	   results->data[results->num_results-1].start == -1)
      results->num_results--;
    if (x<results->num_results && results->data[x].start == -1) {
      swap = results->data[x];
      results->data[x] = results->data[results->num_results-1];
      results->data[results->num_results-1] = swap;
      results->num_results--;
    }
  }
  while (results->num_results > 0 &&
	 results->data[results->num_results-1].start == -1)
    results->num_results--;

  sort_results(results);
}

/* Function: CYKScan()
 * Date:     RJK, Sun Mar 24, 2002 [STL->DCA]
 *           SRE, Mon Aug  7 13:15:37 2000 [St. Louis] 
 *                   (from inside() in smallcyk.c)
 *
 * Purpose:  Run the inside phase of a CYK alignment algorithm, on 
 *           a complete sequence of length L.
 *
 * The following changes were made from inside() in smallyck.c:
 * 1.  Removed jp, i0, j0, W, vroot, and vend because full sequence
 * 2.  Added d<=D constraint on d for loops
 * 3.  Removed shadow matrices; we don't do traceback
 * 4.  Replace alpha with gamma[j][d][v] -- makes more efficient use of cache
 * 5.  Explicitiy define End state rather than just assigning the pre-computed
 *     "end deck" 
 * 6.  Use gamma_begl_s and gamma_begr_s [v][j][d] for optimizing bifurcation
 *     states
 * 7.  Local alignment now.
 * 8.  If passed a histogram, fills in with best j for every D+1 place
 * 9.  Only reports best hit at each j to reduce reporting complexity
 * 10. Modified so minimize dereferencing for improved speed.
 * 11. Inner loops rewritten to allow vectorization.
 *
 * Args:     cm        - the model    [0..M-1]
 *           dsq       - the sequence [1..L]   
 *           L         - length of the dsq
 *           cutoff    - cutoff score to report
 *           D         - maximum size of hit
 *           results   - scan_results_t to fill in; if NULL, nothing
 *                       filled in
 *
 * Returns: Score of best hit overall
 *
 */
float CYKScan (CM_t *cm, char *dsq, int L, float cutoff, int D,
	       scan_results_t *results) {

  int     *bestr;               /* Best root state for d at current j */
  int      v,y,z;		/* indices for states  */
  int      j,d,i,k;		/* indices in sequence dimensions */
  int      jmod2, jmin1mod2;    /* For indices into the actual j dimension */
  int      dmax;                /* D of best hit at j */
  float    sc;  	       	/* a temporary variable holding a score */
  int      yoffset;		/* y=base+offset -- counter in child 
                                   states that v can transit to */
  int      M;                   /* Stores cm->M for loop limits */
  int      cnum;                /* Stores cm->cnum[v] for loop limits */
  int      minDj;               /* Minimum cutoff for d between j and D */
  float ***gamma;               /* The main DP matrix [j][d][v] */
  float ***gamma_begl_s;        /* For BEGL_S states -- [v][i][d] */
  float ***gamma_begr_s;        /* For BEGR_S states -- [v][j][d] */ 
  float  **gamma_jmod2;         /* gamma[jmod2] */
  float  **gamma_jmin1mod2;     /* gamma[jmin1mod2] */
  float   *gammap;              /* Pointer to last dimension of gamma to use */
  float   *gamma_begl_s_p;     
  float   *gamma_begr_s_p;
  int       sc_v_size;
  float    *sc_v;               /* Vector of possible scores to maximize over */
  float    *tsc;                /* Points to cm->tsc[v] to make pointer operation simpler in loop I want to vectorize */
  float     endsc;              /* endsc for current state [v] -- set at
				   beginning of each v loop */
  float     beginsc;            /* beginsc for current state[y] */
  char      sttype;             /* Holds cm->sttype[v] */
  float     best_score = IMPOSSIBLE;     /* Best overall score to return */

  /* Set M */
  M = cm->M;

  if (M>D+1) {
    sc_v_size = M;
  } else {
    sc_v_size = D+1;
  }
  /* Initialize sc_v to size of M */
  sc_v = MallocOrDie (sizeof(float) * sc_v_size);

  gamma = MallocOrDie(sizeof(float **) * 2);
  gamma[0] = MallocOrDie(sizeof(float *)*2*M);
  gamma[1] = gamma[0] + M;
  gamma[0][0] = MallocOrDie(sizeof(float)*2*(D+1)*M);
  gamma[1][0] = gamma[0][0] + ((D+1)*M);
  for (v=1; v<M; v++) {
    gamma[0][v] = gamma[0][v-1] + (D+1);
    gamma[1][v] = gamma[1][v-1] + (D+1);
  }

  gamma_begl_s = MallocOrDie(sizeof(float **)*M);
  for (v=0; v<M; v++) {
    if (cm->stid[v] == BEGL_S) {
      /* For Bifurcatoins, we may need up to D+1 */
      gamma_begl_s[v] = MallocOrDie(sizeof(float *) * (D+1));
      for (j=0; j<D+1; j++) 
	gamma_begl_s[v][j] = MallocOrDie(sizeof(float)*(D+1));
    } else {
      gamma_begl_s[v] = NULL;
    }
  }
  gamma_begr_s = MallocOrDie(sizeof(float **)*M);
  for (v=0; v<M; v++) {
    if (cm->stid[v] == BEGR_S) {
      gamma_begr_s[v] = MallocOrDie(sizeof(float *)*2);
      for (j=0; j<2; j++)
	gamma_begr_s[v][j] = MallocOrDie(sizeof(float)*(D+1));
    } else {
      gamma_begr_s[v] = NULL;
    }
  }

  bestr = MallocOrDie(sizeof(int)*(D+1));

  /* Main recursion */
  for (j=0; j<=L; j++) {
    jmod2 = j % 2;
    if (j == 0)	
      jmin1mod2 = 1;
    else 
      jmin1mod2 = (j-1)%2;
    gamma_jmod2 = gamma[jmod2];
    gamma_jmin1mod2 = gamma[jmin1mod2];
    if (j < D) {
      minDj = j;
    } else {
      minDj = D;
    }
    for (v = M-1; v > 0; v--) {          /* Handle ROOT specially */ 
      endsc = cm->endsc[v];              /* It shouldn't change in this loop */
      sttype = cm->sttype[v];            /* This also shouldn't change */
      if (sttype == E_st) {
	gammap = gamma_jmod2[v];
	*gammap = 0.;
	for (d=1; d<=minDj; d++) {
	  gammap++;
	  *gammap = IMPOSSIBLE;  /* gamma[jmod2][v][d] */
	}
      } 
      else if (sttype == D_st || sttype == S_st) {
	y = cm->cfirst[v];
	cnum = cm->cnum[v];
	tsc = cm->tsc[v];
	for (d = 0; d <= minDj; d++)
	  sc_v[d] = endsc;
	for (yoffset= 0; yoffset < cnum; yoffset++) {
	  gammap = gamma_jmod2[y+yoffset];
#ifdef INTEL_COMPILER
#pragma ivdep
#endif
	  for (d = 0; d <= minDj; d++) {
	    sc = gammap[d] + tsc[yoffset];
	    if (sc > sc_v[d])
	      sc_v[d] = sc;
	  }
	}
	gammap = gamma_jmod2[v];
	for (d = 0; d<= minDj; d++) {
	  sc = sc_v[d];
	  if (sc<IMPOSSIBLE) sc = IMPOSSIBLE;
	  gammap[d] = sc;
	  if (cm->stid[v] == BEGL_S) gamma_begl_s[v][(j-d+1)%(D+1)][d] = sc;
	  if (cm->stid[v] == BEGR_S) gamma_begr_s[v][jmod2][d] = sc;
	}
      }
      else if (sttype == B_st) {
	y = cm->cfirst[v];
	z = cm->cnum[v];
	for (d = 0; d <= minDj; d++) {
	  sc = endsc;
	  gamma_begl_s_p = gamma_begl_s[y][(j-d+1)%(D+1)];
	  gamma_begr_s_p = gamma_begr_s[z][jmod2];
	  for (k=0; k<=d; k++)
	    sc_v[k] = gamma_begl_s_p[d-k];
#ifdef INTEL_COMPILER
#pragma ivdep
#endif
	  for (k = 0; k <= d; k++) {
	    sc_v[k] += gamma_begr_s_p[k];
	    if (sc_v[k] > sc) sc = sc_v[k];
	  }
	  if (sc<IMPOSSIBLE) sc = IMPOSSIBLE;
	  gamma_jmod2[v][d] = sc;
	}
      }
      else if (sttype == MP_st) {
	gamma_jmod2[v][0] = IMPOSSIBLE;
	if (j>0) gamma_jmod2[v][1] = IMPOSSIBLE;
	y = cm->cfirst[v];
	cnum = cm->cnum[v];
	tsc = cm->tsc[v];
	for (d = 2; d <= minDj; d++) 
	  sc_v[d] = endsc;
	for (yoffset = 0; yoffset < cnum; yoffset++) {
	  gammap = gamma_jmin1mod2[y+yoffset];
#ifdef INTEL_COMPILER
#pragma ivdep
#endif
	  for (d = 2; d <= minDj; d++) {
	    sc = gammap[d-2] + tsc[yoffset];
	    if (sc > sc_v[d]) {
	      sc_v[d] = sc;
	    }
	  }
	}
	for (d = 2; d <= minDj; d++) {
	  i = j-d+1;
	  sc = sc_v[d];
	  if (dsq[i] < Alphabet_size && dsq[j] < Alphabet_size)
	    sc += cm->esc[v][(int) (dsq[i]*Alphabet_size+dsq[j])];
	  else
	    sc += DegeneratePairScore(cm->esc[v], dsq[i], dsq[j]);
	  if (sc < IMPOSSIBLE) sc = IMPOSSIBLE;
	  gamma_jmod2[v][d] = sc;
	}
      }
      else if (sttype == ML_st) {                /* IL_st done below
						    because it points to
						    itself so gamma[j][v][d]
						    depends on gamma[j][v][d-1]
						 */
	gamma_jmod2[v][0] = IMPOSSIBLE;
	y = cm->cfirst[v];
	cnum = cm->cnum[v];
	tsc = cm->tsc[v];
	for (d = 1; d <= minDj; d++) 
	  sc_v[d] = endsc;
	for (yoffset=0; yoffset<cnum; yoffset++) {
	  gammap = gamma_jmod2[y+yoffset];
#ifdef INTEL_COMPILER
#pragma ivdep
#endif
	  for (d = 1; d <= minDj; d++) {
	    sc = gammap[d-1] + tsc[yoffset];
	    if (sc > sc_v[d]) {
	      sc_v[d] = sc;
	    }
	  }
	}
	for (d = 1; d <= minDj; d++) {
	  i = j-d+1;
	  sc = sc_v[d];
	  if (dsq[i] < Alphabet_size)
	    sc += cm->esc[v][(int) dsq[i]];
	  else
	    sc += DegenerateSingletScore(cm->esc[v], dsq[i]);
	  if (sc<IMPOSSIBLE) sc = IMPOSSIBLE;
	  gamma_jmod2[v][d] = sc;
	}
      }
      else if (sttype == IL_st) {         /* ML dealt with above, iterating
					     yoffset before d.  Can't do that
					     here because gamma[j][v][d] 
					     depends on gamma[j][v][d-1] */
	gamma_jmod2[v][0] = IMPOSSIBLE;
	y = cm->cfirst[v];
	cnum = cm->cnum[v];
	tsc = cm->tsc[v];
	for (d = 1; d <= minDj; d++) {
	  sc = endsc;
	  for (yoffset=0; yoffset<cnum; yoffset++) {
	    sc_v[yoffset] = gamma_jmod2[y+yoffset][d-1] + tsc[yoffset];
	    if (sc_v[yoffset] > sc) {
	      sc = sc_v[yoffset];
	    }
	  }
	  i = j-d+1;
	  if (dsq[i] < Alphabet_size)
	    sc += cm->esc[v][(int) dsq[i]];
	  else
	    sc += DegenerateSingletScore(cm->esc[v], dsq[i]);
	  if (sc<IMPOSSIBLE) sc = IMPOSSIBLE;
	  gamma_jmod2[v][d] = sc;
	}
      }
      else if (sttype == IR_st || sttype == MR_st) {
	gamma_jmod2[v][0] = IMPOSSIBLE;
	y = cm->cfirst[v];
	cnum = cm->cnum[v];
	tsc = cm->tsc[v];
	for (d = 1; d <= minDj; d++) 
	  sc_v[d] = endsc;
	for (yoffset = 0; yoffset < cnum; yoffset++) {
	  gammap = gamma_jmin1mod2[y+yoffset];
#ifdef INTEL_COMPILER
#pragma ivdep
#endif
	  for (d = 1; d <= minDj; d++) {
	    sc = gammap[d-1] + tsc[yoffset];
	    if (sc > sc_v[d]) {
	      sc_v[d] = sc;
	    }
	  }
	}
	for (d = 1; d <= minDj; d++) {
	  sc = sc_v[d];
	  if (dsq[j] < Alphabet_size)
	    sc += cm->esc[v][(int) dsq[j]];
	  else
	    sc += DegenerateSingletScore(cm->esc[v], dsq[j]);
	  if (sc < IMPOSSIBLE) sc = IMPOSSIBLE;
	  gamma_jmod2[v][d] = sc;
	}
      }
    }

    /* Now do ROOT_S (v=0) -- local begins */
    /* First do standard states to transition to */
    y = cm->cfirst[0];
    cnum=cm->cnum[0];
    tsc = cm->tsc[0];
    for (d = 0; d <= minDj; d++)
      sc_v[d] = IMPOSSIBLE;
    for (yoffset = 0; yoffset < cnum; yoffset++) {
      gammap = gamma_jmod2[y+yoffset];
#ifdef INTEL_COMPILER
#pragma ivdep
#endif
      for (d = 0; d <= minDj; d++) {
	sc = gammap[d] + tsc[yoffset];
	if (sc > sc_v[d]) {
	  sc_v[d] = sc;
	  bestr[d] = y+yoffset;
	}
      }
    }
    /* Now, if doing local BEGINS, try that */
    if (cm->flags & CM_LOCAL_BEGIN) {
      tsc = cm->beginsc;         /* Really cm->beginsc, not tsc */
      for (y = 1; y < M; y++) {
	beginsc = tsc[y];
	gammap = gamma_jmod2[y];
#ifdef INTEL_COMPILER
#pragma ivdep
#endif
	for (d = 0; d <= minDj; d++) {
	  sc = gammap[d] + beginsc;
	  if (sc > sc_v[d]) {
	    sc_v[d] = sc;
	    bestr[d] = y;
	  }
	}
      }
    }
    gammap = gamma_jmod2[0];
#ifdef INTEL_COMPILER
#pragma ivdep
#endif
    for (d = 0; d <= minDj; d++) {
      sc = sc_v[d];
      if (sc<IMPOSSIBLE) sc = IMPOSSIBLE;
      gammap[d] = sc;
    }

    if (results != NULL) {
      /* Now, report the hit.  At least one hit is sent back for each j here.
	 However, some hits can already be removed for the greedy overlap
	 resolution algorithm.  Specifically, at the given j, any hit with a
	 d of d1 is guaranteed to mask any hit of lesser score with a d > d1 */
      /* First, report hit with d of 1 if > cutoff */
      if (j > 0 && gamma_jmod2[0][1] >= cutoff) 
	report_hit (j, j, bestr[1], gamma_jmod2[0][1], results);

      dmax = 1;
      /* Now, if current score is greater than maximum seen previous, report
	 it if >= cutoff and set new max */
      for (d=2; d<=minDj; d++) {
	if (gamma_jmod2[0][d] > gamma_jmod2[0][dmax]) {
	  if (j > 0 && gamma_jmod2[0][d] >= cutoff)
	    report_hit (j-d+1, j, bestr[d], gamma_jmod2[0][d], results);
	  dmax = d;
	}
      }
    }
    for (d=1; d<=minDj; d++) {
      if (j > 0 && gamma_jmod2[0][d] > best_score) {
	best_score = gamma_jmod2[0][d];
      }
    }

  }
  free(gamma[0][0]);
  free(gamma[0]);
  free(gamma);

  for (v=0; v<M; v++) {
    if (gamma_begl_s[v] != NULL) {
      for (d=0; d<=D; d++) {
	free(gamma_begl_s[v][d]);
      }
      free(gamma_begl_s[v]);
    }
  }
  free (gamma_begl_s);

  for (v=0; v<M; v++) {
    if (gamma_begr_s[v] != NULL) {
      free(gamma_begr_s[v][0]);
      free(gamma_begr_s[v][1]);
      free(gamma_begr_s[v]);
    }
  }
  free(gamma_begr_s);

  free(bestr);

  free(sc_v);

  return (best_score);
}


