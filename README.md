# Simple linux shell project for CS210 

### Authors: 

###### Steven Muirhead
###### Lewis Beveridge
###### Ivaylo Ivanov 
###### Alejandra Delgado
###### Graham Aim 

## How to Compile: 
    gcc -pedantic -Wall shell.c list.c -o shell -lm 
    
    -lm is important as it links the math library; the shell uses some math functions and will not compile without it

# Functionality
    The simple shell has lots of functionality including: persistent history and aliases, execution of
    of external commands that are located in the path and built in commands which are described below.
## Built in Commmands: 
### **getpath**: used to get the current path
#### Usage: 
    getpath: returns the current path variable
### **setpath**: used to set the current path
#### Usage: 
    setpath [new directory]: sets the path variable to the path specified by [new directory]
### **cd**: used to set the current directory
#### Usage: 
    cd: sets the directory to the users home
    cd [new directory]: sets the directory to the directory specified by [new directory] assumign it's valid
### **alias**: used to create and view aliases
#### Usage
    alias: prints current aliases
    alias [name] [command]: creates an alias which will replace [name] with [command] when [name] is found in users input
### **unalias**: used to remove aliases
#### Usage:
    unalias [alias]: removes the alias with the name specified by [alias]
