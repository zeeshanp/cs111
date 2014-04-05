// UCLA CS 111 Lab 1 command reading

#include "command.h"
#include "command-internals.h"
#include "alloc.h"

#include <error.h>

/* struct definitions */

struct command_tree;
{
	command_t head;
};

struct command_stream
{
	command_tree* command_list;
	int num;		
};

struct command_stack
{
	command_t stack;    
	int top;

};

struct op_stack
{
	enum command_type* stack;
	int top;
};

enum op_priority
{
	FOUR;
	THREE;
	TWO;
	ONE;
};


/* auxillary functions */


void initStack(struct op_stack)
{
	op_stack.stack = NULL;
	int top = 0;
}

void initStack(struct command_stack)
{
	command_stack.stack = NULL;
	int top = 0;
}

char* pop(struct op_stack)
{
	char* op[3];
	strcpy(op,op_stack.stack[top--]);
	return op;
}

command_t pop(struct command_stack)
{
	return command_stack.stack[top--];
}

//pretty sure i did this wrong lol
void push(struct op_stack, char* op)
{
	op_stack.stack = (struct command*)checked_realloc(sizeof(op_stack.stack)+sizeof(struct command*));
	top++;
	strcpy(op_stack.stack[top],op);
}

//will wait till i get the other one
void push(struct command_stack, command_t cmd)
{

}

int isValidChar(int c)
{
	return ( (c >= '0' && c <= '9') || ((c | 'a' - 'A') - 'a' < 26u) 
				|| c == '!' || c == '@' || c == '%' 
				|| c == '^' || c == '-' || c == '_'
				|| c == '+' || c == ':' || c == ',' 
				|| c == '.' || c == '/');
}



command_stream_t
make_command_stream (int (*get_next_byte) (void *),
		     void *get_next_byte_argument)
{
    
	//command_stream_t is an array tree's that represent each complete command

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
