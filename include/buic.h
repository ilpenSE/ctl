/* SPDX-License-Identifier: GPL-3.0-only */
#ifndef BUIC_H
#define BUIC_H

#include <time.h>
#include <stdbool.h>
#include "basic.h"
#include "futil.h"
#include "str.h"
#include "array.h"

#ifdef __cplusplus
  #define BUICDEF extern "C"
#else
  #define BUICDEF extern
#endif

typedef struct {
  bool reset;
  bool log;
} CommandRunOptions;

typedef Array(cchar_ptr) CommandBuilder;
#define cmd_push arr_push
#define cmd_free arr_free

#define cmd_run(cmd, ...) \
  cmd_run_impl((cmd), (CommandRunOptions){.reset=true, .log=true, __VA_ARGS__})
BUICDEF bool cmd_run_impl(CommandBuilder* cmd, CommandRunOptions opts);

#define cmd_insta_run(...) cmd_insta_run_impl(__VA_ARGS__, NULL)
bool cmd_insta_run_impl(const char* first, ...);

#define BUIC_REBUILD_URSELF(argc, argv, ...) \
  buic_rebuild_urself((argc), (argv), __FILE__, ##__VA_ARGS__, NULL)

BUICDEF time_t buic_compare_mtimes(const char* f1, const char* f2);
BUICDEF bool buic_rebuild_urself(int argc, char** argv, const char* file_name, const char* first, ...);

#endif /* BUIC_H */
