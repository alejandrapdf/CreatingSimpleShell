/* list.c */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "list.h"

//Utility method to return the node at an index, returns NULL if unsuccessful
Node *get_node(List list, int index);

Node * new_node(char* string){
	char *copy = malloc(strlen(string) * sizeof(char) + 1);
	strcpy(copy, string);
	//Create enough memory space for the size of the string 
    Node *new_node = malloc(sizeof(Node*));
    new_node->value = copy;
    new_node->next = NULL;
    return new_node;
}

void print_list(List list) {
    if(is_empty(list)){
        printf("list is empty\n");
    } else {
        Node *current = *list;
        printf("[");
        do {
            printf("%s, ", current->value);
            current = current->next;
        } while(current != NULL);
        printf("]\n");
    }
}

void delete_node(Node *node){
	free(node->value);
    free(node);
    node=NULL;
}

void push(List list, char* new_value){
    Node *first = *list;
    Node *new = new_node(new_value);
    if(is_empty(list)){
        *list=new;
        return;
    }
    Node *last=first;
  	while(last->next!=NULL){
        last=last->next;
   	}
    last->next=new;
}

void add(List list, char* new_value){
    Node *current_head=*list;
    Node *new = new_node(new_value);
    new->next=current_head;
    *list=new;
}

List new_list(){
    List list = malloc(sizeof(Node));
    *list=NULL;
    return list;
}


Boolean is_empty(List list){
    if(*list==NULL)
        return true;
    return false;
}

int size(List list){
    if(is_empty(list))
        return 0;
    Node *current = *list;
    int size=1;
    while(current->next!=NULL){
        current=current->next;
        size++;
    }
    return size;
}

void clear(List list){
    while(!is_empty(list)){
        rem(list);
    }
}

char *rem(List list) {
	if(is_empty(list))
		return NULL;
	char *result = (*list)->value;
	Node *next = (*list) -> next;
	delete_node(*list);
	*list = next;
	return result;
}

int index_of(List list, const char *string){
    if(is_empty(list))
        return -1;
    Node *current = *list;
    int n = 0;
    while(current->next != NULL){
//        if(current->value == string)
        if(strcmp(current->value, string) == 0)
            return n;
        n++;
        current = current->next;
    }
    if(current->value == string)
        return n;
    return -1;
}

Boolean contains(List list, char* string){
    if(is_empty(list))
        return false;
    return (index_of(list, string) != -1) ? true : false;
}

Node *get_node(List list, int index){
    if(is_empty(list))
        return NULL;
    if(index<0 || index >= size(list))
        return NULL;
    Node *current = *list;
    for(int i = 0; i < index; i++){
        current=current->next;
    }
    return current;
}

char* get_at(List list, int index){
    Node *node = get_node(list,index);
    if(node!=NULL)
        return node->value;
    return NULL;
}

char* remove_at(List list, int index){
    if(index >= size(list) || index < 0)
        return NULL;
    if(index == 0)
        return rem(list);
    Node *prev = get_node(list, index - 1);
    Node *target = prev->next;
    Node *next = target->next;
    prev->next = next;
    char *result = target->value;
    free(target);
    return result;
}

int save_list(List list, char *file_name){
    char *file_location=  malloc(sizeof(getenv("HOME")) + sizeof(file_name) + 10);
    strcpy(file_location, getenv("HOME"));
    strcat(file_location, "/.");
    strcat(file_location, file_name);
    FILE *file = fopen(file_location,"w");
    if(file == NULL)
        return -1;
    int n = 0;
    //Print size of list at top of file to help with loading
    fprintf(file, "size: %d\n",size(list));
    for(int i = 0; i < size(list); i++){
        fprintf(file, "%s\n",get_node(list, i)->value);
        n++;
    }
    fclose(file);
    free(file_location);
    return n;
}

List load_list(char *file_name){
    char *file_location = malloc(sizeof(getenv("HOME"))+sizeof(file_name)+10);
    strcpy(file_location, getenv("HOME"));
    strcat(file_location, "/.");
    strcat(file_location,file_name);
    FILE *file = fopen(file_location,"r");
    if(file == NULL)
        return NULL;
    List list = new_list();
    long num_lines;
    char *bin;
    char first_line[MAX_CHARACTERS + 1];
    fscanf(file, "size: %s\n", first_line);
    num_lines = strtol(first_line, &bin, 10);
    if((int) num_lines == 0) {
        return NULL;
    }
    for(int i = 2; i <= num_lines + 1; i++){
        char string[MAX_CHARACTERS + 2];
        fgets(string, MAX_CHARACTERS + 1,file);
        char *copy=malloc(strlen(string) + 1);
        strcpy(copy, string);
        if(copy[strlen(copy) - 1] == '\n')
            copy[strlen(copy) - 1] = '\0';
        if(copy[0] == '\n')
            copy = "";
        push(list,copy);
    }
    fclose(file);
    free(file_location);
    return list;
}
