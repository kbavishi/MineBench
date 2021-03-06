/*
  File autogenerated by gengetopt version 2.11
  generated with the following command:
  /cs/++/phd/ninio/gengetopt-2.11/src/gengetopt -isimulateSequnce.ggo -FsimulateSequnce_cmdline 

  The developers of gengetopt consider the fixed text that goes in all
  gengetopt output files to be in the public domain:
  we make no copyright claims on it.
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* If we use autoconf.  */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "getopt.h"

#include "simulateSequnce_cmdline.h"

void
cmdline_parser_print_version (void)
{
  printf ("%s %s\n", CMDLINE_PARSER_PACKAGE, CMDLINE_PARSER_VERSION);
}

void
cmdline_parser_print_help (void)
{
  cmdline_parser_print_version ();
  printf("\n"
  "Purpose:\n"
  "  sequence simulation for a give tree\n\n  Command line $Revision: 2399 $ from $Date: 2014-03-13 17:43:51 -0500 (Thu, 13 Mar 2014) $\n"
  "\n"
  "Usage: %s [OPTIONS]...\n", CMDLINE_PARSER_PACKAGE);
  printf("\n");
  printf("  -h, --help                 Print help and exit\n");
  printf("  -V, --version              Print version and exit\n");
  printf("  -t, --tree=FILENAME        Tree file name\n");
  printf("  -n, --length=INT           Required sequence length\n");
  printf("  -r, --seed=LONG            Seed random number generator\n");
  printf("  -o, --outputfile=FILENAME  Output sequence file  (default=`-')\n");
  printf("  -f, --format=STRING        Sequence format: [phylip], clustal, molphy, mase, \n                               fasta  (default=`phylip')\n");
  printf("\nModel Options:\n");
  printf("  -a, --alphabet=4|20        Alphabet Size  (default=`20')\n");
  printf("  -z, --ratio=FLOAT          Transition/Transversion ratio  (default=`2.0')\n");
  printf("  -p, --ACGprob=A,C,G        User input nucleotide frequencies. String \n                               separated list for A,C,G  (default=\n                               `0.25,0.25,0.25')\n");
  printf("  -G, --gamma=Alpha          Use Gamma RVAS (4 bins) and set alpha  (default=\n                               `0.3')\n");
  printf("      --inputRate=FLOAT      Set External globalRate  (default=`1.0')\n");
  printf("  -R, --rateVector=FILENAME  filename for rate pre position\n");
  printf("\n");
  printf(" Group: Model - Chose Evolutionary Model\n");
  printf("      --day                  Use 'day' model\n");
  printf("      --jtt                  Use 'jtt' model (default)\n");
  printf("      --rev                  Use 'rev' model\n");
  printf("      --wag                  Use 'wag' model\n");
  printf("      --cprev                Use 'cprev' model\n");
  printf("      --nucjc                Use nucleic acid JC model\n");
  printf("      --aaJC                 Use amino acid JC model\n");
  printf("      --k2p                  Use 'k2p' model\n");
  printf("      --hky                  Use 'k2p' model\n");
  printf("      --modelfile=NAME       Use user input file as model\n");
  printf("\nLog Options:\n");
  printf("  -v, --verbose=INT          Log report level (verbose)  (default=`1')\n");
  printf("  -l, --Logfile=FILENAME     Log output file name  (default=`-')\n");
}


static char *gengetopt_strdup (const char *s);

/* gengetopt_strdup() */
/* strdup.c replacement of strdup, which is not standard */
char *
gengetopt_strdup (const char *s)
{
  char *result = (char*)malloc(strlen(s) + 1);
  if (result == (char*)0)
    return (char*)0;
  strcpy(result, s);
  return result;
}

int
cmdline_parser (int argc, char * const *argv, struct gengetopt_args_info *args_info)
{
  int c;	/* Character of the parsed option.  */
  int missing_required_options = 0;
  int Model_group_counter = 0;
  

  args_info->help_given = 0 ;
  args_info->version_given = 0 ;
  args_info->tree_given = 0 ;
  args_info->length_given = 0 ;
  args_info->seed_given = 0 ;
  args_info->outputfile_given = 0 ;
  args_info->format_given = 0 ;
  args_info->alphabet_given = 0 ;
  args_info->ratio_given = 0 ;
  args_info->ACGprob_given = 0 ;
  args_info->gamma_given = 0 ;
  args_info->inputRate_given = 0 ;
  args_info->rateVector_given = 0 ;
  args_info->day_given = 0 ;
  args_info->jtt_given = 0 ;
  args_info->rev_given = 0 ;
  args_info->wag_given = 0 ;
  args_info->cprev_given = 0 ;
  args_info->nucjc_given = 0 ;
  args_info->aaJC_given = 0 ;
  args_info->k2p_given = 0 ;
  args_info->hky_given = 0 ;
  args_info->modelfile_given = 0 ;
  args_info->verbose_given = 0 ;
  args_info->Logfile_given = 0 ;
#define clear_args() { \
  args_info->tree_arg = NULL; \
  args_info->outputfile_arg = gengetopt_strdup("-") ;\
  args_info->format_arg = gengetopt_strdup("phylip") ;\
  args_info->alphabet_arg = 20 ;\
  args_info->ratio_arg = 2.0 ;\
  args_info->ACGprob_arg = gengetopt_strdup("0.25,0.25,0.25") ;\
  args_info->gamma_arg = 0.3 ;\
  args_info->inputRate_arg = 1.0 ;\
  args_info->rateVector_arg = NULL; \
  args_info->modelfile_arg = NULL; \
  args_info->verbose_arg = 1 ;\
  args_info->Logfile_arg = gengetopt_strdup("-") ;\
}

  clear_args();

  optarg = 0;
  optind = 1;
  opterr = 1;
  optopt = '?';

  while (1)
    {
      int option_index = 0;
      char *stop_char;

      static struct option long_options[] = {
        { "help",	0, NULL, 'h' },
        { "version",	0, NULL, 'V' },
        { "tree",	1, NULL, 't' },
        { "length",	1, NULL, 'n' },
        { "seed",	1, NULL, 'r' },
        { "outputfile",	1, NULL, 'o' },
        { "format",	1, NULL, 'f' },
        { "alphabet",	1, NULL, 'a' },
        { "ratio",	1, NULL, 'z' },
        { "ACGprob",	1, NULL, 'p' },
        { "gamma",	1, NULL, 'G' },
        { "inputRate",	1, NULL, 0 },
        { "rateVector",	1, NULL, 'R' },
        { "day",	0, NULL, 0 },
        { "jtt",	0, NULL, 0 },
        { "rev",	0, NULL, 0 },
        { "wag",	0, NULL, 0 },
        { "cprev",	0, NULL, 0 },
        { "nucjc",	0, NULL, 0 },
        { "aaJC",	0, NULL, 0 },
        { "k2p",	0, NULL, 0 },
        { "hky",	0, NULL, 0 },
        { "modelfile",	1, NULL, 0 },
        { "verbose",	1, NULL, 'v' },
        { "Logfile",	1, NULL, 'l' },
        { NULL,	0, NULL, 0 }
      };

      stop_char = 0;
      c = getopt_long (argc, argv, "hVt:n:r:o:f:a:z:p:G:R:v:l:", long_options, &option_index);

      if (c == -1) break;	/* Exit from `while (1)' loop.  */

      switch (c)
        {
        case 'h':	/* Print help and exit.  */
          clear_args ();
          cmdline_parser_print_help ();
          exit (EXIT_SUCCESS);

        case 'V':	/* Print version and exit.  */
          clear_args ();
          cmdline_parser_print_version ();
          exit (EXIT_SUCCESS);

        case 't':	/* Tree file name.  */
          if (args_info->tree_given)
            {
              fprintf (stderr, "%s: `--tree' (`-t') option given more than once\n", CMDLINE_PARSER_PACKAGE);
              clear_args ();
              exit (EXIT_FAILURE);
            }
          args_info->tree_given = 1;
          args_info->tree_arg = gengetopt_strdup (optarg);
          break;

        case 'n':	/* Required sequence length.  */
          if (args_info->length_given)
            {
              fprintf (stderr, "%s: `--length' (`-n') option given more than once\n", CMDLINE_PARSER_PACKAGE);
              clear_args ();
              exit (EXIT_FAILURE);
            }
          args_info->length_given = 1;
          args_info->length_arg = strtol (optarg,&stop_char,0);
          break;

        case 'r':	/* Seed random number generator.  */
          if (args_info->seed_given)
            {
              fprintf (stderr, "%s: `--seed' (`-r') option given more than once\n", CMDLINE_PARSER_PACKAGE);
              clear_args ();
              exit (EXIT_FAILURE);
            }
          args_info->seed_given = 1;
          args_info->seed_arg = strtol (optarg,&stop_char,0);
          break;

        case 'o':	/* Output sequence file.  */
          if (args_info->outputfile_given)
            {
              fprintf (stderr, "%s: `--outputfile' (`-o') option given more than once\n", CMDLINE_PARSER_PACKAGE);
              clear_args ();
              exit (EXIT_FAILURE);
            }
          args_info->outputfile_given = 1;
          if (args_info->outputfile_arg)
            free (args_info->outputfile_arg); /* free default string */
          args_info->outputfile_arg = gengetopt_strdup (optarg);
          break;

        case 'f':	/* Sequence format: [phylip], clustal, molphy, mase, fasta.  */
          if (args_info->format_given)
            {
              fprintf (stderr, "%s: `--format' (`-f') option given more than once\n", CMDLINE_PARSER_PACKAGE);
              clear_args ();
              exit (EXIT_FAILURE);
            }
          args_info->format_given = 1;
          if (args_info->format_arg)
            free (args_info->format_arg); /* free default string */
          args_info->format_arg = gengetopt_strdup (optarg);
          break;

        case 'a':	/* Alphabet Size.  */
          if (args_info->alphabet_given)
            {
              fprintf (stderr, "%s: `--alphabet' (`-a') option given more than once\n", CMDLINE_PARSER_PACKAGE);
              clear_args ();
              exit (EXIT_FAILURE);
            }
          args_info->alphabet_given = 1;
          args_info->alphabet_arg = strtol (optarg,&stop_char,0);
          break;

        case 'z':	/* Transition/Transversion ratio.  */
          if (args_info->ratio_given)
            {
              fprintf (stderr, "%s: `--ratio' (`-z') option given more than once\n", CMDLINE_PARSER_PACKAGE);
              clear_args ();
              exit (EXIT_FAILURE);
            }
          args_info->ratio_given = 1;
          args_info->ratio_arg = (float)strtod (optarg, NULL);
          break;

        case 'p':	/* User input nucleotide frequencies. String separated list for A,C,G.  */
          if (args_info->ACGprob_given)
            {
              fprintf (stderr, "%s: `--ACGprob' (`-p') option given more than once\n", CMDLINE_PARSER_PACKAGE);
              clear_args ();
              exit (EXIT_FAILURE);
            }
          args_info->ACGprob_given = 1;
          if (args_info->ACGprob_arg)
            free (args_info->ACGprob_arg); /* free default string */
          args_info->ACGprob_arg = gengetopt_strdup (optarg);
          break;

        case 'G':	/* Use Gamma RVAS (4 bins) and set alpha.  */
          if (args_info->gamma_given)
            {
              fprintf (stderr, "%s: `--gamma' (`-G') option given more than once\n", CMDLINE_PARSER_PACKAGE);
              clear_args ();
              exit (EXIT_FAILURE);
            }
          args_info->gamma_given = 1;
          args_info->gamma_arg = (float)strtod (optarg, NULL);
          break;

        case 'R':	/* filename for rate pre position.  */
          if (args_info->rateVector_given)
            {
              fprintf (stderr, "%s: `--rateVector' (`-R') option given more than once\n", CMDLINE_PARSER_PACKAGE);
              clear_args ();
              exit (EXIT_FAILURE);
            }
          args_info->rateVector_given = 1;
          args_info->rateVector_arg = gengetopt_strdup (optarg);
          break;

        case 'v':	/* Log report level (verbose).  */
          if (args_info->verbose_given)
            {
              fprintf (stderr, "%s: `--verbose' (`-v') option given more than once\n", CMDLINE_PARSER_PACKAGE);
              clear_args ();
              exit (EXIT_FAILURE);
            }
          args_info->verbose_given = 1;
          args_info->verbose_arg = strtol (optarg,&stop_char,0);
          break;

        case 'l':	/* Log output file name.  */
          if (args_info->Logfile_given)
            {
              fprintf (stderr, "%s: `--Logfile' (`-l') option given more than once\n", CMDLINE_PARSER_PACKAGE);
              clear_args ();
              exit (EXIT_FAILURE);
            }
          args_info->Logfile_given = 1;
          if (args_info->Logfile_arg)
            free (args_info->Logfile_arg); /* free default string */
          args_info->Logfile_arg = gengetopt_strdup (optarg);
          break;


        case 0:	/* Long option with no short option */
          /* Set External globalRate.  */
          if (strcmp (long_options[option_index].name, "inputRate") == 0)
          {
            if (args_info->inputRate_given)
              {
                fprintf (stderr, "%s: `--inputRate' option given more than once\n", CMDLINE_PARSER_PACKAGE);
                clear_args ();
                exit (EXIT_FAILURE);
              }
            args_info->inputRate_given = 1;
            args_info->inputRate_arg = (float)strtod (optarg, NULL);
            break;
          }
          
          /* Use 'day' model.  */
          else if (strcmp (long_options[option_index].name, "day") == 0)
          {
            if (args_info->day_given)
              {
                fprintf (stderr, "%s: `--day' option given more than once\n", CMDLINE_PARSER_PACKAGE);
                clear_args ();
                exit (EXIT_FAILURE);
              }
            args_info->day_given = 1; Model_group_counter += 1;
          
            break;
          }
          
          /* Use 'jtt' model (default).  */
          else if (strcmp (long_options[option_index].name, "jtt") == 0)
          {
            if (args_info->jtt_given)
              {
                fprintf (stderr, "%s: `--jtt' option given more than once\n", CMDLINE_PARSER_PACKAGE);
                clear_args ();
                exit (EXIT_FAILURE);
              }
            args_info->jtt_given = 1; Model_group_counter += 1;
          
            break;
          }
          
          /* Use 'rev' model.  */
          else if (strcmp (long_options[option_index].name, "rev") == 0)
          {
            if (args_info->rev_given)
              {
                fprintf (stderr, "%s: `--rev' option given more than once\n", CMDLINE_PARSER_PACKAGE);
                clear_args ();
                exit (EXIT_FAILURE);
              }
            args_info->rev_given = 1; Model_group_counter += 1;
          
            break;
          }
          
          /* Use 'wag' model.  */
          else if (strcmp (long_options[option_index].name, "wag") == 0)
          {
            if (args_info->wag_given)
              {
                fprintf (stderr, "%s: `--wag' option given more than once\n", CMDLINE_PARSER_PACKAGE);
                clear_args ();
                exit (EXIT_FAILURE);
              }
            args_info->wag_given = 1; Model_group_counter += 1;
          
            break;
          }
          
          /* Use 'cprev' model.  */
          else if (strcmp (long_options[option_index].name, "cprev") == 0)
          {
            if (args_info->cprev_given)
              {
                fprintf (stderr, "%s: `--cprev' option given more than once\n", CMDLINE_PARSER_PACKAGE);
                clear_args ();
                exit (EXIT_FAILURE);
              }
            args_info->cprev_given = 1; Model_group_counter += 1;
          
            break;
          }
          
          /* Use nucleic acid JC model.  */
          else if (strcmp (long_options[option_index].name, "nucjc") == 0)
          {
            if (args_info->nucjc_given)
              {
                fprintf (stderr, "%s: `--nucjc' option given more than once\n", CMDLINE_PARSER_PACKAGE);
                clear_args ();
                exit (EXIT_FAILURE);
              }
            args_info->nucjc_given = 1; Model_group_counter += 1;
          
            break;
          }
          
          /* Use amino acid JC model.  */
          else if (strcmp (long_options[option_index].name, "aaJC") == 0)
          {
            if (args_info->aaJC_given)
              {
                fprintf (stderr, "%s: `--aaJC' option given more than once\n", CMDLINE_PARSER_PACKAGE);
                clear_args ();
                exit (EXIT_FAILURE);
              }
            args_info->aaJC_given = 1; Model_group_counter += 1;
          
            break;
          }
          
          /* Use 'k2p' model.  */
          else if (strcmp (long_options[option_index].name, "k2p") == 0)
          {
            if (args_info->k2p_given)
              {
                fprintf (stderr, "%s: `--k2p' option given more than once\n", CMDLINE_PARSER_PACKAGE);
                clear_args ();
                exit (EXIT_FAILURE);
              }
            args_info->k2p_given = 1; Model_group_counter += 1;
          
            break;
          }
          
          /* Use 'k2p' model.  */
          else if (strcmp (long_options[option_index].name, "hky") == 0)
          {
            if (args_info->hky_given)
              {
                fprintf (stderr, "%s: `--hky' option given more than once\n", CMDLINE_PARSER_PACKAGE);
                clear_args ();
                exit (EXIT_FAILURE);
              }
            args_info->hky_given = 1; Model_group_counter += 1;
          
            break;
          }
          
          /* Use user input file as model.  */
          else if (strcmp (long_options[option_index].name, "modelfile") == 0)
          {
            if (args_info->modelfile_given)
              {
                fprintf (stderr, "%s: `--modelfile' option given more than once\n", CMDLINE_PARSER_PACKAGE);
                clear_args ();
                exit (EXIT_FAILURE);
              }
            args_info->modelfile_given = 1;
            args_info->modelfile_arg = gengetopt_strdup (optarg);
            break;
          }
          

        case '?':	/* Invalid option.  */
          /* `getopt_long' already printed an error message.  */
          exit (EXIT_FAILURE);

        default:	/* bug: option not considered.  */
          fprintf (stderr, "%s: option unknown: %c\n", CMDLINE_PARSER_PACKAGE, c);
          abort ();
        } /* switch */
    } /* while */

  if ( Model_group_counter > 1)
    {
      fprintf (stderr, "%s: %d options of group Model were given. At most one is required\n", CMDLINE_PARSER_PACKAGE, Model_group_counter);
      missing_required_options = 1;
    }
  

  if (! args_info->tree_given)
    {
      fprintf (stderr, "%s: '--tree' ('-t') option required\n", CMDLINE_PARSER_PACKAGE);
      missing_required_options = 1;
    }
  if (! args_info->length_given)
    {
      fprintf (stderr, "%s: '--length' ('-n') option required\n", CMDLINE_PARSER_PACKAGE);
      missing_required_options = 1;
    }
  if ( missing_required_options )
    exit (EXIT_FAILURE);

  return 0;
}
