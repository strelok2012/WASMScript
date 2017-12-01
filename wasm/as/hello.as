declare function env$import_function(i_test: i32): i32;

export function export_function(i_test: i32): i32 {
  env$import_function(i_test*2);
  return ++i_test;
}
