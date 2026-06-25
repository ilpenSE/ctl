#include <array.h>
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
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
