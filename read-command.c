// UCLA CS 111 Lab 1 command reading
 
#include "command.h"
#include "command-internals.h"
#include "alloc.h"
 
#include <error.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stddef.h>
#include <ctype.h>
 
 
typedef struct command_node* command_node_t;
 
struct command_node
{
	command_t data;
	command_node_t next;
};
 
typedef struct command_stream* command_stream_t;

struct command_stream
{
	command_node_t head;			
};
 
/* Stack */ 
typedef struct stack* stack_t;
struct stack
{
    void** data;
    size_t count;
    size_t max_count;
};
 
static stack_t stack_init()
{
    stack_t s = (stack_t)checked_malloc(sizeof(struct stack));
    s->max_count = 16;
    s->count = 0;
    s->data = (void**)checked_malloc(s->max_count * sizeof(void*));
    return s;
}
static void stack_free(stack_t s)
{
    if (!s) error(1, 0, "Null stack point given to stack free");
    free(s->data);
    free(s);
}
static void stack_push(stack_t s, void* element)
{
    if (!s) error(1, 0, "Null stack point given to stack push");
    if (s->count == s->max_count)
        s->data = (void**)checked_grow_alloc(s->data, &s->max_count);
    s->data[s->count++] = element;
}
static void* stack_pop(stack_t s)
{
    if (!s) error(1, 0, "Null stack point given to stack pop");
    if (s->count == 0) error(1, 0, "Trying to pop from empty stack");
    return s->data[--s->count];
}
static void* stack_top(stack_t s)
{
    if (!s) error(1, 0, "Null stack point given to stack top");
    if (s->count == 0) return 0;
    return s->data[s->count - 1];
} 
static size_t stack_count(stack_t s)
{
    if (!s) error(1, 0, "Null stack point given to stack count");
    return s->count;
}
 
int isValidChar(int c)
{
	return ( isalpha(c) || isdigit(c)
			|| c == '!' || c == '@' || c == '%' || c == '^' || c == '-' || c == '_'
			|| c == '+' || c == ':' || c == ',' || c == '.' || c == '/');
}

char* readFile(int (*get_next_byte) (void *), void* get_next_byte_argument)
{
	int inputSize = 128;
	int count = 0;
	int c;
	char* stream = (char*)checked_malloc(inputSize*sizeof(char));
 
	while ( (c = get_next_byte(get_next_byte_argument)) != -1)
	{
		stream[count++] = c;
		if (count == inputSize)
		{
			inputSize += 128;
			stream = (char*)checked_realloc(stream, inputSize*sizeof(char));
		}
	}
	stream[count] = -1;
	return stream;
 
}

/*  Tokenizer stuff */
enum token_type
{
	PIPE,
	SEMI_COLON,
	NEWLINE,
	AND,
	OR,
	LESS_THAN,
	GREATER_THAN,
	OPEN_PARA,
	CLOSE_PARA,
	WORD, 
};
 
typedef struct token* token_t;
 
struct token
{
	char* data;
	enum token_type type;
	token_t next;
	token_t prev;
};
 
token_t allocate_token(token_t prev, int dataSize, char* data, enum token_type type)
{
	token_t t = checked_malloc(sizeof(struct token));
	prev->next = t;
	t->prev = prev;
	t->data =(char*)checked_malloc(dataSize*sizeof(char));
	strcpy(t->data,data);
	t->type = type;
	t->next = 0;
	return t;
}
 
void reportError(int linenum)
{
	fprintf(stderr, "Error Parsing: Line %d.", linenum);
	exit(1);
}

token_t create_token_list(char* file)
{
	int i = 0;
 
	token_t head = checked_malloc(sizeof(struct token));
	head->data = 0;
	head->prev = 0;
	head->next = 0;
	token_t prev = head;
	int lineNum = 0;
	while (file[i] != '\0')
	{
		token_t cur;
		int moveForward = 1;
		if (file[i] == ' ' || file[i] == '\t')
		{
			i++;
			continue;
		}
		if (file[i] == '#')
		{
			while (file[i] != '\n' || file[i] != '\0')
				i++;
		}
 
		char c = file[i];
		
		if (c == '&' && file[i+1] == '&')
		{
			char* and = "&&\0";
			cur = allocate_token(prev,3,and, AND);
		}
		else if (c == ';')
		{
			char* semi = ";\0";
			cur = allocate_token(prev,2,semi, SEMI_COLON);
		}
		else if (c == '(')
		{
			char* open = "(\0";
			cur = allocate_token(prev,2,open, OPEN_PARA);
		}
		else if (c == ')')
		{
			char* close = ")\0";
			cur = allocate_token(prev,2,close, CLOSE_PARA);
		}
		else if (c == '<')
		{
			char* less = "<\0";
			cur = allocate_token(prev,2,less, LESS_THAN);
		}
		else if (c == '>')
		{
			char* greater = ">\0";
			cur = allocate_token(prev,2,greater, GREATER_THAN);
		}
		else if (c == '&')
		{
			if (file[i+1] == '&')
			{
				char* and = "&&\0";
				cur = allocate_token(prev,3,and,AND);
				i++;
			}
			else
				reportError(lineNum);
		}
		else if (c == '|')
		{
			if (file[i+1] == '|') 
			{
				char* or = "||\0";
				cur = allocate_token(prev,3,or,OR); 
				i++;
			}
			else
			{
				char* pipe = "|\0";
				cur = allocate_token(prev,2,pipe,PIPE);
			}
		}
		else if (c == '\n')
		{
			char* newline = "\n\0";
			cur = allocate_token(prev,2,newline,NEWLINE);
			
		}
		else
		{
			int count = 1;
			char* word = checked_malloc(count * sizeof(char));
			word[count - 1] = c;
			i++;
			while (file[i] != ' ' && file[i] != '\0' && file[i] != '\n' && file[i] != '\t')
			{
				count++;
				word = checked_realloc(word, count * sizeof(char));
				word[count-1] = file[i];
				i++;
			}
			if (file[i] == '\n')
				moveForward = 0;
			word[count] = '\0';
			cur = allocate_token(prev,count,word,WORD);
		}
		
		prev = cur;
		if (moveForward)
			i++;	
 
	}
	return head;
 
}
 
/* Op stuff  */
typedef struct op* op_t;
 
struct op
{
	char* data;
	enum token_type type;
};
 
int precedence(op_t newOperator, op_t oldOperator)
{
	//return true if new > old
	if (newOperator->type == PIPE)
		return ( oldOperator->type == GREATER_THAN || oldOperator->type == LESS_THAN );
 
	if (newOperator->type == GREATER_THAN || newOperator->type == LESS_THAN)
		return (oldOperator->type != GREATER_THAN && oldOperator->type != LESS_THAN);
 
	if (newOperator->type == AND || newOperator->type == OR)
		return (oldOperator->type != AND || oldOperator->type != OR);
 
}
 
op_t allocate_operator(enum token_type t)
{
	op_t o = checked_malloc(sizeof(struct op));
	o->type = t;
	if (t == GREATER_THAN)
	{
		o->data = (char*) checked_malloc(2*sizeof(char));
		o->data = ">\0";
	}
	else if (t == LESS_THAN)
	{
		o->data = (char*) checked_malloc(2*sizeof(char));
		o->data = "<\0";
	}
	else if (t == PIPE)
	{
		o->data = (char*) checked_malloc(2*sizeof(char));
		o->data = "|\0";
	}
	else if (t == OR)
	{
		o->data = (char*) checked_malloc(3*sizeof(char));
		o->data = "||\0";
	}
	else if (t == AND)
	{
		o->data = (char*) checked_malloc(3*sizeof(char));
		o->data = "&&\0";
	}

	return o;
}
 
command_t combine_command(command_t c1, command_t c2, op_t o)
{
	command_t op_cmd = checked_malloc(sizeof(struct command));
	if (o->type == AND)
		op_cmd->type = AND_COMMAND;
	else if (o->type == OR)
		op_cmd->type = OR_COMMAND;
	else if (o->type == PIPE)
		op_cmd->type = PIPE_COMMAND;
	
	op_cmd->input = 0;
	op_cmd->output = 0;
	(*op_cmd).u.command[0] = c1;
	(*op_cmd).u.command[1] = c2;
	return op_cmd;
}

command_t make_command_t(token_t head)
{
	//initialize stacks.
	stack_t cmd_stack = stack_init();
	stack_t op_stack = stack_init();
 
	//start parsing token list.
	token_t curr = head->next;
	int numLine = 0;

	while (curr != 0)
	{
		//we encounter a simple command.
		if (curr->type == WORD )
		{
			//sequences of 1 or more words are valid for simple commands.
			int len = strlen(curr->data) + 1;
			command_t cmd = checked_malloc(sizeof(struct command));
			cmd->type = SIMPLE_COMMAND;
			cmd->input = 0;
			cmd->output = 0;
			(*cmd).u.word = checked_malloc(sizeof(char*));
			int i = 0;
			(*cmd).u.word[i] = checked_malloc(len * sizeof(char));
			strcpy((*cmd).u.word[i++],curr->data);

			//get rest of command
			curr = curr->next;
			while ( curr != 0 && curr->type == WORD )
			{
				(*cmd).u.word = checked_realloc((*cmd).u.word,(i+1)*sizeof(char*));
				(*cmd).u.word[i] = checked_malloc((strlen(curr->data)+1) * sizeof(char));
				strcpy((*cmd).u.word[i++],curr->data);
				curr = curr->next;
			}
			stack_push(cmd_stack,cmd);
			continue;
		}
 
		if (curr->type == GREATER_THAN || curr->type == LESS_THAN || curr->type == AND || curr->type == OR || curr->type == PIPE)
		{
			op_t new_operator = allocate_operator(curr->type);
			op_t top_operator = stack_top(op_stack);

			size_t test = stack_count(op_stack);
			if (precedence(new_operator,top_operator) && !test)
			{
				stack_push(op_stack, new_operator);
			}
			else
			{
				while (!precedence(new_operator,top_operator) && top_operator->type != OPEN_PARA)
				{
					op_t oper = stack_pop(op_stack);
					command_t two = stack_pop(cmd_stack);
					command_t one = stack_pop(cmd_stack);
					command_t new_cmd = combine_command(one, two, oper);
					stack_push(cmd_stack, new_cmd);

					size_t empty = stack_count(op_stack);
					if (empty)
						break;
				}
				stack_push(op_stack, new_operator);
			}
		}
		
		
	}
	stack_free(cmd_stack);
	stack_free(op_stack);
}
 
 
 
command_stream_t make_command_stream (int (*get_next_byte) (void *), void *get_next_byte_argument)
{
	char* file = readFile(get_next_byte, get_next_byte_argument);
	token_t head = create_token_list(file);

	command_stream_t cs;
	cs->head = checked_malloc(sizeof(struct command_node));
	cs->next = 0;
	while (1)
	{
		

	}
	
}
 
 
command_t
read_command_stream (command_stream_t s)
{
	if (s->head == 0)
		return 0;
	else
	{
		
	}
	
}




int main()
{
	printf("hi\n");

}
