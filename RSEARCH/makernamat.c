/*
 * makernamat.c
 * 
 * Given an MSA with annotated RNA secondary structure, creates a substitution
 * and transition matrix using the BLOSUM algorithm (Henikoff and Henikoff,
 * Proc Natl Acad Sci USA 89:10915-10919 (1992))
 *
 * To fit Sean's style, I've based outline of code on cmbuild.c 
 *
 * Robert J. Klein
 * Started February 25, 2002
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "squid.h"		/* general sequence analysis library    */
#include "msa.h"                /* squid's multiple alignment i/o       */
#include "rnamat.h"

static char banner[] = "makernamat -- build a substiation/transition matrix from an MSA";

static char usage[]  = "\
Usage: makernamat [-options] <alignment file>\n\
  Available options are:\n\
   -h          : help; print brief help on version and usage\n\
   -c <n>      : clustering percentage n (default 100)\n\
   -C <n>      : only count pairs >= n% similar\n\
   -o <file>   : file to put matrix in (default stdout)\n\
";

static char experts[] = "\
   -p            : print probabilities (Qij) rather than log-odds scores\n\
   --informat <s>: specify that input alignment is in format <s>\n\
   --productweights : use product of weights rather than average for counting\n\
";

static struct opt_s OPTIONS[] = {
  { "-h", TRUE, sqdARG_NONE },
  { "-c", TRUE, sqdARG_INT },
  { "-o", TRUE, sqdARG_STRING },
  { "-p", TRUE, sqdARG_NONE },
  { "-C", TRUE, sqdARG_INT },
  { "--informat",  FALSE, sqdARG_STRING },
  { "--productweights", FALSE, sqdARG_NONE},
};
#define NOPTIONS (sizeof(OPTIONS) / sizeof(struct opt_s))

int
main(int argc, char **argv)
{
  char            *alifile;     /* seqfile to read alignment from          */ 
  int              format;	/* format of seqfile                       */
  MSAFILE         *afp;         /* open alignment file                     */
  MSA             *msa;         /* a multiple sequence alignment           */
  int              nali;	/* number of alignments processed          */
  int              idx;

  char *optname;                /* name of option found by Getopt()        */
  char *optarg;                 /* argument found by Getopt()              */
  int   optind;                 /* index in argv[]                         */

  int   clust_perc;             /* Clustering percentage */
  int   cutoff_perc = 0;
  char  outfile[256];
  int   use_other_file = 0;
  FILE *outfp;
  int   use_probabilities = 0;   /* Dump P(A|B) (Qij) rather than lod scores */
  int   product_weights = 0;     /* Use product of weights for counts rather than average */

  fullmat_t fullmat;             /* All matrices information -- paired, \
				    unpaired, gaps */
  double *background_nt;         /* Background nt frequency */
  double cur_q, cur_p;
  int a,b;

  /*********************************************** 
   * Parse command line
   ***********************************************/

  format            = MSAFILE_UNKNOWN;
  clust_perc        = 100;

  while (Getopt(argc, argv, OPTIONS, NOPTIONS, usage,
                &optind, &optname, &optarg))  {
    if (strcmp(optname, "-c") == 0) clust_perc = atoi (optarg);
    else if (strcmp(optname, "-C") == 0) cutoff_perc = atoi(optarg);
    else if (strcmp(optname, "-o") == 0) 
      if (strlen(optarg) > 255)
	Die("Filename %s too long.  Shorten.\n", optarg);
      else {
	strncpy(outfile, optarg, 255);
	use_other_file = 1;
      }
    else if (strcmp(optname, "-p") == 0)
      use_probabilities = 1;
    else if (strcmp(optname, "--informat") == 0) {
      format = String2SeqfileFormat(optarg);
      if (format == MSAFILE_UNKNOWN) 
	Die("unrecognized sequence file format \"%s\"", optarg);
      if (! IsAlignmentFormat(format))
	Die("%s is an unaligned format, can't read as an alignment", optarg);
    }
    else if (strcmp(optname, "--productweights") == 0) {
      product_weights = 1;
    }
    else if (strcmp(optname, "-h") == 0) {
      puts(usage);
      puts(experts);
      exit(EXIT_SUCCESS);
    }
  }

  if (argc - optind != 1) Die("Incorrect number of arguments.\n%s\n", usage);
  alifile = argv[optind++]; 

  /*********************************************** 
   * Preliminaries: open our files for i/o
   ***********************************************/

				/* Open the alignment */
  if ((afp = MSAFileOpen(alifile, format, NULL)) == NULL)
    Die("Alignment file %s could not be opened for reading", alifile);
  
  if (use_other_file == 1) {
    outfp = fopen (outfile, "w");
  } else {
    outfp = stdout;
  }

  /*********************************************** 
   * Show the banner
   ***********************************************/
  
  printf("Alignment file:                    %s\n", alifile);
  printf("File format:                       %s\n", 
	 SeqfileFormat2String(afp->format));
  printf("- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -\n\n");



  /********************************************
   *  Initialize the matrices
   ********************************************/
  fullmat.name = MallocOrDie(sizeof(char)*20);       /* More than enough */
  if (cutoff_perc == 0)
    snprintf (fullmat.name, 20, "%s%d", \
	      use_probabilities ? "RIBOPROB" : "RIBOSUM", clust_perc);
  else
    snprintf (fullmat.name, 20, "%s%d-%d", \
	      use_probabilities ? "RIBOPROB" : "RIBOSUM", clust_perc, cutoff_perc);

  fullmat.unpaired = setup_matrix (RNA_ALPHABET_SIZE);
  fullmat.paired = setup_matrix (RNA_ALPHABET_SIZE*RNA_ALPHABET_SIZE);
  background_nt = (double *)MallocOrDie (sizeof(double)*RNA_ALPHABET_SIZE);
  for (idx=0; idx<RNA_ALPHABET_SIZE; idx++) {
    background_nt[idx] = 0.0;
  }
 
  /********************************************
   * Read in alignments and count frequencies
   ********************************************/
  nali = 0;
  while ((msa = MSAFileRead(afp)) != NULL)
    {
      /* Make all seqs upper case */
      for (idx=0; idx<msa->nseq; idx++)
	s2upper (msa->aseq[idx]);

      /* Print some stuff about what we're about to do.
       */
      if (msa->name != NULL) printf("Alignment:           %s\n",  msa->name);
      else                   printf("Alignment:           #%d\n", nali+1);
      printf                       ("Number of sequences: %d\n",  msa->nseq);
      printf                       ("Number of columns:   %d\n",  msa->alen);
      puts("");
      fflush(stdout);
  
      /*****************************************************
       * Get weights for sequences, using BLOSUM algorithm  
       *****************************************************/
      msa->wgt = MallocOrDie (sizeof(float)*msa->nseq);
      BlosumWeights (msa->aseq, msa->nseq, msa->alen, clust_perc*0.01, msa->wgt);

      /* Count basepairs.  Routine in rnacomp.{c,h} This is equivalent
         to getting Henikoff and Henikoff's Fij */
      count_matrix (msa, &fullmat, background_nt, cutoff_perc, product_weights);
      MSAFree(msa);
    }
  /* Now convert to qij by qij = Fij/sum(all Fij).  Do same for background_nt */
  DNorm (fullmat.unpaired->matrix, fullmat.unpaired->full_size);
  DNorm (fullmat.paired->matrix, fullmat.paired->full_size);
  DNorm (background_nt, RNA_ALPHABET_SIZE);

  if (use_probabilities == 0) {

    /*
     * Now calculate Sij, and E and H at same time
     * Calculate eij = pipj for i==j and 2pipj for i!=j.
     * Sij = log2(qij/eij)
     * H = sum(qij X sij)
     * E = sum(pi X pj X sij)
     */
    for (a=0; a<fullmat.paired->edge_size; a++) {
      for (b=0; b<=a; b++) {
	cur_q = fullmat.paired->matrix[matrix_index(b,a)];
	cur_p = (background_nt[a/RNA_ALPHABET_SIZE]*\
		 background_nt[a%RNA_ALPHABET_SIZE]*\
		 background_nt[b/RNA_ALPHABET_SIZE]*\
		 background_nt[b%RNA_ALPHABET_SIZE]);
	fullmat.paired->matrix[matrix_index(b,a)] = \
	  (cur_q == 0.0) ? \
	  -999 :
	  log(cur_q/ (cur_p * ((a==b) ? 1 : 2)))/log(2);
	fullmat.paired->H += cur_q * fullmat.paired->matrix[matrix_index(b,a)];
	fullmat.paired->E += cur_p * fullmat.paired->matrix[matrix_index(b,a)];
      }
    }
    for (a=0; a<fullmat.unpaired->edge_size; a++) {
      for (b=0; b<=a; b++) {
	cur_q = fullmat.unpaired->matrix[matrix_index(b,a)];
	cur_p = (background_nt[a] * background_nt[b]);
	fullmat.unpaired->matrix[matrix_index(b,a)] = \
	  (cur_q == 0.0) ? \
	  -999 : \
	  log(cur_q/ (cur_p * ((a==b) ? 1 : 2)))/log(2);
	fullmat.unpaired->H += \
	  cur_q * fullmat.unpaired->matrix[matrix_index(b,a)];
	fullmat.unpaired->E += \
	  cur_p * fullmat.unpaired->matrix[matrix_index(b,a)];
      }
    }	
  }

  /* Dump matrix */
  print_matrix (outfp, &fullmat);

  /* Clean up and exit
   */
  free (fullmat.unpaired->matrix);
  free (fullmat.unpaired);
  free (fullmat.paired->matrix);
  free (fullmat.paired);
  MSAFileClose(afp);
  SqdClean();
  return 0;
}


