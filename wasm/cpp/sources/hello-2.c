#include "hello.h"


extern test_struct g_struct;

int header_function(int i_test){
	return i_test*3;
}


test_struct *get_struct() {
	return &g_struct;
}
