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
	size_t max;
	size_t top;
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
		s->data = (void**)checked_grow_alloc( s->data, &s->max);
	s->data[(s->top)++] = data;
}

void* pop(stack_t s)
{
	if (!s)
		error(1,0, "cannot pop from null stack");
	if (s->top == 0)
		error(1,0, "cannot pop from empty stack");
	(s->top)--;
	return s->data[s->top];
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
	SIMPLE,  //ls, tr, simple commands
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
	WORD,  //hello in "echo hello"
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
	token_t t = (token_t)checked_malloc(sizeof(struct token));
	prev->next = t;
	t->prev = prev;
	t->data =(char*)checked_malloc(dataSize*sizeof(char));
	strcpy(t->data,data);
	t->type = type;
	t->next = 0;
	return t;
}

token_t create_token_list(char* file)
{
	int i = 0;
	
	//i think head should be a dummy header (no data)
	token_t head = checked_malloc(sizeof(struct token));
	head->data = 0;
	head->prev = 0;
	head->next = 0;
	token_t prev = head;
	while (file[i] != '\0')
	{
		token_t cur;

		//should each token end in a '\0'? 
		if (!isValidChar(file[i]))
		{
			error(1,0,"invalid token");
		}
		if (file[i] == ' ')
		{
			i++;
			continue;
		}
		//find a comment
		if (file[i] == '#')
		{
			while (file[i] != '\n' || file[i] != '\0')
				i++;
			if (file[i] == '\n')
				i++;
		}

		char c = file[i];
	
		
		if (c == ';')
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
			if (file[i+1] == '\n')
			{
				char* double_new = "\n\n\0";
				cur = allocate_token(prev,3,double_new,DOUBLE_NEWLINE);
				i++;
			}
			else
			{
				char* newline = "\n\0";
				cur = allocate_token(prev,2,newline,NEWLINE);
			}
		}
		else
		{
			//this could be a simple word like "hello" or a command like echo,
			//how to tell the difference?
		}

		
		prev = cur;
		i++;	
		
	}

	return 0;
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

	return 0;

}


command_t
read_command_stream (command_stream_t s)
{
  s->head = 0;
  return 0;
}
