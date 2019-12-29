#ifndef _FILEGUARD_H
#define _FILEGUARD_H

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

#define CONFIG_FILE         "fileguard.yml"
#define BUF_LEN             (10 * (sizeof(struct inotify_event) + NAME_MAX + 1))
#define DEFAULT_FILENAME    "fileguard.log"

/* yaml_t struct: used to define key values that are to be returned when
 * parsing a standard fileguard configuration file
 */
typedef struct
{
  char * inode;         /* inode name */
  char * event;         /* event to watch for */
  char * action;        /* action of execution */
  bool return_flag;     /* what's being returned */
} yaml_t;


/* file_t struct: defines the return types of the file when performing File I/O
 * checking operations
 */
typedef struct
{
  int flag;
  char * data;
} file_t;


/* check if we have proper permission to access inode */
int check_inode_permissions(char * inode_name);

/* fast file creation */
file_t create_file(char * filename, char *data);

/* check if file exists */
file_t file_check(char * filename);

/* raise libnotify notification */
void raise_notification(const char * timeinfo, const char *event);

/* parse event correctly and return value*/
uint32_t parse_event(char * event);

/* get string of the event being caught */
const char * get_event(struct inotify_event *i);

/* time retrieval helper */
struct tm * gettime(time_t rawtime);

/* parser for YAML configuration */
yaml_t parse_yaml_config(char * filename);

#endif
