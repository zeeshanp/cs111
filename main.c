// UCLA CS 111 Lab 1 main program

#include <errno.h>
#include <error.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>

#include "command.h"

static char const *program_name;
static char const *script_name;

static void
usage (void)
{
  error (1, 0, "usage: %s [-ptN] [NTHREADS] SCRIPT-FILE", program_name);
}

static int
get_next_byte (void *stream)
{
  return getc (stream);
}


int
main (int argc, char **argv)
{
  int command_number = 1;
  bool print_tree = false;
  bool time_travel = false;
  program_name = argv[0];
  int N;

  for (;;)
    switch (getopt (argc, argv, "ptN:"))
      {
      case 'p': print_tree = true; N = -1; break;
      case 't': time_travel = true; N = -1; break;
	  case 'N': N = atoi(optarg); break;
      default: usage (); break;
	  case '?':
			if (optopt == 'N')
				error(1,errno, "Option -N requires an argument.\n");
      case -1: goto options_exhausted;
      }
 options_exhausted:;

  // There must be exactly one file argument.
  if (optind != argc - 1)
    usage ();

  script_name = argv[optind];
  FILE *script_stream = fopen (script_name, "r");
  if (! script_stream)
    error (1, errno, "%s: cannot open", script_name);
  command_stream_t command_stream =
    make_command_stream (get_next_byte, script_stream);

  
  //initialize semaphore
  if (N != -1)
  {
	  if ( (key = ftok("/dev/null", 'a')) == -1 )
	  {
		 error(1,errno, "Error creating key");
	  }
  
	  if ( (semid = semget(key, 1, 0666 | IPC_CREAT)) == -1)
	  {
		 error(1,errno, "Failure creating semaphore array");
	  }

	  union semun s;
	  s.val = N;

	  if (semctl(semid, 0, SETVAL, s) == -1)
	  {
		  error(1,errno, "Failure initializing semaphore");
	  }
  }



  command_t command;
	if (time_travel)
	{
		execute_parallel(command_stream, N);
	}
	else
	{
		 while ((command = read_command_stream (command_stream)))
   		 {
     		 if (print_tree)
			 {
				printf ("# %d\n", command_number++);
	  			print_command (command);
			 }
     		 else
			 {

	  			execute_command (command, time_travel);
			 }
    	}
	}

  //destroy semaphore
	if (N != -1)
		if ( semctl(semid, 0, IPC_RMID) == -1)
		{
			error(1, errno, "Error destroying semaphore");
		}  

  //return print_tree || !last_command ? 0 : command_status (last_command);
  return 0;
}
