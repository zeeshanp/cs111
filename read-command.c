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

/* command stream */



struct command_stream
{
	command_t head;			
};

/** Stack definitions
 *                       */

typedef struct stack* stack_t;

struct stack
{
	void** data;
	int max;
	int top;
};

stack_t stack_init()
{
	stack_t temp = (stack_t)checked_malloc(sizeof(struct stack));
	temp->max = 32;
	temp->top = 0;
	temp->data = (void**)checked_malloc(temp->max * sizeof(void*));
	return temp;
}

void stack_free(stack_t s)
{
	if (!s)
		error(1,0, "cannot delete null stack");
	free(s->data);
	free(s);
}

void push(stack_t s, void* data, size_t elemSize)
{
	if (!s)
		error(1,0, "cannot push to null stack");
	if (s->top == s->max)
	{
		s->max *= 2;
		s->data = (void**)checked_realloc( s->data, s->max * sizeof(void*));
	}
	s->data[s->top] = (void*)checked_malloc(elemSize);
	s->data[(s->top)++] = data;
}

void* pop(stack_t s)
{
	if (!s || s->top == 0)
		error(1,0, "cannot pop from null or empty stack");
	return s->data[s->top--];
}

void* top(stack_t s)
{
	if (!s)
		error(1,0, "can't find top of null stack");
	return s->data[s->top-1];
}

int stack_size(stack_t s)
{
	return s->top;
}


/* Auxillary Functions */

int isValidChar(int c)
{
	return ( (c >= '0' && c <= '9') || ((c | ('a' - 'A')) - 'a' < 26u) 
				|| c == '!' || c == '@' || c == '%' 
				|| c == '^' || c == '-' || c == '_'
				|| c == '+' || c == ':' || c == ',' 
				|| c == '.' || c == '/');
}


/*  Tokenizer stuff */


enum token_type
{
	SIMPLE,
	PIPE,
	SEMI_COLON,
	DOUBLE_NEWLINE,  //2 newlines seperates commands
	NEWLINE,
	SUBSHELL,
	INVALID,
	END_OF_FILE,
	WHITESPACE,
	AND,
	OR,
	LESS_THAN,
	GREATER_THAN,
	OPEN_PARA,
	CLOSE_PARA,
	EMPTY,
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

enum token_type get_type(char* data)
{
	int len = strlen(data);

	if (len == 0)
		return EMPTY;	

	int i = 0;
	while (data[i] != '\0')
	{
		char c = data[i];
		if (len == 1)
		{
			switch (c)
			{
				case ' ': return WHITESPACE;
				case '\n': return NEWLINE;
				case -1: return END_OF_FILE;
				case ';': return SEMI_COLON;
				case '(': return OPEN_PARA;
				case ')': return CLOSE_PARA;
				case '<': return LESS_THAN;
				case '>': return GREATER_THAN;
				case '|': return PIPE;
				default: break;
			}
		}
		if (len == 2)
		{
			if (data[i] == '&' && data[i+1] == '&')
				return AND;
			if (data[i] == '|' && data[i+1] == '|')
				return OR; 
			if (data[i] == '\n' && data[i+1] == '\n')
				return DOUBLE_NEWLINE;
		}
		
		if (isValidChar(data[i]))
		{
			i++;
			continue;
		}
		else
		{
			return INVALID;
		}
		i++;
	}
	return WORD;
}

token_t create_token_list(char* file)
{
	int i = 0;
	token_t head = checked_malloc(sizeof(struct token));
	head->next = 0;
	head->prev = 0;
	token_t cur = head;
	while (file[i] != '\0')
	{
		
		if (!isValidChar(data[i]))
		{
			;//error
		}
		if (file[i] == ' ')
		{
			i++;
			continue;
		}

		char c = file[i];

		if (c == -1)
		{
			cur->type = END_OF_FILE; 
			cur->data = c;
		}
		else if (c == ';')
		{
			cur->type = SEMI_COLON;
			cur->data = c;
		}
		else if (c == '(')
		{
			cur->type = OPEN_PARA;
			cur->data = c;
		}
		else if (c == ')')
		{
			cur->type = CLOSE_PARA;
			cur->data = c;
		}
		else if (c == '<')
		{
			cur->type = LESS_THAN;
			cur->data = c;
		}
		else if (c == '>')
		{
			cur->type = GREATER_THAN;
			cur->data = c;
		}
		else if (c == '|')
		{
			cur->type = PIPE;
			cur->data = c;
		}
		else if (c == '\n')
		{
			cur->type = NEWLINE; 
			cur->data = c;
		}

		//check for individual and/ors, and just have a long linked list of tokens with duplicates


		token_t t = checked_malloc(sizeof(struct token));
		


		
	}

}

token_t change_duplicates(token_t head); //aux function: checking linked list of tokens and converting doubles (&&, ||, etc) to single tokens

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


/*** do this ***/
command_stream_t make_command_stream (int (*get_next_byte) (void *), void *get_next_byte_argument)
{
	char* file = readFile(get_next_byte, get_next_byte_argument);
	/* gneeral guideline:
	- create linked list of tokens (white space is removed from this step) using create_token_list
	- remove doubles
	- parse through linked list, this is where the pseudocode with the two stacks will be used
	*/
	done_parsing:	

	

}


command_t
read_command_stream (command_stream_t s)
{
  s->head = 0;
  return 0;
}
