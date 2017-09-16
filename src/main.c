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
   "IN_OPEN",               // File/directory is opened
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

// Global signal counter variable
unsigned int sc = 0;

// catch_sig() is made local to program to interact with sc counter.
void catch_sig(int s){
   printf("Caught signal %d!\n", s);
   sc += s;
}


int main( int argc, char **argv )
{   
    
    struct file yaml, inode_check;                    // target file struct for yaml config
    struct YAML y;                                    // return tokenized yaml
    struct inotify_event *ev;                         // inotify_event struct
    
    uint32_t mask;                                    // convert event -> unsigned 32 int
    
    char buf[BUF_LEN] __attribute__ ((aligned(8)));   // buffer for events
    char *p;                                          // char to check against buf when checking for events
    char *prepend, *command, *str, *mem;              // set yaml tokens into readable strings
    char *sep = " ";                                  // seperator for tokenizing
    
    int fd, wd, rd;                                   // return values for file descriptors
    int c, in, inode_i;                               // various return types when checking
    int x_flag, e_flag;                               // flags for checking against global arrays
    
    NotifyNotification *notification;                 // libnotify instance
    errno = 0;                                        // Set errno flag to 0



    while ((c = getopt (argc, argv, "h")) != -1)
    switch (c){
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
    
    // First, check the file.
    yaml = file_check(CONFIG_FILE);
    if ( yaml.flag < 0){
      
      // No perror, since we got back a errno string.
      fprintf(stderr, "Error %i: Unable to open file: %s\n", yaml.flag, yaml.data);
      
      // Create an empty config file, then quit
      printf("Creating configuration file for you...\n");
      struct file new_file = create_file(CONFIG_FILE, NULL);
      exit(EXIT_FAILURE);
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
    for ( int i = 0; events[i] != NULL; i++){
        if (strcmp(y.event, events[i]) == 0){
          e_flag = 0; break; // Break. Event has been matched
        } else {
          e_flag = 1; continue; // Set e_flag and continue until end of iterator
        }
    }
    // If e_flag retains 1, then the event isn't found
    if( e_flag == 1 ) {
      fprintf(stderr, "\nUnknown inode event supplied: %s\n", y.event);
      exit(EXIT_FAILURE);
    }
    else if ( e_flag == 0 ) printf("\ninode event found! Continuing.\n");
    
    // Iterate through execute_action to check if execute action matches.
    // However, we need to first slice the value.
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
    
    if( x_flag == 1 ) {
      fprintf(stderr, "Unknown command supplied: %s\n", y.execute);
      exit(EXIT_FAILURE);
    }
    else if (x_flag == 0) printf("Command found! Continuing.\n");
    
    // Use a standard file_check to check for existence of inode.
    // We will only be using this to treat the inode as a standard file/directory.
    // Much larger operations will be executed during actual watcher creation.
    inode_check = file_check(y.inode);
    if ( inode_check.flag < 0){
      // No perror, since we got back a errno string.
      fprintf(stderr, "Error %i: Unable to open inode \"%s\": %s\n", inode_check.flag, y.inode, inode_check.data);
      exit(EXIT_FAILURE);
    } else {
      printf("inode successfully found and opened!\n");
      // Check for permission
      inode_i = check_inode_permissions(y.inode);
      if (inode_i < 0) { perror("Permission check for inode failed! Reason"); }
    }
    
    // Let's finally create an inode_watcher!

    // Initialize inotify
    fd = inotify_init();
    if ( fd < 0) {
      perror("Could not initialize inotify. Reason");
    }
    
    // Construct unsigned mask from event string for inotify_add_watch
    mask = parse_event(y.event);
        
    // ADD A FILE WATCHHHHHAH!
    wd = inotify_add_watch(fd, y.inode, mask);
    if ( wd < 0) { 
      perror("Could not add watch. Reason");
    }
      
    // Starting looping and watching for events!
    // BREAK on interrupt!! Important so that program can finish execution
    // by freeing memory and removing watcher
    for (;;) {
      signal(SIGINT, catch_sig);

      if (sc != 0 ) break;
      
      rd = read(fd, buf, BUF_LEN);
      
      if (rd == 0) fprintf(stdout, "read() tossed back a 0");
      
      if (rd == 1 ) {
        perror("Couldn't read event. Reason");
        break;
      }

      for (p = buf; p < buf + rd; ) {
          ev = (struct inotify_event *) p;
          display_event(ev);
          
          if( prepend == "execute" ){          
            int ret = system(command);
            if (WIFSIGNALED(ret) && (WTERMSIG(ret) == SIGINT || WTERMSIG(ret) == SIGQUIT)){
              break;
            }
          } else if ( prepend == "log" ){
            printf("Creating log file\n");
            if (command == NULL) {
              command = DEFAULT_FILENAME;
            }
            struct file tmpLog = create_file(command, "test string");
            if (tmpLog.flag < 0 ){
              fprintf(stderr, "Couldn't create log file. Reason: %s\n", tmpLog.data);
              exit(EXIT_FAILURE);
            }
          }
          
          
          p += sizeof(struct inotify_event) + ev->len;
      }
    
    
    
    
    }
    
    
    // Cleanup, cleanup, everybody everywhere.
    inotify_rm_watch(fd, wd);
    free(mem);   
    return 0;
}
