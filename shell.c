#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h> 
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <readline/readline.h>
#include <dirent.h>

// define constants
#define MAX_INPUT_LENGTH 1024
#define SIGMA 256
#define MAX_PATH_LENGTH 1024
#define MAX_COMMANDS_HISTORY 20
#define MAX_NUMBER_ARGUMENTS 5

// define colours
#define GREEN "\x1b[92m"
#define BLUE "\x1b[94m"
#define CYAN "\x1b[96m"
#define WHITE "\033[0m"
#define RED "\x1b[31m"
#define MAGENTA "\x1b[35m"

char cwd[MAX_PATH_LENGTH];
char commands_history[MAX_COMMANDS_HISTORY][MAX_INPUT_LENGTH];
int exit_status;

// trie implementation
struct Trie {
	int index; // -1 if it is not the end of a command
	struct Trie* children[SIGMA];
	int min_arg, max_arg;
};

// #define children (children + 128) // so that one can access children[-128]
typedef struct Trie* TrieNode;

TrieNode get_new_node() {
	TrieNode node = (TrieNode)malloc(sizeof(struct Trie));
	node->index = -1;
	node->min_arg = 0;
	node->max_arg = 0;
	memset(node->children, 0, sizeof(node->children));
	return node;
}

TrieNode trie_root;

void insert(char* str, int v_min_arg, int v_max_arg, int idx_command) {
	TrieNode node = trie_root;

	for (char* letter = str; *letter; ++letter) {
		if (node->children[*letter] == NULL) 
			node->children[*letter] = get_new_node();

		node = node->children[*letter];
	}

	node->index = idx_command;
	node->min_arg = v_min_arg;
	node->max_arg = v_max_arg;
}

int search(char* str, int* v_min_arg, int* v_max_arg) {
	TrieNode node = trie_root;

	for (char* letter = str; *letter; ++letter) {
		if (node->children[*letter] == NULL) 
			return -1;

		node = node->children[*letter];
	}

	*v_min_arg = node->min_arg;
	*v_max_arg = node->max_arg;
	return node->index;
}

// validate a command
// returns the command index if it is a valid command
// returns -1 otherwise
int valid_command(char* str) {
	if (str == NULL || str == "")
		return false;

	char* str_copy = malloc(strlen(str) * sizeof(*str_copy));
	strcpy(str_copy, str);

	char* token = strtok(str, " \n");
	int command_min_arg, command_max_arg;
	int idx_command = search(token, &command_min_arg, &command_max_arg);

	if (idx_command == -1)
		return -1;

	int no_args = 0;

	while (true) {
		token = strtok(NULL, " \n");
		if (token == NULL)
			break;

		++no_args;
	}

	strcpy(str, str_copy);
	free(str_copy);

	if (command_min_arg > no_args)
		return -1;

	if (command_max_arg != -1 && command_max_arg < no_args)
		return -1;

	return idx_command;
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

void add_command_to_history(char* str) {
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
}

void print_history() {
	for (HistoryLine line = first_line; line; line = line->next_line) {
		printf("%s\n", line->command);
	}
}

// -------------------------- UTILS ------------------------------


//returns if the str contains any special char
//if str is not valid it returns -2
// no special char found -1
//if a special char is found, it returns pos * 10 + type
// type is 1 for ||, 2 for &&, 3 for <, 4 for >, 5 for |
int token_str(char* str){
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

void copy_str(char* dest, char* src, int length){

	for (int i = 0; i < length; i++)
	{
		dest[i] = src[i];
	}
	dest[length] = '\0';
}

//returns if it can find the first word in src
bool get_command_name(char* dest, char* src){

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
	int command_size = strlen(command);
	int args_counter = -1;

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
		//remove the beginning spaces
		while (command[command_ptr] == ' ' && command_ptr < command_size)
			command_ptr ++;
		if (command_ptr == command_size)
			break;
		
		if (command[command_ptr] == '"'){
			++command_ptr;
			while (command[command_ptr] != '"' && command_ptr < command_size)
			{
				arg_ptr++;
				arguments[args_counter][arg_ptr] = command[command_ptr];
				command_ptr++;
			}
			//the argument is invalid, so arguments must be empty
			if (command_ptr == command_size){
				arguments[0][0] = '\0';
				return -1;
			}
			command_ptr++;
			arguments[args_counter][command_ptr] = '\0';
			continue;
		}

		while (command[command_ptr] != ' ' && command_ptr < command_size){
			arg_ptr ++;
			arguments[args_counter][arg_ptr] = command[command_ptr];
			command_ptr ++;
		}
		arguments[args_counter][arg_ptr + 1] = '\0';
	}

	arguments[args_counter + 1][0] = '\0';
	return args_counter + 1;
}

char** create_arguments_matrix()
{
	char** arguments;
	arguments = malloc(MAX_NUMBER_ARGUMENTS * sizeof(*arguments));

	for (int i = 0; i < MAX_NUMBER_ARGUMENTS; i++)
		arguments[i] = malloc(MAX_INPUT_LENGTH * sizeof(*(arguments[i])));
	
	for (int i = 0 ;i < MAX_NUMBER_ARGUMENTS; ++i){
		for (int j = 0; j < MAX_INPUT_LENGTH; ++j)
			arguments[i][j] = '\0';
	}

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

// implement commands

void funct_ls(char** args) {
	exit_status = 1;

	struct dirent** namelist;
	int no_files = scandir(".", &namelist, NULL, alphasort);

	if (no_files == -1) {
		perror("Error ls");
		return;
	}

	for (int i = 0; i < no_files; ++i) {
		if (namelist[i]->d_name[0] != '.') {
			if (namelist[i]->d_type == DT_REG)
				printf("%s%s\n", BLUE, namelist[i]->d_name);
			else if (namelist[i]->d_type == DT_DIR)
				printf("%s%s\n", GREEN, namelist[i]->d_name);
			else
				printf("%s%s\n", CYAN, namelist[i]->d_name);
		}
	}

	printf("%s", WHITE);
	exit_status = 0;
}	

void funct_echo(char** args) {
	exit_status = 1;

	for (int i = 0; args[i][0] != '\0'; ++i) 
		printf("%s ", args[i]);
	printf("\n");

	exit_status = 0;
}

void funct_touch(char** args) {
	exit_status = 1;

	for (int i = 0; args[i][0] != '\0'; ++i) { 
		FILE* file = fopen(args[i], "w");
		if (file == NULL) {
			perror("Error touch");
			return;
		}
		else 
			fclose(file);
	}

	exit_status = 0;
}

void funct_mkdir(char** args) {
	exit_status = 1;

	for (int i = 0; args[i][0] != '\0'; ++i) { 
		if (mkdir(args[i], 0777) == -1) {
			perror("Error mkdir");
			return;
		}
	}

	exit_status = 0;
}

void funct_grep(char** args) {
	exit_status = 1;

	bool single_arg = true;
	if (args[2][0] != '\0')
		single_arg = false;

	char chunk[MAX_INPUT_LENGTH + 10];
	int len = strlen(args[0]);

	for (int i = 1; args[i][0] != '\0'; ++i) {
		FILE* fin = fopen(args[i], "r");

		if (fin == NULL) {
			perror("Error grep");
			continue;
		}

		while(fgets(chunk, sizeof(chunk), fin) != NULL) {
			if(strstr(chunk, args[0])) {
				if (!single_arg)
					printf("%s%s: ", MAGENTA, args[i]);

				char* found = strstr(chunk, args[0]);
				char* last = chunk;

				while (found) {
					for (char* p = last; p != found; ++p)
						printf("%s%c", WHITE, *p);

					if (*(found + len) == '\0')
						break;

					last = found + len;
					found = strstr(found + len, args[0]);
					printf("%s%s", RED, args[0]);
				}

				printf("%s%s", WHITE, last);
			}
		}

		fclose(fin);
	}

	exit_status = 0;
}

void funct_cd(char** args) {
	exit_status = 1;

	if (chdir(args[0]) != 0){
		perror("Error cd");
		return;
	}

	exit_status = 0;
}

void funct_mv(char** args) {

	char* src = args[0];
	char* dst = args[1];
    FILE *f_read, *f_write;

    f_read = fopen(src, "r"); 
    if(f_read == NULL){
		printf("Source file does not exist.\n"); 
		return;
	}

    f_write = fopen(dst, "ab+"); 
    fclose(f_write);

    f_write = fopen(dst, "w+"); //deschid pt scris
    if(f_write == NULL){
        printf("Error while creating destination file.\n");
        fclose(f_read); 
		return;
    }

    if(open(dst, O_WRONLY)<0 || open(src, O_RDONLY)<0){
		printf("An error occured"); 
		return;
	}

    char cp;
    while((cp=getc(f_read))!=EOF)  putc(cp,f_write);

    fclose(f_read); 
	fclose(f_write);

	int fd = open(args[0], O_RDONLY);
	if (fd < 0){
		printf("File does not exist");
		return;
	}
	close(fd);

	if (unlink(args[0])){
		printf("An error occured while deleting the file\n");
		return;
	}
	printf("Succes\n");
	
}

void funct_cp(char** args) {

	char* src = args[0];
	char* dst = args[1];
    FILE *f_read, *f_write;

    f_read = fopen(src, "r"); 
    if(f_read == NULL){
		printf("Source file does not exist.\n"); 
		return;
	}

    f_write = fopen(dst, "ab+"); 
    fclose(f_write);

    f_write = fopen(dst, "w+"); //deschid pt scris
    if(f_write == NULL){
        printf("Error while creating destination file.\n");
        fclose(f_read); 
		return;
    }

    if(open(dst, O_WRONLY)<0 || open(src, O_RDONLY)<0){
		printf("An error occured"); 
		return;
	}

    char cp;
    while((cp=getc(f_read))!=EOF)  putc(cp,f_write);

    fclose(f_read); 
	fclose(f_write);

	printf("Succes\n");
}

void funct_rm(char** args){
	exit_status = 1;

	int fd;
	fd = open(args[0], O_RDONLY);
	if (fd < 0){
		printf("File does not exist");
		return;
	}
	close(fd);

	if (unlink(args[0])){
		printf("An error occured while deleting the file\n");
		return;
	}
	printf("Succes\n");
	exit_status = 0;
	return;
}

void funct_rmdir(char** args){
	exit_status = 1;

	DIR* dir = opendir(args[0]);
	if (dir){
		//it exists
		closedir(dir);
		rmdir(args[0]);
	}
	else if (ENONET == errno){
		printf("Directory does not exits\n");
		return;
	}
	else{
		printf("An error occured\n");
		return;
	}

	exit_status = 0;
}

void funct_clear(char** args) {
	exit_status = 1;
	system("clear");
	exit_status = 0;
}

void funct_history(char** args) {
	exit_status = 1;
	print_history();
	exit_status = 0;
}

void funct_pwd(char** args) {
	exit_status = 1;
	print_curr_dir();
	printf("\n");
	exit_status = 0;
}

void funct_cat(char** args) {
	exit_status = 1;

	char chunk[MAX_INPUT_LENGTH + 10];

	for (int i = 0; args[i][0] != '\0'; ++i) {
		FILE* fin = fopen(args[i], "r");

		if (fin == NULL) {
			perror("Error cat");
			continue;
		}

		while(fgets(chunk, sizeof(chunk), fin) != NULL) {
			printf("%s", chunk);
		}

		fclose(fin);
	} 

	exit_status = 0;
}

void find_command(char* command){


	int command_idx = valid_command(command);
	if (command_idx == -1) {
		printf("Invalid command\n");
		return;
	}

	char* command_name = malloc(MAX_INPUT_LENGTH * sizeof(*command_name));
	char** arguments;
	int args_counter;

	arguments = create_arguments_matrix(); 

	get_command_name(command_name, command);
	args_counter = get_arguments(arguments, command);
	// printf("%d\n", args_counter);
	
	if (command_idx == 0)
		funct_ls(arguments);
	else if (command_idx == 1)
		funct_echo(arguments);
	else if (command_idx == 2)
		funct_touch(arguments);
	else if (command_idx == 3)
		funct_mkdir(arguments);
	else if (command_idx == 4)
		funct_grep(arguments);
	else if (command_idx == 5)
		funct_pwd(arguments);
	else if (command_idx == 6)
		funct_cd(arguments);
	else if (command_idx == 7)
		funct_mv(arguments);
	else if (command_idx == 8)
		funct_rm(arguments);
	else if (command_idx == 9)
		funct_rmdir(arguments);
	else if (command_idx == 10)
		funct_cat(arguments);
	else if (command_idx == 11)
		funct_history(arguments);
	else if (command_idx == 12)
		funct_clear(arguments);
	else if (command_idx == 13)
		funct_cp(arguments);

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

	bool flag = true;

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
			printf("Invalid command\n");
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
				//call the function 
				find_command(command);
				
				if (exit_status == 0){
					flag = false;
					break;
				}
				input_ptr += 2;
			}
			// &&
			if (type == 2)
			{
				
				
				find_command(command);
				
				if (exit_status != 0){
					flag = false;
					break;
				}
				input_ptr += 2;
			}
			// <
			if (type == 3)
			{
				++input_ptr;
			}
			// >
			if (type == 4)
			{
				++input_ptr;
			}
			// |
			if (type == 5)
			{
				++input_ptr;
			}
		}
		if (exit_status)
			flag = false;

	}
	if (flag == true)
		add_command_to_history(command);

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

	int idx_command = 0;

	while(fgets(chunk, sizeof(chunk), fin) != NULL) {
        char* token = strtok(chunk, " \n");
        strcpy(command_text, token);

        token = strtok(NULL, " \n");
        command_min_arg = atoi(token);

        token = strtok(NULL, " \n");
        command_max_arg = atoi(token);

        insert(command_text, command_min_arg, command_max_arg,idx_command);

    	++idx_command;
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