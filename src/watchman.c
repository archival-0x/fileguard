#include "watchman.h"

struct file file_check(char * filename){
  // No malloc(), since I gots some unexpected errors. Plus, I don't know when
  // I should be free-ing my memory :(
  struct file f;
  int fd, len;
  void *data;
  
  // Not the best, but we are creating a FILE pointer for the sake of libyaml.
  // We also use this to conduct initial error checking.
  FILE *fptr = fopen(filename, "r");
  if (!fptr){
    f.flag = -1; f.fpointer = NULL, f.data = strerror(errno);
    return f;
  }
  
  // POSIX-style file-opening. Better for performance, but repetitive.
  fd = open(filename, O_RDONLY);
  len = lseek(fd, 0, SEEK_END);
  data = mmap(0, len, PROT_READ, MAP_PRIVATE, fd, 0);
  
  f.flag = 0; f.fpointer = fptr; f.data = data;
  
  return f;
}

Yaml parse_yaml_config(char arg, FILE *fptr){
  
   yaml_parser_t parser;
   yaml_token_t  token;
   
   // Declare variables for tokenizer
   // Credit: https://stackoverflow.com/questions/20628099/parsing-yaml-to-values-with-libyaml-in-c
   int state = 0;
   char ** datap;
   char *tk;
   
   // Create a struct for yaml config parsing.
   Yaml *config = malloc(sizeof(config));
   
   // Throw error if parser fails to initialize
   if(!yaml_parser_initialize(&parser)){
     config->return_flag = false;
     return *config; 
   }
   // Throw error if file pointer returns a NULL
   if(fptr == NULL){
     config->return_flag = false;
     return *config;
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
             datap = &config->inode;
           } else if (!strcmp(tk, "event")) {
             datap = &config->event;
           } else if (!strcmp(tk, "execute")) {
             datap = &config->execute;
           } else {
             config->return_flag = false;
             return *config;
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
   
   // Set return_flag to true and return config
   if (arg = 'c') config->return_flag = true; return *config;
   
   return *config;
}