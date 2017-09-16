#include "watchman.h"
#include "colorize.h"


// events array: we take advantage of this to iteratively check
// if a user-specified inode event is within our capabilities
const char * events [] = 
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
    struct inotify_event *event;
    
    int c, in, inode_i; // Return types, etc.
    errno = 0; // Set errno flag to 0
    
    gboolean nint;
    
    while ((c = getopt (argc, argv, "chd")) != -1)
    
    switch (c){
      ////////////////////////////////////////////////////////////////////////
      case 'h':
        fprintf(stdout, "Usage: (note that these are optional arguments)\n\t %s -[c|h|d]\n\n"
                "-c : Perform a configuration check on the YAML config file\n"
                "-d : Delete inode watchers.\n"
                "-h : Display this help message\n", argv[0]);
        break;    
      default:
        return 0;
    }
    
    printf("STARTING WATCHMAN!\n");
    
    // Initialize inotify
    in = inotify_init();
    if ( in < 0) perror("Could not initialize inotify. Reason");
    
    // Initialize libnotify
    nint = notify_init("Watchman");
    if (nint == FALSE) perror("Could not initialize libnotify. Reason");
    
    // First, check the file.
    yaml = file_check(CONFIG_FILE);
    if ( yaml.flag < 0){
      // Create an empty config file, then throw error
      struct file new_file;
      new_file = create_file(CONFIG_FILE);
      
      // No perror, since we got back a errno string.
      fprintf(stderr, "Error %i: Unable to open file: %s\n", yaml.flag, yaml.data);
    } else {
      printf("File successfully found and opened!\n");
    }
    
    // Next, check the YAML key-value validity.
    y = parse_yaml_config(CONFIG_FILE);
          
    if ( y.return_flag == false){
      perror("Could not initialize YAML parser. Reason");
    } else if ( y.return_flag == true ){
      printf("Successfully parsed YAML file.\n"
             "\tinode: %s\n\tevent: %s\n\texecute: %s\n",
            y.inode, y.event, y.execute);
    }
            
    // Iterate through events[] array to check if inode specified
    // is specified. e_flag is set to see if comparisions are valid. If all
    // are not, last instance of e_flag remains 1, printing to stderr.
    int e_flag;
    for ( int i = 0; events[i] != NULL; i++){
        if (strcmp(y.event, events[i]) == 0){
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
   
    int x_flag; // Set a flag to dictate success failure 
    char *prepend, *command, *str, *mem; // set strings as tokens
    char *sep = " "; // seperator for tokenizing
    mem = str = strdup(y.execute); // allocate memory, also create a copy
    
    // prepend represents the first part of the command string.
    // We will compare this with the execute_action array.
    prepend = strtok(str, sep);
    
    // This will represent the actual command the user wants to execute
    command = strtok(NULL, "\"");
      
    for ( int i = 0; execute_action[i] != NULL; i++){
      if (strcmp(prepend, execute_action[i]) == 0){
        x_flag = 0; break;
      } else {
        x_flag = 1; continue;
      }
    }
    
    if( x_flag == 1 ) fprintf(stderr, "Unknown command supplied: %s\n", y.execute);
    else if (x_flag == 0) printf("Command found! Continuing.\n");
    
    // Use a standard file_check to check for existence of inode.
    // We will only be using this to treat the inode as a standard file/directory.
    // Much larger operations will be executed during actual watcher creation.
    inode_check = file_check(y.inode);
    if ( inode_check.flag < 0){
      // No perror, since we got back a errno string.
      fprintf(stderr, "Error %i: Unable to open inode \"%s\": %s\n", inode_check.flag, y.inode, inode_check.data);
    } else {
      printf("inode successfully found and opened!\n");
      // Check for permission
      inode_i = check_inode_permissions(y.inode);
      if (inode_i < 0) { perror("Permission check for inode failed! Reason"); }
    }
    
    // Passes: event, inode, fd
    create_inode_watcher(y.event, y.inode, in);
     
   // Cleanup, cleanup, everybody everywhere.
   free(mem);
   notify_uninit();
   
   
    return 0;
}
