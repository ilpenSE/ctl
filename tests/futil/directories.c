#include <stdio.h>
#include <futil.h>
#include <either.h>

// Trailing slash is mandatory
#define DIRECTORY_TEST "folder/"

int main() {
  ErrorOrNot status = mkdir_if_not_exists(DIRECTORY_TEST);
  if (status.is_error) {
    fprintf(stderr, "Couldn't make directory: %s: %s\n",
            err_tostr(status.error.code), status.error.message);
    return 1;
  }
  printf("Directory made: `"DIRECTORY_TEST"`\n");
  printf("Is valid directory: %d\n", is_valid_path(DIRECTORY_TEST));
  return 0;
}
