#include <stdio.h>
#define ARRAY_IMPLEMENTATION
#define BUIC_IMPLEMENTATION
#define FUTIL_IMPLEMENTATION
#define EITHER_IMPLEMENTATION
#define STR_IMPLEMENTATION
#include <buic.h>
#include <futil.h>
#include <either.h>
#include <str.h>
#include <array.h>

#define BUILD_DIR "build/"

int main() {
  if (mkdir_if_not_exists(BUILD_DIR).is_error) return 1;
  CommandBuilder cmd = {
    .compiler = BCCOMP_NATIVE, .has_debug = true,
    .output = "app", .src_path = "src", .build_dir = BUILD_DIR,
  };
  arr_push(&cmd.inputs, "test.c");
  arr_push(&cmd.defines, "urmom");
  if (!cmd_run(&cmd)) return 1;

  arr_push(&cmd.inputs, "main.c");
  cmd.output = "build/main";
  cmd.compiler = BCCOMP_CLANG;
  cmd.has_debug = true;
  if (!cmd_run(&cmd)) return 1;

  return 0;
}
