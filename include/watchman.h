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

int check_inode_permissions(char * inode_name);
file_t create_file(char * filename, char *data);
file_t file_check(char * filename);
NotifyNotification raise_notification(const char * timeinfo, const char *event);
uint32_t parse_event(char * event);
const char * display_event(struct inotify_event *i);
struct tm * gettime(time_t rawtime);
yaml_t parse_yaml_config(char * filename);

#endif
