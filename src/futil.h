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

FUTILDEF Result(String) read_entire_file(const char* path);
FUTILDEF ErrorOrNot mkdir_if_not_exists(const char* path);
FUTILDEF bool is_valid_path(const char* path);

FUTILDEF size_t count_lines_from_file(const char* file_path);
FUTILDEF size_t count_lines_from_str(const String* s);

// IMPLEMENTATION BEGIN
#ifdef FUTIL_IMPLEMENTATION
#include <stdio.h>
#include <errno.h>

#if defined(__unix__) || defined(__APPLE__)
  #include <sys/stat.h>
  #include <limits.h>
#else
  #ifdef _WIN32
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
    #ifndef PATH_MAX
      #define PATH_MAX MAX_PATH
    #endif
  #else
    #error "Platform is not supported."
  #endif /* _WIN32 */
#endif /* __unix__ || __APPLE__ */

#ifdef _WIN32
  #define PATH_SEP '\\'
  #define FUTIL_MKDIR(path) _mkdir(path)
#else
  #define PATH_SEP '/'
  #define FUTIL_MKDIR(path) mkdir(path, 0775)
#endif /* _WIN32 */

ErrorOrNot mkdir_if_not_exists(const char* path) {
  if (!is_valid_path(path)) return EON_ERROR(ERR_INVALID_ARG, "Invalid path");
  for (const char* p = path + 1; *p != '\0'; p++) {
    if (*p == PATH_SEP) {
      size_t i = (size_t)(p - path);
      if (i >= PATH_MAX) return EON_ERROR(ERR_INVALID_ARG, "Path is too long");
      char buf[PATH_MAX] = {0};
      memcpy(buf, path, i);
      int status = FUTIL_MKDIR(buf);
      if (status != 0 && errno != EEXIST) {
        return EON_ERROR(ERR_INTERNAL, strerror(errno));
      }
    }
  }
  return EON_SUCCESS;
}

bool is_valid_path(const char* path) {
  if (!path || !*path) return false;
  // not fully implemented yet
  return true;
}

Result(String) read_entire_file(const char* file_path) {
  FILE* f = fopen(file_path, "rb");
  if (!f) return RES_ERR(String, ERR_NO_SUCH_FILE);

  if (fseek(f, 0, SEEK_END) != 0) goto fail_close;
  long size = ftell(f);
  if (size == -1L) goto fail_close;
  if (fseek(f, 0, SEEK_SET) != 0) goto fail_close;

  String s = {0};
  if (!str_reserve(&s, size)) return RES_ERR(String, ERR_MEM_CORRUPT);
  s.len = (size_t)size;
  size_t nread = fread(s.data, 1, size, f);
  if (nread < s.len) goto fail_str;
  if (fclose(f) != 0) goto fail_str;

  return RES_OK(String, s);
fail_str:
  str_free(&s);
fail_close:
  fclose(f);
  return RES_ERR(String, ERR_INTERNAL);
}

size_t count_lines_from_str(const String* s) {
  size_t lines = 0;
  str_foreach(s, it) {
    if (*it == '\n') lines += 1;
  }
  return lines;
}

size_t count_lines_from_file(const char* file_path) {
  Result(String) res = read_entire_file(file_path);
  if (RES_ISERR(res)) return -1;
  String s = RES_UNWRAP(res);
  size_t lines = count_lines_from_str(&s);
  str_free(&s);
  return lines;
}

#endif // FUTIL_IMPLEMENTATION
// IMPLEMENTATION END
#endif /* FUTIL_H */
