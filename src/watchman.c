#include "watchman.h"

int check_inode_permissions(char * inode_name){
  struct stat file_stat;
  int fd, ac; 
  
  // Check if user can access inode through read and write
  ac = access(inode_name, R_OK | W_OK);
  if( !(ac < 0) ){
    
    // Open for reading.
    fd = open(inode_name, O_RDONLY , 0644 );
    
    // Grab file_stats for log
    fstat (fd, &file_stat); 
    
    return file_stat.st_ino;
  } else {
    return ac; // -1
  }

}

// create_inode_watcher will be running the entirety of the application.
// therefore, this function is crucial and must create and cleanup
void create_inode_watcher(char * event, char * inode, int fd){
	
  struct inotify_event *ev;
	
  // Construct unsigned mask from event string for add_watch
  uint32_t mask = atoi(event);
  NotifyNotification *notification;

  //  ADD A FILE WATCHHHHH!
  int wd = inotify_add_watch(fd, inode, mask);
  if ( wd < 0) { 
    perror("Could not add watch. Reason");
  }
  
  
  // Cleanup and remove the watch.
  inotify_rm_watch(fd, wd);
  
}

NotifyNotification raise_notification(){
  
  // TODO: create notify, show it, and close it. 
  
  notify_notification_new();
  notify_notification_show();
  notify_notification_close ();
}

struct file file_check(char * filename){

  struct file f;
  int fd, len;
  void *data;
  
  // POSIX-style file-opening. Better for performance, but repetitive.
  fd = open(filename, O_RDONLY, 0644);
  // If fd is negative, return error
  if (fd < 0 ) { f.data = strerror(errno); f.flag = fd; return f;}
  len = lseek(fd, 0, SEEK_END);
  data = mmap(0, len, PROT_READ, MAP_PRIVATE, fd, 0);
  
  f.flag = fd; f.data = data;
  
  return f;
}

struct file create_file(char * filename){
  struct file f;
  
  char *path = malloc(strlen(filename) + 1 );
  path = strcpy(path, filename);

  int fd = open(path, O_RDWR | O_APPEND | O_CREAT);   
  if ( fd < 0) { 
    f.flag = fd; f.data = strerror(errno); return f;
  }
  
  f.flag = fd; f.data = filename;
  free(path);
  return f;
}

struct YAML parse_yaml_config(char * filename){
  
   yaml_parser_t parser;
   yaml_token_t  token;
   
   // Declare variables for tokenizer
   // Credit: https://stackoverflow.com/questions/20628099/parsing-yaml-to-values-with-libyaml-in-c
   int state = 0;
   char ** datap;
   char *tk;
   
   FILE *fptr = fopen(filename, "r");
   
   // Create a struct for yaml config parsing.
   struct YAML config;
   
   // Throw error if parser fails to initialize
   if(!yaml_parser_initialize(&parser)){
     config.return_flag = false;
     return config; 
   }
   // Throw error if file pointer returns a NULL
   if(fptr == NULL){
     config.return_flag = false;
     return config;
   }
   
   // Set input file
   yaml_parser_set_input_file(&parser, fptr);  
    
  do {
     // Starting parsing through token method
     yaml_parser_scan(&parser, &token);
     
     // Check key values against the yml struct.
     switch (token.type){
       case YAML_KEY_TOKEN : 
          state = 0; break;
       case YAML_VALUE_TOKEN :
          state = 1; break;
       case YAML_SCALAR_TOKEN :
          tk = token.data.scalar.value;
          if (state == 0) {
           if (!strcmp(tk, "inode")) {
             datap = &config.inode;
           } else if (!strcmp(tk, "event")) {
             datap = &config.event;
           } else if (!strcmp(tk, "execute")) {
             datap = &config.execute;
           } else {
             config.return_flag = false;
             return config;
           }
         } else {
           *datap = strdup(tk);
         }
         break;
     default: break;
     }
   } while (token.type != YAML_STREAM_END_TOKEN);
   
   // Call the janitor.
   yaml_token_delete(&token);
   yaml_parser_delete(&parser);
   fclose(fptr);
   
   config.return_flag = true;
   return config;
}