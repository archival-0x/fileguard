#ifndef _WATCHMAN_H
#define _WATCHMAN_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <signal.h>
#include <stdbool.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/inotify.h>

#include <yaml.h>
#include <glib.h>
#include <libnotify/notify.h>

#define CONFIG_FILE         "watchman.yml"
#define BUF_LEN             (10 * (sizeof(struct inotify_event) + NAME_MAX + 1))
#define DEFAULT_FILENAME    "watchman.log"

/*
 * yaml_t struct: used to define key values that are to be returned when
 * parsing a standard watchman configuration file 
*/
struct 
yaml_t 
{
  char * inode;     /* inode name */
  char * event;     /* event to watch for */
  char * execute;   /* command of execution */
  bool return_flag; /* what's being returned */
};

/* 
 * file_t struct: defines the return types of the file when performing File I/O
 * checking operations
*/
struct 
file_t 
{
  int flag;
  char * data;
};

typedef struct yaml_t yaml_t;
typedef struct file_t file_t;

/* checks if user has proper permissions to watch an inode, and if so, return inode number */
int check_inode_permissions(char * inode_name);

/* creates a new file and return a file_t type describing file info */
file_t create_file(char * filename, char *data);

// Function: Used to read a file and file's data
file_t file_check(char * filename);

// Function: Outputs inotify event to terminal and libnotify (desktop)
NotifyNotification raise_notification(const char * timeinfo, const char *event);

// Function: Parses inotify event into uint32_t
uint32_t parse_event(char * event);

// Function: Print specific inotify event in the case of IN_ALL_EVENTS
const char * display_event(struct inotify_event *i);

// Function: Get local time
struct tm * gettime(time_t rawtime);

// Function: Used to check if YAML configuration has been written correctly.
yaml_t parse_yaml_config(char * filename);

// Function: Used to turn char into array for execvp()
void parse_execute(char * line, char ** argv);

// Function: Execute command with fork() and execvp()
int execute_command(char **argv);

#endif
