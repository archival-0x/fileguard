#include "watchman.h"
#include "colorize.h"

#include <libnotify/notify.h>
#include <sys/inotify.h>
#include <glib.h>
#include <sched.h>

// inotify_events array: we take advantage of this to iteratively check
// if a user-specified inode event is within our capabilities
const char * inotify_events [] = 
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
   "IN_UNMOUNT",            // Filesystem unmounted
   "IN_ALL_EVENTS",         // In all events (except IN_UNMOUNT)
};

// execute_action array: this is a list of commands one can execute once inotify
// catches an event.
const char * execute_action[] = 
{
  // NOTE that kill, start, and restart all deal with processes and the
  // process scheduler. 
  "kill", "start", "restart", "execute", "log",  
};


int main( int argc, char **argv )
{    
    struct file yaml, inode_check; // target file struct for yaml config
    struct YAML y;
    
    int c, in, inode_ch; // Return types, etc.
        
    errno = 0; // Set errno flag to 0
    
    while ((c = getopt (argc, argv, "chd")) != -1)
    switch (c)
      {
      ////////////////////////////////////////////////////////////////////////
      case 'c':
        // Creating a check_yml object just for checking
        
        printf("Checking YAML config for inconsistencies...\n");
        
        // First, check the FILE.
        yaml = file_check(CONFIG_FILE);
        if ( yaml.flag < 0){
          // No perror, since we got back a errno string.
          fprintf(stderr, "Error %i: Unable to open file: %s\n", yaml.flag, yaml.data);
        } else {
          printf("File successfully found and opened!\n");
        }
        
        // Next, check the YAML key-value validity.
        // Pass a 'c' char to signify that we are only checking the file.
        y = parse_yaml_config('c', yaml.fpointer);
              
        if ( y.return_flag == false){
          perror("Could not initialize YAML parser. Reason");
        } else if ( y.return_flag == true ){
          printf("Successfully parsed YAML file.\n"
                 "\tinode: %s\n\tevent: %s\n\texecute: %s\n",
                y.inode, y.event, y.execute);
        }
                
        // Iterate through inotify_events[] array to check if inode specified
        // is specified. e_flag is set to see if comparisions are valid. If all
        // are not, last instance of e_flag remains 1, printing to stderr.
        int e_flag;
        for ( int i = 0; inotify_events[i] != NULL; i++){
            if (strcmp(y.event, inotify_events[i]) == 0){
              e_flag = 0; break; // Break. Event has been matched
            } else {
              e_flag = 1; continue; // Set e_flag and continue until end of iterator
            }
        }
        // If e_flag retains 1, then the event isn't found
        if( e_flag == 1 ) fprintf(stderr, "\nUnknown inode event supplied: %s\n", y.event);
        else if ( e_flag == 0 ) printf("\ninode event found! Continuing.\n");
        
       // Iterate through execute_action to check if execute action matches.
       // However, we need to first slice the value.
        char *token, *str, *mem;
        int x_flag;
        mem = str = strdup(y.execute);
        token = strsep(&str, " "); // get first part of string.
        for ( int i = 0; execute_action[i] != NULL; i++){
          if (strcmp(token, execute_action[i]) == 0){
            x_flag = 0; break;
          } else {
            x_flag = 1; continue;
          }
        }
        if( x_flag == 1 ) fprintf(stderr, "Unknown command supplied: %s\n", y.execute);
        else if (x_flag == 0) printf("Command found! Continuing.\n");
          
      /* Code reuse, this time we file check y.inode, making sure it exists.
         When we are actually executing the program, we will instead be calling
         upon another function, check_inode_permissions(), which we will actually
         use to return an inode number, using fstat and stat. */
        
        inode_check = file_check(y.inode);
        if ( inode_check.flag < 0){
          // No perror, since we got back a errno string.
          fprintf(stderr, "Error %i: Unable to open inode: %s\n", inode_check.flag, inode_check.data);
        } else {
          printf("inode successfully found and opened!\n");
        }
        
        // Cleanup, cleanup, everybody everywhere.            
        free(mem);
        exit(EXIT_SUCCESS);
        
      ////////////////////////////////////////////////////////////////////////
      case 'h':
        fprintf(stdout, "Usage: (note that these are optional arguments)\n\t %s -[c|h|d]\n\n"
                "-c : Perform a configuration check on the YAML config file\n"
                "-d : Delete inode watchers.\n"
                "-h : Display this help message\n", argv[0]);
        exit(EXIT_SUCCESS);
      
      ////////////////////////////////////////////////////////////////////////
      case 'd':
        
        // TODO: case to reset application or somethin
        exit(EXIT_SUCCESS);
      default:
        continue;
      }
  
    // Create a new colorize object
    // colorize * cv = cz_new(WHITE, "Starting watchman!", RESET, BOLD);
    // czprint(cv);
    
    /* Throughout this part of the code, we assume that the user has already
       has already called ./watchman -c in order to perform checking. We are
       not going to spend time and performance checking for unnecessary things,
       such as file existence, but actually moving on to creating watchers
       and calling notify and a process scheduler. */
    
    in = inotify_init();
    if ( in < 0) perror("Could not initialize inotify. Reason");
    
    FILE *tmptr = fopen(CONFIG_FILE, "r"); //hmmm, a lil repetitive...
    y = parse_yaml_config('e', tmptr);
    
    inode_ch = check_inode_permissions(y.inode);
    
    
    // Call the janitor.
    //czclean(cv);
    return 0;
}
