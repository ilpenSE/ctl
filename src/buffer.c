#include <buffer.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

bool _buf_push(BufferGeneric buf, const void* values, size_t count) {
  if (*buf.len + count > buf.nmemb) return false;
  memcpy(buf.items, values, count * buf.elem_size);
  *buf.len += count;
  return true;
}
