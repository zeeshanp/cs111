// UCLA CS 111 Lab 1 command reading

#include "command.h"
#include "command-internals.h"

#include <error.h>


typedef struct node 
{
	command_t command;
	node* next;
} node;


struct command_stream
{
	int num;
    node* head;	
}

command_stream_t
make_command_stream (int (*get_next_byte) (void *),
		     void *get_next_byte_argument)
{
    
    void* arg = get_next_byte_argument;
	command_stream_t stream;
	stream->num = 0;                                                                                           
	int c;
	while ( (c = get_next_byte(arg) != EOF )
	{
        //construct stream here.
	}

	return stream;
}


command_t
read_command_stream (command_stream_t s)
{
  /* FIXME: Replace this with your implementation too.  */
  error (1, 0, "command reading not yet implemented");
  return 0;
}
