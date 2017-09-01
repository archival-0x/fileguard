#ifndef WATCHMAN_H

#define WATCHMAN_H

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

#define CONFIG_FILE "watchman.yaml"

// Events enum: Used to define all inotify events. Good for iteratively checking against
// config file.
typedef enum
{
   IN_ACCESS,             // File accessed
   IN_ATTRIB,             // Metadata changes
   IN_CLOSE_WRITE,        // File opened for writing was closed.
   IN_CLOSE_NOWRITE,      // File or directory not opened for writing was closed.
   IN_CREATE,             // File/directory created
   IN_DELETE,             // File/directory deleted
   IN_DELETE_SELF,        // Watched inode deleted
   IN_MODIFY,             // File modified
   IN_MOVE_SELF,          // Watched inode moved
   IN_MOVED_FROM,         // Directory with old filename when a file is renamed.
   IN_MOVED_TO,           // Directory with new filename when a file is renamed.
   IN_UNMOUNT,            // Filesystem unmounted
   IN_ALL_EVENTS,         // In all events (except IN_UNMOUNT)
} Events;

typedef struct {
  char * inode;
  char * event;
  char * execute;
  bool return_flag;
} Yaml;

struct file {
  int flag;
  FILE * fpointer;
  char * data;
}; 

// Function: Used to read a file and file's data
struct file file_check(char * filename);

// Function: Checks if user has proper permissions to watch an inode
bool check_inode_permissions(/* TODO: Parameters??? */);

// Function: Used to check if YAML configuration has been written correctly.
Yaml parse_yaml_config(char arg, FILE *fptr);

#endif