/* SPDX-License-Identifier: GPL-3.0-only */
#ifndef FUTIL_H
#define FUTIL_H

#ifdef __cplusplus
  #define FUTILDEF extern "C"
#else
  #define FUTILDEF extern
#endif

#include <stddef.h>
#include <stdbool.h>
#include "basic.h"
#include "str.h"
#include "either.h"

#ifdef PLATFORM_POSIX
  #include <sys/stat.h>
  #include <limits.h>
  #include <unistd.h>
  typedef struct stat futil_struct_stat;
  #define PATH_SEP '/'
  #define PATH_SEP_DQ "/"
  #define futil_mkdir(path) (mkdir((path), 0775) == 0)
  #define futil_stat(file_path, st) (stat((file_path), (st)) == 0)
  #define futil_access(file_path, mode) (access((file_path), (mode)) == 0)

#elif defined(PLATFORM_WINDOWS)
  #define WIN32_LEAN_AND_MEAN
  #include <windows.h>
  #include <direct.h>
  #include <io.h>
  #include <sys/stat.h>
  #define PATH_SEP '\\'
  #define PATH_SEP_DQ "\\"
  #ifndef PATH_MAX
    #define PATH_MAX MAX_PATH
  #endif
  typedef struct _stat64 futil_struct_stat;
  #define futil_mkdir(path) (_mkdir((path)) == 0)
  #define futil_stat(file_path, st) (_stat64((file_path), (st)) == 0)
  #define futil_access(file_path, mode) (_access((file_path), (mode)) == 0)
#endif

#define concat_path(path, ...) \
  concat_path_impl((path), __VA_ARGS__, NULL)

FUTILDEF Result(String) read_entire_file(const char* path);
FUTILDEF ErrorOrNot mkdir_if_not_exists(const char* path);
FUTILDEF bool is_valid_path(const char* path);
FUTILDEF bool concat_path_impl(String* path, const char* first, ...);

FUTILDEF size_t count_lines_from_file(const char* file_path);
FUTILDEF size_t count_lines_from_str(const String* s);

#endif /* FUTIL_H */
