#define MAX_CHARACTERS 512
#define MAX_HISTORY 20
#define MAX_ALIASES 10

typedef struct NODE Node;
typedef Node** List;
typedef enum {true = 1, false = 0} Boolean; 

struct NODE{
    char *value;
    Node *next;
};

//Pushes a value onto the end of the linked list
void push(List list, char* new_value);


//Adds a value onto the start of the linked list
void add(List list, char* new_value);

//Creates a new list
List new_list();

//Prints out the values in the list
void print_list(List list);

//Returns 1 if list is empty, 0 if not
Boolean is_empty(List list);

//Returns number of elements in list
int size(List list);

//Removes all elements in list
void clear(List list);

//Returns the index of a value in list, if not on list return -1
int index_of(List list, const char *string);

//Determines if a value is in the list, returns 1 if yes, 0 if no
Boolean contains(List list, char* string);

//Returns the value at an index in list, return NULL if out of bounds
char* get_at(List list, int index);

//Replaces the value at an index in the list, return 1 for success, 0 for failure
int replace_at(List list, int index, char* string);

//Adds a node at the specified index, returns 1 for success, 0 for failure
int insert_at(List list, int index, char* string);

//Removes the node from the specified index, returns value of node, or NULL if failure
char* remove_at(List list, int index);

//Returns the value of the first node and removes it
char *rem(List list);

//Saves the list to the specified file name
int save_list(List list, char *file_name);

//Loads the list from program location
List load_list(char *file_name);
