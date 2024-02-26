#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <dirent.h>
#include <math.h>
#include "shell.h"


int main(void) {
    List history;
    List aliases;
    int command_count;
    char *input;
	char expanded_input[MAX_CHARACTERS + 1] = {0};
    char original_input[MAX_CHARACTERS  + 1] = {0};
	Tokens tokens;

	//Find the user home directory from the environment
	char *home_path = getenv("HOME");
	//Set current working directory to user home directory
	chdir(home_path);
	//Save the current path
	char *original_path = getenv("PATH");
	load_history(&history, &command_count);
	load_aliases(&aliases);

	//While loop so that the user will keep being prompted for an input
	while (1) {
		input = prompt();
		//Make a copy of the original input to pass to history if appropriate
        strcpy(original_input, input);
		//If replace aliases returns false something has went wrong, prompt user again
		if(replace_aliases(input, expanded_input, aliases) == false) {
            free(tokens.array);
            free(input);
		    continue;
		}
		//If exit signal isn't passed, continue with operations
		if (strncmp(expanded_input, "exit", 4) != 0) {
            //Tokenize input so that it can be manipulated
			tokens = tokenize_string(expanded_input);
            /*creating a new process is very expensive, only execute
              if there's something to work with*/
            if(tokens.num_indices > 0 ){
                //This check is needed in case an alias results in a history invocation
                if(expanded_input[0] != '!') {
                    add_history(original_input, &history, &command_count);
                }
    			execute_command(tokens, history, aliases, command_count);
			}
			/* free memory assigned by the prompt and tokenize_string functions
			   to avoid memory leaks */
			free(tokens.array);
			free(input);
		} else {
			//Input signal has been passed -> exit shell
			break;
		}
	}
	save_history(history);
	save_aliases(aliases);
	//Path is restored to its original value before exiting
 	set_path(original_path);
	return 0;
}

char *prompt() {
	char* input = malloc(MAX_CHARACTERS * sizeof(char) + 1);
    char character;
	printf("->");

	/*If ctrl+d is pressed, returns exit to stop the program, second test is to test for ctrl+d
	  when input contains text, if ctrl+d is pressed no \n will be passed therefore stop */
	if (fgets(input, (MAX_CHARACTERS + 1), stdin) == NULL   // if fgets returns NULL, then EOF has been passed e.g. exit
	    || (((input[strlen(input) - 1]) !='\n') && (strlen(input) < MAX_CHARACTERS))) {
		//add a new line due to absence from user
		printf("\n");
		strcpy(input, "exit");
		return input;
	}

	ulong length = strlen(input) - 1;
	//If input is is larger than 512 character the program returns to main() to prompt an input form user again
	if (input[length] != '\n') {
		printf("\nThe command you have entered is too long, please try again.\n");
		//clear the remaining input buffer as command is too long
		while((character = (char) getchar()) != '\n' && character != EOF){
			//continuously clear the input buffer until the end of line.
		}
		strcpy(input, "");
		return input;
	}
	//Removes the new line from the user input
	if(input[length] == '\n') {
		input[length] = '\0';

	}
	return input;
}

Boolean replace_aliases(char* input, char* output, List aliases){
    //Use a linked list to test for multiple recursive calls of an alias
    List previously_seen = new_list();
    Boolean result = replace_aliases_r(input, output, aliases, &previously_seen);
    clear(previously_seen);
    free(previously_seen);
    return result;
}

Boolean replace_aliases_r(char *input, char *output, List aliases, List *previously_seen) {
    //If there are no aliases, do not modify the string
    if (is_empty(aliases)) {
        strcpy(output, input);
        return true;
    }
    char input_copy[MAX_CHARACTERS + 1];
    char token[MAX_CHARACTERS + 1];
    char result[MAX_CHARACTERS + 1];
    strcpy(input_copy, input);
    Tokens tokens = tokenize_string(input_copy);
    if (tokens.num_indices == 0) {
        strcpy(output, input);
        return true;
    }
    if (strcmp("alias", tokens.array[0]) == 0 || strcmp("unalias", tokens.array[0]) == 0) {
        //do not replace any aliases in this command
        strcpy(output, input);
        return true;
    }
    //Make sure output is empty
    strcpy(output, "");
    //test every token to see if it's an alias or not
    for(int i = 0; i < tokens.num_indices; i++) {
        //while is history invocation or alias
        strcpy(token, tokens.array[i]);
        //while token is a history invocation
        if(contains(*previously_seen, token) == true) {
            printf("Warning: alias '%s' has been used recursively, please unalias or use a different command\n", token);
            return false;
        }
        while(1) {
            strcpy(result, check_aliases(token, aliases));
            if(strcmp(result, "") != 0) {
                //Replace the command with the aliased command
                push(*previously_seen, token);
                strcpy(token, result);
                //test the aliased command for further aliases
                if(replace_aliases_r(result, token, aliases, previously_seen) == false){
                    return false;
                }
            } else {
            	//Make sure that the alias invocations don't overflow the output buffer
                if((strlen(output) + strlen(token)) <= MAX_CHARACTERS) {
                    strcat(output, token);
                } else {
                    printf("Warning: alias invocation causes command to be too long, please try a shorter command!\n");
                    return false;
                }
                break;
            }
        }
        strcat(output, " ");
    }
    return true;
}

char *check_aliases(char *string, List aliases) {
    Node *current = *aliases;
    int string_length = (int) strcspn(current->value, " ");
    do {
        if(strncmp(current->value, string, string_length) == 0) {
            //matches an aliases => return alias with name trimmed off
            return (current->value) + strcspn(current->value, " ") + 1;
        }
        current = current->next;
    } while (current != NULL);
    return "";
}

Tokens tokenize_string(char *string) {
	Tokens tokens;
	tokens.array = malloc(MAX_CHARACTERS * sizeof(char));
	//Keeps count of how many tokens there are
	int count = 0;
	//Takes first token and puts it an an array
	tokens.array[count] = strtok(string, " |><&;\t");
	/*Continues to add tokens to the array until there are none left
	  and Makes sure there are no null tokens at the end of the array */

	while (tokens.array[count] != NULL){
		count++;
		tokens.array[count] = strtok(NULL, " |><&;\t");

	}

	//Puts token count into the tokens struct so it can be passed out
	tokens.num_indices = count;
	tokens.array[count] = NULL;
	return tokens;
}

void execute_command(Tokens tokens, List history, List aliases, int command_count) {
	//Check if command entered is an internal command, if not execute externally
	if(strcmp(*tokens.array, "setpath") == 0) {
		execute_set_path(tokens);

	} else if (strcmp(*tokens.array, "getpath") == 0) {
		execute_get_path(tokens);

	} else if (strcmp(*tokens.array, "cd") == 0) {
		execute_change_directory(tokens);

	} else if(strcmp(*tokens.array, "history") == 0) {
		execute_print_history(tokens, history);

	} else if(strncmp(*tokens.array, "!", 1) == 0) {
	    execute_history_invocation(tokens, history, aliases, command_count);

	} else if(strcmp(*tokens.array, "alias") == 0) {
		 execute_add_alias(tokens, aliases);

	} else if(strcmp(*tokens.array, "unalias") == 0) {
		 remove_alias(tokens, aliases);

	} else {
		execute_external_command(tokens);

	}
}

void print_alias(List aliases) {
	char *result;
	int name_index;
	int num_aliases = size(aliases);
	if(num_aliases == 0){
	    printf("There are no aliases saved \n");
	    return;
	}
	//this for loops indexes through the aliases and splits them up to print
	for(int i = 0; i < num_aliases; i++) {
		result = get_at(aliases, i);
		//strcspn gets the number of characters in the result before a space (e.g. the first token)
		name_index = (int) strcspn(result, " ");
		printf("'%.*s' = '%s'\n", name_index, result, result + name_index + 1);
	}
}

Boolean check_duplicate_aliases(Tokens tokens, List aliases){

	for(int i = 0; i < size(aliases); i++){
		//this if compares the second value of the token to the value of an alias up until the size 
		//of tokens.array[1] to ensure it does not compare the token to the full value of alias
		//and only compares the existing alias name to the name entered by user
		if(strncmp(tokens.array[1], get_at(aliases, i), strlen(tokens.array[1])) == 0 ){
			//if alias is found it is removed
		    remove_at(aliases, i);
		    return true;
		}
	}
	return false;
}

void execute_add_alias(Tokens tokens, List aliases) {

	//if alias is the only token print_alias is called.
	if(tokens.num_indices == 1){
	    print_alias(aliases);
	} else {
		//checks if this alias name exists and overwrites the alias
        if(check_duplicate_aliases(tokens, aliases) == true) {
            printf("alias \"%s\" has been overwritten\n", tokens.array[1]);
        }
        // this part of the code pieces the tokens together as a string to save them into the file
	    char name[MAX_CHARACTERS];
        //uses the second value of token which would be the alias name and
        //saves it into a temp variable
	    strcpy(name,tokens.array[1]);
       
	    //saves the rest of the value of the token
	    //also into a temp variable with spaces inbetween
	    for(int i = 2 ; i < tokens.num_indices; i++){
            strcat(name, " ");
            strcat(name,tokens.array[i]);
        }

		//this statement checks that there is the minimum amount of tokens in order
		//to add the alias
        if(tokens.num_indices >= 3){
            add_alias(name, aliases);

        } else {
            printf("Too few arguments for add alias, use: \"alias\" //<name> <command> \n");

        }
    }
}

//this method adds an alias and checks if there is space for new aliases
void add_alias(char* command, List aliases) {
	if(size(aliases) == MAX_ALIASES){
	    printf("You have reached the maximum capacity of aliases allowed, if you wish to add another alias, remove an alias that already exists\n");
    	return;
	}
	printf("alias \"%.*s\" is set\n", (int) strcspn(command, " ") , command);
	push(aliases, command);

}

//this method removes an alias by checking for a duplicate and using the check_duplicate_alias method to remove the alias
// it also checks if there is too few arguments or non-existent aliases
void remove_alias(Tokens tokens, List aliases) {
	if (is_empty(aliases)){
	    printf("There are no aliases stored\n");
	} else if(tokens.num_indices == 1){
	    printf("Too few arguments for unalias, use: \"unalias\" <alias name> \n");
	} else if(tokens.num_indices > 2) {
		printf("Too many arguments for unalias, use: \"unalias\" <alias name>\n");
	} else if(check_duplicate_aliases(tokens, aliases) == true){
        printf("Alias \"%s\" has been removed\n", tokens.array[1]);
	} else {
    	 printf("Alias \"%s\" does not exist\n", tokens.array[1]);
	}
}


void print_tokens(Tokens tokens) {
	for(int i = 0; i < tokens.num_indices; i++) {
		printf("\"%s\"\n", tokens.array[i]);
	}
}

void execute_set_path(Tokens tokens) {
	//Check arguments for setpath and handle appropriately
	if(tokens.num_indices == 2) {
		if(set_path(tokens.array[1]) == true) {
			printf("Path changed to %s\n", tokens.array[1]);

		} else {
			perror("Path error");

		}
	} else {
		printf("Incorrect number of arguments for setpath, use: \"setpath [directory]\"\n");

	}
}

Boolean set_path(char *directory) {
	setenv("PATH", directory, 1);
	if(strcmp(getenv("PATH"), directory) == 0) {
		return true;
	} else {
		return false;
	}
}

void execute_get_path(Tokens tokens){
	if(tokens.num_indices == 1){
		char *check = getenv("PATH");
		printf("%s\n",check);
	} else{
		printf("Too many arguments for getpath, use: \"getpath\"\n");

	}
}


void execute_change_directory(Tokens tokens) {
	if(tokens.num_indices <= 2){
		change_directory(tokens.array[1]);

	} else {
		printf("Too many arguments for cd, use: \"cd\" for home dir or \"cd [directory]\"\n");

	}
}

void change_directory(char* directory) {
	if(directory == NULL) {
		//if directory is null, then command is "cd" -> change to home dir
		chdir(getenv("HOME"));
	} else {
		DIR *check = opendir(directory);
		if (check) {
			closedir(check);
			chdir(directory);
		} else {
			//Construct an informative error message for cd
			char *error_msg = malloc(MAX_CHARACTERS*sizeof(char*));
			sprintf(error_msg,"Can't access directory \"%s\"", directory);
			perror(error_msg);
			free(error_msg);
		}
	}
}

void execute_history_invocation(Tokens tokens, List history, List aliases, int command_count) {
    long index;
    char *bin;
    /* NOTE: the current command will have been added to the history
     * before this function was called, therefore always decrement the
     * index before invoking history */
    if(tokens.num_indices < 2) {
        //if there's no commands, don't bother executing
        if(command_count < 1) {
            printf("No history stored\n");
            return;
        }
        //Get index from command
        if(strcmp(tokens.array[0], "!!") == 0) {
            invoke_history(command_count - 1, history, aliases, command_count);
            return;
        } else if (tokens.array[0][1] == '-') {
            index = strtol(tokens.array[0]+2, &bin, 10);
            if(bin[0] != '\0' || index > size(history) || index < 1) {
                   printf("Invalid input for history invocation, please use integer between 1 and %d e.g. !-1\n", size(history));
            } else {
                invoke_history(command_count - (int) index, history, aliases, command_count);
            }
            return;
        } else if (tokens.array[0][1] >= '0' || tokens.array[0][1] <= '9') {
            index = strtol(tokens.array[0]+1, &bin, 10);
            if(bin[0] != '\0' || index > command_count) {
                printf("Invalid input for history invocation, please use integer between %d and %d e.g. !%d\n",(command_count - size(history)) + 1, command_count, command_count);
            } else {
                invoke_history((int) index - 1, history, aliases, command_count);
            }
            return;
        }
    }
    printf("Too many arguments for history invocation\n");

}

void invoke_history(int index, List history, List aliases, int command_count) {
    int hist_size = size(history);
    index = index - command_count + hist_size;
    if(index >= hist_size || index < 0){
         printf("Invalid input for history invocation, please use integer between %d and %d\n",(command_count - hist_size) + 1, command_count);
        return;
    } else {
        char command[MAX_CHARACTERS + 1];
        strcpy(command, get_at(history, index));
        //shift is the digits of command_count trim the number off of the string
        int shift;
        /*floor(log10(command_count)) + 1 returns the number of digits 
         *in the number e.g. log10(9) = 0.95, floor(0.95) + 1 = 1 */
        shift = (int) floor(log10(command_count)) + 1;
        //
        char expanded_input[MAX_CHARACTERS + 1] = {0};
        //If replace aliases returns false something has went wrong, prompt user again
        if(replace_aliases(command + shift, expanded_input, aliases) == false) {
            return;
        }
        Tokens tokens = tokenize_string(expanded_input);
        /*history invocations should never reach history, but if users specifically try and break
         *history with aliases then it is possible with this check*/
        if (tokens.num_indices > 0 && tokens.array[0][0] == '!') {
            printf("Recursive history invocation detected!\n");
        } else {
            execute_command(tokens, history, aliases, command_count);
        }
    }
}

void execute_external_command(Tokens tokens) {
	pid_t child_pid = fork();
	if(child_pid < 0) {
		//something went wrong with the fork
		perror("\nsomething went wrong, please try again\n");
	} else if (child_pid == 0) { 
		//fork was successful, should now be in child process

		/*executes the command entered by the user in the new process
		 *if the process is a valid command, process will end itself
		 *naturally, if not shell must handle it */
		execvp(*tokens.array, tokens.array);

		/*If we get to this point, the command entered was not valid
		 and we will still be inside the child process
		 -> exit child process*/
		perror(*tokens.array);
		exit(-1);

	} else {
		//parent should wait for child to complete
		while(wait(NULL) != -1);
	}
}

void print_history(List history) {
	char *result;
	int n = size(history);
	if(n == 0) {
	    printf("No history\n");
	}
	for(int i = 1; i<=n; i++) {
		result = get_at(history, i-1);
		printf("%s\n", result);
	}
}

void execute_print_history(Tokens tokens, List history) {
	if(tokens.num_indices == 1) {
		print_history(history);
	} else {
	    printf("Too many arguments for history\n");
	}
}

void save_history(List history){
	if(save_list(history, "shell.hist_list") == -1) {
		perror("Warning: recent history not saved");
	}
}

void load_history(List *history, int *command_count) {
	*history = load_list("shell.hist_list");
	//If null, file doesn't exist => start new list
	if(*history == NULL) {
		*history = new_list();
		*command_count = 0;
		return;
	}
	//If a list exists, get the command count from the latest command in the list
    char *bin;
    *command_count = (int) strtol(get_at(*history, size(*history) - 1 ), &bin, 10);
    if(*command_count < 1) {
        /*As the list was not NULL, there should be a command count therefore there must be something wrong
         * with the list => warn user and reset history */
         printf("Warning: error loading history, possible corruption, history has been reset!\n");
        *history = new_list();
    }
}

void save_aliases(List aliases) {
	if(save_list(aliases, "shell.aliases") == -1) {
		perror("Warning: recent aliases not saved");
	}
}

void load_aliases(List *aliases) {
	*aliases = load_list("shell.aliases");
	//If null, file doesn't exist => start new list
	if(*aliases == NULL) {
		*aliases = new_list();
	}
}

void add_history(char* command, List *history, int *command_count) {
    //If command is a history invocation, do not add to history
    if(command[0] == '!') {
        return;
    }
    int history_size = size(*history);
    //shift is the digits of command_count + 1 to trim the number off of the string
    int shift;
    if(*command_count > 0) {
        /*floor(log10(command_count)) + 1 returns the number of digits 
         *in the number e.g. log10(9) = 0.95, floor(0.95) + 1 = 1 */
        shift = (int) floor(log10(*command_count)) + 1;
        //if command is a copy of the previous history entry, do not add to history
        char *previous_command = get_at(*history, history_size - 1);
        if (strcmp(previous_command + shift + 1, command) == 0) {
            return;
        }
    }
    (*command_count)++;
    //If history exceeds the maximum size, then remove oldest value
    if(history_size == MAX_HISTORY){
        rem(*history);
    }
    char *history_entry = malloc(strlen(command) + shift);
    sprintf(history_entry,"%d %s", *command_count, command);
	push(*history, history_entry);
	free(history_entry);
}
