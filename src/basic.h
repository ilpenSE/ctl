/* SPDX-License-Identifier: GPL-3.0-only */
#ifndef BASIC_H
#define BASIC_H

#ifdef _MSC_VER
  #error "This header does not support MSVC, please don't use garbage slop compilers."
#endif

#ifdef __cplusplus
  #define BASICDEF extern "C"
#else
  #define BASICDEF extern
#endif

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdarg.h>
#include "array.h"
#include "either.h"

/* Custom malloc, realloc and free function types (if you have some sort of an arena) */
#ifndef __str_allocator_t_defined
#define __str_allocator_t_defined
typedef void* (*str_allocator_t)(size_t);
#endif

#ifndef __str_reallocator_t_defined
#define __str_reallocator_t_defined
typedef void* (*str_reallocator_t)(void*, size_t);
#endif

#ifndef __str_freer_t_defined
#define __str_freer_t_defined
typedef void  (*str_freer_t)(void*);
#endif

#ifndef __StringMemory_defined
#define __StringMemory_defined
typedef struct {
  str_allocator_t allocator;
  str_reallocator_t reallocator;
  str_freer_t freer;
} StringMemory;
#endif

#ifndef __String_defined
#define __String_defined
typedef struct {
  char* data;
  size_t len; /* does not include \0 */
  size_t cap;
  StringMemory memory;
} String;
#endif

#ifndef __StringView_defined
#define __StringView_defined
typedef struct {
  const char* data; /* not null terminated */
  size_t len;
} StringView;
#endif

#ifndef __StringSlice_defined
#define __StringSlice_defined
typedef struct {
  size_t start;
  size_t end;
} StringSlice;
#endif

#ifndef __Result_String_defined
#define __Result_String_defined
DECL_RESULT(String, String);
#endif

#ifndef __Result_double_defined
#define __Result_double_defined
DECL_RESULT(double, double);
#endif

#ifndef __Result_bool_defined
#define __Result_bool_defined
DECL_RESULT(bool, bool);
#endif

/* Predefined typedefs to use in DECL_ARRAY and Array() */
#ifndef __char_ptr_defined
#define __char_ptr_defined
typedef char* char_ptr;
#endif

#ifndef __cchar_ptr_defined
#define __cchar_ptr_defined
typedef const char* cchar_ptr;
#endif

#ifndef __int_ptr_defined
#define __int_ptr_defined
typedef int* int_ptr;
#endif

#ifndef __uchar_t_defined
#define __uchar_t_defined
typedef unsigned char uchar_t;
#endif

/* Array declerations */
#ifndef __Array_cchar_ptr_defined
#define __Array_cchar_ptr_defined
DECL_ARRAY(const char*, cchar_ptr);
#endif

/* Utility macros */
/* TODO macro, use this for not-implemented features */
#define TODO(fmt, ...) \
  do { \
    fprintf(stderr, "%s:%d: TODO: "fmt"\n", __FILE__, __LINE__, ##__VA_ARGS); \
    abort(); \
  } while (0)

/* UNREACHABLE macro, use it for regions that are not meant to be reached */
#define UNREACHABLE(fmt, ...) \
  do { \
    fprintf(stderr, "%s:%d: UNREACHABLE: "fmt"\n", __FILE__, __LINE__, ##__VA_ARGS__); \
    __builtin_unreachable(); \
  } while(0)

#define STRINGIFY(macro) #macro

/* Temporary buffer */
#define _BASIC_TEMP_BUFFER_SIZE 1024
char _basic_temp[_BASIC_TEMP_BUFFER_SIZE];
size_t _basic_temp_len = 0;

/* Math */
#define _BASIC_TYPES_WITH_BITS(X) \
  X(u64, uint64_t) \
  X(u32, uint32_t) \
  X(u16, uint16_t) \
  X(u8,  uint8_t) \
  \
  X(i64, int64_t) \
  X(i32, int32_t) \
  X(i16, int16_t) \
  X(i8,  int8_t)

/* Declerations */
BASICDEF char* temp_sprintf(const char* fmt, ...);
BASICDEF char* temp_vsprintf(const char* fmt, va_list args);
BASICDEF void temp_reset();
BASICDEF void temp_clear();

#define _BASIC_CLAMP_DECLARATION(TName, T) \
  T clamp##TName(T a, T lower, T upper); \
  typedef T TName;
_BASIC_TYPES_WITH_BITS(_BASIC_CLAMP_DECLARATION)

#define _BASIC_CLAMP_CASE(TName, T) T: clamp##TName,
#define clamp(a, lower, upper) _Generic((a), \
    _BASIC_TYPES_WITH_BITS(_BASIC_CLAMP_CASE) \
    default: clampi32 \
)(a, lower, upper)

// IMPLEMENTATION BEGIN
#ifdef BASIC_IMPLEMENTATION
#include <string.h>
#include <stdio.h>
// Temp
char* temp_sprintf(const char* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  char* result = temp_vsprintf(fmt, args);
  va_end(args);
  return result;
}

char* temp_vsprintf(const char* fmt, va_list args) {
  int n = vsnprintf(_basic_temp, sizeof(_basic_temp), fmt, args);
  if (n < 0) return NULL;
  _basic_temp_len = clamp((size_t)n + _basic_temp_len, 0, _basic_temp_len);
  return _basic_temp;
}

void temp_reset() {
  _basic_temp[0] = '\0';
  _basic_temp_len = 0;
}

void temp_clear() {
  memset(_basic_temp, 0, sizeof(_basic_temp));
  _basic_temp_len = 0;
}

// Math
#define X(TName, T) \
  T clamp##TName(T a, T lower, T upper) { \
    return a > upper ? upper : a < lower ? lower : a; \
  }
_BASIC_TYPES_WITH_BITS(X)
#undef X

#endif // BASIC_IMPLEMENTATION
// IMPLEMENTATION END
#endif /* BASIC_H */
