#ifndef MAIN_H
#define MAIN_H

#include "watchman.h"

/* represents an array filled with events for watching */
const char *
events[] =
{
   "IN_ACCESS",             // File accessed
   "IN_ATTRIB",             // Metadata changes
   "IN_CLOSE_WRITE",        // File opened for writing was closed.
   "IN_CLOSE_NOWRITE",      // File or directory not opened for writing was closed.
   "IN_CREATE",             // File/directory created
   "IN_DELETE",             // File/directory deleted
   "IN_DELETE_SELF",        // Watched inode deleted
   "IN_MODIFY",             // File modified
   "IN_MOVE_SELF",          // Watched inode moved
   "IN_MOVED_FROM",         // Directory with old filename when a file is renamed.
   "IN_MOVED_TO",           // Directory with new filename when a file is renamed.
   "IN_OPEN",               // File/directory is opened
   "IN_UNMOUNT",            // Filesystem unmounted
   "IN_ALL_EVENTS",         // In all events (except IN_UNMOUNT)
};

/* array of commands to compare to when parsing yaml */
const char *
execute_action[] =
{
    "kill",
    "start",
    "restart",
    "execute",
    "log",
};


/* 
 * global signal counter variable, with sig_atomic_t typedef 
 * for initialization and signal handling
 */
volatile sig_atomic_t sc = true;

/*
 * global file descriptor and watch descriptor for initialization and
 * cleanup
 */
int fd, wd;

/* 
 * global chunk of unallocated memory for both initialization and
 * cleanup
 */
char * mem;


#endif
