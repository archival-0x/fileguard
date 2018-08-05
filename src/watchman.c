#include "watchman.h"

int 
check_inode_permissions(char * inode_name)
{
    struct stat file_stat;
    int fd, ac; 
  
    /* check if user can access inode through read and write */
    ac = access(inode_name, R_OK | W_OK);
    if (!(ac < 0)){
        fd = open(inode_name, O_RDONLY , 0644 );
        fstat (fd, &file_stat); 
        return file_stat.st_ino;
    } else
        return ac; /* aka -1 */
}


NotifyNotification 
raise_notification(const char * timeinfo, const char *event)
{
    gboolean nint;
    nint = notify_init("Watchman");

    if (nint == FALSE) {
        perror("Could not initialize libnotify. Reason");
    }
  
    NotifyNotification * display = notify_notification_new(timeinfo, event, NULL);
    notify_notification_show(display, NULL);
  
    /* close after delay */
    //notify_notification_close(display, NULL);
  
    notify_uninit();
}

uint32_t 
parse_event(char * event)
{
    
    /* TODO: Maybe implement switch case; try to modify for comparing chars
     * TODO: Implement support for bitwise inotify events, in case one wants to
     *       specify a multitude of events (ie IN_ACCESS|IN_CREATE )
    */

    if (strcmp(event, "IN_ACCESS"))               return IN_ACCESS;
    else if (strcmp(event, "IN_ATTRIB"))          return IN_ATTRIB;
    else if (strcmp(event, "IN_CLOSE_WRITE"))     return IN_CLOSE_WRITE;
    else if (strcmp(event, "IN_CLOSE_NOWRITE"))   return IN_CLOSE_NOWRITE;
    else if (strcmp(event, "IN_CREATE"))          return IN_CREATE;
    else if (strcmp(event, "IN_DELETE"))          return IN_DELETE;
    else if (strcmp(event, "IN_DELETE_SELF"))     return IN_DELETE_SELF;
    else if (strcmp(event, "IN_MODIFY"))          return IN_MODIFY;
    else if (strcmp(event, "IN_MOVE_SELF"))       return IN_MOVE_SELF;
    else if (strcmp(event, "IN_MOVED_FROM"))      return IN_MOVED_FROM;
    else if (strcmp(event, "IN_MOVED_TO"))        return IN_MOVED_TO;
    else if (strcmp(event, "IN_OPEN"))            return IN_OPEN;
    else if (strcmp(event, "IN_UNMOUNT"))         return IN_UNMOUNT;
    else if (strcmp(event, "IN_ALL_EVENTS"))      return IN_ALL_EVENTS;
}

const char * 
display_event(struct inotify_event *i)
{
    if (i->mask & IN_ACCESS)        printf("IN_ACCESS occurred!\n"); return "IN_ACCESS";
    if (i->mask & IN_ATTRIB)        printf("IN_ATTRIB occurred!\n"); return "IN_ATTRIB";
    if (i->mask & IN_CLOSE_WRITE)   printf("IN_CLOSE_WRITE occurred!\n"); return "IN_CLOSE_WRITE";
    if (i->mask & IN_CLOSE_NOWRITE) printf("IN_CLOSE_NOWRITE occurred!\n"); return "IN_CLOSE_NOWRITE";
    if (i->mask & IN_CREATE)        printf("IN_CREATE occurred!\n"); return "IN_CREATE";
    if (i->mask & IN_DELETE)        printf("IN_DELETE occurred!\n"); return "IN_DELETE";
    if (i->mask & IN_DELETE_SELF)   printf("IN_DELETE_SELF occurred!\n"); return "IN_DELETE_SELF";
    if (i->mask & IN_MODIFY)        printf("IN_MODIFY occurred!\n"); return "IN_MODIFY";
    if (i->mask & IN_MOVE_SELF)     printf("IN_MOVE_SELF occurred!\n"); return "IN_MOVE_SELF";
    if (i->mask & IN_MOVED_FROM)    printf("IN_MOVED_FROM occurred!\n"); return "IN_MOVED_FROM ";
    if (i->mask & IN_MOVED_TO)      printf("IN_MOVED_TO occurred!\n"); return "IN_MOVED_TO";
    if (i->mask & IN_OPEN)          printf("IN_OPEN occurred!\n"); return "IN_OPEN";
    if (i->mask & IN_UNMOUNT)       printf("IN_UNMOUNT occurred!\n"); return "IN_UNMOUNT";
}


file_t 
file_check(char * filename)
{
    file_t f;
    int fd, len;
    void *data;
  
  
    fd = open(filename, O_RDONLY, 0644);
    if (fd < 0){ 
        f.data = strerror(errno); 
        f.flag = fd; 
        return f;
    }
    
    len = lseek(fd, 0, SEEK_END);
    data = mmap(0, len, PROT_READ, MAP_PRIVATE, fd, 0);
  
    f.flag = fd; f.data = data;
  
    return f;
}

file_t 
create_file(char * filename, char * data)
{
    file_t f;
  
    char *path = malloc(strlen(filename) + 1 );
    path = strcpy(path, filename);

    int fd = open(path, O_RDWR | O_APPEND | O_CREAT);   
    if ( fd < 0) { 
        f.flag = fd; 
        f.data = strerror(errno); 
        return f;
    }  

    if (data != NULL){
        dprintf(fd, "%s", data);
    }
  
    f.flag = fd; f.data = filename;
    free(path);
    return f;
}

struct tm * 
gettime(time_t rawtime)
{
    time(&rawtime);
    struct tm * timeinfo = localtime ( &rawtime );
    return timeinfo;
}

yaml_t 
parse_yaml_config(char * filename)
{
  
    yaml_parser_t parser;
    yaml_token_t  token;
   
    /* initialize tokenizer for yaml */
    /* source: https://stackoverflow.com/questions/20628099/parsing-yaml-to-values-with-libyaml-in-c */
    int state = 0;
    char ** datap;
    char *tk;
   
    /* initialize new yaml_t config */
    yaml_t config; 
    if (!yaml_parser_initialize(&parser)){
        config.return_flag = false;
        return config; 
    }
  
    /* initialize file for reading */ 
    FILE *fptr = fopen(filename, "r");
    if (fptr == NULL){
        config.return_flag = false;
        return config;
    }
   
    /* set input file */ 
    yaml_parser_set_input_file(&parser, fptr);  
    
    do {
        // Starting parsing through token method
        yaml_parser_scan(&parser, &token);
     
        // Check key values against the yml struct.
        switch (token.type){
            case YAML_KEY_TOKEN : 
                state = 0; 
                break;
            case YAML_VALUE_TOKEN :
                state = 1; 
                break;
            case YAML_SCALAR_TOKEN :
                tk = token.data.scalar.value;
                
                /* check for keys */
                if (state == 0) {
                    if (!strcmp(tk, "inode"))
                        datap = &config.inode;
                    else if (!strcmp(tk, "event"))
                        datap = &config.event;
                    else if (!strcmp(tk, "execute"))
                        datap = &config.execute;
                    else {
                        config.return_flag = false;
                        return config;
                    }
                } 
                else
                    *datap = strdup(tk);
                break;
            default: 
                break;
        }
    } while (token.type != YAML_STREAM_END_TOKEN);
   
        /* cleanup */
        yaml_token_delete(&token);
        yaml_parser_delete(&parser);
        fclose(fptr);
   
        config.return_flag = true;
        return config;
}

void 
parse_execute(char * line, char ** argv)
{
    while (*line != '\0') {       
        while (*line == ' ' || *line == '\t' || *line == '\n')
            *line++ = '\0';    
        *argv++ = line;          
        while (*line != '\0' && *line != ' ' && 
            *line != '\t' && *line != '\n') 
            line++;             
    }
    *argv = '\0';             
}

int 
execute_command(char **argv)
{
    pid_t  pid;     /* child process id */
    int    status;  /* status */

    if ((pid = fork()) < 0) {    
        return -1;
    } else if (pid == 0) {         
        if (execvp(*argv, argv) < 0)
            return -1;
    } else
        while (wait(&status) != pid);
    
    return 0;
}
