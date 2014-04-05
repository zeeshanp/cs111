// UCLA CS 111 Lab 1 command reading

#include "command.h"
#include "command-internals.h"
#include "alloc.h"

#include <error.h>

/* command stream */

struct command_tree;
{
	command_t head;
};

struct command_stream
{
	command_tree* command_list;
	int num;		
};

/** Stack definitions
 *  TODO: Test, garbage collection **/


struct command_stack
{
	command_t stack;    
	int top;
};

enum op_priority
{
	FOUR;
	THREE;
	TWO;
	ONE;
}

struct op
{
	enum op_priority priority;
	char* op[3];
};

struct op_stack
{
	struct op* stack;	
	int top;
};


void initStack(struct op_stack* op_stack)
{
	op_stack->stack = NULL;
	op_stack->top = 0;
}

void initStack(struct command_stack* cmd_stack)
{
	cmd_stack->stack = NULL;
	cmd_stack->top = 0;
}

char* pop(struct op_stack* op_stack)
{
	char* op[3];
	strcpy(op,op_stack->stack[top--].op);
	return op;
}

command_t pop(struct command_stack* cmd_stack)
{
	return cmd_stack->stack[top--];
}

//pretty sure i did this wrong lol
void push(struct op_stack* op_stack, struct op op)
{
	op_stack->top++;
	op_stack->stack = (struct op*)checked_realloc( top * sizeof(struct op) );
	strcpy(op_stack->stack[op_stack->top]->op,op);
}

//probably wrong as well
void push(struct command_stack* cmd_stack, struct command cmd)
{
	cmd_stack->top++;
	cmd_stack->stack = (command_t)checked_realloc( top * sizeof(struct command));
	cmd_stack->stack[top] = cmd;
}



/* Auxillary Functions */

int isValidChar(int c)
{
	return ( (c >= '0' && c <= '9') || ((c | 'a' - 'A') - 'a' < 26u) 
				|| c == '!' || c == '@' || c == '%' 
				|| c == '^' || c == '-' || c == '_'
				|| c == '+' || c == ':' || c == ',' 
				|| c == '.' || c == '/');
}


/*  Tokenizer stuff */


enum token_type
{
	SIMPLE;
	PIPE;
	SEMI_COLON;
	NEWLINE;
	SUBSHELL;
	INVALID;
	END_OF_FILE;
	WHITESPACE;
	AND;
	OR;
	LESS_THAN;
	GREATER_THAN;
	OPEN_PARA;
	CLOSE_PARA;
	BAD;
	
}

struct token
{
	char data*;
	enum token_type type;
	struct token* next;
	struct token* prev;
};

enum token_type get_type(char* data);

struct token* get_next_token(int* (get_next_byte) (void*), void* arg);



command_stream_t
make_command_stream (int (*get_next_byte) (void *),
		     void *get_next_byte_argument)
{
    
	

	command_stream_t stream = (command_stream_t)checked_malloc(sizeof(struct command_stream)); 
	
	struct op_stack op_stack;
	struct command_stack cmd_stack;
	initStack(


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
