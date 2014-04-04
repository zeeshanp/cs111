// UCLA CS 111 Lab 1 command reading

#include "command.h"
#include "command-internals.h"
#include "alloc.h"

#include <error.h>

typedef struct command_node* command_node_t;

struct command_node 
{
	command_t command;
	command_node_t next;
};


struct command_stream
{
	command_node_t head;	
};

command_stream_t
make_command_stream (int (*get_next_byte) (void *),
		     void *get_next_byte_argument)
{
    
	command_stream_t stream = (command_stream_t)checked_malloc(sizeof(struct command_stream)); 
	stream->head = 0;
	int c;
	while ((c = get_next_byte(get_next_byte_argument)) != -1)
		continue;

	return stream;
}


command_t
read_command_stream (command_stream_t s)
{
  s->head = 0;
  return 0;
}
