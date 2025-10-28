#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "hash.h"

int
echo_case (char *message)
{
  // check if the message is trying to check the return value
  if (message[6] == '?')
    {
      // find the return value in the global map
      char *value = hash_find ("?");
      printf ("%s\n", value);
      return 0;
    }

  // find the starting positions to search for '{' and '}'
  char *searchStartClose = message + 8;
  char *searchStartOpen = message + 6;
  char *found_close = strchr (searchStartClose, '}');
  char *found_open = strchr (searchStartOpen, '{');

  // if both brackets were found, we are checking a variable
  if (found_close != NULL && found_open != NULL)
    {
      // set pointers to the bracket characters
      char *start = strchr (message, '{');
      char *end = strchr (message, '}');
      size_t len = end - start - 1;
      // allocate a string that will be the variable name
      char *str = calloc (len + 1, sizeof (char));
      strncpy (str, start + 1, len);
      str[len] = '\0';
      // find the variable by name in the global hash map
      char *value = hash_find (str);
      free (str);
      // print the remainder of the message, switching the variable name with
      // its value
      int index = found_open - message;
      char before[32];
      strncpy (before, message, index);
      before[index - 1] = '\0';
      char result[32];
      strncpy (result, before + 5, 32);
      // if the value can be found from the hash map print the whole string
      if (value != NULL)
        {
          printf ("%s%s\n", result, value);
        }
      else
        {
          // otherwise, just print the string without the variable
          printf ("%s\n", result);
        }
      return 0;
    }
  else
    {
      // return failure
      return 1;
    }
}

int
echo_var (char *message)
{
  // make a copy of the message without the beginning "echo" portion
  char *copy = malloc (strlen (message + 5) + 1);
  strncpy (copy, message + 5, strlen (message + 5) + 1);

  char *src = copy;
  char *dst = copy;
  int in_space = 0;

  while (*src)
    {
      // turn areas with more than 1 space into 1 space
      if (*src == ' ')
        {
          if (!in_space)
            {
              *dst++ = *src;
              in_space = 1;
            }
        }
      else
        {
          // reset for next encounter of multiple spaces
          *dst++ = *src;
          in_space = 0;
        }
      src++;
    }
  // null terminate the string
  *dst = '\0';
  src = copy;
  dst = copy;
  while (*src)
    {
      // turn areas with a newline string into a literal '\n' character
      if (src[0] == '\\' && src[1] == 'n')
        {
          *dst++ = '\n';
          src += 2;
        }
      else
        {
          // otherwise, continue through the string
          *dst++ = *src++;
        }
    }
  // null terminate the string
  *dst = '\0';
  printf ("%s\n", copy);
  return 0;
}

// Given a message as input, print it to the screen followed by a
// newline ('\n'). If the message contains the two-byte escape sequence
// "\\n", print a newline '\n' instead. No other escape sequence is
// allowed. If the sequence contains a '$', it must be an environment
// variable or the return code variable ("$?"). Environment variable
// names must be wrapped in curly braces (e.g., ${PATH}).
//
// Returns 0 for success, 1 for errors (invalid escape sequence or no
// curly braces around environment variables).
int
echo (char *message)
{
  char *dollar = strchr (message, '$');
  // check for the '$' character in the string for variable or regular printing
  if (dollar != NULL)
    {
      // regular print case
      echo_case (message);
    }
  else
    {
      // variable case
      echo_var (message);
    }
  return 0;
}

// Given a key-value pair string (e.g., "alpha=beta"), insert the mapping
// into the global hash table (hash_insert ("alpha", "beta")).
//
// Returns 0 on success, 1 for an invalid pair string (kvpair is NULL or
// there is no '=' in the string).
//
// NOTE: For some strange reason, clang-format (used for checking the style)
// expects export to be initially formatted this way...

int export (char *kvpair)
{
  // if no arg was passed, terminate
  if (kvpair == NULL)
    {
      return 0;
    }
  // seperate the key and the value
  char *key = strtok (kvpair, "=");
  char *value = strtok (NULL, " ");
  if (key == NULL || value == NULL)
    {
      return 1;
    }
  // inset the values in the global hash map
  hash_insert (key, value);
  return 0;
}

// Prints the current working directory (see getcwd()). Returns 0.
int
pwd (void)
{
  // get the current directory from this c function
  char *buffer;
  buffer = getcwd (NULL, 0);
  printf ("%s\n", buffer);
  return 0;
}

// Removes a key-value pair from the global hash table.
// Returns 0 on success, 1 if the key does not exist.
int
unset (char *key)
{
  // check for invalid arg
  if (key == NULL)
    {
      return 1;
    }
  // remove the given key from the map
  hash_remove (key);
  return 0;
}

// Given a string of commands, find their location(s) in the $PATH global
// variable. If the string begins with "-a", print all locations, not just
// the first one.
//
// Returns 0 if at least one location is found, 1 if no commands were
// passed or no locations found.
int

which (char *cmdline)
{
  // check what the first argument is
  if (strcmp (cmdline, "cd") == 0 || strcmp (cmdline, "echo") == 0
      || strcmp (cmdline, "pwd") == 0 || strcmp (cmdline, "which") == 0
      || strcmp (cmdline, "export") == 0 || strcmp (cmdline, "unset") == 0
      || strcmp (cmdline, "quit") == 0)
    {
      // print this message if the given command is builtin
      printf ("%s: dukesh built-in command\n", cmdline);
      return 0;
    }

  // if the executable starts with "./" it must be a utility
  if (strncmp (cmdline, "./", 2) == 0)
    {
      if (access (cmdline, X_OK) == 0)
        {
          printf ("%s\n", cmdline);
          return 0;
        }
    }

  // get the current path from the given enviorment variables
  char *path = getenv ("PATH");
  char *p = strdup (path);
  char *dir = strtok (p, ":");

  while (dir != NULL)
    {
      char fullpath[1024];
      // print the full path of the executable location
      snprintf (fullpath, sizeof (fullpath), "%s/%s", dir, cmdline);
      if (access (fullpath, X_OK) == 0)
        {
          printf ("%s\n", fullpath);
          // free the allocated memory
          free (p);
          return 0;
        }
      dir = strtok (NULL, ":");
    }

  free (p);
  return 1;
}
