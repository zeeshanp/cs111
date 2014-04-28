// UCLA CS 111 Lab 1 command execution
#include <error.h>
#include <unistd.h>
#include <error.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>

#include "alloc.h"
#include "command.h"
#include "command-internals.h"

/* executing commands */
void executingSimple(command_t c);
void executingSubshell(command_t c);
void executingAnd(command_t c);
void executingOr(command_t c);
void executingSequence(command_t c);
void executingPipe(command_t c);
int execute_switch(command_t c);

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
		if (strcmp(c->u.word[0], "exec") != 0) 	
    		execvp(c->u.word[0], c->u.word);
    	else
    		execvp(c->u.word[1],c->u.word+1);
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
int command_status (command_t c)
{
	return c->status;
}


/* List Implementation (for parallel execution)*/
typedef struct list
{
	void** data;
	size_t count;
	size_t alloc_len;
} *list_t;
list_t list_init()
{
	list_t l = (list_t) checked_malloc(sizeof(struct list));
	l->count = 0;
	l->alloc_len = 16;
	l->data = (void**)checked_malloc(l->alloc_len * sizeof(void*));
	return l;
}
void list_free(list_t l)
{
	//need to free each individual element? what about graph_node_free()
	free(l->data);
}
void list_push(list_t l, void* elem)
{
	if (l->count == l->alloc_len)
		l->data = (void**)checked_grow_alloc(l->data, &(l->alloc_len));
	l->data[(l->count)++] = elem;
}
void* list_pop(list_t l)
{
	if (l->count == 0)
		return NULL;
	return l->data[--(l->count)];
}
void* list_peek(list_t l)
{
	if (l->count == 0)
		return NULL;
	return l->data[l->count - 1];
}
int isEmpty(list_t l)
{
	if (l->count == 0)
		return 1;
	else
		return 0;
}
void* list_elem(list_t src, int i)
{
	return src->data[i];
}

void appendList(list_t dest, list_t src)
{
	size_t i;
	for (i = 0; i < src->count; i++)
	{
		list_push(dest, src->data[i]);
	}
}


/* GRAPHNODE: Holds command, all other nodes which it depends on*/
typedef struct graph_node
{
	command_t cmd;
	list_t before;    //list of graph nodes that current node depends on
 	list_t readlist;     //list of any input in the command. can be input or word[1..n]
	list_t writelist;    //list of any output in the command. can only be output
	pid_t pid;
} *graph_node_t;

void construct_lists(graph_node_t g);

graph_node_t construct_graph_node(command_t cmd)
{
	graph_node_t g = (graph_node_t) checked_malloc(sizeof(struct graph_node));
	g->pid = -1;
	g->cmd = cmd;
	g->readlist = list_init();
	g->writelist = list_init();
	g->before = list_init();
	construct_lists(g);
	return g;
}

void graph_node_free(graph_node_t g)
{
	list_free(g->before);
	list_free(g->readlist);
	list_free(g->writelist);
}
 
void construct_lists(graph_node_t g)
{
	command_t cmd = g->cmd;

	
	if (cmd->type == SIMPLE_COMMAND)
	{
		int i = 1;
		while ( (cmd->u.word)[i] != NULL)
			list_push(g->readlist,(cmd->u.word)[i++]);

		if (cmd->output != NULL)
			list_push(g->writelist, cmd->output);
		if (cmd->input != NULL)
			list_push(g->readlist, cmd->input);
	}
	else if (cmd->type == SUBSHELL_COMMAND)
	{

		command_t sub_cmd = cmd->u.subshell_command;
		graph_node_t sub_grph = construct_graph_node(sub_cmd);
		//transfer sub_graph lists into normal ones.
		appendList(g->writelist, sub_grph->writelist);
		appendList(g->readlist, sub_grph->readlist);

		if (cmd->output != NULL)
			list_push(g->writelist, cmd->output);
		if (cmd->input != NULL)
			list_push(g->readlist, cmd->input);
	}
	else
	{
		//handle both sides
		command_t left = cmd->u.command[0];
		command_t right = cmd->u.command[1];
		graph_node_t sub_left = construct_graph_node(left);
		appendList(g->writelist, sub_left->writelist);
		appendList(g->readlist, sub_left->readlist);
		graph_node_t sub_right = construct_graph_node(right);
		appendList(g->writelist, sub_right->writelist);
		appendList(g->readlist, sub_right->readlist);
	} 

}

void construct_dependencies(graph_node_t g, list_t graph_nodes)
{
	size_t q,i,ii,j,jj,k,kk;
	bool detect = false;

	for (q = 0; q < graph_nodes->count; q++) //iterate thru each node
	{
		//RAW
		graph_node_t cur = graph_nodes->data[q];
		for (i = 0; i < cur->writelist->count; i++) //iterate thru writelist
		{
			for (ii = 0; ii < g->readlist->count; ii++)
			{
				if (strcmp(cur->readlist->data[i], g->readlist->data[ii]) != 0)
				{
					detect = true;
					goto add;
				}
			}

		}

		//WAR
		for (j = 0; j < cur->readlist->count; j++)
		{
			for (jj = 0; jj < g->writelist->count; jj++)
			{
				if (strcmp(cur->readlist->data[j], g->writelist->data[jj]) != 0)
				{
					detect = true;
					goto add;
				}
			}
		}

		//WAW
		for (k = 0; k < cur->writelist->count; k++)
		{
			for (kk = 0; kk < g->writelist->count; kk++)
			{
				if (strcmp(cur->writelist->data[j], g->writelist->data[kk]) != 0)
				{
					detect = true;
					goto add;
				}
			}
		}

		add:
		if (detect)
			list_push(g->before, graph_nodes->data[q]);
	}

}


void execute_parallel(command_stream_t cs)
{

	list_t no_dependencies = list_init();
	list_t dependencies = list_init();
	list_t graph_nodes = list_init();

	command_t command;

	while ((command = read_command_stream(cs)) != 0)
	{
		graph_node_t g = construct_graph_node(command);   //make readlist, writelist.
		construct_dependencies(g, graph_nodes);  // check read/write for others in graph_nodes and create before** array
		list_push(graph_nodes, g);
	}

	
	//debugging statements

		
	while(!isEmpty(graph_nodes))
	{
		graph_node_t g = list_pop(graph_nodes);
		if (isEmpty(g->before))
			list_push(no_dependencies, g);
		else
			list_push(dependencies, g);
	}

	printf("dep: %d\n no dep: %d\n", dependencies->count,no_dependencies->count);
	
	size_t i;
	for (i = 0; i < no_dependencies->count; i++)
	{
		graph_node_t g = list_elem(no_dependencies, i);
		pid_t pid = fork();
		if (pid < 0)
			error(1,errno, "Error Forking");
		else if (pid == 0)
		{
			int status = execute_switch(g->cmd);
			_exit(status);
		}
		else if (pid > 0)
			g->pid = pid;
	}
	/*
	//execute dependencies
	size_t j;
	for (j = 0; j < dependencies->count; j++)
	{
		graph_node_t g = list_elem(dependencies, j);
		label:;
			size_t k;
			for(k = 0; k < g->before->count; k++)
			{
				graph_node_t gg = list_elem(g->before, k);
				if (gg->pid == -1)
					goto label;
			}
		int status;
		size_t x;
		for (x = 0; x < g->before->count; x++)
		{
			graph_node_t gg = list_elem(g->before, x);
			waitpid(gg->pid, status, 0);
		}		
		pid_t pid = fork();
		if (pid == 0)
			execute_command(g->cmd, false);
		else if (pid > 0)
			g->pid = pid;
		else if (pid < 0)
			error(1,errno, "Error forking");
	}*/
				
}



void	
execute_command (command_t c, bool time_travel)
{
  	if (!time_travel)
	    execute_switch(c);
	//else
	//	execute_parallel(c);


}
