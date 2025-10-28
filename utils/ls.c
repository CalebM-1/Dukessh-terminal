#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <linux/limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/stat.h>

static void usage (void);

// WARNING WARNING WARNING:
// When using opendir and readdir to read directory listings, the files
// are returned in the order they were created. There is randomness to that
// creation that can cause tests to fail. You MUST sort the file names as
// expected in the test case files. To avoid making your task harder than
// it needs to be, we strongly suggest you use scandir instead.

// Function made using ChatGPT prompt: "Create a c function that sorts
// filenames alphabetically, ignoring the leading ."
int
ignore_dotcmp (const struct dirent **a, const struct dirent **b)
{
  const char *nameA = (*a)->d_name;
  const char *nameB = (*b)->d_name;

  while (*nameA == '.')
    nameA++;
  while (*nameB == '.')
    nameB++;

  return strcasecmp (nameA, nameB);
}

// translates the mode of the string into the correct bitmask
static void
mode_to_string (mode_t mode, char *out)
{
  // checking if the file is a directory
  if (S_ISDIR (mode))
    out[0] = 'd';
  else
    out[0] = '-';

  // bit masks assigned to file
  const mode_t masks[] = { S_IRUSR, S_IWUSR, S_IXUSR, S_IRGRP, S_IWGRP,
                           S_IXGRP, S_IROTH, S_IWOTH, S_IXOTH };

  // cooresponding permission characters
  const char symbols[] = { 'r', 'w', 'x', 'r', 'w', 'x', 'r', 'w', 'x' };

  // create a string using the bitmask
  for (int i = 0; i < 9; i++)
    {
      if (mode & masks[i])
        out[i + 1] = symbols[i];
      else
        out[i + 1] = '-';
    }
  out[10] = '\0';
}

int
main (int argc, char *argv[])
{
  // possible arguments
  bool aOpt = false;
  bool pOpt = false;
  bool sOpt = false;

  opterr = 0;
  char *optionStr = "+aps";
  char opt;

  // retrieving the current arg in argv
  while ((opt = getopt (argc, argv, optionStr)) != -1)
    {
      switch (opt)
        {
        // setting the cooresponding bool to true if the arg is found
        case 'a':
          aOpt = true;
          break;
        case 'p':
          pOpt = true;
          break;
        case 's':
          sOpt = true;
          break;
        default:
          // error message
          printf ("./bin/ls: invalid option -- \'%c\'\n", optopt);
          return EXIT_FAILURE;
        }
    }

  // get the directory we are checking
  const char *dirpath = argv[optind];
  struct dirent **namelist;

  // get the list of files from the directory
  int n = scandir (dirpath, &namelist, NULL, ignore_dotcmp);
  if (n == -1)
    {
      // files not found
      return EXIT_FAILURE;
    }

  // iterate though the files found by scandir
  for (int i = 0; i < n; i++)
    {
      struct dirent *entry = namelist[i];

      // if the file is current or parent directory, ignore
      if (strcmp (entry->d_name, ".") == 0
          || strcmp (entry->d_name, "..") == 0)
        {
          free (entry);
          continue;
        }
      // check for the case of a hidden file
      if (!aOpt && entry->d_name[0] == '.')
        {
          free (entry);
          continue;
        }

      char path[PATH_MAX];

      // set a given path
      snprintf (path, sizeof (path), "%s/%s", dirpath, entry->d_name);

      struct stat st;
      // if inode cannot be found, free the entry
      if (stat (path, &st) == -1)
        {
          free (entry);
          continue;
        }

      if (sOpt && S_ISDIR (st.st_mode))
        {
          free (entry);
          continue;
        }

      // print the size of the files found if s is used
      if (sOpt)
        {
          printf ("%ld ", st.st_size);
        }

      // print the permissions for each file
      if (pOpt)
        {
          char perm[11];
          mode_to_string (st.st_mode, perm);
          printf ("%s ", perm);
        }

      // print the file name and free the entry from memory
      printf ("%s\n", entry->d_name);
      free (entry);
    }
  // free the allocated list of names
  free (namelist);

  return EXIT_SUCCESS;
}

// usage
static void usage (void) __attribute__ ((unused));
static void
usage (void)
{
  printf ("ls, list directory contents\n");
  printf ("usage: ls [FLAG ...] [DIR]\n");
  printf ("FLAG is one or more of:\n");
  printf ("  -a       list all files (even hidden ones)\n");
  printf ("  -p       list permission bitmask\n");
  printf ("  -s       list file sizes\n");
  printf ("If no DIR specified, list current directory contents.\n\n");
  printf ("Files must be sorted alphabetically, case insensitive.\n");
  printf ("Leading dots should be ignored when sorting.\n\n");
  printf ("With the -s flag, do not show entries for subdirectories.\n");
  printf ("Permission bitmasks are 10-character strings such as:\n");
  printf ("  -rwxr-x---\n\n");
  printf (
      "The first character is d for directories and - for regular files.\n\n");
  printf ("Do not show the \".\" or \"..\" directory entries.\n");
}
