declare function env$importFunction(i_test: i32): i32;

export function exportFunction(i_test: i32): i32 {
  env$importFunction(i_test*2);
  return ++i_test;
}
