#include <stdio.h>
#define CTL_IMPLEMENTATION
#include "../../ctl.h"

#define BUILD_FOLDER "build/"

int main(int argc, char** argv) {
  BUIC_REBUILD_URSELF(argc, argv, "-lm");

  if (mkdir_if_not_exists(BUILD_FOLDER).is_error) return 1;
  CommandBuilder cmd = {0};
  cmd_push(&cmd, "clang");
  cmd_push(&cmd, "src/test.c");
  cmd_push(&cmd, "-Durmom");
  cmd_push(&cmd, "-Dkral=2");
  cmd_push(&cmd, "-o", BUILD_FOLDER"app");
  if (!cmd_run(&cmd)) return 1;

  cmd_push(&cmd, "clang");
  cmd_push(&cmd, "main.c");
  cmd_push(&cmd, "-ggdb");
  cmd_push(&cmd, "-O3");
  cmd_push(&cmd, "-o", BUILD_FOLDER"main");
  if (!cmd_run(&cmd)) return 1;

  return 0;
}
