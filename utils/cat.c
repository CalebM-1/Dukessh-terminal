#include <stdio.h>
#include <stdlib.h>

static void usage (void);

int
main (int argc, char *argv[])
{
  // get the file (only argument)
  FILE *f = NULL;
  if (argv[1])
    {
      f = fopen (argv[1], "r");
      if (!f)
        {
          // if you cannot open the file, print the usage
          usage ();
          return EXIT_FAILURE;
        }
    }
  else
    {
      // if the file was not given, use stdin instead (piping)
      f = stdin;
    }

  // get and print every line of the file
  char buffer[1024];
  while (fgets (buffer, sizeof (buffer), f) != NULL)
    {
      printf ("%s", buffer);
    }
  return EXIT_SUCCESS;
}

// usage
static void
usage (void)
{
  printf ("cat, print the contents of a file\n");
  printf ("usage: cat FILE\n");
}
