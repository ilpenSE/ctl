#ifndef FUTIL_H
#define FUTIL_H

#ifdef __cplusplus
  #define FUTILDEF extern "C"
#else
  #define FUTILDEF extern
#endif

#include <stddef.h>
#include <stdbool.h>
#include "str.h"
#include "either.h"

DECL_RESULT(String, String);

FUTILDEF Result(String) read_entire_file(const char* path);
FUTILDEF size_t count_lines_from_file(const char* file_path);
FUTILDEF size_t count_lines_from_str(const String* s);

// IMPLEMENTATION BEGIN
#ifdef FUTIL_IMPLEMENTATION
#include <stdio.h>

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
  size_t lines = 0;
  Result(String) res = read_entire_file(file_path);
  if (RES_ISERR(res)) return -1;
  String s = RES_UNWRAP(res);
  str_foreach(&s, it) {
    if (*it == '\n') lines += 1;
  }
  str_free(&s);
  return lines;
}

#endif // FUTIL_IMPLEMENTATION
// IMPLEMENTATION END
#endif // FUTIL_H
