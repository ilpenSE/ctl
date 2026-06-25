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

#define concat_path(path, ...) \
  concat_path_impl((path), __VA_ARGS__, NULL)

FUTILDEF Result(String) read_entire_file(const char* path);
FUTILDEF ErrorOrNot mkdir_if_not_exists(const char* path);
FUTILDEF bool is_valid_path(const char* path);
FUTILDEF bool concat_path_impl(String* path, const char* first, ...);

FUTILDEF size_t count_lines_from_file(const char* file_path);
FUTILDEF size_t count_lines_from_str(const String* s);

#endif /* FUTIL_H */
