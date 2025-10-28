#ifndef __cs361_process__
#define __cs361_process__

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
int run_process (char *str, char *cmd[], int fd[]);

#endif
