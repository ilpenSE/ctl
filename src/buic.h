/* SPDX-License-Identifier: GPL-3.0-only */
#ifndef BUIC_H
#define BUIC_H

#include <stdbool.h>
#include "basic.h"
#include "array.h"

typedef enum {
  BCCOMP_NATIVE = 0,
  BCCOMP_GCC,
  BCCOMP_CLANG,
  BCCOMP_MSVC,
  _BuildCompiler_count,
} BuildCompiler;

typedef struct {
  Array(cchar_ptr) flags; // custom (arbitrary) flags
  Array(cchar_ptr) defines; // -D OR /D
  Array(cchar_ptr) inputs;
  Array(cchar_ptr) include_paths; // /include -> -I./include
  Array(cchar_ptr) lib_paths; // /lib -> -L./lib
  Array(cchar_ptr) libs; // -l:libmylib.so
  const char* src_path; // /src -> main.c -> src/main.c
  const char* build_dir;
  const char* output;
  BuildCompiler compiler;
  bool has_debug; // -g OR -ggdb OR /Zi
} CommandBuilder;

typedef struct {
  bool reset;
  bool log;
} CommandRunOptions;

bool cmd_reset(CommandBuilder* cmd);
bool cmd_run_opt(CommandBuilder* cmd, CommandRunOptions opts);
const char* bcomp_to_cstr(BuildCompiler compiler);

#define cmd_run(cmd, ...) \
  cmd_run_opt((cmd), (CommandRunOptions){.reset=true, .log=true, __VA_ARGS__})

// IMPLEMENTATION BEGIN
#ifdef BUIC_IMPLEMENTATION
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

#if defined(_WIN32)
  #define NATIVE_COMPILER "cl"
#elif defined(__APPLE__)
  #define NATIVE_COMPILER "clang"
#else
  #define NATIVE_COMPILER "gcc"
#endif

#ifdef _WIN32
  #define _BUIC_PATH_SEP "\\"
#else
  #define _BUIC_PATH_SEP "/"
#endif // _WIN32

const char* bcomp_to_cstr(BuildCompiler compiler) {
  switch (compiler) {
  case BCCOMP_NATIVE: return NATIVE_COMPILER;
  case BCCOMP_GCC: return "gcc";
  case BCCOMP_CLANG: return "clang";
  case BCCOMP_MSVC: return "cl";
  default: return NULL;
  }
}

bool cmd_run_opt(CommandBuilder* cmd, CommandRunOptions opts) {
  bool ok = false;
  Array(cchar_ptr) cmdline = {0};
  const char* ptrs[8] = {0};
  size_t idx = 0;
  if (!cmd) goto defer;

  // Add compiler
  const char* compiler_str = bcomp_to_cstr(cmd->compiler);
  if (!compiler_str) goto defer;
  arr_push(&cmdline, compiler_str);

  // Add source files
  if (cmd->src_path && *cmd->src_path) {
    arr_foreach(&cmd->inputs, it) {
      size_t dir_len = strlen(cmd->src_path);
      size_t file_len = strlen(*it);
      size_t total = file_len + dir_len + 2;
      char* buf = malloc(total);
      if (!buf) goto defer;
      ptrs[idx++] = buf;
      assert(snprintf(buf, total,
             "%s"_BUIC_PATH_SEP"%s", cmd->src_path, *it) == (int)total - 1);
      arr_push(&cmdline, buf);
    }
  } else {
    arr_merge(&cmdline, &cmd->inputs);
  }

// (field_name, prefix_str, prefix_len)
#define _BUIC_REPEATING_FIELDS \
  X(include_paths, "-I", 2) \
  X(lib_paths, "-L", 2) \
  X(libs, "-l:", 3) \
  X(defines, "-D", 2)

#define X(field_name, prefix_str, prefix_len) \
  arr_foreach(&cmd->field_name, it) {\
    size_t total = strlen(*it) + prefix_len + 1;\
    char* buf = malloc(total);\
    if (!buf) goto defer;\
    ptrs[idx++] = buf;\
    assert(snprintf(buf, total, prefix_str"%s", *it) == (int)total - 1);\
    arr_push(&cmdline, buf);\
  }
_BUIC_REPEATING_FIELDS
#undef X

  arr_merge(&cmdline, &cmd->flags);
  if (cmd->has_debug) arr_push(&cmdline, "-ggdb");

  const char* output_path = NULL;
  if (cmd->build_dir && *cmd->build_dir) {
    size_t pathsz = strlen(cmd->build_dir) + strlen(cmd->output) + 2;
    char* path = malloc(pathsz);
    if (!path) goto defer;
    ptrs[idx++] = path;
    assert(snprintf(path, pathsz,
          "%s"_BUIC_PATH_SEP"%s", cmd->build_dir, cmd->output) == (int)pathsz - 1);
    output_path = path;
  } else {
    output_path = cmd->output;
  }
  arr_push(&cmdline, "-o", output_path);

  if (opts.log) {
    printf("[INFO] CMD: ");
    size_t len = arr_len(&cmdline);
    for (size_t i = 0; i < len; i++) {
      printf("%s", cmdline.items[i]);
      if (i != len - 1) printf(" ");
    }
    printf("\n");
  }

  pid_t pid = fork();
  if (pid == 0) {
    // child
    arr_push(&cmdline, NULL);
    execvp(cmdline.items[0], (char* const*)cmdline.items);
    perror("execvp");
    exit(1);
  } else if (pid > 0) {
    // parent
    int wstatus = 0;
    waitpid(pid, &wstatus, 0);
    if (WIFEXITED(wstatus)) {
      int exit_code = WEXITSTATUS(wstatus);
      if (exit_code != 0) {
        fprintf(stderr, "[ERROR] Build failed with code %d\n", exit_code);
        goto defer;
      }
    }
  } else {
    perror("fork");
    goto defer;
  }

  // Reset if needed
  if (opts.reset) {
    cmd_reset(cmd);
  }

  ok = true;
defer:
  // Clear internal pointers and vector
  for (int i = 0; i < 8; i++) {
    if (ptrs[i] == NULL) continue;
    free((void*)ptrs[i]);
  }
  arr_free(&cmdline);
  return ok;
}

bool cmd_reset(CommandBuilder* cmd) {
  arr_free(&cmd->flags);
  arr_free(&cmd->defines);
  arr_free(&cmd->inputs);
  arr_free(&cmd->libs);
  arr_free(&cmd->include_paths);
  arr_free(&cmd->lib_paths);
  *cmd = (CommandBuilder){0};
  return true;
}
#endif /* BUIC_IMPLEMENTATION */
// IMPLEMENTATION END
#endif /* BUIC_H */
