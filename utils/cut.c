#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// You may assume that lines are no longer than 1024 bytes
#define LINELEN 1024

static void usage (void);

int
main (int argc, char *argv[])
{
  // set a default delimiter and position
  char *d = " ";
  int f = 1;

  if (!argv)
    {
      return false;
    }

  opterr = 0;
  // provide he proper options
  char *optionStr = "d:f:";
  char opt;

  // get each argument
  while ((opt = getopt (argc, argv, optionStr)) != -1)
    {
      switch (opt)
        {
        case 'd':
          d = optarg;
          break;
        case 'f':
          {
            // if f is found, change the value of f set above
            char *endptr;
            long val = strtol (optarg, &endptr, 10);

            if (*endptr != '\0' || val <= 0)
              {
                usage ();
                return EXIT_FAILURE;
              }
            f = (int)val;
            if (f <= 0)
              {
                usage ();
                return EXIT_FAILURE;
              }

            break;
          }
        default:
          // check for invalid args
          printf ("./bin/cut: invalid option -- \'%c\'\n", optopt);
          return false;
        }
    }

  // get the file from the end of argv
  FILE *file = NULL;
  if (argv[optind])
    {
      file = fopen (argv[optind], "r");
      if (!file)
        {
          usage ();
          return EXIT_FAILURE;
        }
    }
  else
    {
      // use stdin if no file can be found (piping)
      file = stdin;
    }

  char buffer[LINELEN];

  // get every line of the given file
  while (fgets (buffer, sizeof (buffer), file) != NULL)
    {
      // check for empty lines
      size_t len = strlen (buffer);
      if (len > 0 && buffer[len - 1] == '\n')
        {
          buffer[len - 1] = '\0';
        }

      // tokenize each until you reach the desired position
      char *token = strtok (buffer, d);
      for (int i = 1; i < f; i++)
        {
          token = strtok (NULL, d);
        }

      // if that token is not NULL, and exsists, print it
      if (token && token != NULL)
        {
          printf ("%s\n", token);
        }
      else
        {
          // print a newline otherwise
          printf ("\n");
        }
    }

  return EXIT_SUCCESS;
}

// usage
static void usage (void) __attribute__ ((unused));
static void
usage (void)
{
  printf ("cut, splits each line based on a delimiter\n");
  printf ("usage: cut [FLAG] FILE\n");
  printf ("FLAG can be:\n");
  printf (
      "  -d C     split each line based on the character C (default ' ')\n");
  printf ("  -f N     print the Nth field (1 is first, default 1)\n");
  printf ("If no FILE specified, read from STDIN\n");
}
