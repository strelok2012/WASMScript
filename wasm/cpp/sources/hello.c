#include "hello.h"

extern void import_function(int);

long long GlobalVariable = 0;

const char *STRING_1 = "string1";
char *STRING_2 = "string2";

test_struct g_struct = {
	.valueChar = -1,
	.valueShort = -2,
	.valueInt = -3,
	.valueLong = -4,
	.valueLongLong = -5,
	.valueUnsignedChar = 1,
	.valueUnsignedShort = 2,
	.valueUnsignedInt = 3,
	.valueUnsignedLong = 4,
	.valueUnsignedLongLong = 5,
	.valueFloat = 123.0f,
	.valueDouble = 5.0,
	.valueCharPointer = "StringValue"
};

extern test_struct e_struct;
extern test_struct e_struct2;

int export_function(int i_test) {
	GlobalVariable += 1;
  import_function(i_test * 3);
  return ++i_test;
}

long long get_global_value() {
	return GlobalVariable;
}

const char *get_string1() {
	return STRING_1;
}

char *get_string2() {
	return STRING_2;
}

test_struct *get_struct2() {
	return &e_struct;
}
test_struct *get_struct3() {
	return &e_struct2;
}
