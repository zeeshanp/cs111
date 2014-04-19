// UCLA CS 111 Lab 1 command execution

#include <error.h>
#include <unistd.h>
#include <error.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "command.h"
#include "command-internals.h"

//function declarations
void executingSimple(command_t c);
void executingSubshell(command_t c);
void executingAnd(command_t c);
void executingOr(command_t c);
void executingSequence(command_t c);
void executingPipe(command_t c);
int execute_switch(command_t c);


int command_status (command_t c)
{
	return c->status;
}

void executingSubshell(command_t c)
{
	int status;
	int child = fork();
	if (child == 0)
	{
		int fp_in,fp_out;
		if (c->input != NULL)
		{
			if ( (fp_in = open(c->input,O_RDONLY)) < 0)
				error(1, errno, "Unable to open %s.", c->input);
			if (  dup2(fp_in,0) < 0 )
				error(1, errno, "Error with dup2 on %s.", c->input);
		}
		if (c->output != NULL)
    	{
    		if ( (fp_out = open(c->output, O_RDWR|O_TRUNC|O_CREAT,0644)) < 0 )
    			error(1, errno, "Unable to write to %s.", c->output);
    		if ( dup2(fp_out,1) < 0 )
    			error(1, errno, "Error with dup2 on %s.", c->output);
    	}
    	
    	execute_switch(c->u.subshell_command);
    	
    	if (c->input != NULL)
    		close(fp_in);
    	if (c->output != NULL)
    		close(fp_out);

    	_exit(status);
	}
	else
	{
		//back in parent process.
		waitpid(child, &status, 0);
		c->status = WEXITSTATUS(status);
	}


}

void executingSequence(command_t c)
{
	execute_switch(c->u.command[0]);
	c->status = execute_switch(c->u.command[1]);
}

void executingSimple(command_t c)
{
	int status;
    int child = fork();
    if (child == 0)
    {
    	int fp_in, fp_out;
    	if (c->input != NULL)
    	{
    		
    		if ( (fp_in = open(c->input, O_RDONLY)) < 0 )
    			error(1, errno, "Unable to open %s.", c->input);
    		if ( dup2(fp_in,0) < 0 )
    			error(1, errno, "Error with dup2 on %s.", c->input);
    	}
    	if (c->output != NULL)
    	{
    		if ( (fp_out = open(c->output, O_RDWR|O_TRUNC|O_CREAT,0644)) < 0 )
    			error(1, errno, "Unable to write to %s.", c->output);
    		if ( dup2(fp_out,1) < 0 )
    			error(1, errno, "Error with dup2 on %s.", c->output);
    	}
    	execvp(c->u.word[0], c->u.word);
    	error(1, errno, "Could not execute: %s.", c->u.word[0]);
    }
    else
    {
    	//parent process.
    	waitpid(child,&status,0);    	
    }
    c->status = WEXITSTATUS(status);
}


void executingOr(command_t c)
{
	//execute the left command.
	int status = execute_switch(c->u.command[0]);

	//if failed
	if (status != 0)  
	{
		c->status = execute_switch(c->u.command[1]);
	}
	else
		c->status = status;
}


void executingAnd(command_t c)
{
	int status = execute_switch(c->u.command[0]);
	
	if (!status) //success;
		c->status = execute_switch(c->u.command[1]);
	else
		c->status = status;	
}


int execute_switch(command_t c)
{
	switch(c->type)
	{
	case SIMPLE_COMMAND:
		executingSimple(c);
		break;
	case SUBSHELL_COMMAND:
		executingSubshell(c);
		break;
	case AND_COMMAND:
		executingAnd(c);
		break;
	case OR_COMMAND:
		executingOr(c);
		break;
	case SEQUENCE_COMMAND:
		executingSequence(c);
		break;
	case PIPE_COMMAND:
		executingPipe(c);
		break;
	default:
		error(1, 0, "Not a valid command");
	}
	return c->status;
}

void executingPipe(command_t c)
{
	pid_t returnedPid;
	pid_t firstPid;
	pid_t secondPid;
	int buffer[2];
	int eStatus;

	if ( pipe(buffer) < 0 )  //create pipe
	{
		error (1, errno, "pipe was not created");
	}

	firstPid = fork();
	if (firstPid < 0)
    {
		error(1, errno, "fork was unsuccessful");
    }
	else if (firstPid == 0) //child executes command on the right of the pipe
	{
		close(buffer[1]); //close unused write end, the right side is gonna read only.

        //redirect standard input to the read end of the pipe
        //so that input of the command (on the right of the pipe)
        //comes from the pipe
		if ( dup2(buffer[0], 0) < 0 )
		{
			error(1, errno, "error with dup2");
		}
		execute_switch(c->u.command[1]);
		_exit(c->u.command[1]->status);
	}
	else 
	{
		// Parent process
		secondPid = fork(); //fork another child process
                            //have that child process executes command on the left of the pipe
		if (secondPid < 0)
		{
			error(1, 0, "fork was unsuccessful");
		}
        else if (secondPid == 0)
		{
			close(buffer[0]); //close unused read end
			if(dup2(buffer[1], 1) < 0) //redirect standard output to write end of the pipe
            {
				error (1, errno, "error with dup2");
            }
			execute_switch(c->u.command[0]);
			_exit(c->u.command[0]->status);
		}
		else
		{
			// Finishing processes
			returnedPid = waitpid(-1, &eStatus, 0); //this is equivalent to wait(&eStatus);
                        //we now have 2 children. This waitpid will suspend the execution of
                        //the calling process until one of its children terminates
                        //(the other may not terminate yet)

			//Close pipe
			close(buffer[0]);
			close(buffer[1]);

			if (secondPid == returnedPid )
			{
			    //wait for the remaining child process to terminate
				waitpid(firstPid, &eStatus, 0); 
				c->status = WEXITSTATUS(eStatus);
				return;
			}
			
			if (firstPid == returnedPid)
			{
			    //wait for the remaining child process to terminate
   				waitpid(secondPid, &eStatus, 0);
				c->status = WEXITSTATUS(eStatus);
				return;
			}
		}
	}	
}


void
execute_command (command_t c, bool time_travel)
{
  	if (!time_travel)
	    execute_switch(c);
}
