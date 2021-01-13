#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <readline/readline.h>

// define constants
#define MAX_INPUT_LENGTH 1024
#define SIGMA 256

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




// write the current path in the commandline
void print_curr_dir() {
	char cwd[SIGMA];

	if (getcwd(cwd, sizeof(cwd))) 
		printf("%s\n", cwd);
	else 
		perror("getcwd() error");
}




// read input from stdin ----- not working now
void read_input(char* input) {
	char* temp = readline("$ ");
	if (strlen(temp) > 0)
		strcpy(input, temp);
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