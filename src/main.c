#include "main.h"


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


/*
 * displays help and usage
 */
static void 
usage(char * application_name)
{
    /* print help to STDOUT */
    fprintf(stdout, "Usage: (note that these are optional arguments)\n\t %s -[h|v] <other.yaml>\n\n"
            "-h : Display this help message\n"
            "-v : Turns ON verbosity\n"
            "-n : Turns ON libnotify notifications\n"
            ,application_name);
}
/*
 * cleanup routine run by atexit() in order to
 * safely clean up memory and close fds
 */
static void 
cleanup(void)
{
    /* remove watch on file descriptor */
    inotify_rm_watch(fd, wd);
}

/*
 * function routine that continously listens and catches
 * for user-supplied SIGINTs or SIGTERMs and exits
 */
static void 
catch_sig(int s)
{
    printf("Signal %i caught! Cleaning up...\n", s);
    sc = false; 
    cleanup();
    exit(EXIT_SUCCESS);
}



int 
main(int argc, char **argv)
{   
    file_t yaml, inode_check;                         /* target file struct for yaml config */
    yaml_t y;                                         /* returns a tokenized yaml */
    struct inotify_event *ev;                         /* represents a inotify_event struct */
    
    char buf[BUF_LEN] __attribute__ ((aligned(8)));   /* buffer that stores events */
    char *mem;                                        /* memory used for strdup allocation */
    char *p;                                          /* char to check against buf when checking for events */
    char *prepend, *command, *str;                    /* set yaml tokens into readable strings */
    char *sep = " ";                                  /* seperator for tokenizing */
    char *default_yaml = CONFIG_FILE;                 /* name of yaml file to check, parse, and tokenize */
    
    int verbose = 0;                                  /* flag for verbosity. 0 = false */
    int notifier = 0;                                 /* flag for libnotify. 0 = false */
    int rd;                                           /* return value for local file descriptors */
    int c, in, inode_i;                               /* various return values for error-checking */
    int x_flag, e_flag;                               /* flags for checking against global arrays */

    /* set errno to be 0 */
    errno = 0;
    
    /* when exit is called, also initialize cleanup routine */
    atexit(cleanup);

    /* signal handling for SIGINT and SIGTERM */
    signal(SIGINT, catch_sig);
    
    /* argument parsing */
    while ((c = getopt (argc, argv, "hv")) != -1)
    switch (c){
        /* display help menu */
        case 'h': 
            usage(argv[0]); 
            exit(EXIT_SUCCESS);
        /* set verbosity flag */
        case 'v': 
            verbose = 1; 
            break;
        /* set notifier flag */
        case 'n':
            notifier = 1;
            break;
        /* default short to usage */
        default: 
            usage(argv[0]); 
            exit(EXIT_FAILURE);
    }
    
    /* check argument list for any yaml file configuration changes */
    for (int i = 1; i < argc; i++){
        /* checking for string with dot to represent file type */
        char *dot = strrchr(argv[i], '.');
        if (dot && !strcmp(dot, ".yaml")){
            
            if (verbose)
                printf("YAML file specified: %s\n", argv[i]);
            
            /* set default file as argument with YAML extensin */
            default_yaml = argv[i];
            break;
        }
    }
    
    if (verbose)
        printf("STARTING WATCHMAN!\n");
    
    /* perform file-checking */
    yaml = file_check(default_yaml);
    if ( yaml.flag < 0){
        
        /* no perror, since we got back a errno string. */
        fprintf(stderr, "Error %i: Unable to open file: %s\n", yaml.flag, yaml.data);
      
        /* create new file and then quit */
        if (verbose)
            printf("Creating configuration file for you...\n");
        
        file_t new_file = create_file(CONFIG_FILE, NULL);
        exit(EXIT_FAILURE);
    } 
    else {
        /* success! */
        if (verbose)
            printf("File successfully found and opened!\n");
    }
    
    /* check for key-value validity within yaml file */
    y = parse_yaml_config(default_yaml);
    
    if (y.return_flag == false)
        perror("Could not initialize YAML parser. Reason");
    else if ( y.return_flag == true ){
        if (verbose)
            printf("Successfully parsed YAML file.\n"
                "\tinode: %s\n\tevent: %s\n\texecute: %s\n",
                y.inode, y.event, y.execute);
    }
    
            
    /* 
     * Iterate through events[] array to check if inode specified
     * is specified. e_flag is set to see if comparisions are valid. If all
     * are not, last instance of e_flag remains 1, printing to stderr. 
     */
    for (int i = 0; events[i] != NULL; i++){
        if (strcmp(y.event, events[i]) == 0){ 
            e_flag = 0; 
            break; /* break. event has been matched */
        } else {
            e_flag = 1;
            continue; /* set e_flag and continue until end */
        }
    }
    
    /* if e_flag retains 1, then the event wasn't found */
    if (e_flag == 1){
        fprintf(stderr, "\nUnknown inode event supplied: %s\n", y.event);
        exit(EXIT_FAILURE);
    }
    else if (e_flag == 0){
        if (verbose){
            printf("\ninode event found! Continuing.\n");
        }
    }
    
   /*
    * Iterate through execute_action to check if execute action matches.
    * However, we need to first slice the value.
    */    
    mem = str = strdup(y.execute); /* allocate memory and create a copy */
    
   /* 
    * prepend represents the first part of the command string.
    * We will compare this with the execute_action array.
    */
    prepend = strtok(str, sep);
    
    /* represents command user wants to call */
    command = strtok(NULL, "\"");
    
    /* check if the command is valid, and set the appropriate flag */
    for ( int i = 0; execute_action[i] != NULL; i++){
      if (strcmp(prepend, execute_action[i]) == 0){
        x_flag = 0; 
        break;
      } else {
        x_flag = 1; 
        continue;
      }
    }
    
    /* error-checking */
    if (x_flag) {
        fprintf(stderr, "Unknown command supplied: %s\n", y.execute);
        exit(EXIT_FAILURE);
    }
    else if (x_flag == 0) {
        if (verbose){
            printf("Command found! Continuing.\n");
        }
    }
    
   /* 
    * use a standard file_check to check for existence of inode.
    * We will only be using this to treat the inode as a standard file/directory.
    * Much larger operations will be executed during actual watcher creation.
    */
    inode_check = file_check(y.inode);
    if (inode_check.flag < 0){
        fprintf(stderr, "Error %i: Unable to open inode \"%s\": %s\n", inode_check.flag, y.inode, inode_check.data);
        exit(EXIT_FAILURE);
    } 
    else {
        
        /* now check if we have permissions to access */
        if (verbose)
            printf("inode successfully found and opened!\n");
        
        inode_i = check_inode_permissions(y.inode);
        
        if (inode_i < 0)
            perror("Permission check for inode failed! Reason");
    }
    

    /* initialize inotify */
    fd = inotify_init();
    if (fd < 0)
        perror("Could not initialize inotify. Reason");
        
    /* add a file watcher */
    wd = inotify_add_watch(fd, y.inode, IN_ALL_EVENTS);
    if (wd < 0){ 
        perror("Could not add watch. Reason");
    }

    
   /* starting looping and watching for events!
    * NOTE: BREAK on interrupt!! Important so that program can finish execution
    * by freeing memory and removing watcher
    */
    while (sc) {      

        /* read from inotify fd */
        rd = read(fd, buf, BUF_LEN);
        if (rd == 0) 
            fprintf(stdout, "read() tossed back a 0");
        else if (rd == 1){
            perror("Couldn't read event. Reason");
            break;
        }

        /* process events and do what's necessary according to YAML */
        for (p = buf; p < buf + rd;){

            /* made local so that data gets re-initialized within scope */
            struct tm * timeinfo;                             
            time_t rawtime;           
            char *ltime, *eventstr;
            const char *event;                            
          
            /* get time, and create new string */
            timeinfo = gettime(rawtime);
            ltime = asctime(timeinfo);
          
            /* copy over inotify_event */
            ev = (struct inotify_event *) p;
              
            /* display event through terminal*/
            event = display_event(ev);

            /* raise notification if flag was set */
            if (notifier)
                raise_notification(ltime, event);
 
            /* check command, if the specified event matches the current event and execute accordingly */
            if ((strcmp(prepend, "execute") == 0) && (strcmp(y.event, event) == 0)){
                
                /* parse command for fork / execv */
                char *argv_p[64];
                parse_execute(command, argv_p);

                /* execute the command */
                int e_result = execute_command(argv_p);
                if (e_result < 0) {
                    perror("Could not execute command. Reason");
                    exit(EXIT_FAILURE);
                }
                
            } 
            else if (strcmp(prepend, "log") == 0 ){

                /* source: https://stackoverflow.com/questions/5901181/c-string-append */
                if ((eventstr = malloc(strlen(ltime) + strlen(event) + 2)) != NULL){
                    eventstr[0] = '\0';
                    strcat(eventstr, ltime);
                    strcat(eventstr, event);
                    strcat(eventstr, "\n");
                }
                else {
                    perror("malloc failed. Reason");
                    exit(EXIT_FAILURE);
                }
  
                /* if path is none, use default. */
                if (command == NULL) {
                    command = DEFAULT_FILENAME;
                }
                
                /* create a log file, with contents of eventstr */
                file_t tmpLog = create_file(command, eventstr);
                if (tmpLog.flag < 0 ){
                    perror("Couldn't create log file. Reason");
                    exit(EXIT_FAILURE);
                } 
                free(eventstr);
            }
            p += sizeof(struct inotify_event) + ev->len;
        }
    }
    
    /* success */
    exit(EXIT_SUCCESS);
}
