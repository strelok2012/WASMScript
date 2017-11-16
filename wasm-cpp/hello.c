extern void importFunction(int);

int exportFunction(int i_test) {
  importFunction(i_test*2);
  return ++i_test;
}
