#include "hello.h"

extern void import_function(int);

int export_function(int i_test) {
  import_function(header_function(i_test));
  return ++i_test;
}
