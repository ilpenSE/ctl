#include <stdio.h>

#include <futil.h>
#include <either.h>

#define BUILD_PATH "build/"
#define FILE1_PATH BUILD_PATH"file2.txt"
#define FILE2_PATH BUILD_PATH"file.txt"

int main() {
  Result(String) res = read_entire_file(FILE2_PATH);
  if (RES_ISERR(res)) {
    fprintf(stderr, "Couldn't read entire file: %s: %s\n", FILE2_PATH, err_tostr(RES_GETE(res)));
    return 1;
  }
  String s = RES_UNWRAP(res);
  printf("`"FILE2_PATH"` contains %zu bytes\n", s.len);
  printf("`"FILE2_PATH"` contains %zu lines\n", count_lines_from_str(&s));
  printf("`"FILE1_PATH"` contains %zu lines\n", count_lines_from_file(FILE1_PATH));
  printf("Dumped `"FILE2_PATH"`:\n```\n%s\n```\n", s.data);

  str_free(&s);
  return 0;
}
