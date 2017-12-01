#include "hello.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

extern void import_function(int);
extern void print_function(char *);

int export_function(int i_test) {
  print_function("Testing print import");
  import_function(header_function(i_test));
  return ++i_test;
}

char *print_export(){
	return "Print export text";
}

