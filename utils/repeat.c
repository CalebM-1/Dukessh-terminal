#include <stdio.h>
#include <stdlib.h>

#include "../src/hash.c"
static void usage (void);

int
main (int argc, char *argv[], char *envp[])
{
  if (argc < 3)
    {
      usage ();
    }

  for (int i = 1; i < argc; i += 2)
    {
      char *key = argv[i + 1];
      char *endptr;

      long repeat = strtol (argv[i], &endptr, 10);

      char *value = getenv (key);
      if (value == NULL)
        {
          value = "";
        }

      for (int t = 0; t < repeat; t++)
        {
          printf ("%s=%s\n", key, value);
        }
    }
  return EXIT_SUCCESS;
}

static void
usage (void)
{
  printf ("repeat, a tool for printing repeated environment variables\n");
  printf ("usage: repeat N VAR ...\n");
  printf ("each N must be a positive integer\n");
  printf ("N VAR can be repeated, but each repetition must have both\n");
}
