/* SPDX-License-Identifier: GPL-3.0-only */
#ifndef ARRAY_H
#define ARRAY_H

#ifdef _MSC_VER
  #error "This header does not support MSVC, please don't use garbage slop compilers."
#endif

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

typedef struct {
  ArrayHeader* header;
  void** items;
  size_t elem_size;
} ArrayGeneric;

#define ARR_TO_GENERIC(v) ((ArrayGeneric){&(v)->h, (void**)&(v)->items, sizeof(*(v)->items)})

/*
  DISCLAIMER: LT, RT or T is a type name so this means
  if one of them is pointer, you have to use typedef of it.
  For example: DECL_EITHER(int*, bool) won't work
  But, DECL_EITHER(int_ptr, bool) will work
  This pattern is everywhere I put generic type like "T"
*/

#define Array(T) Array_##T

#define DECL_ARRAY(T) \
  typedef struct {    \
    ArrayHeader h;    \
    T* items;         \
  } Array(T);

/* User-space macros, you want to use them: */

#define arr_init(T, ...) (Array(T)){ .h.memory={__VA_ARGS__}}

#define arr_push(v, ...)                                           \
  __extension__({                                                       \
    __typeof__(*(v)->items) _arr[] = {__VA_ARGS__};                \
    _arr_push(ARR_TO_GENERIC((v)), _arr, sizeof(_arr)/sizeof(_arr[0])); \
  })

/*
  Not used this because of double evaluation of v1 and v2:
  #define arr_merge(v1, v2) (
    __builtin_types_compatible_p(__typeof__(*(v1)->items), __typeof__(*(v2)->items))
      ? _arr_merge(ARR_TO_GENERIC((v1)), ARR_TO_GENERIC((v2)))
      : false)
  Used statement expression extension: (only evaluated once)
*/
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
#define arr_isfreed(v) _arr_isfreed(ARR_TO_GENERIC((&v)))

/* Array(T) arr_cast(T type, Array(E) arr) */
#define arr_cast(T, arr) (Array(T)){.items=(T*)((arr).items), .h=(arr).h}

#define arr_equals(lhs, rhs)                                \
  _arr_equals(ARR_TO_GENERIC((lhs)), ARR_TO_GENERIC((rhs)))

/* Convenience accessors */
#define arr_len(v)   ((v).h.len)
#define arr_cap(v)   ((v).h.cap)
#define arr_esize(v) (sizeof(*(v).items))

/* They'll return pointer */
#define arr_last(v) (arr_at((v), arr_len((*v)) - 1))
#define arr_first(v) (arr_at((v), 0))

#define arr_islast(v, item) (arr_last((v)) ? (*arr_last((v)) == item) : false)
#define arr_isfirst(v, item) (arr_first((v)) ? (*arr_first((v)) == item) : false)

#define arr_foreach(v, it)                                              \
  for(__typeof__(*(v).items)* it =                                  \
        (assert(!arr_isfreed((v)) && "Array shouldn't be freed (possible use-after-free)"), (v).items); \
      it < (v).items + arr_len((v)); it++)

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

#endif /* ARRAY_H */
