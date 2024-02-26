#include "list.h"

//Defining a structure for tokens so that both an array
//and a number of tokens can be returned from functions
typedef struct TOKENS{
	char **array;
	int num_indices;
}Tokens;

//Prints out the tokens within a Tokens struct
void print_tokens(Tokens tokens);

//Prompts the user to input a command with a '->' at the start of the line
char *prompt();

char *check_aliases(char *string, List aliases);

/*Splits the given string into tokens, Tokens contains array of tokens
  and the number of indices in the array */
Tokens tokenize_string(char* string);

//Executes the given command using a child process
void execute_command(Tokens tokens, List history, List aliases, int command_count);

//alias commands
void print_alias(List aliases);
Boolean check_duplicate_aliases(Tokens tokens, List aliases);
void execute_add_alias(Tokens tokens, List aliases);
void add_alias(char* command, List aliases);
void remove_alias(Tokens tokens, List alias);

//Executes the setpath command
void execute_set_path(Tokens tokens);

//Gets the path for the user when the user types the appropriate command in the terminal
void execute_get_path(Tokens tokens);

//Executes the cd command
void execute_change_directory(Tokens tokens);

//Prints the users history
void execute_print_history(Tokens tokens, List history);

/*Executes a history invocation command e.g. interprets the users input such as !!, !-5, !20 etc. and determines
 *the index required to retrieve the command from history */
void execute_history_invocation(Tokens tokens, List history, List aliases, int command_count);

/*Retrieves and executes the command in history which is found at the given index. This function can potentially
 *Cause an infinite loop if history invocations are added to history. Therefore it's important to ensure that history
 *invocations e.g. !!, !-1, !5 etc. never enter history, safe otherwise*/
void invoke_history(int index, List history, List aliases, int command_count);

//Executes a command externally from the shell
void execute_external_command(Tokens tokens);

//Sets the path for the user when the user types the appropriate command in the terminal
Boolean set_path(char *directory);

//Changes the users directory to the specified directory, returns success status via Boolean 
void change_directory(char *directory);

//Save and load functions for history and aliases:
void save_history(List history);
void load_history(List *history, int *command_count);
void save_aliases(List aliases);
void load_aliases(List *aliases);

//Adds a command to history
void add_history(char* command, List *history, int *command_count);

//Replaces any aliases in the command with their alias
Boolean replace_aliases(char *input, char *output, List aliases);

//recursive call of replace_aliases
Boolean replace_aliases_r(char *input, char *output, List aliases, List *previously_seen);