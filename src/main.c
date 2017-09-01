#include "watchman.h"
#include "colorize.h"

#include <libnotify/notify.h>
#include <sys/inotify.h>
#include <glib.h>
#include <sched.h>


int main( int argc, char **argv )
{    
    struct file yaml;
    int c, in;
    Yaml *y = malloc(sizeof(y));  
    // Set errno flag to 0
    errno = 0;
    
    while ((c = getopt (argc, argv, "chd")) != -1)
    switch (c)
      {
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
        *y = parse_yaml_config('c', yaml.fpointer);
              
        if ( y->return_flag == false){
          perror("Could not initialize YAML parser. Reason");
        } else if ( y->return_flag == true ){
          printf("Successfully parsed YAML file.\n"
                 "\tinode: %s\n\tevent: %s\n\texecute: %s\n",
                y->inode, y->event, y->execute);
        }
        
        // TODO: Create events enum to interate over and check against user-specified
        // inode.
                
        free(y); 
        exit(EXIT_SUCCESS);
      case 'h':
        fprintf(stdout, "Usage: (note that these are optional arguments)\n\t %s -[c|h|d]\n\n"
                "-c : Perform a configuration check on the YAML config file\n"
                "-d : Delete inode watchers.\n"
                "-h : Display this help message\n", argv[0]);
        exit(EXIT_SUCCESS);
      case 'd':
      
        exit(EXIT_SUCCESS);
      default:
        continue;
      }
  
    // Create a new colorize object
    colorize * cv = cz_new(WHITE, "Starting watchman!", RESET, BOLD);
    czprint(cv);
    
    //TODO: actually create a watcher.
    
    // Call the janitor.
    czclean(cv);
    free(y);
    return 0;
}
