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
};

// #define children (children + 128) // so that one can access children[-128]
typedef struct Trie* TrieNode;

TrieNode get_new_node() {
	TrieNode node = (TrieNode)malloc(sizeof(struct Trie));
	node->is_leaf = false;
	memset(node->children, 0, sizeof(node->children));
	return node;
}

TrieNode trie_root;

void insert(char* str) {
	TrieNode node = trie_root;

	for (char* letter = str; letter; ++letter) {
		if (node->children[*letter] == NULL) 
			node->children[*letter] = get_new_node();

		node = node->children[*letter];
	}

	node->is_leaf = true;
}

bool search(char* str) {
	TrieNode node = trie_root;

	for (char* letter = str; letter; ++letter) {
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



// initialize everything before starting the program
void init() {
	trie_root = get_new_node();
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