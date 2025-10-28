#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// You may assume that lines are no longer than 1024 bytes
#define LINELEN 1024

static void usage (void);

int
main (int argc, char *argv[])
{
  // set the default number of lines to print
  int n = 5;

  // check if argv exsists
  if (!argv)
    {
      return false;
    }

  opterr = 0;
  char *optionStr = "n:";
  char opt;

  // get each argument
  while ((opt = getopt (argc, argv, optionStr)) != -1)
    {
      switch (opt)
        {
        case 'n':
          {
            // if n is an arg, change the value of n
            char *endptr;
            long tmp = strtol (optarg, &endptr, 10);
            n = (int)tmp;
            break;
          }
        default:
          // print if invalid arg is found
          printf ("./bin/head: invalid option -- \'%c\'\n", optopt);
          return false;
        }
    }

  // get the file from the end of argv
  FILE *f = NULL;
  if (argv[optind])
    {
      f = fopen (argv[optind], "r");
      if (!f)
        {
          usage ();
          return EXIT_FAILURE;
        }
    }
  else
    {
      // use stdin if file was not provided (piping)
      f = fdopen (STDIN_FILENO, "r");
    }

  char buffer[LINELEN];
  int i = 0;

  // retrieve n lines from the file
  while (fgets (buffer, sizeof (buffer), f) != NULL && i < n)
    {
      // print the line
      printf ("%s", buffer);
      i++;
    }

  return EXIT_SUCCESS;
}

// usage
static void usage (void) __attribute__ ((unused));
static void
usage (void)
{
  printf ("head, prints the first few lines of a file\n");
  printf ("usage: head [FLAG] FILE\n");
  printf ("FLAG can be:\n");
  printf ("  -n N     show the first N lines (default 5)\n");
  printf ("If no FILE specified, read from STDIN\n");
}
