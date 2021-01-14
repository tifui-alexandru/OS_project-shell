#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <readline/readline.h>

// define constants
#define MAX_INPUT_LENGTH 1024
#define SIGMA 256
#define MAX_PATH_LENGTH 1024
#define MAX_COMMANDS_HISTORY 20
#define MAX_NUMBER_ARGUMENTS 5

char cwd[MAX_PATH_LENGTH];
char commands_history[MAX_COMMANDS_HISTORY][MAX_INPUT_LENGTH];

// trie implementation
struct Trie {
	bool is_leaf;
	struct Trie* children[SIGMA];
	int min_arg, max_arg;
};

// #define children (children + 128) // so that one can access children[-128]
typedef struct Trie* TrieNode;

TrieNode get_new_node() {
	TrieNode node = (TrieNode)malloc(sizeof(struct Trie));
	node->is_leaf = false;
	node->min_arg = 0;
	node->max_arg = 0;
	memset(node->children, 0, sizeof(node->children));
	return node;
}

TrieNode trie_root;

void insert(char* str, int v_min_arg, int v_max_arg) {
	TrieNode node = trie_root;

	for (char* letter = str; *letter; ++letter) {
		if (node->children[*letter] == NULL) 
			node->children[*letter] = get_new_node();

		node = node->children[*letter];
	}

	node->is_leaf = true;
	node->min_arg = v_min_arg;
	node->max_arg = v_max_arg;
}

bool search(char* str) {
	TrieNode node = trie_root;

	for (char* letter = str; *letter; ++letter) {
		if (node->children[*letter] == NULL) 
			return false;

		node = node->children[*letter];
	}

	return node->is_leaf;
}





// mantain history of commands
struct History {
	char command[MAX_INPUT_LENGTH];
	struct History* next_line;
};

typedef struct History* HistoryLine;

HistoryLine first_line, last_line;

HistoryLine get_new_line() {
	HistoryLine line = (HistoryLine)malloc((sizeof(struct History)));
	memset(line->command, 0, sizeof(line->command));
	return line;
}

void add_command(char* str) {
	HistoryLine line = get_new_line();
	strcpy(line->command, str);

	if (last_line == NULL) {
		first_line = last_line = get_new_line();
		first_line = last_line = line;
	}
	else {
		last_line->next_line = line;
		last_line = line;
	}

	free(line);
}

// -------------------------- UTILS ------------------------------


//returns if the str contains any special char
//if str is not valid it returns -2
// no special char found -1
//if a special char is found, it returns pos * 10 + type
// type is 1 for ||, 2 for &&, 3 for <, 4 for >, 5 for |
int token_str(char* str)
{
	char* next_char;
	int difference;
	for (next_char = str; *next_char != '\0' 
	&& *next_char !='<' && *next_char != '>' 
	&& *next_char != '|' && *next_char != '&'; 
	++next_char) 
	{}

	if (*next_char == '\0') 
		return -1;

	difference = next_char - str;

	if (*next_char == '|' && *(next_char+1) == '|')
		return (difference)*10 + 1;
	if (*next_char == '|' && *(next_char+1) != '|')
		return difference*10 + 5;
	if (*next_char == '&' && *(next_char + 1) == '&')
		return (difference)*10 + 2;
	if (*next_char == '&' && *(next_char + 1) != '&')
		return -2;
	if (*next_char == '<')
		return difference * 10 + 3;
	if (*next_char == '>')
		return difference * 10 + 4;
	
	return -2;
}

void copy_str(char* dest, char* src, int length)
{
	for (int i = 0; i < length; i++)
	{
		dest[i] = src[i];
	}
	dest[length] = '\0';
}

//returns if it can find the first word in src
bool get_command_name(char* dest, char* src)
{
	int dest_index, src_index, src_size;
	dest_index = 0;
	src_index = 0;
	src_size = strlen(src);
	
	for (src_index = 0; src[src_index] == ' ' && src_index < src_size; ++src_index){}
	
	if (src_index == src_size)
		return false;

	for (; src[src_index] != ' ' && src_index < src_size; ++src_index)
	{
		dest[dest_index] = src[src_index];
		++dest_index;
	}
	dest[dest_index] = '\0';
	return true;
}

int get_arguments(char** arguments, char* command){
	int command_ptr = 0;
	int command_size = strlen(nd){

	//remove the first word
	while (command[command_ptr] == ' ' && command_ptr < command_size)
		++command_ptr;
	while (command[command_ptr] != ' ' && command_ptr < command_size)
		++command_ptr;
	while (command[command_ptr] == ' ' && command_ptr < command_size)
		++command_ptr;

	while (command_ptr < command_size){
		int arg_ptr = -1;
		args_counter++;
		while (command[command_ptr] == ' ' && command_ptr < command_size)
			command_ptr ++;
		while (command[command_ptr] != ' ' && command_ptr < command_size){
			arg_ptr ++;
			arguments[args_counter][arg_ptr] = command[command_ptr];
			command_ptr ++;
		}
	}
	arguments[args_counter+1][0] = '\0';
	return args_counter + 1;
}

char** create_arguments_matrix()
{
	char** arguments;
	arguments = malloc(MAX_NUMBER_ARGUMENTS * sizeof(*arguments));

	for (int i = 0; i < MAX_NUMBER_ARGUMENTS; i++)
		arguments[i] = malloc(MAX_INPUT_LENGTH * sizeof(*(arguments[i])));
	
	return arguments;
}

void free_arguments_matrix(char** arguments)
{
	for (int i = 0; i < MAX_NUMBER_ARGUMENTS; i++)
		free(arguments[i]);
	free(arguments);
}

// ---------------------------------------------------------------------------





// write the current path in the commandline
void print_curr_dir() {

	if (getcwd(cwd, sizeof(cwd))) 
		printf("%s", cwd);
	else 
		perror("getcwd() error");
}


void funct_cd(char *path)
{
	printf("ok");
}


void find_command(char* command){

	char* command_name = malloc(MAX_INPUT_LENGTH*sizeof(*command_name));
	char** arguments;
	int args_counter;

	arguments = create_arguments_matrix(); 

	get_command_name(command_name, command);
	args_counter = get_arguments(arguments, command);
	printf("%d\n", args_counter);
	


	free_arguments_matrix(arguments);
	free(command_name);
}


// read input from stdin
void read_input(char* input) {
	//to be able to read char by char and not put stdin in a buffer
	

	//printf("$ ");
	char * command = malloc(MAX_INPUT_LENGTH*sizeof(*command));
	//char * temp = malloc(MAX_INPUT_LENGTH*sizeof(*temp));
	int temp_index = -1;
	char c;

	/*
	system ("/bin/stty raw");
	while ((int)(c = getc(stdin)) != 13)
	{
		temp_index++;
		
		if ((int) c == 27)
		{
			printf("comanda sus");
		}
		temp[temp_index] = c;
		
	}
	++temp_index;
	temp[temp_index] = '\0';
	//revert changes
	system ("/bin/stty cooked");
	*/

	char* temp=readline("$ ");

	if (strlen(temp) > 0)
		strcpy(input, temp);

	char* input_ptr;
	int token = 1, type;
	input_ptr = input;

	while (token >= 0)
	{
		token = token_str(input_ptr);
	
		if (token == -1)
		{
			copy_str(command, input_ptr, strlen(input_ptr));
			find_command(command);
			//find what command is and call it.
			break;
		}
		else if (token == -2)
		{
			printf("Comanda invalida\n");
			//invalid command
		}
		else 
		{
			type = token % 10;
			token = token / 10;
			copy_str(command, input_ptr, token);
			input_ptr += token;
			// ||
			if (type == 1)
			{
				int exit_status;
				//call the function 
				if (exit_status == 0)
					break;
			}
			// &&
			if (type == 2)
			{
				int exit_status;
				//call the function
				if (exit_status != 0)
					break;
			}
			// <
			if (type == 3)
			{
				
			}
			// >
			if (type == 4)
			{

			}
			// |
			if (type == 5)
			{

			}
		}

	}
	//free(temp);
	free(command);

}


enum CommandType {
	logic_expression,
	pipe_line,
	redirect_io,
	regular
};


// store the possible commands
void populate_trie() {
	trie_root = get_new_node();

	FILE* fin = fopen("commands.txt", "r");

	char chunk[MAX_INPUT_LENGTH + 10], command_text[MAX_INPUT_LENGTH];
	int command_min_arg, command_max_arg;

	while(fgets(chunk, sizeof(chunk), fin) != NULL) {
        char* token = strtok(chunk, " \n");
        strcpy(command_text, token);

        token = strtok(NULL, " \n");
        command_min_arg = atoi(token);

        token = strtok(NULL, " \n");
        command_max_arg = atoi(token);

        insert(command_text, command_min_arg, command_max_arg);
    }

	fclose(fin);
}


// initialize everything before starting the program
void init() {
	populate_trie();
}

int main() {
	init();

	char input[MAX_INPUT_LENGTH];
	while(true) {
		print_curr_dir();
		read_input(input);
	}

	return 0;
}