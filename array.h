#ifndef ARRAY_H
#define ARRAY_H

#ifdef _MSC_VER
  #error "This header does not support MSVC, please don't use garbage slop compilers."
#endif

/*
  STB-Style single-header array.h library
  Define ARRAY_IMPLEMENTATION in one translation unit before including.

  Usage:
  #define ARRAY_IMPLEMENTATION
  #include "array.h"

  typedef char* char_ptr;
  DECL_ARRAY(int, int)
  DECL_ARRAY(float, float)
  DECL_ARRAY(char*, char_ptr)

  int main(void) {
    Array(int) v = {0};

    arr_push(&v, 42);
    arr_push(&v, 99);

    int x = arr_at(&v, 0);
    int len = arr_len(&v);

    arr_free(&v);
  }
*/

#ifdef __cplusplus
  #define ARRAYDEF extern "C"
#else
  #define ARRAYDEF extern
#endif

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#define ARR_INITIAL_CAPACITY 64
static_assert(ARR_INITIAL_CAPACITY > 1, "ARR_INITIAL_CAPACITY must be >1");

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

#ifndef __arr_allocator_t_defined
#define __arr_allocator_t_defined
typedef void* (*arr_allocator_t)(size_t);
#endif /* __arr_allocator_t_defined */

#ifndef __arr_reallocator_t_defined
#define __arr_reallocator_t_defined
typedef void* (*arr_reallocator_t)(void*, size_t);
#endif /* __arr_reallocator_t_defined */

#ifndef __arr_freer_t_defined
#define __arr_freer_t_defined
typedef void  (*arr_freer_t)(void*);
#endif /* __arr_freer_t_defined */

typedef struct {
  arr_allocator_t allocator;
  arr_reallocator_t reallocator;
  arr_freer_t freer;
} ArrayMemory;

typedef struct {
  size_t cap;
  size_t len;
  bool is_freed;
  ArrayMemory memory;
} ArrayHeader;

#define Array(TName) Array_##TName

typedef struct {
  ArrayHeader* header;
  void** items;
  size_t elem_size;
} ArrayGeneric;

#define ARR_TO_GENERIC(v) ((ArrayGeneric){&(v)->h, (void**)&(v)->items, sizeof(*(v)->items)})

#define DECL_ARRAY(T, TName)                   \
  typedef struct {                              \
    ArrayHeader h;                             \
    T* items;                                   \
  } Array_##TName;

/* User-space macros, you want to use them: */

#define arr_init(TName, ...) (Array(TName)){ .h.memory={__VA_ARGS__}}

#define arr_push(v, ...)                                           \
  __extension__({                                                       \
    __typeof__(*(v)->items) _arr[] = {__VA_ARGS__};                \
    _arr_push(ARR_TO_GENERIC((v)), _arr, sizeof(_arr)/sizeof(_arr[0])); \
  })

// Not used this because of double evaluation of v1 and v2:
// #define arr_merge(v1, v2) (
//   __builtin_types_compatible_p(__typeof__(*(v1)->items), __typeof__(*(v2)->items))
//     ? _arr_merge(ARR_TO_GENERIC((v1)), ARR_TO_GENERIC((v2)))
//     : false)
// Used statement expression extension: (only evaluated once)
#define arr_merge(v1, v2) \
  __extension__({ \
    __typeof__(v1) _v1 = (v1); \
    __typeof__(v2) _v2 = (v2); \
    __builtin_types_compatible_p(__typeof__(*(_v1)->items), __typeof__(*(_v2)->items)) \
        ? _arr_merge(ARR_TO_GENERIC((_v1)), ARR_TO_GENERIC((_v2))) \
        : false; \
  })

#define arr_remove_unord(v, idx) \
  _arr_remove_unord(ARR_TO_GENERIC((v)), (idx))

#define arr_remove_idx(v, idx) \
  _arr_remove_idx(ARR_TO_GENERIC((v)), (idx))

#define arr_at(v, index)                        \
  ((__typeof__(*(v)->items)*)                \
   _arr_at(ARR_TO_GENERIC((v)), (index)))

#define arr_find(v, item)                       \
  __extension__({                               \
      __typeof__(*(v)->items) _tmp = (item); \
      _arr_find(ARR_TO_GENERIC((v)), &_tmp);    \
    })

#define arr_contains(v, item) (arr_find(v, item) != -1)

#define arr_pop(v, out)                         \
  _arr_pop(ARR_TO_GENERIC((v)), (out))

#define arr_reserve(v, extra)                   \
  _arr_reserve(ARR_TO_GENERIC((v)), (extra))

#define arr_free(v) _arr_free(ARR_TO_GENERIC((v)))
#define arr_isfreed(v) _arr_isfreed(ARR_TO_GENERIC((v)))

#define arr_equals(lhs, rhs)                                \
  _arr_equals(ARR_TO_GENERIC((lhs)), ARR_TO_GENERIC((rhs)))

/* Convenience accessors */
#define arr_len(v)   ((v)->h.len)
#define arr_cap(v)   ((v)->h.cap)
#define arr_esize(v) (sizeof(*(v)->items))

/* They'll return pointer */
#define arr_last(v) (arr_at((v), arr_len((v)) - 1))
#define arr_first(v) (arr_at((v), 0))

#define arr_islast(v, item) (arr_last((v)) ? (*arr_last((v)) == item) : false)
#define arr_isfirst(v, item) (arr_first((v)) ? (*arr_first((v)) == item) : false)

#define arr_foreach(v, it)                                              \
  for(__typeof__(*(v)->items)* it =                                  \
        (assert(!arr_isfreed((v)) && "Array shouldn't be freed (possible use-after-free)"), (v)->items); \
      it < (v)->items + arr_len((v)); it++)

/* Raw Functions */

ARRAYDEF bool _arr_push(ArrayGeneric v, const void* values, size_t count);
ARRAYDEF bool _arr_remove_unord(ArrayGeneric v, size_t idx);
ARRAYDEF bool _arr_remove_idx(ArrayGeneric v, size_t idx);
ARRAYDEF void* _arr_at(ArrayGeneric v, size_t index);
ARRAYDEF int _arr_find(ArrayGeneric v, const void* item);
ARRAYDEF bool _arr_pop(ArrayGeneric v, void* out);
ARRAYDEF bool _arr_reserve(ArrayGeneric v, size_t extra);
ARRAYDEF void _arr_free(ArrayGeneric v);
ARRAYDEF bool _arr_isfreed(ArrayGeneric v);
ARRAYDEF bool _arr_equals(ArrayGeneric lhs, ArrayGeneric rhs);
ARRAYDEF bool _arr_merge(ArrayGeneric v1, ArrayGeneric v2);

// IMPLEMENTATION BEGIN
#ifdef ARRAY_IMPLEMENTATION

#include <stdlib.h>
#include <string.h>

bool _arr_reserve(ArrayGeneric v, size_t extra) {
  // no need to check UAF
  if (v.header->cap >= SIZE_MAX / 2) return false;
  size_t needed = v.header->len + extra;

  // enough capacity, no need to realloc
  if (v.header->cap >= needed) return true;

  // calculate new capacity
  size_t new_cap = v.header->cap ? v.header->cap : ARR_INITIAL_CAPACITY;
  while (new_cap < needed)
    new_cap += (new_cap >> 1); // roughly multiply by 1.5

  // Use realloc from stdlib.h if no custom realloc function preset.
  arr_reallocator_t realloc_fn = v.header->memory.reallocator;
  if (realloc_fn == NULL) realloc_fn = realloc;
  void* tmp = realloc_fn(*v.items, new_cap * v.elem_size);
  if (!tmp) return false;

  *v.items = tmp;
  v.header->cap = new_cap;
  return true;
}

bool _arr_push(ArrayGeneric v, const void* values, size_t count) {
  if (_arr_isfreed(v) || !_arr_reserve(v, count)) return false;

  memcpy(
    (char*)(*v.items) + v.header->len * v.elem_size,
    values,
    v.elem_size * count
    );
  v.header->len += count;
  return true;
}

bool _arr_pop(ArrayGeneric v, void* out) {
  if (v.header->len == 0) return false; // already checks UAF
  if (out) memcpy(out, _arr_at(v, v.header->len - 1), v.elem_size);
  v.header->len -= 1;
  return true;
}

bool _arr_remove_unord(ArrayGeneric v, size_t idx) {
  if (idx >= v.header->len) return false; // already checks UAF
  memmove(_arr_at(v, idx), _arr_at(v, v.header->len - 1), v.elem_size);
  v.header->len -= 1;
  return true;
}

bool _arr_remove_idx(ArrayGeneric v, size_t idx) {
  if (idx >= v.header->len) return false; // already checks UAF
  memmove(_arr_at(v, idx), _arr_at(v, idx + 1), (v.header->len - idx - 1)*v.elem_size);
  v.header->len -= 1;
  return true;
}

void _arr_free(ArrayGeneric v) {
  // Use free from stdlib.h if no custom free function preset.
  arr_freer_t free_fn = v.header->memory.freer;
  if (free_fn == NULL) free_fn = free;
  free_fn(*v.items); // it'll trigger double free then abortion
  *v.items = NULL;
  v.header->len = 0;
  v.header->cap = 0;
  v.header->is_freed = true;
}

bool _arr_isfreed(ArrayGeneric v) {
  return v.header->is_freed;
}

void* _arr_at(ArrayGeneric v, size_t idx) {
  if (idx >= v.header->len) return NULL; // already checks UAF
  return (char*)(*v.items) + idx * v.elem_size;
}

int _arr_find(ArrayGeneric v, const void* item) {
  for (size_t i = 0; i < v.header->len; i++) { // already checks boundaries and UAF
    if (memcmp(_arr_at(v, i), item, v.elem_size) == 0) return (int)i;
  }
  return -1;
}

bool _arr_equals(ArrayGeneric lhs, ArrayGeneric rhs) {
  if (_arr_isfreed(lhs) || _arr_isfreed(rhs)) return false;
  if (*lhs.items == *rhs.items) return true;
  if (lhs.elem_size != rhs.elem_size
      || lhs.header->len != rhs.header->len) return false;
  return memcmp(*lhs.items, *rhs.items, lhs.header->len * lhs.elem_size) == 0;
}

bool _arr_merge(ArrayGeneric v1, ArrayGeneric v2) {
  assert(v1.elem_size == v2.elem_size);
  if (v2.header->len + v1.header->len >= v1.header->cap)
    _arr_reserve(v1, v2.header->len);
  _arr_push(v1, *v2.items, v2.header->len);
  return true;
}

#endif // ARRAY_IMPLEMENTATION
// IMPLEMENTATION END
#endif /* ARRAY_H */

/*
  The MIT License
  Copyright (c) 2026 ilpeN

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
*/
