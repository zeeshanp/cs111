// UCLA CS 111 Lab 1 command reading

#include "alloc.h"
#include "command.h"
#include "command-internals.h"

#include <error.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>


/* struct/enum definitions */

enum token_type {
	START,
	PAREN,
	LESS_THAN,
	GREATER_THAN,
	AND,
	OR,
	PIPE,
	SEMICOLON,
	WORD
};

typedef struct command_stream
{
	command_t command;
	command_stream_t next;

} command_stream;

typedef struct stack* stack_t;

struct stack
{
	command_t data[40];
	int count;
}; 

typedef struct token* token_t;

struct token
{
	enum token_type type;
	char* string;
	int line;
	token_t next;

};

typedef struct token_list* token_list_t;

struct token_list
{
	token_t head;
	token_list_t next;
};



/* stack stuff */

void push(stack_t s, command_t cmd)
{
	s->data[s->count] = cmd;
	(s->count)++;
}

command_t top(stack_t s)
{
	return s->data[s->count - 1];
}


command_t pop(stack_t s)
{
	return s->data[--(s->count)];
}


int empty(stack_t s)
{
	return (s->count == 0);
}

int size(stack_t s)
{
	return s->count;
}

/* reads file into stream, ignoring whitespace and comments . */
int readFile(char* stream, int (*get_next_byte) (void *), void* get_next_byte_argument)
{
	int inputSize = 128;
	int count = 0;
	int c;
	stream = (char*)checked_malloc(inputSize*sizeof(char));
	
	while ( (c = get_next_byte(get_next_byte_argument)) != -1)
	{
		if (c == '#')
		{
			while (c != -1 && c != '\n')
				c = get_next_byte(get_next_byte_argument);
		}

		if (c == ' ' || c == '\t')
		{
			while (c == ' ' || c == '\t')
				c = get_next_byte(get_next_byte_argument);
		}

		if (c != -1)
		{
			stream[count++] = c;
			if (count == inputSize)
			{
				inputSize += 128;
				stream = checked_grow_alloc(stream, &inputSize);
			}
		}

	}
	return count;
 
}

void reportError(int lineNum, int err_msg)
{
	switch (err_msg)
	{
	case 1: error(1,0, "Line %d: Extra ) found.", lineNum); break;
	case 2: error(1,0, "Line %d: Missing &.", lineNum); break;
	case 3: error(1,0, "Line %d: Need command after redirect.", lineNum); break;
	case 4: error(1,0, "Line %d: Invalid Character.", lineNum); break;
	case 5: error(1,0, "Line %d: Missing ).",lineNum); break;
	case 6: error(1,0, "Line %d: Two Outputs.", lineNum); break;
	default: error(1,0,"Line %d: Unknown Error.", lineNum); break;
	};
}

token_t allocate_token(enum token_type type, char* data, int line)
{
	token_t t = checked_malloc(sizeof(struct token));
	t->type = type;
	t->line = line;
	t->string = data;
	t->next = 0;
	return t;
}

int isValid(char c)
{
	return ((c >= '0' && c <= '9') || isalpha(c) || c == '!' || c == '@'
		|| c == '%' || c == '^'	|| c == '-' || c == '_'	|| c == '+' || c == ':'
		|| c == ',' || c == '.'	|| c == '/');
}

int merge(stack_t ops, stack_t operands)
{
	if (size(operands) < 2 || size(ops) == 0)
		return 0;

	command_t right = pop(operands);
	command_t left = pop(operands);
	command_t cmd = pop(ops);
	cmd->u.command[0] = left;
	cmd->u.command[1] = right;
	push(operands, cmd);

	return 1;
}


token_list_t make_tokens(char* stream, int count)
{
	token_t head = allocate_token(START, 0, 0);
	token_t curr = head;
	
	token_list_t t = checked_malloc(sizeof(struct token_list));
	token_list_t curr_list = t;
	curr_list->head = head;

	int lineNum = 1;
	int i = 0;
	char c = stream[i];
	while (i < count)
	{
		if (c == '(')
		{
			int subshell_line = lineNum;
			int level = 1;
			int count = 0;
			size_t subshell_size = 10;
			char* subshell = checked_malloc(subshell_size);

			while (level > 0) 
			{
				c = stream[++i];
				if (i == count) 
				{
					reportError(lineNum,5);
					return NULL;
				}

				if (c == '\n') 
				{
					c = ';';
					lineNum++;
				}
				else if (c == '(')
					level++;
				else if (c == ')')
				{
					if (--level == 0)
					{
						c = stream[++i];
						break;
					}
				}
				subshell[count] = c;
				count++;

				if (count == subshell_size)
				{
					subshell_size = subshell_size * 2;
					subshell = checked_grow_alloc (subshell, &subshell_size);
				}
			}
			curr->next = allocate_token(PAREN, subshell, subshell_line);
			curr = curr->next;
		}
		else if (c == ')')
		{
			reportError(lineNum, 1);
			return 0;
		}
		else if (c == '&')
		{
			c = stream[++i];
			if ( i != count && c == '&')
			{
				curr->next = allocate_token(AND, 0, lineNum);
				curr = curr->next;
				c = stream[++i];
			}
			else
			{
				reportError(lineNum,2);
				return 0;
			}
		}
		else if (c == '|')
		{
			c = stream[++i];
			if (i != count && c == '|')
			{
				curr->next = allocate_token(OR,0,lineNum);
				curr = curr->next;
				c = stream[++i];
			}
			else
			{
				curr->next = allocate_token(PIPE,0,lineNum);
				curr = curr->next;
			}
		}
		else if (c == '>')
		{
			curr->next = allocate_token(GREATER_THAN,0,lineNum);
			c = stream[++i];
		}
		else if (c == '<')
		{
			curr->next = allocate_token(LESS_THAN,0,lineNum);
			c = stream[++i];
		}
		else if (c == ';')
		{
			curr->next = allocate_token(SEMICOLON,0,lineNum);
			c = stream[++i];
		}
		else if (c == '\n')
		{
			lineNum++;
			if (curr->type == PAREN || curr->type == WORD)
			{
				curr_list->next = checked_malloc(sizeof(struct token_list));
				curr_list = curr_list->next;
				curr_list->head = allocate_token(START,0,0);
				curr = curr_list->head;
			}
			else if (curr->type == LESS_THAN || curr->type == GREATER_THAN)
			{
				reportError(lineNum, 3);
				return 0;
			}
			c = stream[++i];
		}
		else if (isValid(c))
		{
			int len = 0;
			size_t buff = 16;
			char* word = checked_malloc(buff * sizeof(char));
			do
			{
				word[len] = c;
				len++;
				if (len == buff)
				{
					buff *= 2;
					word = checked_grow_alloc(word,&buff);
				}
				c = stream[++i];

			} while (isValid(c) && i < count);

			curr->next = allocate_token(WORD,word, lineNum);
			curr = curr->next;
		}
		else
		{
			reportError(lineNum,4);
			return 0;
		}
		
	}
	return t;
}

command_t make_command_tree(token_t head)
{
	token_t curr = head;
	int lineNum = curr->line;

	stack_t ops = checked_malloc(sizeof(struct stack));
	stack_t operands = checked_malloc(sizeof(struct stack));
	operands->count = 0;
	ops->count = 0;

	command_t prev_cmd = 0;
	command_t curr_cmd;

	do 
	{
		if(curr->type != LESS_THAN && curr->type != GREATER_THAN)
			curr_cmd = checked_malloc(sizeof(struct command));

		switch (curr->type)
		{
			case PAREN:
				curr_cmd->type = SUBSHELL_COMMAND;
				curr_cmd->u.subshell_command = make_command_tree(make_tokens(curr->string, strlen(curr->string))->head);
				push(operands, curr_cmd);
				break;
			case LESS_THAN:
				if (prev_cmd == NULL ||	(prev_cmd->type != SIMPLE_COMMAND && prev_cmd->type != SUBSHELL_COMMAND))
				{
					reportError(lineNum, 2);
					return NULL;
				}
				else if (prev_cmd->output != NULL)
				{
					reportError(lineNum, 6);
					return NULL;
				}
				else if (prev_cmd->input != NULL) {
					reportError(lineNum, 6);
					return NULL;
				}
				curr = curr->next;
				if (curr->type == WORD)
					prev_cmd->input = curr->string;
				else
				{
					reportError(lineNum, 2);
					return NULL;
				}
				break;
			case GREATER_THAN:
				if (prev_cmd == NULL ||	(prev_cmd->type != SIMPLE_COMMAND && prev_cmd->type != SUBSHELL_COMMAND)) {
					reportError(lineNum, 2);
					return NULL;
				}
				else if (prev_cmd->output != NULL) {
					reportError(lineNum, 6);					
					return NULL;
				}

				curr = curr->next;
				if (curr->type == WORD)
					prev_cmd->output = curr->string;
				else
				{
					reportError(lineNum, 6);
					return NULL;
				}
				break;
			case AND:
			case OR:
				curr_cmd->type = (curr->type == AND ? AND_COMMAND : OR_COMMAND);
								
				if (!empty(ops) && (top(ops)->type == PIPE_COMMAND || top(ops)->type == OR_COMMAND ||
						top(ops)->type == AND_COMMAND))
					if(!merge(ops, operands))
					{
						reportError(lineNum, 10);
						return NULL;
					}
				push(ops, curr_cmd);
				break;
			case PIPE:
				curr_cmd->type = PIPE_COMMAND;
				if (!empty(ops) && top(ops)->type == PIPE_COMMAND)
					if (!merge(ops, operands)) 
					{
						reportError(lineNum, 10);
						return NULL;
					}
				push(ops, curr_cmd);
				break;

			case SEMICOLON:
				curr_cmd->type = SEQUENCE_COMMAND;
				if (!empty(ops))
					if(!merge(ops, operands)) {
						reportError(lineNum, 10);
						return NULL;
					}
				push(ops, curr_cmd);
				break;

			case WORD:
				curr_cmd->type = SIMPLE_COMMAND;

				int num_words = 1; 
				token_t cur_token = curr;
				while (cur_token->next != NULL && cur_token->next->type == WORD)
				{
					num_words++;
					cur_token = cur_token->next;
				}

				curr_cmd->u.word = checked_malloc((num_words + 1) * sizeof(char*));
				int i;
				for (i = 0; i < num_words-1; i++) {
					curr_cmd->u.word[i] = curr->string;
					curr = curr->next;
				}
				curr_cmd->u.word[i] = curr->string;
				curr_cmd->u.word[num_words] = 0;
				push(operands, curr_cmd);
				break;
			default:
				break;
		};
				
		prev_cmd = curr_cmd;
	} while(curr != NULL && (curr = curr->next) != NULL);

	
	while(size(ops) != 0)
		if (!merge(ops, operands)) 
		{
			reportError(lineNum, 10);
			return NULL;
		}

	if (size(ops) != 1) 
	{
		reportError(lineNum, 10);
		return NULL;
	}
	return pop(operands);
}



command_stream_t make_command_tree_root(token_list_t t)
{
	token_list_t curr_list = t;

	command_stream_t head, curr, prev;
	head = 0;
	curr = 0;
	prev = 0;

	while (curr_list != 0 && curr_list->head->next != 0)
	{
		curr = checked_malloc(sizeof(command_stream));
		curr->command = make_command_tree(curr_list->head->next);

		if (!head)
		{ 
			head = curr;
			prev = head;
		}
		else
		{
			prev->next = curr;
			prev = curr;
		}

		curr_list = curr_list->next;
	}

	return head;
}


command_stream_t make_command_stream(int (*get_next_byte) (void *), void *get_next_byte_argument)
{
	char* stream;
	int count = readFile(stream, get_next_byte, get_next_byte_argument);
	token_list_t t = make_tokens(stream, count);
	return make_command_tree_root(t);
}


command_t read_command_stream(command_stream_t s)
{
	if (s == 0 || s->command == 0)
		return 0;
	command_t curr = s->command;
	if (s->next != 0)
	{
		command_stream_t next = s->next;
		s->command = s->next->command;
		s->next = s->next->next;	
	}
	else
		s->command = 0;

	return curr;
}
