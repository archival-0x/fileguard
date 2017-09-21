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
#include <yaml.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <libnotify/notify.h>
#include <sys/inotify.h>
#include <glib.h>

#define CONFIG_FILE "watchman.yaml"
#define BUF_LEN (10 * (sizeof(struct inotify_event) + NAME_MAX + 1))
#define DEFAULT_FILENAME "/root/watchman.log"

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
  char * data;
}; 

// Function: Used to read a file and file's data
struct file file_check(char * filename);

// Function: Used to create a file. Return in the form of flag and data.
struct file create_file(char * filename, char *data);

// Function: Checks if user has proper permissions to watch an inode, return inode number
int check_inode_permissions(char * inode_name);

// Function: Outputs inotify event to terminal and libnotify (desktop)
NotifyNotification raise_notification(const char * timeinfo, const char *event);

// Function: Parses inotify event into uint32_t
uint32_t parse_event(char * event);

// Function: Print specific inotify event in the case of IN_ALL_EVENTS
const char * display_event(struct inotify_event *i);

// Function: Get local time
struct tm * gettime(time_t rawtime);

// Function: Used to check if YAML configuration has been written correctly.
struct YAML parse_yaml_config(char * filename);

#endif