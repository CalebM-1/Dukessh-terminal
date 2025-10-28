#include <fcntl.h>
#include <spawn.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include "builtins.h"
#include "hash.h"

// The contents of this file are up to you, but they should be related to
// running separate processes. It is recommended that you have functions
// for:
//   - performing a $PATH lookup
//   - determining if a command is a built-in or executable
//   - running a single command in a second process
//   - running a pair of commands that are connected with a pipe

// You should provide some function like this to serve as the interface to
// parsing the command line. This function is just a placeholder and you
// should define your own.

int process_num = 1;

bool
run_builtin (char *str, char *cmd[])
{
  // check for all possible builtin functions
  if (strcmp (cmd[0], "quit") == 0)
    {
      printf ("\n");
      // just exits the shell
      exit (0);
    }
  if (strcmp (cmd[0], "echo") == 0)
    {
      // runs the echo function form builtins
      echo (str);
      return true;
    }
  if (strcmp (cmd[0], "pwd") == 0)
    {
      // runs pwd from bultins
      pwd ();
      return true;
    }
  if (strcmp (cmd[0], "cd") == 0)
    {
      // runs chdir from builtins
      chdir (cmd[1]);
      return true;
    }
  if (strcmp (cmd[0], "which") == 0)
    {
      // runs which from builtins
      which (cmd[1]);
      return true;
    }
  if (strcmp (cmd[0], "export") == 0)
    {
      // runs export from builtins
      export (cmd[1]);
      return true;
    }
  if (strcmp (cmd[0], "unset") == 0)
    {
      // runs unset from builtins
      unset (cmd[1]);
      return true;
    }
  return false;
}

// Builds a new enviroment array when spawning child.
// It takes all key-value pairs and puts them in a KEY=VALUE string

// Returns a set of string
static char **
build_env (void)
{

  // Get a list of all keys from hash table
  char **keys = hash_keys ();
  int num_keys = 0;

  // Count number of keys that are in the array
  while (keys[num_keys] != NULL)
    num_keys++;

  // Allocate space for all key=value paris
  char **env = malloc ((num_keys + 2) * sizeof (char *));
  int index = 0;

  // For each key in hash table, build string
  for (int i = 1; i < num_keys + 1; i++)
    {
      char *key = keys[i - 1];
      char *value = hash_find (key);
      size_t len = strlen (key) + strlen (value) + 2;

      // Allocate and format string
      char *kv = malloc (len);
      snprintf (kv, len, "%s=%s", key, value);

      // Store it in the enviroment array
      env[index++] = kv;
    }

  free (keys);

  // Add PATH to the enviroment
  char *path = getenv ("PATH");
  char *path_equals = "PATH=";
  size_t total_length = strlen (path) + strlen (path_equals) + 1;

  // Allocate and build PATH=value
  char *result = calloc (total_length, sizeof (char));
  strncpy (result, path_equals, total_length * sizeof (char));
  strncat (result, path, sizeof (result) - strlen ((result)-1));
  env[index++] = result;

  // NULL terminate array
  env[index] = NULL;
  return env;
}

// Resolves the path of a command

// Returns string with the full path, or NULL of not found
char *
resolve_path (const char *cmd)
{
  // If command contains '/' assume explicit path
  if (strchr (cmd, '/'))
    return strdup (cmd);

  // Gets PATH enviroment variable
  char *path_env = getenv ("PATH");
  if (!path_env)
    return NULL;

  // Make a copy of PATH
  char *paths = strdup (path_env);
  char *token = strtok (paths, ":");

  // Check ech directory in PATH
  while (token)
    {
      char fullpath[1024];
      snprintf (fullpath, sizeof (fullpath), "%s/%s", token, cmd);

      // Return full path if file exists
      if (access (fullpath, X_OK) == 0)
        {
          free (paths);
          return strdup (fullpath);
        }
      // Move to next PATH
      token = strtok (NULL, ":");
    }
  free (paths);
  return NULL;
}

int
run_process (char *str, char *cmd[], int fd[])
{
  static pid_t pending_writer = -1;

  if (!run_builtin (str, cmd))
    {
      if (fd[0] == -1)
        {
          pid_t pid = fork ();
          if (pid == -1)
            {
              return -1;
            }
          if (pid == 0)
            {
              char **env = build_env ();
              char *resolved = resolve_path (cmd[0]);
              execve (resolved, cmd, env);
              free (resolved);
              exit (1);
            }
          if (pid > 0)
            {
              int status;
              if (waitpid (pid, &status, 0) == -1)
                return -1;
              if (WIFEXITED (status))
                return WEXITSTATUS (status);
              return -1;
            }
        }
      else
        {
          if (process_num == 1)
            {
              process_num = 2;
              pid_t pid = fork ();
              if (pid == -1)
                {
                  return -1;
                }
              if (pid == 0)
                {
                  close (fd[0]);
                  dup2 (fd[1], STDOUT_FILENO);
                  close (fd[1]);

                  char **env = build_env ();
                  char *resolved = resolve_path (cmd[0]);
                  execve (resolved, cmd, env);
                  free (resolved);
                  exit (1);
                }
              else
                {
                  pending_writer = pid;
                  close (fd[1]);
                  return 0;
                }
            }
          else
            {
              process_num = 1;
              pid_t reader_pid = fork ();
              if (reader_pid == -1)
                {
                  return -1;
                }

              if (reader_pid == 0)
                {
                  close (fd[1]);
                  dup2 (fd[0], STDIN_FILENO);
                  close (fd[0]);

                  char **env = build_env ();
                  char *resolved = resolve_path (cmd[0]);
                  execve (resolved, cmd, env);

                  free (resolved);
                  exit (1);
                }
              else
                {
                  close (fd[0]);
                  fd[0] = fd[1] = -1;

                  int status;
                  int ret_code = 0;

                  waitpid (reader_pid, &status, 0);

                  if (pending_writer != -1)
                    {
                      waitpid (pending_writer, &status, 0);
                      pending_writer = -1;
                    }

                  return ret_code;
                }
            }
        }
    }
  return 0;
}
