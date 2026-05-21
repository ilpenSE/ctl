#include <stdio.h>
#include <str.h>
#include <futil.h>

#define give_err(res, fmt, ...) \
  do { \
    fprintf(stderr, "%s:%d: "fmt": %s\n", __FILE__, __LINE__, ##__VA_ARGS__, err_tostr(RES_GETE((res)))); \
  } while (0)

int main() {
  String s = str_new("selamın aleyküm");
  printf("%s\n", s.data);

  const char* file_path = "file.txt";
  Result(String) res = read_entire_file(file_path);
  if (RES_ISERR(res)) {
    give_err(res, "read_entire_file failed: %s", file_path);
    return 1;
  }

  String file = RES_UNWRAP(res);
  printf("|%s|\n", file.data);
  str_free(&file);
  return 0;
}
