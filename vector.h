#ifndef VECTOR_H
#define VECTOR_H

/*
  STB-Style single-header vector.h library
  Define VECTOR_IMPLEMENTATION in one translation unit before including.

  Usage:
  #define VECTOR_IMPLEMENTATION
  #include "vector.h"

  typedef char* char_ptr;
  DECL_VECTOR(int, int)
  DECL_VECTOR(float, float)
  DECL_VECTOR(char*, char_ptr)

  int main(void) {
    Vector(int) v = {0};

    vec_push(&v, 42);
    vec_push(&v, 99);

    int x = vec_at(&v, 0);
    int len = vec_len(&v);

    vec_free(&v);
  }
*/

#ifdef __cplusplus
  #define VECTORDEF extern "C"
#else
  #define VECTORDEF extern
#endif

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#define VEC_INITIAL_CAPACITY 64
static_assert(VEC_INITIAL_CAPACITY > 1, "VEC_INITIAL_CAPACITY must be >1");

/* Predefined typedefs to use in DECL_VECTOR and Vector() */
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

typedef struct {
  size_t cap;
  size_t len;
  bool is_freed;
} VectorHeader;

#define Vector(TName) Vector_##TName

typedef struct {
  VectorHeader* header;
  void** items;
  size_t elem_size;
} VectorGeneric;

#define VEC_TO_GENERIC(v) ((VectorGeneric){&(v)->h, (void**)&(v)->items, sizeof(*(v)->items)})

#define DECL_VECTOR(T, TName)                   \
  typedef struct {                              \
    VectorHeader h;                             \
    T* items;                                   \
  } Vector_##TName;

/* User-space macros, you want to use them: */

#define vec_init(TName) (Vector(TName)){0}

#define vec_push(v, ...)                                           \
  __extension__({                                                       \
      __typeof__(*(v)->items) _arr[] = {__VA_ARGS__};                \
      _vec_push(VEC_TO_GENERIC((v)), _arr, sizeof(_arr)/sizeof(_arr[0])); \
    })

// Not used this because of double evaluation of v1 and v2:
// #define vec_merge(v1, v2) (
//   __builtin_types_compatible_p(__typeof__(*(v1)->items), __typeof__(*(v2)->items))
//     ? _vec_merge(VEC_TO_GENERIC((v1)), VEC_TO_GENERIC((v2)))
//     : false)
// Used statement expression extension: (only evaluated once)
#define vec_merge(v1, v2) __extension__({ \
    __typeof__(v1) _v1 = (v1); \
    __typeof__(v2) _v2 = (v2); \
    __builtin_types_compatible_p(__typeof__(*(_v1)->items), __typeof__(*(_v2)->items)) \
        ? _vec_merge(VEC_TO_GENERIC((_v1)), VEC_TO_GENERIC((_v2))) \
        : false; \
})

#define vec_remove_unord(v, idx) \
  _vec_remove_unord(VEC_TO_GENERIC((v)), (idx))

#define vec_remove_idx(v, idx) \
  _vec_remove_idx(VEC_TO_GENERIC((v)), (idx))

#define vec_at(v, index)                        \
  ((__typeof__(*(v)->items)*)                \
   _vec_at(VEC_TO_GENERIC((v)), (index)))

#define vec_find(v, item)                       \
  __extension__({                               \
      __typeof__(*(v)->items) _tmp = (item); \
      _vec_find(VEC_TO_GENERIC((v)), &_tmp);    \
    })

#define vec_contains(v, item) (vec_find(v, item) != -1)

#define vec_pop(v, out)                         \
  _vec_pop(VEC_TO_GENERIC((v)), (out))

#define vec_reserve(v, extra)                   \
  _vec_reserve(VEC_TO_GENERIC((v)), (extra))

#define vec_free(v) _vec_free(VEC_TO_GENERIC((v)))
#define vec_isfreed(v) _vec_isfreed(VEC_TO_GENERIC((v)))

#define vec_equals(lhs, rhs)                                \
  _vec_equals(VEC_TO_GENERIC((lhs)), VEC_TO_GENERIC((rhs)))

/* Convenience accessors */
#define vec_len(v)   ((v)->h.len)
#define vec_cap(v)   ((v)->h.cap)
#define vec_esize(v) (sizeof(*(v)->items))

/* They'll return pointer */
#define vec_last(v) (vec_at((v), vec_len((v)) - 1))
#define vec_first(v) (vec_at((v), 0))

#define vec_islast(v, item) (vec_last((v)) ? (*vec_last((v)) == item) : false)
#define vec_isfirst(v, item) (vec_first((v)) ? (*vec_first((v)) == item) : false)

#define vec_foreach(v, it)                                              \
  for(__typeof__(*(v)->items)* it =                                  \
        (assert(!vec_isfreed((v)) && "Vector shouldn't be freed (possible use-after-free)"), (v)->items); \
      it < (v)->items + vec_len((v)); it++)

/* Raw Functions */

VECTORDEF bool _vec_push(VectorGeneric v, const void* values, size_t count);
VECTORDEF bool _vec_remove_unord(VectorGeneric v, size_t idx);
VECTORDEF bool _vec_remove_idx(VectorGeneric v, size_t idx);
VECTORDEF void* _vec_at(VectorGeneric v, size_t index);
VECTORDEF int _vec_find(VectorGeneric v, const void* item);
VECTORDEF bool _vec_pop(VectorGeneric v, void* out);
VECTORDEF bool _vec_reserve(VectorGeneric v, size_t extra);
VECTORDEF void _vec_free(VectorGeneric v);
VECTORDEF bool _vec_isfreed(VectorGeneric v);
VECTORDEF bool _vec_equals(VectorGeneric lhs, VectorGeneric rhs);
VECTORDEF bool _vec_merge(VectorGeneric v1, VectorGeneric v2);

// IMPLEMENTATION BEGIN
#ifdef VECTOR_IMPLEMENTATION

#include <stdlib.h>
#include <string.h>

bool _vec_reserve(VectorGeneric v, size_t extra) {
  // no need to check UAF
  if (v.header->cap >= SIZE_MAX / 2) return false;
  size_t needed = v.header->len + extra;

  // enough capacity, no need to realloc
  if (v.header->cap >= needed) return true;

  // calculate new capacity
  size_t new_cap = v.header->cap ? v.header->cap : VEC_INITIAL_CAPACITY;
  while (new_cap < needed)
    new_cap += (new_cap >> 1); // roughly multiply by 1.5

  void* tmp = realloc(*v.items, new_cap * v.elem_size);
  if (!tmp) return false;
  *v.items = tmp;
  v.header->cap = new_cap;
  return true;
}

bool _vec_push(VectorGeneric v, const void* values, size_t count) {
  if (_vec_isfreed(v) || !_vec_reserve(v, count)) return false;

  memcpy(
    (char*)(*v.items) + v.header->len * v.elem_size,
    values,
    v.elem_size * count
    );
  v.header->len += count;
  return true;
}

bool _vec_pop(VectorGeneric v, void* out) {
  if (v.header->len == 0) return false; // already checks UAF
  if (out) memcpy(out, _vec_at(v, v.header->len - 1), v.elem_size);
  v.header->len -= 1;
  return true;
}

bool _vec_remove_unord(VectorGeneric v, size_t idx) {
  if (idx >= v.header->len) return false; // already checks UAF
  memmove(_vec_at(v, idx), _vec_at(v, v.header->len - 1), v.elem_size);
  v.header->len -= 1;
  return true;
}

bool _vec_remove_idx(VectorGeneric v, size_t idx) {
  if (idx >= v.header->len) return false; // already checks UAF
  memmove(_vec_at(v, idx), _vec_at(v, idx + 1), (v.header->len - idx - 1)*v.elem_size);
  v.header->len -= 1;
  return true;
}

void _vec_free(VectorGeneric v) {
  free(*v.items); // it'll trigger double free then abortion
  *v.items = NULL;
  v.header->len = 0;
  v.header->cap = 0;
  v.header->is_freed = true;
}

bool _vec_isfreed(VectorGeneric v) {
  return v.header->is_freed;
}

void* _vec_at(VectorGeneric v, size_t idx) {
  if (idx >= v.header->len) return NULL; // already checks UAF
  return (char*)(*v.items) + idx * v.elem_size;
}

int _vec_find(VectorGeneric v, const void* item) {
  for (size_t i = 0; i < v.header->len; i++) { // already checks boundaries and UAF
    if (memcmp(_vec_at(v, i), item, v.elem_size) == 0) return (int)i;
  }
  return -1;
}

bool _vec_equals(VectorGeneric lhs, VectorGeneric rhs) {
  if (_vec_isfreed(lhs) || _vec_isfreed(rhs)) return false;
  if (*lhs.items == *rhs.items) return true;
  if (lhs.elem_size != rhs.elem_size
      || lhs.header->len != rhs.header->len) return false;
  return memcmp(*lhs.items, *rhs.items, lhs.header->len * lhs.elem_size) == 0;
}

bool _vec_merge(VectorGeneric v1, VectorGeneric v2) {
  assert(v1.elem_size == v2.elem_size);
  if (v2.header->len + v1.header->len >= v1.header->cap)
    _vec_reserve(v1, v2.header->len);
  _vec_push(v1, *v2.items, v2.header->len);
  return true;
}

#endif // VECTOR_IMPLEMENTATION
// IMPLEMENTATION END
#endif /* VECTOR_H */

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
