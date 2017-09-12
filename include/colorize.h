#ifndef COLORIZE_H
#define COLORIZE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#define PREPEND "\e[" // How to dictate a color escape code


static const char REGULAR[] = "0;3";
static const char BOLD[] = "1;3";
static const char UNDERLINE[] = "4;3";
static const char BACKGROUND[] = "4";

typedef char color[];

// Define variables denoting color
color BLACK = "0m";
color RED = "1m";
color GREEN = "2m";
color YELLOW = "3m";
color BLUE = "4m";
color PURPLE = "5m";
color CYAN = "6m"; 
color WHITE = "7m";

// ..special case
color RESET = "\e[0m";

// e.g 
// "\e[4;30m"  represents underlined black

typedef struct colorize {
  const char *flag; // set flag if you want output to be altered in some way (bold, underline, etc)
  color *startcolor; // color of actual text
  char *input; // input (TODO: handle format specifiers )
  color *endcolor; // color of text afterwards ( can be a RESET )  
} colorize;

/*
void cz_color(char * color, const char * flag){
  char * final_color = (char *)malloc(strlen(color) + strlen(flag) + 1);
  sprintf(final_color, "%s%s%s", PREPEND, flag, color);
  fprintf(stdout, "%s", final_color);
  free(final_color);
}

void cz_reset(){
  char * reset = (char *)malloc(strlen(RESET) + 1);
  sprintf(reset, "%s", RESET);
  fprintf(stdout, "%s", reset);
  free(reset);
}
*/

// Function for declaring a colorized output. For e.g
// 
//  colorize * c = cz_new(GREEN, "hello world!", RESET, BOLD);
//
colorize *cz_new(color startcolor, char * input, color endcolor, ...){
  
  va_list ap;
  va_start(ap, endcolor);
  
  colorize *c = (colorize *)malloc(sizeof(colorize));
  c->startcolor = (char(*)[]) startcolor;
  c->input = input;
  c->endcolor = (char(*)[]) endcolor;
  // .. optional argument: flag. if not set, set as REGULAR
  c->flag = va_arg(ap, char*);
  
  if( c->flag == NULL){
    c->flag = REGULAR;
  }
  
  va_end(ap);
  return c;
}


// Function for actually printing the colorized output to STDOUT.
//
//    czprint(c);
//
void czprint(colorize *c){  
  char * start_color = (char *)malloc(strlen(*c->startcolor) + strlen(c->flag) + strlen(PREPEND) + 1);
  char * end_color = (char *)malloc(strlen(*c->endcolor) + strlen(c->flag) + strlen(PREPEND) + 1);
  
  if( *c->startcolor != RESET ){
    sprintf(start_color, "%s%s%s", PREPEND, c->flag, *c->startcolor);
  } else if( *c->startcolor == RESET ){
    sprintf(start_color, "%s", RESET);
  }
    
  if( *c->endcolor != RESET ){
    sprintf(end_color, "%s%s%s", PREPEND, c->flag, *c->endcolor);
  }else if( *c->endcolor == RESET ){
    // realloc()?
    strcat(end_color, RESET); 
  }
  
  char * color_string = (char *)malloc(strlen(c->input) + strlen(start_color) + strlen(end_color) + 1);
  
  strcat(color_string, start_color);
  strcat(color_string, c->input);
  strcat(color_string, end_color);
  
  fprintf(stdout, "%s\n", color_string);
  
  free(start_color); free(end_color); free(color_string);
}

// Free allocated memory!
void czclean(colorize *c){
  fflush(stdout);
  free(c);
}

#endif