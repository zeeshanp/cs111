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
 
 
struct command_stream
{
	command_t head;			
};
 
 
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
    if (s->count == 0) error(1, 0, "Trying to get top from empty stack");
    return s->data[s->count - 1];
}
 
static size_t stack_count(stack_t s)
{
    if (!s) error(1, 0, "Null stack point given to stack count");
    return s->count;
}
 
int isValidChar(int c)
{
	return ( (c >= '0' && c <= '9') || ((c | ('a' - 'A')) - 'a' < 26u) 
			|| c == '!' || c == '@' || c == '%' || c == '^' || c == '-' || c == '_'
			|| c == '+' || c == ':' || c == ',' || c == '.' || c == '/');
}
 
 
/*  Tokenizer stuff */
 
 
enum token_type
{
	PIPE,
	SEMI_COLON,
	NEWLINE,
	INVALID,
	END_OF_FILE,
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
			char* newline = "\n\0";
			cur = allocate_token(prev,2,newline,NEWLINE);
			
		}
		else
		{
			int count = 1;
			char* word = checked_malloc( count * sizeof(char));
			word[count - 1] = c;
			i++;
			while (file[i] != ' ' && file[i] != '\0' && file[i] != '\n' && file[i] != '\t')
			{
				count++;
				word = checked_realloc(word, count * sizeof(char));
				word[count-1] = file[i];
				if (file[i] != \n)
					i++;
			}
			word[count] = '\0';
			cur = allocate_token(prev,count,word,WORD);
		}
		
		prev = cur;
		i++;	
 
	}
	return head;
 
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
 
 
 
command_stream_t make_command_stream (int (*get_next_byte) (void *), void *get_next_byte_argument)
{
	/* read in file */
	char* file = readFile(get_next_byte, get_next_byte_argument);
	token_t head = create_token_list(file);
	token_t temp = head->next;
	command_stream_t cs;
	while ( temp != 0)
	{
		//create command tree's here.
 
	}
 
	return cs;
 
 
 
}
 
 
command_t
read_command_stream (command_stream_t s)
{
  s->head = 0;
  return 0;
}