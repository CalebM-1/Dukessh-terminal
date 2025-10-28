#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static void usage (void);

int
main (int argc, char *argv[], char *envp[])
{
  usage ();

  // Count existing environment variables
  int env_count = 0;
  while (envp[env_count] != NULL)
    {
      env_count++;
    }

  // Count inline NAME=VALUE arguments before program path
  int vals = 0;
  for (int i = 1; i < argc; i++)
    {
      if (strchr (argv[i], '/'))
        { // First argument with '/' is the program
          break;
        }
      vals++;
    }

  // Allocate new environment array
  char **env = malloc ((env_count + vals + 1) * sizeof (char *));
  if (!env)
    {
      perror ("malloc");
      return EXIT_FAILURE;
    }

  int index = 0;
  // Copy existing environment variables
  for (int i = 0; i < env_count; i++)
    {
      env[index++] = envp[i];
    }

  // Copy inline NAME=VALUE arguments (like A=5 B=6)
  for (int i = 1; i <= vals; i++)
    {
      env[index++] = argv[i];
    }
  env[index] = NULL; // NULL-terminate for execve

  // Slice program arguments
  int prog_argc = argc - (1 + vals);
  char **prog_argv = malloc ((prog_argc + 1) * sizeof (char *));
  if (!prog_argv)
    {
      perror ("malloc");
      free (env);
      return EXIT_FAILURE;
    }

  for (int i = 0; i <= prog_argc || argv[i + 1 + vals] != NULL; i++)
    {
      prog_argv[i] = argv[i + 1 + vals];
    }
  prog_argv[prog_argc] = NULL; // NULL-terminate

  // Debug prints

  // printf("Environment:\n");
  // for (int i = 0; env[i] != NULL; i++) {
  //     printf("  %s\n", env[i]);
  // }
  // printf("Program args:\n");
  // for (int i = 0; argv[i] != NULL; i++) {
  //     printf("  %s\n", argv[i]);
  // }
  // fflush(stdout);

  // Execute the program
  execve (prog_argv[0], prog_argv, env);

  // If execve fails
  perror ("execve failed");
  free (env);
  free (prog_argv);
  return EXIT_FAILURE;
}

static void
usage (void)
{
  printf ("env, set environment variables and execute program\n");
  printf ("usage: env [NAME=VALUE ...] PROG ARGS\n");
}
