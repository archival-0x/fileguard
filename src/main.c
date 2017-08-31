#include "main.h"
#include <libnotify/notify.h>
#include <sys/inotify.h>

int main( int argc, char **argv )
{
    int c;
    
    while ((c = getopt (argc, argv, "ch")) != -1)
    switch (c)
      {
      case 'c':
        printf("Checking YAML config for inconsistencies...\n");  
        break;
      case 'h':
        fprintf(stdout, "Usage: %s -[c|h]\n(note that these are optional arguments)\n", argv[0]);
        break;
      default:
        return 1;
      }
    
    return 0;
}
