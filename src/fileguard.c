#include "fileguard.h"

int
check_inode_permissions(char * inode_name)
{
    struct stat file_stat;
    int fd, ac;

    /* check if user can access inode through read and write */
    ac = access(inode_name, R_OK | W_OK);
    if (ac != 0)
        return ac;

    /* open file, stat file descriptor and get inode number */
    fd = open(inode_name, O_RDONLY, 0644);
    fstat(fd, &file_stat);
    return file_stat.st_ino;
}


void
raise_notification(const char * timeinfo, const char *event)
{
    gboolean nint;
    nint = notify_init("fileguard");
    if (nint == FALSE)
        perror("Could not initialize libnotify. Reason");

    /* instantiate notification display with time and event */
    NotifyNotification * display = notify_notification_new(timeinfo, event, NULL);
    if (display == NULL)
        perror("Could not initiailize display. Reason");

    /* display, and destroy */
    notify_notification_show(display, NULL);
    notify_uninit();
}


const char *
get_event(struct inotify_event *i)
{
    if (i->mask & IN_ACCESS)                return "IN_ACCESS";
    else if (i->mask & IN_ATTRIB)           return "IN_ATTRIB";
    else if (i->mask & IN_CLOSE_WRITE)      return "IN_CLOSE_WRITE";
    else if (i->mask & IN_CLOSE_NOWRITE)    return "IN_CLOSE_NOWRITE";
    else if (i->mask & IN_CREATE)           return "IN_CREATE";
    else if (i->mask & IN_DELETE)           return "IN_DELETE";
    else if (i->mask & IN_DELETE_SELF)      return "IN_DELETE_SELF";
    else if (i->mask & IN_MODIFY)           return "IN_MODIFY";
    else if (i->mask & IN_MOVE_SELF)        return "IN_MOVE_SELF";
    else if (i->mask & IN_MOVED_FROM)       return "IN_MOVED_FROM";
    else if (i->mask & IN_MOVED_TO)         return "IN_MOVED_TO";
    else if (i->mask & IN_OPEN)             return "IN_OPEN";
    else if (i->mask & IN_UNMOUNT)          return "IN_UNMOUNT";
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

    f.flag = fd;
    f.data = data;

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

    if (data != NULL)
        dprintf(fd, "%s", data);

    f.flag = fd;
    f.data = filename;

    free(path);
    return f;
}

struct tm *
gettime(time_t rawtime)
{
    time(&rawtime);
    struct tm * timeinfo = localtime(&rawtime);
    return timeinfo;
}


yaml_t
parse_yaml_config(char * filename)
{
    yaml_parser_t parser;
    yaml_token_t token;

    /* initialize tokenizer for yaml */
    int state = 0;
    char **datap;
    char *tk;

    /* initialize new yaml_t config */
    yaml_t config;
    if (!yaml_parser_initialize(&parser)) {
        config.return_flag = false;
        return config;
    }

    /* initialize file for reading */
    FILE *fptr = fopen(filename, "r");
    if (fptr == NULL) {
        config.return_flag = false;
        return config;
    }

    /* set input file */
    yaml_parser_set_input_file(&parser, fptr);

    do {
        yaml_parser_scan(&parser, &token);
        switch (token.type) {
            case YAML_KEY_TOKEN:
                state = 0;
                break;
            case YAML_VALUE_TOKEN:
                state = 1;
                break;
            case YAML_SCALAR_TOKEN:
                tk = token.data.scalar.value;

                if (state == 0) {
                    if (!strcmp(tk, "inode"))
                        datap = &config.inode;
                    else if (!strcmp(tk, "event"))
                        datap = &config.event;
                    else if (!strcmp(tk, "action"))
                        datap = &config.action;
                    else {
                        config.return_flag = false;
                        return config;
                    }
                } else {
                    *datap = strdup(tk);
                }
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
