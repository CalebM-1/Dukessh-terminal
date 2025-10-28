#include <assert.h>
#include <fcntl.h>
#include <spawn.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include "cmd.h"
#include "hash.h"
#include "process.h"

int fd[2] = { -1, -1 };

// Integrate the FSM command-line parser from lab 2 here. Note that the FSM
// effects will be vastly different from that of lab 2. Instead of implementing
// the effects here, this file should focus on the model and the parsing. You
// should modify the model's effects table to call functions in process.c or
// builtins.c as appropriate. (Or you can have a function here to call those
// functions indirectly.)

// You should provide some function like this to serve as the interface to
// parsing the command line. This function is just a placeholder and you
// should define your own.

typedef int state_t;
typedef int event_t;

// Needed for circular typedef. This lets action_t use fsm_t in its parameter
// list, while the struct fsm can use action_t as a field.
typedef struct fsm fsm_t;

// All entry, exit, and effect instances use the action type
typedef void (*action_t) (fsm_t *);

// Each FSM instance contains a current state
struct fsm
{
  state_t state; // current state

  // pointer to the FSM's transition function
  state_t (*transition) (struct fsm *, event_t, action_t *);

  // Additional data fields specific to this FSM
  char *command;       // the name of the command to run
  size_t nargs;        // the number of command-line arguments
  char **args;         // the command-line arguments
  char *current_token; // current token being processed
};

// Generic entry point for handling events
bool handle_event (fsm_t *, event_t);

// Additional definitions specific to an FSM for command line processing

#define MAX_ARGUMENTS 20

// Events
typedef enum
{
  TOKEN,   // normal command-line token
  PIPE,    // vertical bar character
  NEWLINE, // newline at the end of the command
  NIL      // invalid non-event
} cmdevt_t;
#define NUM_EVENTS NIL

// States
typedef enum
{
  Init,      // initial state
  Command,   // establishing the command name
  Arguments, // building the argument list
  Make_Pipe, // linking the commands together for a pipe
  Term,      // terminal state (execute program or error)
  NST        // invalid non-state
} cmdst_t;
#define NUM_STATES NST

// Helper functions
fsm_t *cmdline_init (void); // initialize the FSM
event_t lookup (char *);    // convert an event string to its numeric value

// Translate event/state numbers to their string equivalent
const char *event_name (event_t);
const char *state_name (state_t);

void
start_command (fsm_t *cmdmodel)
{
  // printf ("Starting new command: %s\n", cmdmodel->current_token);
  // TODO: Copy the current token to store it in the FSM's command
  // field. Next, create the FSM's args array (length MAX_ARGUMENTS)
  // set the current token as args[0], and initialize nargs to be
  // the number of arguments (1 at this point).
  cmdmodel->command = cmdmodel->current_token;
  cmdmodel->args = calloc (MAX_ARGUMENTS, sizeof (char *));
  cmdmodel->args[0] = cmdmodel->current_token;
  cmdmodel->nargs = 1;
}

/* Executed when processing a token after the command name. For instance,
   if the command line was "ls -l data NL", this function will be called
   when the current token is "-l" and again when it is "data". */
void
append (fsm_t *cmdmodel)
{
  if (cmdmodel->nargs >= MAX_ARGUMENTS)
    return;

  // printf ("Appending %s to the argument list\n", cmdmodel->current_token);
  assert (cmdmodel->args != NULL);

  // TODO: Store the current token into the args array and increment nargs
  cmdmodel->args[cmdmodel->nargs] = cmdmodel->current_token;
  cmdmodel->nargs++;
}

/* Executed when either a NL or | (pipe) is encountered. For instance, if
   the command line is "ls -l data NL", the current token will be "NL"; also,
   the FSM's args array should be complete, containing "ls", "-l", and "data",
   followed by several NULL pointers. */
void
execute (fsm_t *cmdmodel)
{
  assert (cmdmodel->args != NULL);

  // TODO: Print out the argument list similar to the format shown and free
  // the args array.
  char str[MAX_ARGUMENTS * 10] = "\0";
  for (int i = 0; i < cmdmodel->nargs; i++)
    {
      char temp[MAX_ARGUMENTS * 10];
      if (i == 0)
        {
          snprintf (temp, sizeof (temp), "%s", cmdmodel->args[i]);
        }
      else
        {
          snprintf (temp, sizeof (temp), " %s", cmdmodel->args[i]);
        }
      strncat (str, temp, sizeof (str) - strlen (str) - 1);
    }
  /*printf ("Execute %s with arguments { %s, (null) }\n", cmdmodel->args[0],
          str);*/
  int rc = run_process (str, cmdmodel->args, fd);
  char rc_str[20];
  snprintf (rc_str, 20, "%d", rc);
  hash_insert ("?", rc_str);
  // printf ("Return code: %d\n", rc);
  free (cmdmodel->args);
  cmdmodel->args = NULL;
}

// No changes are needed to the effects below

void
link_commands (fsm_t *cmdmodel)
{
  pipe (fd);
  execute (cmdmodel);
}

void
error_pipe (fsm_t *cmdmodel)
{
  printf ("ERROR: Received token %s while in state %s\n",
          cmdmodel->current_token, state_name (cmdmodel->state));
}

void
error_newline (fsm_t *cmdmodel)
{
  printf ("ERROR: Received token %s while in state %s\n",
          cmdmodel->current_token, state_name (cmdmodel->state));
}

static state_t const _transitions[NUM_STATES][NUM_EVENTS] = {
  // TOKEN PIPE NEWLINE
  { Command, Term, NST },         // Init
  { Arguments, Make_Pipe, Term }, // Command
  { Arguments, Make_Pipe, Term }, // Arguments
  { Command, Term, Term },        // Make_Pipe
  { NST, NST, NST }

};

// TODO: Create a table mapping states/events to the effect functions. If
// there is no valid transition, the entry here would be NULL because actions
// are function pointers.

static action_t const _effects[NUM_STATES][NUM_EVENTS] = {
  // TOKEN PIPE NEWLINE
  { start_command, error_pipe, NULL },         // Init
  { append, link_commands, execute },          // Command
  { append, link_commands, execute },          // Arguments
  { start_command, error_pipe, error_newline } // Make_Pipe

};

static state_t transition (fsm_t *fsm, event_t event, action_t *effect);

/* Create an instance of an FSM and initialize its fields as appropriate.
   Some fields are common to most FSMs (such as an initial state or a
   pointer to a transition function). Other fields will be specific to
   this fsm_t declaration. Return NULL if any part of the initialization
   fails. */
fsm_t *
cmdline_init (void)
{
  fsm_t *fsm = (fsm_t *)calloc (1, sizeof (fsm_t));
  fsm->state = Init;
  fsm->transition = transition;
  fsm->command = NULL;
  fsm->nargs = 0;
  fsm->args = NULL;
  fsm->current_token = NULL;
  return fsm;
}

// TODO: Create a transition function that is specific to this type
// of FSM. This function needs to take an fsm_t* and an event, returning
// both the new state and the effect to perform (the latter is returned
// using a call-by-reference parameter. This function should NOT contain
// any "if" types of statements based on the state or event; it should
// simply lookup these values in the tables defined above.
static state_t
transition (fsm_t *fsm, event_t event, action_t *effect)
{
  assert (fsm->state < NST);
  assert (event < NIL);
  *effect = _effects[fsm->state][event];
  state_t next = _transitions[fsm->state][event];
  return next;
}

/* Helper function for providing a printable string name for an event */
const char *
event_name (event_t evt)
{
  assert (evt <= NIL);

  // Event names for printing out
  const char *names[] = { "TOKEN", "PIPE", "NEWLINE", "NIL" };
  return names[evt];
}

/* Helper function for providing a printable string name for an state */
const char *
state_name (state_t st)
{
  assert (st <= NST);

  // State names for printing out
  const char *names[]
      = { "Init", "Command", "Arguments", "Make_Pipe", "Term", "NST" };
  return names[st];
}

/* Generic front-end for handling events. Should do nothing more
   than calling the FSM's transition function, performing an effect
   (if appropriate) and updating the state. Return false if the new
   state is the terminal state. */
bool
handle_event (fsm_t *fsm, event_t event)
{
  // TODO: Look up the current state/event combination in the
  // transition table. Print the following line for debugging
  // purposes just for this lab. This should be printed even
  // if there is no transition.
  //   printf ("[%s.%s -> %s]\n", ...);
  assert (fsm != NULL);
  action_t effect = NULL;
  state_t state = fsm->transition (fsm, event, &effect);
  /* printf ("[%s.%s -> %s]\n", state_name (fsm->state), event_name (event),
          state_name (state));*/
  assert (state != NST);
  fsm->state = state;

  // TODO: If the state/event combination is valid, execute
  // the transition and effect function (if there is one).
  // If the next state is Term (terminated), return false.
  // Otherwise return true

  if (effect != NULL)
    {
      effect (fsm);
    }
  if (state == Term)
    {
      return false;
    }
  return true;
}

/* Given a string, return the event type. Do not modify this function. */
event_t
lookup (char *token)
{
  if (!strcmp (token, "|"))
    return PIPE;

  if (!strcmp (token, "NL"))
    return NEWLINE;

  return TOKEN;
}

char *
parse_buffer (char *buffer)
{
  fsm_t *cmdmodel = cmdline_init ();
  if (cmdmodel == NULL)
    return NULL;

  // TODO: Change this to split the string into tokens, where
  // each token is an event that needs to be handled. After
  // looking up the event number, store the token in the FSM
  // and call handle_event().
  char *input = buffer;
  char *token = strtok (input, " ");
  while (token != NULL)
    {
      cmdmodel->current_token = token;
      event_t event = lookup (token);
      bool running = handle_event (cmdmodel, event);
      if (!running)
        break;
      token = strtok (NULL, " ");
    }
  execute (cmdmodel);

  // Free remaining allocated data
  if (cmdmodel->args != NULL)
    free (cmdmodel->args);
  free (cmdmodel);
  return input;
}
