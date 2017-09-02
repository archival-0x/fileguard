#ifndef _WATCHMAN_H

#define _WATCHMAN_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <yaml.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <sys/stat.h>

#define CONFIG_FILE "watchman.yaml"


// Yaml struct: Used to define key values that are to be returned when parsing
// a standard watchman configuration file
struct YAML {
  char * inode;
  char * event;
  char * execute;
  bool return_flag;
};

// File struct: defines the return types of the file when performing File I/O
// checking operations
struct file {
  int flag;
  FILE * fpointer;
  char * data;
}; 

// Function: Used to read a file and file's data
struct file file_check(char * filename);

// Function: Checks if user has proper permissions to watch an inode, return inode number
int check_inode_permissions(char * inode_name);

// Function: Creates a watcher on specified file
void create_inode_watcher();

// Function: Used to check if YAML configuration has been written correctly.
struct YAML parse_yaml_config(char arg, FILE *fptr);

#endif