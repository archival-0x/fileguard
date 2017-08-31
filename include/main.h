#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <yaml.h>
#include <glib.h>

typedef enum
{
   IN_ACCESS,
   IN_ATTRIB,
  
} Events

bool check_inode_permissions();
bool check_yaml_configuration();