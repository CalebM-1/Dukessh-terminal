#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

static void usage (void);

mode_t
parse_perms (const char *str, mode_t read, mode_t write, mode_t exec)
{
  mode_t mode = 0;
  if (str[0] == 'r')
    mode |= read;
  if (str[1] == 'w')
    mode |= write;
  if (str[2] == 'x')
    mode |= exec;
  return mode;
}

int
main (int argc, char **argv)
{
  if (argc < 3)
    {
      usage ();
      return EXIT_FAILURE;
    }

  char *filename;
  mode_t mode;
  if (argc < 5)
    {
      usage ();
      return 1;
    }
  mode = 0;
  mode |= parse_perms (argv[1], S_IRUSR, S_IWUSR, S_IXUSR);
  mode |= parse_perms (argv[2], S_IRGRP, S_IWGRP, S_IXGRP);
  mode |= parse_perms (argv[3], S_IROTH, S_IWOTH, S_IXOTH);
  filename = argv[4];

  chmod (filename, mode);
  return 0;
}

static void
usage (void)
{
  printf ("chmod, changes permissions on a file\n");
  printf ("usage: chmod USR GRP OTH FILE\n\n");
  printf ("USR, GRP, and OTH must be of the rwx format,\n");
  printf ("with - indicating a permission is not allwed.\n");
}
